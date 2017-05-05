/**
 * @file
 * @date 2017/5/5
 */
#pragma once
#include "DiyFp.hpp"
#include "Bignum.hpp"

namespace moe
{
    namespace internal
    {
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
    }
}
