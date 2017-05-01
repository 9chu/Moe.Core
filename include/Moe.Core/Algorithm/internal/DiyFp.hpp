/**
 * @file
 * @date 2017/4/30
 * @see https://github.com/google/double-conversion
 */
#pragma once
#include "../../Utility/Misc.hpp"

namespace moe
{
    namespace internal
    {
        /**
         * @brief Do It Yourself Floating Point
         *
         * DiyFp实现了基于uint64_t作为有效数字和基于int作为指数部分的浮点数，一个规格化的DiyFp浮点将会拥有最多的有效数字。
         * 注意：乘法和减法不会规格化结果，并且DiyFp不会用于存储特殊值（NaN和无穷）。
         */
        class DiyFp
        {
        public:
            static const int kSignificandSize = 64;  // 有效数字位数

            /**
             * @brief 减法
             * @pre 指数部分必须相同且a的有效数字必须比b大
             * @note 结果不会被规格化
             * @param a 被减数
             * @param b 减数
             * @return 计算结果
             */
            static DiyFp Minus(const DiyFp& a, const DiyFp& b)
            {
                DiyFp result = a;
                result.Subtract(b);
                return result;
            }

            /**
             * @brief 乘法
             * @note 结果不会被规格化
             * @param a 被乘数
             * @param b 乘数
             * @return 计算结果
             */
            static DiyFp Times(const DiyFp& a, const DiyFp& b)
            {
                DiyFp result = a;
                result.Multiply(b);
                return result;
            }

            /**
             * @brief 规格化
             * @param a 待规格数
             * @return 规格化后的结果
             */
            static DiyFp Normalize(const DiyFp& a)
            {
                DiyFp result = a;
                result.Normalize();
                return result;
            }

        private:
            static const uint64_t k10MSBits = 0xFFC0000000000000ull;
            static const uint64_t kUInt64MSB = 0x8000000000000000ull;

        public:
            DiyFp()
                : m_ulSignificand(0), m_iExponent(0) {}
            DiyFp(uint64_t significand, int exponent)
                : m_ulSignificand(significand), m_iExponent(exponent) {}

        public:
            uint64_t Significand()const { return m_ulSignificand; }
            int Exponent()const { return m_iExponent; }

            void SetSignificand(uint64_t f) { m_ulSignificand = f; }
            void SetExponent(int e) { m_iExponent = e; }

            /**
             * @brief 减去
             * @pre 指数部分必须相同且被减数(this)必须比减数大
             * @note 结果不会被规格化
             * @param other 减数
             */
            void Subtract(const DiyFp& other)
            {
                assert(m_iExponent == other.m_iExponent);
                assert(m_ulSignificand >= other.m_ulSignificand);
                m_ulSignificand -= other.m_ulSignificand;
            }

            /**
             * @brief 乘上
             * @note 结果不会被规格化
             * @param other 乘数
             */
            void Multiply(const DiyFp& other);

            /**
             * @brief 规格化结果
             */
            void Normalize()
            {
                assert(m_ulSignificand != 0);
                uint64_t significand = m_ulSignificand;
                int exponent = m_iExponent;

                // 对有效数字进行规格化。通常规格化平均需要左移10个位，因此做一些优化。
                while ((significand & k10MSBits) == 0)
                {
                    significand <<= 10;
                    exponent -= 10;
                }

                while ((significand & kUInt64MSB) == 0)
                {
                    significand <<= 1;
                    exponent--;
                }

                m_ulSignificand = significand;
                m_iExponent = exponent;
            }

        private:
            uint64_t m_ulSignificand;
            int m_iExponent;
        };

