/**
 * @file
 * @brief 数学辅助函数库
 * @date 2017/5/5
 */
#pragma once
#include <cmath>
#include <algorithm>
#include <type_traits>
#include <limits>

namespace moe
{
    /**
     * @brief 数学常量模板
     * @tparam T 数值类型
     */
    template <typename T = float>
    struct MathConstants
    {
        constexpr static const T Pi = T(3.14159265358979323846);
        constexpr static const T PiOver2 = Pi / T(2.);
        constexpr static const T PiOver4 = Pi / T(4.);
        constexpr static const T TwoPi = Pi * T(2.);

        constexpr static const T E = T(2.71828182845904523536);

        constexpr static const T SqrtHalf = T(0.7071067811865475244008443621048490);

        constexpr static const T Epsilon = std::numeric_limits<T>::epsilon();
    };

    constexpr static const float Pi = MathConstants<>::Pi;
    constexpr static const float PiOver2 = MathConstants<>::PiOver2;
    constexpr static const float PiOver4 = MathConstants<>::PiOver4;
    constexpr static const float TwoPi = MathConstants<>::TwoPi;

    template <typename T = float>
    constexpr T Abs(T v)
    {
        return v >= T(0) ? v : -v;
    }

    template <typename T = float>
    constexpr T Min(T v1, T v2)
    {
        return v1 <= v2 ? v1 : v2;
    }

    template <typename T = float>
    constexpr T Max(T v1, T v2)
    {
        return v1 >= v2 ? v1 : v2;
    }

    /**
     * @brief 判断数值符号
     */
    template <typename T = float>
    constexpr T Sign(T v)
    {
        return v == T(0) ? T(0) : (v > T(0) ? T(1) : T(-1));
    }

    /**
     * @brief 开平方函数
     */
    template <typename T = float>
    constexpr T Sqrt(T v)
    {
        return static_cast<T>(std::sqrt(v));
    }

    /**
     * @brief 夹取
     * @tparam T 模板数值类型
     * @param v 被夹取值
     * @param minv 最小值
     * @param maxv 最大值
     *
     * 当value处于minv和maxv中间返回value, 大于maxv就返回maxv
     * 小于minv就返回minv
     */
    template <typename T = float>
    constexpr T Clamp(T v, T minv, T maxv)
    {
        return Min(Max(v, minv), maxv);
    }

    /**
     * @brief 弧度到角度
     *
     * 360角度 = 2Pi弧度
     */
    template <typename T = float>
    constexpr T ToDegrees(T radians)
    {
        return radians * T(180.) / MathConstants<T>::Pi;
    }

    /**
     * @brief 角度到弧度
     */
    template <typename T = float>
    constexpr T ToRadians(T degrees)
    {
        return MathConstants<T>::Pi * T(180.) / degrees;
    }
};
