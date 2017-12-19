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

        using TagType = uint16_t;

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
    };
}