        /**
         * @brief IEEE双精度浮点
         */
        class Double :
            public NonCopyable
        {
        public:
            static const uint64_t kSignMask = 0x8000000000000000ull;  // 符号掩码
            static const uint64_t kExponentMask = 0x7FF0000000000000ull;  // 指数掩码
            static const uint64_t kSignificandMask = 0x000FFFFFFFFFFFFFull;  // 尾数掩码
            static const uint64_t kHiddenBit = 0x0010000000000000ull;  // 隐含位
            static const int kPhysicalSignificandSize = 52;  // 52位尾数
            static const int kSignificandSize = 53;  // 53位有效数字

            static double Infinity() { return Double(kInfinity).ToDouble(); }
            static double Nan() { return Double(kNan).ToDouble(); }

            // Returns the significand size for a given order of magnitude.
            // If v = f*2^e with 2^p-1 <= f <= 2^p then p+e is v's order of magnitude.
            // This function returns the number of significant binary digits v will have
            // once it's encoded into a double. In almost all cases this is equal to
            // kSignificandSize. The only exceptions are denormals. They start with
            // leading zeroes and their effective significand-size is hence smaller.
            static int SignificandSizeForOrderOfMagnitude(int order)
            {
                if (order >= (kDenormalExponent + kSignificandSize))
                    return kSignificandSize;
                if (order <= kDenormalExponent)
                    return 0;
                return order - kDenormalExponent;
            }

        private:
            static const int kExponentBias = 0x3FF + kPhysicalSignificandSize;
            static const int kDenormalExponent = -kExponentBias + 1;
            static const int kMaxExponent = 0x7FF - kExponentBias;
            static const uint64_t kInfinity = 0x7FF0000000000000ull;
            static const uint64_t kNan = 0x7FF8000000000000ull;

            static uint64_t DiyFpToUInt64(DiyFp diyFp)
            {
                uint64_t significand = diyFp.Significand();
                int exponent = diyFp.Exponent();

                while (significand > kHiddenBit + kSignificandMask)
                {
                    significand >>= 1;
                    exponent++;
                }

                if (exponent >= kMaxExponent)
                    return kInfinity;
                if (exponent < kDenormalExponent)
                    return 0;

                while (exponent > kDenormalExponent && (significand & kHiddenBit) == 0)
                {
                    significand <<= 1;
                    exponent--;
                }

                uint64_t biasedExponent;
                if (exponent == kDenormalExponent && (significand & kHiddenBit) == 0)
                    biasedExponent = 0;
                else
                    biasedExponent = static_cast<uint64_t>(exponent + kExponentBias);

                return (significand & kSignificandMask) | (biasedExponent << kPhysicalSignificandSize);
            }

        public:
            Double()
                : m_ulValue(0) {}
            explicit Double(double value)
                : m_ulValue(BitCast<uint64_t>(value)) {}
            explicit Double(uint64_t value)
                : m_ulValue(value) {}
            explicit Double(DiyFp diyFp)
                : m_ulValue(DiyFpToUInt64(diyFp)) {}

        public:
            uint64_t ToUInt64()const { return m_ulValue; }
            double ToDouble()const { return BitCast<double>(m_ulValue); }

            /**
             * @brief 转换到DiyFp
             * @pre 浮点数必须大于或等于+0.0，并且不能是特殊的（无穷或者NaN）
             * @return DiyFp对象
             */
            DiyFp ToDiyFp()const
            {
                assert(Sign() > 0);
                assert(!IsSpecial());
                return DiyFp(Significand(), Exponent());
            }

            /**
             * @brief 转换到规范化的DiyFp
             * @pre 浮点数必须大于+0.0
             * @return 规范化的DiyFp
             */
            DiyFp ToNormalizedDiyFp()const
            {
                assert(ToDouble() > 0.0);
                uint64_t f = Significand();
                int e = Exponent();

                while ((f & kHiddenBit) == 0)  // 当前的浮点数可能不规范
                {
                    f <<= 1;
                    e--;
                }

                f <<= DiyFp::kSignificandSize - kSignificandSize;
                e -= DiyFp::kSignificandSize - kSignificandSize;
                return DiyFp(f, e);
            }

            /**
             * @brief 返回下一个更大的浮点数
             * @return 结果
             *
             * 针对无穷大，返回无穷大。
             */
            double NextDouble()const
            {
                if (m_ulValue == kInfinity)
                    return Double(kInfinity).ToDouble();
                if (Sign() < 0 && Significand() == 0)  // -0.0
                    return 0.0;
                if (Sign() < 0)
                    return Double(m_ulValue - 1).ToDouble();
                return Double(m_ulValue + 1).ToDouble();
            }

            double PreviousDouble()const
            {
                if (m_ulValue == (kInfinity | kSignMask))
                    return -Infinity();
                if (Sign() < 0)
                    return Double(m_ulValue + 1).ToDouble();
                if (Significand() == 0)
                    return -0.0;
                return Double(m_ulValue - 1).ToDouble();
            }

            int Exponent()const
            {
                if (IsDenormal())
                    return kDenormalExponent;
                uint64_t d64 = ToUInt64();
                auto biasedE = static_cast<int>((d64 & kExponentMask) >> kPhysicalSignificandSize);  // 偏置指数
                return biasedE - kExponentBias;
            }

            uint64_t Significand()const
            {
                uint64_t d64 = ToUInt64();
                uint64_t significand = d64 & kSignificandMask;
                if (!IsDenormal())
                    return significand + kHiddenBit;
                return significand;
            }

            /**
             * @brief 检查是否是不规范的浮点数
             * @return 是否不规范
             */
            bool IsDenormal()const
            {
                uint64_t d64 = ToUInt64();
                return (d64 & kExponentMask) == 0;
            }

            /**
             * @brief 检查是否是特殊值
             * @return 是否特殊值
             *
             * 我们假定非规范浮点不是特殊值。因此只有无穷和NaN是特殊值。
             */
            bool IsSpecial()const
            {
                uint64_t d64 = ToUInt64();
                return (d64 & kExponentMask) == kExponentMask;
            }

            /**
             * @brief 检查是否是NaN
             * @return 是否NaN
             */
            bool IsNan()const
            {
                uint64_t d64 = ToUInt64();
                return ((d64 & kExponentMask) == kExponentMask) && ((d64 & kSignificandMask) != 0);
            }

            /**
             * @brief 检查是否是无穷
             * @return 是否无穷
             */
            bool IsInfinite()const
            {
                uint64_t d64 = ToUInt64();
                return ((d64 & kExponentMask) == kExponentMask) && ((d64 & kSignificandMask) == 0);
            }

            /**
             * @brief 获取符号位
             * @return 符号位
             */
            int Sign()const
            {
                uint64_t d64 = ToUInt64();
                return (d64 & kSignMask) == 0 ? 1 : -1;
            }

            /**
             * @brief 返回上界
             * @pre 浮点数必须大于等于+0.0
             * @return DiyFp对象
             */
            DiyFp UpperBoundary()const
            {
                assert(Sign() > 0);
                return DiyFp(Significand() * 2 + 1, Exponent() - 1);
            }

            /**
             * @brief 计算当前值的两个边界
             * @pre 浮点数必须大于0
             * @param[out] outMinus 下界
             * @param[out] outPlus 上界（规范化的）
             */
            void NormalizedBoundaries(DiyFp& outMinus, DiyFp& outPlus)const
            {
                assert(ToDouble() > 0.0);
                DiyFp v = ToDiyFp();
                DiyFp plus = DiyFp::Normalize(DiyFp((v.Significand() << 1) + 1, v.Exponent() - 1));
                DiyFp minus;

                if (LowerBoundaryIsCloser())
                    minus = DiyFp((v.Significand() << 2) - 1, v.Exponent() - 2);
                else
                    minus = DiyFp((v.Significand() << 1) - 1, v.Exponent() - 1);
                minus.SetSignificand(minus.Significand() << (minus.Exponent() - plus.Exponent()));
                minus.SetExponent(plus.Exponent());

                outMinus = minus;
                outPlus = plus;
            }

            /**
             * @brief 检查是否是下界更近
             * @return 检查结果
             */
            bool LowerBoundaryIsCloser()const
            {
                // The boundary is closer if the significand is of the form f == 2^p-1 then
                // the lower boundary is closer.
                // Think of v = 1000e10 and v- = 9999e9.
                // Then the boundary (== (v - v-)/2) is not just at a distance of 1e9 but
                // at a distance of 1e8.
                // The only exception is for the smallest normal: the largest denormal is
                // at the same distance as its successor.
                // Note: denormals have the same exponent as the smallest normals.
                bool physicalSignificandIsZero = ((ToUInt64() & kSignificandMask) == 0);
                return physicalSignificandIsZero && (Exponent() != kDenormalExponent);
            }

        private:
            const uint64_t m_ulValue;
        };

