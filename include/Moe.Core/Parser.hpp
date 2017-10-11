/**
 * @file
 * @date 2017/9/20
 */
#pragma once
#include <cassert>

#include "Optional.hpp"
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
        /**
         * @brief 打印一个Char为可读形式
         * @param ch 被打印字符
         * @return 可读表示
         */
        static std::string PrintChar(char ch);

    public:
        Parser() = default;

    public:
        /**
         * @brief 获取当前的读取器
         */
        Optional<TextReader&> GetReader()noexcept { return m_pReader; }

        /**
         * @brief 执行解析过程
         * @param reader 读取器
         *
         * 需要子类覆盖实现，注意到实现时必须先调用Parser::Run来初始化内部状态。
         */
        virtual void Run(TextReader& reader);

    protected:
        template <typename... Args>
        void ThrowError(const char* format, const Args&... args)
        {
            LexicalException ex;
            ex.SetSourceFile(__FILE__);
            ex.SetFunctionName(__FUNCTION__);
            ex.SetLineNumber(__LINE__);

            if (m_pReader)
            {
                ex.SetDescription(moe::StringUtils::Format("{0}:{1}:{2}:{3}: {4}", m_pReader->GetSourceName(),
                    m_pReader->GetPosition(), m_pReader->GetLine(), m_pReader->GetColumn(),
                    StringUtils::Format(format, args...)));

                // 额外数据
                ex.SetInfo("SourceName", m_pReader->GetSourceName());
                ex.SetInfo("Position", m_pReader->GetPosition());
                ex.SetInfo("Line", m_pReader->GetLine());
                ex.SetInfo("Column", m_pReader->GetColumn());
            }
            else
                ex.SetDescription(StringUtils::Format(format, args...));

            throw ex;
        }

        /**
         * @brief 继续读取下一个字符
         * @return 返回当前的字符
         */
        char Next()
        {
            assert(m_pReader);

            char ch = m_pReader->Read();
            assert(ch == c);

            c = m_pReader->Peek();
            return ch;
        }

        /**
         * @brief 回退一个字符
         */
        void Back()
        {
            assert(m_pReader);

            m_pReader->Back();
            c = m_pReader->Peek();
        }

        /**
         * @brief 尝试匹配一个字符
         * @param ch 被匹配字符
         * @return 是否匹配，若能匹配则提升一个位置
         */
        bool TryAcceptOne(char ch)
        {
            if (ch == c)
            {
                Next();
                return true;
            }
            return false;
        }

        /**
         * @brief 尝试匹配一系列字符
         * @param[out] accept 匹配的字符
         * @param ch 试图匹配的字符
         * @return 若成功返回true并提升一个位置
         */
        bool TryAccept(char& accept, char ch)
        {
            if (TryAcceptOne(ch))
            {
                accept = ch;
                return true;
            }
            accept = '\0';
            return false;
        }

        template <typename... Args>
        typename std::enable_if<sizeof...(Args) != 0, bool>::type TryAccept(char& accept, char ch, Args... args)
        {
            if (TryAccept(accept, ch))
                return true;
            return TryAccept(accept, args...);
        }

        /**
         * @brief 匹配并接受一个字符
         * @exception LexicalException 不能匹配时抛出词法异常
         * @param ch 被匹配字符
         */
        void AcceptOne(char ch)
        {
            if (ch != c)
                ThrowError("Expect {0}, but found {1}", PrintChar(ch), PrintChar(c));
            Next();
        }

        /**
         * @brief 匹配一系列字符
         * @exception LexicalException 不能匹配时抛出词法异常
         * @param ch 被匹配的字符
         * @return 匹配的字符，即等于ch
         */
        char Accept(char ch)
        {
            if (ch != c)
                ThrowError("Expect {0}, but found {1}", PrintChar(ch), PrintChar(c));
            Next();
            return ch;
        }

        template <typename... Args>
        typename std::enable_if<sizeof...(Args) != 0, char>::type Accept(char ch, Args... args)
        {
            if (TryAcceptOne(ch))
                return ch;
            return Accept(args...);
        }

    protected:
        char c = '\0';  // 当前向前看且尚未读取的字符，等同于m_stReader.Peek();

    private:
        Optional<TextReader&> m_pReader;
    };
}
