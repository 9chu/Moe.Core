/**
 * @file
 * @date 2017/5/23
 */
#include <gtest/gtest.h>

#include <Moe.Core/Any.hpp>

using namespace std;
using namespace moe;

TEST(Any, Test)
{
    Any a = 12345;
    EXPECT_EQ(12345, a.SafeCastTo<int>());
    EXPECT_EQ(12345, const_cast<const Any&>(a).SafeCastTo<int>());
    EXPECT_THROW(a.CastTo<unsigned>(), std::bad_cast);
    EXPECT_THROW(const_cast<const Any&>(a).CastTo<unsigned>(), std::bad_cast);

    a = 12345u;
    EXPECT_EQ(12345u, a.SafeCastTo<unsigned>());
    EXPECT_EQ(12345u, const_cast<const Any&>(a).SafeCastTo<unsigned>());
    EXPECT_THROW(a.CastTo<int>(), std::bad_cast);
    EXPECT_THROW(const_cast<const Any&>(a).CastTo<int>(), std::bad_cast);

    a = 123.45;
    Any b = a;
    EXPECT_EQ(123.45, b.SafeCastTo<double>());

    Any c = std::move(a);
    EXPECT_EQ(123.45, c.SafeCastTo<double>());
    EXPECT_TRUE(a);

    a = 1;
    b = 2;
    a.Swap(b);
    EXPECT_EQ(2, a.SafeCastTo<int>());
    EXPECT_EQ(1, b.SafeCastTo<int>());

    a = 1;
    EXPECT_EQ(0., a.SafeCastTo<const double&>(0.));

    a = 1;
    a.CastTo<int&>() = 2;
    EXPECT_EQ(2, a.SafeCastTo<int>());
}

TEST(Any, TestHeapObject)
{
    struct SthBig
    {
        std::string A;
        std::string B;
    };

    Any a = SthBig { "hello", "world" };
    EXPECT_EQ("hello", a.SafeCastTo<SthBig>().A);
    EXPECT_EQ("world", a.SafeCastTo<SthBig>().B);

    Any b = std::move(a);
    EXPECT_EQ("hello", b.SafeCastTo<SthBig>().A);
    EXPECT_EQ("world", b.SafeCastTo<SthBig>().B);
    EXPECT_TRUE(a);
}
