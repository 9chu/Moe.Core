/**
 * @file
 * @date 2017/5/21
 */
#include <gtest/gtest.h>

#include <Moe.Core/StringUtils.hpp>

using namespace std;
using namespace moe;
using namespace StringUtils;

TEST(StringUtils, ToLowerUpper)
{
    EXPECT_EQ('1', ToLower('1'));
    EXPECT_EQ('a', ToLower('a'));
    EXPECT_EQ('a', ToLower('A'));

    EXPECT_EQ('1', ToUpper('1'));
    EXPECT_EQ('A', ToUpper('a'));
    EXPECT_EQ('A', ToUpper('A'));

    EXPECT_STREQ("hello, world!", ToLower("Hello, World!").c_str());
    EXPECT_STREQ("HELLO, WORLD!", ToUpper("Hello, World!").c_str());
}

TEST(StringUtils, Trim)
{
    EXPECT_STREQ("", TrimLeft("").c_str());
    EXPECT_STREQ("", TrimLeft(" ").c_str());
    EXPECT_STREQ("abc", TrimLeft("abc").c_str());
    EXPECT_STREQ("a", TrimLeft(" a").c_str());
    EXPECT_STREQ("a", TrimLeft(" \ta").c_str());
    EXPECT_STREQ("a", TrimLeft(" \t\v a").c_str());
    EXPECT_STREQ("a ", TrimLeft(" a ").c_str());

    EXPECT_STREQ("", TrimRight("").c_str());
    EXPECT_STREQ("", TrimRight(" ").c_str());
    EXPECT_STREQ("abc", TrimRight("abc").c_str());
    EXPECT_STREQ("a", TrimRight("a ").c_str());
    EXPECT_STREQ("a", TrimRight("a\t ").c_str());
    EXPECT_STREQ("a", TrimRight("a \t\v ").c_str());
    EXPECT_STREQ(" a", TrimRight(" a ").c_str());

    EXPECT_STREQ("", Trim("").c_str());
    EXPECT_STREQ("", Trim(" ").c_str());
    EXPECT_STREQ("abc", Trim("abc").c_str());
    EXPECT_STREQ("a", Trim("a ").c_str());
    EXPECT_STREQ("a", Trim("a\t ").c_str());
    EXPECT_STREQ("a", Trim("a \t\v ").c_str());
    EXPECT_STREQ("a", Trim(" a ").c_str());
}

TEST(StringUtils, Join)
{
    vector<string> test1 = {};
    EXPECT_STREQ("", Join(test1.begin(), test1.end(), ',').c_str());
    EXPECT_STREQ("", Join(test1.begin(), test1.end(), ",").c_str());

    vector<string> test2 = { "a" };
    EXPECT_STREQ("a", Join(test2.begin(), test2.end(), ',').c_str());
    EXPECT_STREQ("a", Join(test2.begin(), test2.end(), ",").c_str());
    EXPECT_STREQ("a", Join(test2.begin(), test2.end(), "//").c_str());
    EXPECT_STREQ("a", Join(test2.begin(), test2.end(), "").c_str());

    vector<string> test3 = { "a", "b" };
    EXPECT_STREQ("a,b", Join(test3.begin(), test3.end(), ',').c_str());
    EXPECT_STREQ("a,b", Join(test3.begin(), test3.end(), ",").c_str());
    EXPECT_STREQ("a//b", Join(test3.begin(), test3.end(), "//").c_str());
    EXPECT_STREQ("ab", Join(test3.begin(), test3.end(), "").c_str());
}

