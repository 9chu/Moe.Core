/**
 * @file
 * @author chu
 * @date 2018/3/9
 */
#include <Moe.Core/Url.hpp>
#include <Moe.Core/Exception.hpp>
#include <Moe.Core/StringUtils.hpp>
#include <Moe.Core/Encoding.hpp>
#include <Moe.Core/Idna.hpp>

#include <cassert>
#include <cmath>

using namespace std;
using namespace moe;

static const char* kHexTable[256] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "%21", "%22", "%23", "%24", "%25", "%26", "%27",
    "%28", "%29", "%2A", "%2B", "%2C", "%2D", "%2E", "%2F",
    "%30", "%31", "%32", "%33", "%34", "%35", "%36", "%37",
    "%38", "%39", "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
    "%40", "%41", "%42", "%43", "%44", "%45", "%46", "%47",
    "%48", "%49", "%4A", "%4B", "%4C", "%4D", "%4E", "%4F",
    "%50", "%51", "%52", "%53", "%54", "%55", "%56", "%57",
    "%58", "%59", "%5A", "%5B", "%5C", "%5D", "%5E", "%5F",
    "%60", "%61", "%62", "%63", "%64", "%65", "%66", "%67",
    "%68", "%69", "%6A", "%6B", "%6C", "%6D", "%6E", "%6F",
    "%70", "%71", "%72", "%73", "%74", "%75", "%76", "%77",
    "%78", "%79", "%7A", "%7B", "%7C", "%7D", "%7E", "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF",
};

static const uint8_t kC0ControlEncodeSet[32] = {
    // 00     01      02      03      04      05      06      07
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 08     09      0A      0B      0C      0D      0E      0F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 10     11      12      13      14      15      16      17
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 18     19      1A      1B      1C      1D      1E      1F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 20     21      22      23      24      25      26      27
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 28     29      2A      2B      2C      2D      2E      2F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 30     31      32      33      34      35      36      37
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 38     39      3A      3B      3C      3D      3E      3F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 40     41      42      43      44      45      46      47
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 48     49      4A      4B      4C      4D      4E      4F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 50     51      52      53      54      55      56      57
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 58     59      5A      5B      5C      5D      5E      5F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 60     61      62      63      64      65      66      67
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 68     69      6A      6B      6C      6D      6E      6F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 70     71      72      73      74      75      76      77
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 78     79      7A      7B      7C      7D      7E      7F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x80u,
    // 80     81      82      83      84      85      86      87
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 88     89      8A      8B      8C      8D      8E      8F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 90     91      92      93      94      95      96      97
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 98     99      9A      9B      9C      9D      9E      9F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // A0     A1      A2      A3      A4      A5      A6      A7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // A8     A9      AA      AB      AC      AD      AE      AF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // B0     B1      B2      B3      B4      B5      B6      B7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // B8     B9      BA      BB      BC      BD      BE      BF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // C0     C1      C2      C3      C4      C5      C6      C7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // C8     C9      CA      CB      CC      CD      CE      CF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // D0     D1      D2      D3      D4      D5      D6      D7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // D8     D9      DA      DB      DC      DD      DE      DF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // E0     E1      E2      E3      E4      E5      E6      E7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // E8     E9      EA      EB      EC      ED      EE      EF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // F0     F1      F2      F3      F4      F5      F6      F7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // F8     F9      FA      FB      FC      FD      FE      FF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
};

namespace
{
    /**
     * @brief 检查字符是否允许出现在Host中
     */
    bool IsForbiddenHostChar(char ch)noexcept
    {
        return ch == '\0' || ch == '\t' || ch == '\n' || ch == '\r' || ch == ' ' || ch == '#' || ch == '%' ||
               ch == '/' || ch == ':' || ch == '?' || ch == '@' || ch == '[' || ch == '\\' || ch == ']';
    }

