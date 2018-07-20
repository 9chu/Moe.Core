/**
 * @file
 * @date 2017/10/6
 */
#include <Moe.Core/Json.hpp>
#include <Moe.Core/Parser.hpp>
#include <Moe.Core/Encoding.hpp>

#include <stack>
#include <algorithm>
#include <climits>

using namespace std;
using namespace moe;

//////////////////////////////////////////////////////////////////////////////// JsonValue

namespace
{
    static void SerializeString(string& str, const JsonValue::StringType& input)
    {
        str.reserve(str.length() + input.length() + 2);
        str.push_back('"');

        for (size_t i = 0; i < input.length(); ++i)
        {
            char c = input[i];
            switch (c)
            {
                case '"':
                    str.append("\\\"");
                    break;
                case '\\':
                    str.append("\\\\");
                    break;
                case '/':
                    str.append("\\/");
                    break;
                case '\b':
                    str.append("\\b");
                    break;
                case '\f':
                    str.append("\\f");
                    break;
                case '\n':
                    str.append("\\n");
                    break;
                case '\r':
                    str.append("\\r");
                    break;
                case '\t':
                    str.append("\\t");
                    break;
                default:
#if CHAR_MIN < 0
                    if (c >= 0 && (!::isprint(c) || ::iscntrl(c)))
#else
                    if (!::isprint(c) || ::iscntrl(c))
#endif
                    {
                        static const uint32_t kPreAllocate = 4u;

                        char buffer[kPreAllocate];
                        auto count = Convert::ToHexStringLower((uint8_t)c, buffer, kPreAllocate);
                        assert(count < 4);

                        str.append("\\u");
                        for (unsigned j = 0; j < (4 - count); ++j)
                            str.push_back('0');
                        str.append(buffer);
                    }
                    else
                        str.push_back(c);
                    break;
            }
        }

        str.push_back('"');
    }
}

const JsonValue JsonValue::kNull;

JsonValue JsonValue::MakeObject(std::initializer_list<std::pair<std::string, JsonValue>> val)
{
    JsonValue ret;

    ret.Reset();

    new(&ret.m_stValue.Object) ObjectType();
    ret.m_iType = JsonValueTypes::Object;

    for (auto it = val.begin(); it != val.end(); ++it)
        ret.Append(it->first, it->second);
    return ret;
}

JsonValue::JsonValue()noexcept
{
}

JsonValue::JsonValue(std::nullptr_t)noexcept
{
}

JsonValue::JsonValue(BoolType val)noexcept
{
    Set(val);
}

JsonValue::JsonValue(NumberType val)noexcept
{
    Set(val);
}

JsonValue::JsonValue(const StringType& val)
{
    Set(val);
}

JsonValue::JsonValue(const ArrayType& val)
{
    Set(val);
}

JsonValue::JsonValue(const ObjectType& val)
{
    Set(val);
}

JsonValue::JsonValue(StringType&& val)noexcept
{
    Set(std::move(val));
}

JsonValue::JsonValue(ArrayType&& val)noexcept
{
    Set(std::move(val));
}

JsonValue::JsonValue(ObjectType&& val)noexcept
{
    Set(std::move(val));
}

JsonValue::JsonValue(int val)noexcept
{
    Set(val);
}

JsonValue::JsonValue(const char* val)
{
    Set(val);
}

JsonValue::JsonValue(const ArrayView<char>& val)
{
    Set(val);
}

JsonValue::JsonValue(std::initializer_list<JsonValue> val)
{
    Set(val);
}

JsonValue::JsonValue(const JsonValue& rhs)
    : m_iType(rhs.m_iType)
{
    switch (m_iType)
    {
        case JsonValueTypes::Null:
            break;
        case JsonValueTypes::Bool:
            m_stValue.Bool = rhs.m_stValue.Bool;
            break;
        case JsonValueTypes::Number:
            m_stValue.Number = rhs.m_stValue.Number;
            break;
        case JsonValueTypes::String:
            new(&m_stValue.String) StringType(rhs.m_stValue.String);
            break;
        case JsonValueTypes::Array:
            new(&m_stValue.Array) ArrayType(rhs.m_stValue.Array);
            break;
        case JsonValueTypes::Object:
            new(&m_stValue.Object) ObjectType(rhs.m_stValue.Object);
            break;
        default:
            assert(false);
            break;
    }
}

