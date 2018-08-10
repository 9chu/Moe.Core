/**
 * @file
 * @author chu
 * @date 2018/8/5
 */
#pragma once
#include <array>
#include <string>
#include <vector>
#include <cstring>
#include <functional>

#include "ArrayView.hpp"
#include "Exception.hpp"

namespace moe
{
    /**
     * @brief HTTP状态码
     */
    enum class HttpStatus
    {
        Continue = 100,
        SwitchingProtocols = 101,
        Processing = 102,
        Ok = 200,
        Created = 201,
        Accepted = 202,
        NonAuthoritativeInformation = 203,
        NoContent = 204,
        ResetContent = 205,
        PartialContent = 206,
        MultiStatus = 207,
        AlreadyReported = 208,
        ImUsed = 226,
        MultipleChoices = 300,
        MovedPermanently = 301,
        Found = 302,
        SeeOther = 303,
        NotModified = 304,
        UseProxy = 305,
        TemporaryRedirect = 307,
        PermanentRedirect = 308,
        BadRequest = 400,
        Unauthorized = 401,
        PaymentRequired = 402,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,
        NotAcceptable = 406,
        ProxyAuthenticationRequired = 407,
        RequestTimeout = 408,
        Conflict = 409,
        Gone = 410,
        LengthRequired = 411,
        PreconditionFailed = 412,
        PayloadTooLarge = 413,
        UriTooLong = 414,
        UnsupportedMediaType = 415,
        RangeNotSatisfiable = 416,
        ExpectationFailed = 417,
        MisdirectedRequest = 421,
        UnprocessableEntity = 422,
        Locked = 423,
        FailedDependency = 424,
        UpgradeRequired = 426,
        PreconditionRequired = 428,
        TooManyRequests = 429,
        RequestHeaderFieldsTooLarge = 431,
        UnavailableForLegalReasons = 451,
        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503,
        GatewayTimeout = 504,
        HttpVersionNotSupported = 505,
        VariantAlsoNegotiates = 506,
        InsufficientStorage = 507,
        LoopDetected = 508,
        NotExtended = 510,
        NetworkAuthenticationRequired = 511,
    };

    /**
     * @brief HTTP方法
     */
    enum class HttpMethods
    {
        Unknown = 0,
        Delete = 1,
        Get = 2,
        Head = 3,
        Post = 4,
        Put = 5,
        Connect = 6,
        Options = 7,
        Trace = 8,
        Copy = 9,
        Lock = 10,
        MkCol = 11,
        Move = 12,
        PropFind = 13,
        PropPatch = 14,
        Search = 15,
        Unlock = 16,
        Bind = 17,
        Rebind = 18,
        Unbind = 19,
        Acl = 20,
        Report = 21,
        MkActivity = 22,
        Checkout = 23,
        Merge = 24,
        MSearch = 25,
        Notify = 26,
        Subscribe = 27,
        Unsubscribe = 28,
        Patch = 29,
        Purge = 30,
        MkCalendar = 31,
        Link = 32,
        Unlink = 33,
        Source = 34,
    };

    /**
     * @brief 解析器类型
     */
    enum class HttpParserTypes
    {
        Both = 0,
        Request = 1,
        Response = 2,
    };

    /**
     * @brief 获取HTTP状态码文本
     * @param status 状态码
     */
    const char* GetHttpStatusText(HttpStatus status)noexcept;

    /**
     * @brief 获取HTTP方法文本
     * @param method 方法
     */
    const char* GetHttpMethodsText(HttpMethods method)noexcept;

    /**
     * @brief HTTP解析器设定项
     *
     * 各配置项如下：
     *  - maxHeaderSize 最大HTTP头部大小
     *  - strictToken HTTP字符严格模式
     *  - strictUrlToken URL字符严格模式
     *  - lenientHeaders HTTP头部字符严格模式
     */
    struct HttpParserSettings
    {
        size_t MaxHeaderSize = 80*1024u;
        bool StrictToken = true;
        bool StrictUrlToken = false;
        bool LenientHeaders = false;
    };

