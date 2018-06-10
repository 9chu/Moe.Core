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

std::string StringUtils::Repr(ArrayView<char> input)
{
    string ret;
    ret.reserve(input.GetSize() + 2);
    ret.push_back('"');
    for (size_t i = 0; i < input.GetSize(); ++i)
    {
        auto cp = static_cast<uint8_t>(input[i]);
        switch (cp)
        {
            case '\0':
                ret.append("\\0");
                break;
            case '\\':
                ret.append("\\\\");
                break;
            case '"':
                ret.append("\\\"");
                break;
            case '\n':
                ret.append("\\\n");
                break;
            case '\r':
                ret.append("\\\r");
                break;
            case '\t':
                ret.append("\\\t");
                break;
            case '\f':
                ret.append("\\\f");
                break;
            case '\v':
                ret.append("\\\v");
                break;
            case '\a':
                ret.append("\\\a");
                break;
            case '\b':
                ret.append("\\\b");
                break;
            default:
                if (cp < 128 && ::isprint(cp))
                    ret.push_back(static_cast<char>(cp));
                else
                {
                    char hex[16] = { 0 };
                    size_t len = Convert::ToHexString(cp, hex);
                    switch (len)
                    {
                        case 0:
                            assert(false);
                            ret.append("\\x00");
                            break;
                        case 1:
                            ret.append("\\x0");
                            break;
                        default:
                            ret.append("\\x");
                            break;
                    }
                    ret.append(hex, len);
                }
                break;
        }
    }
    ret.push_back('"');
    return ret;
}

std::string StringUtils::Repr(ArrayView<char32_t> input)
{
    string ret;
    ret.reserve(input.GetSize() + 2);
    ret.push_back('"');
    for (size_t i = 0; i < input.GetSize(); ++i)
    {
        auto cp = static_cast<uint32_t>(input[i]);
        switch (cp)
        {
            case '\0':
                ret.append("\\0");
                break;
            case '\\':
                ret.append("\\\\");
                break;
            case '"':
                ret.append("\\\"");
                break;
            case '\n':
                ret.append("\\\n");
                break;
            case '\r':
                ret.append("\\\r");
                break;
            case '\t':
                ret.append("\\\t");
                break;
            case '\f':
                ret.append("\\\f");
                break;
            case '\v':
                ret.append("\\\v");
                break;
            case '\a':
                ret.append("\\\a");
                break;
            case '\b':
                ret.append("\\\b");
                break;
            default:
                if (cp < 128 && ::isprint(cp))
                    ret.push_back(static_cast<char>(cp));
                else
                {
                    char hex[16] = { 0 };
                    size_t len = Convert::ToHexString(cp, hex);
                    switch (len)
                    {
                        case 0:
                            assert(false);
                            ret.append("\\u0000");
                            break;
                        case 1:
                            ret.append("\\u000");
                            break;
                        case 2:
                            ret.append("\\u00");
                            break;
                        case 3:
                            ret.append("\\u0");
                            break;
                        default:
                            ret.append("\\u");
                            break;
                    }
                    ret.append(hex, len);
                }
                break;
        }
    }
    ret.push_back('"');
    return ret;
}
