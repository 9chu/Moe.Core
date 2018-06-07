/**
 * @file
 * @date 2017/5/30
 */
#include <gtest/gtest.h>

#include <Moe.Core/RefPtr.hpp>

using namespace std;
using namespace moe;

struct foo :
    RefBase<foo>
{
    foo() {}
    virtual ~foo() {}
};

struct bar :
    public foo
{
    bar()
        : foo() {}
};

TEST(RefPtr, Test)
{
    RefPtr<bar> p = MakeRef<bar>();
    EXPECT_EQ(1, p.GetRefCount());

    RefWeakPtr<foo> wp = p;
    EXPECT_EQ(1, wp.GetWeakRefCount());
    EXPECT_EQ(1, p.GetRefCount());

    {
        auto lp = wp.Lock();
        EXPECT_EQ(2, lp.GetRefCount());
    }
    EXPECT_EQ(1, p.GetRefCount());

    p.Reset();
    EXPECT_TRUE(wp.IsExpired());
}
