/**
 * @file
 * @date 2017/7/5
 */
#pragma once
#include <array>
#include <string>
#include <limits>
#include <unordered_map>

#include "ArrayView.hpp"

namespace moe
{
    namespace Hasher
    {
        /**
         * Hash对象需要满足
         *
         * interface T
         * {
         *     using ResultType;
         *
         *     T& Reset()noexcept;
         *     T& Update(BytesView)noexcept;
         *     ResultType Final()noexcept;
         * };
         *
         * 当执行Final后，将不能再执行Update方法。并且Final方法可以重入并返回同一结果。
         * 当且仅当执行Reset后可以继续Update。
         */

        namespace details
        {
            /**
             * @breif 获取MpqHash内部所用的加密表(0x500大小)
             */
            const uint32_t* GetMpqCryptTable()noexcept;

            /**
             * @brief 获取Crc32内部所用的表（256大小）
             */
            const uint32_t* GetCrc32Table()noexcept;
        }

        /**
         * @brief MPQ哈希算法
         * @tparam Offset Offset，可取值0,1,2,3,4
         */
        template <uint32_t Offset>
        class Mpq
        {
            static_assert(Offset < 5, "Offset must be in 0~4");

            enum STATE
            {
                STATE_DEFAULT = 0,
                STATE_FINISHED = 1,
            };

        public:
            using ResultType = uint32_t;

        public:
            Mpq()noexcept { Reset(); }

            /**
             * @brief 重置内部状态
             */
            Mpq& Reset()noexcept
            {
                m_iState = STATE_DEFAULT;
                m_uSeed1 = 0x7FED7FEDu;
                m_uSeed2 = 0xEEEEEEEEu;
                return *this;
            }

            /**
             * @brief 更新内部状态
             * @param input 输入数据
             */
            Mpq& Update(BytesView input)noexcept
            {
                assert(m_iState == STATE_DEFAULT);

                const uint32_t* table = details::GetMpqCryptTable();
                for (size_t i = 0; i < input.GetSize(); ++i)
                {
                    auto b = input[i];
                    m_uSeed1 = table[(Offset << 8u) + b] ^ (m_uSeed1 + m_uSeed2);
                    m_uSeed2 = b + m_uSeed1 + m_uSeed2 + (m_uSeed2 << 5u) + 3u;
                }
                return *this;
            }

            /**
             * @brief 计算最终输出
             */
            ResultType Final()noexcept
            {
                m_iState = STATE_FINISHED;
                return m_uSeed1;
            }

        private:
            STATE m_iState = STATE_DEFAULT;
            uint32_t m_uSeed1 = 0;
            uint32_t m_uSeed2 = 0;
        };

        /**
         * @brief Time33哈希算法
         */
        template <uint32_t Seed=5381>
        class Time33
        {
            enum STATE
            {
                STATE_DEFAULT = 0,
                STATE_FINISHED = 1,
            };

        public:
            using ResultType = uint32_t;

        public:
            Time33()noexcept { Reset(); }

            /**
             * @brief 重置内部状态
             */
            Time33& Reset()noexcept
            {
                m_iState = STATE_DEFAULT;
                m_uHash = Seed;
                return *this;
            }

            /**
             * @brief 更新内部状态
             * @param input 输入数据
             */
            Time33& Update(BytesView input)noexcept
            {
                assert(m_iState == STATE_DEFAULT);

                for (size_t i = 0; i < input.GetSize(); ++i)
                {
                    auto b = input[i];
                    m_uHash += (m_uHash << 5u) + b;
                }
                return *this;
            }

            /**
             * @brief 计算最终输出
             */
            ResultType Final()noexcept
            {
                if (m_iState == STATE_FINISHED)
                    return m_uHash;

                m_iState = STATE_FINISHED;
                m_uHash &= 0x7FFFFFFF;
                return m_uHash;
            }

        private:
            STATE m_iState = STATE_DEFAULT;
            uint32_t m_uHash = 0;
        };

        /**
         * @brief Murmur哈希函数
         * @tparam Seed 种子值
         */
        template <uint32_t Seed>
        class Murmur3
        {
            static_assert(sizeof(uint32_t) == 4, "Bad condition");

            enum STATE
            {
                STATE_DEFAULT = 0,
                STATE_FINISHED = 1,
            };

        public:
            using ResultType = uint64_t;

        public:
            Murmur3()noexcept { Reset(); }

            /**
             * @brief 重置内部状态
             */
            Murmur3& Reset()noexcept
            {
                m_iState = STATE_DEFAULT;
                m_uH1 = Seed;
                m_uRest = 0;
                m_uLength = 0;
                m_stBuf.fill(0);
                return *this;
            }

