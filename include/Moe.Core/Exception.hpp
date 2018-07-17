/**
 * @file
 * @date 2017/5/28
 */
#pragma once
#include <stdexcept>
#include <unordered_map>

#include "Any.hpp"
#include "RefPtr.hpp"
#include "StringUtils.hpp"

namespace moe
{
    /**
     * @brief 异常基类
     */
    class Exception :
        public std::exception
    {
        struct InternalStorage :
            public RefBase<InternalStorage>
        {
            const char* SourceFile = nullptr;
            const char* FunctionName = nullptr;
            uint32_t LineNumber = 0;
            std::string Desc;

            std::string FullDescCache;

            std::unordered_map<std::string, Any> Info;
        };

    public:
        Exception()
        {
            m_pStorage = MakeRef<InternalStorage>();
        }
        Exception(const Exception& rhs)noexcept  // 拷贝构造不能抛出异常
            : m_pStorage(rhs.m_pStorage) {}

        Exception(Exception&& rhs) = delete;
        ~Exception() = default;

    public:
        Exception& operator=(const Exception& rhs)noexcept
        {
            m_pStorage = rhs.m_pStorage;
            return *this;
        }
        Exception& operator=(Exception&&) = delete;

    public:
        /**
         * @brief 获取异常抛出点的源文件
         */
        const char* GetSourceFile()const noexcept
        {
            assert(m_pStorage);
            return m_pStorage->SourceFile;
        }

        /**
         * @brief 设置抛出点的源文件
         * @param[in] filename 源文件，必须是常量
         */
        Exception& SetSourceFile(const char* filename)noexcept
        {
            assert(m_pStorage);
            m_pStorage->SourceFile = filename;
            m_pStorage->FullDescCache.clear();
            return *this;
        }

        /**
         * @brief 获取异常抛出点的函数名
         */
        const char* GetFunctionName()const noexcept
        {
            assert(m_pStorage);
            return m_pStorage->FunctionName;
        }

        /**
         * @brief 设置抛出点的函数名
         * @param[in] name 函数名，必须是常量
         */
        Exception& SetFunctionName(const char* name)noexcept
        {
            assert(m_pStorage);
            m_pStorage->FunctionName = name;
            m_pStorage->FullDescCache.clear();
            return *this;
        }

        /**
         * @brief 获取异常抛出点的行号
         */
        uint32_t GetLineNumber()const noexcept
        {
            assert(m_pStorage);
            return m_pStorage->LineNumber;
        }

        /**
         * @brief 设置抛出点的行号
         * @param[in] line 行号
         */
        Exception& SetLineNumber(uint32_t line)noexcept
        {
            assert(m_pStorage);
            m_pStorage->LineNumber = line;
            m_pStorage->FullDescCache.clear();
            return *this;
        }

        /**
         * @brief 获取异常的描述
         */
        const std::string& GetDescription()const noexcept
        {
            assert(m_pStorage);
            return m_pStorage->Desc;
        }

        /**
         * @brief 设置异常描述
         * @param[in] str 描述字符串
         */
        Exception& SetDescription(const std::string& str)
        {
            assert(m_pStorage);
            m_pStorage->Desc = str;
            m_pStorage->FullDescCache.clear();
            return *this;
        }

        /**
         * @brief 设置异常描述
         * @param[in] str 描述字符串
         */
        Exception& SetDescription(std::string&& str)noexcept
        {
            assert(m_pStorage);
            m_pStorage->Desc = std::move(str);
            m_pStorage->FullDescCache.clear();
            return *this;
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
            assert(m_pStorage);
            auto it = m_pStorage->Info.find(key);
            if (it == m_pStorage->Info.end())
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
        Exception& SetInfo(const std::string& key, T&& value)
        {
            assert(m_pStorage);
            m_pStorage->Info[key] = Any(std::forward<T&&>(value));
            return *this;
        }

        /**
         * @brief 转换到字符串
         */
        virtual const std::string& ToString()const;

    public: // implement for std::exception
        const char* what()const noexcept override;  // 等价于ToString

    private:
        RefPtr<InternalStorage> m_pStorage;
    };
}

/**
 * @brief 抛出一个异常
 * @param Except 异常类型，必须继承自moe::Exception
 * @param ... 提示字符串，使用C-Style格式化文本
 */
#define MOE_THROW(Except, ...) \
    do { \
        throw Except() \
            .SetSourceFile(__FILE__) \
            .SetFunctionName(__FUNCTION__) \
            .SetLineNumber(__LINE__) \
            .SetDescription(moe::StringUtils::Format(__VA_ARGS__)); \
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
    MOE_DEFINE_EXCEPTION(InvalidEncodingException);

    /**
     * @brief 无效格式
     *
     * 通常用于指示文件/参数等格式的错误。
     */
    MOE_DEFINE_EXCEPTION(BadFormatException);

    /**
     * @brief API错误
     *
     * 通常用来指示平台或者第三方组件产生的异常。
     */
    MOE_DEFINE_EXCEPTION(ApiException);

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
    MOE_DEFINE_EXCEPTION(OperationNotSupportException);

    /**
     * @brief 操作被取消异常
     */
    MOE_DEFINE_EXCEPTION(OperationCancelledException);

    /**
     * @brief 错误的状态
     */
    MOE_DEFINE_EXCEPTION(BadStateException);

    /**
     * @brief 尚未实现
     */
    MOE_DEFINE_EXCEPTION(NotImplementException);
}

//////////////////////////////////////// </editor-fold>
