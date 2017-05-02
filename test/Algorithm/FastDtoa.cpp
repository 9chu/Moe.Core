/**
 * @file
 * @date 2017/5/2
 */
#include <gtest/gtest.h>

#include <Moe.Core/Algorithm/internal/FastDtoa.hpp>

using namespace std;
using namespace moe;
using namespace internal;

static const int kBufferSize = 100;

namespace
{
    template <typename T>
    inline void TrimRepresentation(ArrayView<T>& representation)
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

TEST(FastDtoa, FastDtoaShortestVariousDoubles)
{
    char bufferContainer[kBufferSize];
    ArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    size_t point;
    bool status;

    double minDouble = 5e-324;
    status = FastDtoa::Dtoa(minDouble, FastDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("5", buffer.GetBuffer());
    EXPECT_EQ(-323, point);

    double maxDouble = 1.7976931348623157e308;
    status = FastDtoa::Dtoa(maxDouble, FastDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("17976931348623157", buffer.GetBuffer());
    EXPECT_EQ(309, point);

    status = FastDtoa::Dtoa(4294967272.0, FastDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("4294967272", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    status = FastDtoa::Dtoa(4.1855804968213567e298, FastDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("4185580496821357", buffer.GetBuffer());
    EXPECT_EQ(299, point);

    status = FastDtoa::Dtoa(5.5626846462680035e-309, FastDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("5562684646268003", buffer.GetBuffer());
    EXPECT_EQ(-308, point);

    status = FastDtoa::Dtoa(2147483648.0, FastDtoaMode::Shortest, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("2147483648", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    status = FastDtoa::Dtoa(3.5844466002796428e+298, FastDtoaMode::Shortest, 0, buffer, length, point);
    if (status)
    {
        EXPECT_EQ("35844466002796428", buffer.GetBuffer());
        EXPECT_EQ(299, point);
    }

    uint64_t smallestNormal64 = 0x0010000000000000ull;
    double v = Double(smallestNormal64).ToDouble();
    status = FastDtoa::Dtoa(v, FastDtoaMode::Shortest, 0, buffer, length, point);
    if (status)
    {
        EXPECT_EQ("22250738585072014", buffer.GetBuffer());
        EXPECT_EQ(-307, point);
    }

    uint64_t largestDenormal64 = 0x000FFFFFFFFFFFFFull;
    v = Double(largestDenormal64).ToDouble();
    status = FastDtoa::Dtoa(v, FastDtoaMode::Shortest, 0, buffer, length, point);
    if (status)
    {
        EXPECT_EQ("2225073858507201", buffer.GetBuffer());
        EXPECT_EQ(-307, point);
    }
}

TEST(FastDtoa, FastDtoaShortestVariousFloats)
{
    char bufferContainer[kBufferSize];
    ArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    size_t point;
    bool status;

    float minFloat = 1e-45f;
    status = FastDtoa::Dtoa(minFloat, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("1", buffer.GetBuffer());
    EXPECT_EQ(-44, point);

    float maxFloat = 3.4028234e38f;
    status = FastDtoa::Dtoa(maxFloat, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("34028235", buffer.GetBuffer());
    EXPECT_EQ(39, point);

    status = FastDtoa::Dtoa(4294967272.0f, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("42949673", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    status = FastDtoa::Dtoa(3.32306998946228968226e+35f, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("332307", buffer.GetBuffer());
    EXPECT_EQ(36, point);

    status = FastDtoa::Dtoa(1.2341e-41f, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("12341", buffer.GetBuffer());
    EXPECT_EQ(-40, point);

    status = FastDtoa::Dtoa(3.3554432e7, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("33554432", buffer.GetBuffer());
    EXPECT_EQ(8, point);

    status = FastDtoa::Dtoa(3.26494756798464e14f, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("32649476", buffer.GetBuffer());
    EXPECT_EQ(15, point);

    status = FastDtoa::Dtoa(3.91132223637771935344e37f, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    if (status)
    {
        EXPECT_EQ("39113222", buffer.GetBuffer());
        EXPECT_EQ(38, point);
    }

    uint32_t smallestNormal32 = 0x00800000;
    float v = Single(smallestNormal32).ToFloat();
    status = FastDtoa::Dtoa(v, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    if (status)
    {
        EXPECT_EQ("11754944", buffer.GetBuffer());
        EXPECT_EQ(-37, point);
    }

    uint32_t largestDenormal32 = 0x007FFFFF;
    v = Single(largestDenormal32).ToFloat();
    status = FastDtoa::Dtoa(v, FastDtoaMode::ShortestSingle, 0, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("11754942", buffer.GetBuffer());
    EXPECT_EQ(-37, point);
}

TEST(FastDtoa, FastDtoaPrecisionVariousDoubles)
{
    char bufferContainer[kBufferSize];
    ArrayView<char> buffer(bufferContainer, kBufferSize);
    size_t length;
    size_t point;
    bool status;

    status = FastDtoa::Dtoa(1.0, FastDtoaMode::Precision, 3, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_TRUE(3 >= length);
    TrimRepresentation(buffer);
    EXPECT_EQ("1", buffer.GetBuffer());
    EXPECT_EQ(1, point);

    status = FastDtoa::Dtoa(1.5, FastDtoaMode::Precision, 10, buffer, length, point);
    if (status)
    {
        EXPECT_TRUE(10 >= length);
        TrimRepresentation(buffer);
        EXPECT_EQ("15", buffer.GetBuffer());
        EXPECT_EQ(1, point);
    }

    double minDouble = 5e-324;
    status = FastDtoa::Dtoa(minDouble, FastDtoaMode::Precision, 5, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("49407", buffer.GetBuffer());
    EXPECT_EQ(-323, point);

    double maxDouble = 1.7976931348623157e308;
    status = FastDtoa::Dtoa(maxDouble, FastDtoaMode::Precision, 7, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("1797693", buffer.GetBuffer());
    EXPECT_EQ(309, point);

    status = FastDtoa::Dtoa(4294967272.0, FastDtoaMode::Precision, 14, buffer, length, point);
    if (status)
    {
        EXPECT_TRUE(14 >= length);
        TrimRepresentation(buffer);
        EXPECT_EQ("4294967272", buffer.GetBuffer());
        EXPECT_EQ(10, point);
    }

    status = FastDtoa::Dtoa(4.1855804968213567e298, FastDtoaMode::Precision, 17, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("41855804968213567", buffer.GetBuffer());
    EXPECT_EQ(299, point);

    status = FastDtoa::Dtoa(5.5626846462680035e-309, FastDtoaMode::Precision, 1, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("6", buffer.GetBuffer());
    EXPECT_EQ(-308, point);

    status = FastDtoa::Dtoa(2147483648.0, FastDtoaMode::Precision, 5, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("21475", buffer.GetBuffer());
    EXPECT_EQ(10, point);

    status = FastDtoa::Dtoa(3.5844466002796428e+298, FastDtoaMode::Precision, 10, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_TRUE(10 >= length);
    TrimRepresentation(buffer);
    EXPECT_EQ("35844466", buffer.GetBuffer());
    EXPECT_EQ(299, point);

    uint64_t smallestNormal64 = 0x0010000000000000ull;
    double v = Double(smallestNormal64).ToDouble();
    status = FastDtoa::Dtoa(v, FastDtoaMode::Precision, 17, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("22250738585072014", buffer.GetBuffer());
    EXPECT_EQ(-307, point);

    uint64_t largestDenormal64 = 0x000FFFFFFFFFFFFFull;
    v = Double(largestDenormal64).ToDouble();
    status = FastDtoa::Dtoa(v, FastDtoaMode::Precision, 17, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_TRUE(20 >= length);
    TrimRepresentation(buffer);
    EXPECT_EQ("22250738585072009", buffer.GetBuffer());
    EXPECT_EQ(-307, point);

    v = 3.3161339052167390562200598e-237;
    status = FastDtoa::Dtoa(v, FastDtoaMode::Precision, 18, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("331613390521673906", buffer.GetBuffer());
    EXPECT_EQ(-236, point);

    v = 7.9885183916008099497815232e+191;
    status = FastDtoa::Dtoa(v, FastDtoaMode::Precision, 4, buffer, length, point);
    EXPECT_TRUE(status);
    EXPECT_EQ("7989", buffer.GetBuffer());
    EXPECT_EQ(192, point);
}

TEST(FastDtoa, FastDtoaGayShortest)
{
    char bufferContainer[kBufferSize];
    ArrayView<char> buffer(bufferContainer, kBufferSize);
    bool status;
    size_t length;
    size_t point;
    int succeeded = 0;
    int total = 0;
    bool neededMaxLength = false;

    ArrayView<const PrecomputedShortest> precomputed = PrecomputedShortestRepresentations();
    for (size_t i = 0; i < precomputed.Size(); ++i)
    {
        const PrecomputedShortest currentTest = precomputed[i];
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
        EXPECT_EQ(currentTest.representation, buffer.GetBuffer());
    }

    EXPECT_TRUE(succeeded * 1.0 / total > 0.99);
    EXPECT_TRUE(neededMaxLength);
}

TEST(FastDtoa, FastDtoaGayShortestSingle)
{
    char bufferContainer[kBufferSize];
    ArrayView<char> buffer(bufferContainer, kBufferSize);
    bool status;
    int length;
    int point;
    int succeeded = 0;
    int total = 0;
    bool needed_max_length = false;

    ArrayView<const PrecomputedShortestSingle> precomputed = PrecomputedShortestSingleRepresentations();
    for (int i = 0; i < precomputed.Size(); ++i)
    {
        const PrecomputedShortestSingle currentTest = precomputed[i];
        total++;
        float v = current_test.v;
        status = FastDtoa(v, FAST_DTOA_SHORTEST_SINGLE, 0, buffer, &length, &point);
        CHECK(kFastDtoaMaximalSingleLength >= length);
        if (!status) continue;
        if (length == kFastDtoaMaximalSingleLength) needed_max_length = true;
        succeeded++;
        CHECK_EQ(current_test.decimal_point, point);
        CHECK_EQ(current_test.representation, buffer.start());
    }
    CHECK(succeeded*1.0/total > 0.98);
    CHECK(needed_max_length);
}

TEST(FastDtoa, FastDtoaGayPrecision)
{
    char buffer_container[kBufferSize];
    ArrayView<char> buffer(buffer_container, kBufferSize);
    bool status;
    int length;
    int point;
    int succeeded = 0;
    int total = 0;
    // Count separately for entries with less than 15 requested digits.
    int succeeded_15 = 0;
    int total_15 = 0;

    Vector<const PrecomputedPrecision> precomputed =
        PrecomputedPrecisionRepresentations();
    for (int i = 0; i < precomputed.length(); ++i) {
        const PrecomputedPrecision current_test = precomputed[i];
        double v = current_test.v;
        int number_digits = current_test.number_digits;
        total++;
        if (number_digits <= 15) total_15++;
        status = FastDtoa(v, FAST_DTOA_PRECISION, number_digits,
            buffer, &length, &point);
        CHECK(number_digits >= length);
        if (!status) continue;
        succeeded++;
        if (number_digits <= 15) succeeded_15++;
        TrimRepresentation(buffer);
        CHECK_EQ(current_test.decimal_point, point);
        CHECK_EQ(current_test.representation, buffer.start());
    }
    // The precomputed numbers contain many entries with many requested
    // digits. These have a high failure rate and we therefore expect a lower
    // success rate than for the shortest representation.
    CHECK(succeeded*1.0/total > 0.85);
    // However with less than 15 digits almost the algorithm should almost always
    // succeed.
    CHECK(succeeded_15*1.0/total_15 > 0.9999);
}
