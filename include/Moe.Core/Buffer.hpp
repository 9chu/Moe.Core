/**
 * @file
 * @date 2017/6/14
 */
#pragma once
#include <cstring>
#include <new>
#include <type_traits>

#include "Math.hpp"
#include "ArrayView.hpp"

namespace moe
{
    /**
     * @brief 缓冲区
     * @tparam LocalStorageSize 原地分配的大小
     */
    template <size_t LocalStorageSize = 128>
    class Buffer
    {
        template <size_t>
        friend class Buffer;

        static_assert(LocalStorageSize > 0, "LocalStorageSize must be non-zero");

    public:
        Buffer() = default;

        Buffer(const uint8_t* data, size_t sz)
        {
            Resize(sz);
            ::memcpy(GetBuffer(), data, sz);
        }

        Buffer(BytesView view)
        {
            Resize(view.GetSize());
            ::memcpy(GetBuffer(), view.GetBuffer(), view.GetSize());
        }

        template <size_t I>
        Buffer(const Buffer<I>& rhs)
        {
            Resize(rhs.GetSize());
            ::memcpy(GetBuffer(), rhs.GetBuffer(), rhs.GetSize());
        }

        template <size_t I>
        Buffer(Buffer<I>&& rhs)
        {
            if (rhs.GetSize() <= LocalStorageSize)
            {
                // 原地拷贝
                m_uSize = rhs.m_uSize;
                ::memcpy(&m_stStorage, rhs.GetBuffer(), rhs.GetSize());

                rhs.m_uSize = 0;
            }
            else
            {
                if (rhs.m_uHeapCapacity != 0)
                {
                    // 直接move指针
                    m_pBuffer = rhs.m_pBuffer;
                    m_uHeapCapacity = rhs.m_uHeapCapacity;
                    m_uSize = rhs.m_uSize;

                    rhs.m_pBuffer = nullptr;
                    rhs.m_uHeapCapacity = 0;
                    rhs.m_uSize = 0;
                }
                else
                {
                    // 需要分配堆空间
                    Resize(rhs.GetSize());
                    ::memcpy(GetBuffer(), rhs.GetBuffer(), rhs.GetSize());

                    rhs.m_uSize = 0;
                }
            }
        }

        ~Buffer()
        {
            if (m_uHeapCapacity > 0)
            {
                ::free(m_pBuffer);
                m_pBuffer = nullptr;
                m_uHeapCapacity = 0;
            }
            m_uSize = 0;
        }

        template <size_t I>
        Buffer& operator=(const Buffer<I>& rhs)
        {
            Resize(rhs.GetSize());
            ::memcpy(GetBuffer(), rhs.GetBuffer(), rhs.GetSize());

            return *this;
        }

        template <size_t I>
        Buffer& operator=(Buffer<I>&& rhs)
        {
            // 无论如何都需要释放对象自己申请的堆空间
            if (m_uHeapCapacity > 0)
            {
                ::free(m_pBuffer);
                m_uHeapCapacity = 0;
            }
            m_pBuffer = nullptr;
            m_uSize = 0;

            if (rhs.GetSize() <= LocalStorageSize)
            {
                // 原地拷贝
                m_uSize = rhs.m_uSize;
                ::memcpy(&m_stStorage, rhs.GetBuffer(), rhs.GetSize());

                rhs.m_uSize = 0;
            }
            else
            {
                if (rhs.m_uHeapCapacity != 0)
                {
                    // 直接move指针
                    m_pBuffer = rhs.m_pBuffer;
                    m_uHeapCapacity = rhs.m_uHeapCapacity;
                    m_uSize = rhs.m_uSize;

                    rhs.m_pBuffer = nullptr;
                    rhs.m_uHeapCapacity = 0;
                    rhs.m_uSize = 0;
                }
                else
                {
                    // 需要分配堆空间
                    Resize(rhs.GetSize());
                    ::memcpy(GetBuffer(), rhs.GetBuffer(), rhs.GetSize());

                    rhs.m_uSize = 0;
                }
            }

            return *this;
        }

        uint8_t& operator[](size_t i)noexcept
        {
            assert(i < GetSize());
            return GetBuffer()[i];
        }

        uint8_t operator[](size_t i)const noexcept
        {
            assert(i < GetSize());
            return GetBuffer()[i];
        }

    public:
        /**
         * @brief 是否为空
         */
        bool IsEmpty()const noexcept
        {
            return GetSize() == 0;
        }

        /**
         * @brief 获取使用的大小
         */
        size_t GetSize()const noexcept
        {
            return m_uSize;
        }

