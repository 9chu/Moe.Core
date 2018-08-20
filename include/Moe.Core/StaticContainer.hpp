/**
 * @file
 * @author chu
 * @date 2018/7/15
 */
#pragma once
#include "Exception.hpp"

#include <array>
#include <type_traits>

namespace moe
{
    template <typename T, size_t Capacity>
    struct FixedCapacityStorage
    {
        static_assert(Capacity != 0, "Static storage cannot be zero sized.");

        using AlignedStorageType = typename std::aligned_storage<sizeof(typename std::remove_const<T>::type),
            std::alignment_of<typename std::remove_const<T>::type>::value>::type;
        using DataType = typename std::conditional<!std::is_const<T>::value, AlignedStorageType,
            const AlignedStorageType>::type;

        static const size_t kCapacity = Capacity;

        size_t Size = 0;
        alignas(alignof(T)) DataType Data[Capacity] {};
    };

    /**
     * @brief 循环队列
     * @tparam T 存储对象类型
     * @tparam Capacity 队列大小
     */
    template <typename T, size_t Capacity>
    class CircularQueue
    {
        using StorageType = FixedCapacityStorage<T, Capacity + 1>;

    public:
        CircularQueue() = default;
        CircularQueue(const CircularQueue&) = default;
        CircularQueue(CircularQueue&&) = default;

    public:
        T& operator[](size_t index)noexcept
        {
            assert(index < GetSize());
            return *reinterpret_cast<T*>(&m_stStorage.Data[(m_iHead + index) % StorageType::kCapacity]);
        }

        const T& operator[](size_t index)const noexcept
        {
            assert(index < GetSize());
            return *reinterpret_cast<const T*>(&m_stStorage.Data[(m_iHead + index) % StorageType::kCapacity]);
        }

    public:
        /**
         * @brief 是否为空
         */
        bool IsEmpty()const noexcept
        {
            assert((m_iTail == m_iHead) == (m_stStorage.Size == 0));
            return m_stStorage.Size == 0;
        }

        /**
         * @brief 是否为满
         */
        bool IsFull()const noexcept
        {
            assert(((m_iTail + 1) % StorageType::kCapacity == m_iHead) ==
                (m_stStorage.Size >= StorageType::kCapacity - 1));
            return m_stStorage.Size >= StorageType::kCapacity - 1;
        }

        /**
         * @brief 获取元素数量
         */
        size_t GetSize()const noexcept
        {
            assert((m_iTail >= m_iHead ? (m_iTail - m_iHead) : (m_iTail + StorageType::kCapacity - m_iHead)) ==
                m_stStorage.Size);
            return m_stStorage.Size;
        }

        /**
         * @brief 获取最大数量
         */
        size_t GetCapacity()const noexcept
        {
            return Capacity;
        }

        /**
         * @brief 追加一个元素
         * @throw OutOfRangeException 数据越界
         */
        void Push(const T& obj)
        {
            if (IsFull())
                MOE_THROW(OutOfRangeException, "Queue is full");
            new(&m_stStorage.Data[m_iTail]) T(obj);
            m_iTail = (m_iTail + 1) % StorageType::kCapacity;
            ++m_stStorage.Size;
        }

        void Push(T&& obj)
        {
            if (IsFull())
                MOE_THROW(OutOfRangeException, "Queue is full");
            new(&m_stStorage.Data[m_iTail]) T(std::move(obj));
            m_iTail = (m_iTail + 1) % StorageType::kCapacity;
            ++m_stStorage.Size;
        }

        template <typename... Args>
        void Emplace(Args&&... args)
        {
            if (IsFull())
                MOE_THROW(OutOfRangeException, "Queue is full");
            new(&m_stStorage.Data[m_iTail]) T(std::forward<Args>(args)...);
            m_iTail = (m_iTail + 1) % StorageType::kCapacity;
            ++m_stStorage.Size;
        }

        /**
         * @brief 弹出一个元素
         * @throw OutOfRangeException 数据越界
         */
        T Pop()
        {
            if (IsEmpty())
                MOE_THROW(OutOfRangeException, "Queue is empty");
            auto& org = *reinterpret_cast<T*>(&m_stStorage.Data[m_iHead]);
            T ret = std::move(org);
            org.~T();
            m_iHead = (m_iHead + 1) % StorageType::kCapacity;
            --m_stStorage.Size;
            return ret;
        }

