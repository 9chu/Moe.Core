/**
 * @file
 * @date 2017/5/5
 */
#include <gtest/gtest.h>

#include <Moe.Core/Algorithm/internal/BignumDtoa.hpp>

#include "Data/DtoaPrecomputedShortest.hpp"
#include "Data/DtoaPrecomputedShortestSingle.hpp"
#include "Data/DtoaPrecomputedPrecision.hpp"
#include "Data/DtoaPrecomputedFixedRepresentations.hpp"

using namespace std;
using namespace moe;
using namespace internal;

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
}

static const int kBufferSize = 100;

TEST(BignumDtoa, BignumDtoaVariousDoubles)
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

TEST(BignumDtoa,BignumDtoaShortestVariousFloats)
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

TEST(BignumDtoa, BignumDtoaGayShortest)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;

    const ArrayView<Testing::PrecomputedShortest> precomputed = Testing::PrecomputedShortestRepresentations();
    for (size_t i = 0; i < precomputed.Size(); ++i)
    {
        const Testing::PrecomputedShortest& currentTest = precomputed[i];
        double v = currentTest.v;
        BignumDtoa::Dtoa(v, BignumDtoaMode::Shortest, 0, buffer, length, point);
        EXPECT_EQ(currentTest.decimalPoint, point);
        EXPECT_STREQ(currentTest.representation, buffer.GetBuffer());
    }
}

TEST(BignumDtoa, BignumDtoaGayShortestSingle)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;

    const ArrayView<Testing::PrecomputedShortestSingle> precomputed =
        Testing::PrecomputedShortestSingleRepresentations();
    for (size_t i = 0; i < precomputed.Size(); ++i)
    {
        const Testing::PrecomputedShortestSingle& currentTest = precomputed[i];
        float v = currentTest.v;
        BignumDtoa::Dtoa(v, BignumDtoaMode::ShortestSingle, 0, buffer, length, point);
        EXPECT_EQ(currentTest.decimalPoint, point);
        EXPECT_STREQ(currentTest.representation, buffer.GetBuffer());
    }
}

TEST(BignumDtoa, BignumDtoaGayFixed)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;

    const ArrayView<Testing::PrecomputedFixed> precomputed = Testing::PrecomputedFixedRepresentations();
    for (size_t i = 0; i < precomputed.Size(); ++i)
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

TEST(BignumDtoa, BignumDtoaGayPrecision)
{
    char bufferContainer[kBufferSize];
    MutableArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    int point;

    const ArrayView<Testing::PrecomputedPrecision> precomputed = Testing::PrecomputedPrecisionRepresentations();
    for (size_t i = 0; i < precomputed.Size(); ++i)
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
