/**
 * @file
 * @date 2017/10/12
 *
 * MBP: Moe Binary Data-exchange Protocol
 */
#pragma once
#include "Stream.hpp"

namespace moe
{
    enum class MbpWireTypes
    {
        Null,
        Fixed8 = 1,  // bool/char/byte
        Fixed32 = 2,  // float
        Fixed64 = 3,  // double
        Varint = 4,  // int
        Buffer = 5,  // string/bytes
        List = 6,  // vector<T>/array<T>
        Map = 7,  // unordered_map<K,V>/map<K,V>
        Struct = 8,  // struct

        MAX = 9,
    };

    using MbpTag = uint16_t;

    struct MbpStruct
    {
    };

    class MbpReader
    {
    public:
        static uint8_t ReadFixed8(Stream* stream)
        {
            assert(stream);
            int ret;
            if ((ret = stream->ReadByte()) < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            return static_cast<uint8_t>(ret);
        }

        static uint32_t ReadFixed32(Stream* stream)
        {
            assert(stream);
            uint8_t bytes[4];
            if (stream->Read(MutableBytesView(bytes, sizeof(bytes)), sizeof(bytes)) < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            return bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
        }

        static uint64_t ReadFixed64(Stream* stream)
        {
            assert(stream);
            uint8_t bytes[8];
            if (stream->Read(MutableBytesView(bytes, sizeof(bytes)), sizeof(bytes)) < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            return bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | ((uint64_t)bytes[3] << 24) |
                ((uint64_t)bytes[4] << 32) | ((uint64_t)bytes[5] << 40) | ((uint64_t)bytes[6] << 48) |
                ((uint64_t)bytes[7] << 56);
        }

        static uint64_t ReadVarint(Stream* stream)
        {
            /*
             * VarInt编码举例：
             *   整数        0100|1111 000|01111 11|101111 1|0100001
             *   使用varint  [1]0100001 [1]1011111 [1]0111111 [1]1111000 [0]0000100
             *   其中，字节最高位被用来指示是否有后继字节。
             *
             *   考虑到对于uint64类型，如果严格按照这一方式编码需要 ceil(64/7) = 10 个字节
             *   然而 64 = 8 * 7 + 8，故varint在最后一个字节无需使用最高位作为后继字节指示
             *   这样编码uint64至多需要9个字节
             */

            assert(stream);
            int b = stream->ReadByte();  // 1
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            if ((b & 0x80) == 0)
                return static_cast<uint64_t>(b);

            uint64_t value = static_cast<uint64_t>(b & 0x7F);
            b = stream->ReadByte();  // 2
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            value |= (b & 0x7F) << 7;
            if ((b & 0x80) == 0)
                return value;

            b = stream->ReadByte();  // 3
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            value |= (b & 0x7F) << 14;
            if ((b & 0x80) == 0)
                return value;

            b = stream->ReadByte();  // 4
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            value |= (b & 0x7F) << 21;
            if ((b & 0x80) == 0)
                return value;

            b = stream->ReadByte();  // 5
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            value |= static_cast<uint64_t>(b & 0x7F) << 28;
            if ((b & 0x80) == 0)
                return value;

            b = stream->ReadByte();  // 6
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            value |= static_cast<uint64_t>(b & 0x7F) << 35;
            if ((b & 0x80) == 0)
                return value;

            b = stream->ReadByte();  // 7
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            value |= static_cast<uint64_t>(b & 0x7F) << 42;
            if ((b & 0x80) == 0)
                return value;

            b = stream->ReadByte();  // 8
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            value |= static_cast<uint64_t>(b & 0x7F) << 49;
            if ((b & 0x80) == 0)
                return value;

            b = stream->ReadByte();  // 9
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            value |= static_cast<uint64_t>(b) << 56;
            return value;
        }

        static void SkipVarint(Stream* stream)
        {
            assert(stream);
            int b = stream->ReadByte();  // 1
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            if ((b & 0x80) == 0)
                return;

            b = stream->ReadByte();  // 2
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            if ((b & 0x80) == 0)
                return;

            b = stream->ReadByte();  // 3
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            if ((b & 0x80) == 0)
                return;

            b = stream->ReadByte();  // 4
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            if ((b & 0x80) == 0)
                return;

            b = stream->ReadByte();  // 5
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            if ((b & 0x80) == 0)
                return;

            b = stream->ReadByte();  // 6
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            if ((b & 0x80) == 0)
                return;

            b = stream->ReadByte();  // 7
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            if ((b & 0x80) == 0)
                return;

            b = stream->ReadByte();  // 8
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            if ((b & 0x80) == 0)
                return;

            b = stream->ReadByte();  // 9
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
        }

        static int64_t Zag(uint64_t zigged)
        {
            int64_t ret = static_cast<int64_t>(zigged);
            return (-(ret & 0x01)) ^ ((ret >> 1) & ~(1ull << 63));
        }

        static void ReadHead(Stream* stream, MbpTag& tag, MbpWireTypes& type)
        {
            /**
             * 头由一个或者多个字节构成：
             *   8                     0
             *   TAG(4bits)  TYPE(4bits)
             * 其中，TAG使用VARINT编码，即占据前4bits+若干后继字节。
             *
             * 限制TAG最大不超过UINT16_MAX，则至多占据三个字节。
             */

            assert(stream);
            int b;
            if ((b = stream->ReadByte()) < 0)
                MOE_THROW(OutOfRangeException, "EOF");

            int t = (b & 0x0F);
            if (t >= static_cast<int>(MbpWireTypes::MAX))
                MOE_THROW(BadFormat, "Invalid head type near position {0}", stream->GetPosition() - 1);
            type = static_cast<MbpWireTypes>(t);

            b >>= 4;
            tag = static_cast<MbpTag>(b & 0x7);
            if ((b & 0x8) == 0)
                return;

            b = stream->ReadByte();  // 2
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            tag |= static_cast<MbpTag>(b & 0x7F) << 3;
            if ((b & 0x80) == 0)
                return;

            b = stream->ReadByte();  // 3
            if (b < 0)
                MOE_THROW(OutOfRangeException, "EOF");
            tag |= static_cast<MbpTag>(b & 0x3F) << 10;
            if ((b & 0xC0) == 0)
                return;

            MOE_THROW(BadFormat, "Tag is too big near position {0}", stream->GetPosition() - 1);
        }

    public:
        MbpReader(StreamPtr stream)
            : m_pStream(stream) {}

        MbpReader(const MbpReader&) = default;
        MbpReader(MbpReader&&) = default;

    public:
        void Read(MbpTag tag, bool& val)
        {
            MOE_THROW(NotImplementException, "");
        }

    private:
        void ReadHead()
        {
            std::pair<MbpTag, MbpWireTypes> head;
            ReadHead(m_pStream.GetPointer(), head.first, head.second);
            m_stHead = head;
        }

        void SkipToTag(MbpTag tag)
        {
            MOE_THROW(NotImplementException, "");
        }

    private:
        StreamPtr m_pStream;
        Optional<std::pair<MbpTag, MbpWireTypes>> m_stHead;
    };

    class MbpWriter
    {
    public:
        static void WriteFixed8(Stream* stream, uint8_t value)
        {
            assert(stream);
            stream->WriteByte(value);
        }

        static void WriteFixed32(Stream* stream, uint32_t value)
        {
            assert(stream);
            uint8_t bytes[4] = {
                static_cast<uint8_t>(value & 0xFF),
                static_cast<uint8_t>((value >> 8) & 0xFF),
                static_cast<uint8_t>((value >> 16) & 0xFF),
                static_cast<uint8_t>((value >> 24) & 0xFF),
            };
            stream->Write(BytesView(bytes, sizeof(bytes)), sizeof(bytes));
        }

        static void WriteFixed64(Stream* stream, uint64_t value)
        {
            assert(stream);
            uint8_t bytes[8] = {
                static_cast<uint8_t>(value & 0xFF),
                static_cast<uint8_t>((value >> 8) & 0xFF),
                static_cast<uint8_t>((value >> 16) & 0xFF),
                static_cast<uint8_t>((value >> 24) & 0xFF),
                static_cast<uint8_t>((value >> 32) & 0xFF),
                static_cast<uint8_t>((value >> 40) & 0xFF),
                static_cast<uint8_t>((value >> 48) & 0xFF),
                static_cast<uint8_t>((value >> 56) & 0xFF),
            };
            stream->Write(BytesView(bytes, sizeof(bytes)), sizeof(bytes));
        }

        static void WriteVarint(Stream* stream, uint64_t value)
        {
            assert(stream);
            uint8_t bytes[9];
            bytes[0] = static_cast<uint8_t>((value & 0x7F) | 0x80);
            if ((value >>= 7) == 0)
            {
                bytes[0] &= 0x7F;
                stream->WriteByte(bytes[0]);
                return;
            }

            bytes[1] = static_cast<uint8_t>((value & 0x7F) | 0x80);
            if ((value >>= 7) == 0)
            {
                bytes[1] &= 0x7F;
                stream->Write(BytesView(bytes, sizeof(bytes)), 2);
                return;
            }

            bytes[2] = static_cast<uint8_t>((value & 0x7F) | 0x80);
            if ((value >>= 7) == 0)
            {
                bytes[2] &= 0x7F;
                stream->Write(BytesView(bytes, sizeof(bytes)), 3);
                return;
            }

            bytes[3] = static_cast<uint8_t>((value & 0x7F) | 0x80);
            if ((value >>= 7) == 0)
            {
                bytes[3] &= 0x7F;
                stream->Write(BytesView(bytes, sizeof(bytes)), 4);
                return;
            }

            bytes[4] = static_cast<uint8_t>((value & 0x7F) | 0x80);
            if ((value >>= 7) == 0)
            {
                bytes[4] &= 0x7F;
                stream->Write(BytesView(bytes, sizeof(bytes)), 5);
                return;
            }

            bytes[5] = static_cast<uint8_t>((value & 0x7F) | 0x80);
            if ((value >>= 7) == 0)
            {
                bytes[5] &= 0x7F;
                stream->Write(BytesView(bytes, sizeof(bytes)), 6);
                return;
            }

            bytes[6] = static_cast<uint8_t>((value & 0x7F) | 0x80);
            if ((value >>= 7) == 0)
            {
                bytes[6] &= 0x7F;
                stream->Write(BytesView(bytes, sizeof(bytes)), 7);
                return;
            }

            bytes[7] = static_cast<uint8_t>((value & 0x7F) | 0x80);
            if ((value >>= 7) == 0)
            {
                bytes[7] &= 0x7F;
                stream->Write(BytesView(bytes, sizeof(bytes)), 8);
                return;
            }

            bytes[8] = static_cast<uint8_t>(value);
            stream->Write(BytesView(bytes, sizeof(bytes)), 9);
            return;
        }

        static uint64_t Zig(int64_t value)
        {
            return static_cast<uint64_t>((value << 1) ^ (value >> 31));
        }

        static void WriteHead(Stream* stream, MbpTag tag, MbpWireTypes type)
        {
            assert(stream);
            uint8_t bytes[3];
            bytes[0] = static_cast<uint8_t>(((tag & 0x7) << 4) | static_cast<int>(type) | 0x80);
            if ((tag >>= 3) == 0)
            {
                bytes[0] &= 0x7F;
                stream->WriteByte(bytes[0]);
                return;
            }

            bytes[1] = static_cast<uint8_t>((tag & 0x7F) | 0x80);
            if ((tag >>= 7) == 0)
            {
                bytes[1] &= 0x7F;
                stream->Write(BytesView(bytes, sizeof(bytes)), 2);
                return;
            }

            bytes[2] = static_cast<uint8_t>(tag & 0x3F);
            stream->Write(BytesView(bytes, sizeof(bytes)), 3);
        }

    public:
        MbpWriter(StreamPtr stream)
            : m_pStream(stream) {}

        MbpWriter(const MbpWriter&) = default;
        MbpWriter(MbpWriter&&) = default;

    public:

    private:
        StreamPtr m_pStream;
    };
}
