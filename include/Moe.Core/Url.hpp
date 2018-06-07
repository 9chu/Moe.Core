/**
 * @file
 * @author chu
 * @date 2018/3/9
 */
#pragma once
#include <array>
#include <string>
#include <vector>

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
             * @param text 输入文本
             * @param special 是否是特殊Host
             * @return 是否成功解析，若否，将保留对象原始数据。
             *
             * 当设置为special = true的时候才会进行IPV4和域名的解析，否则以透传形式给出。
             */
            bool Parse(const std::string& text, bool special=true);

            /**
             * @brief 解析Host
             * @param text 输入文本
             * @param length 文本长度
             * @param special 是否是特殊Host
             * @return 是否成功解析，若否，将保留对象原始数据。
             *
             * 当设置为special = true的时候才会进行IPV4和域名的解析，否则以透传形式给出。
             */
            bool Parse(const char* text, bool special=true);
            bool Parse(const char* text, size_t length, bool special=true);

            /**
             * @brief 序列化Host
             */
            std::string ToString()const;

        private:
            bool Parse(const char* start, const char* end, bool special);
            bool ParseIpv4(const char* start, const char* end)noexcept;
            bool ParseIpv6(const char* start, const char* end)noexcept;
            bool ParseOpaque(const char* start, const char* end);

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

        enum {
            FLAGS_FAILED = 0x01,
            FLAGS_CANNOT_BE_BASE = 0x02,
            FLAGS_INVALID_PARSE_STATE = 0x04,
            FLAGS_TERMINATED = 0x08,
            FLAGS_SPECIAL = 0x10,
            FLAGS_HAS_USERNAME = 0x20,
            FLAGS_HAS_PASSWORD = 0x40,
            FLAGS_HAS_HOST = 0x80,
            FLAGS_HAS_PATH = 0x100,
            FLAGS_HAS_QUERY = 0x200,
            FLAGS_HAS_FRAGMENT = 0x400,
        };

    public:


    private:
        uint32_t m_uFlags = 0;
        std::string m_stScheme;
        std::string m_stUsername;
        std::string m_stPassword;
        std::string m_stHost;
        uint16_t m_uPort = 0;
        std::string m_stQuery;
        std::string m_stFragment;
        std::vector<std::string> m_stPath;
    };
}
