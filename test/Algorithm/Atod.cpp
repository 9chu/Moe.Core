/**
 * @file
 * @date 2017/5/6
 */
#include <gtest/gtest.h>

#include <cmath>

#include <Moe.Core/Algorithm/Atod.hpp>

using namespace std;
using namespace moe;
using namespace internal;

namespace
{
    template <typename T>
    ArrayView<T> StringToVector(const T* str)
    {
        return ArrayView<T>(str, char_traits<T>::length(str));
    }

    template <typename T>
    double StrtodChar(const T* str, int exponent)
    {
        return StringToDoubleConverter<T>::Strtod(StringToVector(str), exponent);
    }

    template <typename T>
    float StrtofChar(const T* str, int exponent)
    {
        return StringToDoubleConverter<T>::Strtof(StringToVector(str), exponent);
    }

    int CompareBignumToDiyFp(const Bignum& bignumDigits, int bignumExponent, DiyFp diyFp)
    {
        Bignum bignum;
        bignum.AssignBignum(bignumDigits);
        Bignum other;
        other.AssignUInt64(diyFp.Significand());
        if (bignumExponent >= 0)
            bignum.MultiplyByPowerOfTen(bignumExponent);
        else
            other.MultiplyByPowerOfTen(-bignumExponent);

        if (diyFp.Exponent() >= 0)
            other.ShiftLeft(diyFp.Exponent());
        else
            bignum.ShiftLeft(-diyFp.Exponent());

        return Bignum::Compare(bignum, other);
    }

    bool CheckDouble(ArrayView<char> buffer, int exponent, double toCheck)
    {
        DiyFp lowerBoundary;
        DiyFp upperBoundary;
        Bignum inputDigits;
        inputDigits.AssignDecimalString(buffer);

        if (toCheck == 0.0)
        {
            const double kMinDouble = 4e-324;
            Double d(kMinDouble);
            d.NormalizedBoundaries(lowerBoundary, upperBoundary);
            return CompareBignumToDiyFp(inputDigits, exponent, lowerBoundary) <= 0;
        }

        if (toCheck == Double::Infinity())
        {
            const double kMaxDouble = 1.7976931348623157e308;
            // Check that the buffer*10^exponent >= boundary between kMaxDouble and inf.
            Double d(kMaxDouble);
            d.NormalizedBoundaries(lowerBoundary, upperBoundary);
            return CompareBignumToDiyFp(inputDigits, exponent, upperBoundary) >= 0;
        }

        Double d(toCheck);
        d.NormalizedBoundaries(lowerBoundary, upperBoundary);
        if ((d.Significand() & 1) == 0)
        {
            return CompareBignumToDiyFp(inputDigits, exponent, lowerBoundary) >= 0 &&
                CompareBignumToDiyFp(inputDigits, exponent, upperBoundary) <= 0;
        }
        else
        {
            return CompareBignumToDiyFp(inputDigits, exponent, lowerBoundary) > 0 &&
                CompareBignumToDiyFp(inputDigits, exponent, upperBoundary) < 0;
        }
    }

    // Copied from v8.cc and adapted to make the function deterministic.
    uint32_t DeterministicRandom()
    {
        // Random number generator using George Marsaglia's MWC algorithm.
        static uint32_t hi = 0;
        static uint32_t lo = 0;

        // Initialization values don't have any special meaning. (They are the result
        // of two calls to random().)
        if (hi == 0) hi = 0xbfe166e7;
        if (lo == 0) lo = 0x64d1c3c9;

        // Mix the bits.
        hi = 36969 * (hi & 0xFFFF) + (hi >> 16);
        lo = 18273 * (lo & 0xFFFF) + (lo >> 16);
        return (hi << 16) + (lo & 0xFFFF);
    }

    bool CheckFloat(ArrayView<char> buffer, int exponent, float toCheck)
    {
        DiyFp lowerBoundary;
        DiyFp upperBoundary;
        Bignum inputDigits;
        inputDigits.AssignDecimalString(buffer);
        if (toCheck == 0.0)
        {
            const float kMinFloat = 1e-45f;
            // Check that the buffer*10^exponent < (0 + kMinFloat)/2.
            Single s(kMinFloat);
            s.NormalizedBoundaries(lowerBoundary, upperBoundary);
            return CompareBignumToDiyFp(inputDigits, exponent, lowerBoundary) <= 0;
        }

        if (toCheck == static_cast<float>(Double::Infinity()))
        {
            const float kMaxFloat = 3.4028234e38f;
            // Check that the buffer*10^exponent >= boundary between kMaxFloat and inf.
            Single s(kMaxFloat);
            s.NormalizedBoundaries(lowerBoundary, upperBoundary);
            return CompareBignumToDiyFp(inputDigits, exponent, upperBoundary) >= 0;
        }

        Single s(toCheck);
        s.NormalizedBoundaries(lowerBoundary, upperBoundary);
        if ((s.Significand() & 1) == 0)
        {
            return CompareBignumToDiyFp(inputDigits, exponent, lowerBoundary) >= 0 &&
                CompareBignumToDiyFp(inputDigits, exponent, upperBoundary) <= 0;
        }
        else
        {
            return CompareBignumToDiyFp(inputDigits, exponent, lowerBoundary) > 0 &&
                CompareBignumToDiyFp(inputDigits, exponent, upperBoundary) < 0;
        }
    }
}