    /**
     * @brief 解码UTF-8字符
     */
    std::string PercentDecode(const char* start, const char* end)
    {
        std::string ret;
        if (start >= end || start == nullptr)
            return ret;

        ret.reserve(end - start);
        while (start < end)
        {
            char ch = *start;
            size_t remaining = end - start - 1;
            unsigned a = 0, b = 0;
            if (remaining >= 2 && ch == '%' && StringUtils::HexDigitToNumber(a, start[1]) &&
                StringUtils::HexDigitToNumber(b, start[2]))
            {
                ret.push_back(static_cast<char>(a * 16 + b));
                start += 3;
            }
            else
            {
                ret.push_back(ch);
                ++start;
            }
        }

        return ret;
    }

    /**
     * @brief 向字符串中追加一个字符或者是转义后的序列
     * @param out 结果串
     * @param ch 字符
     * @param charset 转义序列
     */
    void AppendOrEscape(std::string& out, char ch, const uint8_t charset[])
    {
        if (!!(charset[ch >> 3] & (1 << (ch & 7))))  // 位图查询
            out.append(kHexTable[static_cast<uint8_t>(ch)]);
        else
            out.push_back(ch);
    }

    /**
     * @brief 解析IPV4中的数字（十进制、十六进制、八进制）
     */
    bool ParseIpv4Number(uint32_t& result, const char* start, const char* end)noexcept
    {
        uint64_t temp = 0;
        unsigned radix = 10;
        if (end - start >= 2)
        {
            if (start[0] == '0' && (start[1] == 'x' || start[1] == 'X'))  // 16进制
            {
                start += 2;
                radix = 16;
            }
            else if (start[0] == '0')  // 8进制
            {
                start += 1;
                radix = 8;
            }
        }

        temp = result = 0;
        if (start >= end)
            return true;

        while (start < end)
        {
            char ch = *(start++);

            if (ch >= 'a' && ch <= 'f')
            {
                if (radix != 16)
                    return false;
                temp = temp * 16 + (ch - 'a') + 10;
            }
            else if (ch >= 'A' && ch <= 'F')
            {
                if (radix != 16)
                    return false;
                temp = temp * 16 + (ch - 'A') + 10;
            }
            else if (ch >= '0' && ch <= '9')
            {
                if ((ch == '8' || ch == '9') && radix == 8)
                    return false;
                temp = temp * radix + (ch - '0');
            }
            else
                return false;

            if (temp > numeric_limits<uint32_t>::max())
                return false;
        }

        result = static_cast<uint32_t>(temp);
        return true;
    }
}

//////////////////////////////////////////////////////////////////////////////// Url::Host

Url::Host::Host()
{
}

Url::Host::Host(const Host& rhs)
    : m_iType(rhs.m_iType)
{
    switch (m_iType)
    {
        case HostTypes::None:
            break;
        case HostTypes::Domain:
            new(&m_stValue.Domain) string(rhs.m_stValue.Domain);
            break;
        case HostTypes::Ipv4:
            m_stValue.Ipv4 = rhs.m_stValue.Ipv4;
            break;
        case HostTypes::Ipv6:
            new(&m_stValue.Ipv6) Ipv6AddressType(rhs.m_stValue.Ipv6);
            break;
        case HostTypes::Opaque:
            new(&m_stValue.Opaque) string(rhs.m_stValue.Opaque);
            break;
        default:
            assert(false);
            break;
    }
}

Url::Host::Host(Host&& rhs)noexcept
    : m_iType(rhs.m_iType)
{
    switch (m_iType)
    {
        case HostTypes::None:
            break;
        case HostTypes::Domain:
            new(&m_stValue.Domain) string(std::move(rhs.m_stValue.Domain));
            break;
        case HostTypes::Ipv4:
            m_stValue.Ipv4 = rhs.m_stValue.Ipv4;
            break;
        case HostTypes::Ipv6:
            new(&m_stValue.Ipv6) Ipv6AddressType(rhs.m_stValue.Ipv6);
            break;
        case HostTypes::Opaque:
            new(&m_stValue.Opaque) string(std::move(rhs.m_stValue.Opaque));
            break;
        default:
            assert(false);
            break;
    }

    rhs.Reset();
}

Url::Host::~Host()
{
    Reset();
}

