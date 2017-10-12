/**
 * @file
 * @date 2017/10/9
 */
#include <Moe.Core/Xml.hpp>
#include <Moe.Core/Parser.hpp>

#include <stack>

using namespace std;
using namespace moe;

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

    static void XmlWriteEscapeString(std::string& out, const std::string& raw)
    {
        out.reserve(out.length() + raw.length());

        for (auto c : raw)
        {
            switch (c)
            {
                case '"':
                    out.append("&quot;");
                    break;
                case '\'':
                    out.append("&apos;");
                    break;
                case '&':
                    out.append("&amp;");
                    break;
                case '<':
                    out.append("&lt;");
                    break;
                case '>':
                    out.append("&gt;");
                    break;
                default:
                    out.push_back(c);
                    break;
            }
        }
    }
}

XmlNode::~XmlNode()
{
}

//////////////////////////////////////////////////////////////////////////////// XmlElement

XmlElement::XmlElement(const std::string& name)
    : m_stName(name)
{
}

XmlElement::XmlElement(std::string&& name)
    : m_stName(std::move(name))
{
}

XmlElement::~XmlElement()
{
}

XmlNodePtr XmlElement::operator[](size_t index)
{
    if (index >= m_stNodes.size())
        MOE_THROW(OutOfRangeException, "Index {0} out of range", index);
    return m_stNodes[index];
}

const XmlNodePtr XmlElement::operator[](size_t index)const
{
    if (index >= m_stNodes.size())
        MOE_THROW(OutOfRangeException, "Index {0} out of range", index);
    return m_stNodes[index];
}

void XmlElement::AppendNode(XmlNodePtr node)
{
    assert(node);
    m_stNodes.push_back(node);
    m_stCache.clear();
}

bool XmlElement::RemoveNode(XmlNodePtr node)
{
    assert(node);
    auto it = m_stNodes.begin();
    while (it != m_stNodes.end())
    {
        if (*it == node)
        {
            m_stNodes.erase(it);
            m_stCache.clear();
            return true;
        }
        ++it;
    }
    return false;
}

void XmlElement::InsertNode(size_t index, XmlNodePtr node)
{
    assert(node);
    if (index > m_stNodes.size())
        index = m_stNodes.size();
    m_stNodes.insert(m_stNodes.begin() + index, node);
    m_stCache.clear();
}

const XmlElementList& XmlElement::FindElementByName(const char* name)const
{
    auto it = m_stCache.find(name);
    if (it != m_stCache.end())
        return it->second;

    XmlElementList list;
    for (size_t i = 0; i < m_stNodes.size(); ++i)
    {
        auto& p = m_stNodes[i];
        if (p->IsElement() && p.CastTo<XmlElement>()->GetName() == name)
            list.push_back(p.CastTo<XmlElement>());
    }
    m_stCache.emplace(name, std::move(list));

    it = m_stCache.find(name);
    assert(it != m_stCache.end());
    return it->second;
}

const XmlElementList& XmlElement::FindElementByName(const std::string& name)const
{
    auto it = m_stCache.find(name);
    if (it != m_stCache.end())
        return it->second;

    XmlElementList list;
    for (size_t i = 0; i < m_stNodes.size(); ++i)
    {
        auto& p = m_stNodes[i];
        if (p->IsElement() && p.CastTo<XmlElement>()->GetName() == name)
            list.push_back(p.CastTo<XmlElement>());
    }
    m_stCache.emplace(name, std::move(list));

    it = m_stCache.find(name);
    assert(it != m_stCache.end());
    return it->second;
}

void XmlElement::AddAttribute(const std::string& key, const std::string& val)
{
    auto it = m_stAttributes.find(key);
    if (it != m_stAttributes.end())
        MOE_THROW(ObjectExistsException, "Attribute \"{0}\" already existed at tag \"{1}\"", key, m_stName);
    m_stAttributes.emplace(key, val);
}