    /**
     * @brief HTTP解析器基类
     * @see https://github.com/nodejs/http-parser
     *
     * 该类实现了基本的HTTP解析逻辑，并且无额外内存分配。通过继承这一基类来实现一些复杂的逻辑。
     */
    class HttpParserBase
    {
    public:
        /**
         * @brief 由用户决定的头部处理结果
         */
        enum class HeadersCompleteResult
        {
            Default,
            SkipBody,
            Upgrade,
        };

    public:
        /**
         * @brief 构造HTTP解析器
         * @param type 解析器解析类型
         * @param settings 解析器配置
         */
        HttpParserBase(HttpParserTypes type=HttpParserTypes::Both,
            const HttpParserSettings& settings=EmptyRefOf<HttpParserSettings>())noexcept
            : m_uMaxHeaderSize(settings.MaxHeaderSize), m_bStrictToken(settings.StrictToken),
            m_bStrictUrlToken(settings.StrictUrlToken), m_bLenientHeaders(settings.LenientHeaders)
        {
            Reset(type);
        }

    public:
        /**
         * @brief 获取解析器类型
         */
        HttpParserTypes GetType()const noexcept { return m_uType; }

        /**
         * @brief 获取解析后的类型
         */
        HttpParserTypes GetParsedType()const noexcept { return m_uParsedType; }

        /**
         * @brief 获取设置的最大Header大小
         */
        size_t GetMaxHeaderSize()const noexcept { return m_uMaxHeaderSize; }

        /**
         * @brief 是否是严格的HTTP字符模式
         */
        bool IsStrictToken()const noexcept { return m_bStrictToken; }

        /**
         * @brief 是否是严格的URL字符模式
         */
        bool IsStrictUrlToken()const noexcept { return m_bStrictUrlToken; }

        /**
         * @brief 是否是宽容的Header字符模式
         */
        bool IsLenientHeaders()const noexcept { return m_bLenientHeaders; }

        /**
         * @brief 获取解析后的方法
         */
        HttpMethods GetMethod()const noexcept { return m_uMethod; }

        /**
         * @brief 获取解析后的HTTP主版本号
         */
        uint8_t GetMajorVersion()const noexcept { return m_uHttpMajor; }

        /**
         * @brief 获取解析后的HTTP子版本号
         */
        uint8_t GetMinorVersion()const noexcept { return m_uHttpMinor; }

        /**
         * @brief 获取解析后的状态码
         */
        unsigned GetStatusCode()const noexcept { return m_uStatusCode; }

        /**
         * @brief 是否发生协议切换
         */
        bool IsUpgrade()const noexcept { return m_bUpgrade; }

        /**
         * @brief 连接是否应当保持
         */
        bool ShouldKeepAlive()noexcept;

        /**
         * @brief 重置解析器状态
         */
        void Reset(HttpParserTypes type)noexcept;

        /**
         * @brief 解析HTTP数据
         * @exception BadFormatException 当解析内容不合法时抛出
         * @param input 输入缓冲区
         * @return 消耗的字节数
         *
         * 注意到：当输入流读取到EOF时，需要手动传入一个空的input来告知EOF发生。
         * 当异常发生时，会重置内部状态。
         */
        size_t Parse(BytesView input)
        {
            try
            {
                return ParseImpl(input);
            }
            catch (...)
            {
                Reset(m_uType);
                throw;
            }
        }

    protected:  // 需要实现的回调函数
        // 下述回调函数用于响应相应的HTTP解析事件。
        // 注意到除去事件触发类回调，所有的数据回调都可能被多次调用。
        virtual void OnMessageBegin() = 0;
        virtual void OnUrl(BytesView data) = 0;
        virtual void OnStatus(BytesView data) = 0;
        virtual void OnHeaderField(BytesView data) = 0;
        virtual void OnHeaderValue(BytesView data) = 0;
        virtual HeadersCompleteResult OnHeadersComplete() = 0;  // 当返回true时说明没有Body，将影响解析器工作
        virtual void OnBody(BytesView data) = 0;
        virtual void OnMessageComplete() = 0;
        virtual void OnChunkHeader(size_t length) = 0;
        virtual void OnChunkComplete() = 0;

    private:
        bool IsEofRequired()noexcept;

        void ParseUrl(char ch);
        void ResetNewMessageState()noexcept;
        size_t ParseImpl(BytesView input);

