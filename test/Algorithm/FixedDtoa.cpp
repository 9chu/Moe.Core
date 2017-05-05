/**
 * @file
 * @date 2017/5/3
 */
#include <gtest/gtest.h>

#include <Moe.Core/Algorithm/internal/FixedDtoa.hpp>

#include "Data/DtoaPrecomputedFixedRepresentations.hpp"

using namespace std;
using namespace moe;
using namespace internal;

static const int kBufferSize = 500;

TEST(FixedDtoa, FastFixedVariousDoubles)
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

TEST(FixedDtoa, FastFixedDtoaGayFixed)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    bool status;
    size_t length;
    int point;

    const ArrayView<Testing::PrecomputedFixed> precomputed = Testing::PrecomputedFixedRepresentations();
    for (size_t i = 0; i < precomputed.Size(); ++i)
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
