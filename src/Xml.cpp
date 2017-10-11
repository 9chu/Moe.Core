/**
 * @file
 * @date 2017/10/9
 */
#include <Moe.Core/Xml.hpp>
#include <Moe.Core/Parser.hpp>

using namespace std;
using namespace moe;

//////////////////////////////////////////////////////////////////////////////// Xml

namespace
{
    bool IsXmlBlankCharacter(char ch)
    {
        return ch == 0x20 || ch == 0x09 || ch == 0x0D || ch == 0x0A;
    }

    static bool IsXmlNamePrefix(char ch)
    {
        if (ch >= 'a' && ch <= 'z')
            return true;
        else if (ch >= 'A' && ch <= 'Z')
            return true;
        else if (ch == '_' || ch == ':')
            return true;
        else if (ch < 0)
            return true;
        return false;
    }

    static bool IsXmlNameLetter(char ch)
    {
        if (ch >= 'a' && ch <= 'z')
            return true;
        else if (ch >= 'A' && ch <= 'Z')
            return true;
        else if (ch >= '0' && ch <= '9')
            return true;
        else if (ch == '_' || ch == ':' || ch == '.' || ch == '-')
            return true;
        else if (ch < 0)
            return true;
        return false;
    }

    class XmlParser :
        public Parser
    {
    public:
        XmlParser(XmlSaxHandler* handler)
            : m_pHandler(handler) {}

    public:
        void Run(TextReader& reader)override
        {
            Parser::Run(reader);

            // 初始化内部状态
            m_stStringBuffer.clear();
            m_stKeyBuffer.clear();
            m_stValueBuffer.clear();

            // 开始解析
            CheckDeclaration();
            SkipIgnorable();
            ParseElement();

            // 检查是否读取完毕
            SkipIgnorable();
            if (c != '\0')
                ThrowError("Bad tailing character {0}", PrintChar(c));
        }

    private:
        void SkipIgnorable()
        {
            while (c != '\0')
            {
                if (IsXmlBlankCharacter(c))
                    Next();
                else
                    break;
            }
        }

        char ParseEntityRef()
        {
            char result = '\0';

            Accept('&');
            if (TryAcceptOne('a'))
            {
                if (TryAcceptOne('m'))
                {
                    Accept('p');
                    result = '&';
                }
                else
                {
                    Accept('p');
                    Accept('o');
                    Accept('s');
                    result = '\'';
                }
            }
            else if (TryAcceptOne('l'))
            {
                Accept('t');
                result = '<';
            }
            else if (TryAcceptOne('g'))
            {
                Accept('t');
                result = '>';
            }
            else
            {
                Accept('q');
                Accept('u');
                Accept('o');
                Accept('t');
                result = '"';
            }
            Accept(';');
            return result;
        }

        void ParseName(string& buffer)
        {
            buffer.clear();

            if (IsXmlNamePrefix(c))
            {
                while (IsXmlNameLetter(c))
                    buffer.push_back(Next());
            }
        }

        bool ParseAttribute(string& key, string& val)
        {
            val.clear();

            SkipIgnorable();

            ParseName(key);
            if (key.empty())
                return false;

            Accept('=');

            char delim = Accept('\'', '"');
            while (c != '\0' && c != '<' && c != '"')
            {
                if (c == '&')
                    val.push_back(ParseEntityRef());
                else
                    val.push_back(Next());
            }

            Accept(delim);
            return true;
        }

        void ParseContent(string& buffer)
        {
            buffer.clear();

            while (true)
            {
                if (c == '\0')
                    ThrowError("Unexpected {0}", PrintChar('\0'));
                else if (c == '<')
                {
                    Next();

                    if (TryAcceptOne('!'))
                    {
                        if (TryAcceptOne('['))
                        {
                            Accept('C');
                            Accept('D');
                            Accept('A');
                            Accept('T');
                            Accept('A');
                            Accept('[');

                            // 解析CDATA
                            int state = 0;
                            while (state >= 0)
                            {
                                char ch = Next();
                                if (ch == '\0')
                                    ThrowError("Unexpected {0}", PrintChar('\0'));

                                if (state == 0)
                                {
                                    if (ch == ']')
                                    {
                                        state = 1;
                                        continue;
                                    }
                                }
                                else if (state == 1)
                                {
                                    if (ch == ']')
                                    {
                                        state = 2;
                                        continue;
                                    }
                                    else
                                    {
                                        buffer.push_back(']');
                                        state = 0;
                                    }
                                }
                                else if (state == 2)
                                {
                                    if (ch == '>')
                                    {
                                        state = -1;
                                        continue;
                                    }
                                    else
                                    {
                                        buffer.append("]]");
                                        state = 0;
                                    }
                                }

                                buffer.push_back(ch);
                            }
                        }
                        else
                        {
                            Accept('-');
                            Accept('-');

                            // 解析注释
                            int state = 0;
                            while (state >= 0)
                            {
                                char ch = Next();
                                if (ch == '\0')
                                    ThrowError("Unexpected {0}", PrintChar('\0'));

                                if (state == 0)
                                {
                                    if (ch == '-')
                                    {
                                        state = 1;
                                        continue;
                                    }
                                }
                                else if (state == 1)
                                {
                                    if (ch == '-')
                                    {
                                        state = 2;
                                        continue;
                                    }
                                    else
                                    {
                                        buffer.push_back('-');
                                        state = 0;
                                    }
                                }
                                else if (state == 2)
                                {
                                    if (ch == '>')
                                    {
                                        state = -1;
                                        continue;
                                    }
                                    else
                                    {
                                        buffer.append("--");
                                        state = 0;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        Back();
                        break;
                    }
                }
                else if (c == '&')
                    buffer.push_back(ParseEntityRef());
                else
                    buffer.push_back(Next());
            }
        }

        void ParseElement()
        {
            Accept('<');

            // 检查节点名称
            ParseName(m_stStringBuffer);  // 拷贝名字, 以便检查标签闭合情况
            if (m_stStringBuffer.size() >= 3 && (m_stStringBuffer[0] == 'x' || m_stStringBuffer[0] == 'X') &&
                (m_stStringBuffer[1] == 'm' || m_stStringBuffer[1] == 'M') &&
                (m_stStringBuffer[2] == 'l' || m_stStringBuffer[2] == 'L'))
            {
                ThrowError("Reserved name {0} found", m_stStringBuffer);
            }

            m_pHandler->OnXmlElementBegin(m_stStringBuffer);

            // 读取属性
            while (ParseAttribute(m_stKeyBuffer, m_stValueBuffer))
                m_pHandler->OnXmlAttribute(m_stKeyBuffer, m_stValueBuffer);

            // 闭合标签
            bool fullyClosed = false;
            if (TryAcceptOne('/'))
            {
                Accept('>');
                fullyClosed = true;
            }
            else
                Accept('>');

            if (fullyClosed)
                m_pHandler->OnXmlElementEnd(m_stStringBuffer);
            else
            {
                while (true)
                {
                    ParseContent(m_stStringBuffer);  // 处理一般文本 CDATA 注释

                    // 调用用户回调
                    if (m_stStringBuffer.size() > 0)
                        m_pHandler->OnXmlContent(m_stStringBuffer);

                    Accept('<');
                    if (TryAcceptOne('/'))
                    {
                        ParseName(m_stStringBuffer);

                        SkipIgnorable();
                        Accept('>');

                        // 调用用户回调
                        m_pHandler->OnXmlElementEnd(m_stStringBuffer);
                        break;  // 结束元素的解析
                    }
                    else
                    {
                        Back();
                        ParseElement();  // 嵌套解析元素
                    }
                }
            }
        }

        void CheckDeclaration()
        {
            if (TryAcceptOne('<'))
            {
                if (TryAcceptOne('?'))
                {
                    ParseName(m_stStringBuffer);
                    StringUtils::ToLowerInPlace(m_stStringBuffer);

                    // 检查节点名称
                    if (m_stStringBuffer != "xml")
                        ThrowError("Xml declaration expected, but found tag \"{0}\"", m_stStringBuffer);

                    // 读取属性
                    while (ParseAttribute(m_stKeyBuffer, m_stValueBuffer))
                    {
                        if (m_stKeyBuffer == "encoding")
                        {
                            StringUtils::ToLowerInPlace(m_stValueBuffer);
                            if (m_stValueBuffer != "utf-8" || m_stValueBuffer != "utf8")
                                ThrowError("Unsupported encoding {0}", m_stValueBuffer);
                        }
                    }

                    // 闭合标签
                    Accept('?');
                    Accept('>');
                }
                else
                    Back();
            }
        }

    private:
        XmlSaxHandler* m_pHandler = nullptr;
        std::string m_stStringBuffer;
        std::string m_stKeyBuffer;
        std::string m_stValueBuffer;
    };
}
