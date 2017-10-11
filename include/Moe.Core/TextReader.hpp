/**
 * @file
 * @date 2017/9/20
 */
#pragma once
#include <vector>

#include "ArrayView.hpp"
#include "Exception.hpp"

namespace moe
{
    /**
     * @brief 文本读取器
     *
     * 文本读取器用于在一个缓冲区中进行逐字符的读取和回退操作，通常被解析器使用。
     * 约定：
     *  - 使用UTF-8编码
     *  - 不约束行尾类型
     *  - 不持有字符串的内存
     */
    class TextReader
    {
    public:
        TextReader();
        TextReader(ArrayView<char> input, const char* sourceName="Unknown");
        TextReader(const std::string& input, const char* sourceName="Unknown");
        TextReader(const TextReader& rhs);

    public:
        /**
         * @brief 获取源名称
         */
        const char* GetSourceName()const noexcept { return m_stSourceName.c_str(); }

        /**
         * @brief 获取总长度
         * @return 总长度
         */
        size_t GetLength()const noexcept { return m_stBuffer.GetSize(); }

        /**
         * @brief 获取当前的读取位置
         *
         * 指示下一个将要读取的字符的位置。
         */
        size_t GetPosition()const noexcept { return m_uPosition; }

        /**
         * @brief 获取当前行号
         *
         * 行号从1开始。
         */
        uint32_t GetLine()const noexcept { return m_uLine; }

        /**
         * @brief 获取当前列号
         *
         * 指示下一个读取位置的列号。
         */
        uint32_t GetColumn()const noexcept { return m_uColumn; }

        /**
         * @brief 判断是否达到了结尾
         */
        bool IsEof()const noexcept { return m_uPosition >= m_stBuffer.GetSize(); }

        /**
         * @brief 读取一个字符，并提升读取的位置
         * @return 若遇到EOF则返回'\0'，否则返回读取的字符
         */
        char Read()noexcept
        {
            if (IsEof())
                return '\0';

            char ch = m_stBuffer[m_uPosition];
            ++m_uPosition;
            ++m_uColumn;

            if ((ch == '\r' && Peek() != '\n') || ch == '\n')
            {
                ++m_uLine;
                m_uColumn = 1;
            }

            return ch;
        }

        /**
         * @brief 读取一个字符
         * @return 若遇到EOF则返回'\0'，否则返回读取的字符
         */
        char Peek()noexcept
        {
            if (IsEof())
                return '\0';
            return m_stBuffer[m_uPosition];
        }

        /**
         * @brief 回退一个字符
         * @exception OutOfRangeException 回退越界时抛出异常
         *
         * Back的代价较大，应当避免经常调用。
         */
        void Back();

    private:
        ArrayView<char> m_stBuffer;
        std::string m_stSourceName;

        size_t m_uPosition = 0;
        uint32_t m_uLine = 1;
        uint32_t m_uColumn = 1;
    };
}