TEST(Atod, Strtod)
{
    ArrayView<char> vector;

    vector = StringToVector("0");
    EXPECT_EQ(0.0, StringToDoubleConverter<char>::Strtod(vector, 1));
    EXPECT_EQ(0.0, StringToDoubleConverter<char>::Strtod(vector, 2));
    EXPECT_EQ(0.0, StringToDoubleConverter<char>::Strtod(vector, -2));
    EXPECT_EQ(0.0, StringToDoubleConverter<char>::Strtod(vector, -999));
    EXPECT_EQ(0.0, StringToDoubleConverter<char>::Strtod(vector, +999));

    vector = StringToVector("1");
    EXPECT_EQ(1.0, StringToDoubleConverter<char>::Strtod(vector, 0));
    EXPECT_EQ(10.0, StringToDoubleConverter<char>::Strtod(vector, 1));
    EXPECT_EQ(100.0, StringToDoubleConverter<char>::Strtod(vector, 2));
    EXPECT_EQ(1e20, StringToDoubleConverter<char>::Strtod(vector, 20));
    EXPECT_EQ(1e22, StringToDoubleConverter<char>::Strtod(vector, 22));
    EXPECT_EQ(1e23, StringToDoubleConverter<char>::Strtod(vector, 23));
    EXPECT_EQ(1e35, StringToDoubleConverter<char>::Strtod(vector, 35));
    EXPECT_EQ(1e36, StringToDoubleConverter<char>::Strtod(vector, 36));
    EXPECT_EQ(1e37, StringToDoubleConverter<char>::Strtod(vector, 37));
    EXPECT_EQ(1e-1, StringToDoubleConverter<char>::Strtod(vector, -1));
    EXPECT_EQ(1e-2, StringToDoubleConverter<char>::Strtod(vector, -2));
    EXPECT_EQ(1e-5, StringToDoubleConverter<char>::Strtod(vector, -5));
    EXPECT_EQ(1e-20, StringToDoubleConverter<char>::Strtod(vector, -20));
    EXPECT_EQ(1e-22, StringToDoubleConverter<char>::Strtod(vector, -22));
    EXPECT_EQ(1e-23, StringToDoubleConverter<char>::Strtod(vector, -23));
    EXPECT_EQ(1e-25, StringToDoubleConverter<char>::Strtod(vector, -25));
    EXPECT_EQ(1e-39, StringToDoubleConverter<char>::Strtod(vector, -39));

    vector = StringToVector("2");
    EXPECT_EQ(2.0, StringToDoubleConverter<char>::Strtod(vector, 0));
    EXPECT_EQ(20.0, StringToDoubleConverter<char>::Strtod(vector, 1));
    EXPECT_EQ(200.0, StringToDoubleConverter<char>::Strtod(vector, 2));
    EXPECT_EQ(2e20, StringToDoubleConverter<char>::Strtod(vector, 20));
    EXPECT_EQ(2e22, StringToDoubleConverter<char>::Strtod(vector, 22));
    EXPECT_EQ(2e23, StringToDoubleConverter<char>::Strtod(vector, 23));
    EXPECT_EQ(2e35, StringToDoubleConverter<char>::Strtod(vector, 35));
    EXPECT_EQ(2e36, StringToDoubleConverter<char>::Strtod(vector, 36));
    EXPECT_EQ(2e37, StringToDoubleConverter<char>::Strtod(vector, 37));
    EXPECT_EQ(2e-1, StringToDoubleConverter<char>::Strtod(vector, -1));
    EXPECT_EQ(2e-2, StringToDoubleConverter<char>::Strtod(vector, -2));
    EXPECT_EQ(2e-5, StringToDoubleConverter<char>::Strtod(vector, -5));
    EXPECT_EQ(2e-20, StringToDoubleConverter<char>::Strtod(vector, -20));
    EXPECT_EQ(2e-22, StringToDoubleConverter<char>::Strtod(vector, -22));
    EXPECT_EQ(2e-23, StringToDoubleConverter<char>::Strtod(vector, -23));
    EXPECT_EQ(2e-25, StringToDoubleConverter<char>::Strtod(vector, -25));
    EXPECT_EQ(2e-39, StringToDoubleConverter<char>::Strtod(vector, -39));

    vector = StringToVector("9");
    EXPECT_EQ(9.0, StringToDoubleConverter<char>::Strtod(vector, 0));
    EXPECT_EQ(90.0, StringToDoubleConverter<char>::Strtod(vector, 1));
    EXPECT_EQ(900.0, StringToDoubleConverter<char>::Strtod(vector, 2));
    EXPECT_EQ(9e20, StringToDoubleConverter<char>::Strtod(vector, 20));
    EXPECT_EQ(9e22, StringToDoubleConverter<char>::Strtod(vector, 22));
    EXPECT_EQ(9e23, StringToDoubleConverter<char>::Strtod(vector, 23));
    EXPECT_EQ(9e35, StringToDoubleConverter<char>::Strtod(vector, 35));
    EXPECT_EQ(9e36, StringToDoubleConverter<char>::Strtod(vector, 36));
    EXPECT_EQ(9e37, StringToDoubleConverter<char>::Strtod(vector, 37));
    EXPECT_EQ(9e-1, StringToDoubleConverter<char>::Strtod(vector, -1));
    EXPECT_EQ(9e-2, StringToDoubleConverter<char>::Strtod(vector, -2));
    EXPECT_EQ(9e-5, StringToDoubleConverter<char>::Strtod(vector, -5));
    EXPECT_EQ(9e-20, StringToDoubleConverter<char>::Strtod(vector, -20));
    EXPECT_EQ(9e-22, StringToDoubleConverter<char>::Strtod(vector, -22));
    EXPECT_EQ(9e-23, StringToDoubleConverter<char>::Strtod(vector, -23));
    EXPECT_EQ(9e-25, StringToDoubleConverter<char>::Strtod(vector, -25));
    EXPECT_EQ(9e-39, StringToDoubleConverter<char>::Strtod(vector, -39));

    vector = StringToVector("12345");
    EXPECT_EQ(12345.0, StringToDoubleConverter<char>::Strtod(vector, 0));
    EXPECT_EQ(123450.0, StringToDoubleConverter<char>::Strtod(vector, 1));
    EXPECT_EQ(1234500.0, StringToDoubleConverter<char>::Strtod(vector, 2));
    EXPECT_EQ(12345e20, StringToDoubleConverter<char>::Strtod(vector, 20));
    EXPECT_EQ(12345e22, StringToDoubleConverter<char>::Strtod(vector, 22));
    EXPECT_EQ(12345e23, StringToDoubleConverter<char>::Strtod(vector, 23));
    EXPECT_EQ(12345e30, StringToDoubleConverter<char>::Strtod(vector, 30));
    EXPECT_EQ(12345e31, StringToDoubleConverter<char>::Strtod(vector, 31));
    EXPECT_EQ(12345e32, StringToDoubleConverter<char>::Strtod(vector, 32));
    EXPECT_EQ(12345e35, StringToDoubleConverter<char>::Strtod(vector, 35));
    EXPECT_EQ(12345e36, StringToDoubleConverter<char>::Strtod(vector, 36));
    EXPECT_EQ(12345e37, StringToDoubleConverter<char>::Strtod(vector, 37));
    EXPECT_EQ(12345e-1, StringToDoubleConverter<char>::Strtod(vector, -1));
    EXPECT_EQ(12345e-2, StringToDoubleConverter<char>::Strtod(vector, -2));
    EXPECT_EQ(12345e-5, StringToDoubleConverter<char>::Strtod(vector, -5));
    EXPECT_EQ(12345e-20, StringToDoubleConverter<char>::Strtod(vector, -20));
    EXPECT_EQ(12345e-22, StringToDoubleConverter<char>::Strtod(vector, -22));
    EXPECT_EQ(12345e-23, StringToDoubleConverter<char>::Strtod(vector, -23));
    EXPECT_EQ(12345e-25, StringToDoubleConverter<char>::Strtod(vector, -25));
    EXPECT_EQ(12345e-39, StringToDoubleConverter<char>::Strtod(vector, -39));

    vector = StringToVector("12345678901234");
    EXPECT_EQ(12345678901234.0, StringToDoubleConverter<char>::Strtod(vector, 0));
    EXPECT_EQ(123456789012340.0, StringToDoubleConverter<char>::Strtod(vector, 1));
    EXPECT_EQ(1234567890123400.0, StringToDoubleConverter<char>::Strtod(vector, 2));
    EXPECT_EQ(12345678901234e20, StringToDoubleConverter<char>::Strtod(vector, 20));
    EXPECT_EQ(12345678901234e22, StringToDoubleConverter<char>::Strtod(vector, 22));
    EXPECT_EQ(12345678901234e23, StringToDoubleConverter<char>::Strtod(vector, 23));
    EXPECT_EQ(12345678901234e30, StringToDoubleConverter<char>::Strtod(vector, 30));
    EXPECT_EQ(12345678901234e31, StringToDoubleConverter<char>::Strtod(vector, 31));
    EXPECT_EQ(12345678901234e32, StringToDoubleConverter<char>::Strtod(vector, 32));
    EXPECT_EQ(12345678901234e35, StringToDoubleConverter<char>::Strtod(vector, 35));
    EXPECT_EQ(12345678901234e36, StringToDoubleConverter<char>::Strtod(vector, 36));
    EXPECT_EQ(12345678901234e37, StringToDoubleConverter<char>::Strtod(vector, 37));
    EXPECT_EQ(12345678901234e-1, StringToDoubleConverter<char>::Strtod(vector, -1));
    EXPECT_EQ(12345678901234e-2, StringToDoubleConverter<char>::Strtod(vector, -2));
    EXPECT_EQ(12345678901234e-5, StringToDoubleConverter<char>::Strtod(vector, -5));
    EXPECT_EQ(12345678901234e-20, StringToDoubleConverter<char>::Strtod(vector, -20));
    EXPECT_EQ(12345678901234e-22, StringToDoubleConverter<char>::Strtod(vector, -22));
    EXPECT_EQ(12345678901234e-23, StringToDoubleConverter<char>::Strtod(vector, -23));
    EXPECT_EQ(12345678901234e-25, StringToDoubleConverter<char>::Strtod(vector, -25));
    EXPECT_EQ(12345678901234e-39, StringToDoubleConverter<char>::Strtod(vector, -39));

    vector = StringToVector("123456789012345");
    EXPECT_EQ(123456789012345.0, StringToDoubleConverter<char>::Strtod(vector, 0));
    EXPECT_EQ(1234567890123450.0, StringToDoubleConverter<char>::Strtod(vector, 1));
    EXPECT_EQ(12345678901234500.0, StringToDoubleConverter<char>::Strtod(vector, 2));
    EXPECT_EQ(123456789012345e20, StringToDoubleConverter<char>::Strtod(vector, 20));
    EXPECT_EQ(123456789012345e22, StringToDoubleConverter<char>::Strtod(vector, 22));
    EXPECT_EQ(123456789012345e23, StringToDoubleConverter<char>::Strtod(vector, 23));
    EXPECT_EQ(123456789012345e35, StringToDoubleConverter<char>::Strtod(vector, 35));
    EXPECT_EQ(123456789012345e36, StringToDoubleConverter<char>::Strtod(vector, 36));
    EXPECT_EQ(123456789012345e37, StringToDoubleConverter<char>::Strtod(vector, 37));
    EXPECT_EQ(123456789012345e39, StringToDoubleConverter<char>::Strtod(vector, 39));
    EXPECT_EQ(123456789012345e-1, StringToDoubleConverter<char>::Strtod(vector, -1));
    EXPECT_EQ(123456789012345e-2, StringToDoubleConverter<char>::Strtod(vector, -2));
    EXPECT_EQ(123456789012345e-5, StringToDoubleConverter<char>::Strtod(vector, -5));
    EXPECT_EQ(123456789012345e-20, StringToDoubleConverter<char>::Strtod(vector, -20));
    EXPECT_EQ(123456789012345e-22, StringToDoubleConverter<char>::Strtod(vector, -22));
    EXPECT_EQ(123456789012345e-23, StringToDoubleConverter<char>::Strtod(vector, -23));
    EXPECT_EQ(123456789012345e-25, StringToDoubleConverter<char>::Strtod(vector, -25));
    EXPECT_EQ(123456789012345e-39, StringToDoubleConverter<char>::Strtod(vector, -39));

    EXPECT_EQ(0.0, StrtodChar("0", 12345));
    EXPECT_EQ(0.0, StrtodChar("", 1324));
    EXPECT_EQ(0.0, StrtodChar("000000000", 123));
    EXPECT_EQ(0.0, StrtodChar("2", -324));
    EXPECT_EQ(4e-324, StrtodChar("3", -324));
    // It would be more readable to put non-zero literals on the left side (i.e.
    //   EXPECT_EQ(1e-325, StrtodChar("1", -325))), but then Gcc complains that
    // they are truncated to zero.
    EXPECT_EQ(0.0, StrtodChar("1", -325));
    EXPECT_EQ(0.0, StrtodChar("1", -325));
    EXPECT_EQ(0.0, StrtodChar("20000", -328));
    EXPECT_EQ(40000e-328, StrtodChar("30000", -328));
    EXPECT_EQ(0.0, StrtodChar("10000", -329));
    EXPECT_EQ(0.0, StrtodChar("90000", -329));
    EXPECT_EQ(0.0, StrtodChar("000000001", -325));
    EXPECT_EQ(0.0, StrtodChar("000000001", -325));
    EXPECT_EQ(0.0, StrtodChar("0000000020000", -328));
    EXPECT_EQ(40000e-328, StrtodChar("00000030000", -328));
    EXPECT_EQ(0.0, StrtodChar("0000000010000", -329));
    EXPECT_EQ(0.0, StrtodChar("0000000090000", -329));

    // It would be more readable to put the literals (and not Double::Infinity())
    // on the left side (i.e. EXPECT_EQ(1e309, StrtodChar("1", 309))), but then Gcc
    // complains that the floating constant exceeds range of 'double'.
    EXPECT_EQ(Double::Infinity(), StrtodChar("1", 309));
    EXPECT_EQ(1e308, StrtodChar("1", 308));
    EXPECT_EQ(1234e305, StrtodChar("1234", 305));
    EXPECT_EQ(1234e304, StrtodChar("1234", 304));
    EXPECT_EQ(Double::Infinity(), StrtodChar("18", 307));
    EXPECT_EQ(17e307, StrtodChar("17", 307));
    EXPECT_EQ(Double::Infinity(), StrtodChar("0000001", 309));
    EXPECT_EQ(1e308, StrtodChar("00000001", 308));
    EXPECT_EQ(1234e305, StrtodChar("00000001234", 305));
    EXPECT_EQ(1234e304, StrtodChar("000000001234", 304));
    EXPECT_EQ(Double::Infinity(), StrtodChar("0000000018", 307));
    EXPECT_EQ(17e307, StrtodChar("0000000017", 307));
    EXPECT_EQ(Double::Infinity(), StrtodChar("1000000", 303));
    EXPECT_EQ(1e308, StrtodChar("100000", 303));
    EXPECT_EQ(1234e305, StrtodChar("123400000", 300));
    EXPECT_EQ(1234e304, StrtodChar("123400000", 299));
    EXPECT_EQ(Double::Infinity(), StrtodChar("180000000", 300));
    EXPECT_EQ(17e307, StrtodChar("170000000", 300));
    EXPECT_EQ(Double::Infinity(), StrtodChar("00000001000000", 303));
    EXPECT_EQ(1e308, StrtodChar("000000000000100000", 303));
    EXPECT_EQ(1234e305, StrtodChar("00000000123400000", 300));
    EXPECT_EQ(1234e304, StrtodChar("0000000123400000", 299));
    EXPECT_EQ(Double::Infinity(), StrtodChar("00000000180000000", 300));
    EXPECT_EQ(17e307, StrtodChar("00000000170000000", 300));
    EXPECT_EQ(1.7976931348623157E+308, StrtodChar("17976931348623157", 292));
    EXPECT_EQ(1.7976931348623158E+308, StrtodChar("17976931348623158", 292));
    EXPECT_EQ(Double::Infinity(), StrtodChar("17976931348623159", 292));

    // The following number is the result of 89255.0/1e-22. Both floating-point
    // numbers can be accurately represented with doubles. However on Linux,x86
    // the floating-point stack is set to 80bits and the double-rounding
    // introduces an error.
    EXPECT_EQ(89255e-22, StrtodChar("89255", -22));

    // Some random values.
    EXPECT_EQ(358416272e-33, StrtodChar("358416272", -33));
    EXPECT_EQ(104110013277974872254e-225, StrtodChar("104110013277974872254", -225));

    EXPECT_EQ(123456789e108, StrtodChar("123456789", 108));
    EXPECT_EQ(123456789e109, StrtodChar("123456789", 109));
    EXPECT_EQ(123456789e110, StrtodChar("123456789", 110));
    EXPECT_EQ(123456789e111, StrtodChar("123456789", 111));
    EXPECT_EQ(123456789e112, StrtodChar("123456789", 112));
    EXPECT_EQ(123456789e113, StrtodChar("123456789", 113));
    EXPECT_EQ(123456789e114, StrtodChar("123456789", 114));
    EXPECT_EQ(123456789e115, StrtodChar("123456789", 115));

    EXPECT_EQ(1234567890123456789012345e108, StrtodChar("1234567890123456789012345", 108));
    EXPECT_EQ(1234567890123456789012345e109, StrtodChar("1234567890123456789012345", 109));
    EXPECT_EQ(1234567890123456789012345e110, StrtodChar("1234567890123456789012345", 110));
    EXPECT_EQ(1234567890123456789012345e111, StrtodChar("1234567890123456789012345", 111));
    EXPECT_EQ(1234567890123456789012345e112, StrtodChar("1234567890123456789012345", 112));
    EXPECT_EQ(1234567890123456789012345e113, StrtodChar("1234567890123456789012345", 113));
    EXPECT_EQ(1234567890123456789012345e114, StrtodChar("1234567890123456789012345", 114));
    EXPECT_EQ(1234567890123456789012345e115, StrtodChar("1234567890123456789012345", 115));

    EXPECT_EQ(1234567890123456789052345e108, StrtodChar("1234567890123456789052345", 108));
    EXPECT_EQ(1234567890123456789052345e109, StrtodChar("1234567890123456789052345", 109));
    EXPECT_EQ(1234567890123456789052345e110, StrtodChar("1234567890123456789052345", 110));
    EXPECT_EQ(1234567890123456789052345e111, StrtodChar("1234567890123456789052345", 111));
    EXPECT_EQ(1234567890123456789052345e112, StrtodChar("1234567890123456789052345", 112));
    EXPECT_EQ(1234567890123456789052345e113, StrtodChar("1234567890123456789052345", 113));
    EXPECT_EQ(1234567890123456789052345e114, StrtodChar("1234567890123456789052345", 114));
    EXPECT_EQ(1234567890123456789052345e115, StrtodChar("1234567890123456789052345", 115));

    EXPECT_EQ(5.445618932859895e-255, StrtodChar("5445618932859895362967233318697132813618813095743952975"
        "4392982234069699615600475529427176366709107287468930197"
        "8628345413991790019316974825934906752493984055268219809"
        "5012176093045431437495773903922425632551857520884625114"
        "6241265881735209066709685420744388526014389929047617597"
        "0302268848374508109029268898695825171158085457567481507"
        "4162979705098246243690189880319928315307816832576838178"
        "2563074014542859888710209237525873301724479666744537857"
        "9026553346649664045621387124193095870305991178772256504"
        "4368663670643970181259143319016472430928902201239474588"
        "1392338901353291306607057623202353588698746085415097902"
        "6640064319118728664842287477491068264828851624402189317"
        "2769161449825765517353755844373640588822904791244190695"
        "2998382932630754670573838138825217065450843010498555058"
        "88186560731", -1035));

    // Boundary cases. Boundaries themselves should round to even.
    //
    // 0x1FFFFFFFFFFFF * 2^3 = 72057594037927928
    //                   next: 72057594037927936
    //               boundary: 72057594037927932  should round up.
    EXPECT_EQ(72057594037927928.0, StrtodChar("72057594037927928", 0));
    EXPECT_EQ(72057594037927936.0, StrtodChar("72057594037927936", 0));
    EXPECT_EQ(72057594037927936.0, StrtodChar("72057594037927932", 0));
    EXPECT_EQ(72057594037927928.0, StrtodChar("7205759403792793199999", -5));
    EXPECT_EQ(72057594037927936.0, StrtodChar("7205759403792793200001", -5));

    // 0x1FFFFFFFFFFFF * 2^10 = 9223372036854774784
    //                    next: 9223372036854775808
    //                boundary: 9223372036854775296 should round up.
    EXPECT_EQ(9223372036854774784.0, StrtodChar("9223372036854774784", 0));
    EXPECT_EQ(9223372036854775808.0, StrtodChar("9223372036854775808", 0));
    EXPECT_EQ(9223372036854775808.0, StrtodChar("9223372036854775296", 0));
    EXPECT_EQ(9223372036854774784.0, StrtodChar("922337203685477529599999", -5));
    EXPECT_EQ(9223372036854775808.0, StrtodChar("922337203685477529600001", -5));

    // 0x1FFFFFFFFFFFF * 2^50 = 10141204801825834086073718800384
    //                    next: 10141204801825835211973625643008
    //                boundary: 10141204801825834649023672221696 should round up.
    EXPECT_EQ(10141204801825834086073718800384.0, StrtodChar("10141204801825834086073718800384", 0));
    EXPECT_EQ(10141204801825835211973625643008.0, StrtodChar("10141204801825835211973625643008", 0));
    EXPECT_EQ(10141204801825835211973625643008.0, StrtodChar("10141204801825834649023672221696", 0));
    EXPECT_EQ(10141204801825834086073718800384.0, StrtodChar("1014120480182583464902367222169599999", -5));
    EXPECT_EQ(10141204801825835211973625643008.0, StrtodChar("1014120480182583464902367222169600001", -5));

    // 0x1FFFFFFFFFFFF * 2^99 = 5708990770823838890407843763683279797179383808
    //                    next: 5708990770823839524233143877797980545530986496
    //                boundary: 5708990770823839207320493820740630171355185152
    // The boundary should round up.
    EXPECT_EQ(5708990770823838890407843763683279797179383808.0,
        StrtodChar("5708990770823838890407843763683279797179383808", 0));
    EXPECT_EQ(5708990770823839524233143877797980545530986496.0,
        StrtodChar("5708990770823839524233143877797980545530986496", 0));
    EXPECT_EQ(5708990770823839524233143877797980545530986496.0,
        StrtodChar("5708990770823839207320493820740630171355185152", 0));
    EXPECT_EQ(5708990770823838890407843763683279797179383808.0,
        StrtodChar("5708990770823839207320493820740630171355185151999", -3));
    EXPECT_EQ(5708990770823839524233143877797980545530986496.0,
        StrtodChar("5708990770823839207320493820740630171355185152001", -3));

    // The following test-cases got some public attention in early 2011 when they
    // sent Java and PHP into an infinite loop.
    EXPECT_EQ(2.225073858507201e-308, StrtodChar("22250738585072011", -324));
    EXPECT_EQ(2.22507385850720138309e-308, StrtodChar("22250738585072011360574097967091319759348195463516456480"
        "23426109724822222021076945516529523908135087914149158913"
        "03962110687008643869459464552765720740782062174337998814"
        "10632673292535522868813721490129811224514518898490572223"
        "07285255133155755015914397476397983411801999323962548289"
        "01710708185069063066665599493827577257201576306269066333"
        "26475653000092458883164330377797918696120494973903778297"
        "04905051080609940730262937128958950003583799967207254304"
        "36028407889577179615094551674824347103070260914462157228"
        "98802581825451803257070188608721131280795122334262883686"
        "22321503775666622503982534335974568884423900265498198385"
        "48794829220689472168983109969836584681402285424333066033"
        "98508864458040010349339704275671864433837704860378616227"
        "71738545623065874679014086723327636718751", -1076));
}

