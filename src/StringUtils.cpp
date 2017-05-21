/**
 * @file
 * @date 2017/5/20
 */
#include <Moe.Core/StringUtils.hpp>

using namespace std;
using namespace moe;

bool StringUtils::IsUnicodeWhitespace(char32_t c)
{
    static const char32_t kWhitespaceTable7[] = { 32, 13, 10, 9, 11, 12 };
    static const char32_t kWhitespaceTable16[] = {
        160, 8232, 8233, 5760,
        6158, 8192, 8193, 8194,
        8195, 8196, 8197, 8198,
        8199, 8200, 8201, 8202,
        8239, 8287, 12288, 65279
    };

    if (c < 128)
    {
        for (size_t i = 0; i < CountOf(kWhitespaceTable7); ++i)
        {
            if (kWhitespaceTable7[i] == c)
                return true;
        }
    }
    else
    {
        for (size_t i = 0; i < CountOf(kWhitespaceTable16); ++i)
        {
            if (kWhitespaceTable16[i] == c)
                return true;
        }
    }

    return false;
}