JsonValue::JsonValue(JsonValue&& rhs)noexcept
    : m_iType(rhs.m_iType)
{
    switch (m_iType)
    {
        case JsonValueTypes::Null:
            break;
        case JsonValueTypes::Bool:
            m_stValue.Bool = rhs.m_stValue.Bool;
            break;
        case JsonValueTypes::Number:
            m_stValue.Number = rhs.m_stValue.Number;
            break;
        case JsonValueTypes::String:
            new(&m_stValue.String) StringType(std::move(rhs.m_stValue.String));
            break;
        case JsonValueTypes::Array:
            new(&m_stValue.Array) ArrayType(std::move(rhs.m_stValue.Array));
            break;
        case JsonValueTypes::Object:
            new(&m_stValue.Object) ObjectType(std::move(rhs.m_stValue.Object));
            break;
        default:
            assert(false);
            break;
    }

    rhs.Reset();
}

JsonValue::~JsonValue()
{
    Reset();
}

JsonValue& JsonValue::operator=(const JsonValue& rhs)
{
    Reset();

    switch (m_iType)
    {
        case JsonValueTypes::Null:
            break;
        case JsonValueTypes::Bool:
            m_stValue.Bool = rhs.m_stValue.Bool;
            break;
        case JsonValueTypes::Number:
            m_stValue.Number = rhs.m_stValue.Number;
            break;
        case JsonValueTypes::String:
            new(&m_stValue.String) StringType(rhs.m_stValue.String);
            break;
        case JsonValueTypes::Array:
            new(&m_stValue.Array) ArrayType(rhs.m_stValue.Array);
            break;
        case JsonValueTypes::Object:
            new(&m_stValue.Object) ObjectType(rhs.m_stValue.Object);
            break;
        default:
            assert(false);
            break;
    }

    m_iType = rhs.m_iType;
    return *this;
}

JsonValue& JsonValue::operator=(JsonValue&& rhs)noexcept
{
    Reset();

    switch (m_iType)
    {
        case JsonValueTypes::Null:
            break;
        case JsonValueTypes::Bool:
            m_stValue.Bool = rhs.m_stValue.Bool;
            break;
        case JsonValueTypes::Number:
            m_stValue.Number = rhs.m_stValue.Number;
            break;
        case JsonValueTypes::String:
            new(&m_stValue.String) StringType(std::move(rhs.m_stValue.String));
            break;
        case JsonValueTypes::Array:
            new(&m_stValue.Array) ArrayType(std::move(rhs.m_stValue.Array));
            break;
        case JsonValueTypes::Object:
            new(&m_stValue.Object) ObjectType(std::move(rhs.m_stValue.Object));
            break;
        default:
            assert(false);
            break;
    }

    m_iType = rhs.m_iType;
    rhs.Reset();
    return *this;
}

bool JsonValue::operator==(std::nullptr_t)const noexcept
{
    return m_iType == JsonValueTypes::Null;
}

bool JsonValue::operator==(BoolType rhs)const noexcept
{
    return m_iType == JsonValueTypes::Bool && m_stValue.Bool == rhs;
}

bool JsonValue::operator==(NumberType rhs)const noexcept
{
    return m_iType == JsonValueTypes::Number && m_stValue.Number == rhs;
}

bool JsonValue::operator==(const char* rhs)const noexcept
{
    return m_iType == JsonValueTypes::String && m_stValue.String == rhs;
}

bool JsonValue::operator==(const StringType& rhs)const noexcept
{
    return m_iType == JsonValueTypes::String && m_stValue.String == rhs;
}

bool JsonValue::operator==(const JsonValue& rhs)const noexcept
{
    if (m_iType != rhs.m_iType)
        return false;

    switch (m_iType)
    {
        case JsonValueTypes::Null:
            return true;
        case JsonValueTypes::Bool:
            return m_stValue.Bool == rhs.m_stValue.Bool;
        case JsonValueTypes::Number:
            return m_stValue.Number == rhs.m_stValue.Number;
        case JsonValueTypes::String:
            return m_stValue.String == rhs.m_stValue.String;
        case JsonValueTypes::Array:
            return m_stValue.Array == rhs.m_stValue.Array;
        case JsonValueTypes::Object:
            return m_stValue.Object == rhs.m_stValue.Object;
        default:
            assert(false);
            return false;
    }
}

bool JsonValue::operator!=(std::nullptr_t)const noexcept
{
    return !operator==(std::nullptr_t());
}

bool JsonValue::operator!=(BoolType rhs)const noexcept
{
    return !operator==(rhs);
}

