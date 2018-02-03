/**
 * @file
 * @date 2017/5/19
 */
#include <gtest/gtest.h>

#include <Moe.Core/Convert.hpp>

#include <cmath>

#include "Data/DtoaPrecomputedShortest.hpp"
#include "Data/DtoaPrecomputedShortestSingle.hpp"
#include "Data/DtoaPrecomputedPrecision.hpp"
#include "Data/DtoaPrecomputedFixedRepresentations.hpp"

using namespace std;
using namespace moe;
using namespace Convert;
using namespace details;

static const int kBufferSize = 1024;

namespace
{
    template <typename T>
    inline void TrimRepresentation(MutableArrayView<T>& representation)
    {
        size_t i;
        size_t len = strlen(representation.GetBuffer());
        if (len == 0)
            return;

        for (i = len; i-- > 0;)
        {
            if (representation[i] != '0')
                break;
        }

        if (representation[i] == '0')
            representation[i] = '\0';
        else
            representation[i + 1] = '\0';
    }

    template <typename T = char>
    inline void AssignHexString(Bignum& bignum, const T* str)
    {
        bignum.AssignHexString(ArrayView<T>(str, ::strlen(str)));
    }

    template <typename T = char>
    inline void AssignDecimalString(Bignum& bignum, const T* str)
    {
        bignum.AssignDecimalString(ArrayView<T>(str, ::strlen(str)));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(Convert, DiyFpSubtract)
{
    DiyFp fp1 = DiyFp(3, 0);
    DiyFp fp2 = DiyFp(1, 0);

    DiyFp diff = DiyFp::Minus(fp1, fp2);
    EXPECT_EQ(2, diff.Significand());
    EXPECT_EQ(0, diff.Exponent());

    fp1.Subtract(fp2);
    EXPECT_EQ(2, fp1.Significand());
    EXPECT_EQ(0, fp1.Exponent());
}

TEST(Convert, DiyFpMultiply)
{
    DiyFp fp1 = DiyFp(3, 0);
    DiyFp fp2 = DiyFp(2, 0);

    DiyFp product = DiyFp::Times(fp1, fp2);
    EXPECT_EQ(0, product.Significand());
    EXPECT_EQ(64, product.Exponent());

    fp1.Multiply(fp2);
    EXPECT_EQ(0, fp1.Significand());
    EXPECT_EQ(64, fp1.Exponent());

    fp1 = DiyFp(0x8000000000000000ull, 11);
    fp2 = DiyFp(2, 13);
    product = DiyFp::Times(fp1, fp2);
    EXPECT_EQ(1, product.Significand());
    EXPECT_EQ(11 + 13 + 64, product.Exponent());

    fp1 = DiyFp(0x8000000000000001ull, 11);
    fp2 = DiyFp(1, 13);
    product = DiyFp::Times(fp1, fp2);
    EXPECT_EQ(1, product.Significand());
    EXPECT_EQ(11 + 13 + 64, product.Exponent());

    fp1 = DiyFp(0x7FFFFFFFFFFFFFFF, 11);
    fp2 = DiyFp(1, 13);
    product = DiyFp::Times(fp1, fp2);
    EXPECT_EQ(0, product.Significand());
    EXPECT_EQ(11 + 13 + 64, product.Exponent());

    fp1 = DiyFp(0xFFFFFFFFFFFFFFFFull, 11);
    fp2 = DiyFp(0xFFFFFFFFFFFFFFFFull, 13);
    product = DiyFp::Times(fp1, fp2);
    EXPECT_EQ(0xFFFFFFFFFFFFFFFEull, product.Significand());
    EXPECT_EQ(11 + 13 + 64, product.Exponent());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(Convert, DiyFpUint64Conversions)
{
    uint64_t ordered = 0x0123456789ABCDEFull;
    EXPECT_EQ(3512700564088504e-318, Double(ordered).ToDouble());

    uint64_t minDouble64 = 0x0000000000000001ull;
    EXPECT_EQ(5e-324, Double(minDouble64).ToDouble());

    uint64_t maxDouble64 = 0X7FEFFFFFFFFFFFFFull;
    EXPECT_EQ(1.7976931348623157e308, Double(maxDouble64).ToDouble());
}

TEST(Convert, DiyFpUint32Conversions)
{
    uint32_t ordered = 0x01234567;
    EXPECT_EQ(2.9988165487136453e-38f, Single(ordered).ToFloat());

    uint32_t minFloat32 = 0x00000001;
    EXPECT_EQ(1.4e-45f, Single(minFloat32).ToFloat());

    uint32_t maxFloat32 = 0X7F7FFFFF;
    EXPECT_EQ(3.4028234e38f, Single(maxFloat32).ToFloat());
}

TEST(Convert, DiyFpDoubleToDiyFp)
{
    uint64_t ordered = 0x0123456789ABCDEFull;
    DiyFp fp = Double(ordered).ToDiyFp();
    EXPECT_EQ(0x12 - 0x3FF - 52, fp.Exponent());
    EXPECT_EQ(0x0013456789ABCDEFull, fp.Significand());

    uint64_t minDouble64 = 0x0000000000000001ull;
    fp = Double(minDouble64).ToDiyFp();
    EXPECT_EQ(-0x3FF - 52 + 1, fp.Exponent());

    EXPECT_EQ(1, fp.Significand());

    uint64_t maxDouble64 = 0x7FEFFFFFFFFFFFFFull;
    fp = Double(maxDouble64).ToDiyFp();
    EXPECT_EQ(0x7FE - 0x3FF - 52, fp.Exponent());
    EXPECT_EQ(0x001FFFFFFFFFFFFFull, fp.Significand());
}

TEST(Convert, DiyFpSingleToDiyFp)
{
    uint32_t ordered = 0x01234567;
    DiyFp fp = Single(ordered).ToDiyFp();
    EXPECT_EQ(0x2 - 0x7F - 23, fp.Exponent());
    EXPECT_EQ(0xA34567, fp.Significand());

    uint32_t minFloat32 = 0x00000001;
    fp = Single(minFloat32).ToDiyFp();
    EXPECT_EQ(-0x7F - 23 + 1, fp.Exponent());
    EXPECT_EQ(1, fp.Significand());

    uint32_t maxFloat32 = 0X7F7FFFFF;
    fp = Single(maxFloat32).ToDiyFp();
    EXPECT_EQ(0xFE - 0x7F - 23, fp.Exponent());
    EXPECT_EQ(0X00FFFFFF, fp.Significand());
}

TEST(Convert, DiyFpToNormalizedDiyFp)
{
    uint64_t ordered = 0x0123456789ABCDEFull;
    DiyFp fp = Double(ordered).ToNormalizedDiyFp();
    EXPECT_EQ(0x12 - 0x3FF - 52 - 11, fp.Exponent());
    EXPECT_EQ((0x0013456789ABCDEFull << 11), fp.Significand());

    uint64_t minDouble64 = 0x0000000000000001ull;
    fp = Double(minDouble64).ToNormalizedDiyFp();
    EXPECT_EQ(-0x3FF - 52 + 1 - 63, fp.Exponent());
    EXPECT_EQ(0x8000000000000000ull, fp.Significand());

    uint64_t maxDouble64 = 0X7FEFFFFFFFFFFFFFull;
    fp = Double(maxDouble64).ToNormalizedDiyFp();
    EXPECT_EQ(0x7FE - 0x3FF - 52 - 11, fp.Exponent());
    EXPECT_EQ((0x001FFFFFFFFFFFFFull << 11), fp.Significand());
}

TEST(Convert, DiyFpDoubleIsDenormal)
{
    uint64_t minDouble64 = 0x0000000000000001ull;
    EXPECT_TRUE(Double(minDouble64).IsDenormal());

    uint64_t bits = 0x000FFFFFFFFFFFFFull;
    EXPECT_TRUE(Double(bits).IsDenormal());

    bits = 0x0010000000000000ull;
    EXPECT_TRUE(!Double(bits).IsDenormal());
}

TEST(Convert, DiyFpSingleIsDenormal)
{
    uint32_t minFloat32 = 0x00000001;
    EXPECT_TRUE(Single(minFloat32).IsDenormal());

    uint32_t bits = 0x007FFFFF;
    EXPECT_TRUE(Single(bits).IsDenormal());

    bits = 0x00800000;
    EXPECT_TRUE(!Single(bits).IsDenormal());
}

TEST(Convert, DiyFpDoubleIsSpecial)
{
    EXPECT_TRUE(Double(Double::Infinity()).IsSpecial());
    EXPECT_TRUE(Double(-Double::Infinity()).IsSpecial());
    EXPECT_TRUE(Double(Double::Nan()).IsSpecial());

    uint64_t bits = 0xFFF1234500000000ull;
    EXPECT_TRUE(Double(bits).IsSpecial());

    EXPECT_TRUE(!Double(5e-324).IsSpecial());
    EXPECT_TRUE(!Double(-5e-324).IsSpecial());
    EXPECT_TRUE(!Double(0.0).IsSpecial());
    EXPECT_TRUE(!Double(-0.0).IsSpecial());
    EXPECT_TRUE(!Double(1.0).IsSpecial());
    EXPECT_TRUE(!Double(-1.0).IsSpecial());
    EXPECT_TRUE(!Double(1000000.0).IsSpecial());
    EXPECT_TRUE(!Double(-1000000.0).IsSpecial());
    EXPECT_TRUE(!Double(1e23).IsSpecial());
    EXPECT_TRUE(!Double(-1e23).IsSpecial());
    EXPECT_TRUE(!Double(1.7976931348623157e308).IsSpecial());
    EXPECT_TRUE(!Double(-1.7976931348623157e308).IsSpecial());
}

TEST(Convert, DiyFpSingleIsSpecial)
{
    EXPECT_TRUE(Single(Single::Infinity()).IsSpecial());
    EXPECT_TRUE(Single(-Single::Infinity()).IsSpecial());
    EXPECT_TRUE(Single(Single::Nan()).IsSpecial());

    uint32_t bits = 0xFFF12345;
    EXPECT_TRUE(Single(bits).IsSpecial());

    EXPECT_TRUE(!Single(1.4e-45f).IsSpecial());
    EXPECT_TRUE(!Single(-1.4e-45f).IsSpecial());
    EXPECT_TRUE(!Single(0.0f).IsSpecial());
    EXPECT_TRUE(!Single(-0.0f).IsSpecial());
    EXPECT_TRUE(!Single(1.0f).IsSpecial());
    EXPECT_TRUE(!Single(-1.0f).IsSpecial());
    EXPECT_TRUE(!Single(1000000.0f).IsSpecial());
    EXPECT_TRUE(!Single(-1000000.0f).IsSpecial());
    EXPECT_TRUE(!Single(1e23f).IsSpecial());
    EXPECT_TRUE(!Single(-1e23f).IsSpecial());
    EXPECT_TRUE(!Single(1.18e-38f).IsSpecial());
    EXPECT_TRUE(!Single(-1.18e-38f).IsSpecial());
}

TEST(Convert, DiyFpDoubleIsInfinite)
{
    EXPECT_TRUE(Double(Double::Infinity()).IsInfinite());
    EXPECT_TRUE(Double(-Double::Infinity()).IsInfinite());
    EXPECT_TRUE(!Double(Double::Nan()).IsInfinite());
    EXPECT_TRUE(!Double(0.0).IsInfinite());
    EXPECT_TRUE(!Double(-0.0).IsInfinite());
    EXPECT_TRUE(!Double(1.0).IsInfinite());
    EXPECT_TRUE(!Double(-1.0).IsInfinite());

    uint64_t minDouble64 = 0x0000000000000001ull;
    EXPECT_TRUE(!Double(minDouble64).IsInfinite());
}


TEST(Convert, DiyFpSingleIsInfinite)
{
    EXPECT_TRUE(Single(Single::Infinity()).IsInfinite());
    EXPECT_TRUE(Single(-Single::Infinity()).IsInfinite());
    EXPECT_TRUE(!Single(Single::Nan()).IsInfinite());
    EXPECT_TRUE(!Single(0.0f).IsInfinite());
    EXPECT_TRUE(!Single(-0.0f).IsInfinite());
    EXPECT_TRUE(!Single(1.0f).IsInfinite());
    EXPECT_TRUE(!Single(-1.0f).IsInfinite());

    uint32_t minFloat32 = 0x00000001;
    EXPECT_TRUE(!Single(minFloat32).IsInfinite());
}

TEST(Convert, DiyFpDoubleIsNan)
{
    EXPECT_TRUE(Double(Double::Nan()).IsNan());

    uint64_t otherNan = 0xFFFFFFFF00000001ull;
    EXPECT_TRUE(Double(otherNan).IsNan());

    EXPECT_TRUE(!Double(Double::Infinity()).IsNan());
    EXPECT_TRUE(!Double(-Double::Infinity()).IsNan());
    EXPECT_TRUE(!Double(0.0).IsNan());
    EXPECT_TRUE(!Double(-0.0).IsNan());
    EXPECT_TRUE(!Double(1.0).IsNan());
    EXPECT_TRUE(!Double(-1.0).IsNan());

    uint64_t minDouble64 = 0x0000000000000001ull;
    EXPECT_TRUE(!Double(minDouble64).IsNan());
}

TEST(Convert, DiyFpSingleIsNan)
{
    EXPECT_TRUE(Single(Single::Nan()).IsNan());

    uint32_t otherNan = 0xFFFFF001;
    EXPECT_TRUE(Single(otherNan).IsNan());

    EXPECT_TRUE(!Single(Single::Infinity()).IsNan());
    EXPECT_TRUE(!Single(-Single::Infinity()).IsNan());
    EXPECT_TRUE(!Single(0.0f).IsNan());
    EXPECT_TRUE(!Single(-0.0f).IsNan());
    EXPECT_TRUE(!Single(1.0f).IsNan());
    EXPECT_TRUE(!Single(-1.0f).IsNan());

    uint32_t minFloat32 = 0x00000001;
    EXPECT_TRUE(!Single(minFloat32).IsNan());
}

TEST(Convert, DiyFpDoubleSign)
{
    EXPECT_EQ(1, Double(1.0).Sign());
    EXPECT_EQ(1, Double(Double::Infinity()).Sign());
    EXPECT_EQ(-1, Double(-Double::Infinity()).Sign());
    EXPECT_EQ(1, Double(0.0).Sign());
    EXPECT_EQ(-1, Double(-0.0).Sign());

    uint64_t minDouble64 = 0x0000000000000001ull;
    EXPECT_EQ(1, Double(minDouble64).Sign());
}

TEST(Convert, DiyFpSingleSign)
{
    EXPECT_EQ(1, Single(1.0f).Sign());
    EXPECT_EQ(1, Single(Single::Infinity()).Sign());
    EXPECT_EQ(-1, Single(-Single::Infinity()).Sign());
    EXPECT_EQ(1, Single(0.0f).Sign());
    EXPECT_EQ(-1, Single(-0.0f).Sign());

    uint32_t minFloat32 = 0x00000001;
    EXPECT_EQ(1, Single(minFloat32).Sign());
}

TEST(Convert, DiyFpDoubleNormalizedBoundaries)
{
    DiyFp boundaryPlus;
    DiyFp boundaryMinus;
    DiyFp fp = Double(1.5).ToNormalizedDiyFp();
    Double(1.5).NormalizedBoundaries(boundaryMinus, boundaryPlus);
    EXPECT_EQ(fp.Exponent(), boundaryMinus.Exponent());
    EXPECT_EQ(fp.Exponent(), boundaryPlus.Exponent());
    EXPECT_EQ(fp.Significand() - boundaryMinus.Significand(), boundaryPlus.Significand() - fp.Significand());
    EXPECT_EQ((1 << 10), fp.Significand() - boundaryMinus.Significand());

    fp = Double(1.0).ToNormalizedDiyFp();
    Double(1.0).NormalizedBoundaries(boundaryMinus, boundaryPlus);
    EXPECT_EQ(fp.Exponent(), boundaryMinus.Exponent());
    EXPECT_EQ(fp.Exponent(), boundaryPlus.Exponent());
    EXPECT_GT(boundaryPlus.Significand() - fp.Significand(), fp.Significand() - boundaryMinus.Significand());
    EXPECT_EQ((1 << 9), fp.Significand() - boundaryMinus.Significand());
    EXPECT_EQ((1 << 10), boundaryPlus.Significand() - fp.Significand());

    uint64_t minDouble64 = 0x0000000000000001ull;
    fp = Double(minDouble64).ToNormalizedDiyFp();
    Double(minDouble64).NormalizedBoundaries(boundaryMinus, boundaryPlus);
    EXPECT_EQ(fp.Exponent(), boundaryMinus.Exponent());
    EXPECT_EQ(fp.Exponent(), boundaryPlus.Exponent());
    EXPECT_EQ(fp.Significand() - boundaryMinus.Significand(), boundaryPlus.Significand() - fp.Significand());
    EXPECT_EQ(static_cast<uint64_t>(1) << 62, fp.Significand() - boundaryMinus.Significand());

    uint64_t smallestNormal64 = 0x0010000000000000ull;
    fp = Double(smallestNormal64).ToNormalizedDiyFp();
    Double(smallestNormal64).NormalizedBoundaries(boundaryMinus, boundaryPlus);
    EXPECT_EQ(fp.Exponent(), boundaryMinus.Exponent());
    EXPECT_EQ(fp.Exponent(), boundaryPlus.Exponent());
    EXPECT_EQ(fp.Significand() - boundaryMinus.Significand(), boundaryPlus.Significand() - fp.Significand());
    EXPECT_EQ((1 << 10), fp.Significand() - boundaryMinus.Significand());

    uint64_t largestDenormal64 = 0x000FFFFFFFFFFFFFull;
    fp = Double(largestDenormal64).ToNormalizedDiyFp();
    Double(largestDenormal64).NormalizedBoundaries(boundaryMinus, boundaryPlus);
    EXPECT_EQ(fp.Exponent(), boundaryMinus.Exponent());
    EXPECT_EQ(fp.Exponent(), boundaryPlus.Exponent());
    EXPECT_EQ(fp.Significand() - boundaryMinus.Significand(), boundaryPlus.Significand() - fp.Significand());
    EXPECT_EQ((1 << 11), fp.Significand() - boundaryMinus.Significand());

    uint64_t maxDouble64 = 0x7FEFFFFFFFFFFFFFull;
    fp = Double(maxDouble64).ToNormalizedDiyFp();
    Double(maxDouble64).NormalizedBoundaries(boundaryMinus, boundaryPlus);
    EXPECT_EQ(fp.Exponent(), boundaryMinus.Exponent());
    EXPECT_EQ(fp.Exponent(), boundaryPlus.Exponent());
    EXPECT_EQ(fp.Significand() - boundaryMinus.Significand(), boundaryPlus.Significand() - fp.Significand());
    EXPECT_EQ((1 << 10), fp.Significand() - boundaryMinus.Significand());
}

TEST(Convert, DiyFpSingleNormalizedBoundaries)
{
    const uint64_t kOne64 = 1;
    DiyFp boundaryPlus;
    DiyFp boundaryMinus;
    DiyFp fp = Single(1.5f).ToDiyFp();
    fp.Normalize();
    Single(1.5f).NormalizedBoundaries(boundaryMinus, boundaryPlus);
    EXPECT_EQ(fp.Exponent(), boundaryMinus.Exponent());
    EXPECT_EQ(fp.Exponent(), boundaryPlus.Exponent());
    EXPECT_EQ(fp.Significand() - boundaryMinus.Significand(), boundaryPlus.Significand() - fp.Significand());
    EXPECT_EQ((kOne64 << 39), fp.Significand() - boundaryMinus.Significand());

    fp = Single(1.0f).ToDiyFp();
    fp.Normalize();
    Single(1.0f).NormalizedBoundaries(boundaryMinus, boundaryPlus);
    EXPECT_EQ(fp.Exponent(), boundaryMinus.Exponent());
    EXPECT_EQ(fp.Exponent(), boundaryPlus.Exponent());
    EXPECT_GT(boundaryPlus.Significand() - fp.Significand(), fp.Significand() - boundaryMinus.Significand());
    EXPECT_EQ((kOne64 << 38), fp.Significand() - boundaryMinus.Significand());
    EXPECT_EQ((kOne64 << 39), boundaryPlus.Significand() - fp.Significand());

    uint32_t minFloat32 = 0x00000001;
    fp = Single(minFloat32).ToDiyFp();
    fp.Normalize();
    Single(minFloat32).NormalizedBoundaries(boundaryMinus, boundaryPlus);
    EXPECT_EQ(fp.Exponent(), boundaryMinus.Exponent());
    EXPECT_EQ(fp.Exponent(), boundaryPlus.Exponent());
    EXPECT_EQ(fp.Significand() - boundaryMinus.Significand(), boundaryPlus.Significand() - fp.Significand());
    EXPECT_EQ((kOne64 << 62), fp.Significand() - boundaryMinus.Significand());

    uint32_t smallestNormal32 = 0x00800000;
    fp = Single(smallestNormal32).ToDiyFp();
    fp.Normalize();
    Single(smallestNormal32).NormalizedBoundaries(boundaryMinus, boundaryPlus);
    EXPECT_EQ(fp.Exponent(), boundaryMinus.Exponent());
    EXPECT_EQ(fp.Exponent(), boundaryPlus.Exponent());
    EXPECT_EQ(fp.Significand() - boundaryMinus.Significand(), boundaryPlus.Significand() - fp.Significand());
    EXPECT_EQ((kOne64 << 39), fp.Significand() - boundaryMinus.Significand());

    uint32_t largestDenormal32 = 0x007FFFFF;
    fp = Single(largestDenormal32).ToDiyFp();
    fp.Normalize();
    Single(largestDenormal32).NormalizedBoundaries(boundaryMinus, boundaryPlus);
    EXPECT_EQ(fp.Exponent(), boundaryMinus.Exponent());
    EXPECT_EQ(fp.Exponent(), boundaryPlus.Exponent());
    EXPECT_EQ(fp.Significand() - boundaryMinus.Significand(), boundaryPlus.Significand() - fp.Significand());
    EXPECT_EQ((kOne64 << 40), fp.Significand() - boundaryMinus.Significand());

    uint32_t maxFloat32 = 0X7F7FFFFF;
    fp = Single(maxFloat32).ToDiyFp();
    fp.Normalize();
    Single(maxFloat32).NormalizedBoundaries(boundaryMinus, boundaryPlus);
    EXPECT_EQ(fp.Exponent(), boundaryMinus.Exponent());
    EXPECT_EQ(fp.Exponent(), boundaryPlus.Exponent());
    EXPECT_EQ(fp.Significand() - boundaryMinus.Significand(), boundaryPlus.Significand() - fp.Significand());
    EXPECT_EQ((kOne64 << 39), fp.Significand() - boundaryMinus.Significand());
}

TEST(Convert, DiyFpNextDouble)
{
    EXPECT_EQ(4e-324, Double(0.0).NextDouble());
    EXPECT_EQ(0.0, Double(-0.0).NextDouble());
    EXPECT_EQ(-0.0, Double(-4e-324).NextDouble());
    EXPECT_GT(Double(Double(-0.0).NextDouble()).Sign(), 0);
    EXPECT_LT(Double(Double(-4e-324).NextDouble()).Sign(), 0);
    Double d0(-4e-324);
    Double d1(d0.NextDouble());
    Double d2(d1.NextDouble());
    EXPECT_EQ(-0.0, d1.ToDouble());
    EXPECT_LT(d1.Sign(), 0);
    EXPECT_EQ(0.0, d2.ToDouble());
    EXPECT_GT(d2.Sign(), 0);
    EXPECT_EQ(4e-324, d2.NextDouble());
    EXPECT_EQ(-1.7976931348623157e308, Double(-Double::Infinity()).NextDouble());
    EXPECT_EQ(Double::Infinity(), Double((uint64_t)0x7FEFFFFFFFFFFFFFull).NextDouble());
}

TEST(Convert, DiyFpPreviousDouble)
{
    EXPECT_EQ(0.0, Double(4e-324).PreviousDouble());
    EXPECT_EQ(-0.0, Double(0.0).PreviousDouble());
    EXPECT_LT(Double(Double(0.0).PreviousDouble()).Sign(), 0);
    EXPECT_EQ(-4e-324, Double(-0.0).PreviousDouble());
    Double d0(4e-324);
    Double d1(d0.PreviousDouble());
    Double d2(d1.PreviousDouble());
    EXPECT_EQ(0.0, d1.ToDouble());
    EXPECT_GT(d1.Sign(), 0);
    EXPECT_EQ(-0.0, d2.ToDouble());
    EXPECT_LT(d2.Sign(), 0);
    EXPECT_EQ(-4e-324, d2.PreviousDouble());
    EXPECT_EQ(1.7976931348623157e308, Double(Double::Infinity()).PreviousDouble());
    EXPECT_EQ(-Double::Infinity(), Double((uint64_t)0xFFEFFFFFFFFFFFFFull).PreviousDouble());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(Convert, BignumAssign)
{
    char buffer[kBufferSize];
    Bignum bignum;
    Bignum bignum2;
    bignum.AssignUInt16(0);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("0", buffer);
    bignum.AssignUInt16(0xA);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("A", buffer);
    bignum.AssignUInt16(0x20);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("20", buffer);

    bignum.AssignUInt64(0);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("0", buffer);
    bignum.AssignUInt64(0xA);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("A", buffer);
    bignum.AssignUInt64(0x20);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("20", buffer);
    bignum.AssignUInt64(0x100);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("100", buffer);

    bignum.AssignUInt64(0x12345678);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("12345678", buffer);

    uint64_t big = 0xFFFFFFFFFFFFFFFFull;
    bignum.AssignUInt64(big);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFFFFFFFFFFF", buffer);

    big = 0x123456789ABCDEF0ull;
    bignum.AssignUInt64(big);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("123456789ABCDEF0", buffer);

    bignum2.AssignBignum(bignum);
    EXPECT_TRUE(bignum2.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("123456789ABCDEF0", buffer);

    AssignDecimalString(bignum, "0");
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("0", buffer);

    AssignDecimalString(bignum, "1");
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    AssignDecimalString(bignum, "1234567890");
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("499602D2", buffer);

    AssignHexString(bignum, "0");
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("0", buffer);

    AssignHexString(bignum, "123456789ABCDEF0");
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("123456789ABCDEF0", buffer);
}

TEST(Convert, BignumShiftLeft)
{
    char buffer[kBufferSize];
    Bignum bignum;
    AssignHexString(bignum, "0");
    bignum.ShiftLeft(100);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("0", buffer);

    AssignHexString(bignum, "1");
    bignum.ShiftLeft(1);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("2", buffer);

    AssignHexString(bignum, "1");
    bignum.ShiftLeft(4);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10", buffer);

    AssignHexString(bignum, "1");
    bignum.ShiftLeft(32);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("100000000", buffer);

    AssignHexString(bignum, "1");
    bignum.ShiftLeft(64);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000000000", buffer);

    AssignHexString(bignum, "123456789ABCDEF");
    bignum.ShiftLeft(64);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("123456789ABCDEF0000000000000000", buffer);
    bignum.ShiftLeft(1);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("2468ACF13579BDE0000000000000000", buffer);
}

TEST(Convert, BignumAddUInt64)
{
    char buffer[kBufferSize];
    Bignum bignum;
    AssignHexString(bignum, "0");
    bignum.AddUInt64(0xA);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("A", buffer);

    AssignHexString(bignum, "1");
    bignum.AddUInt64(0xA);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("B", buffer);

    AssignHexString(bignum, "1");
    bignum.AddUInt64(0x100);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("101", buffer);

    AssignHexString(bignum, "1");
    bignum.AddUInt64(0xFFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000", buffer);

    AssignHexString(bignum, "FFFFFFF");
    bignum.AddUInt64(0x1);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000", buffer);

    AssignHexString(bignum, "10000000000000000000000000000000000000000000");
    bignum.AddUInt64(0xFFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1000000000000000000000000000000000000000FFFF", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    bignum.AddUInt64(0x1);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("100000000000000000000000000000000000000000000", buffer);

    bignum.AssignUInt16(0x1);
    bignum.ShiftLeft(100);
    bignum.AddUInt64(1);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000000000000000001", buffer);

    bignum.AssignUInt16(0x1);
    bignum.ShiftLeft(100);
    bignum.AddUInt64(0xFFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1000000000000000000000FFFF", buffer);

    AssignHexString(bignum, "0");
    bignum.AddUInt64(0xA00000000ull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("A00000000", buffer);

    AssignHexString(bignum, "1");
    bignum.AddUInt64(0xA00000000ull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("A00000001", buffer);

    AssignHexString(bignum, "1");
    bignum.AddUInt64(0x10000000000ull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000001", buffer);

    AssignHexString(bignum, "1");
    bignum.AddUInt64(0xFFFF00000000ull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFF00000001", buffer);

    AssignHexString(bignum, "FFFFFFF");
    bignum.AddUInt64(0x100000000ull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10FFFFFFF", buffer);

    AssignHexString(bignum, "10000000000000000000000000000000000000000000");
    bignum.AddUInt64(0xFFFF00000000ull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000000000000000000000000FFFF00000000", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    bignum.AddUInt64(0x100000000ull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1000000000000000000000000000000000000FFFFFFFF", buffer);

    bignum.AssignUInt16(0x1);
    bignum.ShiftLeft(100);
    bignum.AddUInt64(0x100000000ull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000000000100000000", buffer);

    bignum.AssignUInt16(0x1);
    bignum.ShiftLeft(100);
    bignum.AddUInt64(0xFFFF00000000ull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000000FFFF00000000", buffer);
}

TEST(Convert, BignumAddBignum)
{
    char buffer[kBufferSize];
    Bignum bignum;
    Bignum other;

    AssignHexString(other, "1");
    AssignHexString(bignum, "0");
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    AssignHexString(bignum, "1");
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("2", buffer);

    AssignHexString(bignum, "FFFFFFF");
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFF");
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("100000000000000", buffer);

    AssignHexString(bignum, "10000000000000000000000000000000000000000000");
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000000000000000000000000000000000001", buffer);

    AssignHexString(other, "1000000000000");

    AssignHexString(bignum, "1");
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1000000000001", buffer);

    AssignHexString(bignum, "FFFFFFF");
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("100000FFFFFFF", buffer);

    AssignHexString(bignum, "10000000000000000000000000000000000000000000");
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000000000000000000000001000000000000", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1000000000000000000000000000000FFFFFFFFFFFF", buffer);

    bignum.AssignUInt16(0x1);
    bignum.ShiftLeft(100);
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000001000000000000", buffer);

    other.ShiftLeft(64);  // other == "10000000000000000000000000000"

    bignum.AssignUInt16(0x1);
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000000000000000000001", buffer);

    AssignHexString(bignum, "FFFFFFF");
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1000000000000000000000FFFFFFF", buffer);

    AssignHexString(bignum, "10000000000000000000000000000000000000000000");
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000000010000000000000000000000000000", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("100000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF", buffer);

    bignum.AssignUInt16(0x1);
    bignum.ShiftLeft(100);
    bignum.AddBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10010000000000000000000000000", buffer);
}

TEST(Convert, BignumSubtractBignum)
{
    char buffer[kBufferSize];
    Bignum bignum;
    Bignum other;

    AssignHexString(bignum, "1");
    AssignHexString(other, "0");
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    AssignHexString(bignum, "2");
    AssignHexString(other, "0");
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("2", buffer);

    AssignHexString(bignum, "10000000");
    AssignHexString(other, "1");
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFF", buffer);

    AssignHexString(bignum, "100000000000000");
    AssignHexString(other, "1");
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFFFFFFFFF", buffer);

    AssignHexString(bignum, "10000000000000000000000000000000000000000001");
    AssignHexString(other, "1");
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000000000000000000000000000000000000", buffer);

    AssignHexString(bignum, "1000000000001");
    AssignHexString(other, "1000000000000");
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    AssignHexString(bignum, "100000FFFFFFF");
    AssignHexString(other, "1000000000000");
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFF", buffer);

    AssignHexString(bignum, "10000000000000000000000000000001000000000000");
    AssignHexString(other, "1000000000000");
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000000000000000000000000000000000000", buffer);

    AssignHexString(bignum, "1000000000000000000000000000000FFFFFFFFFFFF");
    AssignHexString(other, "1000000000000");
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", buffer);

    bignum.AssignUInt16(0x1);
    bignum.ShiftLeft(100);  // "10 0000 0000 0000 0000 0000 0000"
    AssignHexString(other, "1000000000000");
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFFFFFFFF000000000000", buffer);

    AssignHexString(other, "1000000000000");
    other.ShiftLeft(48);  // other == "1000000000000000000000000"

    bignum.AssignUInt16(0x1);
    bignum.ShiftLeft(100);  // bignum == "10000000000000000000000000"
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("F000000000000000000000000", buffer);

    other.AssignUInt16(0x1);
    other.ShiftLeft(35);  // other == "800000000"
    AssignHexString(bignum, "FFFFFFF");
    bignum.ShiftLeft(60);  // bignum = FFFFFFF000000000000000
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFEFFFFFF800000000", buffer);

    AssignHexString(bignum, "10000000000000000000000000000000000000000000");
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF800000000", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    bignum.SubtractBignum(other);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7FFFFFFFF", buffer);
}

TEST(Convert, BignumMultiplyUInt32)
{
    char buffer[kBufferSize];
    Bignum bignum;

    AssignHexString(bignum, "0");
    bignum.MultiplyByUInt32(0x25);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("0", buffer);

    AssignHexString(bignum, "2");
    bignum.MultiplyByUInt32(0x5);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("A", buffer);

    AssignHexString(bignum, "10000000");
    bignum.MultiplyByUInt32(0x9);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("90000000", buffer);

    AssignHexString(bignum, "100000000000000");
    bignum.MultiplyByUInt32(0xFFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFF00000000000000", buffer);

    AssignHexString(bignum, "100000000000000");
    bignum.MultiplyByUInt32(0xFFFFFFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFFF00000000000000", buffer);

    AssignHexString(bignum, "1234567ABCD");
    bignum.MultiplyByUInt32(0xFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("12333335552433", buffer);

    AssignHexString(bignum, "1234567ABCD");
    bignum.MultiplyByUInt32(0xFFFFFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("12345679998A985433", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFFFF");
    bignum.MultiplyByUInt32(0x2);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1FFFFFFFFFFFFFFFE", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFFFF");
    bignum.MultiplyByUInt32(0x4);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("3FFFFFFFFFFFFFFFC", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFFFF");
    bignum.MultiplyByUInt32(0xF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("EFFFFFFFFFFFFFFF1", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFFFF");
    bignum.MultiplyByUInt32(0xFFFFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFEFFFFFFFFFF000001", buffer);

    bignum.AssignUInt16(0x1);
    bignum.ShiftLeft(100);  // "10 0000 0000 0000 0000 0000 0000"
    bignum.MultiplyByUInt32(2);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("20000000000000000000000000", buffer);

    bignum.AssignUInt16(0x1);
    bignum.ShiftLeft(100);  // "10 0000 0000 0000 0000 0000 0000"
    bignum.MultiplyByUInt32(0xF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("F0000000000000000000000000", buffer);

    bignum.AssignUInt16(0xFFFF);
    bignum.ShiftLeft(100);  // "FFFF0 0000 0000 0000 0000 0000 0000"
    bignum.MultiplyByUInt32(0xFFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFE00010000000000000000000000000", buffer);

    bignum.AssignUInt16(0xFFFF);
    bignum.ShiftLeft(100);  // "FFFF0 0000 0000 0000 0000 0000 0000"
    bignum.MultiplyByUInt32(0xFFFFFFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFEFFFF00010000000000000000000000000", buffer);

    bignum.AssignUInt16(0xFFFF);
    bignum.ShiftLeft(100);  // "FFFF0 0000 0000 0000 0000 0000 0000"
    bignum.MultiplyByUInt32(0xFFFFFFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFEFFFF00010000000000000000000000000", buffer);

    AssignDecimalString(bignum, "15611230384529777");
    bignum.MultiplyByUInt32(10000000);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("210EDD6D4CDD2580EE80", buffer);
}

TEST(Convert, BignumMultiplyUInt64)
{
    char buffer[kBufferSize];
    Bignum bignum;

    AssignHexString(bignum, "0");
    bignum.MultiplyByUInt64(0x25);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("0", buffer);

    AssignHexString(bignum, "2");
    bignum.MultiplyByUInt64(0x5);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("A", buffer);

    AssignHexString(bignum, "10000000");
    bignum.MultiplyByUInt64(0x9);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("90000000", buffer);

    AssignHexString(bignum, "100000000000000");
    bignum.MultiplyByUInt64(0xFFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFF00000000000000", buffer);

    AssignHexString(bignum, "100000000000000");
    bignum.MultiplyByUInt64(0xFFFFFFFFFFFFFFFFull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFFFFFFFFFFF00000000000000", buffer);

    AssignHexString(bignum, "1234567ABCD");
    bignum.MultiplyByUInt64(0xFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("12333335552433", buffer);

    AssignHexString(bignum, "1234567ABCD");
    bignum.MultiplyByUInt64(0xFFFFFFFFFFull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1234567ABCBDCBA985433", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFFFF");
    bignum.MultiplyByUInt64(0x2);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1FFFFFFFFFFFFFFFE", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFFFF");
    bignum.MultiplyByUInt64(0x4);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("3FFFFFFFFFFFFFFFC", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFFFF");
    bignum.MultiplyByUInt64(0xF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("EFFFFFFFFFFFFFFF1", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFFFF");
    bignum.MultiplyByUInt64(0xFFFFFFFFFFFFFFFFull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFFFFFFFFFFE0000000000000001", buffer);

    bignum.AssignUInt16(0x1);
    bignum.ShiftLeft(100);  // "10 0000 0000 0000 0000 0000 0000"
    bignum.MultiplyByUInt64(2);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("20000000000000000000000000", buffer);

    bignum.AssignUInt16(0x1);
    bignum.ShiftLeft(100);  // "10 0000 0000 0000 0000 0000 0000"
    bignum.MultiplyByUInt64(0xF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("F0000000000000000000000000", buffer);

    bignum.AssignUInt16(0xFFFF);
    bignum.ShiftLeft(100);  // "FFFF0 0000 0000 0000 0000 0000 0000"
    bignum.MultiplyByUInt64(0xFFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFE00010000000000000000000000000", buffer);

    bignum.AssignUInt16(0xFFFF);
    bignum.ShiftLeft(100);  // "FFFF0 0000 0000 0000 0000 0000 0000"
    bignum.MultiplyByUInt64(0xFFFFFFFF);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFEFFFF00010000000000000000000000000", buffer);

    bignum.AssignUInt16(0xFFFF);
    bignum.ShiftLeft(100);  // "FFFF0 0000 0000 0000 0000 0000 0000"
    bignum.MultiplyByUInt64(0xFFFFFFFFFFFFFFFFull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFEFFFFFFFFFFFF00010000000000000000000000000", buffer);

    AssignDecimalString(bignum, "15611230384529777");
    bignum.MultiplyByUInt64(0x8AC7230489E80000ull);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1E10EE4B11D15A7F3DE7F3C7680000", buffer);
}

TEST(Convert, BignumMultiplyPowerOfTen)
{
    char buffer[kBufferSize];
    Bignum bignum;

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(1);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("3034", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(2);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1E208", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(3);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("12D450", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(4);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("BC4B20", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(5);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("75AEF40", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(6);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("498D5880", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(7);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("2DF857500", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(8);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1CBB369200", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(9);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("11F5021B400", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(10);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("B3921510800", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(11);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("703B4D2A5000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(12);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("4625103A72000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(13);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("2BD72A24874000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(14);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1B667A56D488000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(15);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("11200C7644D50000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(16);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("AB407C9EB0520000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(17);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("6B084DE32E3340000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(18);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("42E530ADFCE0080000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(19);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("29CF3E6CBE0C0500000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(20);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1A218703F6C783200000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(21);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1054F4627A3CB1F400000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(22);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("A3518BD8C65EF38800000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(23);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("6612F7677BFB5835000000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(24);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("3FCBDAA0AD7D17212000000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(25);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("27DF68A46C6E2E74B4000000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(26);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("18EBA166C3C4DD08F08000000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(27);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("F9344E03A5B0A259650000000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(28);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("9BC0B0C2478E6577DF20000000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(29);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("61586E796CB8FF6AEB740000000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(30);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("3CD7450BE3F39FA2D32880000000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(31);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("26068B276E7843C5C3F9500000000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(50);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("149D1B4CFED03B23AB5F4E1196EF45C08000000000000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(100);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("5827249F27165024FBC47DFCA9359BF316332D1B91ACEECF471FBAB06D9B2"
        "0000000000000000000000000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(200);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("64C1F5C06C3816AFBF8DAFD5A3D756365BB0FD020E6F084E759C1F7C99E4F"
        "55B9ACC667CEC477EB958C2AEEB3C6C19BA35A1AD30B35C51EB72040920000"
        "0000000000000000000000000000000000000000000000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(500);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("96741A625EB5D7C91039FEB5C5ACD6D9831EDA5B083D800E6019442C8C8223"
        "3EAFB3501FE2058062221E15121334928880827DEE1EC337A8B26489F3A40A"
        "CB440A2423734472D10BFCE886F41B3AF9F9503013D86D088929CA86EEB4D8"
        "B9C831D0BD53327B994A0326227CFD0ECBF2EB48B02387AAE2D4CCCDF1F1A1"
        "B8CC4F1FA2C56AD40D0E4DAA9C28CDBF0A549098EA13200000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000", buffer);

    AssignDecimalString(bignum, "1234");
    bignum.MultiplyByPowerOfTen(1000);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1258040F99B1CD1CC9819C676D413EA50E4A6A8F114BB0C65418C62D399B81"
        "6361466CA8E095193E1EE97173553597C96673AF67FAFE27A66E7EF2E5EF2E"
        "E3F5F5070CC17FE83BA53D40A66A666A02F9E00B0E11328D2224B8694C7372"
        "F3D536A0AD1985911BD361496F268E8B23112500EAF9B88A9BC67B2AB04D38"
        "7FEFACD00F5AF4F764F9ABC3ABCDE54612DE38CD90CB6647CA389EA0E86B16"
        "BF7A1F34086E05ADBE00BD1673BE00FAC4B34AF1091E8AD50BA675E0381440"
        "EA8E9D93E75D816BAB37C9844B1441C38FC65CF30ABB71B36433AF26DD97BD"
        "ABBA96C03B4919B8F3515B92826B85462833380DC193D79F69D20DD6038C99"
        "6114EF6C446F0BA28CC772ACBA58B81C04F8FFDE7B18C4E5A3ABC51E637FDF"
        "6E37FDFF04C940919390F4FF92000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000"
        "0000000000000000000000000000", buffer);

    Bignum bignum2;
    AssignHexString(bignum2, "3DA774C07FB5DF54284D09C675A492165B830D5DAAEB2A7501"
        "DA17CF9DFA1CA2282269F92A25A97314296B717E3DCBB9FE17"
        "41A842FE2913F540F40796F2381155763502C58B15AF7A7F88"
        "6F744C9164FF409A28F7FA0C41F89ED79C1BE9F322C8578B97"
        "841F1CBAA17D901BE1230E3C00E1C643AF32638B5674E01FEA"
        "96FC90864E621B856A9E1CE56E6EB545B9C2F8F0CC10DDA88D"
        "CC6D282605F8DB67044F2DFD3695E7BA63877AE16701536AE6"
        "567C794D0BFE338DFBB42D92D4215AF3BB22BF0A8B283FDDC2"
        "C667A10958EA6D2");
    EXPECT_TRUE(bignum2.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("3DA774C07FB5DF54284D09C675A492165B830D5DAAEB2A7501"
        "DA17CF9DFA1CA2282269F92A25A97314296B717E3DCBB9FE17"
        "41A842FE2913F540F40796F2381155763502C58B15AF7A7F88"
        "6F744C9164FF409A28F7FA0C41F89ED79C1BE9F322C8578B97"
        "841F1CBAA17D901BE1230E3C00E1C643AF32638B5674E01FEA"
        "96FC90864E621B856A9E1CE56E6EB545B9C2F8F0CC10DDA88D"
        "CC6D282605F8DB67044F2DFD3695E7BA63877AE16701536AE6"
        "567C794D0BFE338DFBB42D92D4215AF3BB22BF0A8B283FDDC2"
        "C667A10958EA6D2", buffer);

    bignum.AssignBignum(bignum2);
    bignum.MultiplyByPowerOfTen(1);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("2688A8F84FD1AB949930261C0986DB4DF931E85A8AD2FA8921284EE1C2BC51"
        "E55915823BBA5789E7EC99E326EEE69F543ECE890929DED9AC79489884BE57"
        "630AD569E121BB76ED8DAC8FB545A8AFDADF1F8860599AFC47A93B6346C191"
        "7237F5BD36B73EB29371F4A4EE7A116CB5E8E5808D1BEA4D7F7E3716090C13"
        "F29E5DDA53F0FD513362A2D20F6505314B9419DB967F8A8A89589FC43917C3"
        "BB892062B17CBE421DB0D47E34ACCCE060D422CFF60DCBD0277EE038BD509C"
        "7BC494D8D854F5B76696F927EA99BC00C4A5D7928434", buffer);

    bignum.AssignBignum(bignum2);
    bignum.MultiplyByPowerOfTen(2);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1815699B31E30B3CDFBE17D185F44910BBBF313896C3DC95B4B9314D19B5B32"
        "F57AD71655476B630F3E02DF855502394A74115A5BA2B480BCBCD5F52F6F69D"
        "E6C5622CB5152A54788BD9D14B896DE8CB73B53C3800DDACC9C51E0C38FAE76"
        "2F9964232872F9C2738E7150C4AE3F1B18F70583172706FAEE26DC5A78C77A2"
        "FAA874769E52C01DA5C3499F233ECF3C90293E0FB69695D763DAA3AEDA5535B"
        "43DAEEDF6E9528E84CEE0EC000C3C8495C1F9C89F6218AF4C23765261CD5ADD"
        "0787351992A01E5BB8F2A015807AE7A6BB92A08", buffer);

    bignum.AssignBignum(bignum2);
    bignum.MultiplyByPowerOfTen(5);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("5E13A4863ADEE3E5C9FE8D0A73423D695D62D8450CED15A8C9F368952C6DC3"
        "F0EE7D82F3D1EFB7AF38A3B3920D410AFCAD563C8F5F39116E141A3C5C14B3"
        "58CD73077EA35AAD59F6E24AD98F10D5555ABBFBF33AC361EAF429FD5FBE94"
        "17DA9EF2F2956011F9F93646AA38048A681D984ED88127073443247CCC167C"
        "B354A32206EF5A733E73CF82D795A1AD598493211A6D613C39515E0E0F6304"
        "DCD9C810F3518C7F6A7CB6C81E99E02FCC65E8FDB7B7AE97306CC16A8631CE"
        "0A2AEF6568276BE4C176964A73C153FDE018E34CB4C2F40", buffer);

    bignum.AssignBignum(bignum2);
    bignum.MultiplyByPowerOfTen(10);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("8F8CB8EB51945A7E815809F6121EF2F4E61EF3405CD9432CAD2709749EEAFD"
        "1B81E843F14A3667A7BDCCC9E0BB795F63CDFDB62844AC7438976C885A0116"
        "29607DA54F9C023CC366570B7637ED0F855D931752038A614922D0923E382C"
        "B8E5F6C975672DB76E0DE471937BB9EDB11E28874F1C122D5E1EF38CECE9D0"
        "0723056BCBD4F964192B76830634B1D322B7EB0062F3267E84F5C824343A77"
        "4B7DCEE6DD464F01EBDC8C671BB18BB4EF4300A42474A6C77243F2A12B03BF"
        "0443C38A1C0D2701EDB393135AE0DEC94211F9D4EB51F990800", buffer);

    bignum.AssignBignum(bignum2);
    bignum.MultiplyByPowerOfTen(50);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("107A8BE345E24407372FC1DE442CBA696BC23C4FFD5B4BDFD9E5C39559815"
        "86628CF8472D2D589F2FC2BAD6E0816EC72CBF85CCA663D8A1EC6C51076D8"
        "2D247E6C26811B7EC4D4300FB1F91028DCB7B2C4E7A60C151161AA7E65E79"
        "B40917B12B2B5FBE7745984D4E8EFA31F9AE6062427B068B144A9CB155873"
        "E7C0C9F0115E5AC72DC5A73C4796DB970BF9205AB8C77A6996EB1B417F9D1"
        "6232431E6313C392203601B9C22CC10DDA88DCC6D282605F8DB67044F2DFD"
        "3695E7BA63877AE16701536AE6567C794D0BFE338DFBB42D924CF964BD2C0"
        "F586E03A2FCD35A408000000000000", buffer);

    bignum.AssignBignum(bignum2);
    bignum.MultiplyByPowerOfTen(100);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("46784A90ACD0ED3E7759CC585FB32D36EB6034A6F78D92604E3BAA5ED3D8B"
        "6E60E854439BE448897FB4B7EA5A3D873AA0FCB3CFFD80D0530880E45F511"
        "722A50CE7E058B5A6F5464DB7500E34984EE3202A9441F44FA1554C0CEA96"
        "B438A36F25E7C9D56D71AE2CD313EC37534DA299AC0854FC48591A7CF3171"
        "31265AA4AE62DE32344CE7BEEEF894AE686A2DAAFE5D6D9A10971FFD9C064"
        "5079B209E1048F58B5192D41D84336AC4C8C489EEF00939CFC9D55C122036"
        "01B9C22CC10DDA88DCC6D282605F8DB67044F2DFD3695E7BA3F67B96D3A32"
        "E11FB5561B68744C4035B0800DC166D49D98E3FD1D5BB2000000000000000"
        "0000000000", buffer);

    bignum.AssignBignum(bignum2);
    bignum.MultiplyByPowerOfTen(200);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("508BD351221DF139D72D88CDC0416845A53EE2D0E6B98352509A9AC312F8C"
        "6CB1A144889416201E0B6CE66EA3EBE259B5FD79ECFC1FD77963CE516CC7E"
        "2FE73D4B5B710C19F6BCB092C7A2FD76286543B8DBD2C596DFF2C896720BA"
        "DFF7BC9C366ACEA3A880AEC287C5E6207DF2739B5326FC19D773BD830B109"
        "ED36C7086544BF8FDB9D4B73719C2B5BC2F571A5937EC46876CD428281F6B"
        "F287E1E07F25C1B1D46BC37324FF657A8B2E0071DB83B86123CA34004F406"
        "001082D7945E90C6E8C9A9FEC2B44BE0DDA46E9F52B152E4D1336D2FCFBC9"
        "96E30CA0082256737365158FE36482AA7EB9DAF2AB128F10E7551A3CD5BE6"
        "0A922F3A7D5EED38B634A7EC95BCF7021BA6820A292000000000000000000"
        "00000000000000000000000000000000", buffer);

    bignum.AssignBignum(bignum2);
    bignum.MultiplyByPowerOfTen(500);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("7845F900E475B5086885BAAAE67C8E85185ACFE4633727F82A4B06B5582AC"
        "BE933C53357DA0C98C20C5AC900C4D76A97247DF52B79F48F9E35840FB715"
        "D392CE303E22622B0CF82D9471B398457DD3196F639CEE8BBD2C146873841"
        "F0699E6C41F04FC7A54B48CEB995BEB6F50FE81DE9D87A8D7F849CC523553"
        "7B7BBBC1C7CAAFF6E9650BE03B308C6D31012AEF9580F70D3EE2083ADE126"
        "8940FA7D6308E239775DFD2F8C97FF7EBD525DAFA6512216F7047A62A93DC"
        "38A0165BDC67E250DCC96A0181DE935A70B38704DC71819F02FC5261FF7E1"
        "E5F11907678B0A3E519FF4C10A867B0C26CE02BE6960BA8621A87303C101C"
        "3F88798BB9F7739655946F8B5744E6B1EAF10B0C5621330F0079209033C69"
        "20DE2E2C8D324F0624463735D482BF291926C22A910F5B80FA25170B6B57D"
        "8D5928C7BCA3FE87461275F69BD5A1B83181DAAF43E05FC3C72C4E93111B6"
        "6205EBF49B28FEDFB7E7526CBDA658A332000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000", buffer);
}

TEST(Convert, BignumDivideModuloIntBignum)
{
    char buffer[kBufferSize];
    Bignum bignum;
    Bignum other;
    Bignum third;

    bignum.AssignUInt16(10);
    other.AssignUInt16(2);
    EXPECT_EQ(5, bignum.DivideModuloIntBignum(other));
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("0", buffer);

    bignum.AssignUInt16(10);
    bignum.ShiftLeft(500);
    other.AssignUInt16(2);
    other.ShiftLeft(500);
    EXPECT_EQ(5, bignum.DivideModuloIntBignum(other));
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("0", buffer);

    bignum.AssignUInt16(11);
    other.AssignUInt16(2);
    EXPECT_EQ(5, bignum.DivideModuloIntBignum(other));
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    bignum.AssignUInt16(10);
    bignum.ShiftLeft(500);
    other.AssignUInt16(1);
    bignum.AddBignum(other);
    other.AssignUInt16(2);
    other.ShiftLeft(500);
    EXPECT_EQ(5, bignum.DivideModuloIntBignum(other));
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    bignum.AssignUInt16(10);
    bignum.ShiftLeft(500);
    other.AssignBignum(bignum);
    bignum.MultiplyByUInt32(0x1234);
    third.AssignUInt16(0xFFF);
    bignum.AddBignum(third);
    EXPECT_EQ(0x1234, bignum.DivideModuloIntBignum(other));
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFF", buffer);

    bignum.AssignUInt16(10);
    AssignHexString(other, "1234567890");
    EXPECT_EQ(0, bignum.DivideModuloIntBignum(other));
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("A", buffer);

    AssignHexString(bignum, "12345678");
    AssignHexString(other, "3789012");
    EXPECT_EQ(5, bignum.DivideModuloIntBignum(other));
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("D9861E", buffer);

    AssignHexString(bignum, "70000001");
    AssignHexString(other, "1FFFFFFF");
    EXPECT_EQ(3, bignum.DivideModuloIntBignum(other));
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000004", buffer);

    AssignHexString(bignum, "28000000");
    AssignHexString(other, "12A05F20");
    EXPECT_EQ(2, bignum.DivideModuloIntBignum(other));
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("2BF41C0", buffer);

    bignum.AssignUInt16(10);
    bignum.ShiftLeft(500);
    other.AssignBignum(bignum);
    bignum.MultiplyByUInt32(0x1234);
    third.AssignUInt16(0xFFF);
    other.SubtractBignum(third);
    EXPECT_EQ(0x1234, bignum.DivideModuloIntBignum(other));
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1232DCC", buffer);
    EXPECT_EQ(0, bignum.DivideModuloIntBignum(other));
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1232DCC", buffer);
}

TEST(Convert, BignumCompare)
{
    Bignum bignum1;
    Bignum bignum2;
    bignum1.AssignUInt16(1);
    bignum2.AssignUInt16(1);
    EXPECT_EQ(0, Bignum::Compare(bignum1, bignum2));
    EXPECT_TRUE(Bignum::Equal(bignum1, bignum2));
    EXPECT_TRUE(Bignum::LessEqual(bignum1, bignum2));
    EXPECT_TRUE(!Bignum::Less(bignum1, bignum2));

    bignum1.AssignUInt16(0);
    bignum2.AssignUInt16(1);
    EXPECT_EQ(-1, Bignum::Compare(bignum1, bignum2));
    EXPECT_EQ(+1, Bignum::Compare(bignum2, bignum1));
    EXPECT_TRUE(!Bignum::Equal(bignum1, bignum2));
    EXPECT_TRUE(!Bignum::Equal(bignum2, bignum1));
    EXPECT_TRUE(Bignum::LessEqual(bignum1, bignum2));
    EXPECT_TRUE(!Bignum::LessEqual(bignum2, bignum1));
    EXPECT_TRUE(Bignum::Less(bignum1, bignum2));
    EXPECT_TRUE(!Bignum::Less(bignum2, bignum1));

    AssignHexString(bignum1, "1234567890ABCDEF12345");
    AssignHexString(bignum2, "1234567890ABCDEF12345");
    EXPECT_EQ(0, Bignum::Compare(bignum1, bignum2));

    AssignHexString(bignum1, "1234567890ABCDEF12345");
    AssignHexString(bignum2, "1234567890ABCDEF12346");
    EXPECT_EQ(-1, Bignum::Compare(bignum1, bignum2));
    EXPECT_EQ(+1, Bignum::Compare(bignum2, bignum1));

    AssignHexString(bignum1, "1234567890ABCDEF12345");
    bignum1.ShiftLeft(500);
    AssignHexString(bignum2, "1234567890ABCDEF12345");
    bignum2.ShiftLeft(500);
    EXPECT_EQ(0, Bignum::Compare(bignum1, bignum2));

    AssignHexString(bignum1, "1234567890ABCDEF12345");
    bignum1.ShiftLeft(500);
    AssignHexString(bignum2, "1234567890ABCDEF12346");
    bignum2.ShiftLeft(500);
    EXPECT_EQ(-1, Bignum::Compare(bignum1, bignum2));
    EXPECT_EQ(+1, Bignum::Compare(bignum2, bignum1));

    bignum1.AssignUInt16(1);
    bignum1.ShiftLeft(64);
    AssignHexString(bignum2, "10000000000000000");
    EXPECT_EQ(0, Bignum::Compare(bignum1, bignum2));
    EXPECT_EQ(0, Bignum::Compare(bignum2, bignum1));

    bignum1.AssignUInt16(1);
    bignum1.ShiftLeft(64);
    AssignHexString(bignum2, "10000000000000001");
    EXPECT_EQ(-1, Bignum::Compare(bignum1, bignum2));
    EXPECT_EQ(+1, Bignum::Compare(bignum2, bignum1));

    bignum1.AssignUInt16(1);
    bignum1.ShiftLeft(96);
    AssignHexString(bignum2, "10000000000000001");
    bignum2.ShiftLeft(32);
    EXPECT_EQ(-1, Bignum::Compare(bignum1, bignum2));
    EXPECT_EQ(+1, Bignum::Compare(bignum2, bignum1));

    AssignHexString(bignum1, "FFFFFFFFFFFFFFFF");
    bignum2.AssignUInt16(1);
    bignum2.ShiftLeft(64);
    EXPECT_EQ(-1, Bignum::Compare(bignum1, bignum2));
    EXPECT_EQ(+1, Bignum::Compare(bignum2, bignum1));

    AssignHexString(bignum1, "FFFFFFFFFFFFFFFF");
    bignum1.ShiftLeft(32);
    bignum2.AssignUInt16(1);
    bignum2.ShiftLeft(96);
    EXPECT_EQ(-1, Bignum::Compare(bignum1, bignum2));
    EXPECT_EQ(+1, Bignum::Compare(bignum2, bignum1));

    AssignHexString(bignum1, "FFFFFFFFFFFFFFFF");
    bignum1.ShiftLeft(32);
    bignum2.AssignUInt16(1);
    bignum2.ShiftLeft(95);
    EXPECT_EQ(+1, Bignum::Compare(bignum1, bignum2));
    EXPECT_EQ(-1, Bignum::Compare(bignum2, bignum1));

    AssignHexString(bignum1, "FFFFFFFFFFFFFFFF");
    bignum1.ShiftLeft(32);
    bignum2.AssignUInt16(1);
    bignum2.ShiftLeft(100);
    EXPECT_EQ(-1, Bignum::Compare(bignum1, bignum2));
    EXPECT_EQ(+1, Bignum::Compare(bignum2, bignum1));

    AssignHexString(bignum1, "100000000000000");
    bignum2.AssignUInt16(1);
    bignum2.ShiftLeft(14 * 4);
    EXPECT_EQ(0, Bignum::Compare(bignum1, bignum2));
    EXPECT_EQ(0, Bignum::Compare(bignum2, bignum1));

    AssignHexString(bignum1, "100000000000001");
    bignum2.AssignUInt16(1);
    bignum2.ShiftLeft(14 * 4);
    EXPECT_EQ(+1, Bignum::Compare(bignum1, bignum2));
    EXPECT_EQ(-1, Bignum::Compare(bignum2, bignum1));

    AssignHexString(bignum1, "200000000000000");
    bignum2.AssignUInt16(3);
    bignum2.ShiftLeft(14 * 4);
    EXPECT_EQ(-1, Bignum::Compare(bignum1, bignum2));
    EXPECT_EQ(+1, Bignum::Compare(bignum2, bignum1));
}

TEST(Convert, BignumPlusCompare)
{
    Bignum a;
    Bignum b;
    Bignum c;
    a.AssignUInt16(1);
    b.AssignUInt16(0);
    c.AssignUInt16(1);
    EXPECT_EQ(0, Bignum::PlusCompare(a, b, c));
    EXPECT_TRUE(Bignum::PlusEqual(a, b, c));
    EXPECT_TRUE(Bignum::PlusLessEqual(a, b, c));
    EXPECT_TRUE(!Bignum::PlusLess(a, b, c));

    a.AssignUInt16(0);
    b.AssignUInt16(0);
    c.AssignUInt16(1);
    EXPECT_EQ(-1, Bignum::PlusCompare(a, b, c));
    EXPECT_EQ(+1, Bignum::PlusCompare(c, b, a));
    EXPECT_TRUE(!Bignum::PlusEqual(a, b, c));
    EXPECT_TRUE(!Bignum::PlusEqual(c, b, a));
    EXPECT_TRUE(Bignum::PlusLessEqual(a, b, c));
    EXPECT_TRUE(!Bignum::PlusLessEqual(c, b, a));
    EXPECT_TRUE(Bignum::PlusLess(a, b, c));
    EXPECT_TRUE(!Bignum::PlusLess(c, b, a));

    AssignHexString(a, "1234567890ABCDEF12345");
    b.AssignUInt16(1);
    AssignHexString(c, "1234567890ABCDEF12345");
    EXPECT_EQ(+1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890ABCDEF12344");
    b.AssignUInt16(1);
    AssignHexString(c, "1234567890ABCDEF12345");
    EXPECT_EQ(0, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4);
    AssignHexString(b, "ABCDEF12345");
    AssignHexString(c, "1234567890ABCDEF12345");
    EXPECT_EQ(0, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4);
    AssignHexString(b, "ABCDEF12344");
    AssignHexString(c, "1234567890ABCDEF12345");
    EXPECT_EQ(-1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4);
    AssignHexString(b, "ABCDEF12346");
    AssignHexString(c, "1234567890ABCDEF12345");
    EXPECT_EQ(1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567891");
    a.ShiftLeft(11 * 4);
    AssignHexString(b, "ABCDEF12345");
    AssignHexString(c, "1234567890ABCDEF12345");
    EXPECT_EQ(1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567889");
    a.ShiftLeft(11 * 4);
    AssignHexString(b, "ABCDEF12345");
    AssignHexString(c, "1234567890ABCDEF12345");
    EXPECT_EQ(-1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12345");
    b.ShiftLeft(32);
    AssignHexString(c, "1234567890ABCDEF12345");
    c.ShiftLeft(32);
    EXPECT_EQ(0, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12344");
    b.ShiftLeft(32);
    AssignHexString(c, "1234567890ABCDEF12345");
    c.ShiftLeft(32);
    EXPECT_EQ(-1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12346");
    b.ShiftLeft(32);
    AssignHexString(c, "1234567890ABCDEF12345");
    c.ShiftLeft(32);
    EXPECT_EQ(1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567891");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12345");
    b.ShiftLeft(32);
    AssignHexString(c, "1234567890ABCDEF12345");
    c.ShiftLeft(32);
    EXPECT_EQ(1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567889");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12345");
    b.ShiftLeft(32);
    AssignHexString(c, "1234567890ABCDEF12345");
    c.ShiftLeft(32);
    EXPECT_EQ(-1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12345");
    b.ShiftLeft(32);
    AssignHexString(c, "1234567890ABCDEF1234500000000");
    EXPECT_EQ(0, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12344");
    b.ShiftLeft(32);
    AssignHexString(c, "1234567890ABCDEF1234500000000");
    EXPECT_EQ(-1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12346");
    b.ShiftLeft(32);
    AssignHexString(c, "1234567890ABCDEF1234500000000");
    EXPECT_EQ(1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567891");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12345");
    b.ShiftLeft(32);
    AssignHexString(c, "1234567890ABCDEF1234500000000");
    EXPECT_EQ(1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567889");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12345");
    b.ShiftLeft(32);
    AssignHexString(c, "1234567890ABCDEF1234500000000");
    EXPECT_EQ(-1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12345");
    AssignHexString(c, "123456789000000000ABCDEF12345");
    EXPECT_EQ(0, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12346");
    AssignHexString(c, "123456789000000000ABCDEF12345");
    EXPECT_EQ(1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12344");
    AssignHexString(c, "123456789000000000ABCDEF12345");
    EXPECT_EQ(-1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12345");
    b.ShiftLeft(16);
    AssignHexString(c, "12345678900000ABCDEF123450000");
    EXPECT_EQ(0, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12344");
    b.ShiftLeft(16);
    AssignHexString(c, "12345678900000ABCDEF123450000");
    EXPECT_EQ(-1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12345");
    b.ShiftLeft(16);
    AssignHexString(c, "12345678900000ABCDEF123450001");
    EXPECT_EQ(-1, Bignum::PlusCompare(a, b, c));

    AssignHexString(a, "1234567890");
    a.ShiftLeft(11 * 4 + 32);
    AssignHexString(b, "ABCDEF12346");
    b.ShiftLeft(16);
    AssignHexString(c, "12345678900000ABCDEF123450000");
    EXPECT_EQ(+1, Bignum::PlusCompare(a, b, c));
}

TEST(Convert, BignumSquare)
{
    Bignum bignum;
    char buffer[kBufferSize];

    bignum.AssignUInt16(1);
    bignum.Square();
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    bignum.AssignUInt16(2);
    bignum.Square();
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("4", buffer);

    bignum.AssignUInt16(10);
    bignum.Square();
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("64", buffer);

    AssignHexString(bignum, "FFFFFFF");
    bignum.Square();
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFE0000001", buffer);

    AssignHexString(bignum, "FFFFFFFFFFFFFF");
    bignum.Square();
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FFFFFFFFFFFFFE00000000000001", buffer);
}

TEST(Convert, BignumAssignPowerUInt16)
{
    Bignum bignum;
    char buffer[kBufferSize];

    bignum.AssignPowerUInt16(1, 0);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    bignum.AssignPowerUInt16(1, 1);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    bignum.AssignPowerUInt16(1, 2);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    bignum.AssignPowerUInt16(2, 0);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    bignum.AssignPowerUInt16(2, 1);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("2", buffer);

    bignum.AssignPowerUInt16(2, 2);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("4", buffer);

    bignum.AssignPowerUInt16(16, 1);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10", buffer);

    bignum.AssignPowerUInt16(16, 2);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("100", buffer);

    bignum.AssignPowerUInt16(16, 5);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("100000", buffer);

    bignum.AssignPowerUInt16(16, 8);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("100000000", buffer);

    bignum.AssignPowerUInt16(16, 16);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000000000", buffer);

    bignum.AssignPowerUInt16(16, 30);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1000000000000000000000000000000", buffer);

    bignum.AssignPowerUInt16(10, 0);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    bignum.AssignPowerUInt16(10, 1);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("A", buffer);

    bignum.AssignPowerUInt16(10, 2);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("64", buffer);

    bignum.AssignPowerUInt16(10, 5);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("186A0", buffer);

    bignum.AssignPowerUInt16(10, 8);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("5F5E100", buffer);

    bignum.AssignPowerUInt16(10, 16);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("2386F26FC10000", buffer);

    bignum.AssignPowerUInt16(10, 30);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("C9F2C9CD04674EDEA40000000", buffer);

    bignum.AssignPowerUInt16(10, 31);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("7E37BE2022C0914B2680000000", buffer);

    bignum.AssignPowerUInt16(2, 0);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    bignum.AssignPowerUInt16(2, 100);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("10000000000000000000000000", buffer);

    bignum.AssignPowerUInt16(17, 0);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1", buffer);

    bignum.AssignPowerUInt16(17, 99);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("1942BB9853FAD924A3D4DD92B89B940E0207BEF05DB9C26BC1B757"
        "80BE0C5A2C2990E02A681224F34ED68558CE4C6E33760931",
        buffer);

    bignum.AssignPowerUInt16(0xFFFF, 99);
    EXPECT_TRUE(bignum.ToHexString(buffer, kBufferSize));
    EXPECT_STREQ("FF9D12F09B886C54E77E7439C7D2DED2D34F669654C0C2B6B8C288250"
        "5A2211D0E3DC9A61831349EAE674B11D56E3049D7BD79DAAD6C9FA2BA"
        "528E3A794299F2EE9146A324DAFE3E88967A0358233B543E233E575B9"
        "DD4E3AA7942146426C328FF55BFD5C45E0901B1629260AF9AE2F310C5"
        "50959FAF305C30116D537D80CF6EBDBC15C5694062AF1AC3D956D0A41"
        "B7E1B79FF11E21D83387A1CE1F5882B31E4B5D8DE415BDBE6854466DF"
        "343362267A7E8833119D31D02E18DB5B0E8F6A64B0ED0D0062FFFF",
        buffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(Convert, FastDtoaShortestVariousDoubles)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;
    bool status;

    double minDouble = 5e-324;
    status = FastDtoa::Dtoa(minDouble, FastDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("5", buffer.GetBuffer());
    EXPECT_EQ(-323, point);

    double maxDouble = 1.7976931348623157e308;
    status = FastDtoa::Dtoa(maxDouble, FastDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("17976931348623157", buffer.GetBuffer());
    EXPECT_EQ(309, point);

    status = FastDtoa::Dtoa(4294967272.0, FastDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("4294967272", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    status = FastDtoa::Dtoa(4.1855804968213567e298, FastDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("4185580496821357", buffer.GetBuffer());
    EXPECT_EQ(299, point);

    status = FastDtoa::Dtoa(5.5626846462680035e-309, FastDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("5562684646268003", buffer.GetBuffer());
    EXPECT_EQ(-308, point);

    status = FastDtoa::Dtoa(2147483648.0, FastDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("2147483648", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    status = FastDtoa::Dtoa(3.5844466002796428e+298, FastDtoaMode::Shortest, 0, buffer, length, point);
    if (status)
    {
        EXPECT_STREQ("35844466002796428", buffer.GetBuffer());
        EXPECT_EQ(299, point);
    }

    uint64_t smallestNormal64 = 0x0010000000000000ull;
    double v = Double(smallestNormal64).ToDouble();
    status = FastDtoa::Dtoa(v, FastDtoaMode::Shortest, 0, buffer, length, point);
    if (status)
    {
        EXPECT_STREQ("22250738585072014", buffer.GetBuffer());
        EXPECT_EQ(-307, point);
    }

    uint64_t largestDenormal64 = 0x000FFFFFFFFFFFFFull;
    v = Double(largestDenormal64).ToDouble();
    status = FastDtoa::Dtoa(v, FastDtoaMode::Shortest, 0, buffer, length, point);
    if (status)
    {
        EXPECT_STREQ("2225073858507201", buffer.GetBuffer());
        EXPECT_EQ(-307, point);
    }
}

TEST(Convert, FastDtoaShortestVariousFloats)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;
    bool status;

    float minFloat = 1e-45f;
    status = FastDtoa::Dtoa(minFloat, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-44, point);

    float maxFloat = 3.4028234e38f;
    status = FastDtoa::Dtoa(maxFloat, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("34028235", buffer.GetBuffer());
    EXPECT_EQ(39, point);

    status = FastDtoa::Dtoa(4294967272.0f, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("42949673", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    status = FastDtoa::Dtoa(3.32306998946228968226e+35f, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("332307", buffer.GetBuffer());
    EXPECT_EQ(36, point);

    status = FastDtoa::Dtoa(1.2341e-41f, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("12341", buffer.GetBuffer());
    EXPECT_EQ(-40, point);

    status = FastDtoa::Dtoa(3.3554432e7, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("33554432", buffer.GetBuffer());
    EXPECT_EQ(8, point);

    status = FastDtoa::Dtoa(3.26494756798464e14f, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("32649476", buffer.GetBuffer());
    EXPECT_EQ(15, point);

    status = FastDtoa::Dtoa(3.91132223637771935344e37f, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    if (status)
    {
        EXPECT_STREQ("39113222", buffer.GetBuffer());
        EXPECT_EQ(38, point);
    }

    uint32_t smallestNormal32 = 0x00800000;
    float v = Single(smallestNormal32).ToFloat();
    status = FastDtoa::Dtoa(v, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    if (status)
    {
        EXPECT_STREQ("11754944", buffer.GetBuffer());
        EXPECT_EQ(-37, point);
    }

    uint32_t largestDenormal32 = 0x007FFFFF;
    v = Single(largestDenormal32).ToFloat();
    status = FastDtoa::Dtoa(v, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("11754942", buffer.GetBuffer());
    EXPECT_EQ(-37, point);
}

TEST(Convert, FastDtoaPrecisionVariousDoubles)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;
    bool status;

    status = FastDtoa::Dtoa(1.0, FastDtoaMode::Precision, 3, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_TRUE(3 >= length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    status = FastDtoa::Dtoa(1.5, FastDtoaMode::Precision, 10, buffer, length, point);
    if (status)
    {
        EXPECT_TRUE(10 >= length);
        TrimRepresentation(buffer);
        EXPECT_STREQ("15", buffer.GetBuffer());
        EXPECT_EQ(1, point);
    }

    double minDouble = 5e-324;
    status = FastDtoa::Dtoa(minDouble, FastDtoaMode::Precision, 5, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("49407", buffer.GetBuffer());
    EXPECT_EQ(-323, point);

    double maxDouble = 1.7976931348623157e308;
    status = FastDtoa::Dtoa(maxDouble, FastDtoaMode::Precision, 7, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("1797693", buffer.GetBuffer());
    EXPECT_EQ(309, point);

    status = FastDtoa::Dtoa(4294967272.0, FastDtoaMode::Precision, 14, buffer, length, point);
    if (status)
    {
        EXPECT_TRUE(14 >= length);
        TrimRepresentation(buffer);
        EXPECT_STREQ("4294967272", buffer.GetBuffer());
        EXPECT_EQ(10, point);
    }

    status = FastDtoa::Dtoa(4.1855804968213567e298, FastDtoaMode::Precision, 17, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("41855804968213567", buffer.GetBuffer());
    EXPECT_EQ(299, point);

    status = FastDtoa::Dtoa(5.5626846462680035e-309, FastDtoaMode::Precision, 1, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("6", buffer.GetBuffer());
    EXPECT_EQ(-308, point);

    status = FastDtoa::Dtoa(2147483648.0, FastDtoaMode::Precision, 5, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("21475", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    status = FastDtoa::Dtoa(3.5844466002796428e+298, FastDtoaMode::Precision, 10, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_TRUE(10 >= length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("35844466", buffer.GetBuffer());
    EXPECT_EQ(299, point);

    uint64_t smallestNormal64 = 0x0010000000000000ull;
    double v = Double(smallestNormal64).ToDouble();
    status = FastDtoa::Dtoa(v, FastDtoaMode::Precision, 17, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("22250738585072014", buffer.GetBuffer());
    EXPECT_EQ(-307, point);

    uint64_t largestDenormal64 = 0x000FFFFFFFFFFFFFull;
    v = Double(largestDenormal64).ToDouble();
    status = FastDtoa::Dtoa(v, FastDtoaMode::Precision, 17, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_TRUE(20 >= length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("22250738585072009", buffer.GetBuffer());
    EXPECT_EQ(-307, point);

    v = 3.3161339052167390562200598e-237;
    status = FastDtoa::Dtoa(v, FastDtoaMode::Precision, 18, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("331613390521673906", string(buffer.GetBuffer()));
    EXPECT_EQ(-236, point);

    v = 7.9885183916008099497815232e+191;
    status = FastDtoa::Dtoa(v, FastDtoaMode::Precision, 4, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_STREQ("7989", buffer.GetBuffer());
    EXPECT_EQ(192, point);
}

TEST(Convert, FastDtoaGayShortest)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    bool status;
    size_t length;
    int point;
    int succeeded = 0;
    int total = 0;
    bool neededMaxLength = false;

    const ArrayView<Testing::PrecomputedShortest> precomputed = Testing::PrecomputedShortestRepresentations();
    for (size_t i = 0; i < precomputed.GetSize(); ++i)
    {
        const Testing::PrecomputedShortest& currentTest = precomputed[i];
        ++total;
        double v = currentTest.v;
        status = FastDtoa::Dtoa(v, FastDtoaMode::Shortest, 0, buffer, length, point);
        EXPECT_TRUE(kFastDtoaMaximalLength >= length);
        if (!status)
            continue;
        if (length == kFastDtoaMaximalLength)
            neededMaxLength = true;
        succeeded++;
        EXPECT_EQ(currentTest.decimalPoint, point);
        EXPECT_STREQ(currentTest.representation, buffer.GetBuffer());
    }

    EXPECT_TRUE(succeeded * 1.0 / total > 0.99);
    EXPECT_TRUE(neededMaxLength);
}

TEST(Convert, FastDtoaGayShortestSingle)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    bool status;
    size_t length;
    int point;
    int succeeded = 0;
    int total = 0;
    bool neededMaxLength = false;

    const ArrayView<Testing::PrecomputedShortestSingle> precomputed =
        Testing::PrecomputedShortestSingleRepresentations();
    for (size_t i = 0; i < precomputed.GetSize(); ++i)
    {
        const Testing::PrecomputedShortestSingle& currentTest = precomputed[i];
        total++;
        float v = currentTest.v;
        status = FastDtoa::Dtoa(v, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
        EXPECT_TRUE(kFastDtoaMaximalSingleLength >= length);
        if (!status)
            continue;
        if (length == kFastDtoaMaximalSingleLength)
            neededMaxLength = true;
        succeeded++;
        EXPECT_EQ(currentTest.decimalPoint, point);
        EXPECT_STREQ(currentTest.representation, buffer.GetBuffer());
    }
    EXPECT_TRUE(succeeded * 1.0 / total > 0.98);
    EXPECT_TRUE(neededMaxLength);
}

TEST(Convert, FastDtoaGayPrecision)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    bool status;
    size_t length;
    int point;
    int succeeded = 0;
    int total = 0;
    int succeeded15 = 0;
    int total15 = 0;

    const ArrayView<Testing::PrecomputedPrecision> precomputed = Testing::PrecomputedPrecisionRepresentations();
    for (size_t i = 0; i < precomputed.GetSize(); ++i) {
        const Testing::PrecomputedPrecision& currentTest = precomputed[i];
        double v = currentTest.v;
        size_t numberDigits = static_cast<size_t>(currentTest.numberDigits);
        total++;
        if (numberDigits <= 15) total15++;
        status = FastDtoa::Dtoa(v, FastDtoaMode::Precision, numberDigits, buffer, length, point);
        EXPECT_TRUE(numberDigits >= length);
        if (!status)
            continue;
        succeeded++;
        if (numberDigits <= 15)
            succeeded15++;
        TrimRepresentation(buffer);
        EXPECT_EQ(currentTest.decimalPoint, point);
        EXPECT_STREQ(currentTest.representation, buffer.GetBuffer());
    }

    EXPECT_TRUE(succeeded * 1.0 / total > 0.85);
    EXPECT_TRUE(succeeded15 * 1.0 / total15 > 0.9999);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(Convert, FastFixedVariousDoubles)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;

    EXPECT_TRUE(FixedDtoa::Dtoa(1.0, 1, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1.0, 15, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1.0, 0, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0xFFFFFFFF, 5, buffer, length, point));
    EXPECT_STREQ("4294967295", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(4294967296.0, 5, buffer, length, point));
    EXPECT_STREQ("4294967296", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1e21, 5, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(22, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(999999999999999868928.00, 2, buffer, length, point));
    EXPECT_STREQ("999999999999999868928", buffer.GetBuffer());
    EXPECT_EQ(21, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(6.9999999999999989514240000e+21, 5, buffer, length, point));
    EXPECT_STREQ("6999999999999998951424", buffer.GetBuffer());
    EXPECT_EQ(22, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1.5, 5, buffer, length, point));
    EXPECT_STREQ("15", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1.55, 5, buffer, length, point));
    EXPECT_STREQ("155", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1.55, 1, buffer, length, point));
    EXPECT_STREQ("16", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1.00000001, 15, buffer, length, point));
    EXPECT_STREQ("100000001", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.1, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(0, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.01, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.001, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-2, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0001, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-3, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00001, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-4, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000001, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-5, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000001, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-6, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000001, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-7, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000001, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-8, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000001, 15, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-9, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000000001, 15, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-10, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000001, 15, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-11, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000000001, 15, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-12, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000000000001, 15, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-13, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000001, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-14, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000000000001, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-15, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000000000000001, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-16, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000000001, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-17, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000000000000001, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-18, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000000000000000001, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-19, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.10000000004, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(0, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.01000000004, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00100000004, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-2, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00010000004, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-3, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00001000004, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-4, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000100004, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-5, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000010004, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-6, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000001004, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-7, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000000104, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-8, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000001000004, 15, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-9, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000000100004, 15, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-10, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000000010004, 15, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-11, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000000001004, 15, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-12, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000000000104, 15, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-13, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000001000004, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-14, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000000100004, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-15, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000000010004, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-16, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000000001004, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-17, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000000000104, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-18, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000000000014, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-19, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.10000000006, 10, buffer, length, point));
    EXPECT_STREQ("1000000001", buffer.GetBuffer());
    EXPECT_EQ(0, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.01000000006, 10, buffer, length, point));
    EXPECT_STREQ("100000001", buffer.GetBuffer());
    EXPECT_EQ(-1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00100000006, 10, buffer, length, point));
    EXPECT_STREQ("10000001", buffer.GetBuffer());
    EXPECT_EQ(-2, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00010000006, 10, buffer, length, point));
    EXPECT_STREQ("1000001", buffer.GetBuffer());
    EXPECT_EQ(-3, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00001000006, 10, buffer, length, point));
    EXPECT_STREQ("100001", buffer.GetBuffer());
    EXPECT_EQ(-4, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000100006, 10, buffer, length, point));
    EXPECT_STREQ("10001", buffer.GetBuffer());
    EXPECT_EQ(-5, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000010006, 10, buffer, length, point));
    EXPECT_STREQ("1001", buffer.GetBuffer());
    EXPECT_EQ(-6, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000001006, 10, buffer, length, point));
    EXPECT_STREQ("101", buffer.GetBuffer());
    EXPECT_EQ(-7, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000000106, 10, buffer, length, point));
    EXPECT_STREQ("11", buffer.GetBuffer());
    EXPECT_EQ(-8, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000001000006, 15, buffer, length, point));
    EXPECT_STREQ("100001", buffer.GetBuffer());
    EXPECT_EQ(-9, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000000100006, 15, buffer, length, point));
    EXPECT_STREQ("10001", buffer.GetBuffer());
    EXPECT_EQ(-10, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000000010006, 15, buffer, length, point));
    EXPECT_STREQ("1001", buffer.GetBuffer());
    EXPECT_EQ(-11, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000000001006, 15, buffer, length, point));
    EXPECT_STREQ("101", buffer.GetBuffer());
    EXPECT_EQ(-12, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000000000000106, 15, buffer, length, point));
    EXPECT_STREQ("11", buffer.GetBuffer());
    EXPECT_EQ(-13, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000001000006, 20, buffer, length, point));
    EXPECT_STREQ("100001", buffer.GetBuffer());
    EXPECT_EQ(-14, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000000100006, 20, buffer, length, point));
    EXPECT_STREQ("10001", buffer.GetBuffer());
    EXPECT_EQ(-15, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000000010006, 20, buffer, length, point));
    EXPECT_STREQ("1001", buffer.GetBuffer());
    EXPECT_EQ(-16, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000000001006, 20, buffer, length, point));
    EXPECT_STREQ("101", buffer.GetBuffer());
    EXPECT_EQ(-17, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000000000106, 20, buffer, length, point));
    EXPECT_STREQ("11", buffer.GetBuffer());
    EXPECT_EQ(-18, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000000000000000016, 20, buffer, length, point));
    EXPECT_STREQ("2", buffer.GetBuffer());
    EXPECT_EQ(-19, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.6, 0, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.96, 1, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.996, 2, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.9996, 3, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.99996, 4, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.999996, 5, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.9999996, 6, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.99999996, 7, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.999999996, 8, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.9999999996, 9, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.99999999996, 10, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.999999999996, 11, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.9999999999996, 12, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.99999999999996, 13, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.999999999999996, 14, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.9999999999999996, 15, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00999999999999996, 16, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000999999999999996, 17, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-2, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.0000999999999999996, 18, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-3, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.00000999999999999996, 19, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-4, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.000000999999999999996, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-5, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(323423.234234, 10, buffer, length, point));
    EXPECT_STREQ("323423234234", buffer.GetBuffer());
    EXPECT_EQ(6, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(12345678.901234, 4, buffer, length, point));
    EXPECT_STREQ("123456789012", buffer.GetBuffer());
    EXPECT_EQ(8, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(98765.432109, 5, buffer, length, point));
    EXPECT_STREQ("9876543211", buffer.GetBuffer());
    EXPECT_EQ(5, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(42, 20, buffer, length, point));
    EXPECT_STREQ("42", buffer.GetBuffer());
    EXPECT_EQ(2, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(0.5, 0, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1e-23, 10, buffer, length, point));
    EXPECT_STREQ("", buffer.GetBuffer());
    EXPECT_EQ(-10, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1e-123, 2, buffer, length, point));
    EXPECT_STREQ("", buffer.GetBuffer());
    EXPECT_EQ(-2, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1e-123, 0, buffer, length, point));
    EXPECT_STREQ("", buffer.GetBuffer());
    EXPECT_EQ(0, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1e-23, 20, buffer, length, point));
    EXPECT_STREQ("", buffer.GetBuffer());
    EXPECT_EQ(-20, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1e-21, 20, buffer, length, point));
    EXPECT_STREQ("", buffer.GetBuffer());
    EXPECT_EQ(-20, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1e-22, 20, buffer, length, point));
    EXPECT_STREQ("", buffer.GetBuffer());
    EXPECT_EQ(-20, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(6e-21, 20, buffer, length, point));
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-19, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(9.1193616301674545152000000e+19, 0, buffer, length, point));
    EXPECT_STREQ("91193616301674545152", buffer.GetBuffer());
    EXPECT_EQ(20, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(4.8184662102767651659096515e-04, 19, buffer, length, point));
    EXPECT_STREQ("4818466210276765", buffer.GetBuffer());
    EXPECT_EQ(-3, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1.9023164229540652612705182e-23, 8, buffer, length, point));
    EXPECT_STREQ("", buffer.GetBuffer());
    EXPECT_EQ(-8, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(1000000000000000128.0, 0, buffer, length, point));
    EXPECT_STREQ("1000000000000000128", buffer.GetBuffer());
    EXPECT_EQ(19, point);

    EXPECT_TRUE(FixedDtoa::Dtoa(2.10861548515811875e+15, 17, buffer, length, point));
    EXPECT_STREQ("210861548515811875", buffer.GetBuffer());
    EXPECT_EQ(16, point);
}

TEST(Convert, FastFixedDtoaGayFixed)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    bool status;
    size_t length = 0;
    int point;

    const ArrayView<Testing::PrecomputedFixed> precomputed = Testing::PrecomputedFixedRepresentations();
    for (size_t i = 0; i < precomputed.GetSize(); ++i)
    {
        ::memset(bufferContainer, 0, sizeof(bufferContainer));

        const Testing::PrecomputedFixed& currentTest = precomputed[i];
        double v = currentTest.v;
        size_t numberDigits = static_cast<size_t>(currentTest.numberDigits);
        status = FixedDtoa::Dtoa(v, numberDigits, buffer, length, point);
        EXPECT_TRUE(status);
        EXPECT_EQ(currentTest.decimalPoint, point);
        EXPECT_GE((int)numberDigits, (int)length - point);
        EXPECT_STREQ(currentTest.representation, buffer.GetBuffer());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(Convert, BignumDtoaVariousDoubles)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;

    BignumDtoa::Dtoa(1.0, BignumDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    BignumDtoa::Dtoa(1.0, BignumDtoaMode::Fixed, 3, buffer, length, point);
    EXPECT_GE(3, length - point);
    TrimRepresentation(buffer);
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    BignumDtoa::Dtoa(1.0, BignumDtoaMode::Precision, 3, buffer, length, point);
    EXPECT_GE(3, length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    BignumDtoa::Dtoa(1.5, BignumDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_STREQ("15", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    BignumDtoa::Dtoa(1.5, BignumDtoaMode::Fixed, 10, buffer, length, point);
    EXPECT_GE(10, length - point);
    TrimRepresentation(buffer);
    EXPECT_STREQ("15", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    BignumDtoa::Dtoa(1.5, BignumDtoaMode::Precision, 10, buffer, length, point);
    EXPECT_GE(10, length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("15", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    double min_double = 5e-324;
    BignumDtoa::Dtoa(min_double, BignumDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_STREQ("5", buffer.GetBuffer());
    EXPECT_EQ(-323, point);

    BignumDtoa::Dtoa(min_double, BignumDtoaMode::Fixed, 5, buffer, length, point);
    EXPECT_GE(5, length - point);
    TrimRepresentation(buffer);
    EXPECT_STREQ("", buffer.GetBuffer());

    BignumDtoa::Dtoa(min_double, BignumDtoaMode::Precision, 5, buffer, length, point);
    EXPECT_GE(5, length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("49407", buffer.GetBuffer());
    EXPECT_EQ(-323, point);

    double max_double = 1.7976931348623157e308;
    BignumDtoa::Dtoa(max_double, BignumDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_STREQ("17976931348623157", buffer.GetBuffer());
    EXPECT_EQ(309, point);

    BignumDtoa::Dtoa(max_double, BignumDtoaMode::Precision, 7, buffer, length, point);
    EXPECT_GE(7, length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("1797693", buffer.GetBuffer());
    EXPECT_EQ(309, point);

    BignumDtoa::Dtoa(4294967272.0, BignumDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_STREQ("4294967272", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    BignumDtoa::Dtoa(4294967272.0, BignumDtoaMode::Fixed, 5, buffer, length, point);
    EXPECT_STREQ("429496727200000", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    BignumDtoa::Dtoa(4294967272.0, BignumDtoaMode::Precision, 14, buffer, length, point);
    EXPECT_GE(14, length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("4294967272", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    BignumDtoa::Dtoa(4.1855804968213567e298, BignumDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_STREQ("4185580496821357", buffer.GetBuffer());
    EXPECT_EQ(299, point);

    BignumDtoa::Dtoa(4.1855804968213567e298, BignumDtoaMode::Precision, 20, buffer, length, point);
    EXPECT_GE(20, length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("41855804968213567225", buffer.GetBuffer());
    EXPECT_EQ(299, point);

    BignumDtoa::Dtoa(5.5626846462680035e-309, BignumDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_STREQ("5562684646268003", buffer.GetBuffer());
    EXPECT_EQ(-308, point);

    BignumDtoa::Dtoa(5.5626846462680035e-309, BignumDtoaMode::Precision, 1, buffer, length, point);
    EXPECT_GE(1, length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("6", buffer.GetBuffer());
    EXPECT_EQ(-308, point);

    BignumDtoa::Dtoa(2147483648.0, BignumDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_STREQ("2147483648", buffer.GetBuffer());
    EXPECT_EQ(10, point);


    BignumDtoa::Dtoa(2147483648.0, BignumDtoaMode::Fixed, 2, buffer, length, point);
    EXPECT_GE(2, length - point);
    TrimRepresentation(buffer);
    EXPECT_STREQ("2147483648", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    BignumDtoa::Dtoa(2147483648.0, BignumDtoaMode::Precision, 5, buffer, length, point);
    EXPECT_GE(5, length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("21475", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    BignumDtoa::Dtoa(3.5844466002796428e+298, BignumDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_STREQ("35844466002796428", buffer.GetBuffer());
    EXPECT_EQ(299, point);

    BignumDtoa::Dtoa(3.5844466002796428e+298, BignumDtoaMode::Precision, 10, buffer, length, point);
    EXPECT_GE(10, length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("35844466", buffer.GetBuffer());
    EXPECT_EQ(299, point);

    uint64_t smallestNormal64 = 0x0010000000000000ull;
    double v = Double(smallestNormal64).ToDouble();
    BignumDtoa::Dtoa(v, BignumDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_STREQ("22250738585072014", buffer.GetBuffer());
    EXPECT_EQ(-307, point);

    BignumDtoa::Dtoa(v, BignumDtoaMode::Precision, 20, buffer, length, point);
    EXPECT_GE(20, length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("22250738585072013831", buffer.GetBuffer());
    EXPECT_EQ(-307, point);

    uint64_t largestDenormal64 = 0x000FFFFFFFFFFFFFull;
    v = Double(largestDenormal64).ToDouble();
    BignumDtoa::Dtoa(v, BignumDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_STREQ("2225073858507201", buffer.GetBuffer());
    EXPECT_EQ(-307, point);

    BignumDtoa::Dtoa(v, BignumDtoaMode::Precision, 20, buffer, length, point);
    EXPECT_GE(20, length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("2225073858507200889", buffer.GetBuffer());
    EXPECT_EQ(-307, point);

    BignumDtoa::Dtoa(4128420500802942e-24, BignumDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_STREQ("4128420500802942", buffer.GetBuffer());
    EXPECT_EQ(-8, point);

    v = 3.9292015898194142585311918e-10;
    BignumDtoa::Dtoa(v, BignumDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_STREQ("39292015898194143", buffer.GetBuffer());

    v = 4194304.0;
    BignumDtoa::Dtoa(v, BignumDtoaMode::Fixed, 5, buffer, length, point);
    EXPECT_GE(5, length - point);
    TrimRepresentation(buffer);
    EXPECT_STREQ("4194304", buffer.GetBuffer());

    v = 3.3161339052167390562200598e-237;
    BignumDtoa::Dtoa(v, BignumDtoaMode::Precision, 19, buffer, length, point);
    EXPECT_GE(19, length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("3316133905216739056", buffer.GetBuffer());
    EXPECT_EQ(-236, point);

    v = 7.9885183916008099497815232e+191;
    BignumDtoa::Dtoa(v, BignumDtoaMode::Precision, 4, buffer, length, point);
    EXPECT_GE(4, length);
    TrimRepresentation(buffer);
    EXPECT_STREQ("7989", buffer.GetBuffer());
    EXPECT_EQ(192, point);

    v = 1.0000000000000012800000000e+17;
    BignumDtoa::Dtoa(v, BignumDtoaMode::Fixed, 1, buffer, length, point);
    EXPECT_GE(1, length - point);
    TrimRepresentation(buffer);
    EXPECT_STREQ("100000000000000128", buffer.GetBuffer());
    EXPECT_EQ(18, point);
}

TEST(Convert,BignumDtoaShortestVariousFloats)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;

    float minFloat = 1e-45f;
    BignumDtoa::Dtoa(minFloat, BignumDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_STREQ("1", buffer.GetBuffer());
    EXPECT_EQ(-44, point);

    float maxFloat = 3.4028234e38f;
    BignumDtoa::Dtoa(maxFloat, BignumDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_STREQ("34028235", buffer.GetBuffer());
    EXPECT_EQ(39, point);

    BignumDtoa::Dtoa(4294967272.0f, BignumDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_STREQ("42949673", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    BignumDtoa::Dtoa(3.32306998946228968226e+35f, BignumDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_STREQ("332307", buffer.GetBuffer());
    EXPECT_EQ(36, point);

    BignumDtoa::Dtoa(1.2341e-41f, BignumDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_STREQ("12341", buffer.GetBuffer());
    EXPECT_EQ(-40, point);

    BignumDtoa::Dtoa(3.3554432e7, BignumDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_STREQ("33554432", buffer.GetBuffer());
    EXPECT_EQ(8, point);

    BignumDtoa::Dtoa(3.26494756798464e14f, BignumDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_STREQ("32649476", buffer.GetBuffer());
    EXPECT_EQ(15, point);

    BignumDtoa::Dtoa(3.91132223637771935344e37f, BignumDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_STREQ("39113222", buffer.GetBuffer());
    EXPECT_EQ(38, point);

    uint32_t smallestNormal32 = 0x00800000;
    double v = Single(smallestNormal32).ToFloat();
    BignumDtoa::Dtoa(v, BignumDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_STREQ("11754944", buffer.GetBuffer());
    EXPECT_EQ(-37, point);

    uint32_t largestDenormal32 = 0x007FFFFF;
    v = Single(largestDenormal32).ToFloat();
    BignumDtoa::Dtoa(v, BignumDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_STREQ("11754942", buffer.GetBuffer());
    EXPECT_EQ(-37, point);
}

TEST(Convert, BignumDtoaGayShortest)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;

    const ArrayView<Testing::PrecomputedShortest> precomputed = Testing::PrecomputedShortestRepresentations();
    for (size_t i = 0; i < precomputed.GetSize(); ++i)
    {
        const Testing::PrecomputedShortest& currentTest = precomputed[i];
        double v = currentTest.v;
        BignumDtoa::Dtoa(v, BignumDtoaMode::Shortest, 0, buffer, length, point);
        EXPECT_EQ(currentTest.decimalPoint, point);
        EXPECT_STREQ(currentTest.representation, buffer.GetBuffer());
    }
}

TEST(Convert, BignumDtoaGayShortestSingle)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;

    const ArrayView<Testing::PrecomputedShortestSingle> precomputed =
        Testing::PrecomputedShortestSingleRepresentations();
    for (size_t i = 0; i < precomputed.GetSize(); ++i)
    {
        const Testing::PrecomputedShortestSingle& currentTest = precomputed[i];
        float v = currentTest.v;
        BignumDtoa::Dtoa(v, BignumDtoaMode::ShortestSingle, 0, buffer, length, point);
        EXPECT_EQ(currentTest.decimalPoint, point);
        EXPECT_STREQ(currentTest.representation, buffer.GetBuffer());
    }
}

TEST(Convert, BignumDtoaGayFixed)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;

    const ArrayView<Testing::PrecomputedFixed> precomputed = Testing::PrecomputedFixedRepresentations();
    for (size_t i = 0; i < precomputed.GetSize(); ++i)
    {
        const Testing::PrecomputedFixed& currentTest = precomputed[i];
        double v = currentTest.v;
        int numberDigits = currentTest.numberDigits;
        BignumDtoa::Dtoa(v, BignumDtoaMode::Fixed, numberDigits, buffer, length, point);
        EXPECT_EQ(currentTest.decimalPoint, point);
        EXPECT_GE(numberDigits, (int)length - point);
        TrimRepresentation(buffer);
        EXPECT_STREQ(currentTest.representation, buffer.GetBuffer());
    }
}

TEST(Convert, BignumDtoaGayPrecision)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;

    const ArrayView<Testing::PrecomputedPrecision> precomputed = Testing::PrecomputedPrecisionRepresentations();
    for (size_t i = 0; i < precomputed.GetSize(); ++i)
    {
        const Testing::PrecomputedPrecision& currentTest = precomputed[i];
        double v = currentTest.v;
        int numberDigits = currentTest.numberDigits;
        BignumDtoa::Dtoa(v, BignumDtoaMode::Precision, numberDigits, buffer, length, point);
        EXPECT_EQ(currentTest.decimalPoint, point);
        EXPECT_GE(numberDigits, length);
        TrimRepresentation(buffer);
        EXPECT_STREQ(currentTest.representation, buffer.GetBuffer());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(Convert, DoubleToShortest)
{
    const int kBufferSize = 128;
    char buffer[kBufferSize];
    StringBuilder<> builder(buffer, kBufferSize);
    DtoaFlags flags = static_cast<DtoaFlags>(
        static_cast<int>(DtoaFlags::UniqueZero) |
        static_cast<int>(DtoaFlags::EmitPositiveExponentSign));
    DoubleToStringConverter<> dc(flags, nullptr, nullptr, 'e', -6, 21, 0, 0);

    EXPECT_TRUE(dc.ToShortest(0.0, builder));
    EXPECT_STREQ("0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(12345.0, builder));
    EXPECT_STREQ("12345", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(12345e23, builder));
    EXPECT_STREQ("1.2345e+27", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(1e21, builder));
    EXPECT_STREQ("1e+21", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(1e20, builder));
    EXPECT_STREQ("100000000000000000000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(111111111111111111111.0, builder));
    EXPECT_STREQ("111111111111111110000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(1111111111111111111111.0, builder));
    EXPECT_STREQ("1.1111111111111111e+21", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(11111111111111111111111.0, builder));
    EXPECT_STREQ("1.1111111111111111e+22", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.00001, builder));
    EXPECT_STREQ("-0.00001", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.000001, builder));
    EXPECT_STREQ("-0.000001", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.0000001, builder));
    EXPECT_STREQ("-1e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.0, builder));
    EXPECT_STREQ("0", builder.Finalize());

    flags = DtoaFlags::Default;
    DoubleToStringConverter<> dc2(flags, nullptr, nullptr, 'e', -1, 1, 0, 0);
    builder.Reset();
    EXPECT_TRUE(dc2.ToShortest(0.1, builder));
    EXPECT_STREQ("0.1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToShortest(0.01, builder));
    EXPECT_STREQ("1e-2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToShortest(1.0, builder));
    EXPECT_STREQ("1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToShortest(10.0, builder));
    EXPECT_STREQ("1e1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToShortest(-0.0, builder));
    EXPECT_STREQ("-0", builder.Finalize());

    flags = static_cast<DtoaFlags>(
        static_cast<int>(DtoaFlags::EmitTrailingDecimalPoint) |
        static_cast<int>(DtoaFlags::EmitTrailingZeroAfterPoint));
    DoubleToStringConverter<> dc3(flags, nullptr, nullptr, 'E', -5, 5, 0, 0);

    builder.Reset();
    EXPECT_TRUE(dc3.ToShortest(0.1, builder));
    EXPECT_STREQ("0.1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc3.ToShortest(1.0, builder));
    EXPECT_STREQ("1.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc3.ToShortest(10000.0, builder));
    EXPECT_STREQ("10000.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc3.ToShortest(100000.0, builder));
    EXPECT_STREQ("1E5", builder.Finalize());

    // Test the examples in the comments of ToShortest.
    flags = DtoaFlags::EmitPositiveExponentSign;
    DoubleToStringConverter<> dc4(flags, nullptr, nullptr, 'e', -6, 21, 0, 0);

    builder.Reset();
    EXPECT_TRUE(dc4.ToShortest(0.000001, builder));
    EXPECT_STREQ("0.000001", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToShortest(0.0000001, builder));
    EXPECT_STREQ("1e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToShortest(111111111111111111111.0, builder));
    EXPECT_STREQ("111111111111111110000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToShortest(100000000000000000000.0, builder));
    EXPECT_STREQ("100000000000000000000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToShortest(1111111111111111111111.0, builder));
    EXPECT_STREQ("1.1111111111111111e+21", builder.Finalize());

    // Test special value handling.
    DoubleToStringConverter<> dc5(flags, nullptr, nullptr, 'e', 0, 0, 0, 0);

    builder.Reset();
    EXPECT_TRUE(!dc5.ToShortest(Double::Infinity(), builder));

    builder.Reset();
    EXPECT_TRUE(!dc5.ToShortest(-Double::Infinity(), builder));

    builder.Reset();
    EXPECT_TRUE(!dc5.ToShortest(Double::Nan(), builder));

    builder.Reset();
    EXPECT_TRUE(!dc5.ToShortest(-Double::Nan(), builder));

    DoubleToStringConverter<> dc6(flags, "Infinity", "NaN", 'e', 0, 0, 0, 0);

    builder.Reset();
    EXPECT_TRUE(dc6.ToShortest(Double::Infinity(), builder));
    EXPECT_STREQ("Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc6.ToShortest(-Double::Infinity(), builder));
    EXPECT_STREQ("-Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc6.ToShortest(Double::Nan(), builder));
    EXPECT_STREQ("NaN", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc6.ToShortest(-Double::Nan(), builder));
    EXPECT_STREQ("NaN", builder.Finalize());
}

TEST(Convert, DoubleToShortestSingle)
{
    const int kBufferSize = 128;
    char buffer[kBufferSize];
    StringBuilder<> builder(buffer, kBufferSize);
    DtoaFlags flags = static_cast<DtoaFlags>(
        static_cast<int>(DtoaFlags::UniqueZero) |
        static_cast<int>(DtoaFlags::EmitPositiveExponentSign));
    DoubleToStringConverter<> dc(flags, nullptr, nullptr, 'e', -6, 21, 0, 0);

    EXPECT_TRUE(dc.ToShortestSingle(0.0f, builder));
    EXPECT_STREQ("0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortestSingle(12345.0f, builder));
    EXPECT_STREQ("12345", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortestSingle(12345e23f, builder));
    EXPECT_STREQ("1.2345e+27", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortestSingle(1e21f, builder));
    EXPECT_STREQ("1e+21", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortestSingle(1e20f, builder));
    EXPECT_STREQ("100000000000000000000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortestSingle(111111111111111111111.0f, builder));
    EXPECT_STREQ("111111110000000000000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortestSingle(1111111111111111111111.0f, builder));
    EXPECT_STREQ("1.11111114e+21", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortestSingle(11111111111111111111111.0f, builder));
    EXPECT_STREQ("1.1111111e+22", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortestSingle(-0.00001f, builder));
    EXPECT_STREQ("-0.00001", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortestSingle(-0.000001f, builder));
    EXPECT_STREQ("-0.000001", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortestSingle(-0.0000001f, builder));
    EXPECT_STREQ("-1e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortestSingle(-0.0f, builder));
    EXPECT_STREQ("0", builder.Finalize());

    flags = DtoaFlags::Default;
    DoubleToStringConverter<> dc2(flags, nullptr, nullptr, 'e', -1, 1, 0, 0);
    builder.Reset();
    EXPECT_TRUE(dc2.ToShortestSingle(0.1f, builder));
    EXPECT_STREQ("0.1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToShortestSingle(0.01f, builder));
    EXPECT_STREQ("1e-2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToShortestSingle(1.0f, builder));
    EXPECT_STREQ("1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToShortestSingle(10.0f, builder));
    EXPECT_STREQ("1e1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToShortestSingle(-0.0f, builder));
    EXPECT_STREQ("-0", builder.Finalize());

    flags = static_cast<DtoaFlags>(
        static_cast<int>(DtoaFlags::EmitTrailingDecimalPoint) |
        static_cast<int>(DtoaFlags::EmitTrailingZeroAfterPoint));
    DoubleToStringConverter<> dc3(flags, nullptr, nullptr, 'E', -5, 5, 0, 0);

    builder.Reset();
    EXPECT_TRUE(dc3.ToShortestSingle(0.1f, builder));
    EXPECT_STREQ("0.1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc3.ToShortestSingle(1.0f, builder));
    EXPECT_STREQ("1.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc3.ToShortestSingle(10000.0f, builder));
    EXPECT_STREQ("10000.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc3.ToShortestSingle(100000.0f, builder));
    EXPECT_STREQ("1E5", builder.Finalize());

    // Test the examples in the comments of ToShortestSingle.
    flags = DtoaFlags::EmitPositiveExponentSign;
    DoubleToStringConverter<> dc4(flags, nullptr, nullptr, 'e', -6, 21, 0, 0);

    builder.Reset();
    EXPECT_TRUE(dc4.ToShortestSingle(0.000001f, builder));
    EXPECT_STREQ("0.000001", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToShortestSingle(0.0000001f, builder));
    EXPECT_STREQ("1e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToShortestSingle(111111111111111111111.0f, builder));
    EXPECT_STREQ("111111110000000000000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToShortestSingle(100000000000000000000.0f, builder));
    EXPECT_STREQ("100000000000000000000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToShortestSingle(1111111111111111111111.0f, builder));
    EXPECT_STREQ("1.11111114e+21", builder.Finalize());

    // Test special value handling.
    DoubleToStringConverter<> dc5(flags, nullptr, nullptr, 'e', 0, 0, 0, 0);

    builder.Reset();
    EXPECT_TRUE(!dc5.ToShortestSingle(Single::Infinity(), builder));

    builder.Reset();
    EXPECT_TRUE(!dc5.ToShortestSingle(-Single::Infinity(), builder));

    builder.Reset();
    EXPECT_TRUE(!dc5.ToShortestSingle(Single::Nan(), builder));

    builder.Reset();
    EXPECT_TRUE(!dc5.ToShortestSingle(-Single::Nan(), builder));

    DoubleToStringConverter<> dc6(flags, "Infinity", "NaN", 'e', 0, 0, 0, 0);

    builder.Reset();
    EXPECT_TRUE(dc6.ToShortestSingle(Single::Infinity(), builder));
    EXPECT_STREQ("Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc6.ToShortestSingle(-Single::Infinity(), builder));
    EXPECT_STREQ("-Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc6.ToShortestSingle(Single::Nan(), builder));
    EXPECT_STREQ("NaN", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc6.ToShortestSingle(-Single::Nan(), builder));
    EXPECT_STREQ("NaN", builder.Finalize());
}

TEST(Convert, DoubleToFixed)
{
    const int kBufferSize = 128;
    char buffer[kBufferSize];
    StringBuilder<> builder(buffer, kBufferSize);
    DtoaFlags flags = static_cast<DtoaFlags>(
        static_cast<int>(DtoaFlags::EmitPositiveExponentSign) |
        static_cast<int>(DtoaFlags::UniqueZero));
    DoubleToStringConverter<> dc(flags, "Infinity", "NaN", 'e',
        0, 0, 0, 0);  // Padding zeroes.

    EXPECT_TRUE(dc.ToFixed(0.0, 0, builder));
    EXPECT_STREQ("0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.0, 0, builder));
    EXPECT_STREQ("0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.0, 1, builder));
    EXPECT_STREQ("0.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.0, 1, builder));
    EXPECT_STREQ("0.0", builder.Finalize());

    EXPECT_TRUE(DoubleToStringConverter<>::kMaxFixedDigitsBeforePoint == 60);
    EXPECT_TRUE(DoubleToStringConverter<>::kMaxFixedDigitsAfterPoint == 60);
    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(
        0.0, DoubleToStringConverter<>::kMaxFixedDigitsAfterPoint, builder));
    EXPECT_STREQ("0.000000000000000000000000000000000000000000000000000000000000",
        builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(
        9e59, DoubleToStringConverter<>::kMaxFixedDigitsAfterPoint, builder));
    EXPECT_STREQ("899999999999999918767229449717619953810131273674690656206848."
            "000000000000000000000000000000000000000000000000000000000000",
        builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(
        -9e59, DoubleToStringConverter<>::kMaxFixedDigitsAfterPoint, builder));
    EXPECT_STREQ("-899999999999999918767229449717619953810131273674690656206848."
            "000000000000000000000000000000000000000000000000000000000000",
        builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(!dc.ToFixed(
        1e60, DoubleToStringConverter<>::kMaxFixedDigitsAfterPoint, builder));
    EXPECT_EQ(0, builder.Position());

    builder.Reset();
    EXPECT_TRUE(!dc.ToFixed(
        9e59, DoubleToStringConverter<>::kMaxFixedDigitsAfterPoint + 1, builder));
    EXPECT_EQ(0, builder.Position());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(3.0, 0, builder));
    EXPECT_STREQ("3", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(3.23, 1, builder));
    EXPECT_STREQ("3.2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(3.23, 3, builder));
    EXPECT_STREQ("3.230", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.0323, 2, builder));
    EXPECT_STREQ("0.03", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.0373, 2, builder));
    EXPECT_STREQ("0.04", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.0000373, 2, builder));
    EXPECT_STREQ("0.00", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(1.5, 0, builder));
    EXPECT_STREQ("2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(2.5, 0, builder));
    EXPECT_STREQ("3", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(3.5, 0, builder));
    EXPECT_STREQ("4", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.15, 1, builder));
    EXPECT_STREQ("0.1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.25, 1, builder));
    EXPECT_STREQ("0.3", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.35, 1, builder));
    EXPECT_STREQ("0.3", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.45, 1, builder));
    EXPECT_STREQ("0.5", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.55, 1, builder));
    EXPECT_STREQ("0.6", builder.Finalize());

    // Test positive/negative zeroes.
    DtoaFlags flags2 = DtoaFlags::EmitPositiveExponentSign;
    DoubleToStringConverter<> dc2(flags2, "Infinity", "NaN", 'e',
        0, 0, 0, 0);  // Padding zeroes.
    builder.Reset();
    EXPECT_TRUE(dc2.ToFixed(0.0, 1, builder));
    EXPECT_STREQ("0.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToFixed(-0.0, 1, builder));
    EXPECT_STREQ("-0.0", builder.Finalize());

    // Verify the trailing dot is emitted.
    DtoaFlags flags3 = static_cast<DtoaFlags>(
        static_cast<int>(DtoaFlags::EmitPositiveExponentSign) |
        static_cast<int>(DtoaFlags::EmitTrailingDecimalPoint));
    DoubleToStringConverter<> dc3(flags3, "Infinity", "NaN", 'e',
        0, 0, 0, 0);  // Padding zeroes.
    builder.Reset();
    EXPECT_TRUE(dc3.ToFixed(0.0, 0, builder));
    EXPECT_STREQ("0.", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc3.ToFixed(-0.0, 0, builder));
    EXPECT_STREQ("-0.", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc3.ToFixed(1.0, 0, builder));
    EXPECT_STREQ("1.", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc3.ToFixed(-1.0, 0, builder));
    EXPECT_STREQ("-1.", builder.Finalize());

    // Verify no trailing zero is emitted, even if the configuration is set.
    // The given parameter takes precedence.
    DtoaFlags flags4 = static_cast<DtoaFlags>(
        static_cast<int>(DtoaFlags::EmitPositiveExponentSign) |
        static_cast<int>(DtoaFlags::EmitTrailingDecimalPoint) |
        static_cast<int>(DtoaFlags::EmitTrailingZeroAfterPoint));
    DoubleToStringConverter<> dc4(flags4, "Infinity", "NaN", 'e',
        0, 0, 0, 0);  // Padding zeroes.
    builder.Reset();
    EXPECT_TRUE(dc4.ToFixed(0.0, 0, builder));
    EXPECT_STREQ("0.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToFixed(-0.0, 0, builder));
    EXPECT_STREQ("-0.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToFixed(1.0, 0, builder));
    EXPECT_STREQ("1.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToFixed(-1.0, 0, builder));
    EXPECT_STREQ("-1.0", builder.Finalize());

    // Test the examples in the comments of ToFixed.
    flags = DtoaFlags::Default;
    DoubleToStringConverter<> dc5(flags, nullptr, nullptr, 'e', 0, 0, 0, 0);

    builder.Reset();
    EXPECT_TRUE(dc5.ToFixed(3.12, 1, builder));
    EXPECT_STREQ("3.1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc5.ToFixed(3.1415, 3, builder));
    EXPECT_STREQ("3.142", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc5.ToFixed(1234.56789, 4, builder));
    EXPECT_STREQ("1234.5679", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc5.ToFixed(1.23, 5, builder));
    EXPECT_STREQ("1.23000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc5.ToFixed(0.1, 4, builder));
    EXPECT_STREQ("0.1000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc5.ToFixed(1e30, 2, builder));
    EXPECT_STREQ("1000000000000000019884624838656.00", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc5.ToFixed(0.1, 30, builder));
    EXPECT_STREQ("0.100000000000000005551115123126", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc5.ToFixed(0.1, 17, builder));
    EXPECT_STREQ("0.10000000000000001", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc5.ToFixed(123.45, 0, builder));
    EXPECT_STREQ("123", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc5.ToFixed(0.678, 0, builder));
    EXPECT_STREQ("1", builder.Finalize());

    flags = DtoaFlags::EmitTrailingDecimalPoint;
    DoubleToStringConverter<> dc6(flags, nullptr, nullptr, 'e', 0, 0, 0, 0);

    builder.Reset();
    EXPECT_TRUE(dc6.ToFixed(123.45, 0, builder));
    EXPECT_STREQ("123.", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc6.ToFixed(0.678, 0, builder));
    EXPECT_STREQ("1.", builder.Finalize());

    flags = static_cast<DtoaFlags>(
        static_cast<int>(DtoaFlags::EmitTrailingDecimalPoint) |
        static_cast<int>(DtoaFlags::EmitTrailingZeroAfterPoint));
    DoubleToStringConverter<> dc7(flags, nullptr, nullptr, 'e', 0, 0, 0, 0);

    builder.Reset();
    EXPECT_TRUE(dc7.ToFixed(123.45, 0, builder));
    EXPECT_STREQ("123.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc7.ToFixed(0.678, 0, builder));
    EXPECT_STREQ("1.0", builder.Finalize());

    // Test special value handling.
    DoubleToStringConverter<> dc8(flags, nullptr, nullptr, 'e', 0, 0, 0, 0);

    builder.Reset();
    EXPECT_TRUE(!dc8.ToFixed(Double::Infinity(), 1, builder));

    builder.Reset();
    EXPECT_TRUE(!dc8.ToFixed(-Double::Infinity(), 1, builder));

    builder.Reset();
    EXPECT_TRUE(!dc8.ToFixed(Double::Nan(), 1, builder));

    builder.Reset();
    EXPECT_TRUE(!dc8.ToFixed(-Double::Nan(), 1, builder));

    DoubleToStringConverter<> dc9(flags, "Infinity", "NaN", 'e', 0, 0, 0, 0);

    builder.Reset();
    EXPECT_TRUE(dc9.ToFixed(Double::Infinity(), 1, builder));
    EXPECT_STREQ("Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc9.ToFixed(-Double::Infinity(), 1, builder));
    EXPECT_STREQ("-Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc9.ToFixed(Double::Nan(), 1, builder));
    EXPECT_STREQ("NaN", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc9.ToFixed(-Double::Nan(), 1, builder));
    EXPECT_STREQ("NaN", builder.Finalize());
}


TEST(Convert, DoubleToExponential)
{
    const int kBufferSize = 256;
    char buffer[kBufferSize];
    DtoaFlags flags = static_cast<DtoaFlags>(
        static_cast<int>(DtoaFlags::EmitPositiveExponentSign) |
        static_cast<int>(DtoaFlags::UniqueZero));
    StringBuilder<> builder(buffer, kBufferSize);
    DoubleToStringConverter<> dc(flags, "Infinity", "NaN", 'e', 0, 0, 0, 0);

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.0, 5, builder));
    EXPECT_STREQ("0.00000e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.0, 0, builder));
    EXPECT_STREQ("0e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.0, 1, builder));
    EXPECT_STREQ("0.0e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.123456, 5, builder));
    EXPECT_STREQ("1.23456e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(1.2, 1, builder));
    EXPECT_STREQ("1.2e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.0, 1, builder));
    EXPECT_STREQ("0.0e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.0, 2, builder));
    EXPECT_STREQ("0.00e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.0, 2, builder));
    EXPECT_STREQ("0.00e+0", builder.Finalize());

    EXPECT_TRUE(DoubleToStringConverter<>::kMaxExponentialDigits == 120);
    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(
        0.0, DoubleToStringConverter<>::kMaxExponentialDigits, builder));
    EXPECT_STREQ("0.00000000000000000000000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000e+0",
        builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(
        9e59, DoubleToStringConverter<>::kMaxExponentialDigits, builder));
    EXPECT_STREQ("8.99999999999999918767229449717619953810131273674690656206848"
        "0000000000000000000000000000000000000000000000000000000000000e+59",
        builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(
        -9e59, DoubleToStringConverter<>::kMaxExponentialDigits, builder));
    EXPECT_STREQ("-8.99999999999999918767229449717619953810131273674690656206848"
        "0000000000000000000000000000000000000000000000000000000000000e+59",
        builder.Finalize());

    const double max_double = 1.7976931348623157e308;
    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(
        max_double, DoubleToStringConverter<>::kMaxExponentialDigits, builder));
    EXPECT_STREQ("1.79769313486231570814527423731704356798070567525844996598917"
        "4768031572607800285387605895586327668781715404589535143824642e+308",
        builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.000001, 2, builder));
    EXPECT_STREQ("1.00e-6", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.0000001, 2, builder));
    EXPECT_STREQ("1.00e-7", builder.Finalize());

    // Test the examples in the comments of ToExponential.
    flags = DtoaFlags::Default;
    DoubleToStringConverter<> dc2(flags, "Infinity", "NaN", 'e', 0, 0, 0, 0);

    builder.Reset();
    EXPECT_TRUE(dc2.ToExponential(3.12, 1, builder));
    EXPECT_STREQ("3.1e0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToExponential(5.0, 3, builder));
    EXPECT_STREQ("5.000e0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToExponential(0.001, 2, builder));
    EXPECT_STREQ("1.00e-3", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToExponential(3.1415, -1, builder));
    EXPECT_STREQ("3.1415e0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToExponential(3.1415, 4, builder));
    EXPECT_STREQ("3.1415e0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToExponential(3.1415, 3, builder));
    EXPECT_STREQ("3.142e0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToExponential(123456789000000, 3, builder));
    EXPECT_STREQ("1.235e14", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToExponential(1000000000000000019884624838656.0, -1, builder));
    EXPECT_STREQ("1e30", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToExponential(1000000000000000019884624838656.0, 32, builder));
    EXPECT_STREQ("1.00000000000000001988462483865600e30", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToExponential(1234, 0, builder));
    EXPECT_STREQ("1e3", builder.Finalize());

    // Test special value handling.
    DoubleToStringConverter<> dc3(flags, nullptr, nullptr, 'e', 0, 0, 0, 0);

    builder.Reset();
    EXPECT_TRUE(!dc3.ToExponential(Double::Infinity(), 1, builder));

    builder.Reset();
    EXPECT_TRUE(!dc3.ToExponential(-Double::Infinity(), 1, builder));

    builder.Reset();
    EXPECT_TRUE(!dc3.ToExponential(Double::Nan(), 1, builder));

    builder.Reset();
    EXPECT_TRUE(!dc3.ToExponential(-Double::Nan(), 1, builder));

    DoubleToStringConverter<> dc4(flags, "Infinity", "NaN", 'e', 0, 0, 0, 0);

    builder.Reset();
    EXPECT_TRUE(dc4.ToExponential(Double::Infinity(), 1, builder));
    EXPECT_STREQ("Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToExponential(-Double::Infinity(), 1, builder));
    EXPECT_STREQ("-Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToExponential(Double::Nan(), 1, builder));
    EXPECT_STREQ("NaN", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToExponential(-Double::Nan(), 1, builder));
    EXPECT_STREQ("NaN", builder.Finalize());
}

TEST(Convert, DoubleToPrecision)
{
    const int kBufferSize = 256;
    char buffer[kBufferSize];
    DtoaFlags flags = static_cast<DtoaFlags>(
        static_cast<int>(DtoaFlags::EmitPositiveExponentSign) |
        static_cast<int>(DtoaFlags::UniqueZero));
    StringBuilder<> builder(buffer, kBufferSize);
    DoubleToStringConverter<> dc(flags, "Infinity", "NaN", 'e',
        0, 0,   // Padding zeroes for shortest mode.
        6, 0);  // Padding zeroes for precision mode.

    EXPECT_TRUE(DoubleToStringConverter<>::kMinPrecisionDigits == 1);
    EXPECT_TRUE(dc.ToPrecision(0.0, 1, builder));
    EXPECT_STREQ("0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-0.0, 1, builder));
    EXPECT_STREQ("0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(0.0, 2, builder));
    EXPECT_STREQ("0.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-0.0, 2, builder));
    EXPECT_STREQ("0.0", builder.Finalize());

    EXPECT_TRUE(DoubleToStringConverter<>::kMaxPrecisionDigits == 120);
    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(
        0.0, DoubleToStringConverter<>::kMaxPrecisionDigits, builder));
    EXPECT_STREQ("0.00000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000",
        builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(
        9e59, DoubleToStringConverter<>::kMaxPrecisionDigits, builder));
    EXPECT_STREQ("899999999999999918767229449717619953810131273674690656206848."
        "000000000000000000000000000000000000000000000000000000000000",
        builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(
        -9e59, DoubleToStringConverter<>::kMaxPrecisionDigits, builder));
    EXPECT_STREQ("-899999999999999918767229449717619953810131273674690656206848."
        "000000000000000000000000000000000000000000000000000000000000",
        builder.Finalize());

    const double max_double = 1.7976931348623157e308;
    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(
        max_double, DoubleToStringConverter<>::kMaxPrecisionDigits, builder));
    EXPECT_STREQ("1.79769313486231570814527423731704356798070567525844996598917"
        "476803157260780028538760589558632766878171540458953514382464e+308",
        builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(0.000001, 2, builder));
    EXPECT_STREQ("0.0000010", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(0.0000001, 2, builder));
    EXPECT_STREQ("1.0e-7", builder.Finalize());

    flags = DtoaFlags::Default;
    DoubleToStringConverter<> dc2(flags, nullptr, nullptr, 'e', 0, 0, 0, 1);
    builder.Reset();
    EXPECT_TRUE(dc2.ToPrecision(230.0, 2, builder));
    EXPECT_STREQ("230", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToPrecision(23.0, 2, builder));
    EXPECT_STREQ("23", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToPrecision(2.30, 2, builder));
    EXPECT_STREQ("2.3", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc2.ToPrecision(2300.0, 2, builder));
    EXPECT_STREQ("2.3e3", builder.Finalize());

    flags = DtoaFlags::EmitTrailingDecimalPoint;
    DoubleToStringConverter<> dc3(flags, nullptr, nullptr, 'e', 0, 0, 0, 1);
    builder.Reset();
    EXPECT_TRUE(dc3.ToPrecision(230.0, 2, builder));
    EXPECT_STREQ("230.", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc3.ToPrecision(23.0, 2, builder));
    EXPECT_STREQ("23.", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc3.ToPrecision(2.30, 2, builder));
    EXPECT_STREQ("2.3", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc3.ToPrecision(2300.0, 2, builder));
    EXPECT_STREQ("2.3e3", builder.Finalize());

    flags = static_cast<DtoaFlags>(
        static_cast<int>(DtoaFlags::EmitTrailingDecimalPoint) |
        static_cast<int>(DtoaFlags::EmitTrailingZeroAfterPoint));
    DoubleToStringConverter<> dc4(flags, nullptr, nullptr, 'e', 0, 0, 0, 1);
    builder.Reset();
    EXPECT_TRUE(dc4.ToPrecision(230.0, 2, builder));
    EXPECT_STREQ("2.3e2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToPrecision(23.0, 2, builder));
    EXPECT_STREQ("23.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToPrecision(2.30, 2, builder));
    EXPECT_STREQ("2.3", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc4.ToPrecision(2300.0, 2, builder));
    EXPECT_STREQ("2.3e3", builder.Finalize());

    // Test the examples in the comments of ToPrecision.
    flags = DtoaFlags::Default;
    DoubleToStringConverter<> dc5(flags, "Infinity", "NaN", 'e', 0, 0, 6, 1);
    flags = DtoaFlags::EmitTrailingDecimalPoint;
    DoubleToStringConverter<> dc6(flags, "Infinity", "NaN", 'e', 0, 0, 6, 1);
    flags = static_cast<DtoaFlags>(
        static_cast<int>(DtoaFlags::EmitTrailingDecimalPoint) |
        static_cast<int>(DtoaFlags::EmitTrailingZeroAfterPoint));
    DoubleToStringConverter<> dc7(flags, "Infinity", "NaN", 'e', 0, 0, 6, 1);

    builder.Reset();
    EXPECT_TRUE(dc5.ToPrecision(0.0000012345, 2, builder));
    EXPECT_STREQ("0.0000012", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc5.ToPrecision(0.00000012345, 2, builder));
    EXPECT_STREQ("1.2e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc5.ToPrecision(230.0, 2, builder));
    EXPECT_STREQ("230", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc6.ToPrecision(230.0, 2, builder));
    EXPECT_STREQ("230.", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc7.ToPrecision(230.0, 2, builder));
    EXPECT_STREQ("2.3e2", builder.Finalize());

    flags = DtoaFlags::Default;
    DoubleToStringConverter<> dc8(flags, nullptr, nullptr, 'e', 0, 0, 6, 3);

    builder.Reset();
    EXPECT_TRUE(dc8.ToPrecision(123450.0, 6, builder));
    EXPECT_STREQ("123450", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc8.ToPrecision(123450.0, 5, builder));
    EXPECT_STREQ("123450", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc8.ToPrecision(123450.0, 4, builder));
    EXPECT_STREQ("123500", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc8.ToPrecision(123450.0, 3, builder));
    EXPECT_STREQ("123000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc8.ToPrecision(123450.0, 2, builder));
    EXPECT_STREQ("1.2e5", builder.Finalize());

    // Test special value handling.
    builder.Reset();
    EXPECT_TRUE(!dc8.ToPrecision(Double::Infinity(), 1, builder));

    builder.Reset();
    EXPECT_TRUE(!dc8.ToPrecision(-Double::Infinity(), 1, builder));

    builder.Reset();
    EXPECT_TRUE(!dc8.ToPrecision(Double::Nan(), 1, builder));

    builder.Reset();
    EXPECT_TRUE(!dc8.ToPrecision(-Double::Nan(), 1, builder));

    builder.Reset();
    EXPECT_TRUE(dc7.ToPrecision(Double::Infinity(), 1, builder));
    EXPECT_STREQ("Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc7.ToPrecision(-Double::Infinity(), 1, builder));
    EXPECT_STREQ("-Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc7.ToPrecision(Double::Nan(), 1, builder));
    EXPECT_STREQ("NaN", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc7.ToPrecision(-Double::Nan(), 1, builder));
    EXPECT_STREQ("NaN", builder.Finalize());
}

TEST(Convert, DoubleToStringJavaScript)
{
    const int kBufferSize = 128;
    char buffer[kBufferSize];
    StringBuilder<> builder(buffer, kBufferSize);
    const DoubleToStringConverter<>& dc = DoubleToStringConverter<>::EcmaScriptConverter();

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(Double::Nan(), builder));
    EXPECT_STREQ("NaN", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(Double::Infinity(), builder));
    EXPECT_STREQ("Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-Double::Infinity(), builder));
    EXPECT_STREQ("-Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(0.0, builder));
    EXPECT_STREQ("0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(9.0, builder));
    EXPECT_STREQ("9", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(90.0, builder));
    EXPECT_STREQ("90", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(90.12, builder));
    EXPECT_STREQ("90.12", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(0.1, builder));
    EXPECT_STREQ("0.1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(0.01, builder));
    EXPECT_STREQ("0.01", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(0.0123, builder));
    EXPECT_STREQ("0.0123", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(111111111111111111111.0, builder));
    EXPECT_STREQ("111111111111111110000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(100000000000000000000.0, builder));
    EXPECT_STREQ("100000000000000000000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(1111111111111111111111.0, builder));
    EXPECT_STREQ("1.1111111111111111e+21", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(11111111111111111111111.0, builder));
    EXPECT_STREQ("1.1111111111111111e+22", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(0.00001, builder));
    EXPECT_STREQ("0.00001", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(0.000001, builder));
    EXPECT_STREQ("0.000001", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(0.0000001, builder));
    EXPECT_STREQ("1e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(0.00000012, builder));
    EXPECT_STREQ("1.2e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(0.000000123, builder));
    EXPECT_STREQ("1.23e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(0.00000001, builder));
    EXPECT_STREQ("1e-8", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(0.000000012, builder));
    EXPECT_STREQ("1.2e-8", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(0.000000012, builder));
    EXPECT_STREQ("1.2e-8", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(0.0000000123, builder));
    EXPECT_STREQ("1.23e-8", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.0, builder));
    EXPECT_STREQ("0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-9.0, builder));
    EXPECT_STREQ("-9", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-90.0, builder));
    EXPECT_STREQ("-90", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-90.12, builder));
    EXPECT_STREQ("-90.12", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.1, builder));
    EXPECT_STREQ("-0.1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.01, builder));
    EXPECT_STREQ("-0.01", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.0123, builder));
    EXPECT_STREQ("-0.0123", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-111111111111111111111.0, builder));
    EXPECT_STREQ("-111111111111111110000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-1111111111111111111111.0, builder));
    EXPECT_STREQ("-1.1111111111111111e+21", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-11111111111111111111111.0, builder));
    EXPECT_STREQ("-1.1111111111111111e+22", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.00001, builder));
    EXPECT_STREQ("-0.00001", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.000001, builder));
    EXPECT_STREQ("-0.000001", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.0000001, builder));
    EXPECT_STREQ("-1e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.00000012, builder));
    EXPECT_STREQ("-1.2e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.000000123, builder));
    EXPECT_STREQ("-1.23e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.00000001, builder));
    EXPECT_STREQ("-1e-8", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.000000012, builder));
    EXPECT_STREQ("-1.2e-8", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.000000012, builder));
    EXPECT_STREQ("-1.2e-8", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToShortest(-0.0000000123, builder));
    EXPECT_STREQ("-1.23e-8", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(Double::Nan(), 2, builder));
    EXPECT_STREQ("NaN", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(Double::Infinity(), 2, builder));
    EXPECT_STREQ("Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-Double::Infinity(), 2, builder));
    EXPECT_STREQ("-Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.1, 1, builder));
    EXPECT_STREQ("-0.1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.1, 2, builder));
    EXPECT_STREQ("-0.10", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.1, 3, builder));
    EXPECT_STREQ("-0.100", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.01, 2, builder));
    EXPECT_STREQ("-0.01", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.01, 3, builder));
    EXPECT_STREQ("-0.010", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.01, 4, builder));
    EXPECT_STREQ("-0.0100", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.001, 2, builder));
    EXPECT_STREQ("-0.00", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.001, 3, builder));
    EXPECT_STREQ("-0.001", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.001, 4, builder));
    EXPECT_STREQ("-0.0010", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-1.0, 4, builder));
    EXPECT_STREQ("-1.0000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-1.0, 1, builder));
    EXPECT_STREQ("-1.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-1.0, 0, builder));
    EXPECT_STREQ("-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-12.0, 0, builder));
    EXPECT_STREQ("-12", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-1.1, 0, builder));
    EXPECT_STREQ("-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-12.1, 0, builder));
    EXPECT_STREQ("-12", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-1.12, 0, builder));
    EXPECT_STREQ("-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-12.12, 0, builder));
    EXPECT_STREQ("-12", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.0000006, 7, builder));
    EXPECT_STREQ("-0.0000006", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.00000006, 8, builder));
    EXPECT_STREQ("-0.00000006", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.00000006, 9, builder));
    EXPECT_STREQ("-0.000000060", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.00000006, 10, builder));
    EXPECT_STREQ("-0.0000000600", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0, 0, builder));
    EXPECT_STREQ("0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0, 1, builder));
    EXPECT_STREQ("0.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0, 2, builder));
    EXPECT_STREQ("0.00", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(1000, 0, builder));
    EXPECT_STREQ("1000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.00001, 0, builder));
    EXPECT_STREQ("0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.00001, 5, builder));
    EXPECT_STREQ("0.00001", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.0000000000000000001, 20, builder));
    EXPECT_STREQ("0.00000000000000000010", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.00001, 17, builder));
    EXPECT_STREQ("0.00001000000000000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(1000000000000000128.0, 0, builder));
    EXPECT_STREQ("1000000000000000128", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(1000000000000000128.0, 1, builder));
    EXPECT_STREQ("1000000000000000128.0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(1000000000000000128.0, 2, builder));
    EXPECT_STREQ("1000000000000000128.00", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(1000000000000000128.0, 20, builder));
    EXPECT_STREQ("1000000000000000128.00000000000000000000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.0, 0, builder));
    EXPECT_STREQ("0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-42.0, 3, builder));
    EXPECT_STREQ("-42.000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-1000000000000000128.0, 0, builder));
    EXPECT_STREQ("-1000000000000000128", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.0000000000000000001, 20, builder));
    EXPECT_STREQ("-0.00000000000000000010", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.123123123123123, 20, builder));
    EXPECT_STREQ("0.12312312312312299889", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(0.5, 0, builder));
    EXPECT_STREQ("1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(-0.5, 0, builder));
    EXPECT_STREQ("-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(1.25, 1, builder));
    EXPECT_STREQ("1.3", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(234.20405, 4, builder));
    EXPECT_STREQ("234.2040", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToFixed(234.2040506, 4, builder));
    EXPECT_STREQ("234.2041", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(1.0, -1, builder));
    EXPECT_STREQ("1e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(11.0, -1, builder));
    EXPECT_STREQ("1.1e+1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(112.0, -1, builder));
    EXPECT_STREQ("1.12e+2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(1.0, 0, builder));
    EXPECT_STREQ("1e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(11.0, 0, builder));
    EXPECT_STREQ("1e+1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(112.0, 0, builder));
    EXPECT_STREQ("1e+2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(1.0, 1, builder));
    EXPECT_STREQ("1.0e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(11.0, 1, builder));
    EXPECT_STREQ("1.1e+1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(112.0, 1, builder));
    EXPECT_STREQ("1.1e+2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(1.0, 2, builder));
    EXPECT_STREQ("1.00e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(11.0, 2, builder));
    EXPECT_STREQ("1.10e+1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(112.0, 2, builder));
    EXPECT_STREQ("1.12e+2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(1.0, 3, builder));
    EXPECT_STREQ("1.000e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(11.0, 3, builder));
    EXPECT_STREQ("1.100e+1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(112.0, 3, builder));
    EXPECT_STREQ("1.120e+2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.1, -1, builder));
    EXPECT_STREQ("1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.11, -1, builder));
    EXPECT_STREQ("1.1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.112, -1, builder));
    EXPECT_STREQ("1.12e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.1, 0, builder));
    EXPECT_STREQ("1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.11, 0, builder));
    EXPECT_STREQ("1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.112, 0, builder));
    EXPECT_STREQ("1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.1, 1, builder));
    EXPECT_STREQ("1.0e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.11, 1, builder));
    EXPECT_STREQ("1.1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.112, 1, builder));
    EXPECT_STREQ("1.1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.1, 2, builder));
    EXPECT_STREQ("1.00e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.11, 2, builder));
    EXPECT_STREQ("1.10e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.112, 2, builder));
    EXPECT_STREQ("1.12e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.1, 3, builder));
    EXPECT_STREQ("1.000e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.11, 3, builder));
    EXPECT_STREQ("1.100e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.112, 3, builder));
    EXPECT_STREQ("1.120e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-1.0, -1, builder));
    EXPECT_STREQ("-1e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-11.0, -1, builder));
    EXPECT_STREQ("-1.1e+1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-112.0, -1, builder));
    EXPECT_STREQ("-1.12e+2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-1.0, 0, builder));
    EXPECT_STREQ("-1e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-11.0, 0, builder));
    EXPECT_STREQ("-1e+1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-112.0, 0, builder));
    EXPECT_STREQ("-1e+2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-1.0, 1, builder));
    EXPECT_STREQ("-1.0e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-11.0, 1, builder));
    EXPECT_STREQ("-1.1e+1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-112.0, 1, builder));
    EXPECT_STREQ("-1.1e+2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-1.0, 2, builder));
    EXPECT_STREQ("-1.00e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-11.0, 2, builder));
    EXPECT_STREQ("-1.10e+1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-112.0, 2, builder));
    EXPECT_STREQ("-1.12e+2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-1.0, 3, builder));
    EXPECT_STREQ("-1.000e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-11.0, 3, builder));
    EXPECT_STREQ("-1.100e+1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-112.0, 3, builder));
    EXPECT_STREQ("-1.120e+2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.1, -1, builder));
    EXPECT_STREQ("-1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.11, -1, builder));
    EXPECT_STREQ("-1.1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.112, -1, builder));
    EXPECT_STREQ("-1.12e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.1, 0, builder));
    EXPECT_STREQ("-1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.11, 0, builder));
    EXPECT_STREQ("-1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.112, 0, builder));
    EXPECT_STREQ("-1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.1, 1, builder));
    EXPECT_STREQ("-1.0e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.11, 1, builder));
    EXPECT_STREQ("-1.1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.112, 1, builder));
    EXPECT_STREQ("-1.1e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.1, 2, builder));
    EXPECT_STREQ("-1.00e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.11, 2, builder));
    EXPECT_STREQ("-1.10e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.112, 2, builder));
    EXPECT_STREQ("-1.12e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.1, 3, builder));
    EXPECT_STREQ("-1.000e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.11, 3, builder));
    EXPECT_STREQ("-1.100e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.112, 3, builder));
    EXPECT_STREQ("-1.120e-1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(Double::Nan(), 2, builder));
    EXPECT_STREQ("NaN", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(Double::Infinity(), 2, builder));
    EXPECT_STREQ("Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-Double::Infinity(), 2, builder));
    EXPECT_STREQ("-Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(1.0, 0, builder));
    EXPECT_STREQ("1e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.0, -1, builder));
    EXPECT_STREQ("0e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.0, 2, builder));
    EXPECT_STREQ("0.00e+0", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(11.2356, 0, builder));
    EXPECT_STREQ("1e+1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(11.2356, 4, builder));
    EXPECT_STREQ("1.1236e+1", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.000112356, 4, builder));
    EXPECT_STREQ("1.1236e-4", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.000112356, 4, builder));
    EXPECT_STREQ("-1.1236e-4", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(0.000112356, -1, builder));
    EXPECT_STREQ("1.12356e-4", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToExponential(-0.000112356, -1, builder));
    EXPECT_STREQ("-1.12356e-4", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(Double::Nan(), 1, builder));
    EXPECT_STREQ("NaN", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(Double::Infinity(), 2, builder));
    EXPECT_STREQ("Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-Double::Infinity(), 2, builder));
    EXPECT_STREQ("-Infinity", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(0.000555, 15, builder));
    EXPECT_STREQ("0.000555000000000000", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(0.000000555, 15, builder));
    EXPECT_STREQ("5.55000000000000e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-0.000000555, 15, builder));
    EXPECT_STREQ("-5.55000000000000e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(123456789.0, 1, builder));
    EXPECT_STREQ("1e+8", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(123456789.0, 9, builder));
    EXPECT_STREQ("123456789", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(123456789.0, 8, builder));
    EXPECT_STREQ("1.2345679e+8", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(123456789.0, 7, builder));
    EXPECT_STREQ("1.234568e+8", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-123456789.0, 7, builder));
    EXPECT_STREQ("-1.234568e+8", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-.0000000012345, 2, builder));
    EXPECT_STREQ("-1.2e-9", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-.000000012345, 2, builder));
    EXPECT_STREQ("-1.2e-8", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-.00000012345, 2, builder));
    EXPECT_STREQ("-1.2e-7", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-.0000012345, 2, builder));
    EXPECT_STREQ("-0.0000012", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-.000012345, 2, builder));
    EXPECT_STREQ("-0.000012", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-.00012345, 2, builder));
    EXPECT_STREQ("-0.00012", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-.0012345, 2, builder));
    EXPECT_STREQ("-0.0012", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-.012345, 2, builder));
    EXPECT_STREQ("-0.012", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-.12345, 2, builder));
    EXPECT_STREQ("-0.12", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-1.2345, 2, builder));
    EXPECT_STREQ("-1.2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-12.345, 2, builder));
    EXPECT_STREQ("-12", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-123.45, 2, builder));
    EXPECT_STREQ("-1.2e+2", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-1234.5, 2, builder));
    EXPECT_STREQ("-1.2e+3", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-12345.0, 2, builder));
    EXPECT_STREQ("-1.2e+4", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-12345.67, 4, builder));
    EXPECT_STREQ("-1.235e+4", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(-12344.67, 4, builder));
    EXPECT_STREQ("-1.234e+4", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(1.25, 2, builder));
    EXPECT_STREQ("1.3", builder.Finalize());

    builder.Reset();
    EXPECT_TRUE(dc.ToPrecision(1.35, 2, builder));
    EXPECT_STREQ("1.4", builder.Finalize());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(Convert, CharTest)
{
    char buffer[kBufferSize];

    moe::Convert::ToShortestString(123.456f, buffer);
    EXPECT_STREQ("123.456", buffer);

    moe::Convert::ToShortestString(123.456, buffer);
    EXPECT_STREQ("123.456", buffer);

    moe::Convert::ToFixedString(123.456, 0, buffer);
    EXPECT_STREQ("123", buffer);

    moe::Convert::ToFixedString(123.456, 2, buffer);
    EXPECT_STREQ("123.46", buffer);

    moe::Convert::ToFixedString(123.456, 20, buffer);
    EXPECT_STREQ("123.45600000000000306954", buffer);

    moe::Convert::ToPrecisionString(123.456, 1, buffer);
    EXPECT_STREQ("1e+2", buffer);

    moe::Convert::ToPrecisionString(123.456, 2, buffer);
    EXPECT_STREQ("1.2e+2", buffer);

    moe::Convert::ToPrecisionString(123.456, 21, buffer);
    EXPECT_STREQ("123.456000000000003070", buffer);

    moe::Convert::ToExponentialString(123.456, 0, buffer);
    EXPECT_STREQ("1e+2", buffer);

    moe::Convert::ToExponentialString(123.456, 1, buffer);
    EXPECT_STREQ("1.2e+2", buffer);

    moe::Convert::ToExponentialString(123.456, 20, buffer);
    EXPECT_STREQ("1.23456000000000003070e+2", buffer);
}

TEST(Convert, WCharTest)
{
    wchar_t buffer[kBufferSize];

    moe::Convert::ToShortestString(123.456f, buffer);
    EXPECT_STREQ(L"123.456", buffer);

    moe::Convert::ToShortestString(123.456, buffer);
    EXPECT_STREQ(L"123.456", buffer);

    moe::Convert::ToFixedString(123.456, 0, buffer);
    EXPECT_STREQ(L"123", buffer);

    moe::Convert::ToFixedString(123.456, 2, buffer);
    EXPECT_STREQ(L"123.46", buffer);

    moe::Convert::ToFixedString(123.456, 20, buffer);
    EXPECT_STREQ(L"123.45600000000000306954", buffer);

    moe::Convert::ToPrecisionString(123.456, 1, buffer);
    EXPECT_STREQ(L"1e+2", buffer);

    moe::Convert::ToPrecisionString(123.456, 2, buffer);
    EXPECT_STREQ(L"1.2e+2", buffer);

    moe::Convert::ToPrecisionString(123.456, 21, buffer);
    EXPECT_STREQ(L"123.456000000000003070", buffer);

    moe::Convert::ToExponentialString(123.456, 0, buffer);
    EXPECT_STREQ(L"1e+2", buffer);

    moe::Convert::ToExponentialString(123.456, 1, buffer);
    EXPECT_STREQ(L"1.2e+2", buffer);

    moe::Convert::ToExponentialString(123.456, 20, buffer);
    EXPECT_STREQ(L"1.23456000000000003070e+2", buffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

TEST(Convert, Strtod)
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

TEST(Convert, Strtof)
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

static const int kShortStrtodRandomCount = 2;
static const int kLargeStrtodRandomCount = 2;

TEST(Convert, RandomStrtod)
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
            double strtodResult = details::StringToDoubleConverter<char>::Strtod(vector, exponent);
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
            double strtodResult = details::StringToDoubleConverter<char>::Strtod(vector, exponent);
            EXPECT_TRUE(CheckDouble(vector, exponent, strtodResult));
        }
    }
}

static const int kShortStrtofRandomCount = 2;
static const int kLargeStrtofRandomCount = 2;

TEST(Convert, RandomStrtof)
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
            float strtofResult = details::StringToDoubleConverter<char>::Strtof(vector, exponent);
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
            float strtofResult = details::StringToDoubleConverter<char>::Strtof(vector, exponent);
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

TEST(Convert, StringToDoubleVarious)
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

    EXPECT_TRUE(::isnan(StrToD("x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("42x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("42 x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" + 42 x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" - 42 x", flags, 0.0, processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToD("x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" x", flags, 1.0, processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToD("x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" x", flags, 1.0, processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToD(" + 42 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" - 42 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::Default;

    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("  ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("  ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(42.0, StrToD("42", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD(" + 42 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" - 42 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" x", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("42x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("42 x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" + 42 x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" - 42 x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowLeadingSpaces;

    EXPECT_EQ(0.0, StrToD(" ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD(" ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0, StrToD(" 42", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("42 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowTrailingSpaces;

    EXPECT_EQ(0.0, StrToD(" ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD(" ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0, StrToD("42 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD(" 42", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Convert, StringToDoubleEmptyString)
{
    AtodFlags flags;
    size_t processed;
    bool allUsed;

    flags = AtodFlags::Default;
    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD(" ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowSpacesAfterSign;
    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD(" ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowLeadingSpaces;
    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD(" ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD(" ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    flags = AtodFlags::AllowTrailingSpaces;
    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD(" ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD(" ", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    flags = AtodFlags::AllowTrailingJunk;
    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD("", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD(" ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Convert, StringToDoubleHexString)
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

    EXPECT_TRUE(::isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("0x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x 3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("0x3g", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("0x3.23", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("x3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("0x3 foo", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x3 foo", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("+ 0x3 foo", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("+", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("-", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(-5.0, StrToD("-0x5", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-5.0, StrToD(" - 0x5 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(5.0, StrToD(" + 0x5 ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("- -0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("- +0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("+ +0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowHex;

    EXPECT_EQ(18.0, StrToD("0x12", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("0x0", flags, 1.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0x123456789), StrToD("0x123456789", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD(" 0x12 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x0 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x123456789 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xabcdef", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xABCDEF", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD(" 0xABCDEF ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0xABCDEF ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("0x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x 3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("0x3g", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("0x3.23", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("x3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("+ 0x3 foo", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("+", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("-", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(-5.0, StrToD("-0x5", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD(" - 0x5 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" + 0x5 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("- -0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("- +0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("+ +0x5", flags, 0.0,  processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToD(" 0x12 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x0 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(18.0, StrToD("0x12 ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(4, processed);

    EXPECT_EQ(0.0, StrToD("0x0 ", flags, 1.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x123456789 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xabcdef", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xABCDEF", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD(" 0xabcdef ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0xABCDEF ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xabcdef ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(static_cast<double>(0xABCDEF), StrToD("0xABCDEF ", flags, 0.0, processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0xabcdef", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0xABCDEF", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("0x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x 3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(3.0, StrToD("0x3g", flags, 0.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(3.0, StrToD("0x3.234", flags, 0.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x3g", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x3.234", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("x3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("+ 0x3 foo", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("+", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("-", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(-5.0, StrToD("-0x5", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD(" - 0x5 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" + 0x5 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("- -0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("- +0x5", flags, 0.0,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("+ +0x5", flags, 0.0,  processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToD(" ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("0x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0x 3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ((double)0x3, StrToD("0x3g", flags, 0.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ((double)0x3, StrToD("0x3.234", flags, 0.0, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_TRUE(::isnan(StrToD("x3", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Convert, StringToDoubleOctalString)
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

    EXPECT_TRUE(::isnan(StrToD("01234567e0", flags, Double::Nan(), processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToD(" 012", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 00", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 012", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0123456789", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" + 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" - 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 012 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 00 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 012 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0123456789 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" + 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" - 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("012 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("00 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("012 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("0123456789 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(342391.0, StrToD("+01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0, StrToD("-01234567", flags, Double::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("01234567e0", flags, Double::Nan(), processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToD(" 012", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 00", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 012", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0123456789", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" + 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" - 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 012 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 00 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 012 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0123456789 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" + 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" - 01234567 ", flags, Double::Nan(), processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToD(" 012", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 00", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 012", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0123456789", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" + 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" - 01234567", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 012 ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 00 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 012 ", flags, 1.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 0123456789 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" + 01234567 ", flags, Double::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD(" - 01234567 ", flags, Double::Nan(), processed, allUsed)));
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

TEST(Convert, StringToDoubleSpecialValues)
{
    size_t processed;
    AtodFlags flags = AtodFlags::Default;

    {
        // Use 1.0 as junk_string_value.
        StringToDoubleConverter<char> converter(flags, 0.0, 1.0, "infinity", "NaN");

        EXPECT_TRUE(::isnan(converter.StringToDouble("+NaN", 4, processed)));
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

TEST(Convert, StringToDoubleCommentExamples)
{
    // Make sure the examples in the comments are correct.
    AtodFlags flags;
    size_t processed;
    bool allUsed;

    flags = AtodFlags::AllowHex;

    EXPECT_EQ(4660.0, StrToD("0x1234", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("0x1234.56", flags, 0.0, processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToD("01234.56", flags, 0.0, processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToD(" ", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(processed, 0);

    EXPECT_TRUE(::isnan(StrToD(" 1", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(processed, 0);

    EXPECT_TRUE(::isnan(StrToD("0x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(processed, 0);

    EXPECT_EQ(-123.45, StrToD("-123.45", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("--123.45", flags, 0.0, processed, allUsed)));
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
        EXPECT_TRUE(::isnan(converter.StringToDouble("+NaN", 4, processed)));
        EXPECT_EQ(4, processed);

        EXPECT_EQ(-Double::Infinity(), converter.StringToDouble("-infinity", 9, processed));
        EXPECT_EQ(9, processed);

        EXPECT_EQ(1.0, converter.StringToDouble("Infinity", 9, processed));
        EXPECT_EQ(0, processed);
    }

    flags = static_cast<AtodFlags>(
        static_cast<int>(AtodFlags::AllowOctals) |
        static_cast<int>(AtodFlags::AllowLeadingSpaces));

    EXPECT_TRUE(::isnan(StrToD("0x1234", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(668.0, StrToD("01234", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD("", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0, StrToD(" ", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0, StrToD(" 1", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("0x", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("0123e45", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(1239e45, StrToD("01239e45", flags, 0.0, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToD("-infinity", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToD("NaN", flags, 0.0, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Convert, StringToFloatVarious)
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

    EXPECT_TRUE(::isnan(StrToF("x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("42x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("42 x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" + 42 x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" - 42 x", flags, 0.0f, processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToF("x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" x", flags, 1.0f, processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToF("x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" x", flags, 1.0f, processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToF(" + 42 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" - 42 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::Default;

    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF("  ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("  ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(42.0f, StrToF("42", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF(" + 42 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" - 42 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" x", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("42x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("42 x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" + 42 x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" - 42 x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowLeadingSpaces;

    EXPECT_EQ(0.0f, StrToF(" ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF(" ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0f, StrToF(" 42", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF("42 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowTrailingSpaces;

    EXPECT_EQ(0.0f, StrToF(" ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF(" ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(42.0f, StrToF("42 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF(" 42", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Convert, StringToFloatEmptyString)
{
    AtodFlags flags;
    size_t processed;
    bool allUsed;

    flags = AtodFlags::Default;
    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF("", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF(" ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowSpacesAfterSign;
    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF("", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF(" ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowLeadingSpaces;
    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF("", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF(" ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF(" ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    flags = AtodFlags::AllowTrailingSpaces;
    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF("", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF(" ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF(" ", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    flags = AtodFlags::AllowTrailingJunk;
    EXPECT_EQ(0.0f, StrToF("", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(1.0f, StrToF("", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF("", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF(" ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Convert, StringToFloatHexString)
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

    EXPECT_TRUE(::isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF("0x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x 3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("0x3g", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("0x3.23", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("x3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("0x3 foo", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x3 foo", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("+ 0x3 foo", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("+", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("-", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(-5.0f, StrToF("-0x5", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-5.0f, StrToF(" - 0x5 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(5.0f, StrToF(" + 0x5 ", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF("- -0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("- +0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("+ +0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    flags = AtodFlags::AllowHex;

    EXPECT_EQ(18.0f, StrToF("0x12", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(0.0f, StrToF("0x0", flags, 1.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0x123456789), StrToF("0x123456789", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF(" 0x12 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x0 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x123456789 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xabcdef", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xABCDEF", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF(" 0xabcdef ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0xABCDEF ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("0x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x 3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("0x3g", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("0x3.23", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("x3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("+ 0x3 foo", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("+", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("-", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(-5.0f, StrToF("-0x5", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF(" - 0x5 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" + 0x5 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("- -0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("- +0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("+ +0x5", flags, 0.0f,  processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToF(" 0x12 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x0 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(18.0f, StrToF("0x12 ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(4, processed);

    EXPECT_EQ(0.0f, StrToF("0x0 ", flags, 1.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x123456789 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xabcdef", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xABCDEF", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF(" 0xabcdef ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0xABCDEF ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xabcdef ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_EQ(static_cast<float>(0xABCDEF), StrToF("0xABCDEF ", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(8, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0xabcdef", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0xABCDEF", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("0x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x 3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(3.0f, StrToF("0x3g", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ(3.0f, StrToF("0x3.234", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x3g", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x3.234", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("x3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("+ 0x3 foo", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("+", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("-", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(-5.0f, StrToF("-0x5", flags, 0.0f, processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF(" - 0x5 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" + 0x5 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("- -0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("- +0x5", flags, 0.0f,  processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("+ +0x5", flags, 0.0f,  processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToF(" ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF("0x", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0x 3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ((float)0x3, StrToF("0x3g", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_EQ((float)0x3, StrToF("0x3.234", flags, 0.0f, processed, allUsed));
    EXPECT_EQ(3, processed);

    EXPECT_TRUE(::isnan(StrToF("x3", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);
}

TEST(Convert, StringToFloatOctalString)
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

    EXPECT_TRUE(::isnan(StrToF("01234567e0", flags, Single::Nan(), processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToF(" 012", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 00", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 012", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0123456789", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" + 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" - 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 012 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 00 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 012 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0123456789 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" + 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" - 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("012 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("00 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("012 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("0123456789 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF("01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_EQ(342391.0f, StrToF("+01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_EQ(-342391.0f, StrToF("-01234567", flags, Single::Nan(), processed, allUsed));
    EXPECT_TRUE(allUsed);

    EXPECT_TRUE(::isnan(StrToF("01234567e0", flags, Single::Nan(), processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToF(" 012", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 00", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 012", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0123456789", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" + 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" - 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 012 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 00 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 012 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0123456789 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" + 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" - 01234567 ", flags, Single::Nan(), processed, allUsed)));
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

    EXPECT_TRUE(::isnan(StrToF(" 012", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 00", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 012", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0123456789", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" + 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" - 01234567", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 012 ", flags, 0.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 00 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 012 ", flags, 1.0f, processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 0123456789 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" + 01234567 ", flags, Single::Nan(), processed, allUsed)));
    EXPECT_EQ(0, processed);

    EXPECT_TRUE(::isnan(StrToF(" - 01234567 ", flags, Single::Nan(), processed, allUsed)));
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

TEST(Convert, StringToFloatSpecialValues)
{
    size_t processed;
    AtodFlags flags = AtodFlags::Default;

    {
        // Use 1.0 as junk_string_value.
        StringToDoubleConverter<char> converter(flags, 0.0f, 1.0f, "infinity", "NaN");

        EXPECT_TRUE(::isnan(converter.StringToDouble("+NaN", 4, processed)));
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

TEST(Convert, StringToDoubleFloatWhitespace)
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
    template <typename T, typename TChar, size_t Size>
    inline void TestToDecimal(T value, const TChar* str)
    {
        TChar allocated1[Size];
        TChar allocated2[Size];
        size_t length = char_traits<TChar>::length(str);

        size_t s1 = Convert::ToDecimalString(value, allocated1);
        size_t s2 = Convert::ToDecimalString(value, allocated2, Size);
        EXPECT_EQ(length, s1);
        EXPECT_EQ(s1, s2);
        EXPECT_STREQ(allocated1, allocated2);
        EXPECT_STREQ(str, allocated1);
        EXPECT_STREQ(str, allocated2);
    }

    template <typename T, typename TChar, size_t Size>
    inline void TestToHex(T value, const TChar* str)
    {
        TChar allocated1[Size];
        TChar allocated2[Size];
        size_t length = char_traits<TChar>::length(str);

        size_t s1 = Convert::ToHexString(value, allocated1);
        size_t s2 = Convert::ToHexString(value, allocated2, Size);
        EXPECT_EQ(length, s1);
        EXPECT_EQ(s1, s2);
        EXPECT_STREQ(allocated1, allocated2);
        EXPECT_STREQ(str, allocated1);
        EXPECT_STREQ(str, allocated2);
    }

    template <typename T, typename TChar, size_t Size>
    inline void TestToHexLower(T value, const TChar* str)
    {
        TChar allocated1[Size];
        TChar allocated2[Size];
        size_t length = char_traits<TChar>::length(str);

        size_t s1 = Convert::ToHexStringLower(value, allocated1);
        size_t s2 = Convert::ToHexStringLower(value, allocated2, Size);
        EXPECT_EQ(length, s1);
        EXPECT_EQ(s1, s2);
        EXPECT_STREQ(allocated1, allocated2);
        EXPECT_STREQ(str, allocated1);
        EXPECT_STREQ(str, allocated2);
    }

    template <typename TChar>
    inline void TestParseInt(const TChar* str, int64_t expectResult, int expectReadCount = -1)
    {
        size_t readCount = 0;
        int64_t result = Convert::ParseInt<TChar>(str, readCount);

        EXPECT_EQ(expectResult, result);
        if (expectReadCount < 0)
            EXPECT_EQ(char_traits<TChar>::length(str), readCount);
        else
            EXPECT_EQ(static_cast<size_t>(expectReadCount), readCount);
    }

    template <typename TChar>
    inline void TestParseUInt(const TChar* str, uint64_t expectResult, int expectReadCount = -1)
    {
        size_t readCount = 0;
        uint64_t result = Convert::ParseUInt<TChar>(str, readCount);

        EXPECT_EQ(expectResult, result);
        if (expectReadCount < 0)
            EXPECT_EQ(char_traits<TChar>::length(str), readCount);
        else
            EXPECT_EQ(static_cast<size_t>(expectReadCount), readCount);
    }
}

TEST(Convert, ToDecimalString)
{
    TestToDecimal<int8_t, char, 5>(-128, "-128");
    TestToDecimal<int8_t, char, 5>(127, "127");
    TestToDecimal<uint8_t, char, 4>(0, "0");
    TestToDecimal<uint8_t, char, 4>(255, "255");
    TestToDecimal<int16_t, char, 7>(-32768, "-32768");
    TestToDecimal<int16_t, char, 7>(32767, "32767");
    TestToDecimal<uint16_t, char, 6>(0, "0");
    TestToDecimal<uint16_t, char, 6>(65535, "65535");
    TestToDecimal<int32_t, char, 12>(-2147483648, "-2147483648");
    TestToDecimal<int32_t, char, 12>(2147483647, "2147483647");
    TestToDecimal<uint32_t, char, 11>(0, "0");
    TestToDecimal<uint32_t, char, 11>(4294967295, "4294967295");
    TestToDecimal<int64_t, char, 21>(-9223372036854775808ull, "-9223372036854775808");
    TestToDecimal<int64_t, char, 21>(9223372036854775807ull, "9223372036854775807");
    TestToDecimal<uint64_t, char, 21>(0, "0");
    TestToDecimal<uint64_t, char, 21>(18446744073709551615ull, "18446744073709551615");

    TestToDecimal<int8_t, wchar_t, 5>(-128, L"-128");
    TestToDecimal<int8_t, wchar_t, 5>(127, L"127");
    TestToDecimal<uint8_t, wchar_t, 4>(0, L"0");
    TestToDecimal<uint8_t, wchar_t, 4>(255, L"255");
    TestToDecimal<int16_t, wchar_t, 7>(-32768, L"-32768");
    TestToDecimal<int16_t, wchar_t, 7>(32767, L"32767");
    TestToDecimal<uint16_t, wchar_t, 6>(0, L"0");
    TestToDecimal<uint16_t, wchar_t, 6>(65535, L"65535");
    TestToDecimal<int32_t, wchar_t, 12>(-2147483648, L"-2147483648");
    TestToDecimal<int32_t, wchar_t, 12>(2147483647, L"2147483647");
    TestToDecimal<uint32_t, wchar_t, 11>(0, L"0");
    TestToDecimal<uint32_t, wchar_t, 11>(4294967295, L"4294967295");
    TestToDecimal<int64_t, wchar_t, 21>(-9223372036854775808ull, L"-9223372036854775808");
    TestToDecimal<int64_t, wchar_t, 21>(9223372036854775807ull, L"9223372036854775807");
    TestToDecimal<uint64_t, wchar_t, 21>(0, L"0");
    TestToDecimal<uint64_t, wchar_t, 21>(18446744073709551615ull, L"18446744073709551615");

    TestToDecimal<int8_t, char, 5>(1, "1");
    TestToDecimal<int8_t, char, 5>(123, "123");
    TestToDecimal<uint8_t, char, 4>(1, "1");
    TestToDecimal<uint8_t, char, 4>(123, "123");
    TestToDecimal<int16_t, char, 7>(1, "1");
    TestToDecimal<int16_t, char, 7>(12345, "12345");
    TestToDecimal<uint16_t, char, 6>(1, "1");
    TestToDecimal<uint16_t, char, 6>(12345, "12345");
    TestToDecimal<int32_t, char, 12>(1, "1");
    TestToDecimal<int32_t, char, 12>(1234567890, "1234567890");
    TestToDecimal<uint32_t, char, 11>(1, "1");
    TestToDecimal<uint32_t, char, 11>(1234567890, "1234567890");
    TestToDecimal<int64_t, char, 21>(1, "1");
    TestToDecimal<int64_t, char, 21>(1234567890987654321ull, "1234567890987654321");
    TestToDecimal<uint64_t, char, 21>(1, "1");
    TestToDecimal<uint64_t, char, 21>(1234567890987654321ull, "1234567890987654321");
}

TEST(Convert, ToHexString)
{
    TestToHex<uint64_t, char, 17>(0, "0");
    TestToHex<uint64_t, char, 17>(0xABCDEF, "ABCDEF");
    TestToHex<uint64_t, char, 17>(0x12345678, "12345678");
    TestToHex<uint64_t, char, 17>(0xFFFFFFFFFFFFFFFFull, "FFFFFFFFFFFFFFFF");

    TestToHexLower<uint64_t, char, 17>(0, "0");
    TestToHexLower<uint64_t, char, 17>(0xABCDEF, "abcdef");
    TestToHexLower<uint64_t, char, 17>(0x12345678, "12345678");
    TestToHexLower<uint64_t, char, 17>(0xFFFFFFFFFFFFFFFFull, "ffffffffffffffff");

    TestToHex<uint32_t, char, 9>(0, "0");
    TestToHex<uint32_t, char, 9>(0xABCD, "ABCD");
    TestToHex<uint32_t, char, 9>(0x12345678, "12345678");
    TestToHex<uint32_t, char, 9>(0xFFFFFFFF, "FFFFFFFF");

    TestToHexLower<uint32_t, char, 9>(0, "0");
    TestToHexLower<uint32_t, char, 9>(0xABCD, "abcd");
    TestToHexLower<uint32_t, char, 9>(0x12345678, "12345678");
    TestToHexLower<uint32_t, char, 9>(0xFFFFFFFF, "ffffffff");

    TestToHex<uint16_t, char, 5>(0, "0");
    TestToHex<uint16_t, char, 5>(0xEF, "EF");
    TestToHex<uint16_t, char, 5>(0xABCD, "ABCD");
    TestToHex<uint16_t, char, 5>(0x1234, "1234");
    TestToHex<uint16_t, char, 5>(0xFFFF, "FFFF");

    TestToHexLower<uint16_t, char, 5>(0, "0");
    TestToHexLower<uint16_t, char, 5>(0xEF, "ef");
    TestToHexLower<uint16_t, char, 5>(0xABCD, "abcd");
    TestToHexLower<uint16_t, char, 5>(0x1234, "1234");
    TestToHexLower<uint16_t, char, 5>(0xFFFF, "ffff");

    TestToHex<uint8_t, char, 3>(0, "0");
    TestToHex<uint8_t, char, 3>(0xAB, "AB");
    TestToHex<uint8_t, char, 3>(0xCD, "CD");
    TestToHex<uint8_t, char, 3>(0xEF, "EF");
    TestToHex<uint8_t, char, 3>(0x12, "12");
    TestToHex<uint8_t, char, 3>(0xFF, "FF");

    TestToHexLower<uint8_t, char, 3>(0, "0");
    TestToHexLower<uint8_t, char, 3>(0xAB, "ab");
    TestToHexLower<uint8_t, char, 3>(0xCD, "cd");
    TestToHexLower<uint8_t, char, 3>(0xEF, "ef");
    TestToHexLower<uint8_t, char, 3>(0x12, "12");
    TestToHexLower<uint8_t, char, 3>(0xFF, "ff");
}

TEST(Convert, ParseInt)
{
    TestParseInt<char>("", 0, 0);
    TestParseInt<char>("a0", 0, 0);
    TestParseInt<char>(" a 0", 0, 0);
    TestParseInt<char>(" -x0", 0, 0);
    TestParseInt<char>("0x", 0, 0);
    TestParseInt<char>("x 0", 0, 0);
    TestParseInt<char>("-", 0, 0);

    TestParseInt<char>("0", 0);
    TestParseInt<char>(" 0", 0);
    TestParseInt<char>(" +0", 0);
    TestParseInt<char>(" -0", 0);
    TestParseInt<char>(" + 0", 0);
    TestParseInt<char>(" - 0", 0);
    TestParseInt<char>(" 0.", 0, 2);
    TestParseInt<char>(" 0 ", 0, 3);
    TestParseInt<char>("0 x ", 0, 2);
    TestParseInt<char>("0 x 0", 0, 2);

    TestParseInt<char>("1", 1);
    TestParseInt<char>(" 1", 1);
    TestParseInt<char>(" +1", 1);
    TestParseInt<char>(" -1", -1);
    TestParseInt<char>(" + 1", 1);
    TestParseInt<char>(" - 1", -1);
    TestParseInt<char>(" 1.", 1, 2);
    TestParseInt<char>(" 1 ", 1, 3);

    TestParseInt<char>("1234567890", 1234567890);
    TestParseInt<char>("01234567890", 1234567890);
    TestParseInt<char>("  1234567890", 1234567890);
    TestParseInt<char>("1234567890  ", 1234567890);
    TestParseInt<char>("-1234567890", -1234567890);
    TestParseInt<char>("-01234567890", -1234567890);
    TestParseInt<char>("  -1234567890", -1234567890);
    TestParseInt<char>(" -  1234567890", -1234567890);
    TestParseInt<char>("-  1234567890", -1234567890);
    TestParseInt<char>("-1234 567890", -1234, 6);
    TestParseInt<char>("+1234567890", 1234567890);
    TestParseInt<char>("+01234567890", 1234567890);
    TestParseInt<char>("  +1234567890", 1234567890);
    TestParseInt<char>(" +  1234567890", 1234567890);
    TestParseInt<char>("+  1234567890", 1234567890);
    TestParseInt<char>("+1234 567890", 1234, 6);

    TestParseInt<char>("0xABCDEF", 0xABCDEF);
    TestParseInt<char>("0x12345678", 0x12345678);
    TestParseInt<char>("0x00ABCD", 0xABCD);
    TestParseInt<char>("0x000", 0x0);
    TestParseInt<char>("0x1", 0x1);
    TestParseInt<char>("A", 0, 0);

    TestParseInt<char>("0Xabcdef", 0xABCDEF);
    TestParseInt<char>("0X12345678", 0x12345678);
    TestParseInt<char>("0X00abcd", 0xABCD);
    TestParseInt<char>("0X000", 0x0);
    TestParseInt<char>("0X1", 0x1);
    TestParseInt<char>("a", 0, 0);

    TestParseInt<char>("0", 0);
    TestParseInt<char>("-9223372036854775808", numeric_limits<int64_t>::min());
    TestParseInt<char>("9223372036854775807", numeric_limits<int64_t>::max());

    TestParseUInt<char>("0", 0);
    TestParseUInt<char>("18446744073709551615", numeric_limits<uint64_t>::max());
}
