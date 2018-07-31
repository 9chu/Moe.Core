/**
 * @file
 * @author chu
 * @date 2018/3/9
 */
#pragma once
#include <array>
#include <string>
#include <vector>
#include <cstring>

#include "ArrayView.hpp"

namespace moe
{
    /**
     * @brief URL编解码类
     * @see https://url.spec.whatwg.org
     */
    class Url
    {
    public:
        /**
         * @brief Host解析类
         */
        class Host
        {
        public:
            /**
             * @brief Host值类型
             */
            enum class HostTypes
            {
                None,
                Domain,
                Ipv4,
                Ipv6,
                Opaque,
            };

            /**
             * @brief Ipv6地址类型
             */
            using Ipv6AddressType = std::array<uint16_t, 8>;

        public:
            Host();
            Host(const Host& rhs);
            Host(Host&& rhs)noexcept;
            ~Host();

            Host& operator=(const Host& rhs);
            Host& operator=(Host&& rhs)noexcept;

            bool operator==(const Host& rhs)const noexcept;
            bool operator!=(const Host& rhs)const noexcept;

            operator bool()const noexcept;

        public:
            /**
             * @brief 获取地址类型
             */
            HostTypes GetType()const noexcept;

            /**
             * @brief 获取或设置域名
             */
            const std::string& GetDomain()const noexcept;
            void SetDomain(const std::string& value);
            void SetDomain(std::string&& value)noexcept;

            /**
             * @brief 获取或设置IPV4地址
             */
            uint32_t GetIpv4()const noexcept;
            void SetIpv4(uint32_t value)noexcept;

            /**
             * @brief 获取或设置IPV6地址
             */
            const Ipv6AddressType& GetIpv6()const noexcept;
            void SetIpv6(const Ipv6AddressType& value)noexcept;

            /**
             * @brief 获取或设置透传地址
             */
            const std::string& GetOpaque()const noexcept;
            void SetOpaque(const std::string& value);
            void SetOpaque(std::string&& value)noexcept;

            /**
             * @brief 重置为空值
             */
            void Reset();

            /**
             * @brief 解析Host
             * @exception BadFormatException 解析失败
             * @param text 输入文本
             * @param special 是否是特殊Host
             * @param unicode 是否进行Unicode/IDNA字符转换
             *
             * 当设置为special = true的时候才会进行IPV4和域名的解析，否则以透传形式给出。
             */
            void Parse(const std::string& text, bool special=true, bool unicode=false);

            /**
             * @brief 解析Host
             * @exception BadFormatException 解析失败
             * @param text 输入文本
             * @param length 文本长度
             * @param special 是否是特殊Host
             * @param unicode 是否进行Unicode/IDNA字符转换
             *
             * 当设置为special = true的时候才会进行IPV4和域名的解析，否则以透传形式给出。
             */
            void Parse(const char* text, bool special=true, bool unicode=false);
            void Parse(const char* text, size_t length, bool special=true, bool unicode=false);

            /**
             * @brief 序列化Host
             */
            std::string ToString()const;

        private:
            static bool IsAsciiFastPath(const std::string& domain)noexcept;
            void Parse(const char* start, const char* end, bool special, bool unicode);
            bool ParseIpv4(const char* start, const char* end);
            bool ParseIpv6(const char* start, const char* end)noexcept;
            void ParseOpaque(const char* start, const char* end);

        private:
            union ValueStorage
            {
                std::string Domain;
                uint32_t Ipv4;
                Ipv6AddressType Ipv6;
                std::string Opaque;

                ValueStorage() {}
                ~ValueStorage() {}
            };

            HostTypes m_iType = HostTypes::None;
            ValueStorage m_stValue;
        };

        enum URL_FLAGS
        {
            FLAGS_SPECIAL = 0x01,
            FLAGS_CANNOT_BE_BASE = 0x02,
            FLAGS_HAS_USERNAME = 0x04,
            FLAGS_HAS_PASSWORD = 0x08,
            FLAGS_HAS_HOST = 0x10,
            FLAGS_HAS_PORT = 0x20,
            FLAGS_HAS_PATH = 0x40,
            FLAGS_HAS_QUERY = 0x80,
            FLAGS_HAS_FRAGMENT = 0x100,
        };


    public:
        Url() = default;

        /**
         * @brief 构造URL对象
         * @param url URL字符串
         * @param base 基URL字符串
         */
        Url(ArrayView<char> url, ArrayView<char> base=ArrayView<char>());

        Url(const std::string& url)
            : Url(ArrayView<char>(url.data(), url.length()), ArrayView<char>())
        {}

        Url(const char* url, const char* base=nullptr)
            : Url(ArrayView<char>(url, std::char_traits<char>::length(url)),
                base ? ArrayView<char>(base, std::char_traits<char>::length(base)) : ArrayView<char>())
        {}

        Url(const Url& rhs) = default;
        Url(Url&& rhs)noexcept = default;
        ~Url() = default;

        Url& operator=(const Url& rhs) = default;
        Url& operator=(Url&& rhs) = default;

        bool operator==(const Url& rhs)const noexcept;
        bool operator!=(const Url& rhs)const noexcept;

    public:
        bool IsSpecial()const noexcept { return (m_uFlags & FLAGS_SPECIAL) != 0; }
        bool IsCannotBeBase()const noexcept { return (m_uFlags & FLAGS_CANNOT_BE_BASE) != 0; }

        const std::string& GetScheme()const noexcept { return m_stScheme; }
        void SetScheme(ArrayView<char> value);
        void SetScheme(const char* value) { SetScheme(ArrayView<char>(value, std::strlen(value))); }
        void SetScheme(const std::string& value) { SetScheme(ArrayView<char>(value.data(), value.length())); }