bool JsonValue::operator!=(NumberType rhs)const noexcept
{
    return !operator==(rhs);
}

bool JsonValue::operator!=(const char* rhs)const noexcept
{
    return !operator==(rhs);
}

bool JsonValue::operator!=(const StringType& rhs)const noexcept
{
    return !operator==(rhs);
}

bool JsonValue::operator!=(const JsonValue& rhs)const noexcept
{
    return !operator==(rhs);
}

JsonValue::operator bool()const noexcept
{
    switch (m_iType)
    {
        case JsonValueTypes::Null:
            return false;
        case JsonValueTypes::Bool:
            return m_stValue.Bool;
        default:
            return true;
    }
}

void JsonValue::Reset()noexcept
{
    switch (m_iType)
    {
        case JsonValueTypes::String:
            m_stValue.String.~string();
            break;
        case JsonValueTypes::Array:
            m_stValue.Array.~vector();
            break;
        case JsonValueTypes::Object:
            m_stValue.Object.~map();
            break;
        default:
            break;
    }

    m_iType = JsonValueTypes::Null;
}

void JsonValue::Set(std::nullptr_t)noexcept
{
    Reset();
}

void JsonValue::Set(BoolType val)noexcept
{
    Reset();

    m_stValue.Bool = val;
    m_iType = JsonValueTypes::Bool;
}

void JsonValue::Set(NumberType val)noexcept
{
    Reset();

    m_stValue.Number = val;
    m_iType = JsonValueTypes::Number;
}

void JsonValue::Set(const StringType& val)
{
    Reset();

    new(&m_stValue.String) StringType(val);
    m_iType = JsonValueTypes::String;
}

void JsonValue::Set(const ArrayType& val)
{
    Reset();

    new(&m_stValue.Array) ArrayType(val);
    m_iType = JsonValueTypes::Array;
}

void JsonValue::Set(const ObjectType& val)
{
    Reset();

    new(&m_stValue.Object) ObjectType(val);
    m_iType = JsonValueTypes::Object;
}

void JsonValue::Set(StringType&& val)
{
    Reset();

    new(&m_stValue.String) StringType(std::move(val));
    m_iType = JsonValueTypes::String;
}

void JsonValue::Set(ArrayType&& val)
{
    Reset();

    new(&m_stValue.Array) ArrayType(std::move(val));
    m_iType = JsonValueTypes::Array;
}

void JsonValue::Set(ObjectType&& val)
{
    Reset();

    new(&m_stValue.Object) ObjectType(std::move(val));
    m_iType = JsonValueTypes::Object;
}

void JsonValue::Set(int val)noexcept
{
    Reset();

    m_stValue.Number = val;
    m_iType = JsonValueTypes::Number;
}

void JsonValue::Set(const char* val)
{
    Reset();

    new(&m_stValue.String) StringType(val);
    m_iType = JsonValueTypes::String;
}

void JsonValue::Set(const ArrayView<char>& val)
{
    Reset();

    new(&m_stValue.String) StringType(val.GetBuffer(), val.GetSize());
    m_iType = JsonValueTypes::String;
}

void JsonValue::Set(std::initializer_list<JsonValue> val)
{
    Reset();

    new(&m_stValue.Array) ArrayType();
    m_iType = JsonValueTypes::Array;

    m_stValue.Array.reserve(val.size());
    for (auto it = val.begin(); it != val.end(); ++it)
        m_stValue.Array.push_back(*it);
}

size_t JsonValue::GetElementCount()const
{
    switch (m_iType)
    {
        case JsonValueTypes::Array:
            return m_stValue.Array.size();
        case JsonValueTypes::Object:
            return m_stValue.Object.size();
        default:
            MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);
    }
}

bool JsonValue::HasElement(const char* key)const
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    return m_stValue.Object.find(key) != m_stValue.Object.end();
}

bool JsonValue::HasElement(const std::string& key)const
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    return m_stValue.Object.find(key) != m_stValue.Object.end();
}

JsonValue& JsonValue::GetElementByIndex(size_t index)
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);
    if (index >= m_stValue.Array.size())
        MOE_THROW(OutOfRangeException, "Index {0} out of range", index);

    return m_stValue.Array[index];
}

const JsonValue& JsonValue::GetElementByIndex(size_t index)const
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);
    if (index >= m_stValue.Array.size())
        MOE_THROW(OutOfRangeException, "Index {0} out of range", index);

    return m_stValue.Array[index];
}