TEST(StringUtils, Split)
{
    vector<string> out;

    EXPECT_EQ(1u, Split(out, "", '/'));
    EXPECT_TRUE(out.size() == 1);

    EXPECT_EQ(2u, Split(out, "/usr", '/'));
    EXPECT_TRUE(out.size() == 2);
    EXPECT_STREQ("", out[0].c_str());
    EXPECT_STREQ("usr", out[1].c_str());

    EXPECT_EQ(3u, Split(out, "/usr/var", '/'));
    EXPECT_TRUE(out.size() == 3);
    EXPECT_STREQ("", out[0].c_str());
    EXPECT_STREQ("usr", out[1].c_str());
    EXPECT_STREQ("var", out[2].c_str());

    EXPECT_EQ(4u, Split(out, "/usr/var/", '/'));
    EXPECT_TRUE(out.size() == 4);
    EXPECT_STREQ("", out[0].c_str());
    EXPECT_STREQ("usr", out[1].c_str());
    EXPECT_STREQ("var", out[2].c_str());
    EXPECT_STREQ("", out[3].c_str());

    EXPECT_EQ(2u, Split(out, "/usr/var/", '/', SplitFlags::RemoveEmptyEntries));
    EXPECT_TRUE(out.size() == 2);
    EXPECT_STREQ("usr", out[0].c_str());
    EXPECT_STREQ("var", out[1].c_str());

    EXPECT_EQ(1u, Split(out, "", "/"));
    EXPECT_TRUE(out.size() == 1);

    EXPECT_EQ(2u, Split(out, "/usr", "/"));
    EXPECT_TRUE(out.size() == 2);
    EXPECT_STREQ("", out[0].c_str());
    EXPECT_STREQ("usr", out[1].c_str());

    EXPECT_EQ(3u, Split(out, "/usr/var", "/"));
    EXPECT_TRUE(out.size() == 3);
    EXPECT_STREQ("", out[0].c_str());
    EXPECT_STREQ("usr", out[1].c_str());
    EXPECT_STREQ("var", out[2].c_str());

    EXPECT_EQ(4u, Split(out, "/usr/var/", "/"));
    EXPECT_TRUE(out.size() == 4);
    EXPECT_STREQ("", out[0].c_str());
    EXPECT_STREQ("usr", out[1].c_str());
    EXPECT_STREQ("var", out[2].c_str());
    EXPECT_STREQ("", out[3].c_str());

    EXPECT_EQ(2u, Split(out, "/usr/var/", "/", SplitFlags::RemoveEmptyEntries));
    EXPECT_TRUE(out.size() == 2);
    EXPECT_STREQ("usr", out[0].c_str());
    EXPECT_STREQ("var", out[1].c_str());

    EXPECT_EQ(2u, Split(out, "/usr//var//", "//", SplitFlags::RemoveEmptyEntries));
    EXPECT_TRUE(out.size() == 2);
    EXPECT_STREQ("/usr", out[0].c_str());
    EXPECT_STREQ("var", out[1].c_str());
}

TEST(StringUtils, Replace)
{
    EXPECT_EQ("aec", ReplaceAll("abc", "b", "e"));
    EXPECT_EQ("aeeceec", ReplaceAll("abbcbbc", "b", "e"));
    EXPECT_EQ("abbcbbc", ReplaceAll("abbcbbc", "", "e"));
    EXPECT_EQ("acc", ReplaceAll("abbcbbc", "b", ""));

    EXPECT_EQ("hello world", ReplaceAll("hello", "hello", "hello world"));
    EXPECT_EQ("hello worldabchello world", ReplaceAll("helloabchello", "hello", "hello world"));
}

