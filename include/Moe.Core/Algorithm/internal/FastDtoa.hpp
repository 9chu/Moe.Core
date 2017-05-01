/**
 * @file
 * @date 2017/5/1
 * @see https://github.com/google/double-conversion
 */
#pragma once
#include "DiyFp.hpp"

namespace moe
{
    namespace internal
    {
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
            static bool RoundWeed(Buffer<T>& buffer, size_t length, uint64_t distanceTooHighW, uint64_t unsafeInterval,
                uint64_t rest, uint64_t tenKappa, uint64_t unit)
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
            static bool RoundWeedCounted(Buffer<T>& buffer, size_t length, uint64_t rest, uint64_t tenKappa,
                uint64_t unit, int* kappa)
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
                        (*kappa) += 1;
                    }

                    return true;
                }

                return false;
            }

            static void BiggestPowerTen(uint32_t number, int numberBits, uint32_t* power, int* exponentPlusOne);

            template <typename T>
            static bool DigitGen(DiyFp low, DiyFp w, DiyFp high, Buffer<T>& buffer, size_t* length, int* kappa)
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
                BiggestPowerTen(integrals, DiyFp::kSignificandSize - (-one.Exponent()), &divisor,
                    &divisorExponentPlusOne);
                *kappa = divisorExponentPlusOne;
                *length = 0;

                while (*kappa > 0)
                {
                    int digit = integrals / divisor;
                    assert(digit <= 9);
                    buffer[*length] = static_cast<char>('0' + digit);
                    ++(*length);
                    integrals %= divisor;
                    --(*kappa);
                    uint64_t rest = (static_cast<uint64_t>(integrals) << -one.Exponent()) + fractionals;
                    if (rest < unsafeInterval.Significand())
                    {
                        return RoundWeed(buffer, *length, DiyFp::Minus(tooHigh, w).Significand(),
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
                    buffer[*length] = static_cast<char>('0' + digit);
                    (*length)++;
                    fractionals &= one.Significand() - 1;
                    (*kappa)--;
                    if (fractionals < unsafeInterval.Significand())
                    {
                        return RoundWeed(buffer, *length, DiyFp::Minus(tooHigh, w).Significand() * unit,
                            unsafeInterval.Significand(), fractionals, one.Significand(), unit);
                    }
                }
            }

            template <typename T>
            static bool DigitGenCounted(DiyFp w, int requestedDigits, Buffer<T>& buffer, size_t* length, int* kappa)
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
                BiggestPowerTen(integrals, DiyFp::kSignificandSize - (-one.Exponent()), &divisor,
                    &divisorExponentPlusOne);
                *kappa = divisorExponentPlusOne;
                *length = 0;

                while (*kappa > 0)
                {
                    int digit = integrals / divisor;
                    assert(digit <= 9);
                    buffer[*length] = static_cast<char>('0' + digit);
                    ++(*length);
                    requestedDigits--;
                    integrals %= divisor;
                    --(*kappa);
                    if (requestedDigits == 0)
                        break;
                    divisor /= 10;
                }

                if (requestedDigits == 0)
                {
                    uint64_t rest = (static_cast<uint64_t>(integrals) << -one.Exponent()) + fractionals;
                    return RoundWeedCounted(buffer, *length, rest, static_cast<uint64_t>(divisor) << -one.Exponent(),
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
                    buffer[*length] = static_cast<char>('0' + digit);
                    (*length)++;
                    requestedDigits--;
                    fractionals &= one.Significand() - 1;
                    (*kappa)--;
                }

                if (requestedDigits != 0)
                    return false;

                return RoundWeedCounted(buffer, *length, fractionals, one.Significand(), wError, kappa);
            }

            template <typename T>
            static bool Grisu3(double v, FastDtoaMode mode, Buffer<T>& buffer, size_t* length, int* decimalExponent)
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
                    tenMkMaximalBinaryExponent, &tenMk, &mk);
                assert((kMinimalTargetExponent <= w.Exponent() + tenMk.Exponent() + DiyFp::kSignificandSize) &&
                    (kMaximalTargetExponent >= w.Exponent() + tenMk.Exponent() + DiyFp::kSignificandSize));

                DiyFp scaledW = DiyFp::Times(w, tenMk);
                assert(scaledW.Exponent() == boundaryPlus.Exponent() + tenMk.Exponent() + DiyFp::kSignificandSize);

                DiyFp scaledBoundaryMinus = DiyFp::Times(boundaryMinus, tenMk);
                DiyFp scaledBoundaryPlus  = DiyFp::Times(boundaryPlus, tenMk);

                int kappa = 0;
                bool result = DigitGen(scaledBoundaryMinus, scaledW, scaledBoundaryPlus, buffer, length, &kappa);
                *decimalExponent = -mk + kappa;
                return result;
            }

            template <typename T>
            static bool Grisu3Counted(double v, int requestedDigits, Buffer<T>& buffer, size_t* length,
                int* decimalExponent)
            {
                DiyFp w = Double(v).ToNormalizedDiyFp();
                DiyFp tenMk;
                int mk = 0;
                int tenMkMinimalBinaryExponent = kMinimalTargetExponent - (w.Exponent() + DiyFp::kSignificandSize);
                int tenMkMaximalBinaryExponent = kMaximalTargetExponent - (w.Exponent() + DiyFp::kSignificandSize);
                PowersOfTenCache::GetCachedPowerForBinaryExponentRange(tenMkMinimalBinaryExponent,
                    tenMkMaximalBinaryExponent, &tenMk, &mk);
                assert((kMinimalTargetExponent <= w.Exponent() + tenMk.Exponent() + DiyFp::kSignificandSize) &&
                    (kMaximalTargetExponent >= w.Exponent() + tenMk.Exponent() + DiyFp::kSignificandSize));

                DiyFp scaledW = DiyFp::Times(w, tenMk);

                int kappa = 0;
                bool result = DigitGenCounted(scaledW, requestedDigits, buffer, length, &kappa);
                *decimalExponent = -mk + kappa;
                return result;
            }

            template <typename T>
            static bool FastDtoa(double v, FastDtoaMode mode, int requestedDigits, Buffer<T> buffer, size_t* length,
                int* decimalPoint)
            {
                assert(v > 0);
                assert(!Double(v).IsSpecial());

                bool result = false;
                int decimalExponent = 0;
                switch (mode)
                {
                    case FastDtoaMode::Shortest:
                    case FastDtoaMode::ShortestSingle:
                        result = Grisu3(v, mode, buffer, length, &decimalExponent);
                        break;
                    case FastDtoaMode::Precision:
                        result = Grisu3Counted(v, requestedDigits, buffer, length, &decimalExponent);
                        break;
                    default:
                        MOE_UNREACHABLE();
                }

                if (result)
                {
                    *decimalPoint = *length + decimalExponent;
                    buffer[*length] = '\0';
                }

                return result;
            }
        };
    }
}