    private:
        // 配置
        HttpParserTypes m_uType = HttpParserTypes::Both;
        size_t m_uMaxHeaderSize = 80 * 1024u;  // 最大HTTP头部大小，默认80K
        bool m_bStrictToken = true;  // 严格模式
        bool m_bStrictUrlToken = false;  // URL严格模式
        bool m_bLenientHeaders = false;  // 宽容HTTP头部模式

        // 全局状态
        HttpParserTypes m_uParsedType = HttpParserTypes::Both;
        unsigned m_uState = 0;  // 当前的解析状态

        // 上下文相关状态
        unsigned m_uFlags = 0;  // 当前的特殊标志位
        size_t m_uRead = 0;  // 当前读取的字节数
        unsigned m_uHeaderState = 0;
        unsigned m_uIndex = 0;  // 在一些Token中的索引
        uint64_t m_uContentLength = 0;  // 正文长度

        // 解析结果
        HttpMethods m_uMethod = HttpMethods::Unknown;  // HTTP方法
        uint8_t m_uHttpMajor = 0;
        uint8_t m_uHttpMinor = 0;
        unsigned m_uStatusCode = 0;
        bool m_bUpgrade = false;
    };

    /**
     * @brief HTTP头部
     */
    class HttpHeaders
    {
        struct Hasher
        {
            size_t operator()(const std::string& key)const noexcept
            {
                size_t hash = 5381;
                for (size_t i = 0; i < key.length(); ++i)
                {
                    auto b = StringUtils::ToLower(key[i]);
                    hash += (hash << 5u) + b;
                }
                return hash;
            }
        };

        struct KeyEqual
        {
            bool operator()(const std::string& lhs, const std::string& rhs)const noexcept
            {
                return StringUtils::CaseInsensitiveCompare(lhs, rhs) == 0;
            }
        };

    public:
        using ContainerType = std::unordered_multimap<std::string, std::string, Hasher, KeyEqual>;
        using IteratorType = ContainerType::iterator;
        using ConstIteratorType = ContainerType::const_iterator;

    public:
        /**
         * @brief 索引器
         * @param key 键值
         * @return 返回其中一个满足键值的值，否则，插入一个
         */
        const std::string& operator[](const std::string& key)const noexcept;
        std::string& operator[](const std::string& key)noexcept;

    public:
        /**
         * @brief 获取元素数量
         */
        size_t GetSize()const noexcept;

        /**
         * @brief 是否为空
         */
        bool IsEmpty()const noexcept;

        /**
         * @brief 添加Key-Value
         */
        void Add(const std::string& key, const std::string& value);
        void Add(std::string&& key, std::string value);

        /**
         * @brief 删除Key
         * @return 是否成功删除
         *
         * 删除所有键值为key的对。
         */
        void Remove(const std::string& key)noexcept;

        /**
         * @brief 检查Key是否存在
         */
        bool Contains(const std::string& key)const noexcept;

        /**
         * @brief 检查Key的数量
         */
        size_t Count(const std::string& key)const noexcept;

        /**
         * @brief 清空容器
         */
        void Clear()noexcept;

        /**
         * @brief 寻找符合Key的值
         * @param key 键值
         * @return [begin, end)
         */
        std::pair<ConstIteratorType, ConstIteratorType> Range(const std::string& key)const noexcept;
        std::pair<IteratorType, IteratorType> Range(const std::string& key)noexcept;

        /**
         * @brief 首个元素
         */
        ConstIteratorType First()const noexcept;
        IteratorType First()noexcept;

        /**
         * @brief 最后一个元素
         */
        ConstIteratorType Last()const noexcept;
        IteratorType Last()noexcept;

        /**
         * @brief 序列化追加到
         * @param out 输出
         */
        void SerializeTo(std::string& out)const;

        /**
         * @brief 序列化
         */
        std::string ToString()const;

    private:
        ContainerType m_stHeaders;
    };

