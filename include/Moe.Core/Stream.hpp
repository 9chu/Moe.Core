/**
 * @file
 * @date 2017/7/14
 */
#pragma once
#include <algorithm>

#include "RefPtr.hpp"
#include "Optional.hpp"
#include "ArrayView.hpp"

namespace moe
{
    /**
     * @brief 流查找起点
     */
    enum class StreamSeekOrigin
    {
        Begin,
        Current,
        End
    };

    /**
     * @brief 流式IO抽象接口
     */
    class Stream :
        public RefBase<Stream>
    {
    public:
        Stream() {}
        virtual ~Stream();

    public:
        /**
         * @brief 返回流是否可读
         */
        virtual bool IsReadable()const noexcept = 0;

        /**
         * @brief 返回流是否可写
         */
        virtual bool IsWriteable()const noexcept = 0;

        /**
         * @brief 返回流是否允许查找
         */
        virtual bool IsSeekable()const noexcept = 0;

        /**
         * @brief 返回流的长度
         * @exception OperationNotSupport 若不支持该操作则抛出异常
         */
        virtual size_t GetLength()const = 0;

        /**
         * @brief 返回当前的读写位置
         * @exception OperationNotSupport 若不支持该操作则抛出异常
         */
        virtual size_t GetPosition()const = 0;

        /**
         * @brief 刷新缓冲区
         *
         * 清除该流的所有缓冲区，并使得所有缓冲数据被写入到基础设备。
         * 若操作不支持则不应当忽略该方法的调用。
         */
        virtual void Flush() = 0;

        /**
         * @brief 读取一个字节
         * @exception OperationNotSupport 若不支持该操作则抛出异常
         *
         * 若遇到结束，返回-1，否则返回该字节的值(>=0)。
         */
        virtual int ReadByte() = 0;

        /**
         * @brief 读取若干字节
         * @exception OperationNotSupport 若不支持该操作则抛出异常
         * @param out 输出缓冲区
         * @param count 数量
         * @return 真实读取数量
         *
         * 从流中读取count个字节，返回真实读取的个数并提升读写位置到对应数量的字节。
         */
        virtual size_t Read(MutableBytesView& out, size_t count) = 0;

        /**
         * @brief 寻找读写位置
         * @exception OperationNotSupport 若不支持该操作则抛出异常
         * @param offset 偏移量
         * @param origin 起点
         * @return 新的绝对读写位置
         */
        virtual size_t Seek(int64_t offset, StreamSeekOrigin origin) = 0;

        /**
         * @brief 设置流的长度
         * @exception OperationNotSupport 若不支持该操作则抛出异常
         */
        virtual void SetLength(size_t length) = 0;

        /**
         * @brief 将一个字节写入流
         * @param b 字节
         */
        virtual void WriteByte(uint8_t b) = 0;

        /**
         * @brief 将若干字节写入流
         * @param view 缓冲区
         * @param count 要写入的数量
         */
        virtual void Write(const BytesView& view, size_t count) = 0;

        /**
         * @brief 将流从当前位置全部复制到另一个流中
         * @param other 目标流
         * @return 复制数量
         */
        size_t CopyTo(Stream* other)
        {
            assert(other);

            uint8_t buffer[1024];
            MutableBytesView view(buffer, sizeof(buffer));

            size_t readCount = 0;
            size_t totalCount = 0;
            do
            {
                readCount = Read(view, view.GetSize());
                if (readCount > 0)
                    other->Write(view, readCount);
                totalCount += readCount;
            } while (readCount > 0);

            return totalCount;
        }

        /**
         * @brief 将流从当前位置开始复制若干字节到另一个流中
         * @param other 目标流
         * @param count 复制数量
         * @return 复制数量
         */
        size_t CopyTo(Stream* other, size_t count)
        {
            assert(other);

            uint8_t buffer[1024];
            MutableBytesView view(buffer, sizeof(buffer));

            size_t readCount = 0;
            size_t totalCount = 0;
            do
            {
                readCount = Read(view, std::min(sizeof(buffer), count));
                if (readCount > 0)
                    other->Write(view, readCount);
                totalCount += readCount;
                count -= readCount;
            } while (readCount > 0);

            return totalCount;
        }
    };

