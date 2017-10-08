/**
 * @file
 * @date 2017/9/20
 */
#pragma once
#include <cassert>

#include "TextReader.hpp"
#include "StringUtils.hpp"

namespace moe
{
    /**
     * @brief 词法错误异常
     */
    MOE_DEFINE_EXCEPTION(LexicalException);

    /**
     * @brief 解析器基类
     *
     * 提供了用于手写递归下降解析器的一些基础函数。
     */
    class Parser
    {
    public:
        static std::string PrintChar(char ch)
        {
            if (ch == '\0')
                return "<EOF>";
            else if (ch == '\'')
                return "'\''";
            else if (ch > 0 && ::isprint(ch) != 0)
                return StringUtils::Format("'{0}'", ch);
            return StringUtils::Format("<{0}>", static_cast<uint8_t>(ch));
        }

    public:
        Parser(TextReader& reader)
            : m_stReader(reader) {}

    public:
        /**
         * @brief 获取读取器
         */
        const TextReader& GetReader()const noexcept { return m_stReader; }

    protected:
        template <typename... Args>
        void ThrowError(const char* format, const Args&... args)
        {
            LexicalException ex;
            ex.SetSourceFile(__FILE__);
            ex.SetFunctionName(__FUNCTION__);
            ex.SetLineNumber(__LINE__);
            ex.SetDescription(moe::StringUtils::Format("{0}:{1}:{2}:{3}: {4}", m_stReader.GetSourceName(),
                m_stReader.GetPosition(), m_stReader.GetLine(), m_stReader.GetColumn(),
                StringUtils::Format(format, args...)));

            // 额外数据
            ex.SetInfo("SourceName", m_stReader.GetSourceName());
            ex.SetInfo("Position", m_stReader.GetPosition());
            ex.SetInfo("Line", m_stReader.GetLine());
            ex.SetInfo("Column", m_stReader.GetColumn());
            throw ex;
        }

        /**
         * @brief 匹配并接受一个字符
         * @exception LexicalException 不能匹配时抛出词法异常
         * @param c 被匹配字符
         */
        void Accept(char c)
        {
            char ch = m_stReader.Read();
            if (ch != c)
                ThrowError("Expect {0}, but found {1}", PrintChar(c), PrintChar(ch));
        }

        /**
         * @brief 尝试匹配一个字符
         * @param c 被匹配字符
         * @return 是否匹配，若能匹配则提升一个位置
         */
        bool TryAccept(char c)
        {
            char ch = m_stReader.Peek();
            if (ch == c)
            {
                char cch = m_stReader.Read();
                assert(cch == ch);
                return true;
            }
            return false;
        }

    protected:
        TextReader& m_stReader;
    };
}