void XmlElement::AddAttribute(std::string&& key, std::string&& val)
{
    auto it = m_stAttributes.find(key);
    if (it != m_stAttributes.end())
        MOE_THROW(ObjectExistsException, "Attribute \"{0}\" already existed at tag \"{1}\"", key, m_stName);
    m_stAttributes.emplace(std::move(key), std::move(val));
}

bool XmlElement::RemoveAttribute(const std::string& key)
{
    auto it = m_stAttributes.find(key);
    if (it == m_stAttributes.end())
        return false;
    m_stAttributes.erase(it);
    return true;
}

bool XmlElement::RemoveAttribute(const char* key)
{
    auto it = m_stAttributes.find(key);
    if (it == m_stAttributes.end())
        return false;
    m_stAttributes.erase(it);
    return true;
}

bool XmlElement::ContainsAttribute(const std::string& key)
{
    auto it = m_stAttributes.find(key);
    return it != m_stAttributes.end();
}

bool XmlElement::ContainsAttribute(const char* key)
{
    auto it = m_stAttributes.find(key);
    return it != m_stAttributes.end();
}

bool XmlElement::IsElement()const noexcept
{
    return true;
}

bool XmlElement::IsText()const noexcept
{
    return false;
}

std::string& XmlElement::Stringify(std::string& str, int indent)const
{
    if (m_stName.empty())
        MOE_THROW(BadFormat, "Invalid empty tag name");
    if (!IsXmlNamePrefix(m_stName[0]))
        MOE_THROW(BadFormat, "Invalid tag name \"{0}\"", m_stName);
    for (auto& i : m_stName)
    {
        if (!IsXmlNameLetter(i))
            MOE_THROW(BadFormat, "Invalid tag name \"{0}\"", m_stName);
    }
    for (auto& i : m_stAttributes)
    {
        for (auto& j : i.first)
        {
            if (!IsXmlNameLetter(j))
                MOE_THROW(BadFormat, "Invalid attribute key \"{0}\", tag \"{1}\"", i.first, m_stName);
        }
    }

    bool pureElement = true;
    for (auto& i : m_stNodes)
    {
        if (!i->IsElement())
            pureElement = false;
    }

    // Tag名称
    str.push_back('<');
    str.append(m_stName);

    // 属性
    if (m_stAttributes.size() > 0)
    {
        for (auto& i : m_stAttributes)
        {
            str.push_back(' ');
            str.append(i.first);
            str.push_back('=');
            str.push_back('"');
            XmlWriteEscapeString(str, i.second);
            str.push_back('"');
        }
    }

    // 是否直接闭合
    if (m_stNodes.empty())
    {
        str.append(" />");
        return str;
    }
    str.push_back('>');

    if (!pureElement || indent < 0)  // 如果子节点含有Element元素和Text元素，则使用内联输出
    {
        for (auto& i : m_stNodes)
            i->Stringify(str, -1);
    }
    else  // 使用缩进
    {
        str.push_back('\n');

        ++indent;
        for (auto& i : m_stNodes)
        {
            str.reserve(str.length() + (indent << 2));
            for (int j = 0; j < (indent << 2); ++j)
                str.push_back(' ');

            i->Stringify(str, indent);
            str.push_back('\n');
        }

        str.reserve(str.length() + ((indent - 1) << 2));
        for (int j = 0; j < ((indent - 1) << 2); ++j)
            str.push_back(' ');
    }

    str.push_back('<');
    str.push_back('/');
    str.append(m_stName);
    str.push_back('>');
    return str;
}

//////////////////////////////////////////////////////////////////////////////// XmlText

XmlText::XmlText()
{
}

XmlText::XmlText(const std::string& content)
    : m_stContent(content)
{
}

XmlText::XmlText(std::string&& content)
    : m_stContent(std::move(content))
{
}

XmlText::~XmlText()
{
}

bool XmlText::IsElement()const noexcept
{
    return false;
}

bool XmlText::IsText()const noexcept
{
    return true;
}