        bool HasUsername()const noexcept;
        const std::string& GetUsername()const noexcept;
        void SetUsername(ArrayView<char> value);
        void SetUsername(const char* value) { SetUsername(ArrayView<char>(value, std::strlen(value))); }
        void SetUsername(const std::string& value) { SetUsername(ArrayView<char>(value.data(), value.length())); }

        bool HasPassword()const noexcept;
        const std::string& GetPassword()const noexcept;
        void SetPassword(ArrayView<char> value);
        void SetPassword(const char* value) { SetPassword(ArrayView<char>(value, std::strlen(value))); }
        void SetPassword(const std::string& value) { SetPassword(ArrayView<char>(value.data(), value.length())); }

        bool HasHost()const noexcept;
        const Host& GetHost()const noexcept;
        void SetHost(const Host& host);
        void SetHost(Host&& host);
        void SetHost(ArrayView<char> value);
        void SetHost(const char* value) { SetHost(ArrayView<char>(value, std::strlen(value))); }
        void SetHost(const std::string& value) { SetHost(ArrayView<char>(value.data(), value.length())); }

        bool HasPort()const noexcept;
        uint16_t GetPort()const noexcept;
        void SetPort(uint16_t value);

        bool HasPath()const noexcept;
        const std::vector<std::string>& GetPath()const noexcept;
        void SetPath(ArrayView<char> value);
        void SetPath(const char* value) { SetPath(ArrayView<char>(value, std::strlen(value))); }
        void SetPath(const std::string& value) { SetPath(ArrayView<char>(value.data(), value.length())); }

        bool HasQuery()const noexcept;
        const std::string& GetQuery()const noexcept;
        void SetQuery(ArrayView<char> value);
        void SetQuery(const char* value) { SetQuery(ArrayView<char>(value, std::strlen(value))); }
        void SetQuery(const std::string& value) { SetQuery(ArrayView<char>(value.data(), value.length())); }

        bool HasFragment()const noexcept;
        const std::string& GetFragment()const noexcept;
        void SetFragment(ArrayView<char> value);
        void SetFragment(const char* value) { SetFragment(ArrayView<char>(value, std::strlen(value))); }
        void SetFragment(const std::string& value) { SetFragment(ArrayView<char>(value.data(), value.length())); }

        /**
         * @brief 若不存在Port则返回空串，否则返回Port
         */
        std::string GetPortStandard()const;

        /**
         * @brief 遵循标准的做法，返回字符串构成的路径
         */
        std::string GetPathStandard()const;

        /**
         * @brief 遵循标准的做法，始终返回"?xxx"
         *
         * 当且仅当不包含Query时返回空串。
         */
        std::string GetQueryStandard()const;

        /**
         * @brief 遵循标准的做法，始终返回"#xxx"
         *
         * 当且仅当不包含Fragment时返回空串。
         */
        std::string GetFragmentStandard()const;

        /**
         * @brief 解析URL
         * @exception BadFormatException 当格式错误时抛出该异常
         * @param src 被解析URL
         * @param base 基URL
         * @param trimWhitespace 解析时是否自动剔除首尾空白
         */
        void Parse(ArrayView<char> src, Url* base=nullptr, bool trimWhitespace=true);

        void Parse(const char* src, Url* base=nullptr, bool trimWhitespace=true)
        {
            Parse(ArrayView<char>(src, std::char_traits<char>::length(src)), base, trimWhitespace);
        }

        void Parse(const std::string& src, Url* base=nullptr, bool trimWhitespace=true)
        {
            Parse(ArrayView<char>(src.data(), src.length()), base, trimWhitespace);
        }

        /**
         * @brief 清空结构体
         */
        void Reset()noexcept;

        /**
         * @brief 序列化Url
         * @param excludeFragmentFlag 不包含分片
         */
        std::string ToString(bool excludeFragmentFlag=false)const;

    private:
        enum URL_PARSE_STATES
        {
            PARSE_STATE_UNKNOWN = 0,
            PARSE_STATE_SCHEME_START,
            PARSE_STATE_SCHEME,
            PARSE_STATE_NO_SCHEME,
            PARSE_STATE_SPECIAL_RELATIVE_OR_AUTHORITY,
            PARSE_STATE_PATH_OR_AUTHORITY,
            PARSE_STATE_RELATIVE,
            PARSE_STATE_RELATIVE_SLASH,
            PARSE_STATE_SPECIAL_AUTHORITY_SLASHES,
            PARSE_STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES,
            PARSE_STATE_AUTHORITY,
            PARSE_STATE_HOST,
            PARSE_STATE_HOSTNAME,
            PARSE_STATE_PORT,
            PARSE_STATE_FILE,
            PARSE_STATE_FILE_SLASH,
            PARSE_STATE_FILE_HOST,
            PARSE_STATE_PATH_START,
            PARSE_STATE_PATH,
            PARSE_STATE_CANNOT_BE_BASE,
            PARSE_STATE_QUERY,
            PARSE_STATE_FRAGMENT,
        };

        void NormalizePort()noexcept;
        void ShortenUrlPath()noexcept;
        void Parse(ArrayView<char> input, Url* base, URL_PARSE_STATES stateOverride, bool trimWhitespace);

    private:
        uint32_t m_uFlags = 0;
        std::string m_stScheme;
        std::string m_stUsername;
        std::string m_stPassword;
        Host m_stHost;
        uint16_t m_uPort = 0;
        std::string m_stQuery;
        std::string m_stFragment;
        std::vector<std::string> m_stPath;
    };
}
