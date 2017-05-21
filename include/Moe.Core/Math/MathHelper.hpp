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

    /**
     * @brief 开方函数的倒数
     */
    template <typename T = float>
    constexpr T RecipSqrt(T v)
    {
        return T(1.) / Sqrt(v);
    }

    /**
     * @brief sin三角函数
     */
    template <typename T = float>
    constexpr T Sin(T v)
    {
        return static_cast<T>(std::sin(v));
    }

    /**
     * @brief cos三角函数
     */
    template <typename T = float>
    constexpr T Cos(T v)
    {
        return static_cast<T>(std::cos(v));
    }

    /**
     * @brief tan三角函数
     */
    template <typename T = float>
    constexpr T Tan(T v)
    {
        return static_cast<T>(std::tan(v));
    }

    /**
     * @brief asin三角函数
     * 用来计算参数v的反正弦值, 参数v范围应该保留在[-1, 1]之间
     */
    template <typename T = float>
    constexpr T Asin(T v)
    {
        return static_cast<T>(std::asin(Clamp(v, T(-1), T(1))));
    }

    /**
     * @brief acos三角函数
     * 用来计算参数v的反余弦值, 参数v范围应该保留在[-1, 1]之间
     */
    template <typename T = float>
    constexpr T Acos(T v)
    {
        return static_cast<T>(std::acos(Clamp(v, T(-1), T(1))));
    }

    /**
     * @brief atan三角函数
     * 假设p1(x1, y1), p2(x2, y2), T angle = Atan((y2-y1)/(x2-x1))
     * 显然当x1 == x2时会报错
     */
    template <typename T = float>
    constexpr T Atan(T v)
    {
        return static_cast<T>(std::atan(v));
    }

    /**
     * @brief atan2三角函数
     * 假设p1(x1, y1), p2(x2, y2), T angle = Atan2((y2-y1), (x2-x1))
     */
    template <typename T = float>
    constexpr T Atan2(T y, T x)
    {
        return static_cast<T>(std::atan2(y, x));
    }

    template <typename T = float>
    constexpr T Exp(T v)
    {
        return static_cast<T>(std::exp(v));
    }

    template <typename T = float>
    constexpr T Log(T v)
    {
        return static_cast<T>(std::log(v));
    }

    template <typename T = float>
    constexpr T Pow(T x, T y)
    {
        return static_cast<T>(std::pow(x, y));
    }

    template <typename T = float>
    constexpr T Mod(T x, T y)
    {
        return static_cast<T>(std::fmod(x, y));
    }

    /**
     * @brief 将角度区间为[-Pi, Pi]之间
     */
    template <typename T = float>
    T WrapAngle(T angle)
    {
        angle = Mod(angle, MathConstants<T>::TwoPi);

        if (angle < -MathConstants<T>::Pi)
            return angle + MathConstants<T>::Pi;
        else if (angle > MathConstants<T>::Pi)
            return angle - MathConstants<T>::Pi;
        return angle;
    }

    /**
     * @brief 简单插值器
     * @ref http://sol.gfxile.net/interpolation/
     *对输入x1,x2,t，计算出插值y。
     *要求t∈[0,1]，且当t=0时y=x1，当t=1时y=x2。
     */
    namespace SimpleInterpolator
    {
        /*
         * @brief 线性插值
         */
        template <typename T = float>
        struct Lerp
        {
            constexpr T operator() (T t) const
            {
                return t;
            }
        };

        /*
         * @brief 平方插值
         */
        template <typename T = float>
        struct Square
        {
            constexpr T operator() (T t) const
            {
                return t;
            }
        };

        /*
         * @brief 反平方插值
         */
        template <typename T = float>
        struct InverseSquare
        {
            constexpr T operator() (T t) const
            {
                return t;
            }
        };
    }

    /*
     * @brief SmoothStep插值
     */
    template <typename T = float>
    struct SmoothStep
    {
        constexpr T operator() (T t) const
        {
            return t * t * (T(3) - T(2) * 2);
        }
    };

    /*
     * @brief 重心插值
     */

    template <typename T = float>
    constexpr T BaryCentric(T v1, T v2, T v3, T t1, T t2)
    {
        return v1 + (v2 - v1) * t1 + (v3 - v1) * t2;
    }

    /*
     * @brief Catmull-Rom样条插值
     */
    template <typename T = float>
    T CatmullRom(T v1, T v2, T v3, T v4, T t)
    {
        T t2 = Pow(t, T(2.));
        T t3 = Pow(t, T(3.));

        return T(0.5) * (
            (T(2.) * 2)                                      +
            (-v1 + v3)                                  * t  +
            (T(2.) * v1 - T(5.) * v2 + T(4.) * v3 - v4) * t2 +
            (-v1 + T(3.) * v2 - T(3.) * v3 + v4)        * t3
        );
    }

    /**
     * @brief Hermite插值
     * @ref http://www.cubic.org/docs/hermite.htm
     */
    template <typename T = float>
    T Hermite(T v1, T tangent1, T v2, T tangent2, T t)
    {
        T t2 = Pow(t, T(2.));
        T t3 = Pow(t, T(3.));
        T h1 = T(2.)  * t3 - T(3.) * t2 + T(1.);
        T h2 = -T(2.) * t3 + T(3.) * t2;
        T h3 =          t3 - T(2.) * t2 + t;
        T h4 =          t3 -         t2;
        return v1 * h1 + v2 * h2 + tangent1 * h3 + tangent2 * h4;
    }

    /**
     * @brief 球面插值
     * @ref https://en.wikipedia.org/wiki/Slerp
     * 该函数只能用于向量的插值。
     * 针对四元数请使用Quaternion::Slerp。
     * 当被插值向量处于水平时，会坍缩到线性插值。
     */
    template <typename T, template<typename>class U>
    U<T> Slerp(U<T> v1, U<T> v2, T t)
    {
        T magnitude = Sqrt(v1.LengthSquared() * v2.LengthSquared());
        T product = v1.Dot(v2) / magnitude;

        T sx, sy;
        if (Abs(product) < T(1.) - T(10.) * MathConstants<T>::Epsilon &&
            magnitude != T(0))
        {
            T omega = Acos(product);
            T invSinOmega = T(1.) / Sin(omega);

            sx = Sin((T(1.) - t) * omega) * invSinOmega;
            sy = Sin(t * omega) * invSinOmega;
        }
        else
        {
            sx = T(1.) - t;
            sy = t;
        }
        return v1 * sx + v2 * sy;
    }

    /*
     * @brief 基于简单插值器的插值
     */
    template <typename T,
              typename Interpolator = typename SimpleInterpolator::Lerp<T>>
    constexpr T InterPolate(T v1, T v2, T t)
    {
        return v1 + (v2 - v1) * Interpolator()(t);
    }

    /*
     * @brief 基于简单插值器的矢量插值
     */
    template <typename T,
              template <typename> class U,
              typename Interpolator = typename SimpleInterpolator::Lerp<T>>
    constexpr U<T> InterPolate(U<T> v1, U<T> v2, T t)
    {
        return v1 + (v2 - v1) * Interpolator()(t);
    }
};