std::string& XmlText::Stringify(std::string& str, int indent)const
{
    MOE_UNUSED(indent);

    str.append(m_stContent);
    return str;
}

//////////////////////////////////////////////////////////////////////////////// Xml

namespace
{
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

            SkipIgnorable();  // 宽松策略
            Accept('=');
            SkipIgnorable();

            char delim = Accept('\'', '"');
            while (c != '\0' && c != '<' && c != delim)
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

                                switch (state)
                                {
                                    case 0:
                                        if (ch == ']')
                                            state = 1;
                                        else
                                            buffer.push_back(ch);
                                        break;
                                    case 1:
                                        if (ch == ']')
                                            state = 2;
                                        else
                                        {
                                            buffer.push_back(']');
                                            buffer.push_back(ch);
                                            state = 0;
                                        }
                                        break;
                                    case 2:
                                        if (ch == '>')
                                            state = -1;
                                        else
                                        {
                                            buffer.append("]]");
                                            buffer.push_back(ch);
                                            state = 0;
                                        }
                                        break;
                                    default:
                                        assert(false);
                                        break;
                                }
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

                                switch (state)
                                {
                                    case 0:
                                        if (ch == '-')
                                            state = 1;
                                        break;
                                    case 1:
                                        if (ch == '-')
                                            state = 2;
                                        else
                                            state = 0;
                                        break;
                                    case 2:
                                        if (ch == '>')
                                            state = -1;
                                        else
                                            state = 0;
                                        break;
                                    default:
                                        assert(false);
                                        break;
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
                            if (m_stValueBuffer != "utf-8" && m_stValueBuffer != "utf8")
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

    class SaxHandler :
        public XmlSaxHandler
    {
    public:
        SaxHandler()
        {
        }

    public:
        XmlElementPtr GetRootNode()
        {
            if (!m_pRoot)
                MOE_THROW(BadFormat, "Empty document");
            if (!m_stStack.empty())
                MOE_THROW(BadFormat, "Unclosed root element");
            return m_pRoot;
        }

    protected:  // implement for XmlSaxHandler
        void OnXmlElementBegin(const std::string& name)override
        {
            XmlElementPtr p = MakeRef<XmlElement>(name);

            if (!m_pRoot)
                m_pRoot = p;
            else
                m_stStack.top()->AppendNode(p);

            m_stStack.push(p);
        }

        void OnXmlElementEnd(const std::string& name)override
        {
            assert(!m_stStack.empty());
            if (m_stStack.top()->GetName() != name)
            {
                MOE_THROW(BadFormat, "Xml element not match, expect tag \"{0}\", but found \"{1}\"",
                    m_stStack.top()->GetName(), name);
            }
            m_stStack.pop();
        }

        void OnXmlAttribute(const std::string& key, const std::string& val)override
        {
            assert(!m_stStack.empty());
            auto& p = m_stStack.top();
            p->AddAttribute(key, val);
        }

        void OnXmlContent(const std::string& content)override
        {
            assert(!m_stStack.empty());

            bool white = true;
            for (auto c : content)
            {
                if (!IsXmlBlankCharacter(c))
                {
                    white = false;
                    break;
                }
            }

            // 跳过全空白的Text节点
            if (white)
                return;

            XmlTextPtr p = MakeRef<XmlText>(content);
            m_stStack.top()->AppendNode(p);
        }

    private:
        XmlElementPtr m_pRoot;
        std::stack<XmlElementPtr> m_stStack;
    };
}

void Xml::Parse(XmlSaxHandler* handler, ArrayView<char> data, const char* source)
{
    XmlParser parser(handler);
    TextReader reader(data, source);

    parser.Run(reader);
}

XmlNodePtr Xml::Parse(ArrayView<char> data, const char* source)
{
    SaxHandler handler;
    XmlParser parser(&handler);
    TextReader reader(data, source);

    parser.Run(reader);
    return handler.GetRootNode();
}
