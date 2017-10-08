/**
 * @file
 * @date 2017/10/6
 */
#include <Moe.Core/Json.hpp>
#include <Moe.Core/Parser.hpp>
#include <Moe.Core/Encoding.hpp>

using namespace std;
using namespace moe;

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
                    if (c >= 0 && !::isprint(c))
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

//////////////////////////////////////////////////////////////////////////////// JsonValue

const JsonValue JsonValue::kNull;

JsonValue JsonValue::MakeObject(std::initializer_list<std::pair<std::string, JsonValue>> val)
{
    JsonValue ret;

    ret.Reset();

    new(&ret.m_stValue.m_stObject) ObjectType();
    ret.m_iType = JsonValueTypes::Object;

    ret.m_stValue.m_stObject.reserve(val.size());
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
        case JsonValueTypes::Bool:
            m_stValue.m_bBool = rhs.m_stValue.m_bBool;
            break;
        case JsonValueTypes::Number:
            m_stValue.m_dNumber = rhs.m_stValue.m_dNumber;
            break;
        case JsonValueTypes::String:
            new(&m_stValue.m_stString) StringType(rhs.m_stValue.m_stString);
            break;
        case JsonValueTypes::Array:
            new(&m_stValue.m_stArray) ArrayType(rhs.m_stValue.m_stArray);
            break;
        case JsonValueTypes::Object:
            new(&m_stValue.m_stObject) ObjectType(rhs.m_stValue.m_stObject);
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
        case JsonValueTypes::Bool:
            m_stValue.m_bBool = rhs.m_stValue.m_bBool;
            break;
        case JsonValueTypes::Number:
            m_stValue.m_dNumber = rhs.m_stValue.m_dNumber;
            break;
        case JsonValueTypes::String:
            new(&m_stValue.m_stString) StringType(std::move(rhs.m_stValue.m_stString));
            break;
        case JsonValueTypes::Array:
            new(&m_stValue.m_stArray) ArrayType(std::move(rhs.m_stValue.m_stArray));
            break;
        case JsonValueTypes::Object:
            new(&m_stValue.m_stObject) ObjectType(std::move(rhs.m_stValue.m_stObject));
            break;
        default:
            assert(false);
            break;
    }

    rhs.m_iType = JsonValueTypes::Null;
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
        case JsonValueTypes::Bool:
            m_stValue.m_bBool = rhs.m_stValue.m_bBool;
            break;
        case JsonValueTypes::Number:
            m_stValue.m_dNumber = rhs.m_stValue.m_dNumber;
            break;
        case JsonValueTypes::String:
            new(&m_stValue.m_stString) StringType(rhs.m_stValue.m_stString);
            break;
        case JsonValueTypes::Array:
            new(&m_stValue.m_stArray) ArrayType(rhs.m_stValue.m_stArray);
            break;
        case JsonValueTypes::Object:
            new(&m_stValue.m_stObject) ObjectType(rhs.m_stValue.m_stObject);
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
        case JsonValueTypes::Bool:
            m_stValue.m_bBool = rhs.m_stValue.m_bBool;
            break;
        case JsonValueTypes::Number:
            m_stValue.m_dNumber = rhs.m_stValue.m_dNumber;
            break;
        case JsonValueTypes::String:
            new(&m_stValue.m_stString) StringType(std::move(rhs.m_stValue.m_stString));
            break;
        case JsonValueTypes::Array:
            new(&m_stValue.m_stArray) ArrayType(std::move(rhs.m_stValue.m_stArray));
            break;
        case JsonValueTypes::Object:
            new(&m_stValue.m_stObject) ObjectType(std::move(rhs.m_stValue.m_stObject));
            break;
        default:
            assert(false);
            break;
    }

    m_iType = rhs.m_iType;
    rhs.m_iType = JsonValueTypes::Null;
    return *this;
}

bool JsonValue::operator==(std::nullptr_t)const noexcept
{
    return m_iType == JsonValueTypes::Null;
}

bool JsonValue::operator==(BoolType rhs)const noexcept
{
    return m_iType == JsonValueTypes::Bool && m_stValue.m_bBool == rhs;
}

