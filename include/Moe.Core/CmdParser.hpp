/**
 * @file
 * @date 2017/12/17
 */
#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

#include "Any.hpp"
#include "ArrayView.hpp"
#include "Exception.hpp"
#include "Convert.hpp"

namespace moe
{
    /**
     * @brief 命令行解析失败时抛出
     */
    MOE_DEFINE_EXCEPTION(CmdlineParseException);

    class CmdParser
    {
    public:
        enum class OptionReadResult
        {
            Terminated,
            ParseError,
            NeedMore,
            MoreOrEmpty,
        };

        using OptionReadingStartFunc = OptionReadResult(*)(Any&, const Any&);  // 第二个参数指定默认值，通常只对bool类型有意义
        using OptionReadingFunc = OptionReadResult(*)(Any&, ArrayView<char>);
        using OptionSettingFunc = void(*)(Any&, const Any&);  // 将第二个参数值（T）设置到参数一上（T*）

        template <typename T, typename P = void>
        struct OptionReader;

        template <typename T>
        using TestForceOptional = typename std::conditional<std::is_same<typename std::decay<T>::type, bool>::value,
            std::true_type, std::false_type>::type;

        struct Option
        {
            friend class CmdParser;

        public:
            Option() = default;

            /**
             * @brief 构造一个选项
             * @tparam T 目标变量类型
             * @param[out] out 解析存储变量
             * @param option 选项（长版本），在命令行中以'--'开始，不能是非ASCII字符
             * @param shortOpt 选项（单字符版本），在命令行中以'-'开始，不能是非ASCII字符
             * @param description 描述
             * @param defaultVal 默认值
             *
             * 如果T==bool，则选项总是非必须的。
             */
            template <typename T>
            Option(T& out, const char* option, const char* description)
                : Target(&out), LongOption(option), Description(description), Required(!TestForceOptional<T>::value),
                  ArgumentCount(OptionReader<T>::kArgumentCount), OnStart(OptionReader<T>::OnStart),
                  OnReadArg(OptionReader<T>::OnReadArg), OnSetDefault(OptionReader<T>::OnSetDefault) {}
            template <typename T>
            Option(T& out, const char* option, const char* description, T defaultVal)
                : Target(&out), LongOption(option), Description(description), DefaultValue(defaultVal),
                  ArgumentCount(OptionReader<T>::kArgumentCount), OnStart(OptionReader<T>::OnStart),
                  OnReadArg(OptionReader<T>::OnReadArg), OnSetDefault(OptionReader<T>::OnSetDefault) {}
            template <typename T>
            Option(T& out, const char* option, char shortOpt, const char* description)
                : Target(&out), LongOption(option), ShortOption(shortOpt), Description(description),
                  Required(!TestForceOptional<T>::value), ArgumentCount(OptionReader<T>::kArgumentCount),
                  OnStart(OptionReader<T>::OnStart), OnReadArg(OptionReader<T>::OnReadArg),
                  OnSetDefault(OptionReader<T>::OnSetDefault) {}
            template <typename T>
            Option(T& out, const char* option, char shortOpt, const char* description, T defaultVal)
                : Target(&out), LongOption(option), ShortOption(shortOpt), Description(description),
                  DefaultValue(defaultVal), ArgumentCount(OptionReader<T>::kArgumentCount),
                  OnStart(OptionReader<T>::OnStart), OnReadArg(OptionReader<T>::OnReadArg),
                  OnSetDefault(OptionReader<T>::OnSetDefault) {}

        public:
            bool Valid()const noexcept
            {
                return !Target.IsEmpty() && LongOption && Description && OnStart && OnReadArg && OnSetDefault;
            }

        private:
            Any Target;
            const char* LongOption = nullptr;
            char ShortOption = '\0';
            const char* Description = nullptr;
            bool Required = false;
            Any DefaultValue;
            int ArgumentCount = 0;
            bool Set = false;

            OptionReadingStartFunc OnStart = nullptr;
            OptionReadingFunc OnReadArg = nullptr;
            OptionSettingFunc OnSetDefault = nullptr;
        };

    public:
        CmdParser() = default;

