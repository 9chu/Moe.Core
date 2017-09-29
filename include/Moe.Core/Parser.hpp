/**
 * @file
 * @date 2017/9/20
 */
#pragma once
#include "TextReader.hpp"

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
        static std::string PrintChar(int ch)
        {
            if (ch == -1)
                return "<EOF>";
        }

    public:
        Parser(TextReader& reader)
            : m_stReader(reader) {}

    public:
        /**
         * @brief 获取读取器
         */
        const TextReader& GetReader()const noexcept { return m_stReader; }

        /**
         * @brief 获取行号
         */
        uint32_t GetLine()const noexcept { return m_uLine; }

        /**
         * @brief 获取列号
         */
        uint32_t GetColumn()const noexcept { return m_uColumn; }

    protected:
        /**
         * @brief 匹配并接受一个字符
         * @exception LexicalException 不能匹配时抛出词法异常
         * @param c 被匹配字符
         */
        void Accept(int c)
        {
            int ch = m_stReader.Read();
        }

        /**
         * @brief 尝试匹配一个字符
         * @param c 被匹配字符
         * @return 是否匹配，若能匹配则提升一个位置
         */
        bool TryAccept(int c);

        template <char... Char>
        void Skip();

    private:
        TextReader& m_stReader;
        uint32_t m_uLine = 1;
        uint32_t m_uColumn = 0;
    };
}