            /**
             * @brief 更新内部状态
             * @param input 输入数据
             */
            Murmur3& Update(BytesView input)noexcept
            {
                assert(m_iState == STATE_DEFAULT);

                static const uint32_t c1 = 0xcc9e2d51;
                static const uint32_t c2 = 0x1b873593;

                auto blocks = (m_uRest + input.GetSize()) / sizeof(uint32_t);
                auto rest = (m_uRest + input.GetSize()) % sizeof(uint32_t);

                for (uint32_t i = 0; i < blocks; ++i)
                {
                    uint32_t k1 = 0;

                    if (i == 0 && m_uRest > 0)
                    {
                        for (uint32_t j = 0, k = 0; j < sizeof(uint32_t); ++j, k += 8)
                        {
                            if (j < m_uRest)
                                k1 |= (m_stBuf[j] << k);
                            else
                                k1 |= (input[j - m_uRest] << k);
                        }
                    }
                    else
                    {
                        for (uint32_t j = 0, k = 0; j < sizeof(uint32_t); ++j, k += 8)
                            k1 |= (input[i * sizeof(uint32_t) + j - m_uRest] << k);
                    }

                    k1 *= c1;
                    k1 = (k1 << 15u) | (k1 >> (32u - 15u));
                    k1 *= c2;

                    m_uH1 ^= k1;
                    m_uH1 = (m_uH1 << 13u) | (m_uH1 >> (32u - 13u));
                    m_uH1 = m_uH1 * 5 + 0xE6546B64u;
                }

                // 复制末尾未能成block的结果
                if (blocks > 0)
                    m_uRest = 0;
                for (uint32_t i = 0; i < rest; ++i)
                    m_stBuf[m_uRest + i] = input[input.GetSize() - rest + i];
                m_uRest += rest;
                assert(m_uRest < 4);

                m_uLength += input.GetSize();
                return *this;
            }

            /**
             * @brief 计算最终输出
             */
            ResultType Final()noexcept
            {
                if (m_iState == STATE_FINISHED)
                    return m_uH1;

                static const uint32_t c1 = 0xcc9e2d51;
                static const uint32_t c2 = 0x1b873593;

                m_iState = STATE_FINISHED;

                uint32_t k1 = 0;
                switch (m_uRest)
                {
                    case 3:
                        k1 ^= (m_stBuf[2] << 16);
                    case 2:
                        k1 ^= (m_stBuf[1] << 8);
                    case 1:
                        k1 ^= m_stBuf[0];
                        k1 *= c1;
                        k1 = (k1 << 15u) | (k1 >> (32u - 15u));
                        k1 *= c2;
                        m_uH1 ^= k1;
                    default:
                        break;
                };

                m_uH1 ^= m_uLength;

                // Final mix
                m_uH1 ^= m_uH1 >> 16u;
                m_uH1 *= 0x85EBCA6Bu;
                m_uH1 ^= m_uH1 >> 13u;
                m_uH1 *= 0xC2B2AE35u;
                m_uH1 ^= m_uH1 >> 16u;
                return m_uH1;
            }

        private:
            STATE m_iState = STATE_DEFAULT;
            uint32_t m_uH1 = 0;
            uint32_t m_uRest = 0;
            uint32_t m_uLength = 0;
            std::array<uint8_t, sizeof(uint32_t)> m_stBuf;
        };

        /**
         * @brief Crc32校验
         */
        class Crc32
        {
            enum STATE
            {
                STATE_DEFAULT = 0,
                STATE_FINISHED = 1,
            };

        public:
            using ResultType = uint32_t;

        public:
            Crc32()noexcept { Reset(); }

            /**
             * @brief 重置内部状态
             */
            Crc32& Reset()noexcept
            {
                m_iState = STATE_DEFAULT;
                m_uCrc32 = 0u ^ 0xFFFFFFFFu;
                return *this;
            }

            /**
             * @brief 更新内部状态
             * @param input 输入数据
             */
            Crc32& Update(BytesView input)noexcept
            {
                assert(m_iState == STATE_DEFAULT);

                const uint32_t* table = details::GetCrc32Table();
                for (size_t i = 0; i < input.GetSize(); ++i)
                    m_uCrc32 = (m_uCrc32 >> 8u) ^ table[(m_uCrc32 ^ input[i]) & 0xFFu];
                return *this;
            }

            /**
             * @brief 计算最终输出
             */
            ResultType Final()noexcept
            {
                if (m_iState == STATE_FINISHED)
                    return m_uCrc32;

                m_iState = STATE_FINISHED;
                m_uCrc32 ^= 0xFFFFFFFFu;
                return m_uCrc32;
            }

        private:
            STATE m_iState = STATE_DEFAULT;
            uint32_t m_uCrc32 = 0;
        };

        /**
         * @brief MD5计算
         */
        class Md5
        {
            enum STATE
            {
                STATE_DEFAULT = 0,
                STATE_FINISHED = 1,
            };