TEST(Atod, Strtof)
{
    ArrayView<char> vector;

    vector = StringToVector("0");
    EXPECT_EQ(0.0f, StringToDoubleConverter<char>::Strtof(vector, 1));
    EXPECT_EQ(0.0f, StringToDoubleConverter<char>::Strtof(vector, 2));
    EXPECT_EQ(0.0f, StringToDoubleConverter<char>::Strtof(vector, -2));
    EXPECT_EQ(0.0f, StringToDoubleConverter<char>::Strtof(vector, -999));
    EXPECT_EQ(0.0f, StringToDoubleConverter<char>::Strtof(vector, +999));

    vector = StringToVector("1");
    EXPECT_EQ(1.0f, StringToDoubleConverter<char>::Strtof(vector, 0));
    EXPECT_EQ(10.0f, StringToDoubleConverter<char>::Strtof(vector, 1));
    EXPECT_EQ(100.0f, StringToDoubleConverter<char>::Strtof(vector, 2));
    EXPECT_EQ(1e20f, StringToDoubleConverter<char>::Strtof(vector, 20));
    EXPECT_EQ(1e22f, StringToDoubleConverter<char>::Strtof(vector, 22));
    EXPECT_EQ(1e23f, StringToDoubleConverter<char>::Strtof(vector, 23));
    EXPECT_EQ(1e35f, StringToDoubleConverter<char>::Strtof(vector, 35));
    EXPECT_EQ(1e36f, StringToDoubleConverter<char>::Strtof(vector, 36));
    EXPECT_EQ(1e37f, StringToDoubleConverter<char>::Strtof(vector, 37));
    EXPECT_EQ(1e-1f, StringToDoubleConverter<char>::Strtof(vector, -1));
    EXPECT_EQ(1e-2f, StringToDoubleConverter<char>::Strtof(vector, -2));
    EXPECT_EQ(1e-5f, StringToDoubleConverter<char>::Strtof(vector, -5));
    EXPECT_EQ(1e-20f, StringToDoubleConverter<char>::Strtof(vector, -20));
    EXPECT_EQ(1e-22f, StringToDoubleConverter<char>::Strtof(vector, -22));
    EXPECT_EQ(1e-23f, StringToDoubleConverter<char>::Strtof(vector, -23));
    EXPECT_EQ(1e-25f, StringToDoubleConverter<char>::Strtof(vector, -25));
    EXPECT_EQ(1e-39f, StringToDoubleConverter<char>::Strtof(vector, -39));

    vector = StringToVector("2");
    EXPECT_EQ(2.0f, StringToDoubleConverter<char>::Strtof(vector, 0));
    EXPECT_EQ(20.0f, StringToDoubleConverter<char>::Strtof(vector, 1));
    EXPECT_EQ(200.0f, StringToDoubleConverter<char>::Strtof(vector, 2));
    EXPECT_EQ(2e20f, StringToDoubleConverter<char>::Strtof(vector, 20));
    EXPECT_EQ(2e22f, StringToDoubleConverter<char>::Strtof(vector, 22));
    EXPECT_EQ(2e23f, StringToDoubleConverter<char>::Strtof(vector, 23));
    EXPECT_EQ(2e35f, StringToDoubleConverter<char>::Strtof(vector, 35));
    EXPECT_EQ(2e36f, StringToDoubleConverter<char>::Strtof(vector, 36));
    EXPECT_EQ(2e37f, StringToDoubleConverter<char>::Strtof(vector, 37));
    EXPECT_EQ(2e-1f, StringToDoubleConverter<char>::Strtof(vector, -1));
    EXPECT_EQ(2e-2f, StringToDoubleConverter<char>::Strtof(vector, -2));
    EXPECT_EQ(2e-5f, StringToDoubleConverter<char>::Strtof(vector, -5));
    EXPECT_EQ(2e-20f, StringToDoubleConverter<char>::Strtof(vector, -20));
    EXPECT_EQ(2e-22f, StringToDoubleConverter<char>::Strtof(vector, -22));
    EXPECT_EQ(2e-23f, StringToDoubleConverter<char>::Strtof(vector, -23));
    EXPECT_EQ(2e-25f, StringToDoubleConverter<char>::Strtof(vector, -25));
    EXPECT_EQ(2e-39f, StringToDoubleConverter<char>::Strtof(vector, -39));

    vector = StringToVector("9");
    EXPECT_EQ(9.0f, StringToDoubleConverter<char>::Strtof(vector, 0));
    EXPECT_EQ(90.0f, StringToDoubleConverter<char>::Strtof(vector, 1));
    EXPECT_EQ(900.0f, StringToDoubleConverter<char>::Strtof(vector, 2));
    EXPECT_EQ(9e20f, StringToDoubleConverter<char>::Strtof(vector, 20));
    EXPECT_EQ(9e22f, StringToDoubleConverter<char>::Strtof(vector, 22));
    EXPECT_EQ(9e23f, StringToDoubleConverter<char>::Strtof(vector, 23));
    EXPECT_EQ(9e35f, StringToDoubleConverter<char>::Strtof(vector, 35));
    EXPECT_EQ(9e36f, StringToDoubleConverter<char>::Strtof(vector, 36));
    EXPECT_EQ(9e37f, StringToDoubleConverter<char>::Strtof(vector, 37));
    EXPECT_EQ(9e-1f, StringToDoubleConverter<char>::Strtof(vector, -1));
    EXPECT_EQ(9e-2f, StringToDoubleConverter<char>::Strtof(vector, -2));
    EXPECT_EQ(9e-5f, StringToDoubleConverter<char>::Strtof(vector, -5));
    EXPECT_EQ(9e-20f, StringToDoubleConverter<char>::Strtof(vector, -20));
    EXPECT_EQ(9e-22f, StringToDoubleConverter<char>::Strtof(vector, -22));
    EXPECT_EQ(9e-23f, StringToDoubleConverter<char>::Strtof(vector, -23));
    EXPECT_EQ(9e-25f, StringToDoubleConverter<char>::Strtof(vector, -25));
    EXPECT_EQ(9e-39f, StringToDoubleConverter<char>::Strtof(vector, -39));

    vector = StringToVector("12345");
    EXPECT_EQ(12345.0f, StringToDoubleConverter<char>::Strtof(vector, 0));
    EXPECT_EQ(123450.0f, StringToDoubleConverter<char>::Strtof(vector, 1));
    EXPECT_EQ(1234500.0f, StringToDoubleConverter<char>::Strtof(vector, 2));
    EXPECT_EQ(12345e20f, StringToDoubleConverter<char>::Strtof(vector, 20));
    EXPECT_EQ(12345e22f, StringToDoubleConverter<char>::Strtof(vector, 22));
    EXPECT_EQ(12345e23f, StringToDoubleConverter<char>::Strtof(vector, 23));
    EXPECT_EQ(12345e30f, StringToDoubleConverter<char>::Strtof(vector, 30));
    EXPECT_EQ(12345e31f, StringToDoubleConverter<char>::Strtof(vector, 31));
    EXPECT_EQ(12345e32f, StringToDoubleConverter<char>::Strtof(vector, 32));
    EXPECT_EQ(12345e-1f, StringToDoubleConverter<char>::Strtof(vector, -1));
    EXPECT_EQ(12345e-2f, StringToDoubleConverter<char>::Strtof(vector, -2));
    EXPECT_EQ(12345e-5f, StringToDoubleConverter<char>::Strtof(vector, -5));
    EXPECT_EQ(12345e-20f, StringToDoubleConverter<char>::Strtof(vector, -20));
    EXPECT_EQ(12345e-22f, StringToDoubleConverter<char>::Strtof(vector, -22));
    EXPECT_EQ(12345e-23f, StringToDoubleConverter<char>::Strtof(vector, -23));
    EXPECT_EQ(12345e-25f, StringToDoubleConverter<char>::Strtof(vector, -25));
    EXPECT_EQ(12345e-39f, StringToDoubleConverter<char>::Strtof(vector, -39));

    vector = StringToVector("12345678901234");
    EXPECT_EQ(12345678901234.0f, StringToDoubleConverter<char>::Strtof(vector, 0));
    EXPECT_EQ(123456789012340.0f, StringToDoubleConverter<char>::Strtof(vector, 1));
    EXPECT_EQ(1234567890123400.0f, StringToDoubleConverter<char>::Strtof(vector, 2));
    EXPECT_EQ(12345678901234e20f, StringToDoubleConverter<char>::Strtof(vector, 20));
    EXPECT_EQ(12345678901234e22f, StringToDoubleConverter<char>::Strtof(vector, 22));
    EXPECT_EQ(12345678901234e23f, StringToDoubleConverter<char>::Strtof(vector, 23));
    EXPECT_EQ(12345678901234e-1f, StringToDoubleConverter<char>::Strtof(vector, -1));
    EXPECT_EQ(12345678901234e-2f, StringToDoubleConverter<char>::Strtof(vector, -2));
    EXPECT_EQ(12345678901234e-5f, StringToDoubleConverter<char>::Strtof(vector, -5));
    EXPECT_EQ(12345678901234e-20f, StringToDoubleConverter<char>::Strtof(vector, -20));
    EXPECT_EQ(12345678901234e-22f, StringToDoubleConverter<char>::Strtof(vector, -22));
    EXPECT_EQ(12345678901234e-23f, StringToDoubleConverter<char>::Strtof(vector, -23));
    EXPECT_EQ(12345678901234e-25f, StringToDoubleConverter<char>::Strtof(vector, -25));
    EXPECT_EQ(12345678901234e-39f, StringToDoubleConverter<char>::Strtof(vector, -39));

    vector = StringToVector("123456789012345");
    EXPECT_EQ(123456789012345.0f, StringToDoubleConverter<char>::Strtof(vector, 0));
    EXPECT_EQ(1234567890123450.0f, StringToDoubleConverter<char>::Strtof(vector, 1));
    EXPECT_EQ(12345678901234500.0f, StringToDoubleConverter<char>::Strtof(vector, 2));
    EXPECT_EQ(123456789012345e20f, StringToDoubleConverter<char>::Strtof(vector, 20));
    EXPECT_EQ(123456789012345e22f, StringToDoubleConverter<char>::Strtof(vector, 22));
    EXPECT_EQ(123456789012345e23f, StringToDoubleConverter<char>::Strtof(vector, 23));
    EXPECT_EQ(123456789012345e-1f, StringToDoubleConverter<char>::Strtof(vector, -1));
    EXPECT_EQ(123456789012345e-2f, StringToDoubleConverter<char>::Strtof(vector, -2));
    EXPECT_EQ(123456789012345e-5f, StringToDoubleConverter<char>::Strtof(vector, -5));
    EXPECT_EQ(123456789012345e-20f, StringToDoubleConverter<char>::Strtof(vector, -20));
    EXPECT_EQ(123456789012345e-22f, StringToDoubleConverter<char>::Strtof(vector, -22));
    EXPECT_EQ(123456789012345e-23f, StringToDoubleConverter<char>::Strtof(vector, -23));
    EXPECT_EQ(123456789012345e-25f, StringToDoubleConverter<char>::Strtof(vector, -25));
    EXPECT_EQ(123456789012345e-39f, StringToDoubleConverter<char>::Strtof(vector, -39));

    EXPECT_EQ(0.0f, StrtofChar("0", 12345));
    EXPECT_EQ(0.0f, StrtofChar("", 1324));
    EXPECT_EQ(0.0f, StrtofChar("000000000", 123));
    EXPECT_EQ(0.0f, StrtofChar("2", -324));
    EXPECT_EQ(1e-45f, StrtofChar("1", -45));
    // It would be more readable to put non-zero literals on the left side (i.e.
    //   EXPECT_EQ(1e-46, StrtofChar("1", -45))), but then Gcc complains that
    // they are truncated to zero.
    EXPECT_EQ(0.0f, StrtofChar("1", -46));
    EXPECT_EQ(0.0f, StrtofChar("1", -47));
    EXPECT_EQ(1e-45f, StrtofChar("1", -45));
    EXPECT_EQ(1e-45f, StrtofChar("8", -46));
    EXPECT_EQ(0.0f, StrtofChar("200000", -51));
    EXPECT_EQ(100000e-50f, StrtofChar("100000", -50));
    EXPECT_EQ(0.0f, StrtofChar("100000", -51));
    EXPECT_EQ(0.0f, StrtofChar("900000", -52));
    EXPECT_EQ(0.0f, StrtofChar("000000001", -47));
    EXPECT_EQ(0.0f, StrtofChar("000000001", -47));
    EXPECT_EQ(0.0f, StrtofChar("00000000200000", -51));
    EXPECT_EQ(800000e-50f, StrtofChar("000000800000", -50));
    EXPECT_EQ(0.0f, StrtofChar("00000000100000", -51));
    EXPECT_EQ(1e-45f, StrtofChar("00000000900000", -51));

    // It would be more readable to put the literals (and not Double::Infinity())
    // on the left side (i.e. EXPECT_EQ(3e38, StrtofChar("3", 38))), but then Gcc
    // complains that the floating constant exceeds range of 'double'.
    EXPECT_EQ(Single::Infinity(), StrtofChar("3", 39));
    EXPECT_EQ(3e38f, StrtofChar("3", 38));
    EXPECT_EQ(3401e35f, StrtofChar("3401", 35));
    EXPECT_EQ(3401e34f, StrtofChar("3401", 34));
    EXPECT_EQ(Single::Infinity(), StrtofChar("3410", 35));
    EXPECT_EQ(34e37f, StrtofChar("34", 37));
    EXPECT_EQ(Single::Infinity(), StrtofChar("0000001", 39));
    EXPECT_EQ(3401e35f, StrtofChar("0000003401", 35));
    EXPECT_EQ(3401e34f, StrtofChar("0000003401", 34));
    EXPECT_EQ(Single::Infinity(), StrtofChar("0000003410", 35));
    EXPECT_EQ(34e37f, StrtofChar("00000034", 37));
    EXPECT_EQ(1e38f, StrtofChar("100000", 33));
    EXPECT_EQ(3401e35f, StrtofChar("340100000", 30));
    EXPECT_EQ(3401e34f, StrtofChar("340100000", 29));
    EXPECT_EQ(Single::Infinity(), StrtofChar("341000000", 30));
    EXPECT_EQ(34e37f, StrtofChar("3400000", 32));
    EXPECT_EQ(1e38f, StrtofChar("00000100000", 33));
    EXPECT_EQ(3401e35f, StrtofChar("00000340100000", 30));
    EXPECT_EQ(3401e34f, StrtofChar("00000340100000", 29));
    EXPECT_EQ(Single::Infinity(), StrtofChar("00000341000000", 30));
    EXPECT_EQ(34e37f, StrtofChar("000003400000", 32));
    EXPECT_EQ(3.4028234e+38f, StrtofChar("34028235676", 28));
    EXPECT_EQ(3.4028234e+38f, StrtofChar("34028235677", 28));
    EXPECT_EQ(Single::Infinity(), StrtofChar("34028235678", 28));

    // The following number is the result of 89255.0/1e-22. Both floating-point
    // numbers can be accurately represented with doubles. However on Linux,x86
    // the floating-point stack is set to 80bits and the double-rounding
    // introduces an error.
    EXPECT_EQ(89255e-22f, StrtofChar("89255", -22));

    // Boundary cases. Boundaries themselves should round to even.
    //
    // 0x4f012334 = 2166567936
    //      next:   2166568192
    //  boundary:   2166568064 should round down.
    EXPECT_EQ(2166567936.0f, StrtofChar("2166567936", 0));
    EXPECT_EQ(2166568192.0f, StrtofChar("2166568192", 0));
    EXPECT_EQ(2166567936.0f, StrtofChar("2166568064", 0));
    EXPECT_EQ(2166567936.0f, StrtofChar("216656806399999", -5));
    EXPECT_EQ(2166568192.0f, StrtofChar("216656806400001", -5));
    // Verify that we don't double round.
    // Get the boundary of the boundary.
    EXPECT_EQ(2.1665680640000002384185791015625e9, 2166568064.0);
    // Visual Studio gets this wrong and believes that these two numbers are the
    // same doubles. We want to test our conversion and not the compiler. We
    // therefore disable the check.
    EXPECT_TRUE(2.16656806400000023841857910156251e9 != 2166568064.0);
    EXPECT_EQ(2166568192.0f, StrtofChar("21665680640000002384185791015625", -22));

    // 0x4fffffff = 8589934080
    //      next:   8589934592
    //  boundary:   8589934336 should round up.
    EXPECT_EQ(8589934080.0f, StrtofChar("8589934080", 0));
    EXPECT_EQ(8589934592.0f, StrtofChar("8589934592", 0));
    EXPECT_EQ(8589934592.0f, StrtofChar("8589934336", 0));
    EXPECT_EQ(8589934080.0f, StrtofChar("858993433599999", -5));
    EXPECT_EQ(8589934592.0f, StrtofChar("858993433600001", -5));
    // Verify that we don't double round.
    // Get the boundary of the boundary.
    // Visual Studio gets this wrong. To avoid failing tests because of a broken
    // compiler we disable the following two tests. They were only testing the
    // compiler. The real test is still active.
    EXPECT_EQ(8.589934335999999523162841796875e+09, 8589934336.0);
    EXPECT_TRUE(8.5899343359999995231628417968749e+09 != 8589934336.0);

    EXPECT_EQ(8589934080.0f, StrtofChar("8589934335999999523162841796875", -21));

    // 0x4f000000 = 2147483648
    //      next:   2147483904
    //  boundary:   2147483776 should round down.
    EXPECT_EQ(2147483648.0f, StrtofChar("2147483648", 0));
    EXPECT_EQ(2147483904.0f, StrtofChar("2147483904", 0));
    EXPECT_EQ(2147483648.0f, StrtofChar("2147483776", 0));
    EXPECT_EQ(2147483648.0f, StrtofChar("214748377599999", -5));
    EXPECT_EQ(2147483904.0f, StrtofChar("214748377600001", -5));
}

