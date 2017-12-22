/**
 * @file
 * @date 2017/12/19
 */
#pragma once
#include "Stream.hpp"

namespace moe
{
    /**
     * @brief Moe Data Represent
     *
     * - 二进制数据交换格式
     * - 以小端序为主
     */
    class Mdr
    {
    public:
        enum class WireTypes
        {
            Null,
            Fixed8 = 1,  // bool/char/byte
            Fixed32 = 2,  // float
            Fixed64 = 3,  // double
            Varint = 4,  // int
            Buffer = 5,  // string/bytes
            List = 6,  // vector<T>/array<T>
            Map = 7,  // unordered_map<K,V>/map<K,V>
            Structure = 8,  // struct

            MAX = 9,
        };

        using TagType = uint64_t;

        struct FieldHead
        {
            TagType Tag;
            WireTypes Type;
        };

        /*
         * VarInt编码举例：
         *   整数        0100|1111 000|01111 11|101111 1|0100001
         *   使用varint  [1]0100001 [1]1011111 [1]0111111 [1]1111000 [0]0000100
         *   其中，字节最高位被用来指示是否有后继字节。
         */

        /**
         * @brief 读取变长整数
         * @param stream 流
         * @return 值
         */
        static uint64_t ReadVarint(Stream* stream)
        {
            int b = 0;
            uint64_t ret = 0;
            uint32_t bits = 0;
            for (int i = 0; i < 10; ++i)
            {
                b = stream->ReadByte();
                if (b < 0)
                    MOE_THROW(OutOfRangeException, "EOF");
                ret |= (static_cast<uint64_t>(b & 0x7F) << bits);
                bits += 7;
                if ((b & 0x80) == 0)
                    break;
            }
            if ((b & 0x80) != 0 || (bits == 70 && b != 1))
                MOE_THROW(BadFormatException, "Varint is too big");
            return ret;
        }

        /**
         * @brief 跳过变长整数
         * @param stream 流
         */
        static void SkipVarint(Stream* stream)
        {
            int b = 0;
            do
            {
                b = stream->ReadByte();
                if (b < 0)
                    MOE_THROW(OutOfRangeException, "EOF");
            } while ((b & 0x80) != 0);
        }

        /**
         * @brief 写入变长整数
         * @param stream 流
         * @param value 值
         */
        static void WriteVarint(Stream* stream, uint64_t value)
        {
            unsigned pos = 0;
            uint8_t bytes[10];
            do
            {
                assert(pos < sizeof(bytes));
                auto b = static_cast<unsigned char>(value & 0x7F);
                value >>= 7;
                b |= (value > 0 ? 0x80 : 0);
                bytes[++pos] = b;
            } while (value > 0);
            stream->Write(BytesView(bytes, pos), pos);
        }

        static uint64_t Zigzag(int64_t n)noexcept
        {
            return static_cast<uint64_t>(n << 1) ^ static_cast<uint64_t>(n >> 63);
        }

        static int64_t DeZigzag(uint64_t n)noexcept
        {
            return static_cast<int64_t>(n >> 1) ^ -(static_cast<int64_t>(n) & 1);
        }

        class Reader
        {
        public:
            Reader()noexcept = default;
            Reader(const Reader&)noexcept = default;
            Reader(Reader&&)noexcept = default;

            Reader(Stream* stream)noexcept
                : m_pStream(stream) {}

        public:

        private:
            FieldHead ReadHead()
            {
                assert(m_pStream);
                FieldHead ret;
                auto h = m_pStream->ReadByte();
                if (h < 0)
                    MOE_THROW(OutOfRangeException, "Eof");
                auto t = static_cast<TagType>(h & 0xF0) >> 4;
                auto tt = static_cast<uint32_t>(h & 0x0F);
                if (tt >= static_cast<uint32_t>(WireTypes::MAX))
                    MOE_THROW(BadFormatException, "Unknown wire type {0}", tt);
                if (t == 0xF)
                    t = Mdr::ReadVarint(m_pStream) + 0xF;
                ret.Tag = t;
                ret.Type = static_cast<WireTypes>(tt);
                return ret;
            }

            void ReadFixed8(uint8_t* out)
            {
                assert(m_pStream);
                auto ret = m_pStream->ReadByte();
                if (ret < 0)
                    MOE_THROW(OutOfRangeException, "Eof");
                if (out)
                    *out = static_cast<uint8_t>(ret);
            }

            void ReadFixed32(uint32_t* out)
            {
                assert(m_pStream);
                uint8_t buffer[4];
                if (m_pStream->Read(MutableBytesView(buffer, 4), 4) != 4)
                    MOE_THROW(OutOfRangeException, "Eof");
                if (out)
                    *out = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
            }

            void ReadFixed64(uint64_t* out)
            {
                assert(m_pStream);
                uint8_t buffer[8];
                if (m_pStream->Read(MutableBytesView(buffer, 8), 8) != 8)
                    MOE_THROW(OutOfRangeException, "Eof");
                if (out)
                {
                    *out = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24) |
                        ((uint64_t)buffer[4] << 32) | ((uint64_t)buffer[5] << 40) | ((uint64_t)buffer[6] << 48) |
                        ((uint64_t)buffer[7] << 56);
                }
            }

            void ReadVarint(uint64_t* out)
            {
                assert(m_pStream);
                auto ret = Mdr::ReadVarint(m_pStream);
                if (out)
                    *out = ret;
            }

            void ReadBuffer(std::string* out)
            {
                assert(m_pStream);
                auto len = static_cast<size_t>(Mdr::ReadVarint(m_pStream));
                if (out)
                {
                    if (len == 0)
                        out->clear();
                    else
                    {
                        out->resize(len);
                        if (m_pStream->Read(MutableBytesView(reinterpret_cast<uint8_t*>(&out[0]), out->length()), len)
                            != len)
                        {
                            MOE_THROW(OutOfRangeException, "Eof");
                        }
                    }
                }
                else
                    m_pStream->Skip(len);
            }

            /*
            template <typename Container>
            void ReadList(Container* container)
            {
                assert(m_pStream);
                auto count = static_cast<size_t>(Mdr::ReadVarint(m_pStream));

                if (container)
                {
                    container->clear();
                    container->reserve(count);

                    for (size_t i = 0; i < count; ++i)
                    {
                        Container::value_type v;
                        Read(v, 0);
                        container->emplace_back(std::move(v));
                    }
                }
                else
                {
                    for (size_t i = 0; i < count; ++i)
                        Skip(0);
                }
            }

            template <typename Container>
            void ReadDict(Container* container)
            {
                assert(m_pStream);
                auto count = static_cast<size_t>(Mdr::ReadVarint(m_pStream));

                if (container)
                {
                    container->clear();
                    container->reserve(count);
                    for (size_t i = 0; i < count; ++i)
                    {
                        Container::key_type k;
                        Container::value_type v;
                        Read(k, 0);
                        Read(v, 1);
                        auto ret = container->emplace(std::move(k), std::move(v));
                        if (!ret.second)
                            MOE_THROW(BadFormatException, "Duplicated key \"{0}\"", k);
                    }
                }
                else
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        Skip(0);
                        Skip(1);
                    }
                }
            }
             */

        private:
            Stream* m_pStream = nullptr;
        };
    };
}