        /**
         * @brief 获取分配的大小
         */
        size_t GetCapacity()const noexcept
        {
            return m_uHeapCapacity == 0 ? LocalStorageSize : m_uHeapCapacity;
        }

        /**
         * @brief 获取存储空间
         */
        const uint8_t* GetBuffer()const noexcept
        {
            return m_uHeapCapacity == 0 ? reinterpret_cast<const uint8_t*>(&m_stStorage) : m_pBuffer;
        }

        uint8_t* GetBuffer()noexcept
        {
            return m_uHeapCapacity == 0 ? reinterpret_cast<uint8_t*>(&m_stStorage) : m_pBuffer;
        }

        /**
         * @brief 重新设置大小
         * @param sz 期望的大小
         *
         * 方法不会剔除多余的空间。
         * 针对扩展的空间，其初始值是未定义的。
         */
        void Resize(size_t sz)
        {
            // 当新的大小小于已分配大小时可以直接赋值
            if (sz <= m_uSize || sz <= m_uHeapCapacity || sz <= LocalStorageSize)
            {
                m_uSize = sz;
                return;
            }

            Recapacity(sz);
            m_uSize = sz;
        }

        /**
         * @brief 重新分配大小
         * @param sz 期望的可用空间
         */
        void Recapacity(size_t sz)
        {
            // 大小小于栈空间可用内存的情况下必然可以直接存储
            if (sz <= LocalStorageSize)
                return;

            // 否则需要分配足够内存
            size_t required = Math::NextPowerOf2(sz);
            assert(required >= LocalStorageSize);
            if (required <= m_uHeapCapacity)
                return;

            uint8_t* buffer = nullptr;
            if (m_uHeapCapacity == 0)
                buffer = static_cast<uint8_t*>(::malloc(required));
            else
                buffer = static_cast<uint8_t*>(::realloc(m_pBuffer, required));

            // 若内存分配失败则抛出异常，此时保证原状态不变
            if (!buffer)
                throw std::bad_alloc();

            // 如果原来在栈空间分配，则需要把东西拷贝到新的空间上
            if (m_uHeapCapacity == 0 && m_uSize > 0)
                ::memcpy(buffer, &m_stStorage, m_uSize);

            m_pBuffer = buffer;
            m_uHeapCapacity = required;
        }

        /**
         * @brief 清空
         */
        void Clear()noexcept
        {
            m_uSize = 0;
        }

        /**
         * @brief 追加数据
         * @param data 数据源
         * @param sz 大小
         */
        void Append(void* data, size_t sz)
        {
            if (sz == 0)
                return;

            size_t osz = GetSize();
            Resize(osz + sz);

            // 空间分配成功后拷贝data的内容
            ::memcpy(GetBuffer() + osz, data, sz);
        }

        void Append(BytesView data)
        {
            if (data.GetSize() == 0)
                return;

            size_t osz = GetSize();
            Resize(osz + data.GetSize());

            // 空间分配成功后拷贝data的内容
            ::memcpy(GetBuffer() + osz, data.GetBuffer(), data.GetSize());
        }

        /**
         * @brief 向左平移缓冲区
         * @param index 开始平移的位置
         * @param count 数量
         *
         * 将[index, size)范围内的数据移动到[index - count, size - count)的位置上。
         * 同时会将大小减少count个字节。
         */
        void ShiftLeft(size_t index, size_t count)noexcept
        {
            if (index > GetSize() || count == 0)
                return;
            if (count > index)
                count = index;

            if (GetSize() - index > 0)
                ::memmove(GetBuffer() + index - count, GetBuffer() + index, GetSize() - index);
            Resize(GetSize() - count);
        }

        /**
         * @brief 交换缓冲区数据
         * @tparam I 另一缓冲区的预分配大小
         * @param buffer 被交换的缓冲区
         */
        template <size_t I>
        void Swap(Buffer<I>& buffer)
        {
            Buffer<LocalStorageSize> temp = std::move(buffer);
            buffer = std::move(*this);
            *this = std::move(temp);
        }

        /**
         * @brief 转换到BytesView
         */
        BytesView ToBytesView()const noexcept
        {
            return BytesView(GetBuffer(), GetSize());
        }

        /**
         * @brief 转换到可变BytesView
         */
        MutableBytesView ToMutableBytesView()noexcept
        {
            return MutableBytesView(GetBuffer(), GetSize());
        }

    private:
        union
        {
            uint8_t* m_pBuffer;
            typename std::aligned_storage<LocalStorageSize, alignof(uint8_t*)>::type m_stStorage;
        };

        size_t m_uSize = 0;
        size_t m_uHeapCapacity = 0;
    };
}