        /**
         * @brief 声明一个选项
         * @param opt 选项描述
         * @exception BadArgumentException 如果传入一个无效的选项则抛出异常
         * @exception
         */
        CmdParser& operator<<(Option&& opt);

        /**
         * @brief 解析命令行
         * @param[out] nonOptions 非选项命令行组成
         * @param argc 命令行参数个数，必然大于1
         * @param argv 命令行参数值，忽略argv[0]
         * @return 解析个数
         *
         * 规则：
         *  --指示长选项，后可接等号或者空格分割值，如"--host=127.0.0.1"或者"--host 127.0.0.1"
         *  -指示短选项，后可接空格分割值，如"-h 127.0.0.1"
         *  在解析过程中不被接受的项目被放入nonOptions中等待后续处理。
         *  如果遇到"--"，则后续的命令行将会直接被放弃解析放入nonOptions。
         */
        size_t operator()(uint32_t argc, const char* argv[]) { return Parse(argc, argv, nullptr); }
        size_t operator()(std::vector<std::string>& nonOptions, uint32_t argc, const char* argv[])
        {
            return Parse(argc, argv, &nonOptions);
        }

    public:
        /**
         * @brief 返回选项个数
         */
        size_t GetCount()const noexcept { return m_stOptions.size(); }

        /**
         * @brief 检查是否存在选项
         * @param option 选项长名称
         * @return 是否存在选项
         */
        bool Contains(const std::string& option)noexcept;

        /**
         * @brief 移除选项
         * @param option 选项长名称
         * @return 是否成功移除
         */
        bool Remove(const std::string& option)noexcept;

        /**
         * @brief 清空所有选项
         */
        void Clear()noexcept;

        /**
         * @brief 构建使用文本
         *
         * 例：
         *   Usage: $name --file <...> [OPTIONS] -- ...
         */
        std::string BuildUsageText(const char* name, const char* nonOptionsHint="-- ...")const;

        /**
         * @brief 构建选项文本
         * @param leftPadding 左边的空格数
         * @param centerMargin 中间的空格数
         *
         * 例：
         *   --help, -h <...>  Show this text.
         *   --host, -o <...>  Specific the host address.
         *   --port, -p <...>  Specific the port.
         */
        std::string BuildOptionsText(uint32_t leftPadding=2, uint32_t centerMargin=4)const;

    private:
        /**
         * @brief 解析命令行
         * @exception CmdlineParseException 命令行解析失败时抛出
         * @param argc 参数个数
         * @param argv 参数值列表
         * @param[in,out] nonOptions 非选项结果
         * @return 解析个数
         */
        size_t Parse(uint32_t argc, const char* argv[], std::vector<std::string>* nonOptions);

    private:
        std::vector<Option> m_stOptions;
        std::unordered_map<std::string, size_t> m_stOptionTable;
        std::unordered_map<char, size_t> m_stShortOptTable;
    };

    template <>
    struct CmdParser::OptionReader<bool>
    {
        static const int kArgumentCount = 0;

        static OptionReadResult OnStart(Any& target, const Any&)
        {
            auto p = target.CastTo<bool*>();
            *p = true;
            return OptionReadResult::Terminated;
        }

        static OptionReadResult OnReadArg(Any&, ArrayView<char>)
        {
            return OptionReadResult::ParseError;
        }

        static void OnSetDefault(Any& target, const Any& defaultVal)
        {
            auto p = target.CastTo<bool*>();
            if (defaultVal)
                *p = defaultVal.CastTo<bool>();
            else
                *p = false;
        }
    };

    template <>
    struct CmdParser::OptionReader<std::string>
    {
        static const int kArgumentCount = 1;

        static OptionReadResult OnStart(Any&, const Any&)
        {
            return OptionReadResult::NeedMore;
        }

        static OptionReadResult OnReadArg(Any& target, ArrayView<char> arg)
        {
            auto p = target.CastTo<std::string*>();
            *p = std::string(arg.GetBuffer(), arg.GetSize());
            return OptionReadResult::Terminated;
        }

