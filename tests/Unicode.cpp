/**
 * @file
 * @author chu
 * @date 2018/6/6
 */
#include <gtest/gtest.h>
#include <cstdarg>

#include <Moe.Core/Unicode.hpp>

#include "Data/UnicodeNormalizeData.hpp"

using namespace std;
using namespace moe;

//static void U32_(u32string& out, char32_t ch)
//{
//    out.push_back(ch);
//}

//template <typename... Args>
//static void U32_(u32string& out, char32_t ch, Args... chars)
//{
//    U32_(out, ch);
//    U32_(out, chars...);
//}

//template <typename... Args>
//static u32string U32(Args... chars)
//{
//    u32string ret;
//    U32_(ret, chars...);
//    return ret;
//}

template <size_t Size>
static u32string ToStr(const char32_t (&buffer)[Size])
{
    auto len = Size;
    for (size_t i = 0; i < Size; ++i)
    {
        if (buffer[i] == 0)
        {
            len = i;
            break;
        }
    }
    return u32string(buffer, len);
}

template <size_t Size>
static u32string Nfd(const char32_t (&buffer)[Size])
{
    auto len = Size;
    for (size_t i = 0; i < Size; ++i)
    {
        if (buffer[i] == 0)
        {
            len = i;
            break;
        }
    }

    u32string ret;
    Unicode::Normalize(ret, ArrayView<char32_t>(buffer, len), Unicode::NormalizationFormType::NFD);
    return ret;
}

template <size_t Size>
static u32string Nfkd(const char32_t (&buffer)[Size])
{
    auto len = Size;
    for (size_t i = 0; i < Size; ++i)
    {
        if (buffer[i] == 0)
        {
            len = i;
            break;
        }
    }

    u32string ret;
    Unicode::Normalize(ret, ArrayView<char32_t>(buffer, len), Unicode::NormalizationFormType::NFKD);
    return ret;
}

template <size_t Size>
static u32string Nfc(const char32_t (&buffer)[Size])
{
    auto len = Size;
    for (size_t i = 0; i < Size; ++i)
    {
        if (buffer[i] == 0)
        {
            len = i;
            break;
        }
    }

    u32string ret;
    Unicode::Normalize(ret, ArrayView<char32_t>(buffer, len), Unicode::NormalizationFormType::NFC);
    return ret;
}

template <size_t Size>
static u32string Nfkc(const char32_t (&buffer)[Size])
{
    auto len = Size;
    for (size_t i = 0; i < Size; ++i)
    {
        if (buffer[i] == 0)
        {
            len = i;
            break;
        }
    }

    u32string ret;
    Unicode::Normalize(ret, ArrayView<char32_t>(buffer, len), Unicode::NormalizationFormType::NFKC);
    return ret;
}

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
    // https://unicode.org/Public/10.0.0/ucd/NormalizationTest.txt
    auto& tests = Testing::GetUnicodeNormalizeTestRecords();
    for (size_t i = 0; i < tests.GetSize(); ++i)
    {
        EXPECT_EQ(ToStr(tests[i].Nfc), Nfc(tests[i].Source));
        EXPECT_EQ(ToStr(tests[i].Nfd), Nfd(tests[i].Source));
        EXPECT_EQ(ToStr(tests[i].Nfkc), Nfkc(tests[i].Source));
        EXPECT_EQ(ToStr(tests[i].Nfkd), Nfkd(tests[i].Source));
    }
}
