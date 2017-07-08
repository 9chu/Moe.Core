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
 * @brief 平台宏
 *
 * 根据编译期定义推导出当前的目标平台。
 * 目前支持：
 *   - Windows平台：
 *     MOE_WINDOWS
 *     MOE_PLATFORM = "win"
 *   - Linux平台：
 *     MOE_LINUX
 *     MOE_PLATFORM = "linux"
 *   - OSX平台：
 *     MOE_OSX
 *     MOE_PLATFORM = "osx"
 *
 *   - IOS平台：
 *     MOE_IOS
 *     MOE_PLATFORM = "ios"
 */
#if defined(WIN32) || defined(__MINGW32__)
    #define MOE_WINDOWS
    #define MOE_PLATFORM "win"
#elif defined(__linux__)
    #define MOE_LINUX
    #define MOE_PLATFORM "linux"
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if defined(TARGET_OS_IPHONE)
        #define MOE_IOS
        #define MOE_PLATFORM "ios"
    #else
        #define MOE_OSX
        #define MOE_PLATFORM "osx"
    #endif  // defined(TARGET_OS_IPHONE)
#else
    #error "Unknown platform"
#endif

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

/**
 * @brief 单行断言
 *
 * 检查CHECK，若成立返回EXPR。
 * 用于解决C++11在constexpr中不允许多行代码的问题。
 */
#ifdef NDEBUG
#define MOE_ASSERT_EXPR(CHECK, EXPR) (EXPR)
#else
#define MOE_ASSERT_EXPR(CHECK, EXPR) ((CHECK) ? (EXPR) : ([]{assert(!#CHECK);}(), (EXPR)))
#endif

namespace moe
{
    /**
     * @brief 计算数组大小
     * @note 该方法用于计算一维数组的大小
     * @tparam T 数组类型
     * @return 结果，元素个数
     */
    template <typename T, size_t S>
    constexpr size_t CountOf(T(&)[S])noexcept
    {
        return S;
    }

    /**
     * @brief 不可拷贝基类
     */
    class NonCopyable
    {
    protected:
        constexpr NonCopyable()noexcept = default;
        ~NonCopyable()noexcept = default;

        NonCopyable(const NonCopyable&)noexcept = delete;
        NonCopyable& operator=(const NonCopyable&)noexcept = delete;
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
    constexpr T BitCast(const P& source)noexcept
    {
        static_assert(sizeof(T) == sizeof(P), "Type size mismatched");
        static_assert(std::is_same<uint8_t, unsigned char>::value, "Bad compiler");

        return *reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(&source));
    }
}