Url::Host& Url::Host::operator=(const Host& rhs)
{
    Reset();

    switch (m_iType)
    {
        case HostTypes::None:
            break;
        case HostTypes::Domain:
            new(&m_stValue.Domain) string(rhs.m_stValue.Domain);
            break;
        case HostTypes::Ipv4:
            m_stValue.Ipv4 = rhs.m_stValue.Ipv4;
            break;
        case HostTypes::Ipv6:
            new(&m_stValue.Ipv6) Ipv6AddressType(rhs.m_stValue.Ipv6);
            break;
        case HostTypes::Opaque:
            new(&m_stValue.Opaque) string(rhs.m_stValue.Opaque);
            break;
        default:
            assert(false);
            break;
    }

    m_iType = rhs.m_iType;
    return *this;
}

Url::Host& Url::Host::operator=(Host&& rhs)noexcept
{
    Reset();

    switch (m_iType)
    {
        case HostTypes::None:
            break;
        case HostTypes::Domain:
            new(&m_stValue.Domain) string(std::move(rhs.m_stValue.Domain));
            break;
        case HostTypes::Ipv4:
            m_stValue.Ipv4 = rhs.m_stValue.Ipv4;
            break;
        case HostTypes::Ipv6:
            new(&m_stValue.Ipv6) Ipv6AddressType(rhs.m_stValue.Ipv6);
            break;
        case HostTypes::Opaque:
            new(&m_stValue.Opaque) string(std::move(rhs.m_stValue.Opaque));
            break;
        default:
            assert(false);
            break;
    }

    m_iType = rhs.m_iType;
    rhs.Reset();
    return *this;
}

bool Url::Host::operator==(const Host& rhs)const noexcept
{
    if (m_iType != rhs.m_iType)
        return false;

    switch (m_iType)
    {
        case HostTypes::None:
            return true;
        case HostTypes::Domain:
            return m_stValue.Domain == rhs.m_stValue.Domain;
        case HostTypes::Ipv4:
            return m_stValue.Ipv4 == rhs.m_stValue.Ipv4;
        case HostTypes::Ipv6:
            return m_stValue.Ipv6 == rhs.m_stValue.Ipv6;
        case HostTypes::Opaque:
            return m_stValue.Opaque == rhs.m_stValue.Opaque;
        default:
            return false;
    }
}

bool Url::Host::operator!=(const Host& rhs)const noexcept
{
    return !operator==(rhs);
}

Url::Host::operator bool()const noexcept
{
    return m_iType != HostTypes::None;
}

Url::Host::HostTypes Url::Host::GetType()const noexcept
{
    return m_iType;
}

const std::string& Url::Host::GetDomain()const noexcept
{
    static const string kEmpty;
    return m_iType == HostTypes::Domain ? m_stValue.Domain : kEmpty;
}

void Url::Host::SetDomain(const std::string& value)
{
    Reset();

    new(&m_stValue.Domain) string(value);
    m_iType = HostTypes::Domain;
}

void Url::Host::SetDomain(std::string&& value)noexcept
{
    Reset();

    new(&m_stValue.Domain) string(std::move(value));
    m_iType = HostTypes::Domain;
}

uint32_t Url::Host::GetIpv4()const noexcept
{
    return m_iType == HostTypes::Ipv4 ? m_stValue.Ipv4 : 0u;
}

void Url::Host::SetIpv4(uint32_t value)noexcept
{
    Reset();

    m_iType = HostTypes::Ipv4;
    m_stValue.Ipv4 = value;
}

const Url::Host::Ipv6AddressType& Url::Host::GetIpv6()const noexcept
{
    static const Ipv6AddressType kEmpty = {};
    return m_iType == HostTypes::Ipv6 ? m_stValue.Ipv6 : kEmpty;
}

void Url::Host::SetIpv6(const Ipv6AddressType& value)noexcept
{
    Reset();

    m_iType = HostTypes::Ipv6;
    m_stValue.Ipv6 = value;
}

const std::string& Url::Host::GetOpaque()const noexcept
{
    static const string kEmpty;
    return m_iType == HostTypes::Opaque ? m_stValue.Opaque : kEmpty;
}

