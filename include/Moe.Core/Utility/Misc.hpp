/**
 * @file
 * @date 2017/4/30
 */
#pragma once
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <type_traits>

/**
 * @brief 不可执行分支宏
 */
#define MOE_UNREACHABLE() \
    do { \
        assert(false); \
        ::abort(); \
    } while (false)

/**
 * @brief 标记未使用
 */
#define MOE_UNUSED(x) \
    static_cast<void>(x)

namespace moe
{
    /**
     * @brief 计算数组大小
     * @note 该方法用于计算一维数组的大小
     * @tparam T 数组类型
     * @return 结果，元素个数
     */
    template <typename T, size_t S>
    constexpr size_t CountOf(T(&)[S])
    {
        return S;
    }

    /**
     * @brief 不可拷贝基类
     */
    class NonCopyable
    {
    protected:
        constexpr NonCopyable() = default;
        ~NonCopyable() = default;

        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
    };

    /**
     * @brief 按位强制转换
     * @tparam T 目标类型
     * @tparam P 输入类型
     * @param source 待转换对象
     * @return 转换后结果
     *
     * C++的aliasing rule允许指向不同类型的指针不会相互关联。这导致下述代码不会工作：
     *
     * float f = foo();
     * int fbits = *(int*)(&f);
     *
     * 编译器认为int*类型的fbits不会关联到f上（由于类型不同），所以优化后可能将f置于寄存器，从而使fbits中留下任意随机的结果。
     * 下述方法基于char、unsigned char类型的特殊规则，从而保证不会出现上述情况。
     */
    template <typename T, typename P>
    constexpr T BitCast(const P& source)
    {
        static_assert(sizeof(T) == sizeof(P), "Type size mismatched");
        static_assert(std::is_same<uint8_t, unsigned char>::value, "Bad compiler");

        return *reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(&source));
    }

    /**
     * @brief 数组视图
     * @tparam T 元素类型
     *
     * 用来指示一段具有T类型的数组，不托管所有权。
     */
    template <typename T>
    class ArrayView
    {
    public:
        ArrayView()
            : m_pBuffer(nullptr), m_uSize(0) {}

        ArrayView(const T* data, size_t size)
            : m_pBuffer(data), m_uSize(size)
        {
            assert(size == 0 || (size > 0 && data != nullptr));
        }

        explicit operator bool()const { return m_pBuffer != nullptr; }

        const T& operator[](size_t index)const
        {
            assert(0 <= index && index < m_uSize);
            return m_pBuffer[index];
        }

    public:
        /**
         * @brief 获取缓冲区大小
         * @return 缓冲区元素个数
         */
        size_t Size()const { return m_uSize; }

        /**
         * @brief 缓冲区是否为空
         * @return 是否空
         */
        bool IsEmpty()const { return m_uSize == 0; }

        /**
         * @brief 获取缓冲区
         * @return 缓冲区指针
         */
        const T* GetBuffer()const { return m_pBuffer; }

        /**
         * @brief 获取第一个元素
         * @return 第一个元素
         */
        const T& First()const
        {
            assert(!IsEmpty());
            return m_pBuffer[0];
        }

        /**
         * @brief 获取最后一个元素
         * @return 最后一个元素
         */
        const T& Last()const
        {
            assert(!IsEmpty());
            return m_pBuffer[m_uSize - 1];
        }

        /**
         * @brief 原地截取一段缓冲区
         * @param from 起始位置
         * @param to 结束位置
         * @return 具有相同内存位置的片段
         */
        ArrayView<T> Slice(size_t from, size_t to)const
        {
            assert(to <= m_uSize);
            assert(from < to);
            assert(0 <= from);
            return ArrayView<T>(GetBuffer() + from, to - from);
        }

    protected:
        const T* m_pBuffer;
        size_t m_uSize;
    };

    /**
     * @brief 可变数组视图
     * @tparam T 元素类型
     *
     * 用来指示一段具有T类型的数组，不托管所有权。
     */
    template <typename T>
    class MutableArrayView :
        public ArrayView<T>
    {
    public:
        MutableArrayView()
            : ArrayView<T>() {}

        MutableArrayView(T* data, size_t size)
            : ArrayView<T>(data, size) {}

        const T& operator[](size_t index)const
        {
            return ArrayView<T>::operator[](index);
        }

        T& operator[](size_t index)
        {
            assert(0 <= index && index < ArrayView<T>::m_uSize);
            return const_cast<T*>(ArrayView<T>::m_pBuffer)[index];
        }

    public:
        /**
         * @brief 获取缓冲区
         * @return 缓冲区指针
         */
        T* GetBuffer() { return const_cast<T*>(ArrayView<T>::m_pBuffer); }

        /**
         * @brief 获取第一个元素
         * @return 第一个元素
         */
        T& First()
        {
            assert(!ArrayView<T>::IsEmpty());
            return const_cast<T*>(ArrayView<T>::m_pBuffer)[0];
        }

        /**
         * @brief 获取最后一个元素
         * @return 最后一个元素
         */
        T& Last()
        {
            assert(!ArrayView<T>::IsEmpty());
            return const_cast<T*>(ArrayView<T>::m_pBuffer)[ArrayView<T>::m_uSize - 1];
        }

        /**
         * @brief 原地截取一段缓冲区
         * @param from 起始位置
         * @param to 结束位置
         * @return 具有相同内存位置的片段
         */
        MutableArrayView<T> Slice(size_t from, size_t to)
        {
            assert(to <= ArrayView<T>::m_uSize);
            assert(from < to);
            assert(0 <= from);
            return MutableArrayView<T>(GetBuffer() + from, to - from);
        }
    };
}
