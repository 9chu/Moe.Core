/**
 * @file
 * @date 2017/5/28
 */
#pragma once
#include <cstring>
#include <algorithm>

#include "ArrayView.hpp"

namespace moe
{
    /**
     * @brief 加解密方法
     */
    namespace Cipher
    {
        /**
         * @brief RC4加解密
         */
        class RC4
        {
        private:
            uint8_t S[256];

        public:
            void operator()(ArrayView<uint8_t> buffer, MutableArrayView<uint8_t> output)noexcept
            {
                assert(buffer.GetSize() <= output.GetSize());

                uint8_t Scpy[256];
                ::memcpy(Scpy, S, sizeof(S));

                for (size_t i = 0, j = 0; i < buffer.GetSize(); ++i)
                {
                    // S盒置换
                    size_t i2 = (i + 1) % 256;
                    j = (j + Scpy[i2]) % 256;
                    std::swap(Scpy[i2], Scpy[j]);
                    uint8_t n = Scpy[(Scpy[i2] + Scpy[j]) % 256];

                    // 加解密
                    output[i] = buffer[i] ^ n;
                }
            }

        public:
            RC4(ArrayView<uint8_t> password)noexcept
            {
                size_t len = std::min(len, 256u);

                // 初始化S盒
                for (uint8_t i = 0; i < 256; ++i)
                    S[i] = i;

                // S盒初始置换
                for (size_t i = 0, j = 0; i < 256; i++)
                {
                    j = (j + S[i] + password[i % len]) % 256;
                    std::swap(S[i], S[j]);
                }
            }
        };
    }
}
