/**
 * @file
 * @date 2017/5/20
 */
#include <Moe.Core/StringUtils.hpp>
#include <Moe.Core/Unicode.hpp>

using namespace std;
using namespace moe;

template <>
std::basic_string<char32_t>& StringUtils::TrimLeftInPlace<char32_t>(std::basic_string<char32_t>& str)
{
    if (str.empty())
        return str;

    auto it = str.begin();
    while (it != str.end())
    {
        if (!Unicode::IsWhitespace(*it))
            break;
        ++it;
    }

    str.erase(str.begin(), it);
    return str;
}

template <>
std::basic_string<char32_t>& StringUtils::TrimRightInPlace<char32_t>(std::basic_string<char32_t>& str)
{
    if (str.empty())
        return str;

    auto it = str.end();
    do
    {
        if (!Unicode::IsWhitespace(*(it - 1)))
            break;
        --it;
    } while (it != str.begin());

    str.erase(it, str.end());
    return str;
}
