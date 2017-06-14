/**
 * @file
 * @date 2017/6/14
 */
#include <gtest/gtest.h>

#include <Moe.Core/Buffer.hpp>

using namespace std;
using namespace moe;

TEST(Buffer, Init)
{
    char buffer[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    Buffer<4> b1(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    EXPECT_EQ(8, b1.GetCapacity());
    EXPECT_EQ(8, b1.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b1[i]);

    Buffer<8> b2(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    EXPECT_EQ(8, b2.GetCapacity());
    EXPECT_EQ(8, b2.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b2[i]);

    Buffer<16> b3(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    EXPECT_EQ(16, b3.GetCapacity());
    EXPECT_EQ(8, b3.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b3[i]);

    Buffer<5> b4(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    EXPECT_EQ(8, b4.GetCapacity());
    EXPECT_EQ(8, b4.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b4[i]);
}

TEST(Buffer, Copy)
{
    char buffer[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    Buffer<4> a(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    EXPECT_EQ(8, a.GetCapacity());
    EXPECT_EQ(8, a.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, a[i]);

    Buffer<2> a1 = a;
    EXPECT_EQ(8, a1.GetCapacity());
    EXPECT_EQ(8, a1.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, a1[i]);

    Buffer<10> a2 = a;
    EXPECT_EQ(10, a2.GetCapacity());
    EXPECT_EQ(8, a2.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, a2[i]);

    Buffer<10> b(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    EXPECT_EQ(10, b.GetCapacity());
    EXPECT_EQ(8, b.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b[i]);

    Buffer<4> b1 = b;
    EXPECT_EQ(8, b1.GetCapacity());
    EXPECT_EQ(8, b1.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b1[i]);

    Buffer<16> b2 = b;
    EXPECT_EQ(16, b2.GetCapacity());
    EXPECT_EQ(8, b2.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b2[i]);
}

TEST(Buffer, XCopy)
{
    char buffer[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    Buffer<4> test1Raw(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    Buffer<16> test1 = std::move(test1Raw);
    EXPECT_EQ(0, test1Raw.GetSize());
    EXPECT_EQ(8, test1Raw.GetCapacity());
    EXPECT_EQ(8, test1.GetSize());
    EXPECT_EQ(16, test1.GetCapacity());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, test1[i]);

    Buffer<6> test2Raw(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    auto test2RawPointer = test2Raw.GetBuffer();
    Buffer<4> test2 = std::move(test2Raw);
    auto test2Pointer = test2.GetBuffer();
    EXPECT_EQ(0, test2Raw.GetSize());
    EXPECT_EQ(6, test2Raw.GetCapacity());
    EXPECT_EQ(8, test2.GetSize());
    EXPECT_EQ(8, test2.GetCapacity());
    EXPECT_EQ(test2RawPointer, test2Pointer);
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, test2[i]);

    Buffer<16> test3Raw(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    Buffer<4> test3 = std::move(test3Raw);
    EXPECT_EQ(0, test3Raw.GetSize());
    EXPECT_EQ(16, test3Raw.GetCapacity());
    EXPECT_EQ(8, test3.GetSize());
    EXPECT_EQ(8, test3.GetCapacity());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, test3[i]);
}