    /**
     * @brief BytesView到Stream包装器
     */
    class BytesViewStream :
        public Stream
    {
    public:
        BytesViewStream(const BytesView& view);
        BytesViewStream(const MutableBytesView& view);

    public:
        bool IsReadable()const noexcept;
        bool IsWriteable()const noexcept;
        bool IsSeekable()const noexcept;
        size_t GetLength()const;
        size_t GetPosition()const;
        void Flush();
        int ReadByte();
        size_t Read(MutableBytesView& out, size_t count);
        size_t Seek(int64_t offset, StreamSeekOrigin origin);
        void SetLength(size_t length);
        void WriteByte(uint8_t b);
        void Write(const BytesView& view, size_t count);

    private:
        size_t m_uPosition = 0;
        BytesView m_stView;
        Optional<MutableBytesView> m_stMutableView;
    };

    /**
     * @brief 二进制读取器
     *
     * 封装了流上的一系列二进制转换操作。
     * 注意到读取器不会持有Stream对象。
     */
    template <typename T = Stream>
    class BinaryReader :
        public NonCopyable
    {
    public:
        BinaryReader(T* stream)
            : m_pStream(stream) {}

    public:
        T* GetStream()noexcept { return m_pStream; }

        uint8_t ReadUInt8()
        {
            auto b = m_pStream->ReadByte();
            if (b < 0)
                MOE_THROW(OutOfRangeException, "ReadByte out of range");
            return static_cast<uint8_t>(b);
        }

        uint16_t ReadUInt16LE()
        {
            auto a = ReadUInt8();
            auto b = ReadUInt8();
            return a | (b << 8);
        }

        uint32_t ReadUInt32LE()
        {
            auto a = ReadUInt8();
            auto b = ReadUInt8();
            auto c = ReadUInt8();
            auto d = ReadUInt8();
            return a | (b << 8) | (c << 16) | (d << 24);
        }

        uint64_t ReadUInt64LE()
        {
            uint64_t a = ReadUInt8();
            uint64_t b = ReadUInt8();
            uint64_t c = ReadUInt8();
            uint64_t d = ReadUInt8();
            uint64_t e = ReadUInt8();
            uint64_t f = ReadUInt8();

            uint64_t g = ReadUInt8();
            uint64_t h = ReadUInt8();
            return a | (b << 8) | (c << 16) | (d << 24) | (e << 32) | (f << 40) | (g << 48) | (h << 56);
        }

        char ReadInt8()
        {
            return reinterpret_cast<char>(ReadUInt8());
        }

        int16_t ReadInt16LE()
        {
            return reinterpret_cast<int16_t>(ReadUInt16LE());
        }

        int32_t ReadInt32LE()
        {
            return reinterpret_cast<int32_t>(ReadUInt32LE());
        }

        int64_t ReadInt64LE()
        {
            return reinterpret_cast<int64_t>(ReadUInt64LE());
        }

        uint16_t ReadUInt16BE()
        {
            auto b = ReadUInt8();
            auto a = ReadUInt8();
            return a | (b << 8);
        }

        uint32_t ReadUInt32BE()
        {
            auto d = ReadUInt8();
            auto c = ReadUInt8();
            auto b = ReadUInt8();
            auto a = ReadUInt8();
            return a | (b << 8) | (c << 16) | (d << 24);
        }

        uint64_t ReadUInt64BE()
        {
            uint64_t h = ReadUInt8();
            uint64_t g = ReadUInt8();
            uint64_t f = ReadUInt8();
            uint64_t e = ReadUInt8();
            uint64_t d = ReadUInt8();
            uint64_t c = ReadUInt8();
            uint64_t b = ReadUInt8();
            uint64_t a = ReadUInt8();
            return a | (b << 8) | (c << 16) | (d << 24) | (e << 32) | (f << 40) | (g << 48) | (h << 56);
        }

        int16_t ReadInt16BE()
        {
            return reinterpret_cast<int16_t>(ReadUInt16LE());
        }

        int32_t ReadInt32BE()
        {
            return reinterpret_cast<int32_t>(ReadUInt32LE());
        }

        int64_t ReadInt64BE()
        {
            return reinterpret_cast<int64_t>(ReadUInt64LE());
        }

        std::string ReadString(size_t length)
        {
            std::string ret;
            ret.resize(length, '\0');

            MutableBytesView view(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(ret.data())), ret.length());
            auto count = m_pStream->ReadBytes(view, ret.length());
            if (count < length)
                MOE_THROW(OutOfRangeException, "Expect {0}, but read {1}", length, count);

            ret.resize(std::strlen(ret.data()));
            return ret;
        }

