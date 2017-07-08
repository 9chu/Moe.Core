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
    EXPECT_EQ(8u, b1.GetCapacity());
    EXPECT_EQ(8u, b1.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b1[i]);

    Buffer<8> b2(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    EXPECT_EQ(8u, b2.GetCapacity());
    EXPECT_EQ(8u, b2.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b2[i]);

    Buffer<16> b3(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    EXPECT_EQ(16u, b3.GetCapacity());
    EXPECT_EQ(8u, b3.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b3[i]);

    Buffer<5> b4(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    EXPECT_EQ(8u, b4.GetCapacity());
    EXPECT_EQ(8u, b4.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b4[i]);
}

TEST(Buffer, Copy)
{
    char buffer[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    Buffer<4> a(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    EXPECT_EQ(8u, a.GetCapacity());
    EXPECT_EQ(8u, a.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, a[i]);

    Buffer<2> a1 = a;
    EXPECT_EQ(8u, a1.GetCapacity());
    EXPECT_EQ(8u, a1.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, a1[i]);

    Buffer<10> a2 = a;
    EXPECT_EQ(10u, a2.GetCapacity());
    EXPECT_EQ(8u, a2.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, a2[i]);

    Buffer<10> b(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    EXPECT_EQ(10u, b.GetCapacity());
    EXPECT_EQ(8u, b.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b[i]);

    Buffer<4> b1 = b;
    EXPECT_EQ(8u, b1.GetCapacity());
    EXPECT_EQ(8u, b1.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b1[i]);

    Buffer<16> b2 = b;
    EXPECT_EQ(16u, b2.GetCapacity());
    EXPECT_EQ(8u, b2.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, b2[i]);
}

TEST(Buffer, XCopy)
{
    char buffer[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    Buffer<4> test1Raw(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    Buffer<16> test1 = std::move(test1Raw);
    EXPECT_EQ(0u, test1Raw.GetSize());
    EXPECT_EQ(8u, test1Raw.GetCapacity());
    EXPECT_EQ(8u, test1.GetSize());
    EXPECT_EQ(16u, test1.GetCapacity());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, test1[i]);

    Buffer<6> test2Raw(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    auto test2RawPointer = test2Raw.GetBuffer();
    Buffer<4> test2 = std::move(test2Raw);
    auto test2Pointer = test2.GetBuffer();
    EXPECT_EQ(0u, test2Raw.GetSize());
    EXPECT_EQ(6u, test2Raw.GetCapacity());
    EXPECT_EQ(8u, test2.GetSize());
    EXPECT_EQ(8u, test2.GetCapacity());
    EXPECT_EQ(test2RawPointer, test2Pointer);
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, test2[i]);

    Buffer<16> test3Raw(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    Buffer<4> test3 = std::move(test3Raw);
    EXPECT_EQ(0u, test3Raw.GetSize());
    EXPECT_EQ(16u, test3Raw.GetCapacity());
    EXPECT_EQ(8u, test3.GetSize());
    EXPECT_EQ(8u, test3.GetCapacity());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, test3[i]);
}

TEST(Buffer, ShiftLeft)
{
    char buffer[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    Buffer<> test1(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    test1.ShiftLeft(4, 4);
    EXPECT_EQ(4u, test1.GetSize());
    for (size_t i = 0; i < 4; ++i)
        EXPECT_EQ(i + 4, test1[i]);

    Buffer<> test2(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    test2.ShiftLeft(0, 8);
    EXPECT_EQ(8u, test2.GetSize());
    for (size_t i = 0; i < 8; ++i)
        EXPECT_EQ(i, test2[i]);

    Buffer<> test3(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
    test3.ShiftLeft(4, 8);
    EXPECT_EQ(4u, test3.GetSize());
    for (size_t i = 0; i < 4; ++i)
        EXPECT_EQ(i + 4, test3[i]);
}