JsonValue& JsonValue::GetElementByKey(const std::string& key)
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.Object.find(key);
    if (it == m_stValue.Object.end())
        MOE_THROW(ObjectNotFoundException, "Key \"{0}\" not found", key);
    return it->second;
}

const JsonValue& JsonValue::GetElementByKey(const std::string& key)const
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.Object.find(key);
    if (it == m_stValue.Object.end())
        MOE_THROW(ObjectNotFoundException, "Key \"{0}\" not found", key);
    return it->second;
}

JsonValue& JsonValue::GetElementByKey(const char* key)
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.Object.find(key);
    if (it == m_stValue.Object.end())
        MOE_THROW(ObjectNotFoundException, "Key \"{0}\" not found", key);
    return it->second;
}

const JsonValue& JsonValue::GetElementByKey(const char* key)const
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.Object.find(key);
    if (it == m_stValue.Object.end())
        MOE_THROW(ObjectNotFoundException, "Key \"{0}\" not found", key);
    return it->second;
}

void JsonValue::Append(const JsonValue& val)
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    m_stValue.Array.push_back(val);
}

void JsonValue::Append(JsonValue&& val)
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    m_stValue.Array.emplace_back(std::move(val));
}

void JsonValue::Append(const std::string& key, const JsonValue& val)
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.Object.find(key);
    if (it == m_stValue.Object.end())
        m_stValue.Object.emplace(key, val);
    else
        MOE_THROW(ObjectExistsException, "Key \"{0}\" exists", key);
}

void JsonValue::Append(std::string&& key, JsonValue&& val)
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.Object.find(key);
    if (it == m_stValue.Object.end())
        m_stValue.Object.emplace(std::move(key), std::move(val));
    else
        MOE_THROW(ObjectExistsException, "Key \"{0}\" exists", key);
}

void JsonValue::Insert(size_t index, const JsonValue& val)
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    index = std::min(index, m_stValue.Array.size());
    m_stValue.Array.insert(m_stValue.Array.begin() + index, val);
}

void JsonValue::Insert(size_t index, JsonValue&& val)
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    index = std::min(index, m_stValue.Array.size());
    m_stValue.Array.emplace(m_stValue.Array.begin() + index, std::move(val));
}

bool JsonValue::Remove(size_t index)
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    if (index >= m_stValue.Array.size())
        return false;

    m_stValue.Array.erase(m_stValue.Array.begin() + index);
    return true;
}

bool JsonValue::Remove(const char* key)
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.Object.find(key);
    if (it == m_stValue.Object.end())
        return false;
    m_stValue.Object.erase(it);
    return true;
}

bool JsonValue::Remove(const std::string& key)
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.Object.find(key);
    if (it == m_stValue.Object.end())
        return false;
    m_stValue.Object.erase(it);
    return true;
}

void JsonValue::Clear()
{
    if (m_iType == JsonValueTypes::Array)
        m_stValue.Array.clear();
    else if (m_iType == JsonValueTypes::Object)
        m_stValue.Object.clear();
    else
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);
}

std::string& JsonValue::Stringify(std::string& str)const
{
    return Stringify(str, 0);
}

std::string& JsonValue::StringifyInline(std::string& str)const
{
    size_t i = 0;

    switch (m_iType)
    {
        case JsonValueTypes::Null:
            str.append("null");
            break;
        case JsonValueTypes::Bool:
            m_stValue.Bool ? str.append("true") : str.append("false");
            break;
        case JsonValueTypes::Number:
            {
                static const uint32_t kPreAllocate = 128u;

                auto pos = str.length();
                str.resize(pos + kPreAllocate);
                auto count = Convert::ToShortestString(m_stValue.Number, &str[pos], kPreAllocate);
                str.resize(pos + count);
            }
            break;
        case JsonValueTypes::String:
            SerializeString(str, m_stValue.String);
            break;
        case JsonValueTypes::Array:
            str.reserve(str.length() + (m_stValue.Array.size() << 2));

            str.push_back('[');
            for (; i < m_stValue.Array.size(); ++i)
            {
                const auto& obj = m_stValue.Array[i];
                obj.StringifyInline(str);

                if (i + 1 < m_stValue.Array.size())
                {
                    str.push_back(',');
                    str.push_back(' ');
                }
            }
            str.push_back(']');
            break;
        case JsonValueTypes::Object:
            str.reserve(str.length() + (m_stValue.Object.size() << 2));

            str.push_back('{');
            for (auto it = m_stValue.Object.begin(); it != m_stValue.Object.end(); ++it)
            {
                SerializeString(str, it->first);
                str.push_back(':');
                str.push_back(' ');

                const auto& obj = it->second;
                obj.StringifyInline(str);

                if (i + 1 < m_stValue.Object.size())
                {
                    str.push_back(',');
                    str.push_back(' ');
                }
                ++i;
            }
            str.push_back('}');
            break;
        default:
            assert(false);
            break;
    }

    return str;
}