        /**
         * @brief 尝试追加一个元素
         * @return 是否成功追加元素
         */
        bool TryPush(const T& obj)
        {
            if (IsFull())
                return false;
            new(&m_stStorage.Data[m_iTail]) T(obj);
            m_iTail = (m_iTail + 1) % StorageType::kCapacity;
            ++m_stStorage.Size;
            return true;
        }

        bool TryPush(T&& obj)noexcept
        {
            if (IsFull())
                return false;
            new(&m_stStorage.Data[m_iTail]) T(std::move(obj));
            m_iTail = (m_iTail + 1) % StorageType::kCapacity;
            ++m_stStorage.Size;
            return true;
        }

        /**
         * @brief 尝试弹出一个元素
         * @param[out] ret 结果
         * @return 是否成功弹出一个元素
         */
        bool TryPop(T& ret)noexcept
        {
            if (IsEmpty())
                return false;
            auto& org = *reinterpret_cast<T*>(&m_stStorage.Data[m_iHead]);
            ret = std::move(org);
            org.~T();
            m_iHead = (m_iHead + 1) % StorageType::kCapacity;
            --m_stStorage.Size;
            return true;
        }

        /**
         * @brief 访问顶端元素
         */
        T& First()
        {
            if (IsEmpty())
                MOE_THROW(OutOfRangeException, "Queue is empty");
            return m_stStorage.Data[m_iHead];
        }

        const T& First()const
        {
            if (IsEmpty())
                MOE_THROW(OutOfRangeException, "Queue is empty");
            return m_stStorage.Data[m_iHead];
        }

        /**
         * @brief 访问末尾的元素
         */
        T& Last()
        {
            if (IsEmpty())
                MOE_THROW(OutOfRangeException, "Queue is empty");
            return *reinterpret_cast<T*>(&m_stStorage.Data[(m_iTail + StorageType::kCapacity - 1) %
                StorageType::kCapacity]);
        }

        const T& Last()const
        {
            if (IsEmpty())
                MOE_THROW(OutOfRangeException, "Queue is empty");
            return *reinterpret_cast<T*>(&m_stStorage.Data[(m_iTail + StorageType::kCapacity - 1) %
                StorageType::kCapacity]);
        }

        /**
         * @brief 清空元素
         */
        void Clear()noexcept
        {
            while (!IsEmpty())
            {
                auto& org = *reinterpret_cast<T*>(&m_stStorage.Data[m_iHead]);
                org.~T();
                m_iHead = (m_iHead + 1) % StorageType::kCapacity;
                --m_stStorage.Size;
            }
        }

    private:
        size_t m_iHead = 0;
        size_t m_iTail = 0;
        StorageType m_stStorage;
    };

    /**
     * @brief 静态Vector
     * @tparam T 存储对象类型
     * @tparam Capacity 队列大小
     */
    template <typename T, size_t Capacity>
    class StaticVector
    {
    public:
        StaticVector() = default;
        StaticVector(const StaticVector&) = default;
        StaticVector(StaticVector&&) = default;

    public:
        T& operator[](size_t index)noexcept
        {
            assert(index < GetSize());
            return *reinterpret_cast<T*>(&m_stStorage.Data[index]);
        }

        const T& operator[](size_t index)const noexcept
        {
            assert(index < GetSize());
            return *reinterpret_cast<const T*>(&m_stStorage.Data[index]);
        }

    public:
        /**
         * @brief 是否为空
         */
        bool IsEmpty()const noexcept { return m_stStorage.Size == 0; }

        /**
         * @brief 是否为满
         */
        bool IsFull()const noexcept { return m_stStorage.Size == Capacity; }

        /**
         * @brief 获取元素数量
         */
        size_t GetSize()const noexcept { return m_stStorage.Size; }

        /**
         * @brief 追加一个元素
         * @throw OutOfRangeException 数据越界
         */
        void PushBack(const T& obj)
        {
            if (IsFull())
                MOE_THROW(OutOfRangeException, "Vector is full");
            new(&m_stStorage.Data[m_stStorage.Size]) T(obj);
            ++m_stStorage.Size;
        }

        void PushBack(T&& obj)
        {
            if (IsFull())
                MOE_THROW(OutOfRangeException, "Vector is full");
            new(&m_stStorage.Data[m_stStorage.Size]) T(std::move(obj));
            ++m_stStorage.Size;
        }

