/**
 * @file
 * @date 2017/10/9
 */
#include <gtest/gtest.h>

#include <cmath>

#include <Moe.Core/Json.hpp>
#include <Moe.Core/Parser.hpp>

using namespace std;
using namespace moe;

// See http://seriot.ch/parsing_json.php
TEST(Json, Parse5)
{
    // 标量
    EXPECT_EQ(Json5::Parse("123"), 123.);
    EXPECT_EQ(Json5::Parse("\"asd\""), "asd");

    // 尾随逗号
    EXPECT_TRUE(Json5::Parse("{\"id\":0,}").Is<JsonValue::ObjectType>());
    EXPECT_TRUE(Json5::Parse("[0,]").Is<JsonValue::ArrayType>());

    // 注释
    EXPECT_TRUE(Json5::Parse("[\"a/*b*/c/*d//e\"]").Is<JsonValue::ArrayType>());
    EXPECT_TRUE(Json5::Parse("{\"a\":\"b\"}/**/").Is<JsonValue::ObjectType>());
    EXPECT_TRUE(Json5::Parse("{\"a\":/*comment*/\"b\"}").Is<JsonValue::ObjectType>());

    // 未闭合标记
    EXPECT_THROW(Json5::Parse("{{}"), LexicalException);
    EXPECT_THROW(Json5::Parse("[[]"), LexicalException);
    EXPECT_THROW(Json5::Parse("[[]]]"), LexicalException);
    EXPECT_THROW(Json5::Parse("{\"\":"), LexicalException);
    EXPECT_THROW(Json5::Parse("{}}"), LexicalException);
    EXPECT_THROW(Json5::Parse("/*"), LexicalException);

    // 数字
    EXPECT_TRUE(std::isnan(Json5::Parse("NaN").Get<double>()));
    EXPECT_TRUE(std::isinf(Json5::Parse("-Infinity").Get<double>()));
    EXPECT_EQ(Json5::Parse("0x42").Get<double>(), 0x42);
    EXPECT_EQ(Json5::Parse("0x4F").Get<double>(), 0x4F);
    EXPECT_EQ(Json5::Parse("0xA").Get<double>(), 0xA);
    EXPECT_EQ(Json5::Parse("0xa").Get<double>(), 0xa);
    EXPECT_EQ(Json5::Parse("0E+").Get<double>(), 0);
    EXPECT_EQ(Json5::Parse(".2e-3").Get<double>(), 0.2e-3);
    EXPECT_EQ(Json5::Parse("123.456e-7").Get<double>(), 123.456e-7);
    EXPECT_EQ(Json5::Parse("0E0").Get<double>(), 0);
    EXPECT_EQ(Json5::Parse("0e+1").Get<double>(), 0);
    EXPECT_THROW(Json5::Parse("1eE2"), LexicalException);

    // 数组
    EXPECT_TRUE(Json5::Parse("[[],[[]]]").Is<JsonValue::ArrayType>());
    EXPECT_THROW(Json5::Parse("[,1]"), LexicalException);
    EXPECT_THROW(Json5::Parse("[\"\": 1]"), LexicalException);
    EXPECT_THROW(Json5::Parse("[1,0A10A,1\n"), LexicalException);

    // 字典
    EXPECT_TRUE(Json5::Parse("{\"\":0}").Is<JsonValue::ObjectType>());
    EXPECT_TRUE(Json5::Parse("{key: 'value'}").Is<JsonValue::ObjectType>());
    EXPECT_THROW(Json5::Parse("{\"a\":\"b\",\"a\":\"b\"}"), ObjectExistsException);
    EXPECT_THROW(Json5::Parse("{:\"b\"}"), LexicalException);
    EXPECT_THROW(Json5::Parse("{1:1}"), LexicalException);

    // 字符串
    EXPECT_TRUE(Json5::Parse("\"\\\"\\\\/\\b\\f\\n\\r\\t\"").Is<JsonValue::StringType>());
    EXPECT_TRUE(Json5::Parse("\"\x7F\"").Is<JsonValue::StringType>());
    EXPECT_TRUE(Json5::Parse("\"\\u0000\"").Is<JsonValue::StringType>());
    EXPECT_THROW(Json5::Parse("\"\\"), LexicalException);
    EXPECT_THROW(Json5::Parse("\"a\010a\""), LexicalException);
    EXPECT_THROW(Json5::Parse("\"\\uqqqq\""), LexicalException);
    EXPECT_THROW(Json5::Parse("\"\\u00A\""), LexicalException);
}
