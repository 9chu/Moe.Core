/**
 * @file
 * @date 2017/12/19
 */
#pragma once
#include <map>
#include <array>
#include <vector>
#include <unordered_map>

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
            Zero = 1,  // bool(false)
            One = 2,  // bool(true)
            Fixed8 = 3,  // char/byte
            Fixed32 = 4,  // float
            Fixed64 = 5,  // double
            Varint = 6,  // int
            Buffer = 7,  // string/vector<uint8_t>/MutableBytesView
            List = 8,  // vector<T>/array<T>
            Map = 9,  // unordered_map<K,V>/map<K,V>
            Structure = 10,  // struct

            MAX = 11,
        };

        using TagType = uint64_t;

        struct FieldHead
        {
            TagType Tag;
            WireTypes Type;
        };

        class Reader;
        class Writer;

        /**
         * @brief 结构体基类
         *
         * 任何写入到流的复合类型都必须继承自该类。
         */
        struct StructBase
        {
            virtual void ReadFrom(Reader* reader) = 0;
            virtual void WriteTo(Writer* writer)const = 0;
        };

        /**
         * @brief 读取变长整数
         * @param stream 流
         * @return 值
         *
         * VarInt编码举例：
         *   整数        0100|1111 000|01111 11|101111 1|0100001
         *   使用varint  [1]0100001 [1]1011111 [1]0111111 [1]1111000 [0]0000100
         *   其中，字节最高位被用来指示是否有后继字节。
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
                bytes[pos++] = b;
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

        /**
         * @brief Mdr流读取器
         *
         * 需要注意：
         *   一个Mdr文档和另一个Mdr文档之间必须有严格的边界，不能在一个流中直接并列。
         * 这是因为默认以不可Seek流进行数据读取，当向前看时必然会多读取一个FieldHead。
         * 这将导致越过文档边界，破坏相邻文档的完整性。
         *   如果读取的是Struct，即以StructEnd（WireTypes::Null）终止则可以避免这
         * 个问题。
         */
        class Reader
        {
        public:
            // clang: https://stackoverflow.com/questions/43819314/default-member-initializer-needed-within-definition-
            // of-enclosing-class-outside
            Reader()noexcept {}
            Reader(const Reader& rhs)noexcept = default;
            Reader(Reader&&)noexcept = default;
            ~Reader() = default;

            explicit Reader(Stream* stream)noexcept
                : m_pStream(stream) {}

            Reader& operator=(const Reader&)noexcept = default;

        public:
            /**
             * @brief 获取最大递归深度
             */
            unsigned GetMaxRecursiveDeep()const noexcept { return m_uMaxRecursiveDeep; }

            /**
             * @brief 设置最大递归深度
             * @param m 深度
             */
            void SetMaxRecursiveDeep(unsigned m)noexcept { m_uMaxRecursiveDeep = m; }

            template <typename T>
            typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, void>::type
            Read(T& out, TagType tag, unsigned deep=0u)
            {
                SkipUntil(tag, deep);

                auto head = ReadHead();
                if (head.Tag != tag)
                    MOE_THROW(BadFormatException, "Field with tag {0} not found", tag);
                switch (head.Type)
                {
                    case WireTypes::Zero:
                        out = static_cast<T>(0);
                        break;
                    case WireTypes::One:
                        out = static_cast<T>(1);
                        break;
                    case WireTypes::Fixed8:
                        {
                            uint8_t v = 0;
                            ReadFixed8(&v);
                            out = static_cast<T>(static_cast<signed char>(v));
                        }
                        break;
                    case WireTypes::Fixed32:
                        if (sizeof(T) < 4)
                            MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);
                        else
                        {
                            uint32_t v = 0;
                            ReadFixed32(&v);
                            out = static_cast<T>(static_cast<int32_t>(v));
                        }
                        break;
                    case WireTypes::Fixed64:
                        if (sizeof(T) < 8)
                            MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);
                        else
                        {
                            uint64_t v = 0;
                            ReadFixed64(&v);
                            out = static_cast<T>(static_cast<int64_t>(v));
                        }
                        break;
                    case WireTypes::Varint:
                        {
                            uint64_t v = 0;
                            ReadVarint(&v);
                            int64_t s = DeZigzag(v);
                            if (s < std::numeric_limits<T>::min() || s > std::numeric_limits<T>::max())
                                MOE_THROW(BadFormatException, "Numeric is overflowed near tag {0}", tag);
                            out = static_cast<T>(s);
                        }
                        break;
                    default:
                        MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);
                }
            }

            template <typename T>
            typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, void>::type
            Read(T& out, TagType tag, unsigned deep=0u)
            {
                SkipUntil(tag, deep);

                auto head = ReadHead();
                if (head.Tag != tag)
                    MOE_THROW(BadFormatException, "Field with tag {0} not found", tag);
                switch (head.Type)
                {
                    case WireTypes::Zero:
                        out = static_cast<T>(0);
                        break;
                    case WireTypes::One:
                        out = static_cast<T>(1);
                        break;
                    case WireTypes::Fixed8:
                        {
                            uint8_t v = 0;
                            ReadFixed8(&v);
                            out = static_cast<T>(v);
                        }
                        break;
                    case WireTypes::Fixed32:
                        if (sizeof(T) < 4)
                            MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);
                        else
                        {
                            uint32_t v = 0;
                            ReadFixed32(&v);
                            out = static_cast<T>(v);
                        }
                        break;
                    case WireTypes::Fixed64:
                        if (sizeof(T) < 8)
                            MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);
                        else
                        {
                            uint64_t v = 0;
                            ReadFixed64(&v);
                            out = static_cast<T>(v);
                        }
                        break;
                    case WireTypes::Varint:
                        {
                            uint64_t v = 0;
                            ReadVarint(&v);
                            if (v < std::numeric_limits<T>::min() || v > std::numeric_limits<T>::max())
                                MOE_THROW(BadFormatException, "Numeric is overflowed near tag {0}", tag);
                            out = static_cast<T>(v);
                        }
                        break;
                    default:
                        MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);
                }
            }

            void Read(float& out, TagType tag, unsigned deep=0u)
            {
                SkipUntil(tag, deep);

                auto head = ReadHead();
                if (head.Tag != tag)
                    MOE_THROW(BadFormatException, "Field with tag {0} not found", tag);
                if (head.Type != WireTypes::Fixed32)
                    MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);

                uint32_t v = 0;
                ReadFixed32(&v);
                out = BitCast<float>(v);
            }

            void Read(double& out, TagType tag, unsigned deep=0u)
            {
                SkipUntil(tag, deep);

                auto head = ReadHead();
                if (head.Tag != tag)
                    MOE_THROW(BadFormatException, "Field with tag {0} not found", tag);
                switch (head.Type)
                {
                    case WireTypes::Fixed32:
                        {
                            uint32_t v = 0;
                            ReadFixed32(&v);
                            out = BitCast<float>(v);
                        }
                        break;
                    case WireTypes::Fixed64:
                        {
                            uint64_t v = 0;
                            ReadFixed64(&v);
                            out = BitCast<double>(v);
                        }
                        break;
                    default:
                        MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);
                }
            }

            void Read(std::string& out, TagType tag, unsigned deep=0u)
            {
                SkipUntil(tag, deep);

                auto head = ReadHead();
                if (head.Tag != tag)
                    MOE_THROW(BadFormatException, "Field with tag {0} not found", tag);
                if (head.Type != WireTypes::Buffer)
                    MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);

                auto len = static_cast<size_t>(Mdr::ReadVarint(m_pStream));
                if (len == 0)
                    out.clear();
                else
                {
                    out.resize(len);
                    if (m_pStream->Read(MutableBytesView(reinterpret_cast<uint8_t*>(&out[0]), out.length()), len)
                        != len)
                    {
                        MOE_THROW(OutOfRangeException, "Eof");
                    }
                }
            }

            void Read(std::vector<uint8_t>& out, TagType tag, unsigned deep=0u)
            {
                SkipUntil(tag, deep);

                auto head = ReadHead();
                if (head.Tag != tag)
                    MOE_THROW(BadFormatException, "Field with tag {0} not found", tag);
                if (head.Type != WireTypes::Buffer)
                    MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);

                auto len = static_cast<size_t>(Mdr::ReadVarint(m_pStream));
                if (len == 0)
                    out.clear();
                else
                {
                    out.resize(len);
                    if (m_pStream->Read(MutableBytesView(&out[0], out.size()), len) != len)
                        MOE_THROW(OutOfRangeException, "Eof");
                }
            }

            void Read(MutableBytesView out, size_t& sz, TagType tag, unsigned deep=0u)
            {
                SkipUntil(tag, deep);

                auto head = ReadHead();
                if (head.Tag != tag)
                    MOE_THROW(BadFormatException, "Field with tag {0} not found", tag);
                if (head.Type != WireTypes::Buffer)
                    MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);

                auto len = static_cast<size_t>(Mdr::ReadVarint(m_pStream));
                if (len == 0)
                    sz = 0;
                else
                {
                    size_t readSize = std::min(len, out.GetSize());
                    if (m_pStream->Read(out, readSize) != readSize)
                        MOE_THROW(OutOfRangeException, "Eof");
                    len -= readSize;
                    if (len > 0)
                        m_pStream->Skip(len);
                    sz = readSize;
                }
            }

            template <typename TValue>
            void Read(std::vector<TValue>& out, TagType tag, unsigned deep=0u)
            {
                SkipUntil(tag, deep);

                auto head = ReadHead();
                if (head.Tag != tag)
                    MOE_THROW(BadFormatException, "Field with tag {0} not found", tag);
                if (head.Type != WireTypes::List)
                    MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);

                auto count = static_cast<size_t>(Mdr::ReadVarint(m_pStream));
                out.clear();
                out.reserve(count);
                for (size_t i = 0; i < count; ++i)
                {
                    TValue v;
                    Read(v, 0, deep);
                    out.emplace_back(std::move(v));
                }
            }

            template <typename TValue, size_t Count>
            void Read(std::array<TValue, Count>& out, size_t& sz, TagType tag, unsigned deep=0u)
            {
                SkipUntil(tag, deep);

                auto head = ReadHead();
                if (head.Tag != tag)
                    MOE_THROW(BadFormatException, "Field with tag {0} not found", tag);
                if (head.Type != WireTypes::List)
                    MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);

                auto count = static_cast<size_t>(Mdr::ReadVarint(m_pStream));
                auto realCount = std::min(count, Count);
                for (size_t i = 0; i < realCount; ++i)
                    Read(out[i], 0, deep);
                count -= realCount;
                for (size_t i = 0; i < count; ++i)
                {
                    if (!Skip(0, deep))
                        MOE_THROW(BadFormatException, "Invalid list format");
                }
                sz = realCount;
            }

            template <typename TKey, typename TValue>
            void Read(std::map<TKey, TValue>& out, TagType tag, unsigned deep=0u)
            {
                SkipUntil(tag, deep);

                auto head = ReadHead();
                if (head.Tag != tag)
                    MOE_THROW(BadFormatException, "Field with tag {0} not found", tag);
                if (head.Type != WireTypes::Map)
                    MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);

                auto count = static_cast<size_t>(Mdr::ReadVarint(m_pStream));
                out.clear();
                for (size_t i = 0; i < count; ++i)
                {
                    TKey k;
                    TValue v;
                    Read(k, 0, deep);
                    Read(v, 1, deep);
                    auto ret = out.emplace(std::move(k), std::move(v));
                    if (!ret.second)
                        MOE_THROW(BadFormatException, "Duplicated key \"{0}\"", k);
                }
            }

            template <typename TKey, typename TValue>
            void Read(std::unordered_map<TKey, TValue>& out, TagType tag, unsigned deep=0u)
            {
                SkipUntil(tag, deep);

                auto head = ReadHead();
                if (head.Tag != tag)
                    MOE_THROW(BadFormatException, "Field with tag {0} not found", tag);
                if (head.Type != WireTypes::Map)
                    MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);

                auto count = static_cast<size_t>(Mdr::ReadVarint(m_pStream));
                out.clear();
                out.reserve(count);
                for (size_t i = 0; i < count; ++i)
                {
                    TKey k;
                    TValue v;
                    Read(k, 0, deep);
                    Read(v, 1, deep);
                    auto ret = out.emplace(std::move(k), std::move(v));
                    if (!ret.second)
                        MOE_THROW(BadFormatException, "Duplicated key \"{0}\"", k);
                }
            }

            template <typename T>
            typename std::enable_if<std::is_class<T>::value, void>::type
            Read(T& out, TagType tag, unsigned deep=0u)
            {
                SkipUntil(tag, deep);

                auto head = ReadHead();
                if (head.Tag != tag)
                    MOE_THROW(BadFormatException, "Field with tag {0} not found", tag);
                if (head.Type != WireTypes::Structure)
                    MOE_THROW(BadFormatException, "Field with tag {0} type mismatched", tag);

                out.ReadFrom(this);
                head = ReadHead();
                if (head.Type != WireTypes::Null)
                    MOE_THROW(BadFormatException, "Structure boundary expected, but found {0}", head.Type);
            }

            template <typename T>
            void Read(Optional<T>& out, TagType tag, unsigned deep=0u)
            {
                out.Clear();
                SkipUntil(tag, deep);

                auto peek = PeekHead();
                if (peek && peek->Type != WireTypes::Null && peek->Tag == tag)
                {
                    T v;
                    Read(v, tag, deep);
                    out = std::move(v);
                }
            }

        private:
            bool Skip(TagType tag, unsigned deep)
            {
                auto head = PeekHead();
                while (head && head->Tag <= tag && head->Type != WireTypes::Null)
                {
                    auto curTag = head->Tag;
                    SkipNextField(deep);
                    if (curTag == tag)
                        return true;
                }
                return false;
            }

            void SkipUntil(TagType tag, unsigned deep)
            {
                assert(m_pStream);
                auto head = PeekHead();
                while (head && head->Tag < tag && head->Type != WireTypes::Null)
                    SkipNextField(deep);
            }

            FieldHead ReadHead()
            {
                if (m_stForward)
                {
                    FieldHead ret = *m_stForward;
                    m_stForward.Clear();
                    return ret;
                }

                auto h = m_pStream->ReadByte();
                if (h < 0)
                    MOE_THROW(OutOfRangeException, "Eof");
                auto t = static_cast<TagType>(h & 0xF0) >> 4;  // 高4位存Tag
                auto tt = static_cast<uint32_t>(h & 0x0F);  // 低4位存类型
                if (tt >= static_cast<uint32_t>(WireTypes::MAX))
                    MOE_THROW(BadFormatException, "Unknown wire type {0}", tt);
                if (t == 0xF)
                    t = Mdr::ReadVarint(m_pStream) + 0xF;

                FieldHead ret;
                ret.Tag = t;
                ret.Type = static_cast<WireTypes>(tt);
                return ret;
            }

            const Optional<FieldHead>& PeekHead()
            {
                if (m_stForward)
                    return m_stForward;

                auto h = m_pStream->ReadByte();
                if (h < 0)
                {
                    assert(!m_stForward);
                    return m_stForward;
                }

                auto t = static_cast<TagType>(h & 0xF0) >> 4;
                auto tt = static_cast<uint32_t>(h & 0x0F);
                if (tt >= static_cast<uint32_t>(WireTypes::MAX))
                    MOE_THROW(BadFormatException, "Unknown wire type {0}", tt);
                if (t == 0xF)
                    t = Mdr::ReadVarint(m_pStream) + 0xF;

                FieldHead ret;
                ret.Tag = t;
                ret.Type = static_cast<WireTypes>(tt);
                m_stForward = ret;
                return m_stForward;
            }

            void ReadFixed8(uint8_t* out)
            {
                auto ret = m_pStream->ReadByte();
                if (ret < 0)
                    MOE_THROW(OutOfRangeException, "Eof");
                if (out)
                    *out = static_cast<uint8_t>(ret);
            }

            void ReadFixed32(uint32_t* out)
            {
                uint8_t buffer[4];
                if (m_pStream->Read(MutableBytesView(buffer, 4), 4) != 4)
                    MOE_THROW(OutOfRangeException, "Eof");
                if (out)
                    *out = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
            }

            void ReadFixed64(uint64_t* out)
            {
                uint8_t buffer[8];
                if (m_pStream->Read(MutableBytesView(buffer, 8), 8) != 8)
                    MOE_THROW(OutOfRangeException, "Eof");
                if (out)
                {
                    *out = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | ((uint64_t)buffer[3] << 24) |
                        ((uint64_t)buffer[4] << 32) | ((uint64_t)buffer[5] << 40) | ((uint64_t)buffer[6] << 48) |
                        ((uint64_t)buffer[7] << 56);
                }
            }

            void ReadVarint(uint64_t* out)
            {
                auto ret = Mdr::ReadVarint(m_pStream);
                if (out)
                    *out = ret;
            }

            void SkipBuffer()
            {
                auto len = static_cast<size_t>(Mdr::ReadVarint(m_pStream));
                if (len > 0)
                    m_pStream->Skip(len);
            }

            void SkipList(unsigned deep)
            {
                auto count = static_cast<size_t>(Mdr::ReadVarint(m_pStream));
                for (size_t i = 0; i < count; ++i)
                {
                    if (!Skip(0, deep))
                        MOE_THROW(BadFormatException, "Invalid list format");
                }
            }

            void SkipDict(unsigned deep)
            {
                auto count = static_cast<size_t>(Mdr::ReadVarint(m_pStream));
                for (size_t i = 0; i < count; ++i)
                {
                    if (!Skip(0, deep) || !Skip(1, deep))
                        MOE_THROW(BadFormatException, "Invalid dict format");
                }
            }

            void SkipNextField(unsigned deep)  // 跳过除EndStruct以外的域
            {
                auto head = ReadHead();
                switch (head.Type)
                {
                    case WireTypes::Fixed8:
                        ReadFixed8(nullptr);
                        break;
                    case WireTypes::Fixed32:
                        ReadFixed32(nullptr);
                        break;
                    case WireTypes::Fixed64:
                        ReadFixed64(nullptr);
                        break;
                    case WireTypes::Varint:
                        ReadVarint(nullptr);
                        break;
                    case WireTypes::Buffer:
                        SkipBuffer();
                        break;
                    case WireTypes::List:
                        if (deep + 1 >= m_uMaxRecursiveDeep)
                            MOE_THROW(BadFormatException, "Stack overflow");
                        SkipList(deep + 1);
                        break;
                    case WireTypes::Map:
                        if (deep + 1 >= m_uMaxRecursiveDeep)
                            MOE_THROW(BadFormatException, "Stack overflow");
                        SkipDict(deep + 1);
                        break;
                    case WireTypes::Structure:
                        if (deep + 1 >= m_uMaxRecursiveDeep)
                            MOE_THROW(BadFormatException, "Stack overflow");
                        else
                        {
                            auto adv = PeekHead();
                            while (adv && adv->Type != WireTypes::Null)
                                SkipNextField(deep + 1);
                            ReadHead();
                            assert(head.Type == WireTypes::Null);
                        }
                        break;
                    default:
                        MOE_THROW(BadFormatException, "Unexpected field type {0}", head.Type);
                        break;
                }
            }

        private:
            Stream* m_pStream = nullptr;
            Optional<FieldHead> m_stForward;
            unsigned m_uMaxRecursiveDeep = 16;  // 最大嵌套深度
        };

        /**
         * @brief Mdr流写入器
         */
        class Writer
        {
        public:
            Writer()noexcept {}
            Writer(const Writer& rhs)noexcept = default;
            Writer(Writer&&)noexcept = default;
            ~Writer() = default;

            explicit Writer(Stream* stream)noexcept
                : m_pStream(stream) {}

            Writer& operator=(const Writer&)noexcept = default;

        public:
            template <typename T>
            typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, void>::type
            Write(T value, TagType tag)
            {
                static_assert(sizeof(T) <= 8, "Error");
                assert(m_pStream);
                FieldHead head;
                head.Tag = tag;

                if (value == static_cast<T>(0))
                {
                    head.Type = WireTypes::Zero;
                    WriteHead(head);
                }
                else if (value == static_cast<T>(1))
                {
                    head.Type = WireTypes::One;
                    WriteHead(head);
                }
                else if (sizeof(value) == 1)
                {
                    head.Type = WireTypes::Fixed8;
                    WriteHead(head);
                    WriteFixed8(static_cast<uint8_t>(value));
                }
                else
                {
                    auto encoded = Zigzag(static_cast<int64_t>(value));

                    if (sizeof(value) == 4 && encoded > 268435455)
                    {
                        head.Type = WireTypes::Fixed32;
                        WriteHead(head);
                        WriteFixed32(static_cast<uint32_t>(static_cast<int32_t>(value)));
                    }
                    else if (sizeof(value) == 8 && encoded > 72057594037927935)
                    {
                        head.Type = WireTypes::Fixed64;
                        WriteHead(head);
                        WriteFixed64(static_cast<uint64_t>(static_cast<int64_t>(value)));
                    }
                    else
                    {
                        head.Type = WireTypes::Varint;
                        WriteHead(head);
                        WriteVarint(encoded);
                    }
                }
            }

            template <typename T>
            typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, void>::type
            Write(T value, TagType tag)
            {
                static_assert(sizeof(T) <= 8, "Error");
                assert(m_pStream);
                FieldHead head;
                head.Tag = tag;

                if (value == static_cast<T>(0))
                {
                    head.Type = WireTypes::Zero;
                    WriteHead(head);
                }
                else if (value == static_cast<T>(1))
                {
                    head.Type = WireTypes::One;
                    WriteHead(head);
                }
                else if (sizeof(value) == 1)
                {
                    head.Type = WireTypes::Fixed8;
                    WriteHead(head);
                    WriteFixed8(static_cast<uint8_t>(value));
                }
                else if (sizeof(value) == 4 && static_cast<uint32_t>(value) > 268435455)
                {
                    head.Type = WireTypes::Fixed32;
                    WriteHead(head);
                    WriteFixed32(static_cast<uint32_t>(value));
                }
                else if (sizeof(value) == 8 && static_cast<uint64_t>(value) > 72057594037927935)
                {
                    head.Type = WireTypes::Fixed64;
                    WriteHead(head);
                    WriteFixed64(static_cast<uint64_t>(value));
                }
                else
                {
                    head.Type = WireTypes::Varint;
                    WriteHead(head);
                    WriteVarint(static_cast<uint64_t>(value));
                }
            }

            void Write(float value, TagType tag)
            {
                FieldHead head = { tag, WireTypes::Fixed32 };
                WriteHead(head);
                WriteFixed32(BitCast<uint32_t>(value));
            }

            void Write(double value, TagType tag)
            {
                FieldHead head = { tag, WireTypes::Fixed64 };
                WriteHead(head);
                WriteFixed64(BitCast<uint64_t>(value));
            }

            void Write(const std::string& value, TagType tag)
            {
                FieldHead head = { tag, WireTypes::Buffer };
                WriteHead(head);
                WriteVarint(value.length());
                if (value.length())
                {
                    m_pStream->Write(BytesView(reinterpret_cast<const uint8_t*>(value.data()), value.length()),
                        value.length());
                }
            }

            void Write(const std::vector<uint8_t>& value, TagType tag)
            {
                FieldHead head = { tag, WireTypes::Buffer };
                WriteHead(head);
                WriteVarint(value.size());
                if (value.size())
                    m_pStream->Write(BytesView(value.data(), value.size()), value.size());
            }

            void Write(BytesView value, TagType tag)
            {
                FieldHead head = { tag, WireTypes::Buffer };
                WriteHead(head);
                WriteVarint(value.GetSize());
                if (value.GetSize())
                    m_pStream->Write(value, value.GetSize());
            }

            template <typename TValue>
            void Write(const std::vector<TValue>& value, TagType tag)
            {
                FieldHead head = { tag, WireTypes::List };
                WriteHead(head);
                WriteVarint(value.size());
                for (size_t i = 0; i < value.size(); ++i)
                    Write(value[i], 0);
            }

            template <typename TValue, size_t Count>
            void Write(const std::array<TValue, Count>& value, size_t sz, TagType tag)
            {
                assert(m_pStream && sz <= value.size());
                sz = std::min(sz, value.size());

                FieldHead head = { tag, WireTypes::List };
                WriteHead(head);
                WriteVarint(sz);
                for (size_t i = 0; i < sz; ++i)
                    Write(value[i], 0);
            }

            template <typename TKey, typename TValue>
            void Write(const std::map<TKey, TValue>& value, TagType tag)
            {
                FieldHead head = { tag, WireTypes::Map };
                WriteHead(head);
                WriteVarint(value.size());
                for (const auto& i : value)
                {
                    Write(i->first, 0);
                    Write(i->second, 1);
                }
            }

            template <typename TKey, typename TValue>
            void Write(const std::unordered_map<TKey, TValue>& value, TagType tag)
            {
                FieldHead head = { tag, WireTypes::Map };
                WriteHead(head);
                WriteVarint(value.size());
                for (const auto& i : value)
                {
                    Write(i->first, 0);
                    Write(i->second, 1);
                }
            }

            template <typename T>
            typename std::enable_if<std::is_class<T>::value, void>::type
            Write(const T& value, TagType tag)
            {
                FieldHead head = { tag, WireTypes::Structure };
                WriteHead(head);

                value.WriteTo(this);

                head.Tag = 0;
                head.Type = WireTypes::Null;
                WriteHead(head);
            }

            template <typename T>
            void Write(const Optional<T>& out, TagType tag)
            {
                if (out)
                    Write(*out, tag);
            }

        private:
            void WriteHead(const FieldHead& head)
            {
                assert(head.Type < WireTypes::MAX);
                if (head.Tag < 0xF)
                    m_pStream->WriteByte(static_cast<uint8_t>((head.Tag << 4) | static_cast<uint32_t>(head.Type)));
                else
                {
                    m_pStream->WriteByte(static_cast<uint8_t>(0xF0 | static_cast<uint32_t>(head.Type)));
                    Mdr::WriteVarint(m_pStream, head.Tag - 0xF);
                }
            }

            void WriteFixed8(uint8_t value)
            {
                m_pStream->WriteByte(value);
            }

            void WriteFixed32(uint32_t value)
            {
                uint8_t buffer[4] = {
                    static_cast<uint8_t>(value & 0xFF), static_cast<uint8_t>((value >> 8) & 0xFF),
                    static_cast<uint8_t>((value >> 16) & 0xFF), static_cast<uint8_t>((value >> 24) & 0xFF)
                };
                m_pStream->Write(BytesView(buffer, 4), 4);
            }

            void WriteFixed64(uint64_t value)
            {
                uint8_t buffer[8] = {
                    static_cast<uint8_t>(value & 0xFF), static_cast<uint8_t>((value >> 8) & 0xFF),
                    static_cast<uint8_t>((value >> 16) & 0xFF), static_cast<uint8_t>((value >> 24) & 0xFF),
                    static_cast<uint8_t>((value >> 32) & 0xFF), static_cast<uint8_t>((value >> 40) & 0xFF),
                    static_cast<uint8_t>((value >> 48) & 0xFF), static_cast<uint8_t>((value >> 56) & 0xFF),
                };
                m_pStream->Write(BytesView(buffer, 8), 8);
            }

            void WriteVarint(uint64_t value)
            {
                Mdr::WriteVarint(m_pStream, value);
            }

        private:
            Stream* m_pStream = nullptr;
        };
    };
}
