/**
 * @file
 * @date 2017/11/27
 */
#include <gtest/gtest.h>

#include <Moe.Core/Hasher.hpp>
#include <Moe.Core/StringUtils.hpp>

using namespace std;
using namespace moe;

TEST(Hasher, MD5)
{
    uint8_t buffer[16];

    EXPECT_STREQ("d41d8cd98f00b204e9800998ecf8427e", StringUtils::BufferToHexLower(Hasher::MD5(buffer, "")).c_str());
    EXPECT_STREQ("202cb962ac59075b964b07152d234b70", StringUtils::BufferToHexLower(Hasher::MD5(buffer, "123")).c_str());
    EXPECT_STREQ("5eb63bbbe01eeed093cb22bb8f5acdc3",
        StringUtils::BufferToHexLower(Hasher::MD5(buffer, "hello world")).c_str());
}
