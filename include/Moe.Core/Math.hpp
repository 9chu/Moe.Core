/**
 * @file
 * @date 2017/6/14
 * @see https://github.com/opentk/opentk/blob/master/src/OpenTK.Mathematics/Matrix/Matrix4.cs
 */
#pragma once
#include <cmath>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <type_traits>
#include <limits>

namespace moe
{
    namespace Math
    {
        //////////////////////////////////////// <editor-fold desc="位运算技巧">

        /**
         * @brief 计算不小于v的最近的二次幂值
         * @param v 输入
         * @return 不小于v的二次幂值
         *
         * 例如：
         *   输入0 返回1
         *   输入7 返回8
         *   输入16 返回16
         */
        inline uint32_t NextPowerOf2(uint32_t v)
        {
            v--;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            v |= v >> 8;
            v |= v >> 16;
            v++;
            v += (v == 0);
            return v;
        }

        //////////////////////////////////////// </editor-fold>
        //////////////////////////////////////// <editor-fold desc="数学常数">

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

        //////////////////////////////////////// </editor-fold>
        //////////////////////////////////////// <editor-fold desc="基本数学函数">

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
         * @brief 符号函数
         *
         * 当 v > 0 时返回 1
         * 当 v < 0 时返回 -1
         * 当 v == 0 时返回 0
         */
        template <typename T = float>
        constexpr T Sign(T v)
        {
            return v > T(0) ? T(1) : (v < T(0) ? T(-1) : T(0));
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
         * @tparam T 数值类型
         * @param v 被夹取值
         * @param minv 最小值
         * @param maxv 最大值
         *
         * 当 v > maxv 时返回 maxv
         * 当 v < minv 时返回 minv
         * 否则返回 v
         */
        template <typename T = float>
        constexpr T Clamp(T v, T minv, T maxv)
        {
            return Min(Max(v, minv), maxv);
        }

        /**
         * @brief 弧度到角度
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

        template <typename T = float>
        constexpr T Sin(T v)
        {
            return static_cast<T>(std::sin(v));
        }

        template <typename T = float>
        constexpr T Cos(T v)
        {
            return static_cast<T>(std::cos(v));
        }

        template <typename T = float>
        constexpr T Tan(T v)
        {
            return static_cast<T>(std::tan(v));
        }

        template <typename T = float>
        constexpr T Asin(T v)
        {
            return static_cast<T>(std::asin(Clamp(v, T(-1), T(1))));
        }

        template <typename T = float>
        constexpr T Acos(T v)
        {
            return static_cast<T>(std::acos(Clamp(v, T(-1), T(1))));
        }

        template <typename T = float>
        constexpr T Atan(T v)
        {
            return static_cast<T>(std::atan(v));
        }

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
         * @brief 转换角度到[-Pi, Pi]之间
         */
        template <typename T = float>
        inline T WrapAngle(T angle)
        {
            angle = Mod(angle, MathConstants<T>::TwoPi);

            if (angle < -MathConstants<T>::Pi)
                return angle + MathConstants<T>::Pi;
            else if (angle > MathConstants<T>::Pi)
                return angle - MathConstants<T>::Pi;
            return angle;
        }

        //////////////////////////////////////// </editor-fold>
        //////////////////////////////////////// <editor-fold desc="插值器">

        /**
         * @brief 简单插值器
         * @ref http://sol.gfxile.net/interpolation/
         *
         * 对输入x1,x2,t，计算出插值y。
         * 要求t∈[0,1]，且当t=0时y=x1，当t=1时y=x2。
         */
        namespace SimpleInterpolator
        {
            /**
             * @brief 线性插值
             */
            template <typename T>
            struct Lerp
            {
                constexpr T operator()(T t)const
                {
                    return t;
                }
            };

            /**
             * @brief 平方插值
             */
            template <typename T>
            struct Square
            {
                constexpr T operator()(T t)const
                {
                    return t * t;
                }
            };

            /**
             * @brief 反平方插值
             */
            template <typename T>
            struct InverseSquare
            {
                constexpr T operator()(T t)const
                {
                    return T(1) - (T(1) - t) * (T(1) - t);
                }
            };

            /**
             * @brief SmoothStep插值
             */
            template <typename T>
            struct SmoothStep
            {
                constexpr T operator()(T t)const
                {
                    return t * t * (T(3) - T(2) * t);
                }
            };
        }

        /**
         * @brief 重心插值
         */
        template <typename T>
        constexpr T Barycentric(T v1, T v2, T v3, T t1, T t2)
        {
            return v1 + (v2 - v1) * t1 + (v3 - v1) * t2;
        }

        /**
         * @brief Catmull–Rom样条插值
         * @ref http://www.mvps.org/directx/articles/catmull/
         */
        template <typename T>
        inline T CatmullRom(T v1, T v2, T v3, T v4, T t)
        {
            T t2 = t * t;
            T t3 = t2 * t;

            return ((T(2) * v2) +
                (-v1 + v3) * t +
                (T(2) * v1 - T(5) * v2 + T(4) * v3 - v4) * t2 +
                (-v1 + T(3) * v2 - T(3) * v3 + v4) * t3) * T(0.5);
        }

        /**
         * @brief Hermite插值
         * @ref http://www.cubic.org/docs/hermite.htm
         */
        template <typename T>
        inline T Hermite(T v1, T tangent1, T v2, T tangent2, T t)
        {
            T t2 = t * t;
            T t3 = t2 * t;
            T h1 = T(2) * t3 - T(3) * t2 + T(1);
            T h2 = -T(2) * t3 + T(3) * t2;
            T h3 = t3 - T(2) * t2 + t;
            T h4 = t3 - t2;

            return v1 * h1 + v2 * h2 + tangent1 * h3 + tangent2 * h4;
        }

        /**
         * @brief 球面插值
         * @ref https://en.wikipedia.org/wiki/Slerp
         *
         * 该函数只能用于向量的插值。
         * 针对四元数请使用Quaternion::Slerp。
         * 当被插值向量处于水平时，会坍缩到线性插值。
         */
        template <typename T, template <typename> class U>
        inline U<T> Slerp(U<T> v1, U<T> v2, T t)
        {
            T magnitude = Sqrt(v1.LengthSquared() * v2.LengthSquared());
            T product = v1.Dot(v2) / magnitude;

            T sx, sy;
            if (Abs(product) < T(1) - T(10) * MathConstants<T>::Epsilon && magnitude != T(0))
            {
                T omega = Acos(product);
                T invSinOmega = T(1) / Sin(omega);

                sx = Sin((T(1) - t) * omega) * invSinOmega;
                sy = Sin(t * omega) * invSinOmega;
            }
            else  // 180度时特殊处理，变成线性插值
            {
                sx = T(1) - t;
                sy = t;
            }

            return v1 * sx + v2 * sy;
        }

        /**
         * @brief 基于简单插值器的插值
         */
        template <typename T, typename Interpolator = typename SimpleInterpolator::Lerp<T>>
        constexpr T Interpolate(T v1, T v2, T t)
        {
            return v1 + (v2 - v1) * Interpolator()(t);
        }

        /**
         * @brief 基于简单插值器的插值(矢量版本)
         */
        template <typename T, template <typename> class U,
            typename Interpolator = typename SimpleInterpolator::Lerp<T>>
        constexpr U<T> Interpolate(U<T> v1, U<T> v2, T t)
        {
            return v1 + (v2 - v1) * Interpolator()(t);
        }

        //////////////////////////////////////// </editor-fold>
        //////////////////////////////////////// <editor-fold desc="二维向量">

        /**
         * @brief 二维向量
         *
         * 定义了二维向量模板和定义在二维向量上的运算。
         * 对于二维向量，所有的运算符重载均以分量为单位进行运算。
         * 针对点乘需要调用Dot进行。
         */
        template <typename T = float>
        class Vector2
        {
            static_assert(std::is_arithmetic<T>::value, "T must be a numerical type.");

        public:
            T x, y;

        public:
            constexpr Vector2()
                : x(T(0)), y(T(0)) {}
            constexpr Vector2(T v)
                : x(v), y(v) {}
            constexpr Vector2(T vx, T vy)
                : x(vx), y(vy) {}

            Vector2(const Vector2& rhs)noexcept = default;
            Vector2(Vector2&& rhs)noexcept = default;

            Vector2& operator=(const Vector2& rhs)noexcept = default;
            Vector2& operator=(Vector2&& rhs)noexcept = default;

            T& operator[](unsigned index)
            {
                static T s_Invalid;

                switch (index)
                {
                    case 0:
                        return x;
                    case 1:
                        return y;
                    default:
                        assert(false);
                        return s_Invalid;
                }
            }

            T operator[](unsigned index)const
            {
                switch (index)
                {
                    case 0:
                        return x;
                    case 1:
                        return y;
                    default:
                        assert(false);
                        return 0;
                }
            }

        public:
            T SetX(T vx)
            {
                std::swap(x, vx);
                return vx;
            }

            T SetY(T vy)
            {
                std::swap(y, vy);
                return vy;
            }

            void Set(T vx, T vy)
            {
                x = vx;
                y = vy;
            }

            void SetZero()
            {
                x = T(0);
                y = T(0);
            }

            T SetNormalize()
            {
                T len = Length();
                assert(len != 0);
                x /= len;
                y /= len;
                return len;
            }

            void SetAbsolute()
            {
                x = Abs(x);
                y = Abs(y);
            }

            constexpr bool IsZero()const
            {
                return x == T(0) && y == T(0);
            }

            constexpr T Length()const
            {
                return Math::Sqrt(LengthSquared());
            }

            constexpr T LengthSquared()const
            {
                return x * x + y * y;
            }

            constexpr T Distance(Vector2 rhs)const
            {
                return (rhs - *this).Length();
            }

            constexpr T DistanceSquared(Vector2 rhs)const
            {
                return (rhs - *this).LengthSquared();
            }

            Vector2 Normalize()const
            {
                T len = Length();
                assert(len != 0);
                return Vector2(x / len, y / len);
            }

            constexpr T Dot(Vector2 rhs)const
            {
                return x * rhs.x + y * rhs.y;
            }

            constexpr T Cross(Vector2 rhs)const
            {
                return x * rhs.y - rhs.x * y;
            }

            /**
             * @brief 求与另一个向量的夹角
             * @return 返回值在[0, π]之间
             */
            T Angle(Vector2 rhs)const
            {
                T s = Math::Sqrt(LengthSquared() * rhs.LengthSquared());
                assert(s != 0);
                return Math::Acos(Dot(rhs) / s);
            }

            /**
             * @brief 旋转向量(逆时针)
             * @param[in] angle 角度
             * @return 旋转后向量
             */
            Vector2 Rotate(T angle)const
            {
                T sinA = Math::Sin(angle);
                T cosA = Math::Cos(angle);

                return Vector2(
                    x * cosA - y * sinA,
                    x * sinA + y * cosA
                );
            }

            /**
             * @brief 计算反射向量
             * @ref http://mathworld.wolfram.com/Reflection.html
             * @param[in] normal 法向量（必须确保归一化）
             * @return 出射向量
             */
            Vector2 Reflect(Vector2 normal)const
            {
                assert(normal.Length() == 1);
                T d2 = T(normal.x * x + normal.y * y);  // r = v - 2 * n * (n . v)
                d2 = d2 + d2;
                return Vector2(x - d2 * normal.x, y - d2 * normal.y);
            }

        public:
            static const Vector2 UnitX;
            static const Vector2 UnitY;
            static const Vector2 One;
            static const Vector2 Zero;
        };

        template <typename T>
        const Vector2<T> Vector2<T>::UnitX = Vector2(T(1), T(0));

        template <typename T>
        const Vector2<T> Vector2<T>::UnitY = Vector2(T(0), T(1));

        template <typename T>
        const Vector2<T> Vector2<T>::One = Vector2(T(1));

        template <typename T>
        const Vector2<T> Vector2<T>::Zero = Vector2(T(0));

        template <typename T>
        constexpr Vector2<T> operator+(Vector2<T> v)
        {
            return Vector2<T>(+v.x, +v.y);
        }

        template <typename T>
        constexpr Vector2<T> operator-(Vector2<T> v)
        {
            return Vector2<T>(-v.x, -v.y);
        }

        template <typename T>
        constexpr Vector2<T> operator+(Vector2<T> lhs, Vector2<T> rhs)
        {
            return Vector2<T>(lhs.x + rhs.x, lhs.y + rhs.y);
        }

        template <typename T>
        constexpr Vector2<T> operator-(Vector2<T> lhs, Vector2<T> rhs)
        {
            return Vector2<T>(lhs.x - rhs.x, lhs.y - rhs.y);
        }

        template <typename T>
        constexpr Vector2<T> operator*(Vector2<T> lhs, T rhs)
        {
            return Vector2<T>(lhs.x * rhs, lhs.y * rhs);
        }

        template <typename T>
        constexpr Vector2<T> operator*(T lhs, Vector2<T> rhs)
        {
            return Vector2<T>(lhs * rhs.x, lhs * rhs.y);
        }

        template <typename T>
        constexpr Vector2<T> operator*(Vector2<T> lhs, Vector2<T> rhs)
        {
            return Vector2<T>(lhs.x * rhs.x, lhs.y * rhs.y);
        }

        template <typename T>
        constexpr Vector2<T> operator/(Vector2<T> lhs, T rhs)
        {
            return Vector2<T>(lhs.x / rhs, lhs.y / rhs);
        }

        template <typename T>
        constexpr Vector2<T> operator/(T lhs, Vector2<T> rhs)
        {
            return Vector2<T>(lhs / rhs.x, lhs / rhs.y);
        }

        template <typename T>
        constexpr Vector2<T> operator/(Vector2<T> lhs, Vector2<T> rhs)
        {
            return Vector2<T>(lhs.x / rhs.x, lhs.y / rhs.y);
        }

        template <typename T>
        constexpr bool operator==(Vector2<T> lhs, Vector2<T> rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y;
        }

        template <typename T>
        constexpr bool operator!=(Vector2<T> lhs, Vector2<T> rhs)
        {
            return lhs.x != rhs.x || lhs.y != rhs.y;
        }

        template <typename T>
        Vector2<T>& operator+=(Vector2<T>& lhs, Vector2<T> rhs)
        {
            lhs.x += rhs.x;
            lhs.y += rhs.y;
            return lhs;
        }

        template <typename T>
        Vector2<T>& operator-=(Vector2<T>& lhs, Vector2<T> rhs)
        {
            lhs.x -= rhs.x;
            lhs.y -= rhs.y;
            return lhs;
        }

        template <typename T>
        Vector2<T>& operator*=(Vector2<T>& lhs, T rhs)
        {
            lhs.x *= rhs;
            lhs.y *= rhs;
            return lhs;
        }

        template <typename T>
        Vector2<T>& operator*=(Vector2<T>& lhs, Vector2<T> rhs)
        {
            lhs.x *= rhs.x;
            lhs.y *= rhs.y;
            return lhs;
        }

        template <typename T>
        Vector2<T>& operator/=(Vector2<T>& lhs, T rhs)
        {
            lhs.x /= rhs;
            lhs.y /= rhs;
            return lhs;
        }

        template <typename T>
        Vector2<T>& operator/=(Vector2<T>& lhs, Vector2<T> rhs)
        {
            lhs.x /= rhs.x;
            lhs.y /= rhs.y;
            return lhs;
        }

        template <typename T>
        constexpr Vector2<T> Abs(Vector2<T> v)
        {
            return Vector2<T>(Abs(v.x), Abs(v.y));
        }

        template <typename T>
        constexpr Vector2<T> Min(Vector2<T> v1, Vector2<T> v2)
        {
            return Vector2<T>(Min(v1.x, v2.x), Min(v1.y, v2.y));
        }

        template <typename T>
        constexpr Vector2<T> Max(Vector2<T> v1, Vector2<T> v2)
        {
            return Vector2<T>(Max(v1.x, v2.x), Max(v1.y, v2.y));
        }

        template <typename T>
        constexpr Vector2<T> Clamp(Vector2<T> v, Vector2<T> minv, Vector2<T> maxv)
        {
            return Vector2<T>(Clamp(v.x, minv.x, maxv.x), Clamp(v.y, minv.y, maxv.y));
        }

        //////////////////////////////////////// </editor-fold>
        //////////////////////////////////////// <editor-fold desc="三维向量">

        /**
         * @brief 三维向量
         *
         * 定义了三维向量模板和定义在三维向量上的运算。
         * 对于三维向量，所有的运算符重载均以分量为单位进行运算。
         * 针对点乘和叉乘需要调用Dot和Cross进行。
         */
        template <typename T = float>
        class Vector3
        {
            static_assert(std::is_arithmetic<T>::value, "T must be a numerical type.");

        public:
            T x, y, z;

        public:
            constexpr Vector3()
                : x(T(0)), y(T(0)), z(T(0)) {}
            constexpr Vector3(T v)
                : x(v), y(v), z(v) {}
            constexpr Vector3(T vx, T vy, T vz)
                : x(vx), y(vy), z(vz) {}
            constexpr Vector3(Vector2<T> v, T vz)
                : x(v.x), y(v.y), z(vz) {}

            Vector3(const Vector3& rhs)noexcept = default;
            Vector3(Vector3&& rhs)noexcept = default;

            Vector3& operator=(const Vector3& rhs)noexcept = default;
            Vector3& operator=(Vector3&& rhs)noexcept = default;

            T& operator[](unsigned index)
            {
                static T s_Invalid;

                switch (index)
                {
                    case 0:
                        return x;
                    case 1:
                        return y;
                    case 2:
                        return z;
                    default:
                        assert(false);
                        return s_Invalid;
                }
            }

            T operator[](unsigned index)const
            {
                switch (index)
                {
                    case 0:
                        return x;
                    case 1:
                        return y;
                    case 2:
                        return z;
                    default:
                        assert(false);
                        return 0;
                }
            }
        public:
            T SetX(T vx)
            {
                std::swap(x, vx);
                return vx;
            }

            T SetY(T vy)
            {
                std::swap(y, vy);
                return vy;
            }

            T SetZ(T vz)
            {
                std::swap(z, vz);
                return vz;
            }

            void Set(T vx, T vy, T vz)
            {
                x = vx;
                y = vy;
                z = vz;
            }

            void Set(Vector2<T> v, T vz)
            {
                x = v.x;
                y = v.y;
                z = vz;
            }

            void SetZero()
            {
                x = T(0);
                y = T(0);
                z = T(0);
            }

            T SetNormalize()
            {
                T len = Length();
                // assert(len != 0);
                x /= len;
                y /= len;
                z /= len;
                return len;
            }

            void SetAbsolute()
            {
                x = Abs(x);
                y = Abs(y);
                z = Abs(z);
            }

            constexpr bool IsZero()const
            {
                return x == T(0) && y == T(0) && z == T(0);
            }

            constexpr T Length()const
            {
                return Math::Sqrt(LengthSquared());
            }

            constexpr T LengthSquared()const
            {
                return x * x + y * y + z * z;
            }

            constexpr T Distance(Vector3 rhs)const
            {
                return (rhs - *this).Length();
            }

            constexpr T DistanceSquared(Vector3 rhs)const
            {
                return (rhs - *this).LengthSquared();
            }

            Vector3 Normalize()const
            {
                T len = Length();
                assert(len != 0);
                return Vector3(x / len, y / len, z / len);
            }

            constexpr T Dot(Vector3 rhs)const
            {
                return x * rhs.x + y * rhs.y + z * rhs.z;
            }

            constexpr Vector3 Cross(Vector3 rhs)const
            {
                return Vector3(
                    y * rhs.z - z * rhs.y,
                    z * rhs.x - x * rhs.z,
                    x * rhs.y - y * rhs.x);
            }

            /**
             * @brief 求与另一个向量的夹角
             * @return 返回值在[0, π]之间
             */
            T Angle(Vector3 rhs)const
            {
                T s = Math::Sqrt(LengthSquared() * rhs.LengthSquared());
                assert(s != 0);
                return Math::Acos(Dot(rhs) / s);
            }

            /**
             * @brief 旋转向量(相对旋转轴正方向逆时针)
             * @param[in] axis 旋转轴
             * @param[in] angle 角度
             * @return 旋转后向量
             */
            Vector3 Rotate(Vector3 axis, T angle)const
            {
                Vector3 o = axis * axis.Dot(*this);
                Vector3 v1 = *this - o;
                Vector3 v2 = axis.Cross(*this);
                return (o + v1 * Math::Cos(angle) + v2 * Math::Sin(angle));
            }

            /**
             * @brief 计算反射向量
             * @ref http://mathworld.wolfram.com/Reflection.html
             * @param[in] normal 法向量（必须确保归一化）
             * @return 出射向量
             */
            Vector3 Reflect(Vector3 normal)const
            {
                assert(normal.Length() == 1);
                // r = v - 2 * n * (n . v)
                T d2 = T(normal.x * x + normal.y * y + normal.z * z);
                d2 = d2 + d2;
                return Vector3(
                    x - d2 * normal.x,
                    y - d2 * normal.y,
                    z - d2 * normal.z);
            }

        public:
            static const Vector3 UnitX;
            static const Vector3 UnitY;
            static const Vector3 UnitZ;
            static const Vector3 One;
            static const Vector3 Zero;
            static const Vector3 Right;
            static const Vector3 Left;
            static const Vector3 Up;
            static const Vector3 Down;
            static const Vector3 Backward;
            static const Vector3 Forward;

            /**
             * @brief 计算平面空间
             * @param[in] n 法向量，请确保为单位长度
             * @param[out] p 平面上的轴1
             * @param[out] q 平面上的轴2
             *
             * 以n为法向量计算平面上的正交轴p和q。
             */
            static void PlaneSpace(Vector3<T> n, Vector3<T>& p, Vector3<T>& q)
            {
                if (Abs(n.z) > MathConstants<T>::SqrtHalf)
                {
                    // 在Y-Z平面中选择p
                    T a = n.y * n.y + n.z * n.z;
                    T k = Math::RecipSqrt(a);
                    p.x = T(0);
                    p.y = -n.z * k;
                    p.z = n.y * k;
                    // q = n x p
                    q.x = a * k;
                    q.y = -n.x * p.z;
                    q.z = n.x * p.y;
                }
                else
                {
                    // 在X-Y平面中选择p
                    T a = n.x * n.x + n.y * n.y;
                    T k = Math::RecipSqrt(a);
                    p.x = -n.y * k;
                    p.y = n.x * k;
                    p.z = T(0);
                    // q = n x p
                    q.x = -n.z * p.y;
                    q.y = n.z * p.x;
                    q.z = a * k;
                }
            }
        };

        template <typename T>
        const Vector3<T> Vector3<T>::UnitX = Vector3(T(1), T(0), T(0));

        template <typename T>
        const Vector3<T> Vector3<T>::UnitY = Vector3(T(0), T(1), T(0));

        template <typename T>
        const Vector3<T> Vector3<T>::UnitZ = Vector3(T(0), T(0), T(1));

        template <typename T>
        const Vector3<T> Vector3<T>::One = Vector3(T(1));

        template <typename T>
        const Vector3<T> Vector3<T>::Zero = Vector3(T(0));

        template <typename T>
        const Vector3<T> Vector3<T>::Right = Vector3(T(1), T(0), T(0));

        template <typename T>
        const Vector3<T> Vector3<T>::Left = Vector3(T(-1), T(0), T(0));

        template <typename T>
        const Vector3<T> Vector3<T>::Up = Vector3(T(0), T(1), T(0));

        template <typename T>
        const Vector3<T> Vector3<T>::Down = Vector3(T(0), T(-1), T(0));

        template <typename T>
        const Vector3<T> Vector3<T>::Backward = Vector3(T(0), T(0), T(1));

        template <typename T>
        const Vector3<T> Vector3<T>::Forward = Vector3(T(0), T(0), T(-1));

        template <typename T>
        constexpr Vector3<T> operator+(Vector3<T> v)
        {
            return Vector3<T>(+v.x, +v.y, +v.z);
        }

        template <typename T>
        constexpr Vector3<T> operator-(Vector3<T> v)
        {
            return Vector3<T>(-v.x, -v.y, -v.z);
        }

        template <typename T>
        constexpr Vector3<T> operator+(Vector3<T> lhs, Vector3<T> rhs)
        {
            return Vector3<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
        }

        template <typename T>
        constexpr Vector3<T> operator-(Vector3<T> lhs, Vector3<T> rhs)
        {
            return Vector3<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
        }

        template <typename T>
        constexpr Vector3<T> operator*(Vector3<T> lhs, T rhs)
        {
            return Vector3<T>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
        }

        template <typename T>
        constexpr Vector3<T> operator*(T lhs, Vector3<T> rhs)
        {
            return Vector3<T>(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
        }

        template <typename T>
        constexpr Vector3<T> operator*(Vector3<T> lhs, Vector3<T> rhs)
        {
            return Vector3<T>(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
        }

        template <typename T>
        constexpr Vector3<T> operator/(Vector3<T> lhs, T rhs)
        {
            return Vector3<T>(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
        }

        template <typename T>
        constexpr Vector3<T> operator/(T lhs, Vector3<T> rhs)
        {
            return Vector3<T>(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z);
        }

        template <typename T>
        constexpr Vector3<T> operator/(Vector3<T> lhs, Vector3<T> rhs)
        {
            return Vector3<T>(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
        }

        template <typename T>
        constexpr bool operator==(Vector3<T> lhs, Vector3<T> rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
        }

        template <typename T>
        constexpr bool operator!=(Vector3<T> lhs, Vector3<T> rhs)
        {
            return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z;
        }

        template <typename T>
        Vector3<T>& operator+=(Vector3<T>& lhs, Vector3<T> rhs)
        {
            lhs.x += rhs.x;
            lhs.y += rhs.y;
            lhs.z += rhs.z;
            return lhs;
        }

        template <typename T>
        Vector3<T>& operator-=(Vector3<T>& lhs, Vector3<T> rhs)
        {
            lhs.x -= rhs.x;
            lhs.y -= rhs.y;
            lhs.z -= rhs.z;
            return lhs;
        }

        template <typename T>
        Vector3<T>& operator*=(Vector3<T>& lhs, T rhs)
        {
            lhs.x *= rhs;
            lhs.y *= rhs;
            lhs.z *= rhs;
            return lhs;
        }

        template <typename T>
        Vector3<T>& operator*=(Vector3<T>& lhs, Vector3<T> rhs)
        {
            lhs.x *= rhs.x;
            lhs.y *= rhs.y;
            lhs.z *= rhs.z;
            return lhs;
        }

        template <typename T>
        Vector3<T>& operator/=(Vector3<T>& lhs, T rhs)
        {
            lhs.x /= rhs;
            lhs.y /= rhs;
            lhs.z /= rhs;
            return lhs;
        }

        template <typename T>
        Vector3<T>& operator/=(Vector3<T>& lhs, Vector3<T> rhs)
        {
            lhs.x /= rhs.x;
            lhs.y /= rhs.y;
            lhs.z /= rhs.z;
            return lhs;
        }

        template <typename T>
        constexpr Vector3<T> Abs(Vector3<T> v)
        {
            return Vector3<T>(Abs(v.x), Abs(v.y), Abs(v.z));
        }

        template <typename T>
        constexpr Vector3<T> Min(Vector3<T> v1, Vector3<T> v2)
        {
            return Vector3<T>(Min(v1.x, v2.x), Min(v1.y, v2.y), Min(v1.z, v2.z));
        }

        template <typename T>
        constexpr Vector3<T> Max(Vector3<T> v1, Vector3<T> v2)
        {
            return Vector3<T>(Max(v1.x, v2.x), Max(v1.y, v2.y), Max(v1.z, v2.z));
        }

        template <typename T>
        constexpr Vector3<T> Clamp(Vector3<T> v, Vector3<T> minv, Vector3<T> maxv)
        {
            return Vector3<T>(
                Clamp(v.x, minv.x, maxv.x),
                Clamp(v.y, minv.y, maxv.y),
                Clamp(v.z, minv.z, maxv.z));
        }

        //////////////////////////////////////// </editor-fold>
        //////////////////////////////////////// <editor-fold desc="四维向量">

        /**
         * @brief 四维向量
         *
         * 定义了四维向量模板和定义在四维向量上的运算。
         * 对于四维向量，所有的运算符重载均以分量为单位进行运算。
         */
        template <typename T = float>
        class Vector4
        {
            static_assert(std::is_arithmetic<T>::value, "T must be a numerical type.");

        public:
            T x, y, z, w;

        public:
            constexpr Vector4()
                : x(T(0)), y(T(0)), z(T(0)), w(T(0)) {}
            constexpr Vector4(T v)
                : x(v), y(v), z(v), w(v) {}
            constexpr Vector4(T vx, T vy, T vz, T vw)
                : x(vx), y(vy), z(vz), w(vw) {}
            constexpr Vector4(Vector3<T> v, T vw)
                : x(v.x), y(v.y), z(v.z), w(vw) {}

            Vector4(const Vector4& rhs)noexcept = default;
            Vector4(Vector4&& rhs)noexcept = default;

            Vector4& operator=(const Vector4& rhs)noexcept = default;
            Vector4& operator=(Vector4&& rhs)noexcept = default;

            T& operator[](unsigned index)
            {
                static T s_Invalid;

                switch (index)
                {
                    case 0:
                        return x;
                    case 1:
                        return y;
                    case 2:
                        return z;
                    case 3:
                        return w;
                    default:
                        assert(false);
                        return s_Invalid;
                }
            }

            T operator[](unsigned index)const
            {
                switch (index)
                {
                    case 0:
                        return x;
                    case 1:
                        return y;
                    case 2:
                        return z;
                    case 3:
                        return w;
                    default:
                        assert(false);
                        return 0;
                }
            }
        public:
            T SetX(T vx)
            {
                std::swap(x, vx);
                return vx;
            }

            T SetY(T vy)
            {
                std::swap(y, vy);
                return vy;
            }

            T SetZ(T vz)
            {
                std::swap(z, vz);
                return vz;
            }

            T SetW(T vw)
            {
                std::swap(w, vw);
                return vw;
            }

            void Set(T vx, T vy, T vz, T vw)
            {
                x = vx;
                y = vy;
                z = vz;
                w = vw;
            }

            void Set(Vector3<T> v, T vw)
            {
                x = v.x;
                y = v.y;
                z = v.z;
                w = vw;
            }

            void SetZero()
            {
                x = T(0);
                y = T(0);
                z = T(0);
                w = T(0);
            }

            T SetNormalize()
            {
                T len = Length();
                // assert(len != 0);
                x /= len;
                y /= len;
                z /= len;
                w /= len;
                return len;
            }

            void SetAbsolute()
            {
                x = Abs(x);
                y = Abs(y);
                z = Abs(z);
                w = Abs(w);
            }

            constexpr bool IsZero()const
            {
                return x == T(0) && y == T(0) && z == T(0) && w == T(0);
            }

            constexpr T Length()const
            {
                return Math::Sqrt(LengthSquared());
            }

            constexpr T LengthSquared()const
            {
                return x * x + y * y + z * z + w * w;
            }

            constexpr T Distance(Vector4 rhs)const
            {
                return (rhs - *this).Length();
            }

            constexpr T DistanceSquared(Vector4 rhs)const
            {
                return (rhs - *this).LengthSquared();
            }

            Vector4 Normalize()const
            {
                T len = Length();
                assert(len != 0);
                return Vector4(x / len, y / len, z / len, w / len);
            }

            constexpr T Dot(Vector4 rhs)const
            {
                return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
            }

            /**
             * @brief 求与另一个向量的夹角
             * @return 返回值在[0, π]之间
             */
            T Angle(Vector4 rhs)const
            {
                T s = Math::Sqrt(LengthSquared() * rhs.LengthSquared());
                assert(s != 0);
                return Math::Acos(Dot(rhs) / s);
            }

        public:
            static const Vector4 UnitX;
            static const Vector4 UnitY;
            static const Vector4 UnitZ;
            static const Vector4 UnitW;
            static const Vector4 One;
            static const Vector4 Zero;
        };

        template <typename T>
        const Vector4<T> Vector4<T>::UnitX = Vector4(T(1), T(0), T(0), T(0));

        template <typename T>
        const Vector4<T> Vector4<T>::UnitY = Vector4(T(0), T(1), T(0), T(0));

        template <typename T>
        const Vector4<T> Vector4<T>::UnitZ = Vector4(T(0), T(0), T(1), T(0));

        template <typename T>
        const Vector4<T> Vector4<T>::UnitW = Vector4(T(0), T(0), T(0), T(1));

        template <typename T>
        const Vector4<T> Vector4<T>::One = Vector4(T(1));

        template <typename T>
        const Vector4<T> Vector4<T>::Zero = Vector4(T(0));

        template <typename T>
        constexpr Vector4<T> operator+(Vector4<T> v)
        {
            return Vector4<T>(+v.x, +v.y, +v.z, +v.w);
        }

        template <typename T>
        constexpr Vector4<T> operator-(Vector4<T> v)
        {
            return Vector4<T>(-v.x, -v.y, -v.z, -v.w);
        }

        template <typename T>
        constexpr Vector4<T> operator+(Vector4<T> lhs, Vector4<T> rhs)
        {
            return Vector4<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
        }

        template <typename T>
        constexpr Vector4<T> operator-(Vector4<T> lhs, Vector4<T> rhs)
        {
            return Vector4<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - lhs.w);
        }

        template <typename T>
        constexpr Vector4<T> operator*(Vector4<T> lhs, T rhs)
        {
            return Vector4<T>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);
        }

        template <typename T>
        constexpr Vector4<T> operator*(T lhs, Vector4<T> rhs)
        {
            return Vector4<T>(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
        }

        template <typename T>
        constexpr Vector4<T> operator*(Vector4<T> lhs, Vector4<T> rhs)
        {
            return Vector4<T>(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
        }

        template <typename T>
        constexpr Vector4<T> operator/(Vector4<T> lhs, T rhs)
        {
            return Vector4<T>(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs);
        }

        template <typename T>
        constexpr Vector4<T> operator/(T lhs, Vector4<T> rhs)
        {
            return Vector4<T>(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w);
        }

        template <typename T>
        constexpr Vector4<T> operator/(Vector4<T> lhs, Vector4<T> rhs)
        {
            return Vector4<T>(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
        }

        template <typename T>
        constexpr bool operator==(Vector4<T> lhs, Vector4<T> rhs)
        {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
        }

        template <typename T>
        constexpr bool operator!=(Vector4<T> lhs, Vector4<T> rhs)
        {
            return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z || lhs.w != rhs.w;
        }

        template <typename T>
        Vector4<T>& operator+=(Vector4<T>& lhs, Vector4<T> rhs)
        {
            lhs.x += rhs.x;
            lhs.y += rhs.y;
            lhs.z += rhs.z;
            lhs.w += rhs.w;
            return lhs;
        }

        template <typename T>
        Vector4<T>& operator-=(Vector4<T>& lhs, Vector4<T> rhs)
        {
            lhs.x -= rhs.x;
            lhs.y -= rhs.y;
            lhs.z -= rhs.z;
            lhs.w -= rhs.w;
            return lhs;
        }

        template <typename T>
        Vector4<T>& operator*=(Vector4<T>& lhs, T rhs)
        {
            lhs.x *= rhs;
            lhs.y *= rhs;
            lhs.z *= rhs;
            lhs.w *= rhs;
            return lhs;
        }

        template <typename T>
        Vector4<T>& operator*=(Vector4<T>& lhs, Vector4<T> rhs)
        {
            lhs.x *= rhs.x;
            lhs.y *= rhs.y;
            lhs.z *= rhs.z;
            lhs.w *= rhs.w;
            return lhs;
        }

        template <typename T>
        Vector4<T>& operator/=(Vector4<T>& lhs, T rhs)
        {
            lhs.x /= rhs;
            lhs.y /= rhs;
            lhs.z /= rhs;
            lhs.w /= rhs;
            return lhs;
        }

        template <typename T>
        Vector4<T>& operator/=(Vector4<T>& lhs, Vector4<T> rhs)
        {
            lhs.x /= rhs.x;
            lhs.y /= rhs.y;
            lhs.z /= rhs.z;
            lhs.w /= rhs.w;
            return lhs;
        }

        template <typename T>
        constexpr Vector4<T> Abs(Vector4<T> v)
        {
            return Vector4<T>(Abs(v.x), Abs(v.y), Abs(v.z), Abs(v.w));
        }

        template <typename T>
        constexpr Vector4<T> Min(Vector4<T> v1, Vector4<T> v2)
        {
            return Vector4<T>(Min(v1.x, v2.x), Min(v1.y, v2.y), Min(v1.z, v2.z), Min(v1.w, v2.w));
        }

        template <typename T>
        constexpr Vector4<T> Max(Vector4<T> v1, Vector4<T> v2)
        {
            return Vector4<T>(Max(v1.x, v2.x), Max(v1.y, v2.y), Max(v1.z, v2.z), Max(v1.w, v2.w));
        }

        template <typename T>
        constexpr Vector4<T> Clamp(Vector4<T> v, Vector4<T> minv, Vector4<T> maxv)
        {
            return Vector4<T>(
                Clamp(v.x, minv.x, maxv.x),
                Clamp(v.y, minv.y, maxv.y),
                Clamp(v.z, minv.z, maxv.z),
                Clamp(v.w, minv.w, maxv.w));
        }

        //////////////////////////////////////// </editor-fold>
        //////////////////////////////////////// <editor-fold desc="四元数">

        template <typename T>
        class Matrix4;

        /**
         * @brief 四元数
         *
         * 定义了四元数模板和定义在四元数上的运算。
         * 为确保四元数的数学意义，其乘法运算重载为四元数的乘法操作，而非分量相乘。
         * 其他运算符重载仍然以各个分量为计算单位。
         */
        template <typename T = float>
        class Quaternion
        {
            static_assert(std::is_arithmetic<T>::value, "T must be a numerical type.");

        public:
            T x, y, z, w;

        public:
            constexpr Quaternion()
                : x(T(0)), y(T(0)), z(T(0)), w(T(1)) {}
            constexpr Quaternion(T vx, T vy, T vz, T vw)
                : x(vx), y(vy), z(vz), w(vw) {}

            /**
             * @brief 构造四元数
             * @param[in] axis 旋转轴
             * @param[in] angle 旋转量
             */
            Quaternion(Vector3<T> axis, T angle)
            {
                SetRotation(axis, angle);
            }

            /**
             * @brief 构造四元数
             * @param[in] yaw Y轴旋转量
             * @param[in] pitch X轴旋转量
             * @param[in] roll Z轴旋转量
             *
             * 使用常见的定义，即旋转的先后顺序为Y-X-Z。
             */
            Quaternion(T yaw, T pitch, T roll)
            {
                SetEuler(yaw, pitch, roll);
            }

            /**
             * @brief 从矩阵计算四元数
             * @note 方法仅使用矩阵左上角的3x3部分
             * @param mat 旋转矩阵
             */
            Quaternion(const Matrix4<T>& mat)
            {
                auto trace = mat.GetTrace();

                if (trace > T(0))
                {
                    auto s = Sqrt(trace + T(1)) * T(2);
                    auto invS = T(1) / s;

                    w = s / T(4);
                    x = (mat.a[2][1] - mat.a[1][2]) * invS;
                    y = (mat.a[0][2] - mat.a[2][0]) * invS;
                    z = (mat.a[1][0] - mat.a[0][1]) * invS;
                }
                else
                {
                    auto m00 = mat.a[0][0], m11 = mat.a[1][1], m22 = mat.a[2][2];

                    if (m00 > m11 && m00 > m22)
                    {
                        auto s = Sqrt(T(1) + m00 - m11 - m22) * T(2);
                        auto invS = T(1) / s;

                        w = (mat.a[2][1] - mat.a[1][2]) * invS;
                        x = s / T(4);
                        y = (mat.a[0][1] + mat.a[1][0]) * invS;
                        z = (mat.a[0][2] + mat.a[2][0]) * invS;
                    }
                    else if (m11 > m22)
                    {
                        auto s = Sqrt(T(1) + m11 - m00 - m22) * T(2);
                        auto invS = T(1) / s;

                        w = (mat.a[0][2] - mat.a[2][0]) * invS;
                        x = (mat.a[0][1] + mat.a[1][0]) * invS;
                        y = s / T(4);
                        z = (mat.a[1][2] + mat.a[2][1]) * invS;
                    }
                    else
                    {
                        auto s = Sqrt(T(1) + m22 - m00 - m11) * T(2);
                        auto invS = T(1) / s;

                        w = (mat.a[1][0] - mat.a[0][1]) * invS;
                        x = (mat.a[0][2] + mat.a[2][0]) * invS;
                        y = (mat.a[1][2] + mat.a[2][1]) * invS;
                        z = s / T(4);
                    }
                }
            }

            Quaternion(const Quaternion& rhs)noexcept = default;
            Quaternion(Quaternion&& rhs)noexcept = default;

            Quaternion& operator=(const Quaternion& rhs)noexcept = default;
            Quaternion& operator=(Quaternion&& rhs)noexcept = default;

            T& operator[](unsigned index)
            {
                static T s_Invalid;

                switch (index)
                {
                    case 0:
                        return x;
                    case 1:
                        return y;
                    case 2:
                        return z;
                    case 3:
                        return w;
                    default:
                        assert(false);
                        return s_Invalid;
                }
            }

            T operator[](unsigned index)const
            {
                switch (index)
                {
                    case 0:
                        return x;
                    case 1:
                        return y;
                    case 2:
                        return z;
                    case 3:
                        return w;
                    default:
                        assert(false);
                        return 0;
                }
            }

        public:
            T SetX(T vx)
            {
                std::swap(x, vx);
                return vx;
            }

            T SetY(T vy)
            {
                std::swap(y, vy);
                return vy;
            }

            T SetZ(T vz)
            {
                std::swap(z, vz);
                return vz;
            }

            T SetW(T vw)
            {
                std::swap(w, vw);
                return vw;
            }

            void Set(T vx, T vy, T vz)
            {
                x = vx;
                y = vy;
                z = vz;
            }

            void Set(T vx, T vy, T vz, T vw)
            {
                x = vx;
                y = vy;
                z = vz;
                w = vw;
            }

            /**
             * @brief 设置旋转量
             * @param[in] axis 旋转轴
             * @param[in] angle 旋转量
             */
            void SetRotation(Vector3<T> axis, T angle)
            {
                T d = axis.Length();
                assert(d != 0);
                T s = Math::Sin(angle * T(0.5)) / d;

                x = axis.x * s;
                y = axis.y * s;
                z = axis.z * s;
                w = Math::Cos(angle * T(0.5));
            }

            /**
             * @brief 设置欧拉角
             * @param[in] yaw Y轴旋转量
             * @param[in] pitch X轴旋转量
             * @param[in] roll Z轴旋转量
             */
            void SetEuler(T yaw, T pitch, T roll)
            {
                T halfYaw = yaw * T(0.5);
                T halfPitch = pitch * T(0.5);
                T halfRoll = roll * T(0.5);
                T cosYaw = Math::Cos(halfYaw);
                T sinYaw = Math::Sin(halfYaw);
                T cosPitch = Math::Cos(halfPitch);
                T sinPitch = Math::Sin(halfPitch);
                T cosRoll = Math::Cos(halfRoll);
                T sinRoll = Math::Sin(halfRoll);

                x = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
                y = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
                z = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw;
                w = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
            }

            /**
             * @brief 设置欧拉角(ZYX)
             * @param[in] yaw Z轴旋转量
             * @param[in] pitch Y轴旋转量
             * @param[in] roll X轴旋转量
             */
            void SetEulerZYX(T yaw, T pitch, T roll)
            {
                T halfYaw = yaw * T(0.5);
                T halfPitch = pitch * T(0.5);
                T halfRoll = roll * T(0.5);
                T cosYaw = Math::Cos(halfYaw);
                T sinYaw = Math::Sin(halfYaw);
                T cosPitch = Math::Cos(halfPitch);
                T sinPitch = Math::Sin(halfPitch);
                T cosRoll = Math::Cos(halfRoll);
                T sinRoll = Math::Sin(halfRoll);

                x = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw;
                y = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
                z = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
                w = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
            }

            T SetNormalize()
            {
                T len = Length();
                assert(len != 0);
                x /= len;
                y /= len;
                z /= len;
                w /= len;
                return len;
            }

            void SetConjugate()
            {
                x = -x;
                y = -y;
                z = -z;
            }

            constexpr T Length()const
            {
                return Math::Sqrt(LengthSquared());
            }

            constexpr T LengthSquared()const
            {
                return x * x + y * y + z * z + w * w;
            }

            Quaternion Normalize()const
            {
                T len = Length();
                assert(len != 0);
                return Quaternion(x / len, y / len, z / len, w / len);
            }

            constexpr T Dot(Quaternion rhs)const
            {
                return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
            }

            /**
             * @brief 返回四元数本身的角度
             */
            constexpr T Angle()const
            {
                if (Abs(w) > T(1))
                {
                    auto q = Normalize();
                    return T(2) * Math::Acos(q.w);
                }
                return T(2) * Math::Acos(w);
            }

            /**
             * @brief 求与另一个四元数之间的夹角
             * @return 返回值在[0, π]之间
             */
            T Angle(Quaternion rhs)const
            {
                T s = Math::Sqrt(LengthSquared() * rhs.LengthSquared());
                // assert(s != 0);
                return Math::Acos(Dot(rhs) / s);
            }

            /**
             * @brief 返回旋转轴
             */
            Vector3<T> Axis()const
            {
                T squared = T(1) - w * w;

                if (squared < T(10) * MathConstants<T>::Epsilon)
                    return Vector3<T>::UnitX;  // 防止被0除，随便返回一个

                T s = T(1) / Math::Sqrt(squared);
                return Vector3<T>(x * s, y * s, z * s);
            }

            /**
             * @brief 取共轭
             */
            Quaternion Conjugate()const
            {
                return Quaternion(-x, -y, -z, w);
            }

            /**
             * @brief 取逆
             */
            Quaternion Inverse()const
            {
                assert(LengthSquared() != 0);
                return Conjugate() / LengthSquared();
            }

            /**
             * @brief 计算欧拉角
             * @param[in] yaw Y轴旋转量
             * @param[in] pitch X轴旋转量
             * @param[in] roll Z轴旋转量
             */
            void Euler(T& yaw, T& pitch, T& roll)
            {
                T x2 = x * x;

                yaw = Math::Atan2(T(2) * (w * z + x * y), T(1) - T(2) * (z * z + x2));
                pitch = Math::Asin(T(2) * (w * x - y * z));
                roll = Math::Atan2(T(2) * (w * y + z * x), T(1) - T(2) * (x2 + y * y));
            }

            /**
             * @brief 计算欧拉角
             * @return 返回<yaw, pitch, roll>
             */
            Vector3<T> Euler()
            {
                Vector3<T> ret;
                Euler(ret.x, ret.y, ret.z);
                return ret;
            }

            /**
             * @brief 计算欧拉角(ZYX)
             * @param[in] yaw Z轴旋转量
             * @param[in] pitch Y轴旋转量
             * @param[in] roll X轴旋转量
             */
            void EulerZYX(T& yaw, T& pitch, T& roll)
            {
                T y2 = y * y;

                yaw = Math::Atan2(T(2) * (w * x + y * z), T(1) - T(2) * (x * x + y2));
                pitch = Math::Asin(T(2) * (w * y - z * x));
                roll = Math::Atan2(T(2) * (w * z + x * y), T(1) - T(2) * (y2 + z * z));
            }

            /**
             * @brief 计算欧拉角
             * @return 返回<yaw, pitch, roll>
             */
            Vector3<T> EulerZYX()
            {
                Vector3<T> ret;
                EulerZYX(ret.x, ret.y, ret.z);
                return ret;
            }

            /**
             * @brief 转换到对应的变换矩阵
             */
            Matrix4<T> ToMatrix()
            {
                Quaternion<T> q = *this;

                if (Abs(w) > T(1))
                    q = Normalize();

                auto x2 = q.x + q.x;
                auto y2 = q.y + q.y;
                auto z2 = q.z + q.z;
                auto w2 = q.w + q.w;
                
                return Matrix4<T>(
                    T(1) - y2 * y - z2 * z, x2 * y + w2 * z, x2 * z - w2 * y, T(0),
                    x2 * y - w2 * z, T(1) - x2 * x - z2 * z, y2 * z + w2 * x, T(0),
                    x2 * z + w2 * y, y2 * z - w2 * x, T(1) - x2 * x - y2 * y, T(0),
                    T(0), T(0), T(0), T(1));
            }

        public:
            static const Quaternion Identity;

            /**
             * @brief 求解最短弧四元数
             * @ref 《游戏编程精粹I》 §2.10
             * @param[in] v0 待求解的起始向量
             * @param[in] v1 待求解的终止向量
             * @return 最短弧四元数
             *
             * 求解从起始向量旋转到终止向量的最短弧四元数。
             */
            static Quaternion ShortestArc(Vector3<T> v0, Vector3<T> v1)
            {
                v0.SetNormalize();
                v1.SetNormalize();

                Vector3<T> c = v0.Cross(v1);
                T d = v0.Dot(v1);

                if (d < T(-1) + MathConstants<T>::Epsilon)  // 180度情况
                {
                    Vector3<T> n, unused;
                    Vector3<T>::PlaneSpace(v0, n, unused);

                    // 在v0为法向量的平面上任意取一个向量即可
                    return Quaternion(n.x, n.y, n.z, T(0));
                }

                T s = Math::Sqrt((T(1) + d) * T(2));
                T rs = T(1) / s;

                return Quaternion(c.x * rs, c.y * rs, c.z * rs, s * T(0.5));
            }

            /**
             * @brief 对四元数进行球形插值
             * @param[in] q1 四元数1
             * @param[in] q2 四元数2
             * @param[in] t 插值量，若t=0则返回q1，若t=1则返回q2
             *
             * 当被插值四元数处于水平时，会直接返回q1。
             */
            static Quaternion Slerp(Quaternion q1, Quaternion q2, T t)
            {
                T magnitude = Math::Sqrt(q1.LengthSquared() * q2.LengthSquared());
                T product = q1.Dot(q2) / magnitude;

                if (Abs(product) < T(1) - T(10) * MathConstants<T>::Epsilon &&
                    magnitude != T(0))
                {
                    T sign = product < T(0) ? T(-1) : T(1);

                    T theta = Math::Acos(sign * product);
                    T s1 = Math::Sin(sign * t * theta);
                    T d = T(1) / Math::Sin(theta);
                    T s0 = Math::Sin((T(1) - t) * theta);

                    return Quaternion(
                        (q1.x * s0 + q2.x * s1) * d,
                        (q1.y * s0 + q2.y * s1) * d,
                        (q1.z * s0 + q2.z * s1) * d,
                        (q1.w * s0 + q2.w * s1) * d);
                }
                else
                    return q1;
            }
        };

        template <typename T>
        const Quaternion<T> Quaternion<T>::Identity = Quaternion(T(0), T(0), T(0), T(1));

        template <typename T>
        constexpr Quaternion<T> operator+(Quaternion<T> v)
        {
            return Quaternion<T>(+v.x, +v.y, +v.z, +v.w);
        }

        template <typename T>
        constexpr Quaternion<T> operator-(Quaternion<T> v)
        {
            return Quaternion<T>(-v.x, -v.y, -v.z, -v.w);
        }

        template <typename T>
        constexpr Quaternion<T> operator+(Quaternion<T> lhs, Quaternion<T> rhs)
        {
            return Quaternion<T>(
                lhs.x + rhs.x,
                lhs.y + rhs.y,
                lhs.z + rhs.z,
                lhs.w + rhs.w);
        }

        template <typename T>
        constexpr Quaternion<T> operator-(Quaternion<T> lhs, Quaternion<T> rhs)
        {
            return Quaternion<T>(
                lhs.x - rhs.x,
                lhs.y - rhs.y,
                lhs.z - rhs.z,
                lhs.w - rhs.w);
        }

        template <typename T>
        constexpr Quaternion<T> operator*(Quaternion<T> lhs, T rhs)
        {
            return Quaternion<T>(
                lhs.x * rhs,
                lhs.y * rhs,
                lhs.z * rhs,
                lhs.w * rhs);
        }

        template <typename T>
        constexpr Quaternion<T> operator*(T lhs, Quaternion<T> rhs)
        {
            return Quaternion<T>(
                lhs * rhs.x,
                lhs * rhs.y,
                lhs * rhs.z,
                lhs * rhs.w);
        }

        template <typename T>
        constexpr Quaternion<T> operator*(Quaternion<T> lhs, Quaternion<T> rhs)
        {
            return Quaternion<T>(
                lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
                lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z,
                lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x,
                lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z);
        }

        template <typename T>
        constexpr Quaternion<T> operator/(Quaternion<T> lhs, T rhs)
        {
            return Quaternion<T>(
                lhs.x / rhs,
                lhs.y / rhs,
                lhs.z / rhs,
                lhs.w / rhs);
        }

        template <typename T>
        constexpr Quaternion<T> operator/(T lhs, Quaternion<T> rhs)
        {
            return Quaternion<T>(
                lhs / rhs.x,
                lhs / rhs.y,
                lhs / rhs.z,
                lhs / rhs.w);
        }

        template <typename T>
        constexpr Quaternion<T> operator/(Quaternion<T> lhs, Quaternion<T> rhs)
        {
            return lhs * rhs.Inverse();
        }

        template <typename T>
        constexpr bool operator==(Quaternion<T> lhs, Quaternion<T> rhs)
        {
            return lhs.x == rhs.x &&
                lhs.y == rhs.y &&
                lhs.z == rhs.z &&
                lhs.w == rhs.w;
        }

        template <typename T>
        constexpr bool operator!=(Quaternion<T> lhs, Quaternion<T> rhs)
        {
            return lhs.x != rhs.x ||
                lhs.y != rhs.y ||
                lhs.z != rhs.z ||
                lhs.w != rhs.w;
        }

        template <typename T>
        Quaternion<T>& operator+=(Quaternion<T>& lhs, Quaternion<T> rhs)
        {
            lhs.x += rhs.x;
            lhs.y += rhs.y;
            lhs.z += rhs.z;
            lhs.w += rhs.w;
            return lhs;
        }

        template <typename T>
        Quaternion<T>& operator-=(Quaternion<T>& lhs, Quaternion<T> rhs)
        {
            lhs.x -= rhs.x;
            lhs.y -= rhs.y;
            lhs.z -= rhs.z;
            lhs.w -= rhs.w;
            return lhs;
        }

        template <typename T>
        Quaternion<T>& operator*=(Quaternion<T>& lhs, T rhs)
        {
            lhs.x *= rhs;
            lhs.y *= rhs;
            lhs.z *= rhs;
            lhs.w *= rhs;
            return lhs;
        }

        template <typename T>
        Quaternion<T>& operator*=(Quaternion<T>& lhs, Quaternion<T> rhs)
        {
            lhs = lhs * rhs;
            return lhs;
        }

        template <typename T>
        Quaternion<T>& operator/=(Quaternion<T>& lhs, T rhs)
        {
            lhs.x /= rhs;
            lhs.y /= rhs;
            lhs.z /= rhs;
            lhs.w /= rhs;
            return lhs;
        }

        template <typename T>
        Quaternion<T>& operator/=(Quaternion<T>& lhs, Quaternion<T> rhs)
        {
            lhs = lhs / rhs;
            return lhs;
        }

        //////////////////////////////////////// </editor-fold>
        //////////////////////////////////////// <editor-fold desc="矩阵">

        /**
         * @brief 4x4矩阵
         *
         * 定义了4x4矩阵模板和定义在矩阵上的运算。
         * 为确保矩阵的数学意义，其乘法运算重载为矩阵的乘法操作，而非分量相乘。
         * 以行优先存储。
         */
        template <typename T = float>
        class Matrix4
        {
            static_assert(std::is_arithmetic<T>::value, "T must be a numerical type.");

        public:
            static const unsigned kRow = 4;
            static const unsigned kColumn = 4;

            union {
                struct
                {
                    T m11, m12, m13, m14;
                    T m21, m22, m23, m24;
                    T m31, m32, m33, m34;
                    T m41, m42, m43, m44;
                } m;
                T a[4][4];
            };

            static_assert(sizeof(m) == sizeof(a), "Check alignment");

        public:
            Matrix4()noexcept
            {
                a[0][0] = T(0);  a[0][1] = T(0);  a[0][2] = T(0);  a[0][3] = T(0);
                a[1][0] = T(0);  a[1][1] = T(0);  a[1][2] = T(0);  a[1][3] = T(0);
                a[2][0] = T(0);  a[2][1] = T(0);  a[2][2] = T(0);  a[2][3] = T(0);
                a[3][0] = T(0);  a[3][1] = T(0);  a[3][2] = T(0);  a[3][3] = T(0);
            }

            Matrix4(T v[4][4])noexcept
            {
                for (int i = 0; i < 4; ++i)
                {
                    for (int j = 0; j < 4; ++j)
                        a[i][j] = v[i][j];
                }
            }

            Matrix4(T m11, T m12, T m13, T m14, T m21, T m22, T m23, T m24, T m31, T m32, T m33, T m34, T m41, T m42,
                T m43, T m44)noexcept
            {
                a[0][0] = m11;  a[0][1] = m12;  a[0][2] = m13;  a[0][3] = m14;
                a[1][0] = m21;  a[1][1] = m22;  a[1][2] = m23;  a[1][3] = m24;
                a[2][0] = m31;  a[2][1] = m32;  a[2][2] = m33;  a[2][3] = m34;
                a[3][0] = m41;  a[3][1] = m42;  a[3][2] = m43;  a[3][3] = m44;
            }

            Matrix4(const Matrix4& rhs)noexcept = default;
            Matrix4(Matrix4&& rhs)noexcept = default;

            Matrix4& operator=(const Matrix4& rhs)noexcept = default;
            Matrix4& operator=(Matrix4&& rhs)noexcept = default;

            Vector4<T>& operator[](unsigned index)
            {
                static Vector4<T> s_Invalid;
                static_assert(sizeof(a[0]) == sizeof(s_Invalid), "Check alignment");

                if (index < 4)
                    return *reinterpret_cast<Vector4<T>*>(a[index]);
                assert(false);
                return s_Invalid;
            }

            T operator[](unsigned index)const
            {
                static_assert(sizeof(a[0]) == sizeof(Vector4<T>), "Check alignment");

                if (index < 4)
                    return *reinterpret_cast<Vector4<T>*>(a[index]);
                return Vector4<T>();
            }

        public:
            void Clear()noexcept
            {
                a[0][0] = T(0);  a[0][1] = T(0);  a[0][2] = T(0);  a[0][3] = T(0);
                a[1][0] = T(0);  a[1][1] = T(0);  a[1][2] = T(0);  a[1][3] = T(0);
                a[2][0] = T(0);  a[2][1] = T(0);  a[2][2] = T(0);  a[2][3] = T(0);
                a[3][0] = T(0);  a[3][1] = T(0);  a[3][2] = T(0);  a[3][3] = T(0);
            }

            void SetIdentity()noexcept
            {
                a[0][0] = T(1);  a[0][1] = T(0);  a[0][2] = T(0);  a[0][3] = T(0);
                a[1][0] = T(0);  a[1][1] = T(1);  a[1][2] = T(0);  a[1][3] = T(0);
                a[2][0] = T(0);  a[2][1] = T(0);  a[2][2] = T(1);  a[2][3] = T(0);
                a[3][0] = T(0);  a[3][1] = T(0);  a[3][2] = T(0);  a[3][3] = T(1);
            }

            void Set(T v[4][4])noexcept
            {
                for (int i = 0; i < 4; ++i)
                {
                    for (int j = 0; j < 4; ++j)
                        a[i][j] = v[i][j];
                }
            }

            void Set(T m11, T m12, T m13, T m14, T m21, T m22, T m23, T m24, T m31, T m32, T m33, T m34, T m41, T m42,
                T m43, T m44)noexcept
            {
                a[0][0] = m11;  a[0][1] = m12;  a[0][2] = m13;  a[0][3] = m14;
                a[1][0] = m21;  a[1][1] = m22;  a[1][2] = m23;  a[1][3] = m24;
                a[2][0] = m31;  a[2][1] = m32;  a[2][2] = m33;  a[2][3] = m34;
                a[3][0] = m41;  a[3][1] = m42;  a[3][2] = m43;  a[3][3] = m44;
            }

            /**
             * @brief 转置
             */
            void Transpose()const
            {
                return Matrix4<T>(
                    a[0][0], a[1][0], a[2][0], a[3][0],
                    a[0][1], a[1][1], a[2][1], a[3][1],
                    a[0][2], a[1][2], a[2][2], a[3][2],
                    a[0][3], a[1][3], a[2][3], a[3][3]);
            };

            void SetTranspose()
            {
                Set(
                    a[0][0], a[1][0], a[2][0], a[3][0],
                    a[0][1], a[1][1], a[2][1], a[3][1],
                    a[0][2], a[1][2], a[2][2], a[3][2],
                    a[0][3], a[1][3], a[2][3], a[3][3]);
            };

            /**
             * @brief 计算行列式
             */
            T GetDeterminant()const
            {
                return
                    (a[0][0] * a[1][1] * a[2][2] * a[3][3]) - (a[0][0] * a[1][1] * a[2][3] * a[3][2]) +
                    (a[0][0] * a[1][2] * a[2][3] * a[3][1]) - (a[0][0] * a[1][2] * a[2][1] * a[3][3]) +
                    (a[0][0] * a[1][3] * a[2][1] * a[3][2]) - (a[0][0] * a[1][3] * a[2][2] * a[3][1]) -
                    (a[0][1] * a[1][2] * a[2][3] * a[3][0]) + (a[0][1] * a[1][2] * a[2][0] * a[3][3]) -
                    (a[0][1] * a[1][3] * a[2][0] * a[3][2]) + (a[0][1] * a[1][3] * a[2][2] * a[3][0]) -
                    (a[0][1] * a[1][0] * a[2][2] * a[3][3]) + (a[0][1] * a[1][0] * a[2][3] * a[3][2]) +
                    (a[0][2] * a[1][3] * a[2][0] * a[3][1]) - (a[0][2] * a[1][3] * a[2][1] * a[3][0]) +
                    (a[0][2] * a[1][0] * a[2][1] * a[3][3]) - (a[0][2] * a[1][0] * a[2][3] * a[3][1]) +
                    (a[0][2] * a[1][1] * a[2][3] * a[3][0]) - (a[0][2] * a[1][1] * a[2][0] * a[3][3]) -
                    (a[0][3] * a[1][0] * a[2][1] * a[3][2]) + (a[0][3] * a[1][0] * a[2][2] * a[3][1]) -
                    (a[0][3] * a[1][1] * a[2][2] * a[3][0]) + (a[0][3] * a[1][1] * a[2][0] * a[3][2]) -
                    (a[0][3] * a[1][2] * a[2][0] * a[3][1]) + (a[0][3] * a[1][2] * a[2][1] * a[3][0]);
            }

            /**
             * @brief 获取主对角线
             */
            Vector4<T> GetDiagonal()const
            {
                return Vector4<T>(a[0][0], a[1][1], a[2][2], a[3][3]);
            }

            /**
             * @brief 设置主对角线
             */
            void SetDiagonal(Vector4<T> v)
            {
                a[0][0] = v.x;
                a[1][1] = v.y;
                a[2][2] = v.z;
                a[3][3] = v.w;
            }

            void SetDiagonal(T x, T y, T z, T w)
            {
                a[0][0] = x;
                a[1][1] = y;
                a[2][2] = z;
                a[3][3] = w;
            }

            /**
             * @brief 获取矩阵的迹
             */
            T GetTrace()const
            {
                return a[0][0] + a[1][1] + a[2][2] + a[3][3];
            }

            /**
             * @brief 归一化
             */
            Matrix4<T> Normalize()const
            {
                auto det = GetDeterminant();
                return *this / det;
            }

            T SetNormalize()
            {
                auto det = GetDeterminant();
                *this /= det;
                return det;
            }

            /**
             * @brief 尝试计算逆矩阵
             * @param ret 输出结果
             * @return 若不存在逆矩阵则返回false。
             */
            bool TryInvert(Matrix4<T>& ret)const
            {
                ret = *this;

                int colIdx[] = { 0, 0, 0, 0 };
                int rowIdx[] = { 0, 0, 0, 0 };
                int pivotIdx[] = { -1, -1, -1, -1 };

                int icol = 0;
                int irow = 0;
                for (int i = 0; i < 4; ++i)
                {
                    // Find the largest pivot value
                    T maxPivot = T(0);
                    for (int j = 0; j < 4; ++j)
                    {
                        if (pivotIdx[j] != 0)
                        {
                            for (int k = 0; k < 4; ++k)
                            {
                                if (pivotIdx[k] == -1)
                                {
                                    auto absVal = Abs(ret.a[j][k]);
                                    if (absVal > maxPivot)
                                    {
                                        maxPivot = absVal;
                                        irow = j;
                                        icol = k;
                                    }
                                }
                                else if (pivotIdx[k] > 0)
                                    return true;
                            }
                        }
                    }

                    ++pivotIdx[icol];

                    // Swap rows over so pivot is on diagonal
                    if (irow != icol)
                    {
                        for (int k = 0; k < 4; ++k)
                        {
                            auto f = ret.a[irow][k];
                            ret.a[irow][k] = ret.a[icol][k];
                            ret.a[icol][k] = f;
                        }
                    }

                    rowIdx[i] = irow;
                    colIdx[i] = icol;

                    auto pivot = ret.a[icol][icol];

                    // check for singular matrix
                    if (pivot == T(0))
                    {
                        ret = *this;
                        return false;
                    }

                    // Scale row so it has a unit diagonal
                    auto oneOverPivot = T(1) / pivot;
                    ret.a[icol][icol] = T(1);
                    for (int k = 0; k < 4; ++k)
                        ret.a[icol][k] *= oneOverPivot;

                    // Do elimination of non-diagonal elements
                    for (int j = 0; j < 4; ++j)
                    {
                        // check this isn't on the diagonal
                        if (icol != j)
                        {
                            auto f = ret.a[j][icol];
                            ret.a[j][icol] = T(0);
                            for (int k = 0; k < 4; ++k)
                                ret.a[j][k] -= ret.a[icol][k] * f;
                        }
                    }
                }

                for (int j = 3; j >= 0; --j)
                {
                    auto ir = rowIdx[j];
                    auto ic = colIdx[j];
                    for (int k = 0; k < 4; ++k)
                    {
                        auto f = ret.a[k][ir];
                        ret.a[k][ir] = ret.a[k][ic];
                        ret.a[k][ic] = f;
                    }
                }
                return true;
            }

            /**
             * @brief 计算逆矩阵
             * @return 若不存在逆矩阵则返回原始值。
             */
            Matrix4<T> Invert()const
            {
                Matrix4<T> ret;
                TryInvert(ret);
                return ret;
            }

            /**
             * @brief 计算逆矩阵
             * @return 如果不存在逆矩阵，则返回false。否则返回true。
             */
            bool SetInvert()
            {
                Matrix4<T> t;
                auto ret = TryInvert(t);
                if (ret)
                    *this = t;
                return ret;
            }

            /**
             * @brief 返回清空平移量的矩阵
             */
            Matrix4<T> ClearTranslation()const
            {
                auto ret = *this;
                ret[3][0] = ret[3][1] = ret[3][2] = T(0);
                return ret;
            }

            void SetClearTranslation()
            {
                a[3][0] = a[3][1] = a[3][2] = T(0);
            }

            /**
             * @brief 返回清空缩放量的矩阵
             */
            Matrix4<T> ClearScale()const
            {
                auto ret = *this;
                auto row0 = Vector3<T>(ret[0][0], ret[0][1], ret[0][2]);
                auto row1 = Vector3<T>(ret[1][0], ret[1][1], ret[1][2]);
                auto row2 = Vector3<T>(ret[2][0], ret[2][1], ret[2][2]);
                row0.SetNormalize();
                row1.SetNormalize();
                row2.SetNormalize();
                ret[0][0] = row0.x;  ret[0][1] = row0.y;  ret[0][2] = row0.z;
                ret[1][0] = row1.x;  ret[1][1] = row1.y;  ret[1][2] = row1.z;
                ret[2][0] = row2.x;  ret[2][1] = row2.y;  ret[2][2] = row2.z;
                return ret;
            }

            void SetClearScale()
            {
                auto row0 = Vector3<T>(a[0][0], a[0][1], a[0][2]);
                auto row1 = Vector3<T>(a[1][0], a[1][1], a[1][2]);
                auto row2 = Vector3<T>(a[2][0], a[2][1], a[2][2]);
                row0.SetNormalize();
                row1.SetNormalize();
                row2.SetNormalize();
                a[0][0] = row0.x;  a[0][1] = row0.y;  a[0][2] = row0.z;
                a[1][0] = row1.x;  a[1][1] = row1.y;  a[1][2] = row1.z;
                a[2][0] = row2.x;  a[2][1] = row2.y;  a[2][2] = row2.z;
            }

            /**
             * @brief 返回清空旋转量的矩阵
             */
            Matrix4<T> ClearRotation()const
            {
                auto ret = *this;
                auto row0 = Vector3<T>(ret[0][0], ret[0][1], ret[0][2]);
                auto row1 = Vector3<T>(ret[1][0], ret[1][1], ret[1][2]);
                auto row2 = Vector3<T>(ret[2][0], ret[2][1], ret[2][2]);
                auto len0 = row0.Length();
                auto len1 = row1.Length();
                auto len2 = row2.Length();
                ret[0][0] = len0;  ret[0][1] = T(0);  ret[0][2] = T(0);
                ret[1][0] = T(0);  ret[1][1] = len1;  ret[1][2] = T(0);
                ret[2][0] = T(0);  ret[2][1] = T(0);  ret[2][2] = len2;
                return ret;
            }

            void SetClearRotation()
            {
                auto row0 = Vector3<T>(a[0][0], a[0][1], a[0][2]);
                auto row1 = Vector3<T>(a[1][0], a[1][1], a[1][2]);
                auto row2 = Vector3<T>(a[2][0], a[2][1], a[2][2]);
                auto len0 = row0.Length();
                auto len1 = row1.Length();
                auto len2 = row2.Length();
                a[0][0] = len0;  a[0][1] = T(0);  a[0][2] = T(0);
                a[1][0] = T(0);  a[1][1] = len1;  a[1][2] = T(0);
                a[2][0] = T(0);  a[2][1] = T(0);  a[2][2] = len2;
            }

            /**
             * @brief 返回清空投影量的矩阵
             */
            Matrix4<T> ClearProjection()const
            {
                auto ret = *this;
                ret[0][3] = 0;
                ret[1][3] = 0;
                ret[2][3] = 0;
                ret[3][3] = 0;
                return ret;
            }

            void SetClearProjection()
            {
                a[0][3] = 0;
                a[1][3] = 0;
                a[2][3] = 0;
                a[3][3] = 0;
            }

            /**
             * @brief 提取平移量
             */
            Vector3<T> ExtractTranslation()const
            {
                return Vector3<T>(a[3][0], a[3][1], a[3][2]);
            }

            /**
             * @brief 提取缩放量
             */
            Vector3<T> ExtractScale()const
            {
                auto row0 = Vector3<T>(a[0][0], a[0][1], a[0][2]);
                auto row1 = Vector3<T>(a[1][0], a[1][1], a[1][2]);
                auto row2 = Vector3<T>(a[2][0], a[2][1], a[2][2]);
                return Vector3<T>(row0.Length(), row1.Length(), row2.Length());
            }

            /**
             * @brief 提取旋转分量
             * @param rowNormalize 如果矩阵尚未规格化，则传入true。否则传入false。
             * @return 旋转分量
             */
            Quaternion<T> ExtractRotation(bool rowNormalize=true)const
            {
                auto row0 = Vector3<T>(a[0][0], a[0][1], a[0][2]);
                auto row1 = Vector3<T>(a[1][0], a[1][1], a[1][2]);
                auto row2 = Vector3<T>(a[2][0], a[2][1], a[2][2]);

                if (rowNormalize)
                {
                    row0.SetNormalize();
                    row1.SetNormalize();
                    row2.SetNormalize();
                }

                // code below adapted from Blender
                auto q = Quaternion<T>::Identity;
                auto trace = (row0.x + row1.y + row2.z + T(1)) / T(4);

                if (trace > 0)
                {
                    auto sq = Sqrt(trace);

                    q.w = sq;
                    sq = T(1) / (T(4) * sq);
                    q.x = (row1[2] - row2[1]) * sq;
                    q.y = (row2[0] - row0[2]) * sq;
                    q.z = (row0[1] - row1[0]) * sq;
                }
                else if (row0[0] > row1[1] && row0[0] > row2[2])
                {
                    auto sq = T(2) * Sqrt(T(1) + row0[0] - row1[1] - row2[2]);

                    q.x = sq / T(4);
                    sq = T(1) / sq;
                    q.w = (row2[1] - row1[2]) * sq;
                    q.y = (row1[0] + row0[1]) * sq;
                    q.z = (row2[0] + row0[2]) * sq;
                }
                else if (row1[1] > row2[2])
                {
                    auto sq = T(2) * Sqrt(T(1) + row1[1] - row0[0] - row2[2]);

                    q.y = sq / T(4);
                    sq = T(1) / sq;
                    q.w = (row2[0] - row0[2]) * sq;
                    q.x = (row1[0] + row0[1]) * sq;
                    q.z = (row2[1] + row1[2]) * sq;
                }
                else
                {
                    auto sq = T(2) * Sqrt(T(1) + row2[2] - row0[0] - row1[1]);

                    q.z = sq / T(4);
                    sq = T(1) / sq;
                    q.w = (row1[0] - row0[1]) * sq;
                    q.x = (row2[0] + row0[2]) * sq;
                    q.y = (row2[1] + row1[2]) * sq;
                }

                q.SetNormalize();
                return q;
            }

            /**
             * @brief 提取投影量
             */
            Vector4<T> ExtractProjection()
            {
                return Vector4<T>(a[0][3], a[1][3], a[2][3], a[3][3]);
            }

        public:
            static const Matrix4 Identity;
            
            /**
             * @brief 创建左右手系交换矩阵
             */
            static Matrix4<T> CreateSwaper()
            {
                return Matrix4<T>(
                    T(1), T(0), T(0), T(0),
                    T(0), T(0), T(1), T(0),
                    T(0), T(1), T(0), T(0),
                    T(0), T(0), T(0), T(1));
            };

            /**
             * @brief 创建平移矩阵
             */
            static Matrix4<T> CreateTranslateMatrix(Vector3<T> vec)
            {
                return Matrix4<T>(
                    T(1), T(0), T(0), T(0),
                    T(0), T(1), T(0), T(0),
                    T(0), T(0), T(1), T(0),
                    vec.x, vec.y, vec.z, T(1));
            };

            /**
             * @brief 创建缩放矩阵
             */
            static Matrix4<T> CreateScaleMatrix(T value)
            {
                return Matrix4<T>(
                    value, T(0), T(0), T(0),
                    T(0), value, T(0), T(0),
                    T(0), T(0), value, T(0),
                    T(0), T(0), T(0), T(1));
            };

            static Matrix4<T> CreateScaleMatrix(Vector3<T> vec)
            {
                return Matrix4<T>(
                    vec.x, T(0), T(0), T(0),
                    T(0), vec.y, T(0), T(0),
                    T(0), T(0), vec.z, T(0),
                    T(0), T(0), T(0), T(1));
            }

            /**
             * @brief 返回绕X轴旋转的矩阵
             */
            static Matrix4<T> CreateRotateX(T angle)
            {
                auto angleS = Sin(angle), angleC = Cos(angle);

                return Matrix4<T>(
                    T(1), T(0), T(0), T(0),
                    T(0), angleC, angleS, T(0),
                    T(0), -angleS, angleC, T(0),
                    T(0), T(0), T(0), T(1));
            };

            /**
             * @brief 返回绕Y轴旋转的矩阵
             */
            static Matrix4<T> CreateRotateY(T angle)
            {
                auto angleS = Sin(angle), angleC = Cos(angle);

                return Matrix4<T>(
                    angleC, T(0), -angleS, T(0),
                    T(0), T(1), T(0), T(0),
                    angleS, T(0), angleC, T(0),
                    T(0), T(0), T(0), T(1));
            };

            /**
             * @brief 返回绕Z轴旋转的矩阵
             */
            static Matrix4<T> CreateRotateZ(T angle)
            {
                auto angleS = Sin(angle), angleC = Cos(angle);

                return Matrix4<T>(
                    angleC, angleS, T(0), T(0),
                    -angleS, angleC, T(0), T(0),
                    T(0), T(0), T(1), T(0),
                    T(0), T(0), T(0), T(1));
            };

            /**
             * @brief 返回绕XYZ三轴旋转矩阵
             * @param yaw 横倾是绕 y 轴旋转的角度
             * @param pitch 纵倾是绕 x 轴旋转的角度
             * @param roll 横摆是绕 z 轴旋转的角度
             */
            static Matrix4<T> CreateRotationYawPitchRoll(T yaw, T pitch, T roll)
            {
                auto sinX = Sin(pitch);
                auto cosX = Cos(pitch);
                auto sinY = Sin(yaw);
                auto cosY = Cos(yaw);
                auto sinZ = Sin(roll);
                auto cosZ = Cos(roll);

                return Matrix4<T>(
                    cosY * cosZ, cosY * sinZ, -sinY, T(0),
                    sinX * sinY * cosZ - cosX * sinZ, sinX * sinY * sinZ + cosX * cosZ, sinX * cosY, T(0),
                    cosX * sinY * cosZ + sinX * sinZ, cosX * sinY * sinZ - sinX * cosZ, cosX * cosY, T(0),
                    T(0), T(0), T(0), T(1));
            };

            /**
             * @brief 创建绕任意轴旋转矩阵（右手系）
             * @param axisRotation 旋转向量
             * @param angle 角度
             */
            static Matrix4<T> CreateRotationAxisRH(Vector3<T> axisRotation, T angle)
            {
                T angleS = Sin(-angle), angleC = Cos(-angle);
                auto t = T(1) - angleC;

                auto txx = t * axisRotation.x * axisRotation.x;
                auto txy = t * axisRotation.x * axisRotation.y;
                auto txz = t * axisRotation.x * axisRotation.z;
                auto tyy = t * axisRotation.y * axisRotation.y;
                auto tyz = t * axisRotation.y * axisRotation.z;
                auto tzz = t * axisRotation.z * axisRotation.z;

                auto xs = axisRotation.x * angleS;
                auto ys = axisRotation.y * angleS;
                auto zs = axisRotation.z * angleS;

                return Matrix4<T>(
                    angleC + txx, txy - zs, txz + ys, T(0),
                    txy + zs, angleC + tyy, tyz - xs, T(0),
                    txz - ys, tyz + xs, angleC + tzz, T(0),
                    T(0), T(0), T(0), T(1));
            };

            /**
             * @brief 创建绕任意轴旋转矩阵（左手系）
             * @param axisRotation 旋转向量
             * @param angle 角度
             */
            static Matrix4<T> CreateRotationAxisLH(Vector3<T> axisRotation, T angle)
            {
                T angleS = Sin(angle), angleC = Cos(angle);
                auto t = T(1) - angleC;

                auto txx = t * axisRotation.x * axisRotation.x;
                auto txy = t * axisRotation.x * axisRotation.y;
                auto txz = t * axisRotation.x * axisRotation.z;
                auto tyy = t * axisRotation.y * axisRotation.y;
                auto tyz = t * axisRotation.y * axisRotation.z;
                auto tzz = t * axisRotation.z * axisRotation.z;

                auto xs = axisRotation.x * angleS;
                auto ys = axisRotation.y * angleS;
                auto zs = axisRotation.z * angleS;

                return Matrix4<T>(
                    angleC + txx, txy - zs, txz + ys, T(0),
                    txy + zs, angleC + tyy, tyz - xs, T(0),
                    txz - ys, tyz + xs, angleC + tzz, T(0),
                    T(0), T(0), T(0), T(1));
            };

            /**
             * @brief 创建观察某点的矩阵（左手系）
             * @param eye 眼睛位置
             * @param lookat 观察位置
             * @param up 上方向量
             */
            static Matrix4<T> CreateLookAtLH(Vector3<T> eye, Vector3<T> lookat, Vector3<T> up)
            {
                auto zaxis(lookat - eye);
                zaxis.Normalize();
                auto xaxis = up.Cross(zaxis);
                xaxis.Normalize();
                auto yaxis = zaxis.Cross(xaxis);

                return Matrix4<T>(
                    xaxis.x, yaxis.x, zaxis.x, T(0),
                    xaxis.y, yaxis.y, zaxis.y, T(0),
                    xaxis.z, yaxis.z, zaxis.z, T(0),
                    -(xaxis * eye), -(yaxis * eye), -(zaxis * eye), T(1));
            };

            /**
             * @brief 创建观察某点的矩阵（右手系）
             * @param eye 眼睛位置
             * @param lookat 观察位置
             * @param up 上方向量
             */
            static Matrix4<T> CreateLookAtRH(Vector3<T> eye, Vector3<T> lookat, Vector3<T> up)
            {
                auto xaxis(up);
                xaxis.Normalize();
                auto zaxis(eye - lookat);
                zaxis.Normalize();
                xaxis = xaxis.Cross(zaxis);
                auto yaxis = zaxis.Cross(xaxis);

                return Matrix4<T>(
                    xaxis.x, yaxis.x, zaxis.x, T(0),
                    xaxis.y, yaxis.y, zaxis.y, T(0),
                    xaxis.z, yaxis.z, zaxis.z, T(0),
                    -(xaxis * eye), -(yaxis * eye), -(zaxis * eye), T(1));
            };

            /**
             * @brief 创建正投影矩阵（左手系）
             * @param w 横向可视范围
             * @param h 纵向可视范围
             * @param nearPlane 最近距离
             * @param farPlane  最远距离
             */
            static Matrix4<T> CreateOrthoLH(T w, T h, T nearPlane, T farPlane)
            {
                return Matrix4<T>(
                    T(2) / w, T(0), T(0), T(0),
                    T(0), T(2) / h, T(0), T(0),
                    T(0), T(0), T(1) / (farPlane - nearPlane), T(0),
                    T(0), T(0), nearPlane / (nearPlane - farPlane), T(1));
            };

            /**
             * @brief 创建正投影矩阵（右手系）
             * @param w 横向可视范围
             * @param h 纵向可视范围
             * @param nearPlane 最近距离
             * @param farPlane 最远距离
             */
            static Matrix4<T> CreateOrthoRH(T w, T h, T nearPlane, T farPlane)
            {
                return Matrix4<T>(
                    T(2) / w, T(0), T(0), T(0),
                    T(0), T(2) / h, T(0), T(0),
                    T(0), T(0), T(1) / (nearPlane - farPlane), T(0),
                    T(0), T(0), nearPlane / (nearPlane - farPlane), T(1));
            };

            /**
             * @brief 创建透视投影矩阵（左手系）
             * @param ration 屏幕纵横比（宽：高）
             * @param fovY 纵向视野范围（弧度）
             * @param nearPlane 最近距离
             * @param farPlane 最远距离
             */
            static Matrix4<T> CreatePespctiveLH(T ration, T fovY, T nearPlane, T farPlane)
            {
                auto t = T(1) / Tan(fovY / T(2));

                return Matrix4<T>(
                    t / ration, T(0), T(0), T(0),
                    T(0), t, T(0), T(0),
                    T(0), T(0), farPlane / (farPlane - nearPlane), T(1),
                    T(0), T(0), -(nearPlane * farPlane) / (farPlane - nearPlane), T(0));
            };

            /**
             * @brief 创建透视投影矩阵（右手系）
             * @param ration 屏幕纵横比（宽：高）
             * @param fovY 纵向视野范围（弧度）
             * @param nearPlane 最近距离
             * @param farPlane 最远距离
             */
            static Matrix4<T> CreatePespctiveRH(T ration, T fovY, T nearPlane, T farPlane)
            {
                auto t = T(1) / Tan(fovY / T(2));

                return Matrix4<T>(
                    t / ration, T(0), T(0), T(0),
                    T(0), t, T(0), T(0),
                    T(0), T(0), farPlane / (nearPlane - farPlane), -T(1),
                    T(0), T(0), (nearPlane * farPlane) / (nearPlane - farPlane), T(0));
            };

            /**
             * @brief 创建自定义正交投影矩阵（左手系）
             * @param l  最左侧X值
             * @param r  最右侧X值
             * @param b  最下方Y值
             * @param t  最上方Y值
             * @param zn 最近距离
             * @param zf 最远距离
             */
            static Matrix4<T> CreateOrthoOffCenterLH(T l, T r, T b, T t, T zn, T zf)
            {
                return Matrix4<T>(
                    T(2) / (r - l), T(0), T(0), T(0),
                    T(0), T(2) / (t - b), T(0), T(0),
                    T(0), T(0), T(1) / (zf - zn), T(0),
                    (l + r) / (l - r), (t + b) / (b - t), zn / (zn - zf), T(1));
            };
            
            /**
             * @brief 创建自定义正交投影矩阵（右手系）
             * @param l 最左侧X值
             * @param r 最右侧X值
             * @param b 最下方Y值
             * @param t 最上方Y值
             * @param zn 最近距离
             * @param zf 最远距离
             */
            static Matrix4<T> CreateOrthoOffCenterRH(T l, T r, T b, T t, T zn, T zf)
            {
                return Matrix4<T>(
                    T(2) / (r - l), T(0), T(0), T(0),
                    T(0), T(2) / (t - b), T(0), T(0),
                    T(0), T(0), T(1) / (zn - zf), T(0),
                    (l + r) / (l - r), (t + b) / (b - t), zn / (zn - zf) , T(1));
            };

            /**
             * @brief 创建SRT矩阵
             * @param pos 位移坐标
             * @param rot 旋转
             * @param scale 缩放
             * @return 变换结果
             *
             * 变换顺序为 scale -> rotation -> translate
             */
            static Matrix4<T> CreateSRT(Vector3<T> pos, Quaternion<T> rot, Vector3<T> scale)
            {
                if (Abs(rot.w) > T(1))
                    rot = rot.Normalize();

                auto x2 = rot.x + rot.x;
                auto y2 = rot.y + rot.y;
                auto z2 = rot.z + rot.z;
                auto w2 = rot.w + rot.w;

                Matrix4<T> ret(
                    T(1) - y2 * rot.y - z2 * rot.z, x2 * rot.y + w2 * rot.z, x2 * rot.z - w2 * rot.y, T(0),
                    x2 * rot.y - w2 * rot.z, T(1) - x2 * rot.x - z2 * rot.z, y2 * rot.z + w2 * rot.x, T(0),
                    x2 * rot.z + w2 * rot.y, y2 * rot.z - w2 * rot.x, T(1) - x2 * rot.x - y2 * rot.y, T(0),
                    pos.x, pos.y, pos.z, T(1));

                ret.a[0][0] *= scale.x;
                ret.a[0][1] *= scale.x;
                ret.a[0][2] *= scale.x;

                ret.a[1][0] *= scale.y;
                ret.a[1][1] *= scale.y;
                ret.a[1][2] *= scale.y;

                ret.a[2][0] *= scale.z;
                ret.a[2][1] *= scale.z;
                ret.a[2][2] *= scale.z;
                return ret;
            }
        };

        template <typename T>
        const Matrix4<T> Matrix4<T>::Identity = Matrix4(
            T(1), T(0), T(0), T(0),
            T(0), T(1), T(0), T(0),
            T(0), T(0), T(1), T(0),
            T(0), T(0), T(0), T(1));

        template <typename T>
        constexpr Matrix4<T> operator+(const Matrix4<T>& v)
        {
            return Matrix4<T>(
                +v.a[0][0], +v.a[0][1], +v.a[0][2], +v.a[0][3],
                +v.a[1][0], +v.a[1][1], +v.a[1][2], +v.a[1][3],
                +v.a[2][0], +v.a[2][1], +v.a[2][2], +v.a[2][3],
                +v.a[3][0], +v.a[3][1], +v.a[3][2], +v.a[3][3]);
        }

        template <typename T>
        constexpr Matrix4<T> operator-(const Matrix4<T>& v)
        {
            return Matrix4<T>(
                -v.a[0][0], -v.a[0][1], -v.a[0][2], -v.a[0][3],
                -v.a[1][0], -v.a[1][1], -v.a[1][2], -v.a[1][3],
                -v.a[2][0], -v.a[2][1], -v.a[2][2], -v.a[2][3],
                -v.a[3][0], -v.a[3][1], -v.a[3][2], -v.a[3][3]);
        }

        template <typename T>
        constexpr Matrix4<T> operator+(const Matrix4<T>& l, const Matrix4<T>& r)
        {
            return Matrix4<T>(
                l.a[0][0] + r.a[0][0], l.a[0][1] + r.a[0][1], l.a[0][2] + r.a[0][2], l.a[0][3] + r.a[0][3],
                l.a[1][0] + r.a[1][0], l.a[1][1] + r.a[1][1], l.a[1][2] + r.a[1][2], l.a[1][3] + r.a[1][3],
                l.a[2][0] + r.a[2][0], l.a[2][1] + r.a[2][1], l.a[2][2] + r.a[2][2], l.a[2][3] + r.a[2][3],
                l.a[3][0] + r.a[3][0], l.a[3][1] + r.a[3][1], l.a[3][2] + r.a[3][2], l.a[3][3] + r.a[3][3]);
        }

        template <typename T>
        constexpr Matrix4<T> operator-(const Matrix4<T>& l, const Matrix4<T>& r)
        {
            return Matrix4<T>(
                l.a[0][0] - r.a[0][0], l.a[0][1] - r.a[0][1], l.a[0][2] - r.a[0][2], l.a[0][3] - r.a[0][3],
                l.a[1][0] - r.a[1][0], l.a[1][1] - r.a[1][1], l.a[1][2] - r.a[1][2], l.a[1][3] - r.a[1][3],
                l.a[2][0] - r.a[2][0], l.a[2][1] - r.a[2][1], l.a[2][2] - r.a[2][2], l.a[2][3] - r.a[2][3],
                l.a[3][0] - r.a[3][0], l.a[3][1] - r.a[3][1], l.a[3][2] - r.a[3][2], l.a[3][3] - r.a[3][3]);
        }

        template <typename T>
        constexpr Matrix4<T> operator*(const Matrix4<T>& l, T r)
        {
            return Matrix4<T>(
                l.a[0][0] * r, l.a[0][1] * r, l.a[0][2] * r, l.a[0][3] * r,
                l.a[1][0] * r, l.a[1][1] * r, l.a[1][2] * r, l.a[1][3] * r,
                l.a[2][0] * r, l.a[2][1] * r, l.a[2][2] * r, l.a[2][3] * r,
                l.a[3][0] * r, l.a[3][1] * r, l.a[3][2] * r, l.a[3][3] * r);
        }

        template <typename T>
        constexpr Matrix4<T> operator*(T l, const Matrix4<T>& r)
        {
            return Matrix4<T>(
                l * r.a[0][0], l * r.a[0][1], l * r.a[0][2], l * r.a[0][3],
                l * r.a[1][0], l * r.a[1][1], l * r.a[1][2], l * r.a[1][3],
                l * r.a[2][0], l * r.a[2][1], l * r.a[2][2], l * r.a[2][3],
                l * r.a[3][0], l * r.a[3][1], l * r.a[3][2], l * r.a[3][3]);
        }

        template <typename T>
        constexpr Matrix4<T> operator*(const Matrix4<T>& l, const Matrix4<T>& r)
        {
            return Matrix4<T>(
                l.a[0][0] * r.a[0][0] + l.a[0][1] * r.a[1][0] + l.a[0][2] * r.a[2][0] + l.a[0][3] * r.a[3][0],
                l.a[0][0] * r.a[0][1] + l.a[0][1] * r.a[1][1] + l.a[0][2] * r.a[2][1] + l.a[0][3] * r.a[3][1],
                l.a[0][0] * r.a[0][2] + l.a[0][1] * r.a[1][2] + l.a[0][2] * r.a[2][2] + l.a[0][3] * r.a[3][2],
                l.a[0][0] * r.a[0][3] + l.a[0][1] * r.a[1][3] + l.a[0][2] * r.a[2][3] + l.a[0][3] * r.a[3][3],

                l.a[1][0] * r.a[0][0] + l.a[1][1] * r.a[1][0] + l.a[1][2] * r.a[2][0] + l.a[1][3] * r.a[3][0],
                l.a[1][0] * r.a[0][1] + l.a[1][1] * r.a[1][1] + l.a[1][2] * r.a[2][1] + l.a[1][3] * r.a[3][1],
                l.a[1][0] * r.a[0][2] + l.a[1][1] * r.a[1][2] + l.a[1][2] * r.a[2][2] + l.a[1][3] * r.a[3][2],
                l.a[1][0] * r.a[0][3] + l.a[1][1] * r.a[1][3] + l.a[1][2] * r.a[2][3] + l.a[1][3] * r.a[3][3],

                l.a[2][0] * r.a[0][0] + l.a[2][1] * r.a[1][0] + l.a[2][2] * r.a[2][0] + l.a[2][3] * r.a[3][0],
                l.a[2][0] * r.a[0][1] + l.a[2][1] * r.a[1][1] + l.a[2][2] * r.a[2][1] + l.a[2][3] * r.a[3][1],
                l.a[2][0] * r.a[0][2] + l.a[2][1] * r.a[1][2] + l.a[2][2] * r.a[2][2] + l.a[2][3] * r.a[3][2],
                l.a[2][0] * r.a[0][3] + l.a[2][1] * r.a[1][3] + l.a[2][2] * r.a[2][3] + l.a[2][3] * r.a[3][3],

                l.a[3][0] * r.a[0][0] + l.a[3][1] * r.a[1][0] + l.a[3][2] * r.a[2][0] + l.a[3][3] * r.a[3][0],
                l.a[3][0] * r.a[0][1] + l.a[3][1] * r.a[1][1] + l.a[3][2] * r.a[2][1] + l.a[3][3] * r.a[3][1],
                l.a[3][0] * r.a[0][2] + l.a[3][1] * r.a[1][2] + l.a[3][2] * r.a[2][2] + l.a[3][3] * r.a[3][2],
                l.a[3][0] * r.a[0][3] + l.a[3][1] * r.a[1][3] + l.a[3][2] * r.a[2][3] + l.a[3][3] * r.a[3][3]
            );
        }

        template <typename T>
        constexpr Matrix4<T> operator/(const Matrix4<T>& l, T r)
        {
            return Matrix4<T>(
                l.a[0][0] / r, l.a[0][1] / r, l.a[0][2] / r, l.a[0][3] / r,
                l.a[1][0] / r, l.a[1][1] / r, l.a[1][2] / r, l.a[1][3] / r,
                l.a[2][0] / r, l.a[2][1] / r, l.a[2][2] / r, l.a[2][3] / r,
                l.a[3][0] / r, l.a[3][1] / r, l.a[3][2] / r, l.a[3][3] / r);
        }

        template <typename T>
        constexpr Matrix4<T> operator/(T l, const Matrix4<T>& r)
        {
            return Matrix4<T>(
                l / r.a[0][0], l / r.a[0][1], l / r.a[0][2], l / r.a[0][3],
                l / r.a[1][0], l / r.a[1][1], l / r.a[1][2], l / r.a[1][3],
                l / r.a[2][0], l / r.a[2][1], l / r.a[2][2], l / r.a[2][3],
                l / r.a[3][0], l / r.a[3][1], l / r.a[3][2], l / r.a[3][3]);
        }

        template <typename T>
        constexpr Matrix4<T> operator/(const Matrix4<T>& l, const Matrix4<T>& r)
        {
            return l * r.Inverse();
        }

        template <typename T>
        constexpr bool operator==(const Matrix4<T>& l, const Matrix4<T>& r)
        {
            return l.a[0][0] == r.a[0][0] && l.a[0][1] == r.a[0][1] && l.a[0][2] == r.a[0][2] && l.a[0][3] == r.a[0][3]
                && l.a[1][0] == r.a[1][0] && l.a[1][1] == r.a[1][1] && l.a[1][2] == r.a[1][2] && l.a[1][3] == r.a[1][3]
                && l.a[2][0] == r.a[2][0] && l.a[2][1] == r.a[2][1] && l.a[2][2] == r.a[2][2] && l.a[2][3] == r.a[2][3]
                && l.a[3][0] == r.a[3][0] && l.a[3][1] == r.a[3][1] && l.a[3][2] == r.a[3][2] && l.a[3][3] == r.a[3][3];
        }

        template <typename T>
        constexpr bool operator!=(const Matrix4<T>& l, const Matrix4<T>& r)
        {
            return l.a[0][0] != r.a[0][0] || l.a[0][1] != r.a[0][1] || l.a[0][2] != r.a[0][2] || l.a[0][3] != r.a[0][3]
                || l.a[1][0] != r.a[1][0] || l.a[1][1] != r.a[1][1] || l.a[1][2] != r.a[1][2] || l.a[1][3] != r.a[1][3]
                || l.a[2][0] != r.a[2][0] || l.a[2][1] != r.a[2][1] || l.a[2][2] != r.a[2][2] || l.a[2][3] != r.a[2][3]
                || l.a[3][0] != r.a[3][0] || l.a[3][1] != r.a[3][1] || l.a[3][2] != r.a[3][2] || l.a[3][3] != r.a[3][3];
        }

        template <typename T>
        Matrix4<T>& operator+=(Matrix4<T>& l, const Matrix4<T>& r)
        {
            l.a[0][0] += r.a[0][0];  l.a[0][1] += r.a[0][1];  l.a[0][2] += r.a[0][2];  l.a[0][3] += r.a[0][3];
            l.a[1][0] += r.a[1][0];  l.a[1][1] += r.a[1][1];  l.a[1][2] += r.a[1][2];  l.a[1][3] += r.a[1][3];
            l.a[2][0] += r.a[2][0];  l.a[2][1] += r.a[2][1];  l.a[2][2] += r.a[2][2];  l.a[2][3] += r.a[2][3];
            l.a[3][0] += r.a[3][0];  l.a[3][1] += r.a[3][1];  l.a[3][2] += r.a[3][2];  l.a[3][3] += r.a[3][3];
            return l;
        }

        template <typename T>
        Matrix4<T>& operator-=(Matrix4<T>& l, const Matrix4<T>& r)
        {
            l.a[0][0] -= r.a[0][0];  l.a[0][1] -= r.a[0][1];  l.a[0][2] -= r.a[0][2];  l.a[0][3] -= r.a[0][3];
            l.a[1][0] -= r.a[1][0];  l.a[1][1] -= r.a[1][1];  l.a[1][2] -= r.a[1][2];  l.a[1][3] -= r.a[1][3];
            l.a[2][0] -= r.a[2][0];  l.a[2][1] -= r.a[2][1];  l.a[2][2] -= r.a[2][2];  l.a[2][3] -= r.a[2][3];
            l.a[3][0] -= r.a[3][0];  l.a[3][1] -= r.a[3][1];  l.a[3][2] -= r.a[3][2];  l.a[3][3] -= r.a[3][3];
            return l;
        }

        template <typename T>
        Matrix4<T>& operator*=(Matrix4<T>& l, T r)
        {
            l.a[0][0] *= r;  l.a[0][1] *= r;  l.a[0][2] *= r;  l.a[0][3] *= r;
            l.a[1][0] *= r;  l.a[1][1] *= r;  l.a[1][2] *= r;  l.a[1][3] *= r;
            l.a[2][0] *= r;  l.a[2][1] *= r;  l.a[2][2] *= r;  l.a[2][3] *= r;
            l.a[3][0] *= r;  l.a[3][1] *= r;  l.a[3][2] *= r;  l.a[3][3] *= r;
            return l;
        }

        template <typename T>
        Matrix4<T>& operator*=(Matrix4<T>& l, const Matrix4<T>& r)
        {
            l = l * r;
            return l;
        }

        template <typename T>
        Matrix4<T>& operator/=(Matrix4<T>& l, T r)
        {
            l.a[0][0] /= r;  l.a[0][1] /= r;  l.a[0][2] /= r;  l.a[0][3] /= r;
            l.a[1][0] /= r;  l.a[1][1] /= r;  l.a[1][2] /= r;  l.a[1][3] /= r;
            l.a[2][0] /= r;  l.a[2][1] /= r;  l.a[2][2] /= r;  l.a[2][3] /= r;
            l.a[3][0] /= r;  l.a[3][1] /= r;  l.a[3][2] /= r;  l.a[3][3] /= r;
            return l;
        }

        template <typename T>
        Matrix4<T>& operator/=(Matrix4<T>& l, const Matrix4<T>& r)
        {
            l = l / r;
            return l;
        }

        //////////////////////////////////////// </editor-fold>

        using Vec2 = Vector2<float>;
        using Vec3 = Vector3<float>;
        using Vec4 = Vector4<float>;
        using Quat = Quaternion<float>;
        using Mat4 = Matrix4<float>;
    }
}
