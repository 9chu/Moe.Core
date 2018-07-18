/**
 * @file
 * @date 2017/11/27
 */
#include <gtest/gtest.h>

#include <Moe.Core/Hasher.hpp>
#include <Moe.Core/StringUtils.hpp>

using namespace std;
using namespace moe;

TEST(Hasher, Md5)
{
    array<uint8_t, 16> buffer;

    EXPECT_STREQ("d41d8cd98f00b204e9800998ecf8427e", StringUtils::BufferToHexLower(
        ComputeHash<Hasher::Md5>(buffer, StringToBytesView(""))).c_str());
    EXPECT_STREQ("202cb962ac59075b964b07152d234b70", StringUtils::BufferToHexLower(
        ComputeHash<Hasher::Md5>(buffer, StringToBytesView("123"))).c_str());
    EXPECT_STREQ("5eb63bbbe01eeed093cb22bb8f5acdc3", StringUtils::BufferToHexLower(
        ComputeHash<Hasher::Md5>(buffer, StringToBytesView("hello world"))).c_str());
}

TEST(Hasher, Sha1)
{
    Hasher::Sha1::ResultType buffer;

    EXPECT_STREQ("356a192b7913b04c54574d18c28d46e6395428ab", StringUtils::BufferToHexLower(
        ComputeHash<Hasher::Sha1>(buffer, StringToBytesView("1"))).c_str());
    EXPECT_STREQ("5eda471047663e1597b03ae09d03d98b20affabe", StringUtils::BufferToHexLower(
        ComputeHash<Hasher::Sha1>(buffer, StringToBytesView("1234567890987654321"))).c_str());
}

TEST(Hasher, Sha256)
{
    Hasher::Sha256::ResultType buffer;

    EXPECT_STREQ("6b86b273ff34fce19d6b804eff5a3f5747ada4eaa22f1d49c01e52ddb7875b4b", StringUtils::BufferToHexLower(
        ComputeHash<Hasher::Sha256>(buffer, StringToBytesView("1"))).c_str());
    EXPECT_STREQ("50bfee49c706d766411777aac1c9f35456c33ecea2afb4f3c8f1033b0298bdc9", StringUtils::BufferToHexLower(
        ComputeHash<Hasher::Sha256>(buffer, StringToBytesView("1234567890987654321"))).c_str());
}