bool JsonValue::operator==(NumberType rhs)const noexcept
{
    return m_iType == JsonValueTypes::Number && m_stValue.m_dNumber == rhs;
}

bool JsonValue::operator==(const char* rhs)const noexcept
{
    return m_iType == JsonValueTypes::String && m_stValue.m_stString == rhs;
}

bool JsonValue::operator==(const StringType& rhs)const noexcept
{
    return m_iType == JsonValueTypes::String && m_stValue.m_stString == rhs;
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
            return m_stValue.m_bBool == rhs.m_stValue.m_bBool;
        case JsonValueTypes::Number:
            return m_stValue.m_dNumber == rhs.m_stValue.m_dNumber;
        case JsonValueTypes::String:
            return m_stValue.m_stString == rhs.m_stValue.m_stString;
        case JsonValueTypes::Array:
            return m_stValue.m_stArray == rhs.m_stValue.m_stArray;
        case JsonValueTypes::Object:
            return m_stValue.m_stObject == rhs.m_stValue.m_stObject;
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
            return m_stValue.m_bBool;
        default:
            return true;
    }
}

void JsonValue::Reset()noexcept
{
    switch (m_iType)
    {
        case JsonValueTypes::String:
            m_stValue.m_stString.~string();
            break;
        case JsonValueTypes::Array:
            m_stValue.m_stArray.~vector();
            break;
        case JsonValueTypes::Object:
            m_stValue.m_stObject.~unordered_map();
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

    m_stValue.m_bBool = val;
    m_iType = JsonValueTypes::Bool;
}

void JsonValue::Set(NumberType val)noexcept
{
    Reset();

    m_stValue.m_dNumber = val;
    m_iType = JsonValueTypes::Number;
}

void JsonValue::Set(const StringType& val)
{
    Reset();

    new(&m_stValue.m_stString) StringType(val);
    m_iType = JsonValueTypes::String;
}

void JsonValue::Set(const ArrayType& val)
{
    Reset();

    new(&m_stValue.m_stArray) ArrayType(val);
    m_iType = JsonValueTypes::Array;
}

void JsonValue::Set(const ObjectType& val)
{
    Reset();

    new(&m_stValue.m_stObject) ObjectType(val);
    m_iType = JsonValueTypes::Object;
}

void JsonValue::Set(StringType&& val)
{
    Reset();

    new(&m_stValue.m_stString) StringType(std::move(val));
    m_iType = JsonValueTypes::String;
}

void JsonValue::Set(ArrayType&& val)
{
    Reset();

    new(&m_stValue.m_stArray) ArrayType(std::move(val));
    m_iType = JsonValueTypes::Array;
}

void JsonValue::Set(ObjectType&& val)
{
    Reset();

    new(&m_stValue.m_stObject) ObjectType(std::move(val));
    m_iType = JsonValueTypes::Object;
}

void JsonValue::Set(int val)noexcept
{
    Reset();

    m_stValue.m_dNumber = val;
    m_iType = JsonValueTypes::Number;
}

void JsonValue::Set(const char* val)
{
    Reset();

    new(&m_stValue.m_stString) StringType(val);
    m_iType = JsonValueTypes::String;
}

void JsonValue::Set(const ArrayView<char>& val)
{
    Reset();

    new(&m_stValue.m_stString) StringType(val.GetBuffer(), val.GetSize());
    m_iType = JsonValueTypes::String;
}

void JsonValue::Set(std::initializer_list<JsonValue> val)
{
    Reset();

    new(&m_stValue.m_stArray) ArrayType();
    m_iType = JsonValueTypes::Array;

    m_stValue.m_stArray.reserve(val.size());
    for (auto it = val.begin(); it != val.end(); ++it)
        m_stValue.m_stArray.push_back(*it);
}

size_t JsonValue::GetElementCount()const
{
    switch (m_iType)
    {
        case JsonValueTypes::Array:
            return m_stValue.m_stArray.size();
        case JsonValueTypes::Object:
            return m_stValue.m_stObject.size();
        default:
            MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);
    }
}

bool JsonValue::HasElement(const char* key)const
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    return m_stValue.m_stObject.find(key) != m_stValue.m_stObject.end();
}