std::string& JsonValue::Stringify(std::string& str, uint32_t indent)const
{
    size_t i = 0;

    switch (m_iType)
    {
        case JsonValueTypes::Array:
            str.reserve(str.length() + (m_stValue.Array.size() << 2));

            str.push_back('[');

            if (m_stValue.Array.size() == 1)
            {
                m_stValue.Array[0].StringifyInline(str);
            }
            else if (m_stValue.Array.size() > 1)
            {
                str.push_back('\n');

                ++indent;
                for (; i < m_stValue.Array.size(); ++i)
                {
                    str.reserve(str.length() + (indent << 1));
                    for (unsigned j = 0; j < (indent << 1); ++j)
                        str.push_back(' ');

                    const auto& obj = m_stValue.Array[i];
                    obj.Stringify(str, indent);

                    if (i + 1 < m_stValue.Array.size())
                        str.push_back(',');

                    str.push_back('\n');
                }

                str.reserve(str.length() + ((indent - 1) << 1));
                for (unsigned j = 0; j < ((indent - 1) << 1); ++j)
                    str.push_back(' ');
            }

            str.push_back(']');
            break;
        case JsonValueTypes::Object:
            str.reserve(str.length() + (m_stValue.Object.size() << 2));

            str.push_back('{');

            if (m_stValue.Object.size() > 0)
            {
                str.push_back('\n');

                ++indent;

                for (auto it = m_stValue.Object.begin(); it != m_stValue.Object.end(); ++it)
                {
                    str.reserve(str.length() + (indent << 1));
                    for (unsigned j = 0; j < (indent << 1); ++j)
                        str.push_back(' ');

                    SerializeString(str, it->first);
                    str.push_back(':');
                    str.push_back(' ');

                    const auto& obj = it->second;
                    obj.Stringify(str, indent);

                    if (i + 1 < m_stValue.Object.size())
                        str.push_back(',');

                    str.push_back('\n');
                    ++i;
                }

                str.reserve(str.length() + ((indent - 1) << 1));
                for (unsigned j = 0; j < ((indent - 1) << 1); ++j)
                    str.push_back(' ');
            }

            str.push_back('}');
            break;
        default:
            StringifyInline(str);
            break;
    }

    return str;
}

//////////////////////////////////////////////////////////////////////////////// Json5

