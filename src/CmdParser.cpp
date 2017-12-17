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

ssize_t CmdParser::Parse(uint32_t argc, const char* argv[], std::vector<std::string>* nonOptions)
{
    if (argc < 1)
        MOE_THROW(BadArgumentException, "Commandline arguments cannot be empty");

    for (uint32_t i = 1; i < argc; ++i)
    {
        const char* current = argv[i];
        auto len = ::strlen(current);
        if (len <= 0)
            continue;

        // TODO
    }

    return -1;
}
