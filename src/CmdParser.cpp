/**
 * @file
 * @date 2017/12/17
 */
#include <Moe.Core/CmdParser.hpp>
#include <Moe.Core/Exception.hpp>

using namespace std;
using namespace moe;

CmdParser& CmdParser::operator<<(Option&& opt)
{
    if (!opt.Valid())
        MOE_THROW(BadArgumentException, "Invalid option");
    if (m_stOptionTable.find(opt.LongOption) != m_stOptionTable.end())
        MOE_THROW(ObjectExistsException, "Option \"{0}\" exists", opt.LongOption);
    if (m_stShortOptTable.find(opt.ShortOption) != m_stShortOptTable.end())
        MOE_THROW(ObjectExistsException, "Short option '{0}' exists", opt.ShortOption);

    const char* longOption = opt.LongOption;
    char shortOpt = opt.ShortOption;
    m_stOptions.emplace_back(opt);

    size_t index = m_stOptions.size() - 1;
    m_stOptionTable.emplace(longOption, index);
    m_stShortOptTable.emplace(shortOpt, index);
    return *this;
}

void CmdParser::Clear()noexcept
{
    m_stOptions.clear();
    m_stOptionTable.clear();
    m_stShortOptTable.clear();
}

std::string CmdParser::BuildUsageText(const char* name, const char* nonOptionsHint)const
{
    // Usage: $name --file <...> [OPTIONS] ...
    string ret;
    ret.reserve(64);
    ret.append("Usage: ");
    ret.append(name);
    for (const auto& it : m_stOptions)
    {
        if (!it.Required)
            continue;

        if (it.ShortOption != '\0')
        {
            ret.append(" -");
            ret.push_back(it.ShortOption);
            ret.append(" <...>");
        }
        else
        {
            ret.append(" --");
            ret.append(it.LongOption);
            ret.append(" <...>");
        }
    }
    ret.append(" [OPTIONS]");
    if (nonOptionsHint && ::strlen(nonOptionsHint) > 0)
    {
        ret.push_back(' ');
        ret.append(nonOptionsHint);
    }
    return ret;
}

std::string CmdParser::BuildOptionsText(uint32_t leftPadding, uint32_t centerMargin)const
{
    static const char kOneArgumentText[] = "<...>";
    static const char kManyArgumentText[] = "<...> ...";

    string ret;
    size_t textSize = 0;

    // 计算选项占用的字节数
    for (const auto& it : m_stOptions)
    {
        size_t s = 2 + ::strlen(it.LongOption);  // "--xx"
        if (it.ShortOption != '\0')
            s += 4;  // ", -x"
        if (it.ArgumentCount < 0)
            s += 1 + sizeof(kManyArgumentText) - 1;  // " <...> ..."
        else if (it.ArgumentCount == 1)
            s += 1 + sizeof(kOneArgumentText) - 1;  // " <...>"
        textSize = std::max(textSize, s);
    }
    textSize += leftPadding + centerMargin;

    // 构造结果
    ret.reserve(128);
    string leftPaddingText(leftPadding, ' ');
    string centerMarginText(centerMargin, ' ');
    for (const auto& it : m_stOptions)
    {
        if (!ret.empty())
            ret.push_back('\n');

        auto pos = ret.length();
        ret.append(leftPaddingText);
        ret.append("--");
        ret.append(it.LongOption);
        if (it.ShortOption != '\0')
        {
            ret.append(", -");
            ret.push_back(it.ShortOption);
        }
        if (it.ArgumentCount < 0)
        {
            ret.push_back(' ');
            ret.append(kManyArgumentText);
        }
        else if (it.ArgumentCount == 1)
        {
            ret.push_back(' ');
            ret.append(kOneArgumentText);
        }
        ret.append(centerMarginText);

        auto s = ret.length() - pos;
        while (s < textSize)
        {
            ret.push_back(' ');
            s = ret.length() - pos;
        }
        ret.append(it.Description);
    }
    return ret;
}

