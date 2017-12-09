/**
 * @file
 * @date 2017/5/28
 */
#pragma once
#include "Exception.hpp"

#include <array>

namespace moe
{
    /**
     * @brief 循环队列
     * @tparam T 存储对象类型
     * @tparam Size 队列大小
     */
    template <typename T, size_t Size>
    class CircularQueue
    {
        static_assert(Size != 0, "Invalid argument for queue size.");

    public:
        CircularQueue() = default;
        CircularQueue(const CircularQueue&) = default;
        CircularQueue(CircularQueue&&) = default;

    public:
        /**
         * @brief 是否为空
         */
        bool IsEmpty()noexcept { return m_iHead == m_iTail; }

        /**
         * @brief 是否为满
         */
        bool IsFull()noexcept { return (m_iHead + 1) % m_stStorage.size() == m_iTail; }

        /**
         * @brief 获取元素数量
         */
        size_t GetCount()noexcept
        {
            if (m_iHead >= m_iTail)
                return m_iHead - m_iTail;
            else
                return (m_iHead + m_stStorage.size() - m_iTail);
        }

        /**
         * @brief 追加一个元素
         * @throw OutOfRangeException 数据越界
         */
        void Push(const T &obj)
        {
            if (IsFull())
                MOE_THROW(OutOfRangeException, "Queue is full");
            m_stStorage[m_iHead] = obj;
            m_iHead = (m_iHead + 1) % m_stStorage.size();
        }

        void Push(T &&obj)
        {
            if (IsFull())
                MOE_THROW(OutOfRangeException, "Queue is full");
            m_stStorage[m_iHead] = std::move(obj);
            m_iHead = (m_iHead + 1) % m_stStorage.size();
        }

        /**
         * @brief 弹出一个元素
         * @throw OutOfRangeException 数据越界
         */
        T Pop()
        {
            if (IsEmpty())
                MOE_THROW(OutOfRangeException, "Queue is empty");
            T ret = std::move(m_stStorage[m_iTail]);
            m_iTail = (m_iTail + 1) % m_stStorage.size();
            return ret;
        }

    private:
        size_t m_iHead = 0;
        size_t m_iTail = 0;
        std::array<T, Size + 1> m_stStorage;
    };
}