static const int kBufferSize = 1024;
static const int kShortStrtodRandomCount = 2;
static const int kLargeStrtodRandomCount = 2;

TEST(Atod, RandomStrtod)
{
    char buffer[kBufferSize];
    for (size_t length = 1; length < 15; length++)
    {
        for (size_t i = 0; i < kShortStrtodRandomCount; ++i)
        {
            size_t pos = 0;
            for (size_t j = 0; j < length; ++j)
                buffer[pos++] = static_cast<char>(DeterministicRandom() % 10 + '0');

            int exponent = DeterministicRandom() % (25 * 2 + 1) - 25 - length;
            buffer[pos] = '\0';
            ArrayView<char> vector(buffer, pos);
            double strtodResult = internal::StringToDoubleConverter<char>::Strtod(vector, exponent);
            EXPECT_TRUE(CheckDouble(vector, exponent, strtodResult));
        }
    }

    for (size_t length = 15; length < 800; length += 2)
    {
        for (size_t i = 0; i < kLargeStrtodRandomCount; ++i)
        {
            size_t pos = 0;
            for (size_t j = 0; j < length; ++j)
                buffer[pos++] = static_cast<char>(DeterministicRandom() % 10 + '0');

            int exponent = DeterministicRandom() % (308 * 2 + 1) - 308 - length;
            buffer[pos] = '\0';
            ArrayView<char> vector(buffer, pos);
            double strtodResult = internal::StringToDoubleConverter<char>::Strtod(vector, exponent);
            EXPECT_TRUE(CheckDouble(vector, exponent, strtodResult));
        }
    }
}