void Url::Host::SetOpaque(const std::string& value)
{
    Reset();

    new(&m_stValue.Opaque) string(value);
    m_iType = HostTypes::Opaque;
}

void Url::Host::SetOpaque(std::string&& value)noexcept
{
    Reset();

    new(&m_stValue.Opaque) string(std::move(value));
    m_iType = HostTypes::Opaque;
}

void Url::Host::Reset()
{
    switch (m_iType)
    {
        case HostTypes::Domain:
            m_stValue.Domain.~string();
            break;
        case HostTypes::Ipv4:
            break;
        case HostTypes::Ipv6:
            m_stValue.Ipv6.~Ipv6AddressType();
            break;
        case HostTypes::Opaque:
            m_stValue.Opaque.~string();
            break;
        default:
            break;
    }

    m_iType = HostTypes::None;
}

bool Url::Host::Parse(const std::string& text, bool special, bool unicode)
{
    return Parse(text.c_str(), text.c_str() + text.length(), special, unicode);
}

bool Url::Host::Parse(const char* text, bool special, bool unicode)
{
    return Parse(text, text + std::strlen(text), special, unicode);
}

bool Url::Host::Parse(const char* text, size_t length, bool special, bool unicode)
{
    return Parse(text, text + length, special, unicode);
}

std::string Url::Host::ToString()const
{
    std::string ret;
    switch (m_iType)
    {
        case HostTypes::Domain:
            return m_stValue.Domain;
        case HostTypes::Opaque:
            return m_stValue.Opaque;
        case HostTypes::Ipv4:
            ret.reserve(15);
            for (uint32_t value = m_stValue.Ipv4, n = 0; n < 4; n++)
            {
                char buf[4];
                Convert::ToDecimalString(static_cast<uint8_t>(value % 256), buf);

                ret.insert(0, buf);
                if (n < 3)
                    ret.insert(0, 1, '.');
                value /= 256;
            }
            break;
        case HostTypes::Ipv6:
            ret.reserve(41);
            ret.push_back('[');
            {
                uint32_t start = 0;
                uint32_t compress = 0;

                // 找到最长的0的部分
                uint32_t cur = 0xFFFFFFFFu, count = 0, longest = 0;
                while (start < 8)
                {
                    if (m_stValue.Ipv6[start] == 0)
                    {
                        if (cur == 0xFFFFFFFFu)
                            cur = start;
                        ++count;
                    }
                    else
                    {
                        if (count > longest)
                        {
                            longest = count;
                            compress = cur;
                        }
                        count = 0;
                        cur = 0xFFFFFFFFu;
                    }
                    ++start;
                }
                if (count > longest)
                    compress = cur;

                // 序列化过程
                bool ignore0 = false;
                for (unsigned n = 0; n < 8; ++n)
                {
                    auto piece = m_stValue.Ipv6[n];
                    if (ignore0 && piece == 0)
                        continue;
                    else if (ignore0)
                        ignore0 = false;
                    if (compress == n)
                    {
                        ret.append(n == 0 ? "::" : ":");
                        ignore0 = true;
                        continue;
                    }

                    char buf[5];
                    Convert::ToHexStringLower(piece, buf);
                    ret.append(buf);
                    if (n < 7)
                        ret.push_back(':');
                }
            }
            ret.push_back(']');
            break;
        default:
            break;
    }
    return ret;
}

bool Url::Host::IsAsciiFastPath(const std::string& domain)noexcept
{
    static const char kDeliminators[] = { '.' };
    static const char kPunycodePrefix[] = { 'x', 'n', '-', '-', '\0' };

    // 按dot对domain进行分割
    auto it = StringUtils::SplitByCharsBegin<char>(ToArrayView<char>(domain), ArrayView<char>(kDeliminators, 1));
    while (it != StringUtils::SplitByCharsEnd<char>())
    {
        auto label = *it;

        if (label.GetSize() >= sizeof(kPunycodePrefix) - 1 &&
            ::strncmp(kPunycodePrefix, label.GetBuffer(), sizeof(kPunycodePrefix) - 1) == 0)
        {
            return false;
        }

        for (size_t i = 0; i < label.GetSize(); ++i)
        {
            auto ch = label[i];

            if (ch <= 0x002C)
                return false;
            else if (ch == 0x002F)
                return false;
            else if (0x003A <= ch && ch <= 0x0040)
                return false;
            else if (0x005B <= ch && ch <= 0x0060)
                return false;
            else if (0x007B <= ch)
                return false;
        }

        ++it;
    }

    return true;
}

