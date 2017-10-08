/**
 * @file
 * @date 2017/10/6
 */
#pragma once
#include "Exception.hpp"
#include "ArrayView.hpp"

#include <vector>
#include <unordered_map>

namespace moe
{
    /**
     * @brief JSON值类型
     */
    enum class JsonValueTypes
    {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object,
    };

    /**
     * @brief JSON值
     */
    class JsonValue
    {
    public:
        using BoolType = bool;
        using NumberType = double;
        using StringType = std::string;
        using ArrayType = std::vector<JsonValue>;
        using ObjectType = std::unordered_map<std::string, JsonValue>;

        static const JsonValue kNull;

        static JsonValue MakeObject(std::initializer_list<std::pair<std::string, JsonValue>> val);

    public:
        JsonValue()noexcept;

        JsonValue(std::nullptr_t)noexcept;
        JsonValue(BoolType val)noexcept;
        JsonValue(NumberType val)noexcept;
        JsonValue(const StringType& val);
        JsonValue(const ArrayType& val);
        JsonValue(const ObjectType& val);

        JsonValue(StringType&& val)noexcept;
        JsonValue(ArrayType&& val)noexcept;
        JsonValue(ObjectType&& val)noexcept;

        JsonValue(int val)noexcept;
        JsonValue(const char* val);
        JsonValue(const ArrayView<char>& val);
        JsonValue(std::initializer_list<JsonValue> val);

        JsonValue(const JsonValue& rhs);
        JsonValue(JsonValue&& rhs)noexcept;
        ~JsonValue();

        JsonValue& operator=(const JsonValue& rhs);
        JsonValue& operator=(JsonValue&& rhs)noexcept;

        bool operator==(std::nullptr_t)const noexcept;
        bool operator==(BoolType rhs)const noexcept;
        bool operator==(NumberType rhs)const noexcept;
        bool operator==(const char* rhs)const noexcept;
        bool operator==(const StringType& rhs)const noexcept;
        bool operator==(const JsonValue& rhs)const noexcept;

        bool operator!=(std::nullptr_t)const noexcept;
        bool operator!=(BoolType rhs)const noexcept;
        bool operator!=(NumberType rhs)const noexcept;
        bool operator!=(const char* rhs)const noexcept;
        bool operator!=(const StringType& rhs)const noexcept;
        bool operator!=(const JsonValue& rhs)const noexcept;

        /**
         * @brief 转到布尔型
         *
         * 若值为bool类型的则直接返回字面量，否则null类型返回false，其他类型返回true。
         */
        operator bool()const noexcept;

    public:
        /**
         * @brief 获取值的类型
         */
        JsonValueTypes GetType()const noexcept { return m_iType; }

        /**
         * @brief 重置
         *
         * 释放内存，并将值设置为NULL。
         */
        void Reset()noexcept;

        /**
         * @brief 设置值
         * @param val 值
         */
        void Set(std::nullptr_t)noexcept;
        void Set(BoolType val)noexcept;
        void Set(NumberType val)noexcept;
        void Set(const StringType& val);
        void Set(const ArrayType& val);
        void Set(const ObjectType& val);

        void Set(StringType&& val);
        void Set(ArrayType&& val);
        void Set(ObjectType&& val);

        void Set(int val)noexcept;
        void Set(const char* val);
        void Set(const ArrayView<char>& val);
        void Set(std::initializer_list<JsonValue> val);

        /**
         * @brief 获取元素数量
         * @exception InvalidCallException 类型不匹配时抛出异常
         * @return 元素数量
         *
         * 针对ObjectType和ArrayType可用。
         */
        size_t GetElementCount()const;

        /**
         * @brief 检查是否存在元素
         * @exception InvalidCallException 类型不匹配时抛出异常
         * @param key 键值
         * @return 是否存在
         *
         * 针对ObjectType可用。
         */
        bool HasElement(const char* key)const;
        bool HasElement(const std::string& key)const;