    /**
     * @brief HTTP协议
     *
     * 实现了HTTP协议的解析和序列化。
     */
    class HttpProtocol :
        protected HttpParserBase
    {
    public:
        enum class ProtocolType
        {
            Request,
            Response,
        };

        using HttpParserBase::HeadersCompleteResult;

        using HeadersCompleteCallback = std::function<HeadersCompleteResult()>;
        using BodyDataCallback = std::function<void(BytesView)>;

    public:
        /**
         * @brief 构造HTTP协议
         * @param type 协议类型
         * @param settings 解析器配置
         */
        HttpProtocol(ProtocolType type, const HttpParserSettings& settings=EmptyRefOf<HttpParserSettings>())noexcept;

    public:
        /**
         * @brief 获取协议类型
         */
        ProtocolType GetType()const noexcept { return m_uType; }

        /**
         * @brief 获取或设置方法
         */
        HttpMethods GetMethod()const noexcept { return m_uMethod; }
        void SetMethod(HttpMethods method)noexcept { m_uMethod = method; }

        /**
         * @brief 获取或设置HTTP主版本号
         */
        uint8_t GetMajorVersion()const noexcept { return m_uHttpMajor; }
        void SetMajorVersion(uint8_t major)noexcept { m_uHttpMajor = major; }

        /**
         * @brief 获取或设置HTTP子版本号
         */
        uint8_t GetMinorVersion()const noexcept { return m_uHttpMinor; }
        void SetMinorVersion(uint8_t minor)noexcept { m_uHttpMinor = minor; }

        /**
         * @brief 获取或设置状态码
         */
        HttpStatus GetStatusCode()const noexcept { return m_uStatusCode; }
        void SetStatusCode(HttpStatus code)noexcept { m_uStatusCode = code; }

        /**
         * @brief 获取URL值
         */
        const std::string& GetUrl()const noexcept { return m_stUrl; }

        /**
         * @brief 设置URL值
         */
        void SetUrl(const std::string& str) { m_stUrl = str; }
        void SetUrl(std::string&& str) { m_stUrl = std::move(str); }

        /**
         * @brief 访问Headers对象
         */
        const HttpHeaders& Headers()const noexcept { return m_stHeaders; }
        HttpHeaders& Headers()noexcept { return m_stHeaders; }

        /**
         * @brief 获取或设置Headers解析完成的回调
         */
        HeadersCompleteCallback GetHeadersCompleteCallback()const noexcept { return m_pHeadersCompleteCallback; }
        void SetHeadersCompleteCallback(const HeadersCompleteCallback& cb) { m_pHeadersCompleteCallback = cb; }

        /**
         * @brief 获取或设置Body数据回调
         */
        BodyDataCallback GetBodyDataCallback()const noexcept { return m_pBodyDataCallback; }
        void SetBodyDataCallback(const BodyDataCallback& cb) { m_pBodyDataCallback = cb; }

        /**
         * @brief 重置状态
         */
        void Reset()noexcept;

        /**
         * @brief 从数据解析HTTP请求
         * @param input 输入
         * @param[out] processed 处理的输入数量
         * @return 解析是否完成（指示请求或响应结束）
         *
         * 当解析完成时，调用方需要检查 IsUpgrade() 方法的返回值，以判断协议是否发生变动。
         * 否则，当 ShouldKeepAlive() 返回 false 时，需要处理请求后关闭连接。
         */
        bool Parse(BytesView input, size_t* processed=nullptr);

        /**
         * @brief 是否应当升级协议
         */
        bool IsUpgraded()const noexcept;

        /**
         * @brief 是否应当保持连接
         */
        bool ShouldKeepAlive()const noexcept;

        /**
         * @brief 序列化追加到
         * @param out 输出
         *
         * 仅序列化非正文部分。
         */
        void SerializeTo(std::string& out)const;

        /**
         * @brief 序列化
         */
        std::string ToString()const;

    protected:
        void OnMessageBegin()override;
        void OnUrl(BytesView data)override;
        void OnStatus(BytesView data)override;
        void OnHeaderField(BytesView data)override;
        void OnHeaderValue(BytesView data)override;
        HeadersCompleteResult OnHeadersComplete()override;
        void OnBody(BytesView data)override;
        void OnMessageComplete()override;
        void OnChunkHeader(size_t length)override;
        void OnChunkComplete()override;

    private:
        ProtocolType m_uType = ProtocolType::Request;

        // 协议属性
        HttpMethods m_uMethod = HttpMethods::Unknown;
        uint8_t m_uHttpMajor = 0;
        uint8_t m_uHttpMinor = 0;
        HttpStatus m_uStatusCode = HttpStatus::Ok;
        std::string m_stUrl;
        HttpHeaders m_stHeaders;

        // 解析器状态
        unsigned m_uState = 0;
        std::string m_stKeyBuffer;
        std::string m_stBuffer;
        HeadersCompleteCallback m_pHeadersCompleteCallback;
        BodyDataCallback m_pBodyDataCallback;
    };