        /**
         * @brief IEEE单精度浮点数
         */
        class Single :
            public NonCopyable
        {
        public:
            static const uint32_t kSignMask = 0x80000000;  // 符号位掩码
            static const uint32_t kExponentMask = 0x7F800000;  // 指数掩码
            static const uint32_t kSignificandMask = 0x007FFFFF;  // 尾数掩码
            static const uint32_t kHiddenBit = 0x00800000;
            static const int kPhysicalSignificandSize = 23;  // 尾数尾数，不包括隐藏位
            static const int kSignificandSize = 24;  // 有效数字位数

            static float Infinity() { return Single(kInfinity).ToFloat(); }
            static float Nan() { return Single(kNan).ToFloat(); }

        private:
            static const int kExponentBias = 0x7F + kPhysicalSignificandSize;
            static const int kDenormalExponent = -kExponentBias + 1;
            static const int kMaxExponent = 0xFF - kExponentBias;
            static const uint32_t kInfinity = 0x7F800000;
            static const uint32_t kNan = 0x7FC00000;

        public:
            Single()
                : m_uValue(0) {}
            explicit Single(float value)
                : m_uValue(BitCast<uint32_t>(value)) {}
            explicit Single(uint32_t value)
                : m_uValue(value) {}

        public:
            uint32_t ToUInt32()const { return m_uValue; }
            float ToFloat()const { return BitCast<float>(m_uValue); }

            /**
             * @brief 转换到DiyFp
             * @pre 浮点数必须大于或等于+0.0，并且不能是特殊的（无穷或者NaN）
             * @return DiyFp对象
             */
            DiyFp ToDiyFp()const
            {
                assert(Sign() > 0);
                assert(!IsSpecial());
                return DiyFp(Significand(), Exponent());
            }

            int Exponent()const
            {
                if (IsDenormal())
                    return kDenormalExponent;

                uint32_t d32 = ToUInt32();
                int biased_e = static_cast<int>((d32 & kExponentMask) >> kPhysicalSignificandSize);
                return biased_e - kExponentBias;
            }

            uint32_t Significand()const
            {
                uint32_t d32 = ToUInt32();
                uint32_t significand = d32 & kSignificandMask;
                if (!IsDenormal())
                    return significand + kHiddenBit;
                return significand;
            }

            /**
             * @brief 检查是否是非规格化数
             * @return 是否非规格化
             */
            bool IsDenormal()const
            {
                uint32_t d32 = ToUInt32();
                return (d32 & kExponentMask) == 0;
            }

            /**
             * @brief 检查是否是特殊值
             * @return 是否特殊值
             *
             * 我们假定非规范浮点不是特殊值。因此只有无穷和NaN是特殊值。
             */
            bool IsSpecial()const
            {
                uint32_t d32 = ToUInt32();
                return (d32 & kExponentMask) == kExponentMask;
            }

            bool IsNan()const
            {
                uint32_t d32 = ToUInt32();
                return ((d32 & kExponentMask) == kExponentMask) && ((d32 & kSignificandMask) != 0);
            }

            bool IsInfinite()const
            {
                uint32_t d32 = ToUInt32();
                return ((d32 & kExponentMask) == kExponentMask) && ((d32 & kSignificandMask) == 0);
            }

            int Sign()const
            {
                uint32_t d32 = ToUInt32();
                return (d32 & kSignMask) == 0 ? 1 : -1;
            }

            /**
             * @brief 返回上界
             * @pre 浮点数必须大于等于+0.0
             * @return DiyFp对象
             */
            DiyFp UpperBoundary()const
            {
                assert(Sign() > 0);
                return DiyFp(Significand() * 2 + 1, Exponent() - 1);
            }

            /**
             * @brief 计算当前值的两个边界
             * @pre 浮点数必须大于0
             * @param[out] outMinus 下界
             * @param[out] outPlus 上界（规范化的）
             */
            void NormalizedBoundaries(DiyFp& outMinus, DiyFp& outPlus)const
            {
                assert(ToFloat() > 0.0);
                DiyFp v = ToDiyFp();
                DiyFp plus = DiyFp::Normalize(DiyFp((v.Significand() << 1) + 1, v.Exponent() - 1));
                DiyFp minus;

                if (LowerBoundaryIsCloser())
                    minus = DiyFp((v.Significand() << 2) - 1, v.Exponent() - 2);
                else
                    minus = DiyFp((v.Significand() << 1) - 1, v.Exponent() - 1);
                minus.SetSignificand(minus.Significand() << (minus.Exponent() - plus.Exponent()));
                minus.SetExponent(plus.Exponent());

                outPlus = plus;
                outMinus = minus;
            }

            /**
             * @brief 检查是否是下界更近
             * @return 检查结果
             */
            bool LowerBoundaryIsCloser()const
            {
                // The boundary is closer if the significand is of the form f == 2^p-1 then
                // the lower boundary is closer.
                // Think of v = 1000e10 and v- = 9999e9.
                // Then the boundary (== (v - v-)/2) is not just at a distance of 1e9 but
                // at a distance of 1e8.
                // The only exception is for the smallest normal: the largest denormal is
                // at the same distance as its successor.
                // Note: denormals have the same exponent as the smallest normals.
                bool physicalSignificandIsZero = ((ToUInt32() & kSignificandMask) == 0);
                return physicalSignificandIsZero && (Exponent() != kDenormalExponent);
            }

        private:
            const uint32_t m_uValue;
        };

        /**
         * @brief 十次方数值缓存
         */
        class PowersOfTenCache
        {
        public:
            // Not all powers of ten are cached. The decimal exponent of two neighboring
            // cached numbers will differ by kDecimalExponentDistance.
            static const int kDecimalExponentDistance;

            static const int kMinDecimalExponent;
            static const int kMaxDecimalExponent;

            // Returns a cached power-of-ten with a binary exponent in the range
            // [min_exponent; max_exponent] (boundaries included).
            static void GetCachedPowerForBinaryExponentRange(int minExponent, int maxExponent, DiyFp* power,
                int* decimalExponent);

            // Returns a cached power of ten x ~= 10^k such that
            //   k <= decimal_exponent < k + kCachedPowersDecimalDistance.
            // The given decimal_exponent must satisfy
            //   kMinDecimalExponent <= requested_exponent, and
            //   requested_exponent < kMaxDecimalExponent + kDecimalExponentDistance.
            static void GetCachedPowerForDecimalExponent(int requestedExponent, DiyFp* power, int* foundExponent);
        };
    }
}
