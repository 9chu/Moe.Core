/**
 * @file
 * @date 2017/7/5
 */
#pragma once
#include <string>
#include <unordered_map>
#include <limits>

#include "ArrayView.hpp"

namespace moe
{
    namespace Hasher
    {
        //////////////////////////////////////// <editor-fold desc="MPQHash">

        /**
         * @brief 获取暴雪所用Hash算法的加密表
         * @return 返回大小为0x500的加密表
         */
        const uint32_t* GetMPQCryptTable()noexcept;

        /**
         * @brief 暴雪所用One-Way Hash算法
         * @param str 输入串
         * @param offset 偏移，可取0,1,2,3,4
         * @return Hash值
         */
        inline uint32_t MPQHash(const BytesView& str, unsigned offset=0)
        {
            assert(offset < 5);

            const uint32_t* cryptTable = GetMPQCryptTable();
            uint32_t seed1 = 0x7FED7FEDu, seed2 = 0xEEEEEEEEu;

            for (size_t i = 0; i < str.GetSize(); ++i)
            {
                auto ch = str[i];

                seed1 = cryptTable[(offset << 8) + ch] ^ (seed1 + seed2);
                seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
            }

            return seed1;
        }

        struct MPQHashKey
        {
            uint32_t Key;
            uint32_t HashA;
            uint32_t HashB;

            MPQHashKey()noexcept
                : Key(0), HashA(0), HashB(0) {}

            MPQHashKey(const std::string& key)noexcept
                : MPQHashKey(BytesView(reinterpret_cast<const uint8_t*>(key.data()), key.size())) {}

            MPQHashKey(const BytesView& raw)noexcept
                : Key(MPQHash(raw, 0)), HashA(MPQHash(raw, 1)), HashB(MPQHash(raw, 2)) {}

            MPQHashKey(const MPQHashKey&) = default;
        };

        namespace details
        {
            struct MPQHasher
            {
                uint32_t operator()(const MPQHashKey& x)const noexcept { return x.Key; }
            };
        }

        template <typename T>
        using MPQHashMap = std::unordered_map<MPQHashKey, T, details::MPQHasher>;

        //////////////////////////////////////// </editor-fold>

        //////////////////////////////////////// <editor-fold desc="MurmurHash">

        /**
         * @brief Murmur哈希函数(二代)
         * @see https://github.com/aappleby/smhasher
         * @param data 输入数据
         * @param len 长度
         * @param seed 种子
         * @return 哈希结果
         *
         * 方法用于生成64位的哈希值。
         * 该方法平台无关，在不同大小端机器上能得出相同结果。
         */
        inline uint64_t MurmurHash2(const uint8_t* data, size_t len, uint64_t seed)noexcept
        {
            const uint64_t m = 0xC6A4A7935BD1E995ull;
            const int r = 47;

            uint64_t h = seed ^ (len * m);

            while (len >= 8)
            {
                uint64_t k;

                k = uint64_t(data[0]);
                k |= uint64_t(data[1]) << 8;
                k |= uint64_t(data[2]) << 16;
                k |= uint64_t(data[3]) << 24;
                k |= uint64_t(data[4]) << 32;
                k |= uint64_t(data[5]) << 40;
                k |= uint64_t(data[6]) << 48;
                k |= uint64_t(data[7]) << 56;

                k *= m;
                k ^= k >> r;
                k *= m;

                h ^= k;
                h *= m;

                data += 8;
                len -= 8;
            }

            switch (len)
            {
                case 7:
                    h ^= uint64_t(data[6]) << 48;
                case 6:
                    h ^= uint64_t(data[5]) << 40;
                case 5:
                    h ^= uint64_t(data[4]) << 32;
                case 4:
                    h ^= uint64_t(data[3]) << 24;
                case 3:
                    h ^= uint64_t(data[2]) << 16;
                case 2:
                    h ^= uint64_t(data[1]) << 8;
                case 1:
                    h ^= uint64_t(data[0]);
                    h *= m;
                    break;
                default:
                    break;
            };

            h ^= h >> r;
            h *= m;
            h ^= h >> r;

            return h;
        }

        //////////////////////////////////////// </editor-fold>

        //////////////////////////////////////// <editor-fold desc="MD5">

        namespace details
        {
            struct MD5Context
            {
                uint32_t lo, hi;
                uint32_t a, b, c, d;

                uint8_t Buffer[64];
            };

            void MD5Init(MD5Context* context)noexcept;
            void MD5Update(MD5Context* context, const uint8_t* data, uint32_t size)noexcept;

            /**
             * @brief 计算MD5最终结果
             * @param ctx 上下文
             * @param result 结果，必须为16字节
             *
             * 调用方法后必须使用MD5Init来重置状态。
             */
            void MD5Final(MD5Context* context, uint8_t result[])noexcept;
        }

        /**
         * @brief 计算MD5
         * @tparam Size 输出缓冲区大小
         * @param out 输出缓冲区，必须为16字节
         * @param data 输入数据
         * @param len 输入长度
         * @return 输出缓冲区
         */
        template <size_t Size>
        inline BytesView MD5(uint8_t (&out)[Size], const uint8_t* data, size_t len)noexcept
        {
            static_assert(Size >= 16, "Bad buffer size");
            assert(len <= std::numeric_limits<uint32_t>::max());

            details::MD5Context context;
            details::MD5Init(&context);
            details::MD5Update(&context, data, static_cast<uint32_t>(len));
            details::MD5Final(&context, out);
            return BytesView(out, 16);
        }

        /**
         * @brief 计算字符串MD5
         * @tparam Size 输出缓冲区大小
         * @param out 输出缓冲区，必须为16字节
         * @param data 输入字符串
         * @return 输出缓冲区
         */
        template <size_t Size>
        inline BytesView MD5(uint8_t (&out)[Size], const std::string& data)noexcept
        {
            static_assert(Size >= 16, "Bad buffer size");
            return MD5(out, reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
        }

        //////////////////////////////////////// </editor-fold>
    }
}
