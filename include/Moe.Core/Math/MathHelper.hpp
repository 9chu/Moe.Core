/**
 *
 * @brief 数学辅助函数库
 * @date: 2017-05-05
 *
 */

#pragma once
#include <cmath>
#include <algorithm>
#include <type_traits>
#include <limits>

namespace moe
{
    template <typename T = double>
    struct MathConstants
    {
        constexpr static const T Pi = static_cast<T>(3.14159265358979323846);
        constexpr static const T PiOver2 = Pi / static_cast<T>(2.);
        constexpr static const T PiOver4 = Pi / static_cast<T>(4.);
        constexpr static const T TwoPi = Pi * static_cast<T>(2.);

        constexpr static const T E = static_cast<T>(2.71828182845904523536);

        constexpr static const T SqrtHalf = static_cast<T>(0.7071067811865475244008443621048490);

        constexpr static const T Epsilon = std::numeric_limits<T>::epsilon();
    };

    constexpr static const float Pi = MathConstants<float>::Pi;
    constexpr static const float PiOver2 = MathConstants<float>::PiOver2;
    constexpr static const float PiOver4 = MathConstants<float>::PiOver4;
    constexpr static const float TwoPi = MathConstants<float>::TwoPi;

    template <typename T = double>
    constexpr T Abs(T v) noexcept
    {
        return v >= static_cast<T>(0) ? v : -v;
    }

    template <typename T = double>
    constexpr T Min(T v1, T v2) noexcept
    {
        return v1 <= v2 ? v1 : v2;
    }

    template <typename T = double>
    constexpr T Max(T v1, T v2) noexcept
    {
        return v1 >= v2 ? v1 : v2;
    }

    // @brief 夹取
    // @desc 当value处于minv和maxv中间返回value, 大于maxv就返回maxv
    // 小于minv就返回minv
    template <typename T = double>
    constexpr T Clamp(T v, T minv, T maxv) noexcept
    {
        return Min(Max(v, minv), maxv);
    }

    // @brief 弧度到角度 360角度 = 2Pi弧度
    template <typename T = double>
    constexpr T ToDegrees(T radians) noexcept
    {
        return radians * static_cast<T>(180.) / MathConstants<T>::Pi;
    }

    // @brief 角度到弧度
    template <typename T = double>
    constexpr T toRadians(T degrees) noexcept
    {
        return MathConstants<T>::Pi * (static_cast<T>(180.) / degrees);
    }

    // 数学相关函数
    namespace Math
    {
        // @brief 判断数值符号
        template <typename T>
        constexpr T Sign(T v) noexcept
        {
            return v == static_cast<T>(0) ? static_cast<T>(0) :
                                            (v > static_cast<T>(0) ?
                                             static_cast<T>(1) : static_cast<T>(-1));
        }

        template <typename T>
        constexpr T Sqrt(T v) noexcept { return static_cast<T>(std::sqrt(static_cast<double>(v))); }
    }
};