    private:
        T* m_pStream;
    };

    /**
     * @brief 二进制写入器
     *
     * 封装了流上的一系列二进制转换操作。
     * 注意到写入器不会持有Stream对象。
     */
    template <typename T = Stream>
    class BinaryWriter :
        public NonCopyable
    {
    public:
        BinaryWriter(T* stream)
            : m_pStream(stream) {}

    public:
        T* GetStream()noexcept { return m_pStream; }

        void WriteUInt8(uint8_t b)
        {
            m_pStream->WriteByte(b);
        }

        void WriteUInt16LE(uint16_t value)
        {
            m_pStream->WriteByte(value & 0xFF);
            m_pStream->WriteByte((value >> 8) & 0xFF);
        }

        void WriteUInt32LE(uint32_t value)
        {
            m_pStream->WriteByte(value & 0xFF);
            m_pStream->WriteByte((value >> 8) & 0xFF);
            m_pStream->WriteByte((value >> 16) & 0xFF);
            m_pStream->WriteByte((value >> 24) & 0xFF);
        }

        void WriteUInt64LE(uint64_t value)
        {
            m_pStream->WriteByte(value & 0xFF);
            m_pStream->WriteByte((value >> 8) & 0xFF);
            m_pStream->WriteByte((value >> 16) & 0xFF);
            m_pStream->WriteByte((value >> 24) & 0xFF);
            m_pStream->WriteByte((value >> 32) & 0xFF);
            m_pStream->WriteByte((value >> 40) & 0xFF);
            m_pStream->WriteByte((value >> 48) & 0xFF);
            m_pStream->WriteByte((value >> 56) & 0xFF);
        }

        void WriteInt8(char value)
        {
            WriteUInt8(static_cast<uint8_t>(value));
        }

        void WriteInt16LE(int16_t value)
        {
            WriteUInt16LE(static_cast<uint16_t>(value));
        }

        void WriteInt32LE(int32_t value)
        {
            WriteUInt32LE(static_cast<uint32_t>(value));
        }

        void WriteInt64LE(int64_t value)
        {
            WriteUInt64LE(static_cast<uint64_t>(value));
        }

        void WriteUInt16BE(uint16_t value)
        {
            m_pStream->WriteByte((value >> 8) & 0xFF);
            m_pStream->WriteByte(value & 0xFF);
        }

        void WriteUInt32BE(uint32_t value)
        {
            m_pStream->WriteByte((value >> 24) & 0xFF);
            m_pStream->WriteByte((value >> 16) & 0xFF);
            m_pStream->WriteByte((value >> 8) & 0xFF);
            m_pStream->WriteByte(value & 0xFF);
        }

        void WriteUInt64BE(uint64_t value)
        {
            m_pStream->WriteByte((value >> 56) & 0xFF);
            m_pStream->WriteByte((value >> 48) & 0xFF);
            m_pStream->WriteByte((value >> 40) & 0xFF);
            m_pStream->WriteByte((value >> 32) & 0xFF);
            m_pStream->WriteByte((value >> 24) & 0xFF);
            m_pStream->WriteByte((value >> 16) & 0xFF);
            m_pStream->WriteByte((value >> 8) & 0xFF);
            m_pStream->WriteByte(value & 0xFF);
        }

        void WriteInt16BE(int16_t value)
        {
            WriteUInt16BE(static_cast<uint16_t>(value));
        }

        void WriteInt32BE(int32_t value)
        {
            WriteUInt32BE(static_cast<uint32_t>(value));
        }

        void WriteInt64BE(int64_t value)
        {
            WriteUInt64BE(static_cast<uint64_t>(value));
        }

    private:
        T* m_pStream;
    };
}