        static void OnSetDefault(Any& target, const Any& defaultVal)
        {
            auto p = target.CastTo<std::string*>();
            if (defaultVal)
                *p = defaultVal.CastTo<std::string>();
            else
                p->clear();
        }
    };

    template <typename P>
    struct CmdParser::OptionReader<P, typename std::enable_if<std::is_signed<P>::value, void>::type>
    {
        static const int kArgumentCount = 1;

        static OptionReadResult OnStart(Any&, const Any&)
        {
            return OptionReadResult::NeedMore;
        }

        static OptionReadResult OnReadArg(Any& target, ArrayView<char> arg)
        {
            auto p = target.CastTo<P*>();

            size_t processed = 0;
            auto result = Convert::ParseInt(arg.GetBuffer(), arg.GetSize(), processed);
            if (processed == 0 || result < std::numeric_limits<P>::min() || result > std::numeric_limits<P>::max())
                return OptionReadResult::ParseError;

            *p = static_cast<P>(result);
            return OptionReadResult::Terminated;
        }

        static void OnSetDefault(Any& target, const Any& defaultVal)
        {
            auto p = target.CastTo<P*>();
            if (defaultVal)
                *p = defaultVal.CastTo<P>();
            else
                *p = P();
        }
    };

    template <typename P>
    struct CmdParser::OptionReader<P, typename std::enable_if<std::is_unsigned<P>::value, void>::type>
    {
        static const int kArgumentCount = 1;

        static OptionReadResult OnStart(Any&, const Any&)
        {
            return OptionReadResult::NeedMore;
        }

        static OptionReadResult OnReadArg(Any& target, ArrayView<char> arg)
        {
            auto p = target.CastTo<P*>();

            size_t processed = 0;
            auto result = Convert::ParseUInt(arg.GetBuffer(), arg.GetSize(), processed);
            if (processed == 0 || result > std::numeric_limits<P>::max())
                return OptionReadResult::ParseError;

            *p = static_cast<P>(result);
            return OptionReadResult::Terminated;
        }

        static void OnSetDefault(Any& target, const Any& defaultVal)
        {
            auto p = target.CastTo<P*>();
            *p = defaultVal.CastTo<P>();
        }
    };

    template <typename P>
    struct CmdParser::OptionReader<P, typename std::enable_if<std::is_floating_point<P>::value, void>::type>
    {
        static const int kArgumentCount = 1;

        static OptionReadResult OnStart(Any&, const Any&)
        {
            return OptionReadResult::NeedMore;
        }

        static OptionReadResult OnReadArg(Any& target, ArrayView<char> arg)
        {
            auto p = target.CastTo<P*>();

            size_t processed = 0;
            auto result = Convert::ParseDouble(arg.GetBuffer(), arg.GetSize(), processed);
            if (processed == 0)
                return OptionReadResult::ParseError;

            *p = static_cast<P>(result);
            return OptionReadResult::Terminated;
        }

        static void OnSetDefault(Any& target, const Any& defaultVal)
        {
            auto p = target.CastTo<P*>();
            if (defaultVal)
                *p = defaultVal.CastTo<P>();
            else
                *p = P();
        }
    };

    template <typename P>
    struct CmdParser::OptionReader<std::vector<P>>
    {
        static const int kArgumentCount = -1;

        static OptionReadResult OnStart(Any&, const Any&)
        {
            return OptionReadResult::MoreOrEmpty;
        }

        static OptionReadResult OnReadArg(Any& target, ArrayView<char> arg)
        {
            auto p = target.CastTo<std::vector<P>*>();

            P storage;
            Any storageAny(&storage);
            auto result = OptionReader<P>::OnReadArg(storageAny, arg);
            if (result != OptionReadResult::Terminated)  // 只有P=vector<T>时才会出现Continue，不允许这么做
                return OptionReadResult::ParseError;

            p->emplace_back(std::move(storage));
            return OptionReadResult::MoreOrEmpty;
        }

        static void OnSetDefault(Any& target, const Any& defaultVal)
        {
            auto p = target.CastTo<P*>();
            if (defaultVal)
                *p = defaultVal.CastTo<P>();
            else
                *p = P();
        }
    };
}
