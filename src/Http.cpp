/**
 * @file
 * @author chu
 * @date 2018/8/5
 */
#include <Moe.Core/Http.hpp>

using namespace std;
using namespace moe;

struct HttpStatusTextRecord
{
    HttpStatus Status;
    const char* Text;

    bool operator<(const HttpStatusTextRecord& rhs)const noexcept
    {
        return static_cast<unsigned>(Status) < static_cast<unsigned>(rhs.Status);
    }

    bool operator<(HttpStatus s)const noexcept
    {
        return static_cast<unsigned>(Status) < static_cast<unsigned>(s);
    }
};

static const HttpStatusTextRecord kHttpStatusTexts[] = {
    { HttpStatus::Continue, "Continue" },
    { HttpStatus::SwitchingProtocols, "Switching Protocols" },
    { HttpStatus::Processing, "Processing" },
    { HttpStatus::Ok, "OK" },
    { HttpStatus::Created, "Created" },
    { HttpStatus::Accepted, "Accepted" },
    { HttpStatus::NonAuthoritativeInformation, "Non-Authoritative Information" },
    { HttpStatus::NoContent, "No Content" },
    { HttpStatus::ResetContent, "Reset Content" },
    { HttpStatus::PartialContent, "Partial Content" },
    { HttpStatus::MultiStatus, "Multi-Status" },
    { HttpStatus::AlreadyReported, "Already Reported" },
    { HttpStatus::ImUsed, "IM Used" },
    { HttpStatus::MultipleChoices, "Multiple Choices" },
    { HttpStatus::MovedPermanently, "Moved Permanently" },
    { HttpStatus::Found, "Found" },
    { HttpStatus::SeeOther, "See Other" },
    { HttpStatus::NotModified, "Not Modified" },
    { HttpStatus::UseProxy, "Use Proxy" },
    { HttpStatus::TemporaryRedirect, "Temporary Redirect" },
    { HttpStatus::PermanentRedirect, "Permanent Redirect" },
    { HttpStatus::BadRequest, "Bad Request" },
    { HttpStatus::Unauthorized, "Unauthorized" },
    { HttpStatus::PaymentRequired, "Payment Required" },
    { HttpStatus::Forbidden, "Forbidden" },
    { HttpStatus::NotFound, "Not Found" },
    { HttpStatus::MethodNotAllowed, "Method Not Allowed" },
    { HttpStatus::NotAcceptable, "Not Acceptable" },
    { HttpStatus::ProxyAuthenticationRequired, "Proxy Authentication Required" },
    { HttpStatus::RequestTimeout, "Request Timeout" },
    { HttpStatus::Conflict, "Conflict" },
    { HttpStatus::Gone, "Gone" },
    { HttpStatus::LengthRequired, "Length Required" },
    { HttpStatus::PreconditionFailed, "Precondition Failed" },
    { HttpStatus::PayloadTooLarge, "Payload Too Large" },
    { HttpStatus::UriTooLong, "URI Too Long" },
    { HttpStatus::UnsupportedMediaType, "Unsupported Media Type" },
    { HttpStatus::RangeNotSatisfiable, "Range Not Satisfiable" },
    { HttpStatus::ExpectationFailed, "Expectation Failed" },
    { HttpStatus::MisdirectedRequest, "Misdirected Request" },
    { HttpStatus::UnprocessableEntity, "Unprocessable Entity" },
    { HttpStatus::Locked, "Locked" },
    { HttpStatus::FailedDependency, "Failed Dependency" },
    { HttpStatus::UpgradeRequired, "Upgrade Required" },
    { HttpStatus::PreconditionRequired, "Precondition Required" },
    { HttpStatus::TooManyRequests, "Too Many Requests" },
    { HttpStatus::RequestHeaderFieldsTooLarge, "Request Header Fields Too Large" },
    { HttpStatus::UnavailableForLegalReasons, "Unavailable For Legal Reasons" },
    { HttpStatus::InternalServerError, "Internal Server Error" },
    { HttpStatus::NotImplemented, "Not Implemented" },
    { HttpStatus::BadGateway, "Bad Gateway" },
    { HttpStatus::ServiceUnavailable, "Service Unavailable" },
    { HttpStatus::GatewayTimeout, "Gateway Timeout" },
    { HttpStatus::HttpVersionNotSupported, "HTTP Version Not Supported" },
    { HttpStatus::VariantAlsoNegotiates, "Variant Also Negotiates" },
    { HttpStatus::InsufficientStorage, "Insufficient Storage" },
    { HttpStatus::LoopDetected, "Loop Detected" },
    { HttpStatus::NotExtended, "Not Extended" },
    { HttpStatus::NetworkAuthenticationRequired, "Network Authentication Required" },
};

static const char* kHttpMethodsText[] = {
    "<unknown>",
    "DELETE",
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "CONNECT",
    "OPTIONS",
    "TRACE",
    "COPY",
    "LOCK",
    "MKCOL",
    "MOVE",
    "PROPFIND",
    "PROPPATCH",
    "SEARCH",
    "UNLOCK",
    "BIND",
    "REBIND",
    "UNBIND",
    "ACL",
    "REPORT",
    "MKACTIVITY",
    "CHECKOUT",
    "MERGE",
    "M-SEARCH",
    "NOTIFY",
    "SUBSCRIBE",
    "UNSUBSCRIBE",
    "PATCH",
    "PURGE",
    "MKCALENDAR",
    "LINK",
    "UNLINK",
    "SOURCE",
};
static_assert(static_cast<int>(HttpMethods::Unknown) == 0, "Error");
static_assert(static_cast<int>(HttpMethods::Source) == 34, "Error");
static_assert(sizeof(kHttpMethodsText) / sizeof(const char*) == 35, "Error");

const char* moe::GetHttpStatusText(HttpStatus status)noexcept
{
    auto p = lower_bound(kHttpStatusTexts, kHttpStatusTexts + CountOf(kHttpStatusTexts), status);
    if (p == kHttpStatusTexts + CountOf(kHttpStatusTexts))
        return "Unknown";
    return p->Text;
}

const char* moe::GetHttpMethodsText(HttpMethods method)noexcept
{
    auto idx = static_cast<size_t>(method);
    if (idx >= CountOf(kHttpMethodsText))
    {
        assert(false);
        idx = 0;
    }
    return kHttpMethodsText[idx];
}

//////////////////////////////////////////////////////////////////////////////// HttpParserBase

static const char* const kProxyConnection = "proxy-connection";
static const char* const kConnection = "connection";
static const char* const kContentLength = "content-length";
static const char* const kTransferEncoding = "transfer-encoding";
static const char* const kUpgrade = "upgrade";
static const char* const kChunked = "chunked";
static const char* const kKeepAlive = "keep-alive";
static const char* const kClose = "close";

/* Tokens as defined by rfc 2616. Also lowercases them.
 *        token       = 1*<any CHAR except CTLs or separators>
 *     separators     = "(" | ")" | "<" | ">" | "@"
 *                    | "," | ";" | ":" | "\" | <">
 *                    | "/" | "[" | "]" | "?" | "="
 *                    | "{" | "}" | SP | HT
 */