        /**
         * @brief 通过索引获取元素
         * @exception InvalidCallException 类型不匹配时抛出异常
         * @exception OutOfRangeException 越界时抛出异常
         * @param index 索引
         *
         * 只能针对Array值进行存取。
         */
        JsonValue& GetElementByIndex(size_t index);
        const JsonValue& GetElementByIndex(size_t index)const;

        /**
         * @brief 通过键值获取元素
         * @exception InvalidCallException 类型不匹配时抛出异常
         * @param key 键值
         *
         * 只能针对Object值进行存取。
         */
        JsonValue& GetElementByKey(const std::string& key);
        const JsonValue& GetElementByKey(const std::string& key)const;

        JsonValue& GetElementByKey(const char* key);
        const JsonValue& GetElementByKey(const char* key)const;

        /**
         * @brief 追加元素
         * @param val 值
         *
         * 只针对ArrayType有效。
         */
        void Append(const JsonValue& val);
        void Append(JsonValue&& val);

        /**
         * @brief 追加元素
         * @param key 键
         * @param val 值
         *
         * 只针对ObjectType有效。
         */
        void Append(const std::string& key, const JsonValue& val);
        void Append(std::string&& key, JsonValue&& val);

        /**
         * @brief 插入元素
         * @param index 索引
         * @param val 值
         */
        void Insert(size_t index, const JsonValue& val);
        void Insert(size_t index, JsonValue&& val);

        /**
         * @brief 删除元素
         */
        bool Remove(size_t index);
        bool Remove(const char* key);
        bool Remove(const std::string& key);

        /**
         * @brief 清空元素
         *
         * 针对ArrayType和ObjectType有效。
         */
        void Clear();

        /**
         * @brief 检查是否为某类型
         * @tparam T 类型，可取值BoolType,NumberType,StringType,ArrayType,ObjectType
         */
        template <typename T>
        inline bool Is()const noexcept;

        /**
         * @brief 存取对应类型的值
         * @tparam T 类型，可取值BoolType,NumberType,StringType,ArrayType,ObjectType
         * @exception InvalidCallException 如果类型不匹配则抛出异常
         */
        template <typename T>
        inline T& Get();

        template <typename T>
        inline const T& Get()const;

        /**
         * @brief 写到字符串
         * @param str 字符串
         * @return 即str的引用
         */
        std::string& Stringify(std::string& str)const;

        /**
         * @brief 以单行模式写到字符串
         * @param str 字符串
         * @return 即str的引用
         */
        std::string& StringifyInline(std::string& str)const;

    private:
        std::string& Stringify(std::string& str, uint32_t indent)const;

    private:
        union JsonValueStorage
        {
            BoolType m_bBool;
            NumberType m_dNumber;
            StringType m_stString;
            ArrayType m_stArray;
            ObjectType m_stObject;

            JsonValueStorage() {}
            ~JsonValueStorage() {}
        };

        JsonValueTypes m_iType = JsonValueTypes::Null;
        JsonValueStorage m_stValue;
    };

    //////////////////////////////////////// <editor-fold desc="JsonValue::Is">

    template <>
    inline bool JsonValue::Is<std::nullptr_t>()const noexcept
    {
        return m_iType == JsonValueTypes::Null;
    }

    template <>
    inline bool JsonValue::Is<JsonValue::BoolType>()const noexcept
    {
        return m_iType == JsonValueTypes::Bool;
    }

    template <>
    inline bool JsonValue::Is<JsonValue::NumberType>()const noexcept
    {
        return m_iType == JsonValueTypes::Number;
    }

    template <>
    inline bool JsonValue::Is<JsonValue::StringType>()const noexcept
    {
        return m_iType == JsonValueTypes::String;
    }

    template <>
    inline bool JsonValue::Is<JsonValue::ArrayType>()const noexcept
    {
        return m_iType == JsonValueTypes::Array;
    }

    template <>
    inline bool JsonValue::Is<JsonValue::ObjectType>()const noexcept
    {
        return m_iType == JsonValueTypes::Object;
    }

    //////////////////////////////////////// </editor-fold>
    //////////////////////////////////////// <editor-fold desc="JsonValue::Get">

