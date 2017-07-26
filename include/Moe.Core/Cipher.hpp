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
         * @see https://en.wikipedia.org/wiki/RC4
         */
        class RC4
        {
        public:
            RC4(BytesView password)noexcept
            {
                // 初始化S盒
                for (uint8_t i = 0; i < 256; ++i)
                    m_aS[i] = i;

                // S盒初始置换
                for (size_t i = 0, j = 0; i < 256; ++i)
                {
                    j = (j + m_aS[i] + password[i % password.GetSize()]) % 256;
                    std::swap(m_aS[i], m_aS[j]);
                }
            }

        public:
            void operator()(BytesView buffer, MutableBytesView output)noexcept
            {
                assert(buffer.GetSize() <= output.GetSize());

                size_t& i = m_iI;
                size_t& j = m_iJ;
                for (size_t k = 0; k < buffer.GetSize(); ++k)
                {
                    // S盒置换
                    i = (i + 1) % 256;
                    j = (j + m_aS[i]) % 256;
                    std::swap(m_aS[i], m_aS[j]);
                    uint8_t n = m_aS[(m_aS[i] + m_aS[j]) % 256];

                    // 加解密
                    output[k] = buffer[k] ^ n;
                }
            }

        private:
            uint8_t m_aS[256];
            size_t m_iI = 0, m_iJ = 0;
        };
    }
}