    /**
     * @brief WebSocket协议
     */
    class WebSocketProtocol
    {
    public:
        using HeadersCompleteCallback = std::function<void()>;
        using DataCallback = std::function<void(BytesView)>;
        using MessageCompleteCallback = std::function<void()>;

        using ReservedDataType = std::array<bool, 3>;
        using MaskKeyType = std::array<uint8_t, 4>;

    public:
        WebSocketProtocol()noexcept;

    public:
        /**
         * @brief 获取或设置是否是最后一个包
         */
        bool IsLastPacket()const noexcept { return m_bFin; }
        void SetLastPacket(bool fin)noexcept { m_bFin = fin; }

        /**
         * @brief 获取或设置保留标志位
         */
        ReservedDataType GetReserves()const noexcept { return m_stReserves; }
        void SetReserves(const ReservedDataType& data)noexcept { m_stReserves = data; }

        /**
         * @brief 获取或设置操作码
         */
        uint8_t GetOpCode()const noexcept { return m_bOpCode; }
        void SetOpCode(uint8_t op)noexcept { m_bOpCode = op; }

        /**
         * @brief 获取或设置消息是否有做掩码
         */
        bool IsMasked()const noexcept { return m_bMask; }
        void SetMasked(bool mask)noexcept { m_bMask = mask; }

        /**
         * @brief 获取或设置负载长度
         */
        uint64_t GetPayloadLength()const noexcept { return m_uPayloadLength; }
        void SetPayloadLength(uint64_t length)noexcept { m_uPayloadLength = length; }

        /**
         * @brief 获取或设置掩码字节数组
         */
        MaskKeyType GetMaskKey()const noexcept { return m_stMaskKey; }
        void SetMaskKey(const MaskKeyType& key)noexcept { m_stMaskKey = key; }

        /**
         * @brief 获取或设置当WebSocket头部解析完毕时的回调
         */
        HeadersCompleteCallback GetHeadersCompleteCallback()const noexcept { return m_pHeadersCompleteCallback; }
        void SetHeadersCompleteCallback(const HeadersCompleteCallback& cb) { m_pHeadersCompleteCallback = cb; }

        /**
         * @brief 获取或设置当从WebSocket数据流中取得负载时的回调
         *
         * 注意：调用方需要自行解决数据流的解密操作。
         */
        DataCallback GetDataCallback()const noexcept { return m_pDataCallback; }
        void SetDataCallback(const DataCallback& cb) { m_pDataCallback = cb; }

        /**
         * @brief 获取或设置当一个WebSocket消息被处理完成后的回调
         */
        MessageCompleteCallback GetMessageCompleteCallback()const noexcept { return m_pMessageCompleteCallback; }
        void SetMessageCompleteCallback(const MessageCompleteCallback& cb) { m_pMessageCompleteCallback = cb; }

        /**
         * @brief 重置状态
         */
        void Reset()noexcept;

        /**
         * @brief 解析
         * @param input 输入数据
         */
        void Parse(BytesView input)
        {
            try
            {
                ParseImpl(input);
            }
            catch (...)
            {
                Reset();
                throw;
            }
        }

        /**
         * @brief 序列化追加到
         * @param out 输出
         *
         * 仅序列化非正文部分。
         */
        void SerializeTo(std::string& out)const;

        /**
         * @brief 序列化
         */
        std::string ToString()const;

    private:
        void ParseImpl(BytesView input);

    private:
        // 协议属性
        bool m_bFin = false;
        ReservedDataType m_stReserves;
        uint8_t m_bOpCode = 0;
        bool m_bMask = false;
        uint64_t m_uPayloadLength = 0;
        MaskKeyType m_stMaskKey;

        // 解析器状态
        unsigned m_uState = 0;
        bool m_bPayload16 = false;
        bool m_bPayload64 = false;
        unsigned m_uBodyRead = 0;
        HeadersCompleteCallback m_pHeadersCompleteCallback;
        DataCallback m_pDataCallback;
        MessageCompleteCallback m_pMessageCompleteCallback;
    };
}