    template <>
    inline JsonValue::BoolType& JsonValue::Get<JsonValue::BoolType>()
    {
        if (m_iType != JsonValueTypes::Bool)
            MOE_THROW(InvalidCallException, "Bad access from {0}", m_iType);
        return m_stValue.m_bBool;
    }

    template <>
    inline JsonValue::NumberType& JsonValue::Get<JsonValue::NumberType>()
    {
        if (m_iType != JsonValueTypes::Number)
            MOE_THROW(InvalidCallException, "Bad access from {0}", m_iType);
        return m_stValue.m_dNumber;
    }

    template <>
    inline JsonValue::StringType& JsonValue::Get<JsonValue::StringType>()
    {
        if (m_iType != JsonValueTypes::String)
            MOE_THROW(InvalidCallException, "Bad access from {0}", m_iType);
        return m_stValue.m_stString;
    }

    template <>
    inline JsonValue::ArrayType& JsonValue::Get<JsonValue::ArrayType>()
    {
        if (m_iType != JsonValueTypes::Array)
            MOE_THROW(InvalidCallException, "Bad access from {0}", m_iType);
        return m_stValue.m_stArray;
    }

    template <>
    inline JsonValue::ObjectType& JsonValue::Get<JsonValue::ObjectType>()
    {
        if (m_iType != JsonValueTypes::Object)
            MOE_THROW(InvalidCallException, "Bad access from {0}", m_iType);
        return m_stValue.m_stObject;
    }

    template <>
    inline const JsonValue::BoolType& JsonValue::Get<JsonValue::BoolType>()const
    {
        if (m_iType != JsonValueTypes::Bool)
            MOE_THROW(InvalidCallException, "Bad access from {0}", m_iType);
        return m_stValue.m_bBool;
    }

    template <>
    inline const JsonValue::NumberType& JsonValue::Get<JsonValue::NumberType>()const
    {
        if (m_iType != JsonValueTypes::Number)
            MOE_THROW(InvalidCallException, "Bad access from {0}", m_iType);
        return m_stValue.m_dNumber;
    }

    template <>
    inline const JsonValue::StringType& JsonValue::Get<JsonValue::StringType>()const
    {
        if (m_iType != JsonValueTypes::String)
            MOE_THROW(InvalidCallException, "Bad access from {0}", m_iType);
        return m_stValue.m_stString;
    }

    template <>
    inline const JsonValue::ArrayType& JsonValue::Get<JsonValue::ArrayType>()const
    {
        if (m_iType != JsonValueTypes::Array)
            MOE_THROW(InvalidCallException, "Bad access from {0}", m_iType);
        return m_stValue.m_stArray;
    }

    template <>
    inline const JsonValue::ObjectType& JsonValue::Get<JsonValue::ObjectType>()const
    {
        if (m_iType != JsonValueTypes::Object)
            MOE_THROW(InvalidCallException, "Bad access from {0}", m_iType);
        return m_stValue.m_stObject;
    }

    //////////////////////////////////////// </editor-fold>

    class JsonSaxHandler
    {
    public:
        virtual void OnJsonNull() = 0;
        virtual void OnJsonBool(JsonValue::BoolType val) = 0;
        virtual void OnJsonNumber(JsonValue::NumberType val) = 0;
        virtual void OnJsonString(const JsonValue::StringType& val) = 0;
        virtual void OnJsonArrayBegin() = 0;
        virtual void OnJsonArrayEnd() = 0;
        virtual void OnJsonObjectBegin() = 0;
        virtual void OnJsonObjectKey(const std::string& key) = 0;
        virtual void OnJsonObjectEnd() = 0;
    };

    /**
     * @brief JSON5扩展语法支持
     * @see https://github.com/json5/json5
     */
    class Json5
    {
    public:
        /**
         * @brief 解析Json5
         * @param handler 解析句柄
         * @param data 数据
         */
        static void Parse(JsonSaxHandler* handler, ArrayView<char> data, const char* source="Unknown");

        /**
         * @brief 解析Json5
         * @param out 目标Json对象
         * @param data 数据
         */
        static void Parse(JsonValue& out, ArrayView<char> data);
    };
}
