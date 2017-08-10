/**
 * @file
 * @date 2017/6/14
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
        constexpr Vector3<T> Min(Vector3<T> v1, Vector3<T> v2, Vector3<T> v3)
        {
            return Vector3<T>(
                Min(v1.x, v2.x, v3.x),
                Min(v1.y, v2.y, v3.y),
                Min(v1.z, v2.z, v3.z));
        }

        template <typename T>
        constexpr Vector3<T> Max(Vector3<T> v1, Vector3<T> v2, Vector3<T> v3)
        {
            return Vector3<T>(
                Max(v1.x, v2.x, v3.x),
                Max(v1.y, v2.y, v3.y),
                Max(v1.z, v2.z, v3.z));
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
        //////////////////////////////////////// <editor-fold desc="四元数">

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

        using Vec2 = Vector2<float>;
        using Vec3 = Vector3<float>;
        using Quat = Quaternion<float>;
    }
}