static const char kHttpTokens[256] = {
/*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel  */
    0,       0,       0,       0,       0,       0,       0,       0,
/*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si   */
    0,       0,       0,       0,       0,       0,       0,       0,
/*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
    0,       0,       0,       0,       0,       0,       0,       0,
/*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
    0,       0,       0,       0,       0,       0,       0,       0,
/*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
    ' ',     '!',      0,      '#',     '$',     '%',     '&',    '\'',
/*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
    0,       0,      '*',     '+',      0,      '-',     '.',      0,
/*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
    '0',     '1',     '2',     '3',     '4',     '5',     '6',     '7',
/*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
    '8',     '9',      0,       0,       0,       0,       0,       0,
/*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
    0,      'a',     'b',     'c',     'd',     'e',     'f',     'g',
/*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
    'h',     'i',     'j',     'k',     'l',     'm',     'n',     'o',
/*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
    'p',     'q',     'r',     's',     't',     'u',     'v',     'w',
/*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
    'x',     'y',     'z',      0,       0,       0,      '^',     '_',
/*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
    '`',     'a',     'b',     'c',     'd',     'e',     'f',     'g',
/* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
    'h',     'i',     'j',     'k',     'l',     'm',     'n',     'o',
/* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
    'p',     'q',     'r',     's',     't',     'u',     'v',     'w',
/* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
    'x',     'y',     'z',      0,      '|',      0,      '~',       0 };


static const uint8_t kNormalUrlChar[32] = {
/*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel  */
    0    |   0    |   0    |   0    |   0    |   0    |   0    |   0,
/*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si   */
    0    |   2    |   0    |   0    |   16   |   0    |   0    |   0,
/*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
    0    |   0    |   0    |   0    |   0    |   0    |   0    |   0,
/*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
    0    |   0    |   0    |   0    |   0    |   0    |   0    |   0,
/*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
    0    |   2    |   4    |   0    |   16   |   32   |   64   |  128,
/*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
    1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
    1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
    1    |   2    |   4    |   8    |   16   |   32   |   64   |   0,
/*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
    1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
    1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
    1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
    1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
    1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
    1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
    1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
/* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
    1    |   2    |   4    |   8    |   16   |   32   |   64   |   0, };

enum States
{
    STATE_DEAD = 1,  // 必须大于0

    STATE_START_REQ_OR_RES,
    STATE_RES_OR_RESP_H,
    STATE_START_RES,
    STATE_RES_H,
    STATE_RES_HT,
    STATE_RES_HTT,
    STATE_RES_HTTP,
    STATE_RES_HTTP_MAJOR,
    STATE_RES_HTTP_DOT,
    STATE_RES_HTTP_MINOR,
    STATE_RES_HTTP_END,
    STATE_RES_FIRST_STATUS_CODE,
    STATE_RES_STATUS_CODE,
    STATE_RES_STATUS_START,
    STATE_RES_STATUS,
    STATE_RES_LINE_ALMOST_DONE,

    STATE_START_REQ,

    STATE_REQ_METHOD,
    STATE_REQ_SPACES_BEFORE_URL,
    STATE_REQ_SCHEMA,
    STATE_REQ_SCHEMA_SLASH,
    STATE_REQ_SCHEMA_SLASH_SLASH,
    STATE_REQ_SERVER_START,
    STATE_REQ_SERVER,
    STATE_REQ_SERVER_WITH_AT,
    STATE_REQ_PATH,
    STATE_REQ_QUERY_STRING_START,
    STATE_REQ_QUERY_STRING,
    STATE_REQ_FRAGMENT_START,
    STATE_REQ_FRAGMENT,
    STATE_REQ_HTTP_START,
    STATE_REQ_HTTP_H,
    STATE_REQ_HTTP_HT,
    STATE_REQ_HTTP_HTT,
    STATE_REQ_HTTP_HTTP,
    STATE_REQ_HTTP_MAJOR,
    STATE_REQ_HTTP_DOT,
    STATE_REQ_HTTP_MINOR,
    STATE_REQ_HTTP_END,
    STATE_REQ_LINE_ALMOST_DONE,

    STATE_HEADER_FIELD_START,
    STATE_HEADER_FIELD,
    STATE_HEADER_VALUE_DISCARD_WS,
    STATE_HEADER_VALUE_DISCARD_WS_ALMOST_DONE,
    STATE_HEADER_VALUE_DISCARD_LWS,
    STATE_HEADER_VALUE_START,
    STATE_HEADER_VALUE,
    STATE_HEADER_VALUE_LWS,

    STATE_HEADER_ALMOST_DONE,

    STATE_CHUNK_SIZE_START,
    STATE_CHUNK_SIZE,
    STATE_CHUNK_PARAMETERS,
    STATE_CHUNK_SIZE_ALMOST_DONE,

    STATE_HEADERS_ALMOST_DONE,
    STATE_HEADERS_DONE,  // 此处必须为解析头部的最后一个状态码

    STATE_CHUNK_DATA,
    STATE_CHUNK_DATA_ALMOST_DONE,
    STATE_CHUNK_DATA_DONE,

    STATE_BODY_IDENTITY,
    STATE_BODY_IDENTITY_EOF,

    STATE_MESSAGE_DONE,
};

enum HeaderStates
{
    HEADER_STATE_GENERAL = 0,
    HEADER_STATE_C,
    HEADER_STATE_CO,
    HEADER_STATE_CON,

    HEADER_STATE_MATCHING_CONNECTION,
    HEADER_STATE_MATCHING_PROXY_CONNECTION,
    HEADER_STATE_MATCHING_CONTENT_LENGTH,
    HEADER_STATE_MATCHING_TRANSFER_ENCODING,
    HEADER_STATE_MATCHING_UPGRADE,

    HEADER_STATE_CONNECTION,
    HEADER_STATE_CONTENT_LENGTH,
    HEADER_STATE_CONTENT_LENGTH_NUM,
    HEADER_STATE_CONTENT_LENGTH_WS,
    HEADER_STATE_TRANSFER_ENCODING,
    HEADER_STATE_UPGRADE,

    HEADER_STATE_MATCHING_TRANSFER_ENCODING_CHUNKED,
    HEADER_STATE_MATCHING_CONNECTION_TOKEN_START,
    HEADER_STATE_MATCHING_CONNECTION_KEEP_ALIVE,
    HEADER_STATE_MATCHING_CONNECTION_CLOSE,
    HEADER_STATE_MATCHING_CONNECTION_UPGRADE,
    HEADER_STATE_MATCHING_CONNECTION_TOKEN,

    HEADER_STATE_TRANSFER_ENCODING_CHUNKED,
    HEADER_STATE_CONNECTION_KEEP_ALIVE,
    HEADER_STATE_CONNECTION_CLOSE,
    HEADER_STATE_CONNECTION_UPGRADE,
};

enum Flags
{
    FLAGS_CHUNKED = 1 << 0,
    FLAGS_CONNECTION_KEEP_ALIVE = 1 << 1,
    FLAGS_CONNECTION_CLOSE = 1 << 2,
    FLAGS_CONNECTION_UPGRADE = 1 << 3,
    FLAGS_TRAILING = 1 << 4,
    FLAGS_UPGRADE = 1 << 5,
    FLAGS_SKIPBODY = 1 << 6,
    FLAGS_CONTENTLENGTH = 1 << 7,
};

namespace
{
    /**
     * @brief 检查是否是有效的HttpToken字符
     * @param ch 字符
     * @param strict 严格模式，设为true，则SP为非法字符。
     * @return 若不是有效字符，则返回'\0'，否则返回小写形式。
     */
    char IsHttpToken(char ch, bool strict)noexcept
    {
        if (strict && ch == ' ')
            return '\0';
        return kHttpTokens[static_cast<uint8_t>(ch)];
    }

    /**
     * @brief 是否是正常的HTTP头部字符
     * @param ch 字符
     */
    bool IsHttpHeaderChar(char ch)noexcept
    {
        return ch == '\r' || ch == '\n' || ch == 9 || (static_cast<uint8_t>(ch) > 31 && ch != 127);
    }

    /**
     * @brief 检查是否是UserInfo的字符
     */
    bool IsUserInfoChar(char ch)noexcept
    {
        return StringUtils::IsAlphabet(ch) || StringUtils::IsDigit(ch) || ch == '-' || ch == '_' || ch == '.' ||
            ch == '!' || ch == '~' || ch == '*' || ch == '\'' || ch == '(' ||  ch == ')' || ch == '%' || ch == ';' ||
            ch == ':' || ch == '&' || ch == '=' || ch == '+' ||  ch == '$' || ch == ',';
    }

    /**
     * @brief 检查是否是URL字符
     */
    bool IsUrlChar(char ch)noexcept
    {
        auto b = static_cast<unsigned>(ch);
        return (kNormalUrlChar[b >> 3] & (1 << (b & 7))) != 0;
    }

    /**
     * @brief 是否是解析头部的状态
     */
    inline bool IsParsingHeader(unsigned s)noexcept
    {
        return s <= STATE_HEADERS_DONE;
    }

    /**
     * @brief 检查是否接受字符
     */
    inline void Accept(char ch, char expect)
    {
        if (expect != ch)
            MOE_THROW(BadFormatException, "Character {0} expected, but found {1}", expect, ch);
    }

    /**
     * @brief 检查是否接受数字
     */
    inline uint8_t AcceptDigit(char ch)
    {
        if (!StringUtils::IsDigit(ch))
            MOE_THROW(BadFormatException, "Expected digit, but found {0}", ch);
        return static_cast<uint8_t>(ch - '0');
    }

    /**
     * @brief 检查读取的数量是否超过上限
     */
    inline void CheckSize(size_t& sz, size_t read, size_t limit)
    {
        sz += read;
        if (sz > limit)
            MOE_THROW(BadFormatException, "Read too much content, {0} > {1}", read, limit);
    }
}

bool HttpParserBase::ShouldKeepAlive()noexcept
{
    if (m_uHttpMajor > 0 && m_uHttpMinor > 0)  // HTTP/1.1
    {
        if (m_uFlags & FLAGS_CONNECTION_CLOSE)
            return false;
    }
    else  // HTTP/1.0或者更早
    {
        if (!(m_uFlags & FLAGS_CONNECTION_KEEP_ALIVE))
            return false;
    }
    return !IsEofRequired();
}

void HttpParserBase::Reset(HttpParserTypes type)noexcept
{
    m_uType = m_uParsedType = type;
    m_uState = (type == HttpParserTypes::Request ? STATE_START_REQ : (type == HttpParserTypes::Response ?
        STATE_START_RES : STATE_START_REQ_OR_RES));
    m_uRead = 0;
    m_uHeaderState = 0;
    m_uIndex = 0;
    m_uFlags = 0;
    m_uMethod = HttpMethods::Unknown;
    m_uHttpMajor = 0;
    m_uHttpMinor = 0;
    m_uStatusCode = 0;
    m_bUpgrade = false;
    m_uContentLength = 0;
}

bool HttpParserBase::IsEofRequired()noexcept
{
    if (m_uParsedType == HttpParserTypes::Request)
        return false;

    // RFC 2616 section 4.4
    if (m_uStatusCode / 100 == 1 /* 1xx e.g. Continue */ || m_uStatusCode == 204 /* No Content */ ||
        m_uStatusCode == 304 /* Not Modified */ || m_uFlags & FLAGS_SKIPBODY /* response to a HEAD request */ )
    {
        return false;
    }

    if ((m_uFlags & FLAGS_CHUNKED) || m_uContentLength != numeric_limits<uint64_t>::max())
        return false;
    return true;
}

