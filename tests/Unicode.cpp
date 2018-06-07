/**
 * @file
 * @author chu
 * @date 2018/6/6
 */
#include <gtest/gtest.h>

#include <Moe.Core/Unicode.hpp>

using namespace std;
using namespace moe;

TEST(Unicode, ConvertCase)
{
    EXPECT_EQ('1', Unicode::ToLowercase('1'));
    EXPECT_EQ(0x110000, Unicode::ToLowercase(0x110000));
    EXPECT_EQ('a', Unicode::ToLowercase('A'));
    EXPECT_EQ('z', Unicode::ToLowercase('Z'));

    EXPECT_EQ('1', Unicode::ToUppercase('1'));
    EXPECT_EQ(0x110000, Unicode::ToUppercase(0x110000));
    EXPECT_EQ('A', Unicode::ToUppercase('a'));
    EXPECT_EQ('Z', Unicode::ToUppercase('z'));
}

TEST(Unicode, Whitespace)
{
    EXPECT_TRUE(Unicode::IsWhitespace(' '));
}

TEST(Unicode, Decomposition)
{
    EXPECT_STREQ("", Unicode::Decomposition('a').c_str());
    EXPECT_STREQ("00C5", Unicode::Decomposition(0x212B).c_str());
}

TEST(Unicode, Normalize)
{
    // TODO
}