bool JsonValue::HasElement(const std::string& key)const
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    return m_stValue.m_stObject.find(key) != m_stValue.m_stObject.end();
}

JsonValue& JsonValue::GetElementByIndex(size_t index)
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);
    if (index < m_stValue.m_stArray.size())
        MOE_THROW(OutOfRangeException, "Index {0} out of range", index);

    return m_stValue.m_stArray[index];
}

const JsonValue& JsonValue::GetElementByIndex(size_t index)const
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);
    if (index < m_stValue.m_stArray.size())
        MOE_THROW(OutOfRangeException, "Index {0} out of range", index);

    return m_stValue.m_stArray[index];
}

JsonValue& JsonValue::GetElementByKey(const std::string& key)
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.m_stObject.find(key);
    if (it == m_stValue.m_stObject.end())
        MOE_THROW(ObjectNotFoundException, "Key \"{0}\" not found", key);
    return it->second;
}

const JsonValue& JsonValue::GetElementByKey(const std::string& key)const
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.m_stObject.find(key);
    if (it == m_stValue.m_stObject.end())
        MOE_THROW(ObjectNotFoundException, "Key \"{0}\" not found", key);
    return it->second;
}

JsonValue& JsonValue::GetElementByKey(const char* key)
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.m_stObject.find(key);
    if (it == m_stValue.m_stObject.end())
        MOE_THROW(ObjectNotFoundException, "Key \"{0}\" not found", key);
    return it->second;
}

const JsonValue& JsonValue::GetElementByKey(const char* key)const
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.m_stObject.find(key);
    if (it == m_stValue.m_stObject.end())
        MOE_THROW(ObjectNotFoundException, "Key \"{0}\" not found", key);
    return it->second;
}

void JsonValue::Append(const JsonValue& val)
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    m_stValue.m_stArray.push_back(val);
}

void JsonValue::Append(JsonValue&& val)
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    m_stValue.m_stArray.emplace_back(std::move(val));
}

void JsonValue::Append(const std::string& key, const JsonValue& val)
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    m_stValue.m_stObject.emplace(key, val);
}

void JsonValue::Append(std::string&& key, JsonValue&& val)
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    m_stValue.m_stObject.emplace(std::move(key), std::move(val));
}

void JsonValue::Insert(size_t index, const JsonValue& val)
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    index = std::min(index, m_stValue.m_stArray.size());
    m_stValue.m_stArray.insert(m_stValue.m_stArray.begin() + index, val);
}

void JsonValue::Insert(size_t index, JsonValue&& val)
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    index = std::min(index, m_stValue.m_stArray.size());
    m_stValue.m_stArray.emplace(m_stValue.m_stArray.begin() + index, std::move(val));
}

bool JsonValue::Remove(size_t index)
{
    if (m_iType != JsonValueTypes::Array)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    if (index >= m_stValue.m_stArray.size())
        return false;

    m_stValue.m_stArray.erase(m_stValue.m_stArray.begin() + index);
    return true;
}

bool JsonValue::Remove(const char* key)
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.m_stObject.find(key);
    if (it == m_stValue.m_stObject.end())
        return false;
    m_stValue.m_stObject.erase(it);
    return true;
}

bool JsonValue::Remove(const std::string& key)
{
    if (m_iType != JsonValueTypes::Object)
        MOE_THROW(InvalidCallException, "Bad operation on type {0}", m_iType);

    auto it = m_stValue.m_stObject.find(key);
    if (it == m_stValue.m_stObject.end())
        return false;
    m_stValue.m_stObject.erase(it);
    return true;
}