size_t CmdParser::Parse(uint32_t argc, const char* argv[], std::vector<std::string>* nonOptions)
{
    for (auto& i : m_stOptions)
        i.Set = false;

    if (argc < 1)
        MOE_THROW(BadArgumentException, "Commandline arguments cannot be empty");

    int state = 0;
    Option* option = nullptr;
    for (uint32_t i = 1; i < argc; ++i)
    {
        const char* start = argv[i];

        if (state == 30)
        {
            if (nonOptions)
                nonOptions->emplace_back(start);
            continue;
        }

        auto len = ::strlen(start);
        if (len <= 0)
            continue;
        const char* end = start + len;

        if (state == 20)
        {
            if ((*start) == '-')
            {
                assert(option);
                option->Set = true;
                option = nullptr;
                state = 0;
            }
        }

        const char* argStart = nullptr;
        for (const char* current = start; current <= end; ++current)
        {
            char c = *current;
            switch (state)
            {
                case 0:
                    if (c == '-')
                        state = 1;  // -
                    else
                    {
                        if (nonOptions)
                        {
                            assert(current != end);
                            nonOptions->emplace_back(current);
                        }
                        current = end;
                    }
                    break;
                case 1:
                    if (c == '-')
                    {
                        state = 2;  // --
                        argStart = current + 1;
                    }
                    else if (c == '\0')
                        MOE_THROW(BadFormatException, "Option expected");
                    else
                    {
                        auto it = m_stShortOptTable.find(c);
                        if (it == m_stShortOptTable.end())
                            MOE_THROW(BadFormatException, "Unknown option '{0}'", c);
                        option = &m_stOptions[it->second];

                        // 读取掉末尾的其他字符
                        while ((c = (*(++current))) != '\0')
                        {
                            if (!StringUtils::IsWhitespace(c))
                                MOE_THROW(BadFormatException, "Unexpected character '{0}'", c);
                        }

                        // 通知Option
                        assert(current == end);
                        assert(option->OnStart);
                        switch (option->OnStart(option->Target, option->DefaultValue))
                        {
                            case OptionReadResult::Terminated:
                                option->Set = true;
                                option = nullptr;
                                state = 0;
                                break;
                            case OptionReadResult::ParseError:
                                MOE_THROW(BadFormatException, "Option '{0}' parse error", option->ShortOption);
                            case OptionReadResult::NeedMore:
                                state = 10;
                                break;
                            case OptionReadResult::MoreOrEmpty:
                                state = 20;
                                break;
                            default:
                                assert(false);
                                break;
                        }
                    }
                    break;
                case 2:
                    if (c == '=' || c == '\0')
                    {
                        if (current == argStart)
                        {
                            if (c == '=')
                                MOE_THROW(BadFormatException, "Option expected");

                            // "--"
                            state = 30;
                            break;
                        }

                        auto opt = string(argStart, current - argStart);
                        auto it = m_stOptionTable.find(opt);
                        if (it == m_stOptionTable.end())
                            MOE_THROW(BadFormatException, "Unknown option \"{0}\"", opt);
                        option = &m_stOptions[it->second];

                        // 通知Option
                        assert(option->OnStart);
                        switch (option->OnStart(option->Target, option->DefaultValue))
                        {
                            case OptionReadResult::Terminated:
                                option->Set = true;
                                option = nullptr;
                                state = 0;

                                // 读取掉末尾的其他字符
                                while ((c = (*(++current))) != '\0')
                                {
                                    if (!StringUtils::IsWhitespace(c))
                                        MOE_THROW(BadFormatException, "Unexpected character '{0}'", c);
                                }
                                assert(current == end);
                                break;
                            case OptionReadResult::ParseError:
                                MOE_THROW(BadFormatException, "Option \"{0}\" parse error", option->LongOption);
                            case OptionReadResult::NeedMore:
                                state = 10;
                                break;
                            case OptionReadResult::MoreOrEmpty:
                                state = 20;
                                break;
                            default:
                                assert(false);
                                break;
                        }
                    }
                    break;
                case 10:
                case 20:
                    if (current == end)
                        break;

                    assert(option);
                    assert(option->OnReadArg);
                    switch (option->OnReadArg(option->Target, ArrayView<char>(current, end - current)))
                    {
                        case OptionReadResult::Terminated:
                            option->Set = true;
                            option = nullptr;
                            state = 0;
                            break;
                        case OptionReadResult::ParseError:
                            MOE_THROW(BadFormatException, "Option \"{0}\" parse error", option->LongOption);
                        case OptionReadResult::NeedMore:
                            state = 10;
                            break;
                        case OptionReadResult::MoreOrEmpty:
                            state = 20;
                            break;
                        default:
                            assert(false);
                            break;
                    }
                    break;
                default:
                    assert(false);
                    break;
            }
        }

        if (state > 0 && state < 10)
            MOE_THROW(BadFormatException, "Bad input");
    }

    if (state == 10)
    {
        assert(option);
        MOE_THROW(BadFormatException, "Value expected for option \"{0}\"", option->LongOption);
    }

    size_t cnt = 0;
    for (auto& i : m_stOptions)
    {
        // 检查是否所有必须项被填写
        if (i.Required && !i.Set)
            MOE_THROW(BadFormatException, "Option \"{0}\" must be set", i.LongOption);

        // 设置未填写的参数为默认值
        if (!i.Set)
        {
            assert(i.OnSetDefault);
            i.OnSetDefault(i.Target, i.DefaultValue);
        }
    }
    return cnt;
}
