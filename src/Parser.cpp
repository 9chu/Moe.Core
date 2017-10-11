/**
 * @file
 * @date 2017/10/9
 */
#include <Moe.Core/Parser.hpp>

using namespace std;
using namespace moe;

std::string Parser::PrintChar(char ch)
{
    if (ch == '\0')
        return "<EOF>";
    else if (ch == '\'')
        return "'\''";
    else if (ch > 0 && ::isprint(ch) != 0)
        return StringUtils::Format("'{0}'", ch);
    return StringUtils::Format("<{0}>", static_cast<uint8_t>(ch));
}

void Parser::Run(TextReader& reader)
{
    m_pReader = Optional<TextReader&>(reader);
    c = m_pReader->Peek();
}