bool Url::Host::Parse(const char* start, const char* end, bool special, bool unicode)
{
    if (start >= end || start == nullptr)
        return false;

    if (*start == '[')
    {
        if (*(end - 1) != ']')
            return false;
        return ParseIpv6(start + 1, end - 1);
    }
    if (!special)
        return ParseOpaque(start, end);

    auto decoded = PercentDecode(start, end);

    u32string temp, temp2;
    auto isFastPath = IsAsciiFastPath(decoded);
    if (!isFastPath)
    {
        temp.reserve(decoded.length());
        temp2.reserve(decoded.length());

        // Punycode ToAscii
        Encoding::Convert<Encoding::UTF32, Encoding::UTF8>(temp, ArrayView<char>(decoded.data(), decoded.length()));
        Idna::ToAscii(temp2, ArrayView<char32_t>(temp.data(), temp.length()), false, true, true, true, false, true);
        Encoding::Convert<Encoding::UTF8, Encoding::UTF32>(decoded, ArrayView<char32_t>(temp2.data(), temp2.length()));
    }
    else
    {
        for (size_t i = 0; i < decoded.length(); ++i)
        {
            auto ch = decoded[i];
            if ('A' <= ch && ch <= 'Z')
                decoded[i] = ch - 'A' + 'a';
        }
    }

    // 检查字符合法性
    for (size_t n = 0; n < decoded.size(); ++n)
    {
        char ch = decoded[n];
        if (IsForbiddenHostChar(ch))
            return false;
    }

    // 检查是否是IPV4地址
    if (ParseIpv4(decoded.c_str(), decoded.c_str() + decoded.length()))
        return true;

    if (!isFastPath && unicode)
    {
        // Punycode ToUnicode
        Encoding::Convert<Encoding::UTF32, Encoding::UTF8>(temp, ArrayView<char>(decoded.data(), decoded.length()));
        Idna::ToUnicode(temp2, ArrayView<char32_t>(temp.data(), temp.length()), false, true, true, false, false);
        Encoding::Convert<Encoding::UTF8, Encoding::UTF32>(decoded, ArrayView<char32_t>(temp2.data(), temp2.length()));
    }

    // 不是IPV6或者IPV4，那么就是域名
    SetDomain(std::move(decoded));
    return true;
}

bool Url::Host::ParseIpv4(const char* start, const char* end)noexcept
{
    auto mark = start;
    unsigned parts = 0;
    unsigned tooBigParts = 0;
    uint32_t numbers[4] = { 0, 0, 0, 0 };

    while (start <= end)
    {
        char ch = start < end ? *start : '\0';
        if (ch == '.' || ch == '\0')
        {
            if (++parts > 4)
                return false;  // IPV4地址至多只有4个部分
            if (start == mark)
                return false;  // 无效的空的点分部分

            // 解析[mark, start)部分的数字
            uint32_t n = 0;
            if (!ParseIpv4Number(n, mark, start))
                return false;
            if (n > 255)
                ++tooBigParts;

            numbers[parts - 1] = n;
            mark = start + 1;
            if (ch == '.' && start + 1 >= end)
                break;  // 允许输入的最后一个字符是'.'
        }
        ++start;
    }
    assert(parts > 0);

    if (tooBigParts > 1 || (tooBigParts == 1 && numbers[parts - 1] <= 255) ||
        numbers[parts - 1] >= std::pow(256, static_cast<double>(5 - parts)))
    {
        // 规范要求每个点分部分不能超过255，但是最后一个元素例外
        // 此外，整个IPV4的解析结果也要保证不能溢出
        return false;
    }

    // 计算IPV4值
    uint32_t val = numbers[parts - 1];
    for (unsigned n = 0; n < parts - 1; ++n)
    {
        double b = 3 - n;
        val += numbers[n] * std::pow(256, b);
    }
    SetIpv4(val);
    return true;
}