TEST(StringUtils, Format)
{
    // 无格式化文本测试
    EXPECT_STREQ("", Format("").c_str());
    EXPECT_STREQ("test", Format("test").c_str());

    // 括号不完全匹配测试
    EXPECT_STREQ("{", Format("{").c_str());
    EXPECT_STREQ("{", Format("{{").c_str());
    EXPECT_STREQ("}", Format("}").c_str());
    EXPECT_STREQ("}", Format("}}").c_str());
    EXPECT_STREQ("}{", Format("}{").c_str());
    EXPECT_STREQ("{}", Format("{}").c_str());
    EXPECT_STREQ("{}{}", Format("{}{{}}").c_str());

    // 错误格式
    EXPECT_STREQ("{ 0}", Format("{ 0}", 0).c_str());
    EXPECT_STREQ("{ 00", Format("{ 0{0}", 0).c_str());
    EXPECT_STREQ("{hello}", Format("{hello}", 0).c_str());
    EXPECT_STREQ("{0 ,}", Format("{0 ,}", 0).c_str());
    EXPECT_STREQ("{0 , }", Format("{0 , }", 0).c_str());
    EXPECT_STREQ("{0,-:}", Format("{0,-:}", 0).c_str());
    EXPECT_STREQ("{0,- 1}", Format("{0,- 1}", 0).c_str());
    EXPECT_STREQ("{0,-1 [0]}", Format("{0,-1 [0]}", 0).c_str());
    EXPECT_STREQ("{0,-1[ 0]}", Format("{0,-1[ 0]}", 0).c_str());
    EXPECT_STREQ("{0,-1[0 ]}", Format("{0,-1[0 ]}", 0).c_str());

    // 占位符测试
    EXPECT_STREQ("       123", Format("{0 ,10}", "123").c_str());
    EXPECT_STREQ("123       ", Format("{0 , -10}", "123").c_str());
    EXPECT_STREQ("123", Format("{0,1}", "123").c_str());
    EXPECT_STREQ("123", Format("{0,-1}", "123").c_str());
    EXPECT_STREQ("00123", Format("{0,5[0]}", "123").c_str());
    EXPECT_STREQ("12300", Format("{0,-5[0]}", "123").c_str());

    // 布尔类型测试
    EXPECT_STREQ("true", Format("{0}", true).c_str());
    EXPECT_STREQ("false", Format("{0}", false).c_str());
    EXPECT_STREQ("真", Format("{0:假|真}", true).c_str());
    EXPECT_STREQ("假", Format("{0:假|真}", false).c_str());
    EXPECT_STREQ("", Format("{0:假|}", true).c_str());
    EXPECT_STREQ("", Format("{0:|真}", false).c_str());
    EXPECT_STREQ("{0:真假}", Format("{0:真假}", false).c_str());

    // 整数类型测试
    EXPECT_STREQ("123456", Format("{0}", 123456).c_str());
    EXPECT_STREQ("123456", Format("{0:}", 123456).c_str());
    EXPECT_STREQ("123456", Format("{0:D}", 123456).c_str());
    EXPECT_STREQ("FFFFFFFF", Format("{0:H}", 0xFFFFFFFFu).c_str());
    EXPECT_STREQ("ffffffff", Format("{0:h}", 0xFFFFFFFFu).c_str());
    EXPECT_STREQ("FF", Format("{0:H}", static_cast<int8_t>(-1)).c_str());
    EXPECT_STREQ("FFFF", Format("{0:H}", static_cast<int16_t>(-1)).c_str());
    EXPECT_STREQ("FFFFFFFF", Format("{0:H}", static_cast<int32_t>(-1)).c_str());
    EXPECT_STREQ("FFFFFFFFFFFFFFFF", Format("{0:H}", static_cast<int64_t>(-1)).c_str());
    EXPECT_STREQ("0", Format("{0:H}", 0x0u).c_str());
    EXPECT_STREQ("0", Format("{0:h}", 0x0u).c_str());

    // 特殊整数类型测试
    EXPECT_STREQ("123456", Format("{0}", static_cast<long>(123456)).c_str());
    EXPECT_STREQ("123456", Format("{0}", static_cast<unsigned long>(123456)).c_str());

    // 浮点数类型测试
    EXPECT_STREQ("123.456", Format("{0}", 123.456).c_str());
    EXPECT_STREQ("123.456", Format("{0:}", 123.456).c_str());
    EXPECT_STREQ("1.23456e+2", Format("{0:E}", 123.456).c_str());
    EXPECT_STREQ("{0:P}", Format("{0:P}", 123.456).c_str());
    EXPECT_STREQ("{0:F}", Format("{0:F}", 123.456).c_str());
    EXPECT_STREQ("{0:P }", Format("{0:P }", 123.456).c_str());
    EXPECT_STREQ("{0:F }", Format("{0:F }", 123.456).c_str());
    EXPECT_STREQ("1e+2", Format("{0:E0}", 123.456).c_str());
    EXPECT_STREQ("1.23456000000000003070e+2", Format("{0:E20}", 123.456).c_str());
    EXPECT_STREQ("1.23456000000000003070e+2", Format("{0:E21}", 123.456).c_str());
    EXPECT_STREQ("1e+2", Format("{0:P0}", 123.456).c_str());
    EXPECT_STREQ("1e+2", Format("{0:P1}", 123.456).c_str());
    EXPECT_STREQ("123.456000000000003070", Format("{0:P21}", 123.456).c_str());
    EXPECT_STREQ("123.456000000000003070", Format("{0:P22}", 123.456).c_str());
    EXPECT_STREQ("123", Format("{0:F0}", 123.345).c_str());
    EXPECT_STREQ("123.34499999999999886313", Format("{0:F20}", 123.345).c_str());
    EXPECT_STREQ("123.34499999999999886313", Format("{0:F21}", 123.345).c_str());

    // 字符串格式化测试
    EXPECT_STREQ("hello", Format("{0}", "hello").c_str());
    EXPECT_STREQ("hello", Format("{0}", string("hello")).c_str());
    EXPECT_STREQ("", Format("{0}", "").c_str());
    EXPECT_STREQ("", Format("{0}", string()).c_str());

    // 指针类型测试
    EXPECT_STREQ("null", Format("{0}", nullptr).c_str());
    EXPECT_STREQ("0x1234", Format("{0}", reinterpret_cast<void*>(0x1234)).c_str());

    // 自定义类型测试
    struct MyStructA
    {
        std::string ToString()const
        {
            return "MyStructA";
        }
    };

    struct MyStructB
    {
        std::string ToString(const ArrayView<char>&)const
        {
            return "MyStructB";
        }
    };

    struct MyStructC
    {
        std::string ToString()const
        {
            return "MyStructC";
        }

        std::string ToString(const ArrayView<char>&)const
        {
            return "MyStructC_Ex";
        }
    };

    EXPECT_STREQ("MyStructA", Format("{0}", MyStructA()).c_str());
    EXPECT_STREQ("MyStructB", Format("{0}", MyStructB()).c_str());
    EXPECT_STREQ("MyStructC_Ex", Format("{0}", MyStructC()).c_str());
}