void HttpParserBase::ParseUrl(char ch)
{
    if (ch == ' ' || ch == '\r' || ch == '\n')
        MOE_THROW(BadFormatException, "Unexpected character {0}", ch);
    else if ((ch == '\t' || ch == '\f') && m_bStrictUrlToken)
        MOE_THROW(BadFormatException, "Unexpected character {0}", ch);

    switch (m_uState)
    {
        case STATE_REQ_SPACES_BEFORE_URL:
            if (ch == '/' || ch == '*')  // 代理协议后面会跟绝对路径，其余的应该以'/'或者'*'开头
            {
                m_uState = STATE_REQ_PATH;
                return;
            }
            else if (StringUtils::IsAlphabet(ch))  // 'h'
            {
                m_uState = STATE_REQ_SCHEMA;
                return;
            }
            break;
        case STATE_REQ_SCHEMA:
            if (StringUtils::IsAlphabet(ch))  // 'http'
                return;
            else if (ch == ':')  // 'http:'
            {
                m_uState = STATE_REQ_SCHEMA_SLASH;
                return;
            }
            break;
        case STATE_REQ_SCHEMA_SLASH:
            if (ch == '/')
            {
                m_uState = STATE_REQ_SCHEMA_SLASH_SLASH;
                return;
            }
            break;
        case STATE_REQ_SCHEMA_SLASH_SLASH:
            if (ch == '/')
            {
                m_uState = STATE_REQ_SERVER_START;
                return;
            }
            break;
        case STATE_REQ_SERVER_WITH_AT:
            if (ch == '@')
            {
                m_uState = STATE_DEAD;
                return;
            }
        case STATE_REQ_SERVER_START:
        case STATE_REQ_SERVER:
            if (ch == '/')
            {
                m_uState = STATE_REQ_PATH;
                return;
            }
            else if (ch == '?')
            {
                m_uState = STATE_REQ_QUERY_STRING_START;
                return;
            }
            else if (ch == '@')
            {
                m_uState = STATE_REQ_SERVER_WITH_AT;
                return;
            }
            else if (IsUserInfoChar(ch) || ch == '[' || ch == ']')
            {
                m_uState = STATE_REQ_SERVER;
                return;
            }
            break;
        case STATE_REQ_PATH:
            if (IsUrlChar(ch))
                return;
            switch (ch)
            {
                case '?':
                    m_uState = STATE_REQ_QUERY_STRING_START;
                    return;
                case '#':
                    m_uState = STATE_REQ_FRAGMENT_START;
                    return;
                default:
                    break;
            }
            break;
        case STATE_REQ_QUERY_STRING_START:
        case STATE_REQ_QUERY_STRING:
            if (IsUrlChar(ch))
            {
                m_uState = STATE_REQ_QUERY_STRING;
                return;
            }
            switch (ch)
            {
                case '?':  // 允许在QueryString中出现额外的'?'
                    m_uState = STATE_REQ_QUERY_STRING;
                    return;
                case '#':
                    m_uState = STATE_REQ_FRAGMENT_START;
                    return;
                default:
                    break;
            }
            break;
        case STATE_REQ_FRAGMENT_START:
            if (IsUrlChar(ch))
            {
                m_uState = STATE_REQ_FRAGMENT;
                return;
            }
            switch (ch)
            {
                case '?':
                    m_uState = STATE_REQ_FRAGMENT;
                    return;
                case '#':
                    return;
                default:
                    break;
            }
            break;
        case STATE_REQ_FRAGMENT:
            if (IsUrlChar(ch))
                return;
            switch (ch)
            {
                case '?':
                case '#':
                    return;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    MOE_THROW(BadFormatException, "Unexpected character {0}", ch);
}

void HttpParserBase::ResetNewMessageState()noexcept
{
    auto type = (m_uType == HttpParserTypes::Both ? m_uParsedType : m_uType);
    if (ShouldKeepAlive())
        m_uState = (type == HttpParserTypes::Request ? STATE_START_REQ : STATE_START_RES);
    else
        m_uState = STATE_DEAD;
}

size_t HttpParserBase::ParseImpl(BytesView input)
{
    const uint8_t* statusMark = nullptr;
    const uint8_t* urlMark = nullptr;
    const uint8_t* headerFieldMark = nullptr;
    const uint8_t* headerValueMark = nullptr;
    const uint8_t* bodyMark = nullptr;

    // 处理EOF的情况
    if (input.IsEmpty())
    {
        switch (m_uState)
        {
            case STATE_BODY_IDENTITY_EOF:
                OnMessageComplete();
                return 0;
            case STATE_DEAD:
            case STATE_START_REQ_OR_RES:
            case STATE_START_RES:
            case STATE_START_REQ:
                return 0;
            default:
                MOE_THROW(BadFormatException, "Unexpected EOF");
        }
    }

    // 恢复mark
    switch (m_uState)
    {
        case STATE_HEADER_FIELD:
            headerFieldMark = input.GetBuffer();
            break;
        case STATE_HEADER_VALUE:
            headerValueMark = input.GetBuffer();
            break;
        case STATE_REQ_PATH:
        case STATE_REQ_SCHEMA:
        case STATE_REQ_SCHEMA_SLASH:
        case STATE_REQ_SCHEMA_SLASH_SLASH:
        case STATE_REQ_SERVER_START:
        case STATE_REQ_SERVER:
        case STATE_REQ_SERVER_WITH_AT:
        case STATE_REQ_QUERY_STRING_START:
        case STATE_REQ_QUERY_STRING:
        case STATE_REQ_FRAGMENT_START:
        case STATE_REQ_FRAGMENT:
            urlMark = input.GetBuffer();
            break;
        case STATE_RES_STATUS:
            statusMark = input.GetBuffer();
            break;
        default:
            break;
    }

    // 解析循环
    char c = '\0', ch = '\0';
    auto p = input.GetBuffer();
    auto end = input.GetBuffer() + input.GetSize();
    if (p != end)
    {
        ch = static_cast<char>(*p);
        if (IsParsingHeader(m_uState))
            CheckSize(m_uRead, 1, m_uMaxHeaderSize);
    }

    while (p != end)
    {
        switch (m_uState)
        {
            case STATE_DEAD:
                // 当连接被标识为"Connection: close"时，如果再读到一个HTTP请求数据，则会抛出错误
                if (ch == '\r' || ch == '\n')
                    break;
                MOE_THROW(BadFormatException, "Http connection is already closed");
//////////////////////////////////////// <editor-fold desc="HTTP_BOTH">
            case STATE_START_REQ_OR_RES:
                if (ch == '\r' || ch == '\n')
                    break;
                m_uFlags = 0;
                m_uContentLength = numeric_limits<uint64_t>::max();

                if (ch == 'H')
                {
                    m_uState = STATE_RES_OR_RESP_H;
                    OnMessageBegin();
                }
                else
                {
                    m_uParsedType = HttpParserTypes::Request;
                    m_uState = STATE_START_REQ;
                    continue;
                }
                break;
            case STATE_RES_OR_RESP_H:
                if (ch == 'T')
                {
                    m_uType = HttpParserTypes::Response;
                    m_uState = STATE_RES_HT;
                }
                else
                {
                    Accept(ch, 'E');
                    m_uType = HttpParserTypes::Request;
                    m_uMethod = HttpMethods::Head;
                    m_uIndex = 2;
                    m_uState = STATE_REQ_METHOD;
                }
                break;
//////////////////////////////////////// </editor-fold>
//////////////////////////////////////// <editor-fold desc="HTTP_RES">
            case STATE_START_RES:
                if (ch == '\r' || ch == '\n')
                    break;
                Accept(ch, 'H');  // 'H'
                m_uFlags = 0;
                m_uContentLength = numeric_limits<uint64_t>::max();
                m_uState = STATE_RES_H;
                OnMessageBegin();
                break;
            case STATE_RES_H:
                Accept(ch, 'T');  // 'HT'
                m_uState = STATE_RES_HT;
                break;
            case STATE_RES_HT:
                Accept(ch, 'T');  // 'HTT'
                m_uState = STATE_RES_HTT;
                break;
            case STATE_RES_HTT:
                Accept(ch, 'P');  // 'HTTP'
                m_uState = STATE_RES_HTTP;
                break;
            case STATE_RES_HTTP:
                Accept(ch, '/');  // 'HTTP/'
                m_uState = STATE_RES_HTTP_MAJOR;
                break;
            case STATE_RES_HTTP_MAJOR:
                m_uHttpMajor = AcceptDigit(ch);  // 'HTTP/1'
                m_uState = STATE_RES_HTTP_DOT;
                break;
            case STATE_RES_HTTP_DOT:
                Accept(ch, '.');  // 'HTTP/1.'
                m_uState = STATE_RES_HTTP_MINOR;
                break;
            case STATE_RES_HTTP_MINOR:
                m_uHttpMinor = AcceptDigit(ch);  // 'HTTP/1.1'
                m_uState = STATE_RES_HTTP_END;
                break;
            case STATE_RES_HTTP_END:
                Accept(ch, ' ');  // 'HTTP/1.1 '
                m_uState = STATE_RES_FIRST_STATUS_CODE;
                break;
            case STATE_RES_FIRST_STATUS_CODE:
                if (ch == ' ')  // 'HTTP/1.1  '
                    break;
                m_uStatusCode = AcceptDigit(ch);  // 'HTTP/1.1 2'
                m_uState = STATE_RES_STATUS_CODE;
                break;
            case STATE_RES_STATUS_CODE:
                if (!StringUtils::IsDigit(ch))
                {
                    if (ch == '\r' || ch == '\n')
                    {
                        m_uState = STATE_RES_STATUS_START;
                        continue;
                    }

                    Accept(ch, ' ');  // 'HTTP/1.1 200 '
                    m_uState = STATE_RES_STATUS_START;
                    break;
                }

                m_uStatusCode = m_uStatusCode * 10 + AcceptDigit(ch);  // 'HTTP/1.1 200'
                if (m_uStatusCode > 999)
                    MOE_THROW(BadFormatException, "Status code is overflowed");
                break;
            case STATE_RES_STATUS_START:
                statusMark = p;
                m_uState = STATE_RES_STATUS;
                m_uIndex = 0;
                if (ch == '\r' || ch == '\n')
                    continue;
                break;
            case STATE_RES_STATUS:
                if (ch == '\r')  // 'HTTP/1.1 200 OK\r'
                {
                    m_uState = STATE_RES_LINE_ALMOST_DONE;

                    if (statusMark)
                    {
                        OnStatus(BytesView(statusMark, p - statusMark));
                        statusMark = nullptr;
                    }
                    break;
                }
                else if (ch == '\n')
                {
                    m_uState = STATE_HEADER_FIELD_START;

                    if (statusMark)
                    {
                        OnStatus(BytesView(statusMark, p - statusMark));
                        statusMark = nullptr;
                    }
                    break;
                }
                break;
            case STATE_RES_LINE_ALMOST_DONE:
                Accept(ch, '\n');  // 'HTTP/1.1 200 OK\r\n'
                m_uState = STATE_HEADER_FIELD_START;
                break;
//////////////////////////////////////// </editor-fold>
//////////////////////////////////////// <editor-fold desc="HTTP_REQ">
            case STATE_START_REQ:
                if (ch == '\r' || ch == '\n')
                    break;
                if (!StringUtils::IsAlphabet(ch))
                    MOE_THROW(BadFormatException, "Unexpected character {0}", ch);
                m_uFlags = 0;
                m_uContentLength = numeric_limits<uint64_t>::max();
                m_uMethod = HttpMethods::Unknown;
                m_uIndex = 1;
                switch (ch)
                {
                    case 'A': m_uMethod = HttpMethods::Acl; break;
                    case 'B': m_uMethod = HttpMethods::Bind; break;
                    case 'C': m_uMethod = HttpMethods::Connect; break;  // CONNECT|COPY|CHECKOUT
                    case 'D': m_uMethod = HttpMethods::Delete; break;
                    case 'G': m_uMethod = HttpMethods::Get; break;
                    case 'H': m_uMethod = HttpMethods::Head; break;
                    case 'L': m_uMethod = HttpMethods::Lock; break;  // LOCK|LINK
                    case 'M': m_uMethod = HttpMethods::MkCol; break;  // MKCOL|MOVE|MKACTIVITY|MERGE|M-SEARCH|MKCALENDAR
                    case 'N': m_uMethod = HttpMethods::Notify; break;
                    case 'O': m_uMethod = HttpMethods::Options; break;
                    case 'P': m_uMethod = HttpMethods::Post; break;  // POST|PROPFIND|PROPPATCH|PUT|PATCH|PURGE
                    case 'R': m_uMethod = HttpMethods::Report; break;  // REPORT|REBIND
                    case 'S': m_uMethod = HttpMethods::Subscribe; break;  // SUBSCRIBE|SEARCH|SOURCE
                    case 'T': m_uMethod = HttpMethods::Trace; break;
                    case 'U': m_uMethod = HttpMethods::Unlock; break;  // UNLOCK|UNSUBSCRIBE|UNBIND|UNLINK
                    default:
                        MOE_THROW(BadFormatException, "Unexpected character {0}", ch);
                }
                m_uState = STATE_REQ_METHOD;
                OnMessageBegin();
                break;
            case STATE_REQ_METHOD:
                if (ch == '\0')
                    MOE_THROW(BadFormatException, "Unexpected character {0}", ch);
                else
                {
                    const char* matcher = GetHttpMethodsText(m_uMethod);
                    if (ch == ' ' && matcher[m_uIndex] == '\0')  // 'GET '
                        m_uState = STATE_REQ_SPACES_BEFORE_URL;
                    else if (ch == matcher[m_uIndex])  // 'GET'
                        /* Matched */;
                    else if ((ch >= 'A' && ch <= 'Z') || ch == '-')
                    {
                        switch (static_cast<unsigned>(m_uMethod) << 16 | m_uIndex << 8 | ch)
                        {
                            case static_cast<unsigned>(HttpMethods::Post) << 16 | 1 << 8 | 'U':
                                m_uMethod = HttpMethods::Put;
                                break;
                            case static_cast<unsigned>(HttpMethods::Post) << 16 | 1 << 8 | 'A':
                                m_uMethod = HttpMethods::Patch;
                                break;
                            case static_cast<unsigned>(HttpMethods::Post) << 16 | 1 << 8 | 'R':
                                m_uMethod = HttpMethods::PropFind;
                                break;
                            case static_cast<unsigned>(HttpMethods::Put) << 16 | 2 << 8 | 'R':
                                m_uMethod = HttpMethods::Purge;
                                break;
                            case static_cast<unsigned>(HttpMethods::Connect) << 16 | 1 << 8 | 'H':
                                m_uMethod = HttpMethods::Checkout;
                                break;
                            case static_cast<unsigned>(HttpMethods::Connect) << 16 | 2 << 8 | 'P':
                                m_uMethod = HttpMethods::Copy;
                                break;
                            case static_cast<unsigned>(HttpMethods::MkCol) << 16 | 1 << 8 | 'O':
                                m_uMethod = HttpMethods::Move;
                                break;
                            case static_cast<unsigned>(HttpMethods::MkCol) << 16 | 1 << 8 | 'E':
                                m_uMethod = HttpMethods::Merge;
                                break;
                            case static_cast<unsigned>(HttpMethods::MkCol) << 16 | 1 << 8 | '-':
                                m_uMethod = HttpMethods::MSearch;
                                break;
                            case static_cast<unsigned>(HttpMethods::MkCol) << 16 | 2 << 8 | 'A':
                                m_uMethod = HttpMethods::MkActivity;
                                break;
                            case static_cast<unsigned>(HttpMethods::MkCol) << 16 | 3 << 8 | 'A':
                                m_uMethod = HttpMethods::MkCalendar;
                                break;
                            case static_cast<unsigned>(HttpMethods::Subscribe) << 16 | 1 << 8 | 'E':
                                m_uMethod = HttpMethods::Search;
                                break;
                            case static_cast<unsigned>(HttpMethods::Subscribe) << 16 | 1 << 8 | 'O':
                                m_uMethod = HttpMethods::Source;
                                break;
                            case static_cast<unsigned>(HttpMethods::Report) << 16 | 2 << 8 | 'B':
                                m_uMethod = HttpMethods::Rebind;
                                break;
                            case static_cast<unsigned>(HttpMethods::PropFind) << 16 | 4 << 8 | 'P':
                                m_uMethod = HttpMethods::PropPatch;
                                break;
                            case static_cast<unsigned>(HttpMethods::Lock) << 16 | 1 << 8 | 'I':
                                m_uMethod = HttpMethods::Link;
                                break;
                            case static_cast<unsigned>(HttpMethods::Unlock) << 16 | 2 << 8 | 'S':
                                m_uMethod = HttpMethods::Unsubscribe;
                                break;
                            case static_cast<unsigned>(HttpMethods::Unlock) << 16 | 2 << 8 | 'B':
                                m_uMethod = HttpMethods::Unbind;
                                break;
                            case static_cast<unsigned>(HttpMethods::Unlock) << 16 | 3 << 8 | 'I':
                                m_uMethod = HttpMethods::Unlink;
                                break;
                            default:
                                MOE_THROW(BadFormatException, "Unexpected character {0}", ch);
                                break;
                        }
                    }
                    else
                        MOE_THROW(BadFormatException, "Unexpected character {0}", ch);
                    ++m_uIndex;
                }
                break;
            case STATE_REQ_SPACES_BEFORE_URL:
                if (ch == ' ')
                    break;
                urlMark = p;
                if (m_uMethod == HttpMethods::Connect)  // 'CONNECT '
                    m_uState = STATE_REQ_SERVER_START;
                ParseUrl(ch);
                break;
            case STATE_REQ_SCHEMA:
            case STATE_REQ_SCHEMA_SLASH:
            case STATE_REQ_SCHEMA_SLASH_SLASH:
            case STATE_REQ_SERVER_START:
                switch (ch)
                {
                    case ' ':
                    case '\r':
                    case '\n':
                        MOE_THROW(BadFormatException, "Unexpected character {0}", ch);
                    default:
                        ParseUrl(ch);
                        break;
                }
                break;
            case STATE_REQ_SERVER:
            case STATE_REQ_SERVER_WITH_AT:
            case STATE_REQ_PATH:
            case STATE_REQ_QUERY_STRING_START:
            case STATE_REQ_QUERY_STRING:
            case STATE_REQ_FRAGMENT_START:
            case STATE_REQ_FRAGMENT:
                switch (ch)
                {
                    case ' ':  // 'GET / '
                        m_uState = STATE_REQ_HTTP_START;

                        if (urlMark)
                        {
                            OnUrl(BytesView(urlMark, p - urlMark));
                            urlMark = nullptr;
                        }
                        break;
                    case '\r':
                    case '\n':
                        m_uHttpMajor = 0;
                        m_uHttpMinor = 9;
                        m_uState = (ch == '\r' ? STATE_REQ_LINE_ALMOST_DONE : STATE_HEADER_FIELD_START);

                        if (urlMark)
                        {
                            OnUrl(BytesView(urlMark, p - urlMark));
                            urlMark = nullptr;
                        }
                        break;
                    default:
                        ParseUrl(ch);
                        break;
                }
                break;
            case STATE_REQ_HTTP_START:
                switch (ch)
                {
                    case ' ':
                        break;
                    default:
                        Accept(ch, 'H');  // 'GET / H'
                        m_uState = STATE_REQ_HTTP_H;
                        break;
                }
                break;
            case STATE_REQ_HTTP_H:
                Accept(ch, 'T');  // 'GET / HT'
                m_uState = STATE_REQ_HTTP_HT;
                break;
            case STATE_REQ_HTTP_HT:
                Accept(ch, 'T');  // 'GET / HTT'
                m_uState = STATE_REQ_HTTP_HTT;
                break;
            case STATE_REQ_HTTP_HTT:
                Accept(ch, 'P');  // 'GET / HTTP'
                m_uState = STATE_REQ_HTTP_HTTP;
                break;
            case STATE_REQ_HTTP_HTTP:
                Accept(ch, '/');  // 'GET / HTTP/'
                m_uState = STATE_REQ_HTTP_MAJOR;
                break;
            case STATE_REQ_HTTP_MAJOR:
                m_uHttpMajor = AcceptDigit(ch);  // 'GET / HTTP/1'
                m_uState = STATE_REQ_HTTP_DOT;
                break;
            case STATE_REQ_HTTP_DOT:
                Accept(ch, '.');  // 'GET / HTTP/1.'
                m_uState = STATE_REQ_HTTP_MINOR;
                break;
            case STATE_REQ_HTTP_MINOR:
                m_uHttpMinor = AcceptDigit(ch);  // 'GET / HTTP/1.1'
                m_uState = STATE_REQ_HTTP_END;
                break;
            case STATE_REQ_HTTP_END:
                if (ch == '\r')
                {
                    m_uState = STATE_REQ_LINE_ALMOST_DONE;
                    break;
                }
                else if (ch == '\n')
                {
                    m_uState = STATE_HEADER_FIELD_START;
                    break;
                }
                MOE_THROW(BadFormatException, "Unexpected character {0}", ch);
                break;
            case STATE_REQ_LINE_ALMOST_DONE:
                Accept(ch, '\n');
                m_uState = STATE_HEADER_FIELD_START;
                break;
//////////////////////////////////////// </editor-fold>
//////////////////////////////////////// <editor-fold desc="HTTP_HEADER">
            case STATE_HEADER_FIELD_START:
                if (ch == '\r')
                {
                    m_uState = STATE_HEADERS_ALMOST_DONE;
                    break;
                }
                else if (ch == '\n')
                {
                    // 对于不规范的请求，可能会发送'\n'而不是'\r\n'
                    m_uState = STATE_HEADERS_ALMOST_DONE;
                    continue;
                }

                if ((c = IsHttpToken(ch, m_bStrictToken)) == '\0')
                    MOE_THROW(BadFormatException, "Unexpected character {0}", ch);

                headerFieldMark = p;
                m_uIndex = 0;
                m_uState = STATE_HEADER_FIELD;
                switch (c)
                {
                    case 'c':
                        m_uHeaderState = HEADER_STATE_C;
                        break;
                    case 'p':
                        m_uHeaderState = HEADER_STATE_MATCHING_PROXY_CONNECTION;
                        break;
                    case 't':
                        m_uHeaderState = HEADER_STATE_MATCHING_TRANSFER_ENCODING;
                        break;
                    case 'u':
                        m_uHeaderState = HEADER_STATE_MATCHING_UPGRADE;
                        break;
                    default:
                        m_uHeaderState = HEADER_STATE_GENERAL;
                        break;
                }
                break;
            case STATE_HEADER_FIELD:
                {
                    auto start = p;
                    for (; p != end; ++p)
                    {
                        ch = *p;
                        c = IsHttpToken(ch, m_bStrictToken);

                        if (!c)
                            break;

                        switch (m_uHeaderState)
                        {
                            case HEADER_STATE_GENERAL:
                                {
                                    size_t limit = end - p;
                                    limit = min(limit, m_uMaxHeaderSize);
                                    while (p + 1 < input.GetBuffer() + limit && IsHttpToken(p[1], m_bStrictToken))
                                        ++p;
                                }
                                break;
                            case HEADER_STATE_C:
                                ++m_uIndex;
                                m_uHeaderState = (c == 'o' ? HEADER_STATE_CO : HEADER_STATE_GENERAL);
                                break;
                            case HEADER_STATE_CO:
                                ++m_uIndex;
                                m_uHeaderState = (c == 'n' ? HEADER_STATE_CON : HEADER_STATE_GENERAL);
                                break;
                            case HEADER_STATE_CON:
                                ++m_uIndex;
                                switch (c)
                                {
                                    case 'n':
                                        m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION;
                                        break;
                                    case 't':
                                        m_uHeaderState = HEADER_STATE_MATCHING_CONTENT_LENGTH;
                                        break;
                                    default:
                                        m_uHeaderState = HEADER_STATE_GENERAL;
                                        break;
                                }
                                break;
                            case HEADER_STATE_MATCHING_CONNECTION:
                                ++m_uIndex;
                                if (m_uIndex > strlen(kConnection) || c != kConnection[m_uIndex])
                                    m_uHeaderState = HEADER_STATE_GENERAL;
                                else if (m_uIndex + 1 == strlen(kConnection))
                                    m_uHeaderState = HEADER_STATE_CONNECTION;
                                break;
                            case HEADER_STATE_MATCHING_PROXY_CONNECTION:
                                ++m_uIndex;
                                if (m_uIndex > strlen(kProxyConnection) || c != kProxyConnection[m_uIndex])
                                    m_uHeaderState = HEADER_STATE_GENERAL;
                                else if (m_uIndex + 1 == strlen(kProxyConnection))
                                    m_uHeaderState = HEADER_STATE_CONNECTION;
                                break;
                            case HEADER_STATE_MATCHING_CONTENT_LENGTH:
                                ++m_uIndex;
                                if (m_uIndex > strlen(kContentLength) || c != kContentLength[m_uIndex])
                                    m_uHeaderState = HEADER_STATE_GENERAL;
                                else if (m_uIndex + 1 == sizeof(kContentLength))
                                    m_uHeaderState = HEADER_STATE_CONTENT_LENGTH;
                                break;
                            case HEADER_STATE_MATCHING_TRANSFER_ENCODING:
                                ++m_uIndex;
                                if (m_uIndex > strlen(kTransferEncoding) ||
                                    c != kTransferEncoding[m_uIndex])
                                {
                                    m_uHeaderState = HEADER_STATE_GENERAL;
                                }
                                else if (m_uIndex + 1 == strlen(kTransferEncoding))
                                    m_uHeaderState = HEADER_STATE_TRANSFER_ENCODING;
                                break;
                            case HEADER_STATE_MATCHING_UPGRADE:
                                ++m_uIndex;
                                if (m_uIndex > strlen(kUpgrade) || c != kUpgrade[m_uIndex])
                                    m_uHeaderState = HEADER_STATE_GENERAL;
                                else if (m_uIndex + 1 == strlen(kUpgrade))
                                    m_uHeaderState = HEADER_STATE_UPGRADE;
                                break;
                            case HEADER_STATE_CONNECTION:
                            case HEADER_STATE_CONTENT_LENGTH:
                            case HEADER_STATE_TRANSFER_ENCODING:
                            case HEADER_STATE_UPGRADE:
                                if (ch != ' ')
                                    m_uHeaderState = HEADER_STATE_GENERAL;
                                break;
                            default:
                                assert(false);
                                break;
                        }
                    }

                    if (p == end)
                    {
                        --p;
                        CheckSize(m_uRead, p - start, m_uMaxHeaderSize);
                        break;
                    }

                    CheckSize(m_uRead, p - start, m_uMaxHeaderSize);

                    if (ch == ':')
                    {
                        m_uState = STATE_HEADER_VALUE_DISCARD_WS;

                        if (headerFieldMark)
                        {
                            OnHeaderField(BytesView(headerFieldMark, p - headerFieldMark));
                            headerFieldMark = nullptr;
                        }
                        break;
                    }
                    MOE_THROW(BadFormatException, "Unexpected character {0}", ch);
                }
                break;
            case STATE_HEADER_VALUE_DISCARD_WS:
                if (ch == ' ' || ch == '\t')
                    break;
                else if (ch == '\r')
                {
                    m_uState = STATE_HEADER_VALUE_DISCARD_WS_ALMOST_DONE;
                    break;
                }
                else if (ch == '\n')
                {
                    m_uState = STATE_HEADER_VALUE_DISCARD_LWS;
                    break;
                }
            case STATE_HEADER_VALUE_START:
                headerValueMark = p;
                m_uState = STATE_HEADER_VALUE;
                m_uIndex = 0;

                c = StringUtils::ToLower(ch);

                switch (m_uHeaderState)
                {
                    case HEADER_STATE_UPGRADE:
                        m_uFlags |= FLAGS_UPGRADE;
                        m_uHeaderState = HEADER_STATE_GENERAL;
                        break;
                    case HEADER_STATE_TRANSFER_ENCODING:  // 寻找'Transfer-Encoding: chunked'
                        if ('c' == c)
                            m_uHeaderState = HEADER_STATE_MATCHING_TRANSFER_ENCODING_CHUNKED;
                        else
                            m_uHeaderState = HEADER_STATE_GENERAL;
                        break;
                    case HEADER_STATE_CONTENT_LENGTH:
                        if (m_uFlags & FLAGS_CONTENTLENGTH)
                            MOE_THROW(BadFormatException, "Unexpected Content-Length");
                        m_uFlags |= FLAGS_CONTENTLENGTH;
                        m_uContentLength = AcceptDigit(ch);
                        m_uHeaderState = HEADER_STATE_CONTENT_LENGTH_NUM;
                        break;
                    case HEADER_STATE_CONNECTION:  // 寻找'Connection: keep-alive'
                        if (c == 'k')
                            m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_KEEP_ALIVE;
                        else if (c == 'c')  // 寻找'Connection: close'
                            m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_CLOSE;
                        else if (c == 'u')
                            m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_UPGRADE;
                        else
                            m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_TOKEN;
                        break;
                    case HEADER_STATE_MATCHING_CONNECTION_TOKEN_START:  // Multi-value `Connection` header
                        break;
                    default:
                        m_uHeaderState = HEADER_STATE_GENERAL;
                        break;
                }
                break;
            case STATE_HEADER_VALUE:
                {
                    auto start = p;
                    for (; p != end; ++p)
                    {
                        ch = *p;
                        if (ch == '\r')
                        {
                            m_uState = STATE_HEADER_ALMOST_DONE;

                            if (headerValueMark)
                            {
                                OnHeaderValue(BytesView(headerValueMark, p - headerValueMark));
                                headerValueMark = nullptr;
                            }
                            break;
                        }
                        else if (ch == '\n')
                        {
                            m_uState = STATE_HEADER_ALMOST_DONE;
                            CheckSize(m_uRead, p - start, m_uMaxHeaderSize);

                            if (headerValueMark)
                            {
                                OnHeaderValue(BytesView(headerValueMark, p - headerValueMark));
                                headerValueMark = nullptr;
                            }
                            continue;
                        }

                        if (!m_bLenientHeaders && !IsHttpHeaderChar(ch))
                            MOE_THROW(BadFormatException, "Unexpected character {0}", ch);

                        c = StringUtils::ToLower(ch);

                        switch (m_uHeaderState)
                        {
                            case HEADER_STATE_GENERAL:
                                {
                                    size_t limit = end - p;
                                    limit = min(limit, m_uMaxHeaderSize);
                                    auto cr = static_cast<const uint8_t*>(memchr(p, '\r', limit));
                                    auto lf = static_cast<const uint8_t*>(memchr(p, '\n', limit));
                                    if (cr)
                                    {
                                        if (lf && cr >= lf)
                                            p = lf;
                                        else
                                            p = cr;
                                    }
                                    else if (lf)
                                        p = lf;
                                    else
                                        p = end;
                                    --p;
                                }
                                break;
                            case HEADER_STATE_CONNECTION:
                            case HEADER_STATE_TRANSFER_ENCODING:
                                assert(false);
                                break;
                            case HEADER_STATE_CONTENT_LENGTH:
                                if (ch == ' ')
                                    break;
                                m_uHeaderState = HEADER_STATE_CONTENT_LENGTH_NUM;
                            case HEADER_STATE_CONTENT_LENGTH_NUM:
                                {
                                    uint64_t t = 0;
                                    if (ch == ' ')
                                    {
                                        m_uHeaderState = HEADER_STATE_CONTENT_LENGTH_WS;
                                        break;
                                    }
                                    t = m_uContentLength;
                                    t *= 10;
                                    t += AcceptDigit(ch);

                                    // 简单检查下是否溢出了
                                    if ((numeric_limits<uint64_t>::max() - 10) / 10 < m_uContentLength)
                                        MOE_THROW(BadFormatException, "Invalid Content-Length value");
                                    m_uContentLength = t;
                                }
                                break;
                            case HEADER_STATE_CONTENT_LENGTH_WS:
                                Accept(ch, ' ');
                                break;
                            case HEADER_STATE_MATCHING_TRANSFER_ENCODING_CHUNKED:  // Transfer-Encoding: chunked
                                ++m_uIndex;
                                if (m_uIndex > strlen(kChunked) || c != kChunked[m_uIndex])
                                    m_uHeaderState = HEADER_STATE_GENERAL;
                                else if (m_uIndex == strlen(kChunked) - 1)
                                    m_uHeaderState = HEADER_STATE_TRANSFER_ENCODING_CHUNKED;
                                break;
                            case HEADER_STATE_MATCHING_CONNECTION_TOKEN_START:
                                if (c == 'k')  // Connection: keep-alive
                                    m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_KEEP_ALIVE;
                                else if (c == 'c')   // Connection: close
                                    m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_CLOSE;
                                else if (c == 'u')
                                    m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_UPGRADE;
                                else if (IsHttpToken(ch, true))
                                    m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_TOKEN;
                                else if (c == ' ' || c == '\t')
                                    /* Skip it */ ;
                                else
                                    m_uHeaderState = HEADER_STATE_GENERAL;
                                break;
                            case HEADER_STATE_MATCHING_CONNECTION_KEEP_ALIVE:  // Connection: keep-alive
                                ++m_uIndex;
                                if (m_uIndex > strlen(kKeepAlive) || c != kKeepAlive[m_uIndex])
                                    m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_TOKEN;
                                else if (m_uIndex == strlen(kKeepAlive) - 1)
                                    m_uHeaderState = HEADER_STATE_CONNECTION_KEEP_ALIVE;
                                break;
                            case HEADER_STATE_MATCHING_CONNECTION_CLOSE:  // Connection: close
                                ++m_uIndex;
                                if (m_uIndex > strlen(kClose) || c != kClose[m_uIndex])
                                    m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_TOKEN;
                                else if (m_uIndex == strlen(kClose) - 1)
                                    m_uHeaderState = HEADER_STATE_CONNECTION_CLOSE;
                                break;
                            case HEADER_STATE_MATCHING_CONNECTION_UPGRADE:  // Connection: upgrade
                                ++m_uIndex;
                                if (m_uIndex > strlen(kUpgrade) || c != kUpgrade[m_uIndex])
                                    m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_TOKEN;
                                else if (m_uIndex == strlen(kUpgrade) - 1)
                                    m_uHeaderState = HEADER_STATE_CONNECTION_UPGRADE;
                                break;
                            case HEADER_STATE_MATCHING_CONNECTION_TOKEN:
                                if (ch == ',')
                                {
                                    m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_TOKEN_START;
                                    m_uIndex = 0;
                                }
                                break;
                            case HEADER_STATE_TRANSFER_ENCODING_CHUNKED:
                                if (ch != ' ')
                                    m_uHeaderState = HEADER_STATE_GENERAL;
                                break;
                            case HEADER_STATE_CONNECTION_KEEP_ALIVE:
                            case HEADER_STATE_CONNECTION_CLOSE:
                            case HEADER_STATE_CONNECTION_UPGRADE:
                                if (ch == ',')
                                {
                                    if (m_uHeaderState == HEADER_STATE_CONNECTION_KEEP_ALIVE)
                                        m_uFlags |= FLAGS_CONNECTION_KEEP_ALIVE;
                                    else if (m_uHeaderState == HEADER_STATE_CONNECTION_CLOSE)
                                        m_uFlags |= FLAGS_CONNECTION_CLOSE;
                                    else if (m_uHeaderState == HEADER_STATE_CONNECTION_UPGRADE)
                                        m_uFlags |= FLAGS_CONNECTION_UPGRADE;
                                    m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_TOKEN_START;
                                    m_uIndex = 0;
                                }
                                else if (ch != ' ')
                                    m_uHeaderState = HEADER_STATE_MATCHING_CONNECTION_TOKEN;
                                break;
                            default:
                                m_uState = STATE_HEADER_VALUE;
                                m_uHeaderState = HEADER_STATE_GENERAL;
                                break;
                        }
                    }

                    if (p == end)
                        --p;

                    CheckSize(m_uRead, p - start, m_uMaxHeaderSize);
                }
                break;
            case STATE_HEADER_ALMOST_DONE:
                Accept(ch, '\n');
                m_uState = STATE_HEADER_VALUE_LWS;
                break;
            case STATE_HEADER_VALUE_LWS:
                if (ch == ' ' || ch == '\t')
                {
                    m_uState = STATE_HEADER_VALUE_START;
                    continue;
                }

                switch (m_uHeaderState)
                {
                    case HEADER_STATE_CONNECTION_KEEP_ALIVE:
                        m_uFlags |= FLAGS_CONNECTION_KEEP_ALIVE;
                        break;
                    case HEADER_STATE_CONNECTION_CLOSE:
                        m_uFlags |= FLAGS_CONNECTION_CLOSE;
                        break;
                    case HEADER_STATE_TRANSFER_ENCODING_CHUNKED:
                        m_uFlags |= FLAGS_CHUNKED;
                        break;
                    case HEADER_STATE_CONNECTION_UPGRADE:
                        m_uFlags |= FLAGS_CONNECTION_UPGRADE;
                        break;
                    default:
                        break;
                }

                m_uState = STATE_HEADER_FIELD_START;
                continue;
            case STATE_HEADER_VALUE_DISCARD_WS_ALMOST_DONE:
                Accept(ch, '\n');
                m_uState = STATE_HEADER_VALUE_DISCARD_LWS;
                break;
            case STATE_HEADER_VALUE_DISCARD_LWS:
                if (ch == ' ' || ch == '\t')
                    m_uState = STATE_HEADER_VALUE_DISCARD_WS;
                else
                {
                    switch (m_uHeaderState)
                    {
                        case HEADER_STATE_CONNECTION_KEEP_ALIVE:
                            m_uFlags |= FLAGS_CONNECTION_KEEP_ALIVE;
                            break;
                        case HEADER_STATE_CONNECTION_CLOSE:
                            m_uFlags |= FLAGS_CONNECTION_CLOSE;
                            break;
                        case HEADER_STATE_TRANSFER_ENCODING_CHUNKED:
                            m_uFlags |= FLAGS_CHUNKED;
                            break;
                        case HEADER_STATE_CONNECTION_UPGRADE:
                            m_uFlags |= FLAGS_CONNECTION_UPGRADE;
                            break;
                        default:
                            break;
                    }

                    // HeaderValue为空
                    headerValueMark = p;
                    m_uState = STATE_HEADER_FIELD_START;

                    if (headerValueMark)
                    {
                        OnHeaderValue(BytesView(headerValueMark, p - headerValueMark));
                        headerValueMark = nullptr;
                    }
                    continue;
                }
                break;
            case STATE_HEADERS_ALMOST_DONE:
                Accept(ch, '\n');
                if (m_uFlags & FLAGS_TRAILING)
                {
                    // Chunked Encoding 请求尾
                    m_uState = STATE_MESSAGE_DONE;
                    OnChunkComplete();
                    continue;
                }
                // Content-Length和ChunkedEncoding是冲突的
                if ((m_uFlags & FLAGS_CHUNKED) && (m_uFlags & FLAGS_CONTENTLENGTH))
                    MOE_THROW(BadFormatException, "Unexpected content length");

                m_uState = STATE_HEADERS_DONE;

                // 对于响应，当且仅当存在"101 Switching Protocols"时Upgrade才有意义
                if ((m_uFlags & FLAGS_UPGRADE) && (m_uFlags & FLAGS_CONNECTION_UPGRADE))
                    m_bUpgrade = (m_uType == HttpParserTypes::Request || m_uStatusCode == 101);
                else
                    m_bUpgrade = (m_uMethod == HttpMethods::Connect);

                switch (OnHeadersComplete())
                {
                    case HeadersCompleteResult::Upgrade:
                        m_bUpgrade = true;
                    case HeadersCompleteResult::SkipBody:
                        m_uFlags |= FLAGS_SKIPBODY;
                        break;
                    case HeadersCompleteResult::Default:
                    default:
                        break;
                }
                continue;
            case STATE_HEADERS_DONE:
                Accept(ch, '\n');
                m_uRead = 0;

                {
                    auto hasBody = m_uFlags & FLAGS_CHUNKED || (m_uContentLength > 0 &&
                        m_uContentLength != numeric_limits<uint64_t>::max());
                    if (m_bUpgrade && (m_uMethod == HttpMethods::Connect || (m_uFlags & FLAGS_SKIPBODY) || !hasBody))
                    {
                        ResetNewMessageState();
                        OnMessageComplete();
                        return p - input.GetBuffer() + 1;
                    }
                    if (m_uFlags & FLAGS_SKIPBODY)
                    {
                        ResetNewMessageState();
                        OnMessageComplete();
                    }
                    else if (m_uFlags & FLAGS_CHUNKED)  // Chunked Encoding
                        m_uState = STATE_CHUNK_SIZE_START;
                    else
                    {
                        if (m_uContentLength == 0)  // 有ContentLength并且为0
                        {
                            ResetNewMessageState();
                            OnMessageComplete();
                        }
                        else if (m_uContentLength != numeric_limits<uint64_t>::max())  // 有Content-Length并且非0
                            m_uState = STATE_BODY_IDENTITY;
                        else
                        {
                            if (!IsEofRequired())  // 假定没有Content-Length了
                            {
                                ResetNewMessageState();
                                OnMessageComplete();
                            }
                            else
                                m_uState = STATE_BODY_IDENTITY_EOF;  // 直到EOF，全是Body
                        }
                    }
                }
                break;
//////////////////////////////////////// </editor-fold>
//////////////////////////////////////// <editor-fold desc="HTTP_BODY">
            case STATE_BODY_IDENTITY:
                {
                    assert(m_uContentLength != 0 && m_uContentLength != numeric_limits<uint64_t>::max());
                    uint64_t read = min<uint64_t>(m_uContentLength, end - p);

                    bodyMark = p;
                    m_uContentLength -= read;
                    p += read - 1;
                    if (m_uContentLength == 0)
                    {
                        m_uState = STATE_MESSAGE_DONE;

                        if (bodyMark)
                        {
                            OnBody(BytesView(bodyMark, p - bodyMark));
                            bodyMark = nullptr;
                        }
                        continue;
                    }
                }
                break;
            case STATE_BODY_IDENTITY_EOF:  // 读直到EOF
                bodyMark = p;
                p = end - 1;
                break;
            case STATE_MESSAGE_DONE:
                ResetNewMessageState();
                OnMessageComplete();
                if (m_bUpgrade)  // 协议已经变了
                    return p - input.GetBuffer() + 1;
                break;
            case STATE_CHUNK_SIZE_START:
                {
                    assert(m_uRead == 1);
                    assert(m_uFlags & FLAGS_CHUNKED);

                    unsigned hex = 0;
                    if (!StringUtils::HexDigitToNumber(hex, ch))
                        MOE_THROW(BadFormatException, "Unexpected character {0}", ch);

                    m_uContentLength = hex;
                    m_uState = STATE_CHUNK_SIZE;
                }
                break;
            case STATE_CHUNK_SIZE:
                {
                    assert(m_uFlags & FLAGS_CHUNKED);

                    if (ch == '\r')
                    {
                        m_uState = STATE_CHUNK_SIZE_ALMOST_DONE;
                        break;
                    }

                    unsigned hex = 0;
                    if (!StringUtils::HexDigitToNumber(hex, ch))
                    {
                        if (ch == ';' || ch == ' ')
                        {
                            m_uState = STATE_CHUNK_PARAMETERS;
                            break;
                        }

                        MOE_THROW(BadFormatException, "Unexpected character {0}", ch);
                    }

                    auto t = m_uContentLength;
                    t *= 16;
                    t += hex;

                    // 简单检查下是否溢出了
                    if ((numeric_limits<uint64_t>::max() - 16) / 16 < t)
                        MOE_THROW(BadFormatException, "Invalid Content-Length value");
                    m_uContentLength = t;
                }
                break;
            case STATE_CHUNK_PARAMETERS:
                assert(m_uFlags & FLAGS_CHUNKED);
                // 不处理，忽略这坨参数
                if (ch == '\r')
                    m_uState = STATE_CHUNK_SIZE_ALMOST_DONE;
                break;
            case STATE_CHUNK_SIZE_ALMOST_DONE:
                assert(m_uFlags & FLAGS_CHUNKED);
                Accept(ch, '\n');

                m_uRead = 0;
                if (m_uContentLength == 0)
                {
                    m_uFlags |= FLAGS_TRAILING;
                    m_uState = STATE_HEADER_FIELD_START;
                }
                else
                    m_uState = STATE_CHUNK_DATA;
                OnChunkHeader(m_uContentLength);
                break;
            case STATE_CHUNK_DATA:
                {
                    assert(m_uFlags & FLAGS_CHUNKED);
                    assert(m_uContentLength != 0 && m_uContentLength != numeric_limits<uint64_t>::max());

                    uint64_t read = min<uint64_t>(m_uContentLength, end - p);
                    bodyMark = p;
                    m_uContentLength -= read;
                    p += read - 1;

                    if (m_uContentLength == 0)
                        m_uState = STATE_CHUNK_DATA_ALMOST_DONE;
                }
                break;
            case STATE_CHUNK_DATA_ALMOST_DONE:
                assert(m_uFlags & FLAGS_CHUNKED);
                assert(m_uContentLength == 0);
                Accept(ch, '\r');
                m_uState = STATE_CHUNK_DATA_DONE;

                if (bodyMark)
                {
                    OnBody(BytesView(bodyMark, p - bodyMark));
                    bodyMark = nullptr;
                }
                break;
            case STATE_CHUNK_DATA_DONE:
                assert(m_uFlags & FLAGS_CHUNKED);
                Accept(ch, '\n');
                m_uRead = 0;
                m_uState = STATE_CHUNK_SIZE_START;
                OnChunkComplete();
                break;
//////////////////////////////////////// </editor-fold>
            default:
                assert(false);
                MOE_THROW(InvalidCallException, "Internal error, bad state {0}", m_uState);
        }

        // 读取下一个字符
        ++p;
        if (p != end)
        {
            ch = static_cast<char>(*p);
            if (IsParsingHeader(m_uState))
                CheckSize(m_uRead, 1, m_uMaxHeaderSize);
        }
    }

    // 在这里执行所有没有执行完的回调
    assert(((headerFieldMark ? 1 : 0) + (headerValueMark ? 1 : 0) + (urlMark ? 1 : 0) + (bodyMark ? 1 : 0) +
        (statusMark ? 1 : 0)) <= 1);
    if (headerFieldMark)
        OnHeaderField(BytesView(headerFieldMark, p - headerFieldMark));
    if (headerValueMark)
        OnHeaderValue(BytesView(headerValueMark, p - headerValueMark));
    if (urlMark)
        OnUrl(BytesView(urlMark, p - urlMark));
    if (bodyMark)
        OnBody(BytesView(bodyMark, p - bodyMark));
    if (statusMark)
        OnStatus(BytesView(statusMark, p - statusMark));
    return input.GetSize();
}

//////////////////////////////////////////////////////////////////////////////// HttpHeader

const std::string& HttpHeaders::operator[](const std::string& key)const noexcept
{
    auto it = m_stHeaders.find(key);
    if (it == m_stHeaders.end())
        return EmptyRefOf<string>();
    return it->second;
}

std::string& HttpHeaders::operator[](const std::string& key)noexcept
{
    auto it = m_stHeaders.find(key);
    if (it == m_stHeaders.end())
        it = m_stHeaders.emplace(key, string());
    return it->second;
}

size_t HttpHeaders::GetSize()const noexcept
{
    return m_stHeaders.size();
}

bool HttpHeaders::IsEmpty()const noexcept
{
    return m_stHeaders.empty();
}

void HttpHeaders::Add(const std::string& key, const std::string& value)
{
    m_stHeaders.insert(pair<string, string>(key, value));
}

void HttpHeaders::Add(std::string&& key, std::string value)
{
    m_stHeaders.emplace(key, value);
}

void HttpHeaders::Remove(const std::string& key)noexcept
{
    auto its = m_stHeaders.equal_range(key);
    if (its.first != its.second)
        m_stHeaders.erase(its.first, its.second);
}

bool HttpHeaders::Contains(const std::string& key)const noexcept
{
    auto it = m_stHeaders.find(key);
    return it != m_stHeaders.end();
}

size_t HttpHeaders::Count(const std::string& key)const noexcept
{
    return m_stHeaders.count(key);
}

void HttpHeaders::Clear()noexcept
{
    m_stHeaders.clear();
}

std::pair<HttpHeaders::ConstIteratorType, HttpHeaders::ConstIteratorType> HttpHeaders::Range(
    const std::string& key)const noexcept
{
    return m_stHeaders.equal_range(key);
}

std::pair<HttpHeaders::IteratorType, HttpHeaders::IteratorType> HttpHeaders::Range(const std::string& key)noexcept
{
    return m_stHeaders.equal_range(key);
}

HttpHeaders::ConstIteratorType HttpHeaders::First()const noexcept
{
    return m_stHeaders.begin();
}

HttpHeaders::IteratorType HttpHeaders::First()noexcept
{
    return m_stHeaders.begin();
}

HttpHeaders::ConstIteratorType HttpHeaders::Last()const noexcept
{
    return m_stHeaders.end();
}

HttpHeaders::IteratorType HttpHeaders::Last()noexcept
{
    return m_stHeaders.end();
}

void HttpHeaders::SerializeTo(std::string& out)const
{
    // 计算预分配大小
    size_t sz = 0;
    for (const auto& i : m_stHeaders)
    {
        sz += i.first.size();
        sz += 2;
        sz += i.second.size();
        sz += 2;
    }
    out.reserve(out.length() + sz);

    // 写出HTTP头
    for (const auto& i : m_stHeaders)
    {
        out.append(i.first);
        out.append(": ");
        out.append(i.second);
        out.append("\r\n");
    }
}

std::string HttpHeaders::ToString()const
{
    string ret;
    SerializeTo(ret);
    return ret;
}

//////////////////////////////////////////////////////////////////////////////// HttpProtocol

static const string kConnectionString = "Connection";
static const string kContentLengthString = "Content-Length";
static const string kUpgradeString = "upgrade";
static const string kCloseString = "close";
static const string kKeepAliveString = "keep-alive";
static const string kChunkedString = "chunked";

enum HttpParserStates
{
    HTTP_STATE_INIT,  // 初始化状态
    HTTP_STATE_PARSING,  // 解析请求的状态
    HTTP_STATE_PARSING_URL,
    HTTP_STATE_PARSING_STATUS,
    HTTP_STATE_PARSING_HEADER_KEY,
    HTTP_STATE_PARSING_HEADER_VALUE,
    HTTP_STATE_PARSING_BODY,
    HTTP_STATE_COMPLETE,  // 解析完成
    HTTP_STATE_UPGRADED,  // 协议已升级
};

HttpProtocol::HttpProtocol(ProtocolType type, const HttpParserSettings& settings)noexcept
    : HttpParserBase(type == ProtocolType::Request ? HttpParserTypes::Request : HttpParserTypes::Response, settings),
    m_uType(type)
{
}

void HttpProtocol::Reset()noexcept
{
    HttpParserBase::Reset(HttpParserBase::GetType());

    m_uMethod = HttpMethods::Unknown;
    m_uHttpMajor = 0;
    m_uHttpMinor = 0;
    m_uStatusCode = HttpStatus::Ok;
    m_stUrl.clear();
    m_stHeaders.Clear();

    m_uState = HTTP_STATE_INIT;
    m_stKeyBuffer.clear();
    m_stBuffer.clear();
}

bool HttpProtocol::Parse(BytesView input, size_t* processed)
{
    if (m_uState == HTTP_STATE_UPGRADED)
    {
        if (processed)
            *processed = 0;
        return true;
    }
    else if (m_uState == HTTP_STATE_COMPLETE)
        Reset();

    size_t sz = HttpParserBase::Parse(input);
    if (processed)
        *processed = sz;
    return (m_uState == HTTP_STATE_COMPLETE || m_uState == HTTP_STATE_UPGRADED);
}

bool HttpProtocol::IsUpgraded()const noexcept
{
    if (m_stHeaders.Contains(kUpgradeString) && m_stHeaders.Contains(kConnectionString) &&
        StringUtils::CaseInsensitiveCompare(m_stHeaders[kConnectionString], kUpgradeString) == 0)
    {
        return m_uType == ProtocolType::Request || m_uStatusCode == HttpStatus::SwitchingProtocols;
    }
    else
        return m_uMethod == HttpMethods::Connect;
}

bool HttpProtocol::ShouldKeepAlive()const noexcept
{
    if (m_uHttpMajor > 0 && m_uHttpMinor > 0)
    {
        if (m_stHeaders.Contains(kConnectionString) &&
            StringUtils::CaseInsensitiveCompare(m_stHeaders[kConnectionString], kCloseString) == 0)
        {
            return false;
        }
    }
    else
    {
        if (!(m_stHeaders.Contains(kConnectionString) &&
            StringUtils::CaseInsensitiveCompare(m_stHeaders[kConnectionString], kKeepAliveString) == 0))
        {
            return false;
        }
    }

    if (m_uType == ProtocolType::Request)
        return false;

    if (static_cast<int>(m_uStatusCode) / 100 == 1 || m_uStatusCode == HttpStatus::NoContent ||
        m_uStatusCode == HttpStatus::NotModified)
    {
        return false;
    }

    if ((m_stHeaders.Contains(kConnectionString) &&
        StringUtils::CaseInsensitiveCompare(m_stHeaders[kConnectionString], kChunkedString) == 0) ||
        m_stHeaders.Contains(kContentLengthString))
    {
        return false;
    }
    return true;
}

void HttpProtocol::SerializeTo(std::string& out)const
{
    char buf[64] = { 0 };

    if (GetType() == ProtocolType::Request)
    {
        out.reserve(out.length() + 24 + m_stUrl.length());
        out.append(GetHttpMethodsText(m_uMethod));
        out.push_back(' ');
        out.append(m_stUrl);
        out.push_back(' ');
        out.append("HTTP/");
        Convert::ToDecimalString(m_uHttpMajor, buf);
        out.append(buf);
        out.push_back('.');
        Convert::ToDecimalString(m_uHttpMinor, buf);
        out.append(buf);
        out.append("\r\n");
        m_stHeaders.SerializeTo(out);
        out.append("\r\n");
    }
    else
    {
        out.reserve(out.length() + 24 + m_stUrl.length());
        out.append("HTTP/");
        Convert::ToDecimalString(m_uHttpMajor, buf);
        out.append(buf);
        out.push_back('.');
        Convert::ToDecimalString(m_uHttpMinor, buf);
        out.append(buf);
        out.push_back(' ');
        Convert::ToDecimalString(static_cast<unsigned>(m_uStatusCode), buf);
        out.append(buf);
        out.push_back(' ');
        out.append(GetHttpStatusText(m_uStatusCode));
        out.append("\r\n");
        m_stHeaders.SerializeTo(out);
        out.append("\r\n");
    }
}

std::string HttpProtocol::ToString()const
{
    string ret;
    SerializeTo(ret);
    return ret;
}

void HttpProtocol::OnMessageBegin()
{
    if (m_uState == HTTP_STATE_COMPLETE)
        MOE_THROW(BadFormatException, "Multiple HTTP request/response has been found");
    m_uState = HTTP_STATE_PARSING;
}

void HttpProtocol::OnUrl(BytesView data)
{
    if (m_uState != HTTP_STATE_PARSING_URL)
    {
        assert(m_uState == HTTP_STATE_PARSING);
        m_uState = HTTP_STATE_PARSING_URL;
        m_stBuffer.clear();
    }

    m_stBuffer.append(reinterpret_cast<const char*>(data.GetBuffer()), data.GetSize());
}

void HttpProtocol::OnStatus(BytesView data)
{
    if (m_uState != HTTP_STATE_PARSING_STATUS)
    {
        assert(m_uState == HTTP_STATE_PARSING);
        m_uState = HTTP_STATE_PARSING_STATUS;
    }

    MOE_UNUSED(data);  // Do nothing
}

void HttpProtocol::OnHeaderField(BytesView data)
{
    if (m_uState != HTTP_STATE_PARSING_HEADER_KEY)
    {
        if (m_uState == HTTP_STATE_PARSING_URL)
        {
            m_uMethod = HttpParserBase::GetMethod();
            m_uHttpMajor = HttpParserBase::GetMajorVersion();
            m_uHttpMinor = HttpParserBase::GetMinorVersion();
            m_stUrl = std::move(m_stBuffer);
        }
        else if (m_uState == HTTP_STATE_PARSING_STATUS)
        {
            m_uHttpMajor = HttpParserBase::GetMajorVersion();
            m_uHttpMinor = HttpParserBase::GetMinorVersion();
            m_uStatusCode = static_cast<HttpStatus>(HttpParserBase::GetStatusCode());
        }
        else if (m_uState == HTTP_STATE_PARSING_HEADER_VALUE)
            m_stHeaders.Add(std::move(m_stKeyBuffer), std::move(m_stBuffer));

        m_uState = HTTP_STATE_PARSING_HEADER_KEY;
        m_stKeyBuffer.clear();
        m_stBuffer.clear();
    }

    m_stKeyBuffer.append(reinterpret_cast<const char*>(data.GetBuffer()), data.GetSize());
}

void HttpProtocol::OnHeaderValue(BytesView data)
{
    if (m_uState != HTTP_STATE_PARSING_HEADER_VALUE)
    {
        assert(m_uState == HTTP_STATE_PARSING_HEADER_KEY);
        m_uState = HTTP_STATE_PARSING_HEADER_VALUE;
    }

    m_stBuffer.append(reinterpret_cast<const char*>(data.GetBuffer()), data.GetSize());
}

HttpParserBase::HeadersCompleteResult HttpProtocol::OnHeadersComplete()
{
    if (m_uState != HTTP_STATE_PARSING_BODY)
    {
        if (m_uState == HTTP_STATE_PARSING_URL)
        {
            m_uMethod = HttpParserBase::GetMethod();
            m_uHttpMajor = HttpParserBase::GetMajorVersion();
            m_uHttpMinor = HttpParserBase::GetMinorVersion();
            m_stUrl = std::move(m_stBuffer);
        }
        else if (m_uState == HTTP_STATE_PARSING_STATUS)
        {
            m_uHttpMajor = HttpParserBase::GetMajorVersion();
            m_uHttpMinor = HttpParserBase::GetMinorVersion();
            m_uStatusCode = static_cast<HttpStatus>(HttpParserBase::GetStatusCode());
        }
        else if (m_uState == HTTP_STATE_PARSING_HEADER_VALUE)
            m_stHeaders.Add(std::move(m_stKeyBuffer), std::move(m_stBuffer));

        m_uState = HTTP_STATE_PARSING_BODY;
    }

    if (m_pHeadersCompleteCallback)
        return m_pHeadersCompleteCallback();
    return HeadersCompleteResult::Default;
}

void HttpProtocol::OnBody(BytesView data)
{
    if (m_uState != HTTP_STATE_PARSING_BODY)
    {
        if (m_uState == HTTP_STATE_PARSING_HEADER_VALUE)
            m_stHeaders.Add(std::move(m_stKeyBuffer), std::move(m_stBuffer));
        m_uState = HTTP_STATE_PARSING_BODY;
    }

    if (m_pBodyDataCallback)
        m_pBodyDataCallback(data);
}

void HttpProtocol::OnMessageComplete()
{
    m_uState = HTTP_STATE_COMPLETE;
}

void HttpProtocol::OnChunkHeader(size_t length)
{
    MOE_UNUSED(length);
}

void HttpProtocol::OnChunkComplete()
{
}

//////////////////////////////////////////////////////////////////////////////// WebSocketProtocol

enum WebSocketParserStates
{
    WS_STATE_HEADER_0 = 0,
    WS_STATE_HEADER_1,
    WS_STATE_HEADER_2,
    WS_STATE_HEADER_3,
    WS_STATE_HEADER_4,
    WS_STATE_HEADER_5,
    WS_STATE_HEADER_6,
    WS_STATE_HEADER_7,
    WS_STATE_HEADER_8,
    WS_STATE_HEADER_9,
    WS_STATE_HEADER_10,
    WS_STATE_HEADER_11,
    WS_STATE_HEADER_12,
    WS_STATE_HEADER_13,
    WS_STATE_BODY,
};

WebSocketProtocol::WebSocketProtocol()noexcept
{
    Reset();
}

void WebSocketProtocol::Reset()noexcept
{
    m_bFin = false;
    m_stReserves.fill(false);
    m_bOpCode = 0;
    m_bMask = false;
    m_uPayloadLength = 0;
    m_stMaskKey.fill(0);

    m_uState = WS_STATE_HEADER_0;
    m_bPayload16 = false;
    m_bPayload64 = false;
    m_uBodyRead = 0;
}

void WebSocketProtocol::ParseImpl(BytesView input)
{
    auto p = input.GetBuffer();
    auto end = p + input.GetSize();
    while (p < end)
    {
        auto b = *p;
        switch (m_uState)
        {
            case WS_STATE_HEADER_0:
                m_bFin = ((b >> 7) & 0x01) != 0;
                m_stReserves[0] = ((b >> 6) & 0x01) != 0;
                m_stReserves[1] = ((b >> 5) & 0x01) != 0;
                m_stReserves[2] = ((b >> 4) & 0x01) != 0;
                m_bOpCode = static_cast<uint8_t>(b & 0x0F);
                m_bMask = false;
                m_uPayloadLength = 0;
                m_bPayload16 = false;
                m_bPayload64 = false;
                m_stMaskKey[0] = m_stMaskKey[1] = m_stMaskKey[2] = m_stMaskKey[3] = 0;
                m_uState = WS_STATE_HEADER_1;
                break;
            case WS_STATE_HEADER_1:
                m_bMask = ((b >> 7) & 0x01) != 0;
                m_uPayloadLength = b & 0x7Fu;
                if (m_uPayloadLength == 127)
                {
                    m_bPayload64 = true;
                    m_uPayloadLength = 0;
                }
                else if (m_uPayloadLength == 126)
                {
                    m_bPayload16 = true;
                    m_uPayloadLength = 0;
                }
                else if (!m_bMask)
                {
                    if (m_uPayloadLength > 0)
                    {
                        if (m_pHeadersCompleteCallback)
                            m_pHeadersCompleteCallback();
                        m_uBodyRead = 0;
                        m_uState = WS_STATE_BODY;
                    }
                    else
                    {
                        if (m_pHeadersCompleteCallback)
                            m_pHeadersCompleteCallback();
                        if (m_pMessageCompleteCallback)
                            m_pMessageCompleteCallback();
                        m_uState = WS_STATE_HEADER_0;
                    }
                    break;
                }
                m_uState = WS_STATE_HEADER_2;
                break;
            case WS_STATE_HEADER_2:
                if (m_bPayload16)
                    m_uPayloadLength += (b << 8);
                else if (m_bPayload64)
                    m_uPayloadLength += (static_cast<uint64_t>(b) << 56ull);
                else
                    m_stMaskKey[0] = b;
                m_uState = WS_STATE_HEADER_3;
                break;
            case WS_STATE_HEADER_3:
                if (m_bPayload16)
                {
                    m_uPayloadLength += b;
                    if (!m_bMask)
                    {
                        if (m_uPayloadLength > 0)
                        {
                            if (m_pHeadersCompleteCallback)
                                m_pHeadersCompleteCallback();
                            m_uBodyRead = 0;
                            m_uState = WS_STATE_BODY;
                        }
                        else
                        {
                            if (m_pHeadersCompleteCallback)
                                m_pHeadersCompleteCallback();
                            if (m_pMessageCompleteCallback)
                                m_pMessageCompleteCallback();
                            m_uState = WS_STATE_HEADER_0;
                        }
                        break;
                    }
                }
                else if (m_bPayload64)
                    m_uPayloadLength += (static_cast<uint64_t>(b) << 48ull);
                else
                    m_stMaskKey[1] = b;
                m_uState = WS_STATE_HEADER_4;
                break;
            case WS_STATE_HEADER_4:
                if (m_bPayload16)
                    m_stMaskKey[0] = b;
                else if (m_bPayload64)
                    m_uPayloadLength += (static_cast<uint64_t>(b) << 40ull);
                else
                    m_stMaskKey[2] = b;
                m_uState = WS_STATE_HEADER_5;
                break;
            case WS_STATE_HEADER_5:
                if (m_bPayload16)
                    m_stMaskKey[1] = b;
                else if (m_bPayload64)
                    m_uPayloadLength += (static_cast<uint64_t>(b) << 32ull);
                else
                {
                    m_stMaskKey[3] = b;
                    if (m_uPayloadLength > 0)
                    {
                        if (m_pHeadersCompleteCallback)
                            m_pHeadersCompleteCallback();
                        m_uBodyRead = 0;
                        m_uState = WS_STATE_BODY;
                    }
                    else
                    {
                        if (m_pHeadersCompleteCallback)
                            m_pHeadersCompleteCallback();
                        if (m_pMessageCompleteCallback)
                            m_pMessageCompleteCallback();
                        m_uState = WS_STATE_HEADER_0;
                    }
                    break;
                }
                m_uState = WS_STATE_HEADER_6;
                break;
            case WS_STATE_HEADER_6:
                if (m_bPayload16)
                    m_stMaskKey[2] = b;
                else
                {
                    assert(m_bPayload64);
                    m_uPayloadLength += (b << 24);
                }
                m_uState = WS_STATE_HEADER_7;
                break;
            case WS_STATE_HEADER_7:
                if (m_bPayload16)
                {
                    m_stMaskKey[3] = b;
                    if (m_uPayloadLength > 0)
                    {
                        if (m_pHeadersCompleteCallback)
                            m_pHeadersCompleteCallback();
                        m_uBodyRead = 0;
                        m_uState = WS_STATE_BODY;
                    }
                    else
                    {
                        if (m_pHeadersCompleteCallback)
                            m_pHeadersCompleteCallback();
                        if (m_pMessageCompleteCallback)
                            m_pMessageCompleteCallback();
                        m_uState = WS_STATE_HEADER_0;
                    }
                    break;
                }
                assert(m_bPayload64);
                m_uPayloadLength += (b << 16);
                m_uState = WS_STATE_HEADER_8;
                break;
            case WS_STATE_HEADER_8:
                assert(m_bPayload64);
                m_uPayloadLength += b << 8;
                m_uState = WS_STATE_HEADER_9;
                break;
            case WS_STATE_HEADER_9:
                assert(m_bPayload64);
                m_uPayloadLength += b;
                m_uState = WS_STATE_HEADER_10;
                break;
            case WS_STATE_HEADER_10:
                assert(m_bPayload64);
                m_stMaskKey[0] = b;
                m_uState = WS_STATE_HEADER_11;
                break;
            case WS_STATE_HEADER_11:
                assert(m_bPayload64);
                m_stMaskKey[1] = b;
                m_uState = WS_STATE_HEADER_12;
                break;
            case WS_STATE_HEADER_12:
                assert(m_bPayload64);
                m_stMaskKey[2] = b;
                m_uState = WS_STATE_HEADER_13;
                break;
            case WS_STATE_HEADER_13:
                assert(m_bPayload64);
                m_stMaskKey[3] = b;
                if (m_uPayloadLength > 0)
                {
                    if (m_pHeadersCompleteCallback)
                        m_pHeadersCompleteCallback();
                    m_uBodyRead = 0;
                    m_uState = WS_STATE_BODY;
                }
                else
                {
                    if (m_pHeadersCompleteCallback)
                        m_pHeadersCompleteCallback();
                    if (m_pMessageCompleteCallback)
                        m_pMessageCompleteCallback();
                    m_uState = WS_STATE_HEADER_0;
                }
                break;
            case WS_STATE_BODY:
                {
                    uint64_t read = m_uPayloadLength - m_uBodyRead;
                    if (read == 0)
                    {
                        m_uState = WS_STATE_HEADER_0;
                        break;
                    }

                    if (read <= static_cast<size_t>(end - p))
                    {
                        if (m_pDataCallback)
                            m_pDataCallback(BytesView(p, read));
                        if (m_pMessageCompleteCallback)
                            m_pMessageCompleteCallback();
                        m_uState = WS_STATE_HEADER_0;
                        p += read;
                    }
                    else
                    {
                        read = end - p;
                        if (m_pDataCallback)
                            m_pDataCallback(BytesView(p, read));
                        p += read;
                        m_uBodyRead += read;
                    }
                }
                continue;
            default:
                assert(false);
                break;
        }

        ++p;
    }
}

void WebSocketProtocol::SerializeTo(std::string& out)const
{
    int b = 0;
    out.reserve(out.size() + 16);

    // 第一个字节
    b = (static_cast<uint8_t>(m_bFin) << 7) | (static_cast<uint8_t>(m_stReserves[0]) << 6) |
        (static_cast<uint8_t>(m_stReserves[1]) << 5) | (static_cast<uint8_t>(m_stReserves[2]) << 4) |
        (m_bOpCode & 0x0F);
    out.push_back(static_cast<char>(b));

    // 第二个字节
    b = (static_cast<uint8_t>(m_bMask) << 7) | static_cast<uint8_t>(m_uPayloadLength <= 125 ? m_uPayloadLength :
        (m_uPayloadLength <= numeric_limits<uint16_t>::max() ? 126 : 127));
    out.push_back(static_cast<char>(b));

    if (m_uPayloadLength > 125)
    {
        if (m_uPayloadLength <= numeric_limits<uint16_t>::max())
        {
            // 第三\四个字节
            out.push_back(static_cast<char>((m_uPayloadLength & 0xFF00) >> 8));
            out.push_back(static_cast<char>(m_uPayloadLength & 0xFF));
        }
        else
        {
            // 第三~十个字节
            out.push_back(static_cast<char>((m_uPayloadLength & 0xFF00000000000000ull) >> 56ull));
            out.push_back(static_cast<char>((m_uPayloadLength & 0xFF000000000000ull) >> 48ull));
            out.push_back(static_cast<char>((m_uPayloadLength & 0xFF0000000000ull) >> 40ull));
            out.push_back(static_cast<char>((m_uPayloadLength & 0xFF00000000ull) >> 32ull));
            out.push_back(static_cast<char>((m_uPayloadLength & 0xFF000000ull) >> 24ull));
            out.push_back(static_cast<char>((m_uPayloadLength & 0xFF0000ull) >> 16ull));
            out.push_back(static_cast<char>((m_uPayloadLength & 0xFF00ull) >> 8ull));
            out.push_back(static_cast<char>(m_uPayloadLength & 0xFFull));
        }
    }

    if (m_bMask)
    {
        out.push_back(static_cast<char>(m_stMaskKey[0]));
        out.push_back(static_cast<char>(m_stMaskKey[1]));
        out.push_back(static_cast<char>(m_stMaskKey[2]));
        out.push_back(static_cast<char>(m_stMaskKey[3]));
    }
}

std::string WebSocketProtocol::ToString()const
{
    string ret;
    SerializeTo(ret);
    return ret;
}