namespace
{
    class Json5Parser :
        public Parser
    {
    public:
        Json5Parser(JsonSaxHandler* handler)
            : m_pHandler(handler) {}

    public:
        void Run(TextReader& reader)override
        {
            Parser::Run(reader);

            // 初始化内部状态
            m_stStringBuffer.clear();

            // 开始解析
            ParseValue();
            SkipIgnorable();

            if (c != '\0')
                ThrowError("Bad tailing character {0}", PrintChar(c));
        }

    private:
        void BufferUnicodeCharacter(char32_t ch)
        {
            uint32_t count = 0;
            array<char, Encoding::Utf8::Encoder::kMaxOutputCount> buffer {};
            Encoding::Utf8::Encoder encoder;

            if (Encoding::EncodingResult::Accept != encoder(ch, buffer, count))
                ThrowError("Encoding {0} to utf-8 failed", (int)ch);

            m_stStringBuffer.append(buffer.data(), count);
        }

        void ReadString()
        {
            m_stStringBuffer.clear();

            char delim = Accept('\'', '"');
            while (c != '\0')
            {
                if (c == delim)
                {
                    Next();
                    return;
                }
                else if (c == '\\')
                {
                    Next();

                    char ch;
                    switch (ch = Next())
                    {
                        case '\'':
                            m_stStringBuffer.push_back('\'');
                            break;
                        case '"':
                            m_stStringBuffer.push_back('"');
                            break;
                        case '\\':
                            m_stStringBuffer.push_back('\\');
                            break;
                        case '/':
                            m_stStringBuffer.push_back('/');
                            break;
                        case 'b':
                            m_stStringBuffer.push_back('\b');
                            break;
                        case 'f':
                            m_stStringBuffer.push_back('\f');
                            break;
                        case 'n':
                            m_stStringBuffer.push_back('\n');
                            break;
                        case 'r':
                            m_stStringBuffer.push_back('\r');
                            break;
                        case 't':
                            m_stStringBuffer.push_back('\t');
                            break;
                        case 'u':
                            {
                                char32_t u32 = 0;
                                for (int i = 0; i < 4; ++i)
                                {
                                    int hex = 0;
                                    if (!StringUtils::HexDigitToNumber(hex, c))
                                        ThrowError("Unexpected hex character {0}", PrintChar(c));
                                    u32 = (u32 << 4) + hex;
                                    Next();
                                }
                                BufferUnicodeCharacter(u32);
                            }
                            break;
                        case '\n':
                            break;
                        case '\r':
                            if (c == '\n')
                                Next();
                            break;
                        default:
                            ThrowError("Unexpected escape character {0}", PrintChar(ch));
                    }
                }
                else if (c > 0 && c <= 0x1F && c != '\t')  // 控制字符必须被escape，例外的，我们允许\t
                    ThrowError("Unexpected character {0}", PrintChar(c));
                else
                    m_stStringBuffer.push_back(Next());
            }

            ThrowError("Unterminated string");
        }

        void ReadIdentifier()
        {
            m_stStringBuffer.clear();

            if ((c != '_' && c != '$') && (c < 'a' || c > 'z') && (c < 'A' || c > 'Z'))
                ThrowError("Bad identifier character {0}", PrintChar(c));
            m_stStringBuffer.push_back(Next());

            while (c == '_' || c == '$' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9'))
            {
                m_stStringBuffer.push_back(Next());
            }
        }

        void ParseWord()
        {
            // true, false, null, Infinity, NaN
            switch (c)
            {
                case 't':  // true
                    Accept('t');
                    Accept('r');
                    Accept('u');
                    Accept('e');
                    m_pHandler->OnJsonBool(true);
                    break;
                case 'f':  // false
                    Accept('f');
                    Accept('a');
                    Accept('l');
                    Accept('s');
                    Accept('e');
                    m_pHandler->OnJsonBool(false);
                    break;
                case 'n':  // null
                    Accept('n');
                    Accept('u');
                    Accept('l');
                    Accept('l');
                    m_pHandler->OnJsonNull();
                    break;
                case 'I':  // Infinity
                    Accept('I');
                    Accept('n');
                    Accept('f');
                    Accept('i');
                    Accept('n');
                    Accept('i');
                    Accept('t');
                    Accept('y');
                    m_pHandler->OnJsonNumber(numeric_limits<double>::infinity());
                    break;
                case 'N':  // NaN
                    Accept('N');
                    Accept('a');
                    Accept('N');
                    m_pHandler->OnJsonNumber(numeric_limits<double>::quiet_NaN());
                    break;
                default:
                    ThrowError("Unexpected character {0}", PrintChar(c));
            }
        }

        void ParseNumber()
        {
            char sign = '+';

            if (c == '-' || c == '+')
            {
                sign = c;
                Next();
            }

            // 检查是否是Infinity或者NaN
            if (c == 'I')  // Infinity
            {
                Accept('I');
                Accept('n');
                Accept('f');
                Accept('i');
                Accept('n');
                Accept('i');
                Accept('t');
                Accept('y');
                m_pHandler->OnJsonNumber(sign == '+' ?
                    numeric_limits<double>::infinity() : -numeric_limits<double>::infinity());
                return;
            }
            else if (c == 'N')  // NaN
            {
                Accept('N');
                Accept('a');
                Accept('N');
                m_pHandler->OnJsonNumber(numeric_limits<double>::quiet_NaN());  // 不用考虑符号
                return;
            }

            int base = 10;

            m_stStringBuffer.clear();
            if (c == '0')
            {
                m_stStringBuffer.push_back(Next());

                if (c == 'x' || c == 'X')
                {
                    m_stStringBuffer.push_back('x');
                    Next();

                    if (!(('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F')))
                        ThrowError("Unexpected character {0}", PrintChar(c));

                    base = 16;
                }
                else if ('0' <= c && c <= '9')
                    ThrowError("Unexpected character {0}", PrintChar(c));
            }

            switch (base)
            {
                case 10:
                    while ('0' <= c && c <= '9')
                        m_stStringBuffer.push_back(Next());

                    if (c == '.')
                    {
                        m_stStringBuffer.push_back(Next());

                        while ('0' <= c && c <= '9')
                            m_stStringBuffer.push_back(Next());
                    }
                    if (c == 'e' || c == 'E')
                    {
                        m_stStringBuffer.push_back(Next());

                        if (c == '-' || c == '+')
                            m_stStringBuffer.push_back(Next());

                        while ('0' <= c && c <= '9')
                            m_stStringBuffer.push_back(Next());
                    }
                    break;
                case 16:
                    while (('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f'))
                        m_stStringBuffer.push_back(Next());
                    break;
                default:
                    assert(false);
                    break;
            }

            if (base == 16)
            {
                size_t processed = 0;
                auto result = Convert::ParseInt(m_stStringBuffer.c_str(), m_stStringBuffer.length(), processed);
                if (processed != m_stStringBuffer.length())
                    ThrowError("Parse int \"{0}\" failed", m_stStringBuffer);

                result = (sign == '-' ? -result : result);

                m_pHandler->OnJsonNumber(static_cast<JsonValue::NumberType>(result));
            }
            else
            {
                size_t processed = 0;
                auto result = Convert::ParseDouble(m_stStringBuffer.c_str(), m_stStringBuffer.length(), processed);
                if (processed != m_stStringBuffer.length())
                    ThrowError("Parse double \"{0}\" failed", m_stStringBuffer);

                result = (sign == '-' ? -result : result);

                m_pHandler->OnJsonNumber(result);
            }
        }

        void ParseString()
        {
            ReadString();
            m_pHandler->OnJsonString(m_stStringBuffer);
        }

        void ParseArray()
        {
            Accept('[');
            SkipIgnorable();

            m_pHandler->OnJsonArrayBegin();

            while (c != '\0')
            {
                if (c == ']')
                {
                    Next();
                    m_pHandler->OnJsonArrayEnd();
                    return;
                }

                if (c == ',')
                    ThrowError("Missing array element");
                else
                    ParseValue();

                SkipIgnorable();

                if (c != ',')
                {
                    Accept(']');
                    m_pHandler->OnJsonArrayEnd();
                    return;
                }

                Accept(',');
                SkipIgnorable();
            }

            ThrowError("Unterminated array");
        }

        void ParseObject()
        {
            Accept('{');
            SkipIgnorable();

            m_pHandler->OnJsonObjectBegin();

            while (c != '\0')
            {
                if (c == '}')
                {
                    Next();
                    m_pHandler->OnJsonObjectEnd();
                    return;
                }

                if (c == '"' || c == '\'')
                    ReadString();
                else
                    ReadIdentifier();

                m_pHandler->OnJsonObjectKey(m_stStringBuffer);

                SkipIgnorable();
                Accept(':');
                ParseValue();
                SkipIgnorable();

                if (c != ',')
                {
                    Accept('}');
                    m_pHandler->OnJsonObjectEnd();
                    return;
                }

                Accept(',');
                SkipIgnorable();
            }

            ThrowError("Unterminated object");
        }

        void ParseComment()
        {
            Accept('/');

            if (c == '/')  // 单行注释
            {
                do
                {
                    Next();

                    if (c == '\n' || c == '\r')
                    {
                        Next();
                        return;
                    }
                } while (c != '\0');
            }
            else if (c == '*')  // 块注释
            {
                do
                {
                    Next();

                    while (c == '*')
                    {
                        Next();

                        if (c == '/')
                        {
                            Next();
                            return;
                        }
                    }
                } while (c != '\0');

                ThrowError("Unterminated block comment");
            }

            ThrowError("Unexpected character {0}", PrintChar(c));
        }

        void SkipIgnorable()
        {
            // 跳过空白符和注释
            while (c != '\0')
            {
                if (c == '/')
                    ParseComment();
                else
                {
                    switch (static_cast<uint8_t>(c))
                    {
                        case ' ':
                        case '\t':
                        case '\r':
                        case '\n':
                        case '\v':
                        case '\f':
                        case 160:  // 0xA0
                            Next();
                            break;
                        default:
                            return;
                    }
                }
            }
        }

        void ParseValue()
        {
            SkipIgnorable();

            switch (c)
            {
                case '{':
                    ParseObject();
                    break;
                case '[':
                    ParseArray();
                    break;
                case '"':
                case '\'':
                    ParseString();
                    break;
                case '-':
                case '+':
                case '.':
                    ParseNumber();
                    break;
                default:
                    if (c >= '0' && c <= '9')
                        ParseNumber();
                    else
                        ParseWord();
                    break;
            }
        }

    public:
        JsonSaxHandler* m_pHandler = nullptr;
        std::string m_stStringBuffer;
    };

    class SaxHandler :
        public JsonSaxHandler
    {
    public:
        SaxHandler(JsonValue& out)
        {
            m_stStack.push(&out);
        }

    protected:  // implement for JsonSaxHandler
        void OnJsonNull()override
        {
            JsonValue& top = *m_stStack.top();

            if (m_stStack.size() == 1)
                top.Reset();
            else if (top.Is<JsonValue::ArrayType>())
                top.Append(JsonValue());
            else if (top.Is<JsonValue::ObjectType>())
                top.Append(m_stKey, JsonValue());
            else
                assert(false);
        }

        void OnJsonBool(JsonValue::BoolType val)override
        {
            JsonValue& top = *m_stStack.top();

            if (m_stStack.size() == 1)
                top.Set(val);
            else if (top.Is<JsonValue::ArrayType>())
                top.Append(JsonValue(val));
            else if (top.Is<JsonValue::ObjectType>())
                top.Append(m_stKey, JsonValue(val));
            else
                assert(false);
        }

        void OnJsonNumber(JsonValue::NumberType val)override
        {
            JsonValue& top = *m_stStack.top();

            if (m_stStack.size() == 1)
                top.Set(val);
            else if (top.Is<JsonValue::ArrayType>())
                top.Append(JsonValue(val));
            else if (top.Is<JsonValue::ObjectType>())
                top.Append(m_stKey, JsonValue(val));
            else
                assert(false);
        }

        void OnJsonString(const JsonValue::StringType& val)override
        {
            JsonValue& top = *m_stStack.top();

            if (m_stStack.size() == 1)
                top.Set(val);
            else if (top.Is<JsonValue::ArrayType>())
                top.Append(JsonValue(val));
            else if (top.Is<JsonValue::ObjectType>())
                top.Append(m_stKey, JsonValue(val));
            else
                assert(false);
        }

        void OnJsonArrayBegin()override
        {
            JsonValue& top = *m_stStack.top();

            if (m_stStack.size() == 1)
            {
                top.Set(JsonValue::ArrayType());
                m_stStack.push(&top);
            }
            else if (top.Is<JsonValue::ArrayType>())
            {
                top.Append(JsonValue::ArrayType());
                m_stStack.push(&top.GetElementByIndex(top.GetElementCount() - 1));
            }
            else if (top.Is<JsonValue::ObjectType>())
            {
                top.Append(m_stKey, JsonValue::ArrayType());
                m_stStack.push(&top.GetElementByKey(m_stKey));
            }
            else
                assert(false);
        }

        void OnJsonArrayEnd()override
        {
            assert(m_stStack.size() > 1);

            JsonValue& top = *m_stStack.top();
            MOE_UNUSED(top);
            assert(top.Is<JsonValue::ArrayType>());
            m_stStack.pop();
        }

        void OnJsonObjectBegin()override
        {
            JsonValue& top = *m_stStack.top();

            if (m_stStack.size() == 1)
            {
                top.Set(JsonValue::ObjectType());
                m_stStack.push(&top);
            }
            else if (top.Is<JsonValue::ArrayType>())
            {
                top.Append(JsonValue::ObjectType());
                m_stStack.push(&top.GetElementByIndex(top.GetElementCount() - 1));
            }
            else if (top.Is<JsonValue::ObjectType>())
            {
                top.Append(m_stKey, JsonValue::ObjectType());
                m_stStack.push(&top.GetElementByKey(m_stKey));
            }
            else
                assert(false);
        }

        void OnJsonObjectKey(const std::string& key)override
        {
            m_stKey = key;
        }

        void OnJsonObjectEnd()override
        {
            assert(m_stStack.size() > 1);

            JsonValue& top = *m_stStack.top();
            MOE_UNUSED(top);
            assert(top.Is<JsonValue::ObjectType>());
            m_stStack.pop();
        }

    private:
        stack<JsonValue*> m_stStack;
        string m_stKey;
    };
}

void Json5::Parse(JsonSaxHandler* handler, ArrayView<char> data, const char* source)
{
    Json5Parser parser(handler);
    TextReader reader(data, source);

    parser.Run(reader);
}

void Json5::Parse(JsonValue& out, ArrayView<char> data, const char* source)
{
    SaxHandler handler(out);
    Json5Parser parser(&handler);
    TextReader reader(data, source);

    parser.Run(reader);
}