TEST(StringUtils, WstringFormat)
{
    // 无格式化文本测试
    EXPECT_STREQ(L"", Format(L"").c_str());
    EXPECT_STREQ(L"test", Format(L"test").c_str());

    // 括号不完全匹配测试
    EXPECT_STREQ(L"{", Format(L"{").c_str());
    EXPECT_STREQ(L"{", Format(L"{{").c_str());
    EXPECT_STREQ(L"}", Format(L"}").c_str());
    EXPECT_STREQ(L"}", Format(L"}}").c_str());
    EXPECT_STREQ(L"}{", Format(L"}{").c_str());
    EXPECT_STREQ(L"{}", Format(L"{}").c_str());
    EXPECT_STREQ(L"{}{}", Format(L"{}{{}}").c_str());

    // 错误格式
    EXPECT_STREQ(L"{ 0}", Format(L"{ 0}", 0).c_str());
    EXPECT_STREQ(L"{ 00", Format(L"{ 0{0}", 0).c_str());
    EXPECT_STREQ(L"{hello}", Format(L"{hello}", 0).c_str());
    EXPECT_STREQ(L"{0 ,}", Format(L"{0 ,}", 0).c_str());
    EXPECT_STREQ(L"{0 , }", Format(L"{0 , }", 0).c_str());
    EXPECT_STREQ(L"{0,-:}", Format(L"{0,-:}", 0).c_str());
    EXPECT_STREQ(L"{0,- 1}", Format(L"{0,- 1}", 0).c_str());
    EXPECT_STREQ(L"{0,-1 [0]}", Format(L"{0,-1 [0]}", 0).c_str());
    EXPECT_STREQ(L"{0,-1[ 0]}", Format(L"{0,-1[ 0]}", 0).c_str());
    EXPECT_STREQ(L"{0,-1[0 ]}", Format(L"{0,-1[0 ]}", 0).c_str());

    // 占位符测试
    EXPECT_STREQ(L"       123", Format(L"{0 ,10}", L"123").c_str());
    EXPECT_STREQ(L"123       ", Format(L"{0 , -10}", L"123").c_str());
    EXPECT_STREQ(L"123", Format(L"{0,1}", L"123").c_str());
    EXPECT_STREQ(L"123", Format(L"{0,-1}", L"123").c_str());
    EXPECT_STREQ(L"00123", Format(L"{0,5[0]}", L"123").c_str());
    EXPECT_STREQ(L"12300", Format(L"{0,-5[0]}", L"123").c_str());

    // 布尔类型测试
    EXPECT_STREQ(L"true", Format(L"{0}", true).c_str());
    EXPECT_STREQ(L"false", Format(L"{0}", false).c_str());
    EXPECT_STREQ(L"真", Format(L"{0:假|真}", true).c_str());
    EXPECT_STREQ(L"假", Format(L"{0:假|真}", false).c_str());
    EXPECT_STREQ(L"", Format(L"{0:假|}", true).c_str());
    EXPECT_STREQ(L"", Format(L"{0:|真}", false).c_str());
    EXPECT_STREQ(L"{0:真假}", Format(L"{0:真假}", false).c_str());

    // 整数类型测试
    EXPECT_STREQ(L"123456", Format(L"{0}", 123456).c_str());
    EXPECT_STREQ(L"123456", Format(L"{0:}", 123456).c_str());
    EXPECT_STREQ(L"123456", Format(L"{0:D}", 123456).c_str());
    EXPECT_STREQ(L"FFFFFFFF", Format(L"{0:H}", 0xFFFFFFFFu).c_str());
    EXPECT_STREQ(L"ffffffff", Format(L"{0:h}", 0xFFFFFFFFu).c_str());
    EXPECT_STREQ(L"FF", Format(L"{0:H}", static_cast<int8_t>(-1)).c_str());
    EXPECT_STREQ(L"FFFF", Format(L"{0:H}", static_cast<int16_t>(-1)).c_str());
    EXPECT_STREQ(L"FFFFFFFF", Format(L"{0:H}", static_cast<int32_t>(-1)).c_str());
    EXPECT_STREQ(L"FFFFFFFFFFFFFFFF", Format(L"{0:H}", static_cast<int64_t>(-1)).c_str());
    EXPECT_STREQ(L"0", Format(L"{0:H}", 0x0u).c_str());
    EXPECT_STREQ(L"0", Format(L"{0:h}", 0x0u).c_str());

    // 特殊整数类型测试
    EXPECT_STREQ(L"123456", Format(L"{0}", static_cast<long>(123456)).c_str());
    EXPECT_STREQ(L"123456", Format(L"{0}", static_cast<unsigned long>(123456)).c_str());

    // 浮点数类型测试
    EXPECT_STREQ(L"123.456", Format(L"{0}", 123.456).c_str());
    EXPECT_STREQ(L"123.456", Format(L"{0:}", 123.456).c_str());
    EXPECT_STREQ(L"1.23456e+2", Format(L"{0:E}", 123.456).c_str());
    EXPECT_STREQ(L"{0:P}", Format(L"{0:P}", 123.456).c_str());
    EXPECT_STREQ(L"{0:F}", Format(L"{0:F}", 123.456).c_str());
    EXPECT_STREQ(L"{0:P }", Format(L"{0:P }", 123.456).c_str());
    EXPECT_STREQ(L"{0:F }", Format(L"{0:F }", 123.456).c_str());
    EXPECT_STREQ(L"1e+2", Format(L"{0:E0}", 123.456).c_str());
    EXPECT_STREQ(L"1.23456000000000003070e+2", Format(L"{0:E20}", 123.456).c_str());
    EXPECT_STREQ(L"1.23456000000000003070e+2", Format(L"{0:E21}", 123.456).c_str());
    EXPECT_STREQ(L"1e+2", Format(L"{0:P0}", 123.456).c_str());
    EXPECT_STREQ(L"1e+2", Format(L"{0:P1}", 123.456).c_str());
    EXPECT_STREQ(L"123.456000000000003070", Format(L"{0:P21}", 123.456).c_str());
    EXPECT_STREQ(L"123.456000000000003070", Format(L"{0:P22}", 123.456).c_str());
    EXPECT_STREQ(L"123", Format(L"{0:F0}", 123.345).c_str());
    EXPECT_STREQ(L"123.34499999999999886313", Format(L"{0:F20}", 123.345).c_str());
    EXPECT_STREQ(L"123.34499999999999886313", Format(L"{0:F21}", 123.345).c_str());

    // 字符串格式化测试
    EXPECT_STREQ(L"hello", Format(L"{0}", L"hello").c_str());
    EXPECT_STREQ(L"hello", Format(L"{0}", wstring(L"hello")).c_str());
    EXPECT_STREQ(L"", Format(L"{0}", L"").c_str());
    EXPECT_STREQ(L"", Format(L"{0}", wstring()).c_str());

    // 指针类型测试
    EXPECT_STREQ(L"null", Format(L"{0}", nullptr).c_str());
    EXPECT_STREQ(L"0x1234", Format(L"{0}", reinterpret_cast<void*>(0x1234)).c_str());

    // 自定义类型测试
    struct MyStructA
    {
        std::wstring ToString()const
        {
            return L"MyStructA";
        }
    };

    struct MyStructB
    {
        std::wstring ToString(const ArrayView<wchar_t>&)const
        {
            return L"MyStructB";
        }
    };

    struct MyStructC
    {
        std::wstring ToString()const
        {
            return L"MyStructC";
        }

        std::wstring ToString(const ArrayView<wchar_t>&)const
        {
            return L"MyStructC_Ex";
        }
    };

    EXPECT_STREQ(L"MyStructA", Format(L"{0}", MyStructA()).c_str());
    EXPECT_STREQ(L"MyStructB", Format(L"{0}", MyStructB()).c_str());
    EXPECT_STREQ(L"MyStructC_Ex", Format(L"{0}", MyStructC()).c_str());
}