void JsonValue::Clear()
{
    if (m_iType == JsonValueTypes::Array)
        m_stValue.m_stArray.clear();
    else if (m_iType == JsonValueTypes::Object)
        m_stValue.m_stObject.clear();
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
            m_stValue.m_bBool ? str.append("true") : str.append("false");
            break;
        case JsonValueTypes::Number:
            {
                static const uint32_t kPreAllocate = 128u;

                auto pos = str.length();
                str.resize(pos + kPreAllocate);
                auto count = Convert::ToShortestString(m_stValue.m_dNumber, &str[pos], kPreAllocate);
                str.resize(pos + count);
            }
            break;
        case JsonValueTypes::String:
            SerializeString(str, m_stValue.m_stString);
            break;
        case JsonValueTypes::Array:
            str.reserve(str.length() + (m_stValue.m_stArray.size() << 2));

            str.push_back('[');
            for (; i < m_stValue.m_stArray.size(); ++i)
            {
                const auto& obj = m_stValue.m_stArray[i];
                obj.StringifyInline(str);

                if (i + 1 < m_stValue.m_stArray.size())
                {
                    str.push_back(',');
                    str.push_back(' ');
                }
            }
            str.push_back(']');
            break;
        case JsonValueTypes::Object:
            str.reserve(str.length() + (m_stValue.m_stObject.size() << 2));

            str.push_back('{');
            for (auto it = m_stValue.m_stObject.begin(); it != m_stValue.m_stObject.end(); ++it)
            {
                SerializeString(str, it->first);
                str.push_back(':');
                str.push_back(' ');

                const auto& obj = it->second;
                obj.StringifyInline(str);

                if (i + 1 < m_stValue.m_stObject.size())
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
            str.reserve(str.length() + (m_stValue.m_stArray.size() << 2));

            str.push_back('[');

            if (m_stValue.m_stArray.size() == 1)
            {
                m_stValue.m_stArray[0].StringifyInline(str);
            }
            else if (m_stValue.m_stArray.size() > 1)
            {
                str.push_back('\n');

                ++indent;
                string indentStr(indent << 1, ' ');

                for (; i < m_stValue.m_stArray.size(); ++i)
                {
                    str.append(indentStr);

                    const auto& obj = m_stValue.m_stArray[i];
                    obj.Stringify(str, indent);

                    if (i + 1 < m_stValue.m_stArray.size())
                        str.push_back(',');

                    str.push_back('\n');
                }

                str.append(string((indent - 1) << 1, ' '));
            }

            str.push_back(']');
            break;
        case JsonValueTypes::Object:
            str.reserve(str.length() + (m_stValue.m_stObject.size() << 2));

            str.push_back('{');

            if (m_stValue.m_stObject.size() > 0)
            {
                str.push_back('\n');

                ++indent;
                string indentStr(indent << 1, ' ');

                for (auto it = m_stValue.m_stObject.begin(); it != m_stValue.m_stObject.end(); ++it)
                {
                    str.append(indentStr);

                    SerializeString(str, it->first);
                    str.push_back(':');
                    str.push_back(' ');

                    const auto& obj = it->second;
                    obj.Stringify(str, indent);

                    if (i + 1 < m_stValue.m_stObject.size())
                        str.push_back(',');

                    str.push_back('\n');
                    ++i;
                }

                str.append(string((indent - 1) << 1, ' '));
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
        Json5Parser(ArrayView<char> data, JsonSaxHandler* handler, const char* source)
            : Parser(m_stReader), m_stReader(data, source), m_pHandler(handler) {}

    public:
        void DoParse()
        {
            ParseValue();
            SkipIgnorable();

            if (!m_stReader.IsEof())
                ThrowError("Bad tailing character {0}", PrintChar(m_stReader.Peek()));
        }

    private:
        void BufferUnicodeCharacter(char32_t ch)
        {
            char buffer[Encoding::UTF8::kMaxCodePointSize + 1];

            uint32_t count = 0;
            Encoding::UTF8::Encoder encoder;
            if (Encoding::EncodingResult::Accept != encoder(ch, buffer, count))
                ThrowError("Encoding {0} to utf-8 failed", (int)ch);

            buffer[count] = '\0';
            m_stStringBuffer.append(buffer);
        }

        void ReadString()
        {
            m_stStringBuffer.clear();

            char ch = m_stReader.Read();
            if (ch != '"' && ch != '\'')
                ThrowError("Unexpected character {0}", PrintChar(ch));

            char delim = ch;
            ch = m_stReader.Read();
            while (ch != '\0')
            {
                if (ch == delim)
                    return;
                else if (ch == '\\')
                {
                    ch = m_stReader.Read();

                    if (ch == 'u')
                    {
                        char32_t u32 = 0;
                        for (int i = 0; i < 4; ++i)
                        {
                            int hex = 0;

                            ch = m_stReader.Read();
                            if (ch >= '0' && ch <= '9')
                                hex = ch - '0' + 0;
                            else if (ch >= 'a' && ch <= 'f')
                                hex = ch - 'a' + 10;
                            else if (ch >= 'A' && ch <= 'F')
                                hex = ch - 'A' + 10;
                            else
                                ThrowError("Unexpected hex character {0}", PrintChar(ch));

                            u32 = (u32 << 4) + hex;
                        }

                        BufferUnicodeCharacter(u32);
                    }
                    else
                    {
                        switch (ch)
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
                            case '\n':
                                break;
                            case '\r':
                                ch = m_stReader.Peek();
                                if (ch == '\n')
                                    m_stReader.Read();
                                break;
                            default:
                                ThrowError("Unexpected escape character {0}", PrintChar(ch));
                        }
                    }
                }
                else if (ch == '\r' || ch == '\n')
                    ThrowError("Unexpected character {0}", PrintChar(ch));
                else
                    m_stStringBuffer.push_back(ch);

                ch = m_stReader.Read();
            }

            ThrowError("Unterminated string");
        }

        void ReadIdentifier()
        {
            m_stStringBuffer.clear();

            char ch = m_stReader.Read();
            if ((ch != '_' && ch != '$') && (ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z'))
                ThrowError("Bad identifier character {0}", PrintChar(ch));

            m_stStringBuffer.push_back(ch);

            ch = m_stReader.Peek();
            while (ch == '_' || ch == '$' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                (ch >= '0' && ch <= '9'))
            {
                m_stStringBuffer.push_back(ch);

                m_stReader.Read();
                ch = m_stReader.Peek();
            }
        }

        void ParseWord()
        {
            // true, false, null, Infinity, NaN
            char ch = m_stReader.Peek();
            switch (ch)
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
                    ThrowError("Unexpected character {0}", PrintChar(ch));
            }
        }

        void ParseNumber()
        {
            /*
            char sign = '+';

            char ch = m_stReader.Peek();
            if (ch == '-' || ch == '+')
            {
                sign = ch;
                m_stReader.Read();
                ch = m_stReader.Peek();
            }

            // 检查是否是Infinity或者NaN
            switch (ch)
            {
                case 'I':  // Infinity
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
                case 'N':  // NaN，不考虑-NaN
                    Accept('N');
                    Accept('a');
                    Accept('N');
                    m_pHandler->OnJsonNumber(numeric_limits<double>::quiet_NaN());
                    return;
                case '0':

            }


            var number,
                string = '',
                base = 10;

            if (ch === '0') {
                string += ch;
                next();
                if (ch === 'x' || ch === 'X') {
                    string += ch;
                    next();
                    base = 16;
                } else if (ch >= '0' && ch <= '9') {
                    error('Octal literal');
                }
            }

            switch (base) {
                case 10:
                    while (ch >= '0' && ch <= '9' ) {
                        string += ch;
                        next();
                    }
                    if (ch === '.') {
                        string += '.';
                        while (next() && ch >= '0' && ch <= '9') {
                            string += ch;
                        }
                    }
                    if (ch === 'e' || ch === 'E') {
                        string += ch;
                        next();
                        if (ch === '-' || ch === '+') {
                            string += ch;
                            next();
                        }
                        while (ch >= '0' && ch <= '9') {
                            string += ch;
                            next();
                        }
                    }
                    break;
                case 16:
                    while (ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'F' || ch >= 'a' && ch <= 'f') {
                        string += ch;
                        next();
                    }
                    break;
            }

            if(sign === '-') {
                number = -string;
            } else {
                number = +string;
            }

            if (!isFinite(number)) {
                error("Bad number");
            } else {
                return number;
            }
            */
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

            char ch = m_stReader.Peek();
            while (ch != '\0')
            {
                if (ch == ']')
                {
                    m_stReader.Read();
                    m_pHandler->OnJsonArrayEnd();
                    return;
                }

                if (ch == ',')
                    ThrowError("Missing array element");
                else
                    ParseValue();

                SkipIgnorable();

                ch = m_stReader.Peek();
                if (ch != ',')
                {
                    Accept(']');
                    m_pHandler->OnJsonArrayEnd();
                    return;
                }

                Accept(',');
                SkipIgnorable();

                ch = m_stReader.Peek();
            }

            ThrowError("Unterminated array");
        }

        void ParseObject()
        {
            Accept('{');
            SkipIgnorable();

            m_pHandler->OnJsonObjectBegin();

            char ch = m_stReader.Peek();
            while (ch != '\0')
            {
                if (ch == '}')
                {
                    m_stReader.Read();
                    m_pHandler->OnJsonObjectEnd();
                    return;
                }

                if (ch == '"' || ch == '\'')
                    ReadString();
                else
                    ReadIdentifier();

                m_pHandler->OnJsonObjectKey(m_stStringBuffer);

                SkipIgnorable();
                Accept(':');
                ParseValue();
                SkipIgnorable();

                ch = m_stReader.Peek();
                if (ch != ',')
                {
                    Accept('}');
                    m_pHandler->OnJsonObjectEnd();
                    return;
                }

                Accept(',');
                SkipIgnorable();

                ch = m_stReader.Peek();
            }

            ThrowError("Unterminated object");
        }

        void ParseComment()
        {
            Accept('/');

            char ch = m_stReader.Peek();
            if (ch == '/')  // 单行注释
            {
                do
                {
                    m_stReader.Read();
                    ch = m_stReader.Peek();

                    if (ch == '\n' || ch == '\r')
                    {
                        m_stReader.Read();
                        return;
                    }
                } while (ch != '\0');
            }
            else if (ch == '*')  // 块注释
            {
                do
                {
                    m_stReader.Read();
                    ch = m_stReader.Peek();

                    while (ch == '*')
                    {
                        m_stReader.Read();
                        ch = m_stReader.Peek();

                        if (ch == '/')
                        {
                            Accept('/');
                            return;
                        }
                    }
                } while (ch != '\0');

                ThrowError("Unterminated block comment");
            }

            ThrowError("Unexpected character {0}", PrintChar(ch));
        }

        void SkipIgnorable()
        {
            char ch = m_stReader.Peek();

            // 跳过空白符和注释
            while (ch != '\0')
            {
                if (ch == '/')
                    ParseComment();
                else
                {
                    switch (static_cast<uint8_t>(ch))
                    {
                        case ' ':
                        case '\t':
                        case '\r':
                        case '\n':
                        case '\v':
                        case '\f':
                        case 160:  // 0xA0
                            m_stReader.Read();
                            break;
                        default:
                            return;
                    }
                }

                ch = m_stReader.Peek();
            }
        }

        void ParseValue()
        {
            SkipIgnorable();

            char ch = m_stReader.Peek();
            switch (ch)
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
                    if (ch >= '0' && ch <= '9')
                        ParseNumber();
                    else
                        ParseWord();
                    break;
            }
        }

    public:
        TextReaderFromView m_stReader;
        JsonSaxHandler* m_pHandler = nullptr;
        std::string m_stStringBuffer;
    };

    class SaxHandler :
        public JsonSaxHandler
    {
        enum class State
        {

        };

    public:
        SaxHandler(JsonValue& out)
            : m_stValue(out) {}

    protected:  // implement for JsonSaxHandler
        void OnJsonNull()override
        {
        }

        void OnJsonBool(JsonValue::BoolType val)override
        {
        }

        void OnJsonNumber(JsonValue::NumberType val)override
        {
        }

        void OnJsonString(const JsonValue::StringType& val)override
        {
        }

        void OnJsonArrayBegin()override
        {
        }

        void OnJsonArrayEnd()override
        {
        }

        void OnJsonObjectBegin()override
        {
        }

        void OnJsonObjectKey(const std::string& key)override
        {
        }

        void OnJsonObjectEnd()override
        {
        }

    private:
        JsonValue& m_stValue;
    };
}

void Json5::Parse(JsonSaxHandler* handler, ArrayView<char> data, const char* source)
{
    Json5Parser parser(data, handler, source);
    parser.DoParse();
}

void Json5::Parse(JsonValue& out, ArrayView<char> data)
{

}