        template <typename... Args>
        void EmplaceBack(Args&&... args)
        {
            if (IsFull())
                MOE_THROW(OutOfRangeException, "Vector is full");
            new(&m_stStorage.Data[m_stStorage.Size]) T(std::forward<Args>(args)...);
            ++m_stStorage.Size;
        }

        /**
         * @brief 弹出一个元素
         * @throw OutOfRangeException 数据越界
         */
        T PopBack()
        {
            if (IsEmpty())
                MOE_THROW(OutOfRangeException, "Vector is empty");
            auto& org = *reinterpret_cast<T*>(&m_stStorage.Data[m_stStorage.Size - 1]);
            T ret = std::move(org);
            org.~T();
            --m_stStorage.Size;
            return ret;
        }

        /**
         * @brief 访问顶端元素
         */
        T& First()
        {
            if (IsEmpty())
                MOE_THROW(OutOfRangeException, "Vector is empty");
            return m_stStorage.Data[0];
        }

        const T& First()const
        {
            if (IsEmpty())
                MOE_THROW(OutOfRangeException, "Vector is empty");
            return m_stStorage.Data[0];
        }

        /**
         * @brief 访问末尾的元素
         */
        T& Last()
        {
            if (IsEmpty())
                MOE_THROW(OutOfRangeException, "Vector is empty");
            return m_stStorage.Data[m_stStorage.Size - 1];
        }

        const T& Last()const
        {
            if (IsEmpty())
                MOE_THROW(OutOfRangeException, "Vector is empty");
            return m_stStorage.Data[m_stStorage.Size - 1];
        }

        /**
         * @brief 清空元素
         */
        void Clear()noexcept
        {
            while (!IsEmpty())
            {
                auto& org = *reinterpret_cast<T*>(&m_stStorage.Data[m_stStorage.Size - 1]);
                org.~T();
                --m_stStorage.Size;
            }
        }

        /**
         * @brief 在指定位置插入对象
         * @param idx 索引
         * @param obj 对象
         */
        void Insert(size_t idx, const T& obj)
        {
            if (idx > Capacity)
                MOE_THROW(OutOfRangeException, "Index is out of range");
            if (IsFull())
                MOE_THROW(OutOfRangeException, "Vector is full");

            T copy = obj;

            // 移动元素，假定Move不抛出异常
            for (size_t i = idx; i < m_stStorage.Size; ++i)
            {
                auto pos = m_stStorage.Size - (i - idx);
                new(m_stStorage.Data[pos]) T(std::move(*reinterpret_cast<T*>(&m_stStorage.Data[pos - 1])));
                m_stStorage.Data[pos - 1].~T();
            }

            // 在目标位置构造对象
            new(m_stStorage.Data[idx]) T(std::move(copy));
            ++m_stStorage.Size;
        }

        /**
         * @brief 在指定位置插入对象
         * @param idx 索引
         * @param obj 对象
         */
        void Insert(size_t idx, T&& obj)
        {
            if (idx > m_stStorage.Size)
                MOE_THROW(OutOfRangeException, "Index is out of range");
            if (IsFull())
                MOE_THROW(OutOfRangeException, "Vector is full");

            // 移动元素，假定Move不抛出异常
            for (size_t i = idx; i < m_stStorage.Size; ++i)
            {
                auto pos = m_stStorage.Size - (i - idx);
                new(m_stStorage.Data[pos]) T(std::move(*reinterpret_cast<T*>(&m_stStorage.Data[pos - 1])));
                m_stStorage.Data[pos - 1].~T();
            }

            // 在目标位置构造对象
            new(m_stStorage.Data[idx]) T(std::move(obj));
            ++m_stStorage.Size;
        }

        /**
         * @brief 删除指定位置的元素
         * @param idx 索引
         */
        void RemoveAt(size_t idx)
        {
            if (idx >= m_stStorage.Size)
                MOE_THROW(OutOfRangeException, "Index is out of range");

            m_stStorage.Data[idx].~T();

            // 移动元素，假定Move不抛出异常
            for (size_t i = idx + 1; i < m_stStorage.Size; ++i)
            {
                new(m_stStorage.Data[i - 1]) T(std::move(*reinterpret_cast<T*>(&m_stStorage.Data[i])));
                m_stStorage.Data[i].~T();
            }

            --m_stStorage.Size;
        }

    private:
        FixedCapacityStorage<T, Capacity> m_stStorage;
    };
}
