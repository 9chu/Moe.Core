/**
 * @file
 * @date 2017/4/30
 */
#pragma once
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <type_traits>
#include <string>

//////////////////////////////////////// <editor-fold desc="辅助宏">

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
#if defined(WIN32) || defined(__MINGW32__) || defined(_MSC_VER)
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

/**
 * @brief MAP宏
 * @see https://github.com/swansontec/map-macro
 */
#define MOE_MAP(f, ...) MOE_MAP__EVAL(MOE_MAP__MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

#define MOE_MAP__EVAL0(...) __VA_ARGS__
#define MOE_MAP__EVAL1(...) MOE_MAP__EVAL0(MOE_MAP__EVAL0(MOE_MAP__EVAL0(__VA_ARGS__)))
#define MOE_MAP__EVAL2(...) MOE_MAP__EVAL1(MOE_MAP__EVAL1(MOE_MAP__EVAL1(__VA_ARGS__)))
#define MOE_MAP__EVAL3(...) MOE_MAP__EVAL2(MOE_MAP__EVAL2(MOE_MAP__EVAL2(__VA_ARGS__)))
#define MOE_MAP__EVAL4(...) MOE_MAP__EVAL3(MOE_MAP__EVAL3(MOE_MAP__EVAL3(__VA_ARGS__)))
#define MOE_MAP__EVAL(...) MOE_MAP__EVAL4(MOE_MAP__EVAL4(MOE_MAP__EVAL4(__VA_ARGS__)))

#define MOE_MAP__END(...)
#define MOE_MAP__OUT

#define MOE_MAP__GET_END2() 0, MOE_MAP__END
#define MOE_MAP__GET_END1(...) MOE_MAP__GET_END2
#define MOE_MAP__GET_END(...) MOE_MAP__GET_END1
#define MOE_MAP__NEXT0(test, next, ...) next MOE_MAP__OUT
#define MOE_MAP__NEXT1(test, next) MOE_MAP__NEXT0(test, next, 0)
#define MOE_MAP__NEXT(test, next) MOE_MAP__NEXT1(MOE_MAP__GET_END test, next)

#define MOE_MAP__MAP0(f, x, peek, ...) f(x) MOE_MAP__NEXT(peek, MOE_MAP__MAP1)(f, peek, __VA_ARGS__)
#define MOE_MAP__MAP1(f, x, peek, ...) f(x) MOE_MAP__NEXT(peek, MOE_MAP__MAP0)(f, peek, __VA_ARGS__)

//////////////////////////////////////// </editor-fold>

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

    /**
     * @brief 单件
     * @tparam T 类型
     */
    template <typename T>
    class Singleton :
        public NonCopyable
    {
    public:
        static T& Instance()
        {
            static T s_stInstance;  // C++11保证对该static的初始化操作是原子的
            return s_stInstance;
        }
    };

    /**
     * @brief ScopeGuard
     * @tparam T 函数类型
     */
    template <typename T>
    class ScopeExit :
        public NonCopyable
    {
    public:
        explicit ScopeExit(const T& func)
            : m_stFunc(func) {}
        explicit ScopeExit(T&& func)
            : m_stFunc(std::move(func)) {}

        ScopeExit(ScopeExit&& rhs)
            : m_stFunc(std::move(rhs.m_stFunc)), m_bDismiss(rhs.m_bDismiss) {}

        ~ScopeExit()noexcept
        {
            if (!m_bDismiss)
            {
                if (m_stFunc)
                    m_stFunc();
            }
        }

    public:
        void Dismiss()noexcept { m_bDismiss = true; }

    private:
        T m_stFunc;
        bool m_bDismiss = false;
    };

    /**
     * @brief 读取整个文件（二进制的）
     * @exception IOException 打开文件失败抛出
     * @param[out] out 目标缓冲区
     * @param path 路径
     * @return 数据，即out
     */
    std::string& ReadWholeFile(std::string& out, const char* path);

    inline std::string ReadWholeFile(const char* path)
    {
        std::string ret;
        ReadWholeFile(ret, path);
        return ret;
    }

    inline std::string ReadWholeFile(const std::string& path)
    {
        std::string ret;
        ReadWholeFile(ret, path.c_str());
        return ret;
    }
}