bool Url::Host::ParseIpv6(const char* start, const char* end)noexcept
{
    char ch = start < end ? *start : '\0';
    unsigned current = 0;  // 指示当前解析的部分
    uint32_t compress = 0xFFFFFFFFu;  // 指示压缩可扩充的位置

    Ipv6AddressType address;
    address.fill(0);

    // 压缩的case '::xxxx'
    if (ch == ':')
    {
        if (end - start < 2 || start[1] != ':')
            return false;  // 无效的':
        start += 2;
        ch = start < end ? *start : '\0';
        ++current;
        compress = current;
    }

    while (ch != '\0')
    {
        if (current >= address.max_size())
            return false;  // 无效的地址（至多8个部分）

        // 压缩的case 'fe80::xxxxx'
        if (ch == ':')
        {
            if (compress != 0xFFFFFFFFu)
                return false;  // 不可能同时存在两个压缩部分
            ++start;
            ch = start < end ? *start : '\0';
            address[current++] = 0;
            compress = current;
            continue;
        }

        uint32_t value = 0;
        unsigned len = 0;
        while (len < 4 && StringUtils::IsHexDigit(ch))
        {
            value = value * 16 + StringUtils::HexDigitToNumber(ch);

            ++start;
            ch = start < end ? *start : '\0';
            ++len;
        }

        switch (ch)
        {
            case '.':  // 内嵌IPV4地址
                if (len == 0)
                    return false;
                start -= len;  // 回退
                ch = start < end ? *start : '\0';
                if (current > address.max_size() - 2)  // IPV4地址占两个Piece
                    return false;

                // 解析IPV4部分（这一地址只允许使用十进制点分结构）
                {
                    unsigned numbers = 0;
                    while (ch != '\0')
                    {
                        value = 0xFFFFFFFF;
                        if (numbers > 0)
                        {
                            if (ch == '.' && numbers < 4)
                            {
                                ++start;
                                ch = start < end ? *start : '\0';
                            }
                            else
                                return false;
                        }
                        if (!StringUtils::IsDigit(ch))
                            return false;
                        while (StringUtils::IsDigit(ch))
                        {
                            unsigned number = ch - '0';
                            if (value == 0xFFFFFFFF)
                                value = number;
                            else if (value == 0)
                                return false;
                            else
                                value = value * 10 + number;
                            if (value > 255)
                                return false;
                            ++start;
                            ch = start < end ? *start : '\0';
                        }
                        address[current] = static_cast<uint16_t>(address[current] * 0x100 + value);
                        ++numbers;
                        if (numbers == 2 || numbers == 4)
                            ++current;
                    }
                    if (numbers != 4)
                        return false;
                }
                continue;
            case ':':
                ++start;
                ch = start < end ? *start : '\0';
                if (ch == '\0')
                    return false;
                break;
            case '\0':
                break;
            default:
                return false;
        }
        address[current] = value;
        ++current;
    }

    if (compress != 0xFFFFFFFFu)
    {
        auto swaps = current - compress;
        current = address.max_size() - 1;
        while (current != 0 && swaps > 0)
        {
            auto swap = compress + swaps - 1;
            std::swap(address[current], address[swap]);
            --current;
            --swaps;
        }
    }
    else if (current != address.max_size())  // 没有压缩，则必然读取了所有的部分
        return false;

    SetIpv6(address);
    return true;
}

bool Url::Host::ParseOpaque(const char* start, const char* end)
{
    std::string output;
    output.reserve((end - start) * 3);  // 最坏情况下所有字符都需要转义

    while (start < end)
    {
        char ch = *(start++);
        if (ch != '%' && IsForbiddenHostChar(ch))
            return false;
        AppendOrEscape(output, ch, kC0ControlEncodeSet);
    }

    SetOpaque(std::move(output));
    return true;
}
