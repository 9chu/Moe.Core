/**
 * @file
 * @date 2017/5/1
 */
#include <gtest/gtest.h>

#include <Moe.Core/Algorithm/internal/DiyFp.hpp>

using namespace std;
using namespace moe;
using namespace internal;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(DiyFp, Subtract)
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

TEST(DiyFp, Multiply)
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

TEST(DiyFp, Uint64Conversions)
{
    uint64_t ordered = 0x0123456789ABCDEFull;
    EXPECT_EQ(3512700564088504e-318, Double(ordered).ToDouble());

    uint64_t minDouble64 = 0x0000000000000001ull;
    EXPECT_EQ(5e-324, Double(minDouble64).ToDouble());

    uint64_t maxDouble64 = 0X7FEFFFFFFFFFFFFFull;
    EXPECT_EQ(1.7976931348623157e308, Double(maxDouble64).ToDouble());
}

TEST(DiyFp, Uint32Conversions)
{
    uint32_t ordered = 0x01234567;
    EXPECT_EQ(2.9988165487136453e-38f, Single(ordered).ToFloat());

    uint32_t minFloat32 = 0x00000001;
    EXPECT_EQ(1.4e-45f, Single(minFloat32).ToFloat());

    uint32_t maxFloat32 = 0X7F7FFFFF;
    EXPECT_EQ(3.4028234e38f, Single(maxFloat32).ToFloat());
}

TEST(DiyFp, DoubleToDiyFp)
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

TEST(DiyFp, SingleToDiyFp)
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

TEST(DiyFp, ToNormalizedDiyFp)
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

TEST(DiyFp, DoubleIsDenormal)
{
    uint64_t minDouble64 = 0x0000000000000001ull;
    EXPECT_TRUE(Double(minDouble64).IsDenormal());

    uint64_t bits = 0x000FFFFFFFFFFFFFull;
    EXPECT_TRUE(Double(bits).IsDenormal());

    bits = 0x0010000000000000ull;
    EXPECT_TRUE(!Double(bits).IsDenormal());
}

TEST(DiyFp, SingleIsDenormal)
{
    uint32_t minFloat32 = 0x00000001;
    EXPECT_TRUE(Single(minFloat32).IsDenormal());

    uint32_t bits = 0x007FFFFF;
    EXPECT_TRUE(Single(bits).IsDenormal());

    bits = 0x00800000;
    EXPECT_TRUE(!Single(bits).IsDenormal());
}

TEST(DiyFp, DoubleIsSpecial)
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

TEST(DiyFp, SingleIsSpecial)
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

TEST(DiyFp, DoubleIsInfinite)
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


TEST(DiyFp, SingleIsInfinite)
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

TEST(DiyFp, DoubleIsNan)
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

TEST(DiyFp, SingleIsNan)
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

TEST(DiyFp, DoubleSign)
{
    EXPECT_EQ(1, Double(1.0).Sign());
    EXPECT_EQ(1, Double(Double::Infinity()).Sign());
    EXPECT_EQ(-1, Double(-Double::Infinity()).Sign());
    EXPECT_EQ(1, Double(0.0).Sign());
    EXPECT_EQ(-1, Double(-0.0).Sign());

    uint64_t minDouble64 = 0x0000000000000001ull;
    EXPECT_EQ(1, Double(minDouble64).Sign());
}

TEST(DiyFp, SingleSign)
{
    EXPECT_EQ(1, Single(1.0f).Sign());
    EXPECT_EQ(1, Single(Single::Infinity()).Sign());
    EXPECT_EQ(-1, Single(-Single::Infinity()).Sign());
    EXPECT_EQ(1, Single(0.0f).Sign());
    EXPECT_EQ(-1, Single(-0.0f).Sign());

    uint32_t minFloat32 = 0x00000001;
    EXPECT_EQ(1, Single(minFloat32).Sign());
}

TEST(DiyFp, DoubleNormalizedBoundaries)
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

TEST(DiyFp, SingleNormalizedBoundaries)
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

TEST(DiyFp, NextDouble)
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
    EXPECT_EQ(Double::Infinity(), Double(0x7FEFFFFFFFFFFFFFull).NextDouble());
}

TEST(DiyFp, PreviousDouble)
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
    EXPECT_EQ(-Double::Infinity(), Double(0xFFEFFFFFFFFFFFFFull).PreviousDouble());
}