        public:
            using ResultType = std::array<uint8_t, 16>;

        public:
            Md5()noexcept { Reset(); }

            /**
             * @brief 重置内部状态
             */
            Md5& Reset()noexcept
            {
                m_iState = STATE_DEFAULT;
                m_uLo = m_uHi = 0;
                m_uA = 0x67452301u;
                m_uB = 0xeFCDAB89u;
                m_uC = 0x98BADCFEu;
                m_uD = 0x10325476u;
                m_stBuffer.fill(0);
                m_stResult.fill(0);
                return *this;
            }

            /**
             * @brief 更新内部状态
             * @param input 输入数据
             */
            Md5& Update(BytesView input)noexcept;

            /**
             * @brief 计算最终输出
             */
            const ResultType& Final()noexcept;

        private:
            const uint8_t* Transform(const uint8_t* data, size_t length)noexcept;

        private:
            STATE m_iState = STATE_DEFAULT;
            uint32_t m_uLo = 0, m_uHi = 0, m_uA = 0, m_uB = 0, m_uC = 0, m_uD = 0;
            std::array<uint8_t, 64> m_stBuffer;
            ResultType m_stResult;
        };

        /**
         * @brief SHA1计算
         */
        class Sha1
        {
            enum STATE
            {
                STATE_DEFAULT = 0,
                STATE_FINISHED = 1,
            };

        public:
            static const uint32_t kHashSize = 160 / 8;
            using ResultType = std::array<uint8_t, kHashSize>;

        public:
            Sha1()noexcept { Reset(); }

            /**
             * @brief 重置内部状态
             */
            Sha1& Reset()noexcept
            {
                m_iState = STATE_DEFAULT;
                m_uState[0] = 0x67452301u;
                m_uState[1] = 0xEFCDAB89u;
                m_uState[2] = 0x98BADCFEu;
                m_uState[3] = 0x10325476u;
                m_uState[4] = 0xC3D2E1F0u;
                m_uCount[0] = 0;
                m_uCount[1] = 0;
                m_stBuffer.fill(0);
                m_stResult.fill(0);
                return *this;
            }

            /**
             * @brief 更新内部状态
             * @param input 输入数据
             */
            Sha1& Update(BytesView input)noexcept;

            /**
             * @brief 计算最终输出
             */
            const ResultType& Final()noexcept;

        private:
            void Transform(const uint8_t buffer[64])noexcept;

        private:
            STATE m_iState = STATE_DEFAULT;
            std::array<uint32_t, 5> m_uState;
            std::array<uint32_t, 2> m_uCount;
            std::array<uint8_t, 64> m_stBuffer;
            ResultType m_stResult;
        };

        /**
         * @brief SHA256计算
         */
        class Sha256
        {
            enum STATE
            {
                STATE_DEFAULT = 0,
                STATE_FINISHED = 1,
            };

            static const uint32_t kBlockSize = 64;

        public:
            static const uint32_t kHashSize = 256 / 8;
            using ResultType = std::array<uint8_t, kHashSize>;

        public:
            Sha256()noexcept { Reset(); }

            /**
             * @brief 重置内部状态
             */
            Sha256& Reset()noexcept
            {
                m_iState = STATE_DEFAULT;
                m_uLength = m_uCurrent = 0;
                m_uState[0] = 0x6A09E667u;
                m_uState[1] = 0xBB67AE85u;
                m_uState[2] = 0x3C6EF372u;
                m_uState[3] = 0xA54FF53Au;
                m_uState[4] = 0x510E527Fu;
                m_uState[5] = 0x9B05688Cu;
                m_uState[6] = 0x1F83D9ABu;
                m_uState[7] = 0x5BE0CD19u;
                m_stBuffer.fill(0);
                m_stResult.fill(0);
                return *this;
            }

            /**
             * @brief 更新内部状态
             * @param input 输入数据
             */
            Sha256& Update(BytesView input)noexcept;

            /**
             * @brief 计算最终输出
             */
            const ResultType& Final()noexcept;

        private:
            void Transform(const uint8_t* buffer)noexcept;

        private:
            STATE m_iState = STATE_DEFAULT;
            uint64_t m_uLength = 0;
            std::array<uint32_t, 8> m_uState;
            uint32_t m_uCurrent = 0;
            std::array<uint8_t, 64> m_stBuffer;
            ResultType m_stResult;
        };
    }

    /**
     * @brief 计算Hash
     * @tparam Hasher 哈希对象
     * @param out 计算结果
     * @param input 输入
     * @return 计算结果的引用
     */
    template <typename Hasher>
    const typename Hasher::ResultType& ComputeHash(typename Hasher::ResultType& out, BytesView input)noexcept
    {
        Hasher h;
        h.Update(input);
        out = h.Final();
        return out;
    }
}
