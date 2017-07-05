/**
 * @file
 * @date 2017/7/5
 */
#pragma once
#include <string>
#include <unordered_map>

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
        inline uint32_t MPQHash(const ArrayView<uint8_t>& str, unsigned offset=0)
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
                : MPQHashKey(ArrayView<uint8_t>(reinterpret_cast<const uint8_t*>(key.data()), key.size())) {}

            MPQHashKey(const ArrayView<uint8_t>& raw)noexcept
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
    }
}
