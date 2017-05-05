/**
 * @file
 * @date 2017/5/5
 */
#include <Moe.Core/Algorithm/internal/BignumDtoa.hpp>

#include <cmath>

using namespace moe;
using namespace internal;

static void InitialScaledStartValuesPositiveExponent(uint64_t significand, int exponent, int estimatedPower,
    bool needBoundaryDeltas, Bignum& numerator, Bignum& denominator, Bignum& deltaMinus, Bignum& deltaPlus)
{
    assert(estimatedPower >= 0);

    numerator.AssignUInt64(significand);
    numerator.ShiftLeft(exponent);
    denominator.AssignPowerUInt16(10, estimatedPower);

    if (needBoundaryDeltas)
    {
        denominator.ShiftLeft(1);
        numerator.ShiftLeft(1);

        deltaPlus.AssignUInt16(1);
        deltaPlus.ShiftLeft(exponent);

        deltaMinus.AssignUInt16(1);
        deltaMinus.ShiftLeft(exponent);
    }
}

static void InitialScaledStartValuesNegativeExponentPositivePower(uint64_t significand, int exponent,
    int estimatedPower, bool needBoundaryDeltas, Bignum& numerator, Bignum& denominator, Bignum& deltaMinus,
    Bignum& deltaPlus)
{
    numerator.AssignUInt64(significand);
    denominator.AssignPowerUInt16(10, estimatedPower);
    denominator.ShiftLeft(-exponent);

    if (needBoundaryDeltas)
    {
        denominator.ShiftLeft(1);
        numerator.ShiftLeft(1);

        deltaPlus.AssignUInt16(1);

        deltaMinus.AssignUInt16(1);
    }
}

static void InitialScaledStartValuesNegativeExponentNegativePower(uint64_t significand, int exponent,
    int estimatedPower, bool needBoundaryDeltas, Bignum& numerator, Bignum& denominator, Bignum& deltaMinus,
    Bignum& deltaPlus)
{
    Bignum& powerTen = numerator;
    powerTen.AssignPowerUInt16(10, -estimatedPower);

    if (needBoundaryDeltas)
    {
        deltaPlus.AssignBignum(powerTen);
        deltaMinus.AssignBignum(powerTen);
    }

    assert(&numerator == &powerTen);
    numerator.MultiplyByUInt64(significand);

    denominator.AssignUInt16(1);
    denominator.ShiftLeft(-exponent);

    if (needBoundaryDeltas)
    {
        numerator.ShiftLeft(1);
        denominator.ShiftLeft(1);
    }
}

int BignumDtoa::NormalizedExponent(uint64_t significand, int exponent)
{
    assert(significand != 0);
    while ((significand & Double::kHiddenBit) == 0)
    {
        significand = significand << 1;
        exponent = exponent - 1;
    }

    return exponent;
}

int BignumDtoa::EstimatePower(int exponent)
{
    const double k1Log10 = 0.30102999566398114;  // 1/lg(10)

    const int kSignificandSize = Double::kSignificandSize;
    double estimate = std::ceil((exponent + kSignificandSize - 1) * k1Log10 - 1e-10);
    return static_cast<int>(estimate);
}

void BignumDtoa::InitialScaledStartValues(uint64_t significand, int exponent, bool lowerBoundaryIsCloser,
    int estimatedPower, bool needBoundaryDeltas, Bignum& numerator, Bignum& denominator, Bignum& deltaMinus,
    Bignum& deltaPlus)
{
    if (exponent >= 0)
    {
        InitialScaledStartValuesPositiveExponent(significand, exponent, estimatedPower, needBoundaryDeltas, numerator,
            denominator, deltaMinus, deltaPlus);
    }
    else if (estimatedPower >= 0)
    {
        InitialScaledStartValuesNegativeExponentPositivePower(significand, exponent, estimatedPower, needBoundaryDeltas,
            numerator, denominator, deltaMinus, deltaPlus);
    }
    else
    {
        InitialScaledStartValuesNegativeExponentNegativePower(significand, exponent, estimatedPower, needBoundaryDeltas,
            numerator, denominator, deltaMinus, deltaPlus);
    }

    if (needBoundaryDeltas && lowerBoundaryIsCloser)
    {
        denominator.ShiftLeft(1);
        numerator.ShiftLeft(1);
        deltaPlus.ShiftLeft(1);
    }
}

void BignumDtoa::FixupMultiply10(int estimatedPower, bool isEven, int& decimalPoint, Bignum& numerator,
    Bignum& denominator, Bignum& deltaMinus, Bignum& deltaPlus)
{
    bool inRange;
    if (isEven)
        inRange = Bignum::PlusCompare(numerator, deltaPlus, denominator) >= 0;
    else
        inRange = Bignum::PlusCompare(numerator, deltaPlus, denominator) > 0;

    if (inRange)
        decimalPoint = estimatedPower + 1;
    else
    {
        decimalPoint = estimatedPower;
        numerator.Times10();
        if (Bignum::Equal(deltaMinus, deltaPlus))
        {
            deltaMinus.Times10();
            deltaPlus.AssignBignum(deltaMinus);
        }
        else
        {
            deltaMinus.Times10();
            deltaPlus.Times10();
        }
    }
}
