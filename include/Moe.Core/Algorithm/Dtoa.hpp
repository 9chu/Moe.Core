/**
 * @file
 * @date 2017/4/30
 * @see https://github.com/google/double-conversion
 */
#pragma once
#include "../Utility/Misc.hpp"

#include <string>

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
            static void GetCachedPowerForBinaryExponentRange(int minExponent, int maxExponent, DiyFp& power,
                int& decimalExponent);

            // Returns a cached power of ten x ~= 10^k such that
            //   k <= decimal_exponent < k + kCachedPowersDecimalDistance.
            // The given decimal_exponent must satisfy
            //   kMinDecimalExponent <= requested_exponent, and
            //   requested_exponent < kMaxDecimalExponent + kDecimalExponentDistance.
            static void GetCachedPowerForDecimalExponent(int requestedExponent, DiyFp& power, int& foundExponent);
        };
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        enum FastDtoaMode
        {
            Shortest,
            ShortestSingle,
            Precision,
        };

        static const int kFastDtoaMaximalLength = 17;
        static const int kFastDtoaMaximalSingleLength = 9;

        class FastDtoa
        {
        public:
            static const int kMinimalTargetExponent = -60;
            static const int kMaximalTargetExponent = -32;

            template <typename T>
            static bool RoundWeed(MutableArrayView<T>& buffer, size_t length, uint64_t distanceTooHighW,
                uint64_t unsafeInterval, uint64_t rest, uint64_t tenKappa, uint64_t unit)
            {
                uint64_t smallDistance = distanceTooHighW - unit;
                uint64_t bigDistance = distanceTooHighW + unit;

                assert(rest <= unsafeInterval);
                while (rest < smallDistance && unsafeInterval - rest >= tenKappa &&
                    (rest + tenKappa < smallDistance || smallDistance - rest >= rest + tenKappa - smallDistance))
                {
                    --buffer[length - 1];
                    rest += tenKappa;
                }

                if (rest < bigDistance && unsafeInterval - rest >= tenKappa &&
                    (rest + tenKappa < bigDistance || bigDistance - rest > rest + tenKappa - bigDistance))
                {
                    return false;
                }

                return (2 * unit <= rest) && (rest <= unsafeInterval - 4 * unit);
            }

            template <typename T>
            static bool RoundWeedCounted(MutableArrayView<T>& buffer, size_t length, uint64_t rest, uint64_t tenKappa,
                uint64_t unit, int& kappa)
            {
                assert(rest < tenKappa);

                if (unit >= tenKappa)
                    return false;
                if (tenKappa - unit <= unit)
                    return false;
                if ((tenKappa - rest > rest) && (tenKappa - 2 * rest >= 2 * unit))
                    return true;
                if ((rest > unit) && (tenKappa - (rest - unit) <= (rest - unit)))
                {
                    buffer[length - 1]++;
                    for (size_t i = length - 1; i > 0; --i)
                    {
                        if (buffer[i] != '0' + 10)
                            break;
                        buffer[i] = '0';
                        ++buffer[i - 1];
                    }

                    if (buffer[0] == '0' + 10)
                    {
                        buffer[0] = '1';
                        kappa += 1;
                    }

                    return true;
                }

                return false;
            }

            static void BiggestPowerTen(uint32_t number, size_t numberBits, uint32_t& power, int& exponentPlusOne)
            {
                static unsigned int const kSmallPowersOfTen[] = {
                    0, 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
                };

                assert(number < (1u << (numberBits + 1)));
                int exponentPlusOneGuess = ((numberBits + 1) * 1233 >> 12);
                exponentPlusOneGuess++;
                if (number < kSmallPowersOfTen[exponentPlusOneGuess])
                    --exponentPlusOneGuess;
                power = kSmallPowersOfTen[exponentPlusOneGuess];
                exponentPlusOne = exponentPlusOneGuess;
            }

            template <typename T>
            static bool DigitGen(DiyFp low, DiyFp w, DiyFp high, MutableArrayView<T>& buffer, size_t& length,
                int& kappa)
            {
                assert(low.Exponent() == w.Exponent() && w.Exponent() == high.Exponent());
                assert(low.Significand() + 1 <= high.Significand() - 1);
                assert(kMinimalTargetExponent <= w.Exponent() && w.Exponent() <= kMaximalTargetExponent);

                uint64_t unit = 1;
                DiyFp tooLow = DiyFp(low.Significand() - unit, low.Exponent());
                DiyFp tooHigh = DiyFp(high.Significand() + unit, high.Exponent());

                DiyFp unsafeInterval = DiyFp::Minus(tooHigh, tooLow);
                DiyFp one = DiyFp(static_cast<uint64_t>(1) << -w.Exponent(), w.Exponent());
                uint32_t integrals = static_cast<uint32_t>(tooHigh.Significand() >> -one.Exponent());
                uint64_t fractionals = tooHigh.Significand() & (one.Significand() - 1);
                uint32_t divisor = 0;
                int divisorExponentPlusOne = 0;
                BiggestPowerTen(integrals, static_cast<size_t>(DiyFp::kSignificandSize + one.Exponent()), divisor,
                    divisorExponentPlusOne);
                kappa = divisorExponentPlusOne;
                length = 0;

                while (kappa > 0)
                {
                    int digit = integrals / divisor;
                    assert(digit <= 9);
                    buffer[length] = static_cast<char>('0' + digit);
                    ++(length);
                    integrals %= divisor;
                    --(kappa);
                    uint64_t rest = (static_cast<uint64_t>(integrals) << -one.Exponent()) + fractionals;
                    if (rest < unsafeInterval.Significand())
                    {
                        return RoundWeed(buffer, length, DiyFp::Minus(tooHigh, w).Significand(),
                            unsafeInterval.Significand(), rest, static_cast<uint64_t>(divisor) << -one.Exponent(),
                            unit);
                    }
                    divisor /= 10;
                }

                assert(one.Exponent() >= -60);
                assert(fractionals < one.Significand());
                assert(0xFFFFFFFFFFFFFFFFull / 10 >= one.Significand());
                for (;;)
                {
                    fractionals *= 10;
                    unit *= 10;
                    unsafeInterval.SetSignificand(unsafeInterval.Significand() * 10);
                    int digit = static_cast<int>(fractionals >> -one.Exponent());
                    assert(digit <= 9);
                    buffer[length] = static_cast<char>('0' + digit);
                    (length)++;
                    fractionals &= one.Significand() - 1;
                    (kappa)--;
                    if (fractionals < unsafeInterval.Significand())
                    {
                        return RoundWeed(buffer, length, DiyFp::Minus(tooHigh, w).Significand() * unit,
                            unsafeInterval.Significand(), fractionals, one.Significand(), unit);
                    }
                }
            }

            template <typename T>
            static bool DigitGenCounted(DiyFp w, size_t requestedDigits, MutableArrayView<T>& buffer, size_t& length,
                int& kappa)
            {
                assert(kMinimalTargetExponent <= w.Exponent() && w.Exponent() <= kMaximalTargetExponent);
                assert(kMinimalTargetExponent >= -60);
                assert(kMaximalTargetExponent <= -32);

                uint64_t wError = 1;
                DiyFp one = DiyFp(static_cast<uint64_t>(1) << -w.Exponent(), w.Exponent());
                uint32_t integrals = static_cast<uint32_t>(w.Significand() >> -one.Exponent());
                uint64_t fractionals = w.Significand() & (one.Significand() - 1);
                uint32_t divisor;
                int divisorExponentPlusOne;
                BiggestPowerTen(integrals, static_cast<size_t>(DiyFp::kSignificandSize + one.Exponent()), divisor,
                    divisorExponentPlusOne);
                kappa = divisorExponentPlusOne;
                length = 0;

                while (kappa > 0)
                {
                    int digit = integrals / divisor;
                    assert(digit <= 9);
                    buffer[length] = static_cast<char>('0' + digit);
                    ++(length);
                    requestedDigits--;
                    integrals %= divisor;
                    --(kappa);
                    if (requestedDigits == 0)
                        break;
                    divisor /= 10;
                }

                if (requestedDigits == 0)
                {
                    uint64_t rest = (static_cast<uint64_t>(integrals) << -one.Exponent()) + fractionals;
                    return RoundWeedCounted(buffer, length, rest, static_cast<uint64_t>(divisor) << -one.Exponent(),
                        wError, kappa);
                }

                assert(one.Exponent() >= -60);
                assert(fractionals < one.Significand());
                assert(0xFFFFFFFFFFFFFFFFull / 10 >= one.Significand());
                while (requestedDigits > 0 && fractionals > wError)
                {
                    fractionals *= 10;
                    wError *= 10;

                    int digit = static_cast<int>(fractionals >> -one.Exponent());
                    assert(digit <= 9);
                    buffer[length] = static_cast<char>('0' + digit);
                    (length)++;
                    requestedDigits--;
                    fractionals &= one.Significand() - 1;
                    (kappa)--;
                }

                if (requestedDigits != 0)
                    return false;

                return RoundWeedCounted(buffer, length, fractionals, one.Significand(), wError, kappa);
            }

            template <typename T>
            static bool Grisu3(double v, FastDtoaMode mode, MutableArrayView<T>& buffer, size_t& length,
                int& decimalExponent)
            {
                DiyFp w = Double(v).ToNormalizedDiyFp();
                DiyFp boundaryMinus, boundaryPlus;
                if (mode == FastDtoaMode::Shortest)
                    Double(v).NormalizedBoundaries(boundaryMinus, boundaryPlus);
                else
                {
                    assert(mode == FastDtoaMode ::ShortestSingle);
                    float singleV = static_cast<float>(v);
                    Single(singleV).NormalizedBoundaries(boundaryMinus, boundaryPlus);
                }

                assert(boundaryPlus.Exponent() == w.Exponent());
                DiyFp tenMk;
                int mk = 0;
                int tenMkMinimalBinaryExponent = kMinimalTargetExponent - (w.Exponent() + DiyFp::kSignificandSize);
                int tenMkMaximalBinaryExponent = kMaximalTargetExponent - (w.Exponent() + DiyFp::kSignificandSize);
                PowersOfTenCache::GetCachedPowerForBinaryExponentRange(tenMkMinimalBinaryExponent,
                    tenMkMaximalBinaryExponent, tenMk, mk);
                assert((kMinimalTargetExponent <= w.Exponent() + tenMk.Exponent() + DiyFp::kSignificandSize) &&
                    (kMaximalTargetExponent >= w.Exponent() + tenMk.Exponent() + DiyFp::kSignificandSize));

                DiyFp scaledW = DiyFp::Times(w, tenMk);
                assert(scaledW.Exponent() == boundaryPlus.Exponent() + tenMk.Exponent() + DiyFp::kSignificandSize);

                DiyFp scaledBoundaryMinus = DiyFp::Times(boundaryMinus, tenMk);
                DiyFp scaledBoundaryPlus  = DiyFp::Times(boundaryPlus, tenMk);

                int kappa = 0;
                bool result = DigitGen(scaledBoundaryMinus, scaledW, scaledBoundaryPlus, buffer, length, kappa);
                decimalExponent = -mk + kappa;
                return result;
            }

            template <typename T>
            static bool Grisu3Counted(double v, size_t requestedDigits, MutableArrayView<T>& buffer, size_t& length,
                int& decimalExponent)
            {
                DiyFp w = Double(v).ToNormalizedDiyFp();
                DiyFp tenMk;
                int mk = 0;
                int tenMkMinimalBinaryExponent = kMinimalTargetExponent - (w.Exponent() + DiyFp::kSignificandSize);
                int tenMkMaximalBinaryExponent = kMaximalTargetExponent - (w.Exponent() + DiyFp::kSignificandSize);
                PowersOfTenCache::GetCachedPowerForBinaryExponentRange(tenMkMinimalBinaryExponent,
                    tenMkMaximalBinaryExponent, tenMk, mk);
                assert((kMinimalTargetExponent <= w.Exponent() + tenMk.Exponent() + DiyFp::kSignificandSize) &&
                    (kMaximalTargetExponent >= w.Exponent() + tenMk.Exponent() + DiyFp::kSignificandSize));

                DiyFp scaledW = DiyFp::Times(w, tenMk);

                int kappa = 0;
                bool result = DigitGenCounted(scaledW, requestedDigits, buffer, length, kappa);
                decimalExponent = -mk + kappa;
                return result;
            }

            template <typename T>
            static bool Dtoa(double v, FastDtoaMode mode, size_t requestedDigits, MutableArrayView<T>& buffer,
                size_t& length, int& decimalPoint)
            {
                assert(v > 0);
                assert(!Double(v).IsSpecial());

                bool result = false;
                int decimalExponent = 0;
                switch (mode)
                {
                    case FastDtoaMode::Shortest:
                    case FastDtoaMode::ShortestSingle:
                        result = Grisu3(v, mode, buffer, length, decimalExponent);
                        break;
                    case FastDtoaMode::Precision:
                        result = Grisu3Counted(v, requestedDigits, buffer, length, decimalExponent);
                        break;
                    default:
                        MOE_UNREACHABLE();
                        break;
                }

                if (result)
                {
                    decimalPoint = length + decimalExponent;
                    buffer[length] = '\0';
                }

                return result;
            }
        };
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        class UInt128
        {
        private:
            static const uint64_t kMask32 = 0xFFFFFFFF;

        public:
            UInt128()
                : m_ulHighBits(0), m_ulLowBits(0) {}
            UInt128(uint64_t high, uint64_t low)
                : m_ulHighBits(high), m_ulLowBits(low) {}

            void Multiply(uint32_t multiplicand)
            {
                uint64_t accumulator;

                accumulator = (m_ulLowBits & kMask32) * multiplicand;
                uint32_t part = static_cast<uint32_t>(accumulator & kMask32);
                accumulator >>= 32;
                accumulator = accumulator + (m_ulLowBits >> 32) * multiplicand;
                m_ulLowBits = (accumulator << 32) + part;
                accumulator >>= 32;
                accumulator = accumulator + (m_ulHighBits & kMask32) * multiplicand;
                part = static_cast<uint32_t>(accumulator & kMask32);
                accumulator >>= 32;
                accumulator = accumulator + (m_ulHighBits >> 32) * multiplicand;
                m_ulHighBits = (accumulator << 32) + part;

                assert((accumulator >> 32) == 0);
            }

            void Shift(int shiftAmount)
            {
                assert(-64 <= shiftAmount && shiftAmount <= 64);

                if (shiftAmount == 0)
                    return;

                if (shiftAmount == -64)
                {
                    m_ulHighBits = m_ulLowBits;
                    m_ulLowBits = 0;
                }
                else if (shiftAmount == 64)
                {
                    m_ulLowBits = m_ulHighBits;
                    m_ulHighBits = 0;
                }
                else if (shiftAmount <= 0)
                {
                    m_ulHighBits <<= -shiftAmount;
                    m_ulHighBits += m_ulLowBits >> (64 + shiftAmount);
                    m_ulLowBits <<= -shiftAmount;
                }
                else
                {
                    m_ulLowBits >>= shiftAmount;
                    m_ulLowBits += m_ulHighBits << (64 - shiftAmount);
                    m_ulHighBits >>= shiftAmount;
                }
            }

            int DivModPowerOf2(int power)
            {
                if (power >= 64)
                {
                    int result = static_cast<int>(m_ulHighBits >> (power - 64));
                    m_ulHighBits -= static_cast<uint64_t>(result) << (power - 64);
                    return result;
                }

                uint64_t partLow = m_ulLowBits >> power;
                uint64_t partHigh = m_ulHighBits << (64 - power);
                int result = static_cast<int>(partLow + partHigh);
                m_ulHighBits = 0;
                m_ulLowBits -= partLow << power;
                return result;
            }

            bool IsZero()const { return m_ulHighBits == 0 && m_ulLowBits == 0; }

            int BitAt(size_t position)const
            {
                if (position >= 64)
                    return static_cast<int>(m_ulHighBits >> (position - 64)) & 1;
                return static_cast<int>(m_ulLowBits >> position) & 1;
            }

        private:
            uint64_t m_ulHighBits;
            uint64_t m_ulLowBits;
        };

        class FixedDtoa
        {
        public:
            static const int kDoubleSignificandSize = 53;

            template <typename T>
            static void FillDigits32FixedLength(uint32_t number, size_t requestedLength, MutableArrayView<T>& buffer,
                size_t& length)
            {
                // for (int i = requestedLength - 1; i >= 0; --i)
                for (size_t i = requestedLength; i-- > 0;)
                {
                    buffer[length + i] = '0' + number % 10;
                    number /= 10;
                }

                length += requestedLength;
            }

            template <typename T>
            static void FillDigits32(uint32_t number, MutableArrayView<T>& buffer, size_t& length)
            {
                size_t numberLength = 0;
                while (number != 0)
                {
                    int digit = number % 10;
                    number /= 10;
                    buffer[length + numberLength] = static_cast<T>('0' + digit);
                    numberLength++;
                }

                size_t i = length;
                size_t j = length + numberLength - 1;
                while (i < j)
                {
                    T tmp = buffer[i];
                    buffer[i] = buffer[j];
                    buffer[j] = tmp;
                    i++;
                    j--;
                }

                length += numberLength;
            }

            template <typename T>
            static void FillDigits64FixedLength(uint64_t number, MutableArrayView<T>& buffer, size_t& length)
            {
                const uint32_t kTen7 = 10000000;

                uint32_t part2 = static_cast<uint32_t>(number % kTen7);
                number /= kTen7;
                uint32_t part1 = static_cast<uint32_t>(number % kTen7);
                uint32_t part0 = static_cast<uint32_t>(number / kTen7);

                FillDigits32FixedLength(part0, 3, buffer, length);
                FillDigits32FixedLength(part1, 7, buffer, length);
                FillDigits32FixedLength(part2, 7, buffer, length);
            }

            template <typename T>
            static void FillDigits64(uint64_t number, MutableArrayView<T>& buffer, size_t& length)
            {
                const uint32_t kTen7 = 10000000;

                uint32_t part2 = static_cast<uint32_t>(number % kTen7);
                number /= kTen7;
                uint32_t part1 = static_cast<uint32_t>(number % kTen7);
                uint32_t part0 = static_cast<uint32_t>(number / kTen7);

                if (part0 != 0)
                {
                    FillDigits32(part0, buffer, length);
                    FillDigits32FixedLength(part1, 7, buffer, length);
                    FillDigits32FixedLength(part2, 7, buffer, length);
                }
                else if (part1 != 0)
                {
                    FillDigits32(part1, buffer, length);
                    FillDigits32FixedLength(part2, 7, buffer, length);
                }
                else
                    FillDigits32(part2, buffer, length);
            }

            template <typename T>
            static void RoundUp(MutableArrayView<T>& buffer, size_t& length, int& decimalPoint)
            {
                if (length == 0)
                {
                    buffer[0] = '1';
                    decimalPoint = 1;
                    length = 1;
                    return;
                }

                buffer[length - 1]++;
                for (size_t i = length - 1; i > 0; --i)
                {
                    if (buffer[i] != '0' + 10)
                        return;
                    buffer[i] = '0';
                    buffer[i - 1]++;
                }

                if (buffer[0] == '0' + 10)
                {
                    buffer[0] = '1';
                    ++decimalPoint;
                }
            }

            template <typename T>
            static void FillFractionals(uint64_t fractionals, int exponent, size_t fractionalCount,
                MutableArrayView<T>& buffer, size_t& length, int& decimalPoint)
            {
                assert(-128 <= exponent && exponent <= 0);

                if (-exponent <= 64)
                {
                    assert(fractionals >> 56 == 0);
                    int point = -exponent;
                    for (size_t i = 0; i < fractionalCount; ++i)
                    {
                        if (fractionals == 0)
                            break;

                        fractionals *= 5;
                        point--;
                        int digit = static_cast<int>(fractionals >> point);
                        assert(digit <= 9);
                        buffer[length] = static_cast<char>('0' + digit);
                        ++length;
                        fractionals -= static_cast<uint64_t>(digit) << point;
                    }

                    assert(fractionals == 0 || point - 1 >= 0);
                    if ((fractionals != 0) && ((fractionals >> (point - 1)) & 1) == 1)
                        RoundUp(buffer, length, decimalPoint);
                }
                else
                {
                    assert(64 < -exponent && -exponent <= 128);
                    UInt128 fractionals128 = UInt128(fractionals, 0);
                    fractionals128.Shift(-exponent - 64);
                    size_t point = 128;
                    for (size_t i = 0; i < fractionalCount; ++i)
                    {
                        if (fractionals128.IsZero())
                            break;

                        fractionals128.Multiply(5);
                        point--;
                        int digit = fractionals128.DivModPowerOf2(point);
                        assert(digit <= 9);
                        buffer[length] = static_cast<char>('0' + digit);
                        ++length;
                    }
                    if (fractionals128.BitAt(point - 1) == 1)
                        RoundUp(buffer, length, decimalPoint);
                }
            }

            template <typename T>
            static void TrimZeros(MutableArrayView<T>& buffer, size_t& length, int& decimalPoint)
            {
                while (length > 0 && buffer[length - 1] == '0')
                    --length;

                size_t firstNonZero = 0;
                while (firstNonZero < length && buffer[firstNonZero] == '0')
                    firstNonZero++;

                if (firstNonZero != 0)
                {
                    for (size_t i = firstNonZero; i < length; ++i)
                        buffer[i - firstNonZero] = buffer[i];

                    length -= firstNonZero;
                    decimalPoint -= firstNonZero;
                }
            }

            template <typename T>
            static bool Dtoa(double v, size_t fractionalCount, MutableArrayView<T>& buffer, size_t& length,
                int& decimalPoint)
            {
                const uint32_t kMaxUInt32 = 0xFFFFFFFF;
                uint64_t significand = Double(v).Significand();
                int exponent = Double(v).Exponent();

                if (exponent > 20)
                    return false;
                if (fractionalCount > 20)
                    return false;

                length = 0;
                if (exponent + kDoubleSignificandSize > 64)
                {
                    const uint64_t kFive17 = 0xB1A2BC2EC5ull;  // 5^17
                    uint64_t divisor = kFive17;
                    int divisor_power = 17;
                    uint64_t dividend = significand;
                    uint32_t quotient;
                    uint64_t remainder;

                    if (exponent > divisor_power)
                    {
                        dividend <<= exponent - divisor_power;
                        quotient = static_cast<uint32_t>(dividend / divisor);
                        remainder = (dividend % divisor) << divisor_power;
                    }
                    else
                    {
                        divisor <<= divisor_power - exponent;
                        quotient = static_cast<uint32_t>(dividend / divisor);
                        remainder = (dividend % divisor) << exponent;
                    }

                    FillDigits32(quotient, buffer, length);
                    FillDigits64FixedLength(remainder, buffer, length);
                    decimalPoint = length;
                }
                else if (exponent >= 0)
                {
                    significand <<= exponent;
                    FillDigits64(significand, buffer, length);
                    decimalPoint = length;
                }
                else if (exponent > -kDoubleSignificandSize)
                {
                    uint64_t integrals = significand >> -exponent;
                    uint64_t fractionals = significand - (integrals << -exponent);
                    if (integrals > kMaxUInt32)
                        FillDigits64(integrals, buffer, length);
                    else
                        FillDigits32(static_cast<uint32_t>(integrals), buffer, length);

                    decimalPoint = length;
                    FillFractionals(fractionals, exponent, fractionalCount, buffer, length, decimalPoint);
                }
                else if (exponent < -128)
                {
                    assert(fractionalCount <= 20);
                    buffer[0] = '\0';
                    length = 0;
                    decimalPoint = -fractionalCount;
                }
                else
                {
                    decimalPoint = 0;
                    FillFractionals(significand, exponent, fractionalCount, buffer, length, decimalPoint);
                }

                TrimZeros(buffer, length, decimalPoint);
                buffer[length] = '\0';
                if (length == 0)
                    decimalPoint = -fractionalCount;
                return true;
            }
        };
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        class Bignum :
            public NonCopyable
        {
        public:
            static const size_t kMaxSignificantBits = 3584;

            static int Compare(const Bignum& a, const Bignum& b);
            static bool Equal(const Bignum& a, const Bignum& b) { return Compare(a, b) == 0; }
            static bool LessEqual(const Bignum& a, const Bignum& b) { return Compare(a, b) <= 0; }
            static bool Less(const Bignum& a, const Bignum& b) { return Compare(a, b) < 0; }

            static int PlusCompare(const Bignum& a, const Bignum& b, const Bignum& c);

            static bool PlusEqual(const Bignum& a, const Bignum& b, const Bignum& c)
            {
                return PlusCompare(a, b, c) == 0;
            }

            static bool PlusLessEqual(const Bignum& a, const Bignum& b, const Bignum& c)
            {
                return PlusCompare(a, b, c) <= 0;
            }

            static bool PlusLess(const Bignum& a, const Bignum& b, const Bignum& c) { return PlusCompare(a, b, c) < 0; }

        private:
            using Chunk = uint32_t;
            using DoubleChunk = uint64_t;

            static const int kChunkSize = sizeof(Chunk) * 8;
            static const int kDoubleChunkSize = sizeof(DoubleChunk) * 8;
            static const size_t kBigitSize = 28;
            static const Chunk kBigitMask = (1 << kBigitSize) - 1;
            static const size_t kBigitCapacity = kMaxSignificantBits / kBigitSize;

            template<typename T>
            static size_t BitSize(T value)
            {
                MOE_UNUSED(value);
                return 8 * sizeof(value);
            }

            template <typename T>
            static size_t SizeInHexChars(T number)
            {
                assert(number > 0);
                T val = number;
                size_t result = 0;

                while (val != 0)
                {
                    val >>= 4;
                    result++;
                }

                return result;
            }

            template <typename T>
            static T HexCharOfValue(int value)
            {
                assert(0 <= value && value <= 16);
                if (value < 10)
                    return static_cast<T>(value + '0');
                return static_cast<T>(value - 10 + 'A');
            }

            template <typename T>
            static uint64_t ReadUInt64(const ArrayView<T>& buffer, size_t from, size_t digitsToRead)
            {
                uint64_t result = 0;
                for (size_t i = from; i < from + digitsToRead; ++i)
                {
                    int digit = buffer[i] - '0';
                    assert(0 <= digit && digit <= 9);
                    result = result * 10 + digit;
                }
                return result;
            }

            template <typename T>
            static int HexCharValue(T c)
            {
                if ('0' <= c && c <= '9')
                    return c - '0';
                if ('a' <= c && c <= 'f')
                    return 10 + c - 'a';
                assert('A' <= c && c <= 'F');
                return 10 + c - 'A';
            }

        public:
            Bignum();

        public:
            void AssignUInt16(uint16_t value);
            void AssignUInt64(uint64_t value);
            void AssignBignum(const Bignum& other);
            void AssignPowerUInt16(uint16_t base, int powerExponent);

            template <typename T = char>
            void AssignDecimalString(const ArrayView<T>& value)
            {
                const int kMaxUint64DecimalDigits = 19;

                Zero();
                size_t length = value.Size();
                unsigned int pos = 0;
                while (length >= kMaxUint64DecimalDigits)
                {
                    uint64_t digits = ReadUInt64(value, pos, kMaxUint64DecimalDigits);
                    pos += kMaxUint64DecimalDigits;
                    length -= kMaxUint64DecimalDigits;
                    MultiplyByPowerOfTen(kMaxUint64DecimalDigits);
                    AddUInt64(digits);
                }

                uint64_t digits = ReadUInt64(value, pos, length);
                MultiplyByPowerOfTen(length);
                AddUInt64(digits);
                Clamp();
            }

            template <typename T = char>
            void AssignHexString(const ArrayView<T>& value)
            {
                Zero();
                size_t length = value.Size();

                size_t neededBigits = length * 4 / kBigitSize + 1;
                EnsureCapacity(neededBigits);
                int stringIndex = static_cast<int>(length) - 1;
                for (size_t i = 0; i < neededBigits - 1; ++i)
                {
                    Chunk currentBigit = 0;
                    for (int j = 0; j < static_cast<int>(kBigitSize) / 4; j++)
                        currentBigit += HexCharValue(value[stringIndex--]) << (j * 4);
                    m_stBigits[i] = currentBigit;
                }
                m_uUsedDigits = neededBigits - 1;

                Chunk mostSignificantBigit = 0;
                for (int j = 0; j <= stringIndex; ++j)
                {
                    mostSignificantBigit <<= 4;
                    mostSignificantBigit += HexCharValue(value[j]);
                }

                if (mostSignificantBigit != 0)
                {
                    m_stBigits[m_uUsedDigits] = mostSignificantBigit;
                    m_uUsedDigits++;
                }

                Clamp();
            }

            void AddUInt64(uint64_t operand);
            void AddBignum(const Bignum& other);
            void SubtractBignum(const Bignum& other);

            void Square();
            void ShiftLeft(int shiftAmount);
            void MultiplyByUInt32(uint32_t factor);
            void MultiplyByUInt64(uint64_t factor);
            void MultiplyByPowerOfTen(int exponent);
            void Times10() { return MultiplyByUInt32(10); }
            uint16_t DivideModuloIntBignum(const Bignum& other);

            template <typename T = char>
            bool ToHexString(T* buffer, size_t bufferSize)const
            {
                assert(IsClamped());
                assert(kBigitSize % 4 == 0);
                const int kHexCharsPerBigit = kBigitSize / 4;

                if (m_uUsedDigits == 0)
                {
                    if (bufferSize < 2)
                        return false;
                    buffer[0] = '0';
                    buffer[1] = '\0';
                    return true;
                }

                size_t neededChars = (BigitLength() - 1) * kHexCharsPerBigit +
                    SizeInHexChars(m_stBigits[m_uUsedDigits - 1]) + 1;
                if (neededChars > bufferSize)
                    return false;

                size_t stringIndex = neededChars - 1;
                buffer[stringIndex--] = '\0';
                for (int i = 0; i < m_iExponent; ++i)
                {
                    for (int j = 0; j < kHexCharsPerBigit; ++j)
                    {
                        buffer[stringIndex--] = '0';
                    }
                }
                for (size_t i = 0; i < m_uUsedDigits - 1; ++i)
                {
                    Chunk currentBigit = m_stBigits[i];
                    for (int j = 0; j < kHexCharsPerBigit; ++j)
                    {
                        buffer[stringIndex--] = HexCharOfValue<T>(currentBigit & 0xF);
                        currentBigit >>= 4;
                    }
                }

                Chunk mostSignificantBigit = m_stBigits[m_uUsedDigits - 1];
                while (mostSignificantBigit != 0)
                {
                    buffer[stringIndex--] = HexCharOfValue<T>(mostSignificantBigit & 0xF);
                    mostSignificantBigit >>= 4;
                }

                return true;
            }

        private:
            void EnsureCapacity(size_t size)
            {
                if (size > kBigitCapacity)
                    MOE_UNREACHABLE();
            }
            void Align(const Bignum& other);
            void Clamp();
            bool IsClamped()const;
            void Zero();
            void BigitsShiftLeft(int shiftAmount);
            int BigitLength()const { return static_cast<int>(m_uUsedDigits) + m_iExponent; }
            Chunk BigitAt(int index)const;
            void SubtractTimes(const Bignum& other, int factor);

        private:
            Chunk m_aBigitsBuffer[kBigitCapacity];
            MutableArrayView<Chunk> m_stBigits;
            size_t m_uUsedDigits;
            int m_iExponent;
        };
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        enum class BignumDtoaMode
        {
            Shortest,
            ShortestSingle,
            Fixed,
            Precision,
        };

        class BignumDtoa
        {
        public:
            static int NormalizedExponent(uint64_t significand, int exponent);
            static int EstimatePower(int exponent);
            static void InitialScaledStartValues(uint64_t significand, int exponent, bool lowerBoundaryIsCloser,
                int estimatedPower, bool needBoundaryDeltas, Bignum& numerator, Bignum& denominator, Bignum& deltaMinus,
                Bignum& deltaPlus);
            static void FixupMultiply10(int estimatedPower, bool isEven, int& decimalPoint, Bignum& numerator,
                Bignum& denominator, Bignum& deltaMinus, Bignum& deltaPlus);

            template <typename T>
            static void GenerateShortestDigits(Bignum& numerator, Bignum& denominator, Bignum* deltaMinus,
                Bignum* deltaPlus, bool isEven, MutableArrayView<T>& buffer, size_t& length)
            {
                if (Bignum::Equal(*deltaMinus, *deltaPlus))
                    deltaPlus = deltaMinus;

                length = 0;
                for (;;)
                {
                    uint16_t digit;
                    digit = numerator.DivideModuloIntBignum(denominator);
                    assert(digit <= 9);
                    buffer[length++] = static_cast<T>(digit + '0');

                    bool inDeltaRoomMinus;
                    bool inDeltaRoomPlus;
                    if (isEven)
                        inDeltaRoomMinus = Bignum::LessEqual(numerator, *deltaMinus);
                    else
                        inDeltaRoomMinus = Bignum::Less(numerator, *deltaMinus);

                    if (isEven)
                        inDeltaRoomPlus = Bignum::PlusCompare(numerator, *deltaPlus, denominator) >= 0;
                    else
                        inDeltaRoomPlus = Bignum::PlusCompare(numerator, *deltaPlus, denominator) > 0;

                    if (!inDeltaRoomMinus && !inDeltaRoomPlus)
                    {
                        numerator.Times10();
                        deltaMinus->Times10();

                        if (deltaMinus != deltaPlus)
                            deltaPlus->Times10();
                    }
                    else if (inDeltaRoomMinus && inDeltaRoomPlus)
                    {
                        int compare = Bignum::PlusCompare(numerator, numerator, denominator);
                        if (compare < 0)
                        {
                        }
                        else if (compare > 0)
                        {
                            assert(buffer[length - 1] != '9');
                            ++buffer[length - 1];
                        }
                        else
                        {
                            if ((buffer[length - 1] - '0') % 2 == 0)
                            {
                            }
                            else
                            {
                                assert(buffer[length - 1] != '9');
                                ++buffer[length - 1];
                            }
                        }
                        return;
                    }
                    else if (inDeltaRoomMinus)
                        return;
                    else
                    {
                        assert(buffer[length -1] != '9');
                        buffer[length - 1]++;
                        return;
                    }
                }
            }

            template <typename T>
            static void GenerateCountedDigits(size_t count, int& decimalPoint, Bignum& numerator, Bignum& denominator,
                MutableArrayView<T>& buffer, size_t& length)
            {
                assert(count >= 1);
                for (size_t i = 0; i < count - 1; ++i)
                {
                    uint16_t digit;
                    digit = numerator.DivideModuloIntBignum(denominator);
                    assert(digit <= 9);
                    buffer[i] = static_cast<T>(digit + '0');
                    numerator.Times10();
                }

                uint16_t digit;
                digit = numerator.DivideModuloIntBignum(denominator);
                if (Bignum::PlusCompare(numerator, numerator, denominator) >= 0)
                    digit++;
                assert(digit <= 10);
                buffer[count - 1] = static_cast<T>(digit + '0');

                for (size_t i = count - 1; i > 0; --i)
                {
                    if (buffer[i] != '0' + 10)
                        break;
                    buffer[i] = '0';
                    buffer[i - 1]++;
                }

                if (buffer[0] == '0' + 10)
                {
                    buffer[0] = '1';
                    ++decimalPoint;
                }

                length = count;
            }

            template <typename T>
            static void BignumToFixed(int requestedDigits, int& decimalPoint, Bignum& numerator, Bignum& denominator,
                MutableArrayView<T>& buffer, size_t& length)
            {
                if (-decimalPoint > requestedDigits)
                {
                    decimalPoint = -requestedDigits;
                    length = 0;
                    return;
                }
                else if (-decimalPoint == requestedDigits)
                {
                    assert(decimalPoint == -requestedDigits);

                    denominator.Times10();
                    if (Bignum::PlusCompare(numerator, numerator, denominator) >= 0)
                    {
                        buffer[0] = '1';
                        length = 1;
                        ++decimalPoint;
                    }
                    else
                        length = 0;
                    return;
                }
                else
                {
                    int neededDigits = decimalPoint + requestedDigits;
                    assert(neededDigits >= 0);
                    GenerateCountedDigits(static_cast<size_t>(neededDigits), decimalPoint, numerator, denominator,
                        buffer, length);
                }
            }

            template <typename T>
            static void Dtoa(double v, BignumDtoaMode mode, size_t requestedDigits, MutableArrayView<T>& buffer,
                size_t& length, int& decimalPoint)
            {
                assert(v > 0);
                assert(!Double(v).IsSpecial());
                uint64_t significand;
                int exponent;
                bool lowerBoundaryIsCloser;
                if (mode == BignumDtoaMode::ShortestSingle)
                {
                    float f = static_cast<float>(v);
                    assert(f == v);
                    significand = Single(f).Significand();
                    exponent = Single(f).Exponent();
                    lowerBoundaryIsCloser = Single(f).LowerBoundaryIsCloser();
                }
                else
                {
                    significand = Double(v).Significand();
                    exponent = Double(v).Exponent();
                    lowerBoundaryIsCloser = Double(v).LowerBoundaryIsCloser();
                }

                bool needBoundaryDeltas = (mode == BignumDtoaMode::Shortest || mode == BignumDtoaMode::ShortestSingle);

                bool isEven = (significand & 1) == 0;
                int normalizedExponent = NormalizedExponent(significand, exponent);
                int estimatedPower = EstimatePower(normalizedExponent);

                if (mode == BignumDtoaMode::Fixed && -estimatedPower - 1 > static_cast<int>(requestedDigits))
                {
                    buffer[0] = '\0';
                    length = 0;
                    decimalPoint = -requestedDigits;
                    return;
                }

                Bignum numerator;
                Bignum denominator;
                Bignum deltaMinus;
                Bignum deltaPlus;
                assert(Bignum::kMaxSignificantBits >= 324*4);
                InitialScaledStartValues(significand, exponent, lowerBoundaryIsCloser, estimatedPower,
                    needBoundaryDeltas, numerator, denominator, deltaMinus, deltaPlus);
                FixupMultiply10(estimatedPower, isEven, decimalPoint, numerator, denominator, deltaMinus, deltaPlus);

                switch (mode)
                {
                    case BignumDtoaMode::Shortest:
                    case BignumDtoaMode::ShortestSingle:
                        GenerateShortestDigits(numerator, denominator, &deltaMinus, &deltaPlus, isEven, buffer, length);
                        break;
                    case BignumDtoaMode::Fixed:
                        BignumToFixed(requestedDigits, decimalPoint, numerator, denominator, buffer, length);
                        break;
                    case BignumDtoaMode::Precision:
                        GenerateCountedDigits(requestedDigits, decimalPoint, numerator, denominator, buffer, length);
                        break;
                    default:
                        MOE_UNREACHABLE();
                        break;
                }

                buffer[length] = '\0';
            }
        };
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T = char>
        class StringBuilder :
            public NonCopyable
        {
        public:
            StringBuilder(T* buffer, size_t bufferSize)
                : m_stBuffer(buffer, bufferSize) {}

            ~StringBuilder()
            {
                if (!isFinalized())
                    Finalize();
            }

        public:
            size_t Size()const
            {
                return m_stBuffer.Size();
            }

            size_t Position()const
            {
                return m_uPosition;
            }

            void Reset()
            {
                m_uPosition = 0;
                m_bFinalized = false;
            }

            void AddCharacter(T c)
            {
                assert(c != '\0');
                assert(!isFinalized() && m_uPosition < m_stBuffer.Size());
                m_stBuffer[m_uPosition++] = c;
            }

            void AddString(const T* s)
            {
                AddSubstring(s, std::char_traits<T>::length(s));
            }

            void AddSubstring(const T* s, size_t n)
            {
                assert(!isFinalized() && m_uPosition + n < m_stBuffer.Size());
                assert(n <= std::char_traits<T>::length(s));
                ::memmove(&m_stBuffer[m_uPosition], s, n * sizeof(T));
                m_uPosition += n;
            }

            void AddPadding(T c, size_t count)
            {
                for (size_t i = 0; i < count; i++)
                    AddCharacter(c);
            }

            bool isFinalized()const
            {
                return m_bFinalized;
            }

            T* Finalize()
            {
                assert(!isFinalized() && m_uPosition < m_stBuffer.Size());
                m_stBuffer[m_uPosition] = static_cast<T>('\0');
                // Make sure nobody managed to add a 0-character to the
                // buffer while building the string.
                assert(std::char_traits<T>::length(m_stBuffer.GetBuffer()) == static_cast<size_t>(m_uPosition));
                m_bFinalized = true;
                return m_stBuffer.GetBuffer();
            }

        private:
            MutableArrayView<T> m_stBuffer;
            size_t m_uPosition = 0;
            bool m_bFinalized = false;
        };
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        enum class DtoaFlags
        {
            Default = 0,
            EmitPositiveExponentSign = 1,
            EmitTrailingDecimalPoint = 2,
            EmitTrailingZeroAfterPoint = 4,
            UniqueZero = 8,
        };

        template <typename T = char>
        class DoubleToStringConverter :
            public NonCopyable
        {
        public:
            // When calling ToFixed with a double > 10^kMaxFixedDigitsBeforePoint
            // or a requested_digits parameter > kMaxFixedDigitsAfterPoint then the
            // function returns false.
            static const int kMaxFixedDigitsBeforePoint = 60;
            static const int kMaxFixedDigitsAfterPoint = 60;

            // When calling ToExponential with a requested_digits
            // parameter > kMaxExponentialDigits then the function returns false.
            static const int kMaxExponentialDigits = 120;

            // When calling ToPrecision with a requested_digits
            // parameter < kMinPrecisionDigits or requested_digits > kMaxPrecisionDigits
            // then the function returns false.
            static const int kMinPrecisionDigits = 1;
            static const int kMaxPrecisionDigits = 120;

            // The maximal number of digits that are needed to emit a double in base 10.
            // A higher precision can be achieved by using more digits, but the shortest
            // accurate representation of any double will never use more digits than
            // kBase10MaximalLength.
            // Note that DoubleToAscii null-terminates its input. So the given buffer
            // should be at least kBase10MaximalLength + 1 characters long.
            static const int kBase10MaximalLength = 17;

            enum DtoaMode
            {
                // Produce the shortest correct representation.
                // For example the output of 0.299999999999999988897 is (the less accurate
                // but correct) 0.3.
                Shortest,
                // Same as SHORTEST, but for single-precision floats.
                ShortestSingle,
                // Produce a fixed number of digits after the decimal point.
                // For instance fixed(0.1, 4) becomes 0.1000
                // If the input number is big, the output will be big.
                Fixed,
                // Fixed number of digits (independent of the decimal point).
                Precision
            };

            // Returns a converter following the EcmaScript specification.
            static const DoubleToStringConverter& EcmaScriptConverter()
            {
                static const T kInfinityString[] = { 'I', 'n', 'f', 'i', 'n', 'i', 't', 'y', '\0' };
                static const T kNanString[] = { 'N', 'a', 'N', '\0' };
                static DoubleToStringConverter s_stConverter(
                    static_cast<DtoaFlags>(
                        static_cast<int>(DtoaFlags::UniqueZero) |
                        static_cast<int>(DtoaFlags::EmitPositiveExponentSign)),
                    kInfinityString,
                    kNanString,
                    'e',
                    -6, 21,
                    6, 0);
                return s_stConverter;
            }

        public:
            // Flags should be a bit-or combination of the possible Flags-enum.
            //  - NO_FLAGS: no special flags.
            //  - EMIT_POSITIVE_EXPONENT_SIGN: when the number is converted into exponent
            //    form, emits a '+' for positive exponents. Example: 1.2e+2.
            //  - EMIT_TRAILING_DECIMAL_POINT: when the input number is an integer and is
            //    converted into decimal format then a trailing decimal point is appended.
            //    Example: 2345.0 is converted to "2345.".
            //  - EMIT_TRAILING_ZERO_AFTER_POINT: in addition to a trailing decimal point
            //    emits a trailing '0'-character. This flag requires the
            //    EXMIT_TRAILING_DECIMAL_POINT flag.
            //    Example: 2345.0 is converted to "2345.0".
            //  - UNIQUE_ZERO: "-0.0" is converted to "0.0".
            //
            // Infinity symbol and nan_symbol provide the string representation for these
            // special values. If the string is NULL and the special value is encountered
            // then the conversion functions return false.
            //
            // The exponent_character is used in exponential representations. It is
            // usually 'e' or 'E'.
            //
            // When converting to the shortest representation the converter will
            // represent input numbers in decimal format if they are in the interval
            // [10^decimal_in_shortest_low; 10^decimal_in_shortest_high[
            //    (lower boundary included, greater boundary excluded).
            // Example: with decimal_in_shortest_low = -6 and
            //               decimal_in_shortest_high = 21:
            //   ToShortest(0.000001)  -> "0.000001"
            //   ToShortest(0.0000001) -> "1e-7"
            //   ToShortest(111111111111111111111.0)  -> "111111111111111110000"
            //   ToShortest(100000000000000000000.0)  -> "100000000000000000000"
            //   ToShortest(1111111111111111111111.0) -> "1.1111111111111111e+21"
            //
            // When converting to precision mode the converter may add
            // max_leading_padding_zeroes before returning the number in exponential
            // format.
            // Example with max_leading_padding_zeroes_in_precision_mode = 6.
            //   ToPrecision(0.0000012345, 2) -> "0.0000012"
            //   ToPrecision(0.00000012345, 2) -> "1.2e-7"
            // Similarily the converter may add up to
            // max_trailing_padding_zeroes_in_precision_mode in precision mode to avoid
            // returning an exponential representation. A zero added by the
            // EMIT_TRAILING_ZERO_AFTER_POINT flag is counted for this limit.
            // Examples for max_trailing_padding_zeroes_in_precision_mode = 1:
            //   ToPrecision(230.0, 2) -> "230"
            //   ToPrecision(230.0, 2) -> "230."  with EMIT_TRAILING_DECIMAL_POINT.
            //   ToPrecision(230.0, 2) -> "2.3e2" with EMIT_TRAILING_ZERO_AFTER_POINT.
            DoubleToStringConverter(DtoaFlags flags, const T* infinitySymbol, const T* nanSymbol, T exponentCharacter,
                int decimalInShortestLow, int decimalInShortestHigh, size_t maxLeadingPaddingZeroesInPrecisionMode,
                size_t maxTrailingPaddingZeroesInPrecisionMode)
                : m_iFlags(static_cast<int>(flags)), m_szInfinitySymbol(infinitySymbol), m_szNanSymbol(nanSymbol),
                m_cExponentCharacter(exponentCharacter), m_iDecimalInShortestLow(decimalInShortestLow),
                m_iDecimalInShortestHigh(decimalInShortestHigh),
                m_iMaxLeadingPaddingZeroesInPrecisionMode(maxLeadingPaddingZeroesInPrecisionMode),
                m_iMaxTrailingPaddingZeroesInPrecisionMode(maxTrailingPaddingZeroesInPrecisionMode)
            {
                assert(((static_cast<int>(flags) & static_cast<int>(DtoaFlags::EmitTrailingDecimalPoint)) != 0) ||
                    !((static_cast<int>(flags) & static_cast<int>(DtoaFlags::EmitTrailingZeroAfterPoint)) != 0));
            }

            // Computes the shortest string of digits that correctly represent the input
            // number. Depending on decimal_in_shortest_low and decimal_in_shortest_high
            // (see constructor) it then either returns a decimal representation, or an
            // exponential representation.
            // Example with decimal_in_shortest_low = -6,
            //              decimal_in_shortest_high = 21,
            //              EMIT_POSITIVE_EXPONENT_SIGN activated, and
            //              EMIT_TRAILING_DECIMAL_POINT deactived:
            //   ToShortest(0.000001)  -> "0.000001"
            //   ToShortest(0.0000001) -> "1e-7"
            //   ToShortest(111111111111111111111.0)  -> "111111111111111110000"
            //   ToShortest(100000000000000000000.0)  -> "100000000000000000000"
            //   ToShortest(1111111111111111111111.0) -> "1.1111111111111111e+21"
            //
            // Note: the conversion may round the output if the returned string
            // is accurate enough to uniquely identify the input-number.
            // For example the most precise representation of the double 9e59 equals
            // "899999999999999918767229449717619953810131273674690656206848", but
            // the converter will return the shorter (but still correct) "9e59".
            //
            // Returns true if the conversion succeeds. The conversion always succeeds
            // except when the input value is special and no infinity_symbol or
            // nan_symbol has been given to the constructor.
            bool ToShortest(double value, StringBuilder<T>& resultBuilder)const
            {
                return ToShortestIeeeNumber(value, resultBuilder, DtoaMode::Shortest);
            }

            // Same as ToShortest, but for single-precision floats.
            bool ToShortestSingle(float value, StringBuilder<T>& resultBuilder)const
            {
                return ToShortestIeeeNumber(value, resultBuilder, DtoaMode::ShortestSingle);
            }

            // Computes a decimal representation with a fixed number of digits after the
            // decimal point. The last emitted digit is rounded.
            //
            // Examples:
            //   ToFixed(3.12, 1) -> "3.1"
            //   ToFixed(3.1415, 3) -> "3.142"
            //   ToFixed(1234.56789, 4) -> "1234.5679"
            //   ToFixed(1.23, 5) -> "1.23000"
            //   ToFixed(0.1, 4) -> "0.1000"
            //   ToFixed(1e30, 2) -> "1000000000000000019884624838656.00"
            //   ToFixed(0.1, 30) -> "0.100000000000000005551115123126"
            //   ToFixed(0.1, 17) -> "0.10000000000000001"
            //
            // If requested_digits equals 0, then the tail of the result depends on
            // the EMIT_TRAILING_DECIMAL_POINT and EMIT_TRAILING_ZERO_AFTER_POINT.
            // Examples, for requested_digits == 0,
            //   let EMIT_TRAILING_DECIMAL_POINT and EMIT_TRAILING_ZERO_AFTER_POINT be
            //    - false and false: then 123.45 -> 123
            //                             0.678 -> 1
            //    - true and false: then 123.45 -> 123.
            //                            0.678 -> 1.
            //    - true and true: then 123.45 -> 123.0
            //                           0.678 -> 1.0
            //
            // Returns true if the conversion succeeds. The conversion always succeeds
            // except for the following cases:
            //   - the input value is special and no infinity_symbol or nan_symbol has
            //     been provided to the constructor,
            //   - 'value' > 10^kMaxFixedDigitsBeforePoint, or
            //   - 'requested_digits' > kMaxFixedDigitsAfterPoint.
            // The last two conditions imply that the result will never contain more than
            // 1 + kMaxFixedDigitsBeforePoint + 1 + kMaxFixedDigitsAfterPoint characters
            // (one additional character for the sign, and one for the decimal point).
            bool ToFixed(double value, size_t requestedDigits, StringBuilder<T>& resultBuilder)const
            {
                assert(kMaxFixedDigitsBeforePoint == 60);
                const double kFirstNonFixed = 1e60;

                if (Double(value).IsSpecial())
                    return HandleSpecialValues(value, resultBuilder);

                if (requestedDigits > kMaxFixedDigitsAfterPoint)
                    return false;
                if (value >= kFirstNonFixed || value <= -kFirstNonFixed)
                    return false;

                // Find a sufficiently precise decimal representation of n.
                int decimalPoint;
                bool sign;
                // Add space for the '\0' byte.
                const size_t kDecimalRepCapacity = kMaxFixedDigitsBeforePoint + kMaxFixedDigitsAfterPoint + 1;
                T decimalRep[kDecimalRepCapacity];
                size_t decimalRepLength;
                DoubleToAscii(value, DtoaMode::Fixed, requestedDigits, decimalRep, kDecimalRepCapacity, sign,
                    decimalRepLength, decimalPoint);

                bool uniqueZero = ((m_iFlags & static_cast<int>(DtoaFlags::UniqueZero)) != 0);
                if (sign && (value != 0.0 || !uniqueZero))
                    resultBuilder.AddCharacter('-');

                CreateDecimalRepresentation(decimalRep, decimalRepLength, decimalPoint, requestedDigits, resultBuilder);
                return true;
            }

            // Computes a representation in exponential format with requested_digits
            // after the decimal point. The last emitted digit is rounded.
            // If requested_digits equals -1, then the shortest exponential representation
            // is computed.
            //
            // Examples with EMIT_POSITIVE_EXPONENT_SIGN deactivated, and
            //               exponent_character set to 'e'.
            //   ToExponential(3.12, 1) -> "3.1e0"
            //   ToExponential(5.0, 3) -> "5.000e0"
            //   ToExponential(0.001, 2) -> "1.00e-3"
            //   ToExponential(3.1415, -1) -> "3.1415e0"
            //   ToExponential(3.1415, 4) -> "3.1415e0"
            //   ToExponential(3.1415, 3) -> "3.142e0"
            //   ToExponential(123456789000000, 3) -> "1.235e14"
            //   ToExponential(1000000000000000019884624838656.0, -1) -> "1e30"
            //   ToExponential(1000000000000000019884624838656.0, 32) ->
            //                     "1.00000000000000001988462483865600e30"
            //   ToExponential(1234, 0) -> "1e3"
            //
            // Returns true if the conversion succeeds. The conversion always succeeds
            // except for the following cases:
            //   - the input value is special and no infinity_symbol or nan_symbol has
            //     been provided to the constructor,
            //   - 'requested_digits' > kMaxExponentialDigits.
            // The last condition implies that the result will never contain more than
            // kMaxExponentialDigits + 8 characters (the sign, the digit before the
            // decimal point, the decimal point, the exponent character, the
            // exponent's sign, and at most 3 exponent digits).
            bool ToExponential(double value, int requestedDigits, StringBuilder<T>& resultBuilder)const
            {
                if (Double(value).IsSpecial())
                    return HandleSpecialValues(value, resultBuilder);

                if (requestedDigits < -1)
                    return false;
                if (requestedDigits > kMaxExponentialDigits)
                    return false;

                int decimalPoint;
                bool sign;
                // Add space for digit before the decimal point and the '\0' character.
                const size_t kDecimalRepCapacity = kMaxExponentialDigits + 2;
                assert(kDecimalRepCapacity > kBase10MaximalLength);
                T decimalRep[kDecimalRepCapacity];
                size_t decimalRepLength;

                if (requestedDigits == -1)
                {
                    DoubleToAscii(value, DtoaMode::Shortest, 0, decimalRep, kDecimalRepCapacity, sign, decimalRepLength,
                        decimalPoint);
                }
                else
                {
                    DoubleToAscii(value, DtoaMode::Precision, static_cast<size_t>(requestedDigits + 1), decimalRep,
                        kDecimalRepCapacity, sign, decimalRepLength, decimalPoint);
                    assert(decimalRepLength <= static_cast<size_t>(requestedDigits + 1));

                    for (int i = decimalRepLength; i < requestedDigits + 1; ++i)
                        decimalRep[i] = '0';

                    decimalRepLength = static_cast<size_t>(requestedDigits + 1);
                }

                bool uniqueZero = ((m_iFlags & static_cast<int>(DtoaFlags::UniqueZero)) != 0);
                if (sign && (value != 0.0 || !uniqueZero))
                    resultBuilder.AddCharacter('-');

                int exponent = decimalPoint - 1;
                CreateExponentialRepresentation(decimalRep, decimalRepLength, exponent, resultBuilder);
                return true;
            }

            // Computes 'precision' leading digits of the given 'value' and returns them
            // either in exponential or decimal format, depending on
            // max_{leading|trailing}_padding_zeroes_in_precision_mode (given to the
            // constructor).
            // The last computed digit is rounded.
            //
            // Example with max_leading_padding_zeroes_in_precision_mode = 6.
            //   ToPrecision(0.0000012345, 2) -> "0.0000012"
            //   ToPrecision(0.00000012345, 2) -> "1.2e-7"
            // Similarily the converter may add up to
            // max_trailing_padding_zeroes_in_precision_mode in precision mode to avoid
            // returning an exponential representation. A zero added by the
            // EMIT_TRAILING_ZERO_AFTER_POINT flag is counted for this limit.
            // Examples for max_trailing_padding_zeroes_in_precision_mode = 1:
            //   ToPrecision(230.0, 2) -> "230"
            //   ToPrecision(230.0, 2) -> "230."  with EMIT_TRAILING_DECIMAL_POINT.
            //   ToPrecision(230.0, 2) -> "2.3e2" with EMIT_TRAILING_ZERO_AFTER_POINT.
            // Examples for max_trailing_padding_zeroes_in_precision_mode = 3, and no
            //    EMIT_TRAILING_ZERO_AFTER_POINT:
            //   ToPrecision(123450.0, 6) -> "123450"
            //   ToPrecision(123450.0, 5) -> "123450"
            //   ToPrecision(123450.0, 4) -> "123500"
            //   ToPrecision(123450.0, 3) -> "123000"
            //   ToPrecision(123450.0, 2) -> "1.2e5"
            //
            // Returns true if the conversion succeeds. The conversion always succeeds
            // except for the following cases:
            //   - the input value is special and no infinity_symbol or nan_symbol has
            //     been provided to the constructor,
            //   - precision < kMinPericisionDigits
            //   - precision > kMaxPrecisionDigits
            // The last condition implies that the result will never contain more than
            // kMaxPrecisionDigits + 7 characters (the sign, the decimal point, the
            // exponent character, the exponent's sign, and at most 3 exponent digits).
            bool ToPrecision(double value, size_t precision, StringBuilder<T>& resultBuilder)const
            {
                if (Double(value).IsSpecial())
                    return HandleSpecialValues(value, resultBuilder);

                if (precision < kMinPrecisionDigits || precision > kMaxPrecisionDigits)
                    return false;

                // Find a sufficiently precise decimal representation of n.
                int decimalPoint;
                bool sign;
                // Add one for the terminating null character.
                const size_t kDecimalRepCapacity = kMaxPrecisionDigits + 1;
                T decimalRep[kDecimalRepCapacity];
                size_t decimalRepLength;

                DoubleToAscii(value, DtoaMode::Precision, precision, decimalRep, kDecimalRepCapacity, sign,
                    decimalRepLength, decimalPoint);
                assert(decimalRepLength <= precision);

                bool uniqueZero = ((m_iFlags & static_cast<int>(DtoaFlags::UniqueZero)) != 0);
                if (sign && (value != 0.0 || !uniqueZero))
                    resultBuilder.AddCharacter('-');

                // The exponent if we print the number as x.xxeyyy. That is with the
                // decimal point after the first digit.
                int exponent = decimalPoint - 1;

                int extraZero = ((m_iFlags & static_cast<int>(DtoaFlags::EmitTrailingZeroAfterPoint)) != 0) ? 1 : 0;
                if ((-decimalPoint + 1 > m_iMaxLeadingPaddingZeroesInPrecisionMode) ||
                    (decimalPoint - static_cast<int>(precision) + extraZero >
                        m_iMaxTrailingPaddingZeroesInPrecisionMode))
                {
                    // Fill buffer to contain 'precision' digits.
                    // Usually the buffer is already at the correct length, but 'DoubleToAscii'
                    // is allowed to return less characters.
                    for (size_t i = decimalRepLength; i < precision; ++i)
                        decimalRep[i] = '0';

                    CreateExponentialRepresentation(decimalRep, precision, exponent, resultBuilder);
                }
                else
                {
                    CreateDecimalRepresentation(decimalRep, decimalRepLength, decimalPoint,
                        static_cast<int>(precision) > decimalPoint ? precision - decimalPoint : 0u, resultBuilder);
                }

                return true;
            }

            // Converts the given double 'v' to ascii. 'v' must not be NaN, +Infinity, or
            // -Infinity. In SHORTEST_SINGLE-mode this restriction also applies to 'v'
            // after it has been casted to a single-precision float. That is, in this
            // mode static_cast<float>(v) must not be NaN, +Infinity or -Infinity.
            //
            // The result should be interpreted as buffer * 10^(point-length).
            //
            // The output depends on the given mode:
            //  - SHORTEST: produce the least amount of digits for which the internal
            //   identity requirement is still satisfied. If the digits are printed
            //   (together with the correct exponent) then reading this number will give
            //   'v' again. The buffer will choose the representation that is closest to
            //   'v'. If there are two at the same distance, than the one farther away
            //   from 0 is chosen (halfway cases - ending with 5 - are rounded up).
            //   In this mode the 'requested_digits' parameter is ignored.
            //  - SHORTEST_SINGLE: same as SHORTEST but with single-precision.
            //  - FIXED: produces digits necessary to print a given number with
            //   'requested_digits' digits after the decimal point. The produced digits
            //   might be too short in which case the caller has to fill the remainder
            //   with '0's.
            //   Example: toFixed(0.001, 5) is allowed to return buffer="1", point=-2.
            //   Halfway cases are rounded towards +/-Infinity (away from 0). The call
            //   toFixed(0.15, 2) thus returns buffer="2", point=0.
            //   The returned buffer may contain digits that would be truncated from the
            //   shortest representation of the input.
            //  - PRECISION: produces 'requested_digits' where the first digit is not '0'.
            //   Even though the length of produced digits usually equals
            //   'requested_digits', the function is allowed to return fewer digits, in
            //   which case the caller has to fill the missing digits with '0's.
            //   Halfway cases are again rounded away from 0.
            // DoubleToAscii expects the given buffer to be big enough to hold all
            // digits and a terminating null-character. In SHORTEST-mode it expects a
            // buffer of at least kBase10MaximalLength + 1. In all other modes the
            // requested_digits parameter and the padding-zeroes limit the size of the
            // output. Don't forget the decimal point, the exponent character and the
            // terminating null-character when computing the maximal output size.
            // The given length is only used in debug mode to ensure the buffer is big
            // enough.
            static void DoubleToAscii(double v, DtoaMode mode, size_t requestedDigits, T* buffer, size_t bufferLength,
                bool& sign, size_t& length, int& point)
            {
                MutableArrayView<T> vector(buffer, bufferLength);
                assert(!Double(v).IsSpecial());
                assert(mode == DtoaMode::Shortest || mode == DtoaMode::ShortestSingle || requestedDigits >= 0);

                if (Double(v).Sign() < 0)
                {
                    sign = true;
                    v = -v;
                }
                else
                    sign = false;

                if (mode == DtoaMode::Precision && requestedDigits == 0)
                {
                    vector[0] = '\0';
                    length = 0;
                    return;
                }

                if (v == 0)
                {
                    vector[0] = '0';
                    vector[1] = '\0';
                    length = 1;
                    point = 1;
                    return;
                }

                bool fastWorked;
                switch (mode)
                {
                    case DtoaMode::Shortest:
                        fastWorked = FastDtoa::Dtoa(v, FastDtoaMode::Shortest, 0, vector, length, point);
                        break;
                    case DtoaMode::ShortestSingle:
                        fastWorked = FastDtoa::Dtoa(v, FastDtoaMode::ShortestSingle, 0, vector, length, point);
                        break;
                    case DtoaMode::Fixed:
                        fastWorked = FixedDtoa::Dtoa(v, requestedDigits, vector, length, point);
                        break;
                    case DtoaMode::Precision:
                        fastWorked = FastDtoa::Dtoa(v, FastDtoaMode::Precision, requestedDigits, vector, length, point);
                        break;
                    default:
                        fastWorked = false;
                        MOE_UNREACHABLE();
                        break;
                }
                if (fastWorked)
                    return;

                // If the fast dtoa didn't succeed use the slower bignum version.
                BignumDtoaMode bignumMode;
                switch (mode)
                {
                    case DtoaMode::Shortest:
                        bignumMode = BignumDtoaMode::Shortest;
                        break;
                    case DtoaMode::ShortestSingle:
                        bignumMode = BignumDtoaMode::ShortestSingle;
                        break;
                    case DtoaMode::Fixed:
                        bignumMode = BignumDtoaMode::Fixed;
                        break;
                    case DtoaMode::Precision:
                        bignumMode = BignumDtoaMode::Precision;
                        break;
                    default:
                        MOE_UNREACHABLE();
                        break;
                }

                BignumDtoa::Dtoa(v, bignumMode, requestedDigits, vector, length, point);
                vector[length] = '\0';
            }

        private:
            // Implementation for ToShortest and ToShortestSingle.
            bool ToShortestIeeeNumber(double value, StringBuilder<T>& resultBuilder, DtoaMode mode)const
            {
                assert(mode == DtoaMode::Shortest || mode == DtoaMode::ShortestSingle);
                if (Double(value).IsSpecial())
                    return HandleSpecialValues(value, resultBuilder);

                int decimalPoint;
                bool sign;
                const size_t kDecimalRepCapacity = kBase10MaximalLength + 1;
                T decimalRep[kDecimalRepCapacity];
                size_t decimalRepLength;

                DoubleToAscii(value, mode, 0, decimalRep, kDecimalRepCapacity, sign, decimalRepLength, decimalPoint);

                bool uniqueZero = (m_iFlags & static_cast<int>(DtoaFlags::UniqueZero)) != 0;
                if (sign && (value != 0.0 || !uniqueZero))
                    resultBuilder.AddCharacter('-');

                int exponent = decimalPoint - 1;
                if ((m_iDecimalInShortestLow <= exponent) && (exponent < m_iDecimalInShortestHigh))
                {
                    CreateDecimalRepresentation(decimalRep, decimalRepLength, decimalPoint,
                        static_cast<int>(decimalRepLength) > decimalPoint ?
                            decimalRepLength - decimalPoint : 0u, resultBuilder);
                }
                else
                    CreateExponentialRepresentation(decimalRep, decimalRepLength, exponent, resultBuilder);

                return true;
            }

            // If the value is a special value (NaN or Infinity) constructs the
            // corresponding string using the configured infinity/nan-symbol.
            // If either of them is NULL or the value is not special then the
            // function returns false.
            bool HandleSpecialValues(double value, StringBuilder<T>& resultBuilder)const
            {
                Double doubleInspect(value);
                if (doubleInspect.IsInfinite())
                {
                    if (m_szInfinitySymbol == nullptr)
                        return false;
                    if (value < 0)
                        resultBuilder.AddCharacter('-');

                    resultBuilder.AddString(m_szInfinitySymbol);
                    return true;
                }

                if (doubleInspect.IsNan())
                {
                    if (m_szNanSymbol == nullptr)
                        return false;

                    resultBuilder.AddString(m_szNanSymbol);
                    return true;
                }

                return false;
            }

            // Constructs an exponential representation (i.e. 1.234e56).
            // The given exponent assumes a decimal point after the first decimal digit.
            void CreateExponentialRepresentation(const T* decimalDigits, size_t length, int exponent,
                StringBuilder<T>& resultBuilder)const
            {
                assert(length != 0);
                resultBuilder.AddCharacter(decimalDigits[0]);
                if (length != 1)
                {
                    resultBuilder.AddCharacter('.');
                    resultBuilder.AddSubstring(&decimalDigits[1], length - 1);
                }

                resultBuilder.AddCharacter(m_cExponentCharacter);

                if (exponent < 0)
                {
                    resultBuilder.AddCharacter('-');
                    exponent = -exponent;
                }
                else
                {
                    if ((m_iFlags & static_cast<int>(DtoaFlags::EmitPositiveExponentSign)) != 0)
                        resultBuilder.AddCharacter('+');
                }

                if (exponent == 0)
                {
                    resultBuilder.AddCharacter('0');
                    return;
                }

                assert(exponent < 1e4);
                const size_t kMaxExponentLength = 5;
                T buffer[kMaxExponentLength + 1];
                buffer[kMaxExponentLength] = '\0';
                size_t firstCharPos = kMaxExponentLength;
                while (exponent > 0)
                {
                    buffer[--firstCharPos] = static_cast<T>('0' + (exponent % 10));
                    exponent /= 10;
                }

                resultBuilder.AddSubstring(&buffer[firstCharPos], kMaxExponentLength - firstCharPos);
            }

            // Creates a decimal representation (i.e 1234.5678).
            void CreateDecimalRepresentation(const T* decimalDigits, size_t length, int decimalPoint,
                size_t digitsAfterPoint, StringBuilder<T>& resultBuilder)const
            {
                // Create a representation that is padded with zeros if needed.
                if (decimalPoint <= 0)
                {
                    // "0.00000decimal_rep" or "0.000decimal_rep00".
                    resultBuilder.AddCharacter('0');
                    if (digitsAfterPoint > 0)
                    {
                        resultBuilder.AddCharacter('.');
                        assert(-decimalPoint >= 0);
                        resultBuilder.AddPadding('0', static_cast<size_t>(-decimalPoint));
                        assert(length <= static_cast<size_t>(digitsAfterPoint - (-decimalPoint)));
                        resultBuilder.AddSubstring(decimalDigits, length);
                        int remainingDigits = digitsAfterPoint - (-decimalPoint) - length;
                        resultBuilder.AddPadding('0', static_cast<size_t>(remainingDigits));
                    }
                }
                else if (decimalPoint >= 0 && static_cast<size_t>(decimalPoint) >= length)
                {
                    // "decimal_rep0000.00000" or "decimal_rep.0000".
                    resultBuilder.AddSubstring(decimalDigits, length);
                    resultBuilder.AddPadding('0', decimalPoint - length);
                    if (digitsAfterPoint > 0)
                    {
                        resultBuilder.AddCharacter('.');
                        resultBuilder.AddPadding('0', static_cast<size_t>(digitsAfterPoint));
                    }
                }
                else
                {
                    // "decima.l_rep000".
                    assert(digitsAfterPoint > 0);
                    resultBuilder.AddSubstring(decimalDigits, static_cast<size_t>(decimalPoint));
                    resultBuilder.AddCharacter('.');
                    assert(length - decimalPoint <= digitsAfterPoint);
                    resultBuilder.AddSubstring(&decimalDigits[decimalPoint],
                        length - decimalPoint);
                    int remainingDigits = digitsAfterPoint - (length - decimalPoint);
                    resultBuilder.AddPadding('0', static_cast<size_t>(remainingDigits));
                }

                if (digitsAfterPoint == 0)
                {
                    if ((m_iFlags & static_cast<int>(DtoaFlags::EmitTrailingDecimalPoint)) != 0)
                        resultBuilder.AddCharacter('.');
                    if ((m_iFlags & static_cast<int>(DtoaFlags::EmitTrailingZeroAfterPoint)) != 0)
                        resultBuilder.AddCharacter('0');
                }
            }

        private:
            const int m_iFlags;
            const T* const m_szInfinitySymbol;
            const T* const m_szNanSymbol;
            const T m_cExponentCharacter;
            const int m_iDecimalInShortestLow;
            const int m_iDecimalInShortestHigh;
            const int m_iMaxLeadingPaddingZeroesInPrecisionMode;
            const int m_iMaxTrailingPaddingZeroesInPrecisionMode;
        };
    }

    /**
     * @brief 以最小表示转换单精度浮点到字符串
     * @tparam T 目标字符串类型
     * @tparam Size 缓冲区大小
     * @pre length足够大以容纳所有结果
     * @param d 被转换浮点
     * @param buffer 目标缓冲区
     * @return 转换是否成功
     *
     * 转换单精度浮点到最小字符串表示。
     * 缓冲区需要足够大以容纳结果，否则可能在运行时导致崩溃。
     */
    template <typename T = char, size_t Size>
    inline bool DoubleToShortestString(float d, T (&buffer)[Size])
    {
        internal::StringBuilder<T> builder(buffer, Size);
        return internal::DoubleToStringConverter<T>::EcmaScriptConverter().ToShortestSingle(d, builder);
    }

    /**
     * @brief 以最小表示转换单精度浮点到字符串
     * @tparam T 目标字符串类型
     * @pre length足够大以容纳所有结果
     * @param d 被转换浮点
     * @param buffer 目标缓冲区
     * @param length 缓冲区大小
     * @return 转换是否成功
     *
     * 转换单精度浮点到最小字符串表示。
     * 缓冲区需要足够大以容纳结果，否则可能在运行时导致崩溃。
     */
    template <typename T = char>
    inline bool DoubleToShortestString(float d, T* buffer, size_t length)
    {
        internal::StringBuilder<T> builder(buffer, length);
        return internal::DoubleToStringConverter<T>::EcmaScriptConverter().ToShortestSingle(d, builder);
    }

    /**
     * @brief 以最小表示转换双精度浮点到字符串
     * @tparam T 目标字符串类型
     * @tparam Size 缓冲区大小
     * @pre length足够大以容纳所有结果
     * @param d 被转换浮点
     * @param buffer 目标缓冲区
     * @return 转换是否成功
     *
     * 转换双精度浮点到最小字符串表示。
     * 缓冲区需要足够大以容纳结果，否则可能在运行时导致崩溃。
     */
    template <typename T = char, size_t Size>
    inline bool DoubleToShortestString(double d, T (&buffer)[Size])
    {
        internal::StringBuilder<T> builder(buffer, Size);
        return internal::DoubleToStringConverter<T>::EcmaScriptConverter().ToShortest(d, builder);
    }

    /**
     * @brief 以最小表示转换双精度浮点到字符串
     * @tparam T 目标字符串类型
     * @pre length足够大以容纳所有结果
     * @param d 被转换浮点
     * @param buffer 目标缓冲区
     * @param length 缓冲区大小
     * @return 转换是否成功
     *
     * 转换双精度浮点到最小字符串表示。
     * 缓冲区需要足够大以容纳结果，否则可能在运行时导致崩溃。
     */
    template <typename T = char>
    inline bool DoubleToShortestString(double d, T* buffer, size_t length)
    {
        internal::StringBuilder<T> builder(buffer, length);
        return internal::DoubleToStringConverter<T>::EcmaScriptConverter().ToShortest(d, builder);
    }

    /**
     * @brief 以定点数表示转换双精度浮点到字符串
     * @tparam T 目标字符串类型
     * @tparam Size 缓冲区大小
     * @pre length足够大以容纳所有结果且 requestDigits <= 20
     * @param d 被转换浮点数
     * @param requestDigits 需要的小数位数
     * @param buffer 目标缓冲区
     * @return 转换是否成功
     *
     * 转换双精度浮点到字符串并四舍五入指定的小数位数。
     * 缓冲区需要足够大以容纳结果，否则可能在运行时导致崩溃。
     */
    template <typename T = char, size_t Size>
    inline bool DoubleToFixedString(double d, size_t requestDigits, T (&buffer)[Size])
    {
        assert(requestDigits <= 20);
        requestDigits = std::min(requestDigits, 20u);

        internal::StringBuilder<T> builder(buffer, Size);
        return internal::DoubleToStringConverter<T>::EcmaScriptConverter().ToFixed(d, requestDigits, builder);
    }

    /**
     * @brief 以定点数表示转换双精度浮点到字符串
     * @tparam T 目标字符串类型
     * @pre length足够大以容纳所有结果且 requestDigits <= 20
     * @param d 被转换浮点数
     * @param requestDigits 需要的小数位数
     * @param buffer 目标缓冲区
     * @param length 缓冲区大小
     * @return 转换是否成功
     *
     * 转换双精度浮点到字符串并四舍五入指定的小数位数。
     * 缓冲区需要足够大以容纳结果，否则可能在运行时导致崩溃。
     */
    template <typename T = char>
    inline bool DoubleToFixedString(double d, size_t requestDigits, T* buffer, size_t length)
    {
        assert(requestDigits <= 20);
        requestDigits = std::min(requestDigits, 20u);

        internal::StringBuilder<T> builder(buffer, length);
        return internal::DoubleToStringConverter<T>::EcmaScriptConverter().ToFixed(d, requestDigits, builder);
    }

    /**
     * @brief 以有效数字表示转换双精度浮点到字符串
     * @tparam T 目标字符串类型
     * @tparam Size 缓冲区大小
     * @pre length足够大以容纳所有结果且 1 <= precision && precision <= 21
     * @param d 被转换浮点数
     * @param precision 精度
     * @param buffer 目标缓冲区
     * @return 转换是否成功
     *
     * 转换双精度浮点且四舍五入到保留指定的有效数字。
     * 缓冲区需要足够大以容纳结果，否则可能在运行时导致崩溃。
     */
    template <typename T = char, size_t Size>
    inline bool DoubleToPrecisionString(double d, size_t precision, T (&buffer)[Size])
    {
        assert(1 <= precision && precision <= 21);
        precision = std::max(std::min(precision, 21u), 1u);

        internal::StringBuilder<T> builder(buffer, Size);
        return internal::DoubleToStringConverter<T>::EcmaScriptConverter().ToPrecision(d, precision, builder);
    }

    /**
     * @brief 以有效数字表示转换双精度浮点到字符串
     * @tparam T 目标字符串类型
     * @pre length足够大以容纳所有结果且 1 <= precision && precision <= 21
     * @param d 被转换浮点数
     * @param precision 精度
     * @param buffer 目标缓冲区
     * @param length 缓冲区大小
     * @return 转换是否成功
     *
     * 转换双精度浮点且四舍五入到保留指定的有效数字。
     * 缓冲区需要足够大以容纳结果，否则可能在运行时导致崩溃。
     */
    template <typename T = char>
    inline bool DoubleToPrecisionString(double d, size_t precision, T* buffer, size_t length)
    {
        assert(1 <= precision && precision <= 21);
        precision = std::max(std::min(precision, 21u), 1u);

        internal::StringBuilder<T> builder(buffer, length);
        return internal::DoubleToStringConverter<T>::EcmaScriptConverter().ToPrecision(d, precision, builder);
    }

    /**
     * @brief 使用科学计数法表示转换双精度浮点到字符串
     * @tparam T 目标字符串类型
     * @tparam Size 缓冲区大小
     * @pre length足够大以容纳所有结果且 0 <= requestedDigits && requestedDigits <= 20
     * @param d 被转换浮点数
     * @param requestedDigits 小数点后保留的有效位数
     * @param buffer 目标缓冲区
     * @return 转换是否成功
     *
     * 以科学计数法表示转换双精度浮点。
     * 缓冲区需要足够大以容纳结果，否则可能在运行时导致崩溃。
     */
    template <typename T = char, size_t Size>
    inline bool DoubleToExponentialString(double d, size_t requestedDigits, T (&buffer)[Size])
    {
        assert(0 <= requestedDigits && requestedDigits <= 20);
        requestedDigits = std::min(requestedDigits, 20u);

        internal::StringBuilder<T> builder(buffer, Size);
        return internal::DoubleToStringConverter<T>::EcmaScriptConverter().ToExponential(d,
            static_cast<int>(requestedDigits), builder);
    }

    /**
     * @brief 使用科学计数法表示转换双精度浮点到字符串
     * @tparam T 目标字符串类型
     * @pre length足够大以容纳所有结果且 0 <= requestedDigits && requestedDigits <= 20
     * @param d 被转换浮点数
     * @param requestedDigits 小数点后保留的有效位数
     * @param buffer 目标缓冲区
     * @param length 缓冲区大小
     * @return 转换是否成功
     *
     * 以科学计数法表示转换双精度浮点。
     * 缓冲区需要足够大以容纳结果，否则可能在运行时导致崩溃。
     */
    template <typename T = char>
    inline bool DoubleToExponentialString(double d, size_t requestedDigits, T* buffer, size_t length)
    {
        assert(0 <= requestedDigits && requestedDigits <= 20);
        requestedDigits = std::min(requestedDigits, 20u);

        internal::StringBuilder<T> builder(buffer, length);
        return internal::DoubleToStringConverter<T>::EcmaScriptConverter().ToExponential(d,
            static_cast<int>(requestedDigits), builder);
    }

    /**
     * @brief 使用科学计数法表示转换双精度浮点到字符串
     * @tparam T 目标字符串类型
     * @tparam Size 缓冲区大小
     * @pre length足够大以容纳所有结果且 0 <= requestedDigits && requestedDigits <= 20
     * @param d 被转换浮点数
     * @param buffer 目标缓冲区
     * @return 转换是否成功
     *
     * 以科学计数法表示转换双精度浮点。本方法尽可能保留足够的小数位数。
     * 缓冲区需要足够大以容纳结果，否则可能在运行时导致崩溃。
     */
    template <typename T = char, size_t Size>
    inline bool DoubleToExponentialString(double d, T (&buffer)[Size])
    {
        internal::StringBuilder<T> builder(buffer, Size);
        return internal::DoubleToStringConverter<T>::EcmaScriptConverter().ToExponential(d, -1, builder);
    }

    /**
     * @brief 使用科学计数法表示转换双精度浮点到字符串
     * @tparam T 目标字符串类型
     * @pre length足够大以容纳所有结果且 0 <= requestedDigits && requestedDigits <= 20
     * @param d 被转换浮点数
     * @param buffer 目标缓冲区
     * @param length 缓冲区大小
     * @return 转换是否成功
     *
     * 以科学计数法表示转换双精度浮点。本方法尽可能保留足够的小数位数。
     * 缓冲区需要足够大以容纳结果，否则可能在运行时导致崩溃。
     */
    template <typename T = char>
    inline bool DoubleToExponentialString(double d, T* buffer, size_t length)
    {
        internal::StringBuilder<T> builder(buffer, length);
        return internal::DoubleToStringConverter<T>::EcmaScriptConverter().ToExponential(d, -1, builder);
    }
}
