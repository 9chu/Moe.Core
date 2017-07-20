/**
 * @file
 * @date 2017/5/28
 */
#pragma once
#include <stdexcept>
#include <unordered_map>

#include "Any.hpp"
#include "StringUtils.hpp"

namespace moe
{
    /**
     * @brief 异常基类
     */
    class Exception :
        public std::exception
    {
    public:
        Exception() = default;
        Exception(const Exception& rhs) = default;
        Exception(Exception&& rhs)noexcept = default;
        ~Exception() = default;

    public:
        Exception& operator=(const Exception& rhs) = default;
        Exception& operator=(Exception&& rhs)noexcept = default;

    public:
        /**
         * @brief 获取异常抛出点的源文件
         */
        const char* GetSourceFile()const noexcept { return m_pszSourceFile; }

        /**
         * @brief 设置抛出点的源文件
         * @param[in] filename 源文件，必须是常量
         */
        void SetSourceFile(const char* filename)noexcept
        {
            m_pszSourceFile = filename;
            m_strFullDescCache.clear();
        }

        /**
         * @brief 获取异常抛出点的函数名
         */
        const char* GetFunctionName()const noexcept { return m_pszFunctionName; }

        /**
         * @brief 设置抛出点的函数名
         * @param[in] name 函数名，必须是常量
         */
        void SetFunctionName(const char* name)noexcept
        {
            m_pszFunctionName = name;
            m_strFullDescCache.clear();
        }

        /**
         * @brief 获取异常抛出点的行号
         */
        uint32_t GetLineNumber()const noexcept { return m_iLineNumber; }

        /**
         * @brief 设置抛出点的行号
         * @param[in] line 行号
         */
        void SetLineNumber(uint32_t line)noexcept
        {
            m_iLineNumber = line;
            m_strFullDescCache.clear();
        }

        /**
         * @brief 获取异常的描述
         */
        const std::string& GetDescription()const noexcept { return m_strDesc; }

        /**
         * @brief 设置异常描述
         * @param[in] str 描述字符串
         */
        void SetDescription(const std::string& str)
        {
            m_strDesc = str;
            m_strFullDescCache.clear();
        }

        /**
         * @brief 设置异常描述
         * @param[in] str 描述字符串
         */
        void SetDescription(std::string&& str)noexcept
        {
            m_strDesc = std::move(str);
            m_strFullDescCache.clear();
        }

        /**
         * @brief 获取携带的额外信息
         * @tparam T 类型
         * @param key 键值
         * @return 携带的数据
         *
         * 如果不存在额外的信息或者类型不匹配，则返回一个空的对象。
         */
        template <typename T>
        T GetInfo(const std::string& key)const noexcept
        {
            auto it = m_mapInfo.find(key);
            if (it == m_mapInfo.end())
                return T();
            return it->second.SafeCastTo<T>();
        }

        /**
         * @brief 设置携带的额外信息
         * @tparam T 类型
         * @param key 键值
         * @param value 值
         */
        template <typename T>
        void SetInfo(const std::string& key, T&& value)
        {
            m_mapInfo[key] = Any(std::forward<T&&>(value));
        }

        /**
         * @brief 转换到字符串
         */
        virtual const std::string& ToString()const;

    public: // implement for std::exception
        const char* what()const noexcept override;  // 等价于ToString

    private:
        const char* m_pszSourceFile = nullptr;
        const char* m_pszFunctionName = nullptr;
        uint32_t m_iLineNumber = 0;
        std::string m_strDesc;

        mutable std::string m_strFullDescCache;

        std::unordered_map<std::string, Any> m_mapInfo;
    };
}

/**
 * @brief 抛出一个异常
 * @param Except 异常类型，必须继承自moe::Exception
 * @param ... 提示字符串，使用C-Style格式化文本
 */
#define MOE_THROW(Except, ...) \
    do { \
        Except ex; \
        ex.SetSourceFile(__FILE__); \
        ex.SetFunctionName(__FUNCTION__); \
        ex.SetLineNumber(__LINE__); \
        ex.SetDescription(moe::StringUtils::Format(__VA_ARGS__)); \
        throw ex; \
    } while (false)

/**
 * @brief 定义一个异常
 * @param Name 异常名称
 */
#define MOE_DEFINE_EXCEPTION(Name) \
    class Name : \
        public Exception \
    { \
    public: \
        using Exception::Exception; \
    }

//////////////////////////////////////// <editor-fold desc="预定义异常">

namespace moe
{
    /**
     * @brief 非法调用异常
     */
    MOE_DEFINE_EXCEPTION(InvalidCallException);

    /**
     * @brief 无效参数异常
     */
    MOE_DEFINE_EXCEPTION(BadArgumentException);

    /**
     * @brief 越界异常
     */
    MOE_DEFINE_EXCEPTION(OutOfRangeException);

    /**
     * @brief 非法编码
     *
     * 通常用于指示字符串编码错误。
     */
    MOE_DEFINE_EXCEPTION(InvalidEncoding);

    /**
     * @brief 无效格式
     *
     * 通常用于指示文件/参数等格式的错误。
     */
    MOE_DEFINE_EXCEPTION(BadFormat);

    /**
     * @brief API错误
     *
     * 通常用来指示平台或者第三方组件产生的异常。
     */
    MOE_DEFINE_EXCEPTION(APIException);

    /**
     * @brief IO错误
     *
     * 区别于APIException，通常明确指示特定的IO异常。
     */
    MOE_DEFINE_EXCEPTION(IOException);

    /**
     * @brief 对象或者键值已存在错误
     */
    MOE_DEFINE_EXCEPTION(ObjectExistsException);

    /**
     * @brief 对象或者键值不存在错误
     */
    MOE_DEFINE_EXCEPTION(ObjectNotFoundException);

    /**
     * @brief 不支持的操作
     */
    MOE_DEFINE_EXCEPTION(OperationNotSupport);

    /**
     * @brief 错误的状态
     */
    MOE_DEFINE_EXCEPTION(BadStateException);
}

//////////////////////////////////////// </editor-fold>
