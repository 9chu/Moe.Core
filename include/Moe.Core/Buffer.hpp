/**
 * @file
 * @date 2017/6/14
 */
#pragma once
#include <cstring>
#include <type_traits>
#include <new>
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
        static_assert(LocalStorageSize > 0, "LocalStorageSize must be non-zero");

    public:
        Buffer() = default;

        Buffer(const uint8_t* data, size_t sz)
        {
            Resize(sz);
            ::memcpy(GetBuffer(), data, sz);
        }

        Buffer(ArrayView<uint8_t> view)
        {
            Resize(view.Size());
            ::memcpy(GetBuffer(), view.GetBuffer(), view.Size());
        }

        template <size_t I>
        Buffer(const Buffer<I>& rhs)
        {
            Resize(rhs.Size());
            ::memcpy(GetBuffer(), rhs.GetBuffer(), rhs.Size());
        }

        template <size_t I>
        Buffer(Buffer<I>&& rhs)
        {
            const uint8_t* buffer = rhs.GetBuffer();

            if (rhs.Size() <= LocalStorageSize)
            {
                // 原地拷贝
                ::memcpy(&m_stStorage, buffer, rhs.Size());
                std::swap(m_uSize, rhs.m_uSize);
            }
            else
            {
                if (rhs.m_uHeapCapacity != 0)
                {
                    // 直接move指针
                    m_pBuffer = rhs.m_pBuffer;
                    m_uHeapCapacity = rhs.m_uHeapCapacity;

                    rhs.m_pBuffer = nullptr;
                    rhs.m_uHeapCapacity = 0;
                    std::swap(m_uSize, rhs.m_uSize);
                }
                else
                {
                    // 需要分配堆空间
                    Resize(rhs.Size());
                    ::memcpy(GetBuffer(), rhs.GetBuffer(), rhs.Size());

                    rhs.m_uSize = 0;
                }
            }
        }

        template <size_t I>
        Buffer& operator=(const Buffer<I>& rhs)
        {
            Resize(rhs.Size());
            ::memcpy(GetBuffer(), rhs.GetBuffer(), rhs.Size());

            return *this;
        }

        /*
        template <size_t I>
        Buffer& operator=(Buffer<I>&& rhs)
        {

        }
         */

    public:
        /**
         * @brief 获取使用的大小
         */
        size_t Size()const noexcept
        {
            return m_uSize;
        }

        /**
         * @brief 获取分配的大小
         */
        size_t Capacity()const noexcept
        {
            return m_uHeapCapacity == 0 ? LocalStorageSize : m_uHeapCapacity;
        }

        /**
         * @brief 获取存储空间
         */
        const uint8_t* GetBuffer()const noexcept
        {
            return m_uHeapCapacity == 0 ? static_cast<const uint8_t*>(&m_stStorage) : m_pBuffer;
        }

        uint8_t* GetBuffer()noexcept
        {
            return m_uHeapCapacity == 0 ? static_cast<uint8_t*>(&m_stStorage) : m_pBuffer;
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
            size_t required = NextPowerOf2(sz);
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
            size_t osz = Size();
            Resize(osz + sz);

            // 空间分配成功后拷贝data的内容
            ::memcpy(GetBuffer() + osz, data, sz);
        }

        void Append(ArrayView<uint8_t> data)
        {
            size_t osz = Size();
            Resize(osz + data.Size());

            // 空间分配成功后拷贝data的内容
            ::memcpy(GetBuffer() + osz, data.GetBuffer(), data.Size());
        }

        template <size_t I>
        void Swap(Buffer<I>& buffer)
        {
            Buffer<LocalStorageSize> temp = std::move(buffer);
            buffer = std::move(*this);
            *this = std::move(temp);
        }

    private:
        union
        {
            uint8_t* m_pBuffer;
            std::aligned_storage<LocalStorageSize, alignof(uint8_t*)>::type m_stStorage;
        };

        size_t m_uSize = 0;
        size_t m_uHeapCapacity = 0;
    };
}