static const int kShortStrtofRandomCount = 2;
static const int kLargeStrtofRandomCount = 2;

TEST(Atod, RandomStrtof)
{
    char buffer[kBufferSize];
    for (size_t length = 1; length < 15; length++)
    {
        for (size_t i = 0; i < kShortStrtofRandomCount; ++i)
        {
            size_t pos = 0;
            for (size_t j = 0; j < length; ++j)
                buffer[pos++] = static_cast<char>(DeterministicRandom() % 10 + '0');

            int exponent = DeterministicRandom() % (5*2 + 1) - 5 - length;
            buffer[pos] = '\0';
            ArrayView<char> vector(buffer, pos);
            float strtofResult = internal::StringToDoubleConverter<char>::Strtof(vector, exponent);
            EXPECT_TRUE(CheckFloat(vector, exponent, strtofResult));
        }
    }

    for (size_t length = 15; length < 800; length += 2)
    {
        for (size_t i = 0; i < kLargeStrtofRandomCount; ++i)
        {
            size_t pos = 0;
            for (size_t j = 0; j < length; ++j)
                buffer[pos++] = static_cast<char>(DeterministicRandom() % 10 + '0');

            int exponent = DeterministicRandom() % (38*2 + 1) - 38 - length;
            buffer[pos] = '\0';
            ArrayView<char> vector(buffer, pos);
            float strtofResult = internal::StringToDoubleConverter<char>::Strtof(vector, exponent);
            EXPECT_TRUE(CheckFloat(vector, exponent, strtofResult));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
    double StrToD16(const char16_t* str16, size_t length, AtodFlags flags, double emptyStringValue,
        size_t& processedCharactersCount, bool& processedAll)
    {
        StringToDoubleConverter<char16_t> converter(flags, emptyStringValue, Double::Nan(), nullptr, nullptr);
        double result = converter.StringToDouble(str16, length, processedCharactersCount);
        processedAll = (length == processedCharactersCount);
        return result;
    }

    double StrToD(const char* str, AtodFlags flags, double emptyStringValue, size_t& processedCharactersCount,
        bool& processedAll)
    {
        StringToDoubleConverter<char> converter(flags, emptyStringValue, Double::Nan(), nullptr, nullptr);
        double result = converter.StringToDouble(str, strlen(str), processedCharactersCount);
        processedAll = ((strlen(str) == processedCharactersCount));

        char16_t buffer16[256];
        assert(strlen(str) < CountOf(buffer16));
        size_t len = strlen(str);
        for (size_t i = 0; i < len; i++)
            buffer16[i] = static_cast<char16_t>(str[i]);
        
        size_t processedCharactersCount16;
        bool processedAll16;
        double result16 = StrToD16(buffer16, len, flags, emptyStringValue, processedCharactersCount16, processedAll16);
        EXPECT_FALSE(result != result16 && (result == result || result16 == result16));
        EXPECT_EQ(processedCharactersCount, processedCharactersCount16);
        return result;
    }

    float StrToF16(const char16_t* str16, size_t length, AtodFlags flags,
        double emptyStringValue, size_t& processedCharactersCount, bool& processedAll)
    {
        StringToDoubleConverter<char16_t> converter(flags, emptyStringValue, Single::Nan(), nullptr, nullptr);
        double result = converter.StringToFloat(str16, length, processedCharactersCount);
        processedAll = (length == processedCharactersCount);
        return result;
    }

     float StrToF(const char* str, AtodFlags flags, double emptyStringValue, size_t& processedCharactersCount,
         bool& processedAll)
     {
        StringToDoubleConverter<char> converter(flags, emptyStringValue, Single::Nan(), nullptr, nullptr);
        float result = converter.StringToFloat(str, strlen(str), processedCharactersCount);
        processedAll = ((strlen(str) == processedCharactersCount));

        char16_t buffer16[256];
        assert(strlen(str) < CountOf(buffer16));
        size_t len = strlen(str);
        for (size_t i = 0; i < len; i++)
            buffer16[i] = static_cast<char16_t>(str[i]);

        size_t processedCharactersCount16;
        bool processedAll16;
        float result16 = StrToF16(buffer16, len, flags, emptyStringValue, processedCharactersCount16, processedAll16);
        EXPECT_FALSE(result != result16 && (result == result || result16 == result16));
        EXPECT_EQ(processedCharactersCount, processedCharactersCount16);
        return result;
    }
}

TEST(Atod, StringToDoubleVarious)
{
    AtodFlags flags;
    size_t processed;
    bool allUsed;

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowSpacesAfterSign) |
        static_cast<int>(AtodFlags::AllowTrailingSpaces));

    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("  ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("  ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0, StrToD("42", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0, StrToD(" + 42 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-42.0, StrToD(" - 42 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("42x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("42 x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" + 42 x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" - 42 x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowSpacesAfterSign) |
        static_cast<int>(AtodFlags::AllowTrailingSpaces) |
        static_cast<int>(AtodFlags::AllowTrailingJunk));

    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("  ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("  ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0, StrToD("42", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0, StrToD(" + 42 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-42.0, StrToD(" - 42 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(42.0, StrToD("42x", flags, 0.0, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(42.0, StrToD("42 x", flags, 0.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(42.0, StrToD(" + 42 x", flags, 0.0, processed, allUsed));
    EXPECT_EQ(6, processed);

    EXPECT_EQ(-42.0, StrToD(" - 42 x", flags, 0.0, processed, allUsed));
    EXPECT_EQ(6, processed);
    
    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowSpacesAfterSign) |
        static_cast<int>(AtodFlags::AllowTrailingJunk));

    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("  ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("  ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0, StrToD("42", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0, StrToD(" + 42 ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(5, processed);

    EXPECT_EQ(-42.0, StrToD(" - 42 ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(5, processed);

    EXPECT_TRUE(isnan(StrToD("x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(42.0, StrToD("42x", flags, 0.0, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(42.0, StrToD("42 x", flags, 0.0, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(42.0, StrToD(" + 42 x", flags, 0.0, processed, allUsed));
    EXPECT_EQ(5, processed);

    EXPECT_EQ(-42.0, StrToD(" - 42 x", flags, 0.0, processed, allUsed));
    EXPECT_EQ(5, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowTrailingJunk));

    EXPECT_EQ(42.0, StrToD(" +42 ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(4, processed);

    EXPECT_EQ(-42.0, StrToD(" -42 ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(4, processed);

    EXPECT_TRUE(isnan(StrToD(" + 42 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" - 42 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::Default;

    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("  ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("  ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(42.0, StrToD("42", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" + 42 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" - 42 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("42x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("42 x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" + 42 x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" - 42 x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowLeadingSpaces;

    EXPECT_EQ(0.0, StrToD(" ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD(" ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0, StrToD(" 42", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("42 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowTrailingSpaces;

    EXPECT_EQ(0.0, StrToD(" ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD(" ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0, StrToD("42 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" 42", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Atod, StringToDoubleEmptyString)
{
    AtodFlags flags;
    size_t processed;
    bool allUsed;

    flags = AtodFlags::Default;
    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowSpacesAfterSign;
    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowLeadingSpaces;
    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD(" ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD(" ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    flags = AtodFlags::AllowTrailingSpaces;
    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD(" ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD(" ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    flags = AtodFlags::AllowTrailingJunk;
    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Atod, StringToDoubleHexString)
{
    AtodFlags flags;
    size_t processed;
    bool allUsed;

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowHex) |
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowTrailingSpaces) |
        static_cast<int>(AtodFlags::AllowSpacesAfterSign));

    EXPECT_EQ(18.0, StrToD("0x12", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("0x0", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0x123456789), StrToD("0x123456789", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(18.0, StrToD(" 0x12 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD(" 0x0 ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0x123456789), StrToD(" 0x123456789 ", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xabcdef", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xABCDEF", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD(" 0xabcdef ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD(" 0xABCDEF ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("0x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x 3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("0x3g", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("0x3.23", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("x3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("0x3 foo", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x3 foo", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("+ 0x3 foo", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("+", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("-", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(-5.0, StrToD("-0x5", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-5.0, StrToD(" - 0x5 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(5.0, StrToD(" + 0x5 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("- -0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("- +0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("+ +0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowHex;

    EXPECT_EQ(18.0, StrToD("0x12", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("0x0", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0x123456789), StrToD("0x123456789", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" 0x12 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x0 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x123456789 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xabcdef", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xABCDEF", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" 0xABCDEF ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0xABCDEF ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("0x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x 3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("0x3g", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("0x3.23", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("x3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("+ 0x3 foo", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("+", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("-", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(-5.0, StrToD("-0x5", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" - 0x5 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" + 0x5 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("- -0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("- +0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("+ +0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowTrailingJunk) |
        static_cast<int>(AtodFlags::AllowHex));

    EXPECT_EQ(18.0, StrToD("0x12", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("0x0", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0x123456789), StrToD("0x123456789", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" 0x12 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x0 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(18.0, StrToD("0x12 ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(4, processed);

    EXPECT_EQ(0.0, StrToD("0x0 ", flags, 1.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x123456789 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xabcdef", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xABCDEF", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" 0xabcdef ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0xABCDEF ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xabcdef ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xABCDEF ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_TRUE(isnan(StrToD(" 0xabcdef", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0xABCDEF", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("0x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x 3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(3.0, StrToD("0x3g", flags, 0.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(3.0, StrToD("0x3.234", flags, 0.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x3g", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x3.234", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("x3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("+ 0x3 foo", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("+", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("-", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(-5.0, StrToD("-0x5", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" - 0x5 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" + 0x5 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("- -0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("- +0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("+ +0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowTrailingJunk) |
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowTrailingSpaces) |
        static_cast<int>(AtodFlags::AllowSpacesAfterSign) |
        static_cast<int>(AtodFlags::AllowHex));

    EXPECT_EQ(18.0, StrToD("0x12", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("0x0", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0x123456789), StrToD("0x123456789", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(18.0, StrToD(" 0x12 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD(" 0x0 ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0x123456789), StrToD(" 0x123456789 ", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xabcdef", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xABCDEF", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD(" 0xabcdef ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD(" 0xABCDEF ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0xABC), StrToD(" 0xabc def ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(7, processed);

    EXPECT_EQ(static_cast<double>(0xABC), StrToD(" 0xABC DEF ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(7, processed);

    EXPECT_EQ(static_cast<double>(0x12), StrToD(" 0x12 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD(" 0x0 ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0x123456789), StrToD(" 0x123456789 ", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("0x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0x 3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ((double)0x3, StrToD("0x3g", flags, 0.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ((double)0x3, StrToD("0x3.234", flags, 0.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_TRUE(isnan(StrToD("x3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Atod, StringToDoubleOctalString)
{
    AtodFlags flags;
    size_t processed;
    bool allUsed;

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowOctals) |
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowTrailingSpaces) |
        static_cast<int>(AtodFlags::AllowSpacesAfterSign));

    EXPECT_EQ(10.0, StrToD("012", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("00", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD("012", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0, StrToD("0123456789", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD("01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD("+01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0, StrToD("-01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD(" 012", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD("\n012", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD(" 00", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("\t00", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD(" 012", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD("\n012", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0, StrToD(" 0123456789", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD(" 01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD("\n01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD(" + 01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0, StrToD(" - 01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0, StrToD("\n-\t01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD(" 012 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD(" 00 ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD(" 012 ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0, StrToD(" 0123456789 ", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD(" 01234567 ", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD(" + 01234567 ", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0, StrToD(" - 01234567 ", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD("012 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("00 ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD("012 ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0, StrToD("0123456789 ", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD("01234567 ", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD("+01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0, StrToD("-01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("01234567e0", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowOctals;
    EXPECT_EQ(10.0, StrToD("012", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("00", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD("012", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0, StrToD("0123456789", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD("01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD("+01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0, StrToD("-01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" 012", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 00", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 012", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0123456789", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" + 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" - 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 012 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 00 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 012 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0123456789 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" + 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" - 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("012 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("00 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("012 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("0123456789 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(342391.0, StrToD("+01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0, StrToD("-01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("01234567e0", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowOctals) |
        static_cast<int>(AtodFlags::AllowTrailingJunk));
    EXPECT_EQ(10.0, StrToD("012", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("00", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD("012", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0, StrToD("0123456789", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD("01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD("+01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0, StrToD("-01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" 012", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 00", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 012", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0123456789", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" + 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" - 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 012 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 00 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 012 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0123456789 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" + 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" - 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(10.0, StrToD("012 ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(0.0, StrToD("00 ", flags, 1.0, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(123456789.0, StrToD("0123456789 ", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(10, processed);

    EXPECT_EQ(342391.0, StrToD("01234567 ", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(342391.0, StrToD("+01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0, StrToD("-01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD("012foo ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(0.0, StrToD("00foo ", flags, 1.0, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(123456789.0, StrToD("0123456789foo ", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(10, processed);

    EXPECT_EQ(342391.0, StrToD("01234567foo ", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(342391.0, StrToD("+01234567foo", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(-342391.0, StrToD("-01234567foo", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(10.0, StrToD("012 foo ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(0.0, StrToD("00 foo ", flags, 1.0, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(123456789.0, StrToD("0123456789 foo ", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(10, processed);

    EXPECT_EQ(342391.0, StrToD("01234567 foo ", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(342391.0, StrToD("+01234567 foo", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(-342391.0, StrToD("-01234567 foo", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(342391.0, StrToD("01234567e0", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(342391.0, StrToD("01234567e", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(8, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowOctals) |
        static_cast<int>(AtodFlags::AllowTrailingSpaces) |
        static_cast<int>(AtodFlags::AllowTrailingJunk));
    EXPECT_EQ(10.0, StrToD("012", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("00", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD("012", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0, StrToD("0123456789", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD("01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD("+01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0, StrToD("-01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" 012", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 00", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 012", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0123456789", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" + 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" - 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 012 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 00 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 012 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 0123456789 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" + 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD(" - 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(10.0, StrToD("012 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("00 ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0, StrToD("0123456789 ", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD("01234567 ", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0, StrToD("+01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0, StrToD("-01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0, StrToD("012foo ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(0.0, StrToD("00foo ", flags, 1.0, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(123456789.0, StrToD("0123456789foo ", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(10, processed);

    EXPECT_EQ(342391.0, StrToD("01234567foo ", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(342391.0, StrToD("+01234567foo", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(-342391.0, StrToD("-01234567foo", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(10.0, StrToD("012 foo ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(4, processed);

    EXPECT_EQ(0.0, StrToD("00 foo ", flags, 1.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(123456789.0, StrToD("0123456789 foo ", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(11, processed);

    EXPECT_EQ(342391.0, StrToD("01234567 foo ", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(342391.0, StrToD("+01234567 foo", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(10, processed);

    EXPECT_EQ(-342391.0, StrToD("-01234567 foo", flags, Double::Nan(), processed, allUsed));
    EXPECT_EQ(10, processed);
}

TEST(Atod, StringToDoubleSpecialValues)
{
    size_t processed;
    AtodFlags flags = AtodFlags::Default;

    {
        // Use 1.0 as junk_string_value.
        StringToDoubleConverter<char> converter(flags, 0.0, 1.0, "infinity", "NaN");

        EXPECT_TRUE(isnan(converter.StringToDouble("+NaN", 4, processed)));
        EXPECT_EQ(4, processed);

        EXPECT_EQ(-Double::Infinity(), converter.StringToDouble("-infinity", 9, processed));
        EXPECT_EQ(9, processed);

        EXPECT_EQ(1.0, converter.StringToDouble("Infinity", 8, processed));
        EXPECT_EQ(0, processed);

        EXPECT_EQ(1.0, converter.StringToDouble("++NaN", 5, processed));
        EXPECT_EQ(0, processed);
    }

    {
        // Use 1.0 as junk_string_value.
        StringToDoubleConverter<char> converter(flags, 0.0, 1.0, "+infinity", "1NaN");

        // The '+' is consumed before trying to match the infinity string.
        EXPECT_EQ(1.0, converter.StringToDouble("+infinity", 9, processed));
        EXPECT_EQ(0, processed);

        // The match for "1NaN" triggers, and doesn't let the 1234.0 complete.
        EXPECT_EQ(1.0, converter.StringToDouble("1234.0", 6, processed));
        EXPECT_EQ(0, processed);
    }
}

TEST(Atod, StringToDoubleCommentExamples)
{
    // Make sure the examples in the comments are correct.
    AtodFlags flags;
    size_t processed;
    bool allUsed;

    flags = AtodFlags::AllowHex;

    EXPECT_EQ(4660.0, StrToD("0x1234", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("0x1234.56", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(flags) |
        static_cast<int>(AtodFlags::AllowTrailingJunk));
    EXPECT_EQ(4660.0, StrToD("0x1234.56", flags, 0.0, processed, allUsed));
    EXPECT_EQ(6, processed);

    flags = AtodFlags::AllowOctals;
    EXPECT_EQ(668.0, StrToD("01234", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(12349.0, StrToD("012349", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("01234.56", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(processed, 0);

    flags = static_cast<AtodFlags>(
        static_cast<int>(flags) |
        static_cast<int>(AtodFlags::AllowTrailingJunk));
    EXPECT_EQ(668.0, StrToD("01234.56", flags, 0.0, processed, allUsed));
    EXPECT_EQ(processed, 5);

    flags  = AtodFlags::AllowSpacesAfterSign;
    EXPECT_EQ(-123.2, StrToD("-   123.2", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    flags  = AtodFlags::AllowSpacesAfterSign;
    EXPECT_EQ(123.2, StrToD("+   123.2", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowHex) |
        static_cast<int>(AtodFlags::AllowTrailingJunk));

    EXPECT_EQ(4660.0, StrToD("0x1234", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(4660.0, StrToD("0x1234K", flags, 0.0, processed, allUsed));
    EXPECT_EQ(processed, 6);

    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD(" ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(processed, 0);

    EXPECT_TRUE(isnan(StrToD(" 1", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(processed, 0);

    EXPECT_TRUE(isnan(StrToD("0x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(processed, 0);

    EXPECT_EQ(-123.45, StrToD("-123.45", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("--123.45", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(processed, 0);

    EXPECT_EQ(123e45, StrToD("123e45", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123e45, StrToD("123E45", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123e45, StrToD("123e+45", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123e-45, StrToD("123e-45", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123.0, StrToD("123e", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123.0, StrToD("123e-", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    {
        StringToDoubleConverter<char> converter(flags, 0.0, 1.0, "infinity", "NaN");
        EXPECT_TRUE(isnan(converter.StringToDouble("+NaN", 4, processed)));
        EXPECT_EQ(4, processed);

        EXPECT_EQ(-Double::Infinity(), converter.StringToDouble("-infinity", 9, processed));
        EXPECT_EQ(9, processed);

        EXPECT_EQ(1.0, converter.StringToDouble("Infinity", 9, processed));
        EXPECT_EQ(0, processed);
    }

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowOctals) |
        static_cast<int>(AtodFlags::AllowLeadingSpaces));

    EXPECT_TRUE(isnan(StrToD("0x1234", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(668.0, StrToD("01234", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD(" ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD(" 1", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("0x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("0123e45", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(1239e45, StrToD("01239e45", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToD("-infinity", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToD("NaN", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Atod, StringToFloatVarious)
{
    AtodFlags flags;
    size_t processed;
    bool allUsed;

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowSpacesAfterSign) |
        static_cast<int>(AtodFlags::AllowTrailingSpaces));

    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("  ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("  ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0f, StrToF("42", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0f, StrToF(" + 42 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-42.0f, StrToF(" - 42 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("42x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("42 x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" + 42 x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" - 42 x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowSpacesAfterSign) |
        static_cast<int>(AtodFlags::AllowTrailingSpaces) |
        static_cast<int>(AtodFlags::AllowTrailingJunk));

    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("  ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("  ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0f, StrToF("42", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0f, StrToF(" + 42 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-42.0f, StrToF(" - 42 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(42.0f, StrToF("42x", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(42.0f, StrToF("42 x", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(42.0f, StrToF(" + 42 x", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(6, processed);

    EXPECT_EQ(-42.0f, StrToF(" - 42 x", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(6, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowSpacesAfterSign) |
        static_cast<int>(AtodFlags::AllowTrailingJunk));

    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("  ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("  ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0f, StrToF("42", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0f, StrToF(" + 42 ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(5, processed);

    EXPECT_EQ(-42.0f, StrToF(" - 42 ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(5, processed);

    EXPECT_TRUE(isnan(StrToF("x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(42.0f, StrToF("42x", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(42.0f, StrToF("42 x", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(42.0f, StrToF(" + 42 x", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(5, processed);

    EXPECT_EQ(-42.0f, StrToF(" - 42 x", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(5, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowTrailingJunk));

    EXPECT_EQ(42.0f, StrToF(" +42 ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(4, processed);

    EXPECT_EQ(-42.0f, StrToF(" -42 ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(4, processed);

    EXPECT_TRUE(isnan(StrToF(" + 42 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" - 42 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::Default;

    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("  ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("  ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(42.0f, StrToF("42", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" + 42 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" - 42 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("42x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("42 x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" + 42 x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" - 42 x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowLeadingSpaces;

    EXPECT_EQ(0.0f, StrToF(" ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF(" ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0f, StrToF(" 42", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("42 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowTrailingSpaces;

    EXPECT_EQ(0.0f, StrToF(" ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF(" ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0f, StrToF("42 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" 42", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Atod, StringToFloatEmptyString)
{
    AtodFlags flags;
    size_t processed;
    bool allUsed;

    flags = AtodFlags::Default;
    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowSpacesAfterSign;
    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowLeadingSpaces;
    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF(" ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF(" ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    flags = AtodFlags::AllowTrailingSpaces;
    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF(" ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF(" ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    flags = AtodFlags::AllowTrailingJunk;
    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Atod, StringToFloatHexString)
{
    AtodFlags flags;
    size_t processed;
    bool allUsed;
    double d;
    float f;

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowHex) |
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowTrailingSpaces) |
        static_cast<int>(AtodFlags::AllowSpacesAfterSign));

    // Check that no double rounding occurs:
    const char* doubleRoundingExample1 = "0x100000100000008";
    d = StrToD(doubleRoundingExample1, flags, 0.0, processed, allUsed);
    f = StrToF(doubleRoundingExample1, flags, 0.0f, processed, allUsed);
    EXPECT_TRUE(f != static_cast<float>(d));
    EXPECT_EQ(72057602627862528.0f, StrToF(doubleRoundingExample1, flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    const char* doubleRoundingExample2 = "0x1000002FFFFFFF8";
    d = StrToD(doubleRoundingExample2, flags, 0.0, processed, allUsed);
    f = StrToF(doubleRoundingExample2, flags, 0.0f, processed, allUsed);
    EXPECT_TRUE(f != static_cast<float>(d));
    EXPECT_EQ(72057602627862528.0f, StrToF(doubleRoundingExample2, flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(18.0f, StrToF("0x12", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("0x0", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0x123456789), StrToF("0x123456789", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(18.0f, StrToF(" 0x12 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF(" 0x0 ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0x123456789), StrToF(" 0x123456789 ", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xabcdef", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xABCDEF", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF(" 0xabcdef ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF(" 0xABCDEF ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("0x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x 3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("0x3g", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("0x3.23", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("x3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("0x3 foo", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x3 foo", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("+ 0x3 foo", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("+", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("-", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(-5.0f, StrToF("-0x5", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-5.0f, StrToF(" - 0x5 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(5.0f, StrToF(" + 0x5 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("- -0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("- +0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("+ +0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowHex;

    EXPECT_EQ(18.0f, StrToF("0x12", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("0x0", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0x123456789), StrToF("0x123456789", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" 0x12 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x0 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x123456789 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xabcdef", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xABCDEF", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" 0xabcdef ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0xABCDEF ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("0x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x 3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("0x3g", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("0x3.23", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("x3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("+ 0x3 foo", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("+", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("-", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(-5.0f, StrToF("-0x5", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" - 0x5 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" + 0x5 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("- -0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("- +0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("+ +0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowTrailingJunk) |
        static_cast<int>(AtodFlags::AllowHex));

    EXPECT_EQ(18.0f, StrToF("0x12", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("0x0", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0x123456789), StrToF("0x123456789", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" 0x12 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x0 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(18.0f, StrToF("0x12 ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(4, processed);

    EXPECT_EQ(0.0f, StrToF("0x0 ", flags, 1.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x123456789 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xabcdef", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xABCDEF", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" 0xabcdef ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0xABCDEF ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xabcdef ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xABCDEF ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_TRUE(isnan(StrToF(" 0xabcdef", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0xABCDEF", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("0x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x 3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(3.0f, StrToF("0x3g", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(3.0f, StrToF("0x3.234", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x3g", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x3.234", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("x3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("+ 0x3 foo", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("+", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("-", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(-5.0f, StrToF("-0x5", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" - 0x5 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" + 0x5 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("- -0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("- +0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("+ +0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowTrailingJunk) |
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowTrailingSpaces) |
        static_cast<int>(AtodFlags::AllowSpacesAfterSign) |
        static_cast<int>(AtodFlags::AllowHex));

    EXPECT_EQ(18.0f, StrToF("0x12", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("0x0", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0x123456789), StrToF("0x123456789", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(18.0f, StrToF(" 0x12 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF(" 0x0 ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0x123456789), StrToF(" 0x123456789 ", flags, Single::Nan(),
            processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xabcdef", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xABCDEF", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF(" 0xabcdef ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0xabcdef), StrToF(" 0xABCDEF ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0xabc), StrToF(" 0xabc def ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(7, processed);

    EXPECT_EQ(static_cast<float>(0xabc), StrToF(" 0xABC DEF ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(7, processed);

    EXPECT_EQ(static_cast<float>(0x12), StrToF(" 0x12 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF(" 0x0 ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0x123456789), StrToF(" 0x123456789 ", flags, Single::Nan(),
            processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("0x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0x 3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ((float)0x3, StrToF("0x3g", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ((float)0x3, StrToF("0x3.234", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_TRUE(isnan(StrToF("x3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Atod, StringToFloatOctalString)
{
    AtodFlags flags;
    size_t processed;
    bool allUsed;
    double d;
    float f;

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowOctals) |
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowTrailingSpaces) |
        static_cast<int>(AtodFlags::AllowSpacesAfterSign));

    // Check that no double rounding occurs:
    const char* doubleRoundingExample1 = "04000000040000000010";
    d = StrToD(doubleRoundingExample1, flags, 0.0, processed, allUsed);
    f = StrToF(doubleRoundingExample1, flags, 0.0f, processed, allUsed);
    EXPECT_TRUE(f != static_cast<float>(d));
    EXPECT_EQ(72057602627862528.0f, StrToF(doubleRoundingExample1, flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    const char* doubleRoundingExample2 = "04000000137777777770";
    d = StrToD(doubleRoundingExample2, flags, 0.0, processed, allUsed);
    f = StrToF(doubleRoundingExample2, flags, 0.0f, processed, allUsed);
    EXPECT_TRUE(f != static_cast<float>(d));
    EXPECT_EQ(72057602627862528.0f, StrToF(doubleRoundingExample2, flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0f, StrToF("012", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("00", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0f, StrToF("012", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0f, StrToF("0123456789", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF("01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF("+01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0f, StrToF("-01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0f, StrToF(" 012", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF(" 00", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0f, StrToF(" 012", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0f, StrToF(" 0123456789", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF(" 01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF(" + 01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0f, StrToF(" - 01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0f, StrToF(" 012 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF(" 00 ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0f, StrToF(" 012 ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0f, StrToF(" 0123456789 ", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF(" 01234567 ", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF(" + 01234567 ", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0f, StrToF(" - 01234567 ", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0f, StrToF("012 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("00 ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0f, StrToF("012 ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0f, StrToF("0123456789 ", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF("01234567 ", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF("+01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0f, StrToF("-01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("01234567e0", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowOctals;
    EXPECT_EQ(10.0f, StrToF("012", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("00", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0f, StrToF("012", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0f, StrToF("0123456789", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF("01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF("+01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0f, StrToF("-01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" 012", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 00", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 012", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0123456789", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" + 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" - 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 012 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 00 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 012 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0123456789 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" + 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" - 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("012 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("00 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("012 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("0123456789 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF("01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(342391.0f, StrToF("+01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0f, StrToF("-01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF("01234567e0", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowOctals) |
        static_cast<int>(AtodFlags::AllowTrailingJunk));
    EXPECT_EQ(10.0f, StrToF("012", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("00", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0f, StrToF("012", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0f, StrToF("0123456789", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF("01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF("+01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0f, StrToF("-01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" 012", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 00", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 012", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0123456789", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" + 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" - 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 012 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 00 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 012 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0123456789 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" + 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" - 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(10.0f, StrToF("012 ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(0.0f, StrToF("00 ", flags, 1.0f, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(123456789.0f, StrToF("0123456789 ", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(10, processed);

    EXPECT_EQ(342391.0f, StrToF("01234567 ", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(342391.0f, StrToF("+01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0f, StrToF("-01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0f, StrToF("012foo ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(0.0f, StrToF("00foo ", flags, 1.0f, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(123456789.0f, StrToF("0123456789foo ", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(10, processed);

    EXPECT_EQ(342391.0f, StrToF("01234567foo ", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(342391.0f, StrToF("+01234567foo", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(-342391.0f, StrToF("-01234567foo", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(10.0f, StrToF("012 foo ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(0.0f, StrToF("00 foo ", flags, 1.0f, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(123456789.0f, StrToF("0123456789 foo ", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(10, processed);

    EXPECT_EQ(342391.0f, StrToF("01234567 foo ", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(342391.0f, StrToF("+01234567 foo", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(-342391.0f, StrToF("-01234567 foo", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(342391.0f, StrToF("01234567e0", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(342391.0f, StrToF("01234567e", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(8, processed);

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowOctals) |
        static_cast<int>(AtodFlags::AllowTrailingSpaces) |
        static_cast<int>(AtodFlags::AllowTrailingJunk));
    EXPECT_EQ(10.0f, StrToF("012", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("00", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0f, StrToF("012", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0f, StrToF("0123456789", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF("01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF("+01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0f, StrToF("-01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(isnan(StrToF(" 012", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 00", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 012", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0123456789", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" + 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" - 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 012 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 00 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 012 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 0123456789 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" + 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(isnan(StrToF(" - 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(10.0f, StrToF("012 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("00 ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(123456789.0f, StrToF("0123456789 ", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF("01234567 ", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(342391.0f, StrToF("+01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0f, StrToF("-01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(10.0f, StrToF("012foo ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(0.0f, StrToF("00foo ", flags, 1.0f, processed, allUsed));
    EXPECT_EQ(2, processed);

    EXPECT_EQ(123456789.0f, StrToF("0123456789foo ", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(10, processed);

    EXPECT_EQ(342391.0f, StrToF("01234567foo ", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(342391.0f, StrToF("+01234567foo", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(-342391.0f, StrToF("-01234567foo", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(10.0f, StrToF("012 foo ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(4, processed);

    EXPECT_EQ(0.0f, StrToF("00 foo ", flags, 1.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(123456789.0f, StrToF("0123456789 foo ", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(11, processed);

    EXPECT_EQ(342391.0f, StrToF("01234567 foo ", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(9, processed);

    EXPECT_EQ(342391.0f, StrToF("+01234567 foo", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(10, processed);

    EXPECT_EQ(-342391.0f, StrToF("-01234567 foo", flags, Single::Nan(), processed, allUsed));
    EXPECT_EQ(10, processed);
}

TEST(Atod, StringToFloatSpecialValues)
{
    size_t processed;
    AtodFlags flags = AtodFlags::Default;

    {
        // Use 1.0 as junk_string_value.
        StringToDoubleConverter<char> converter(flags, 0.0f, 1.0f, "infinity", "NaN");

        EXPECT_TRUE(isnan(converter.StringToDouble("+NaN", 4, processed)));
        EXPECT_EQ(4, processed);

        EXPECT_EQ(-Single::Infinity(), converter.StringToDouble("-infinity", 9, processed));
        EXPECT_EQ(9, processed);

        EXPECT_EQ(1.0f, converter.StringToDouble("Infinity", 8, processed));
        EXPECT_EQ(0, processed);

        EXPECT_EQ(1.0f, converter.StringToDouble("++NaN", 5, processed));
        EXPECT_EQ(0, processed);
    }

    {
        // Use 1.0 as junk_string_value.
        StringToDoubleConverter<char> converter(flags, 0.0f, 1.0f, "+infinity", "1NaN");

        // The '+' is consumed before trying to match the infinity string.
        EXPECT_EQ(1.0f, converter.StringToDouble("+infinity", 9, processed));
        EXPECT_EQ(0, processed);

        // The match for "1NaN" triggers, and doesn't let the 1234.0 complete.
        EXPECT_EQ(1.0f, converter.StringToDouble("1234.0", 6, processed));
        EXPECT_EQ(0, processed);
    }
}

TEST(Atod, StringToDoubleFloatWhitespace)
{
    AtodFlags flags;
    size_t processed;
    bool allUsed;

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowLeadingSpaces) |
        static_cast<int>(AtodFlags::AllowTrailingSpaces) |
        static_cast<int>(AtodFlags::AllowSpacesAfterSign));

    const char kWhitespaceAscii[] = {
        0x0A, 0x0D, 0x09, 0x0B, 0x0C, 0x20,
        '-',
        0x0A, 0x0D, 0x09, 0x0B, 0x0C, 0x20,
        '1', '.', '2',
        0x0A, 0x0D, 0x09, 0x0B, 0x0C, 0x20,
        0x00
    };
    EXPECT_EQ(-1.2, StrToD(kWhitespaceAscii, flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);
    EXPECT_EQ(-1.2f, StrToF(kWhitespaceAscii, flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    const char16_t kOghamSpaceMark = 0x1680;
    const char16_t kMongolianVowelSeparator = 0x180E;
    const char16_t kEnQuad = 0x2000;
    const char16_t kEmQuad = 0x2001;
    const char16_t kEnSpace = 0x2002;
    const char16_t kEmSpace = 0x2003;
    const char16_t kThreePerEmSpace = 0x2004;
    const char16_t kFourPerEmSpace = 0x2005;
    const char16_t kSixPerEmSpace = 0x2006;
    const char16_t kFigureSpace = 0x2007;
    const char16_t kPunctuationSpace = 0x2008;
    const char16_t kThinSpace = 0x2009;
    const char16_t kHairSpace = 0x200A;
    const char16_t kNarrowNoBreakSpace = 0x202F;
    const char16_t kMediumMathematicalSpace = 0x205F;
    const char16_t kIdeographicSpace = 0x3000;

    const char16_t kWhitespace16[] = {
        0x0A, 0x0D, 0x09, 0x0B, 0x0C, 0x20, 0xA0, 0xFEFF,
        kOghamSpaceMark, kMongolianVowelSeparator, kEnQuad, kEmQuad,
        kEnSpace, kEmSpace, kThreePerEmSpace, kFourPerEmSpace, kSixPerEmSpace,
        kFigureSpace, kPunctuationSpace, kThinSpace, kHairSpace,
        kNarrowNoBreakSpace, kMediumMathematicalSpace, kIdeographicSpace,
        '-',
        0x0A, 0x0D, 0x09, 0x0B, 0x0C, 0x20, 0xA0, 0xFEFF,
        kOghamSpaceMark, kMongolianVowelSeparator, kEnQuad, kEmQuad,
        kEnSpace, kEmSpace, kThreePerEmSpace, kFourPerEmSpace, kSixPerEmSpace,
        kFigureSpace, kPunctuationSpace, kThinSpace, kHairSpace,
        kNarrowNoBreakSpace, kMediumMathematicalSpace, kIdeographicSpace,
        '1', '.', '2',
        0x0A, 0x0D, 0x09, 0x0B, 0x0C, 0x20, 0xA0, 0xFEFF,
        kOghamSpaceMark, kMongolianVowelSeparator, kEnQuad, kEmQuad,
        kEnSpace, kEmSpace, kThreePerEmSpace, kFourPerEmSpace, kSixPerEmSpace,
        kFigureSpace, kPunctuationSpace, kThinSpace, kHairSpace,
        kNarrowNoBreakSpace, kMediumMathematicalSpace, kIdeographicSpace,
    };
    const int kWhitespace16Length = CountOf(kWhitespace16);
    EXPECT_EQ(-1.2, StrToD16(kWhitespace16, kWhitespace16Length, flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);
    EXPECT_EQ(-1.2f, StrToF16(kWhitespace16, kWhitespace16Length, flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);
}
