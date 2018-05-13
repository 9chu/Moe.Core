/**
 * @file
 * @author chu
 * @date 2018/3/9
 */
#pragma once
#include <array>
#include <string>

namespace moe
{
    /**
     * @brief URL编解码类
     * @see https://url.spec.whatwg.org
     */
    struct Url
    {
        /**
         * @brief Host解析类
         */
        class Host
        {
        public:
            /**
             * @brief Host值类型
             */
            enum class ValueTypes
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
            Host(Host&& rhs);
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
            ValueTypes GetType()const noexcept;

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

        private:
            bool ParseIpv4Part(const char* start, const char* end);
            bool ParseIpv6Part(const char* start, const char* end);
            void ParseOpaqueHost();

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

            ValueTypes m_iType = ValueTypes::None;
            ValueStorage m_stValue;
        };
    };
}
