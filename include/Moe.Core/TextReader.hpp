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
     * 文本读取器定义了针对文本读取的一般接口。
     * 这一接口约定了字符编码为UTF-8，但对行尾符没有约束。
     */
    class TextReader
    {
    public:
        TextReader() = default;
        virtual ~TextReader() = default;

    public:
        /**
         * @brief 获取源名称
         */
        virtual const char* GetSourceName()const noexcept = 0;

        /**
         * @brief 获取总长度
         * @return 总长度
         */
        virtual size_t GetLength()const noexcept = 0;

        /**
         * @brief 获取当前的读取位置
         *
         * 指示下一个将要读取的字符的位置。
         */
        virtual size_t GetPosition()const noexcept = 0;

        /**
         * @brief 获取当前行号
         *
         * 行号从1开始。
         */
        virtual uint32_t GetLine()const noexcept = 0;

        /**
         * @brief 获取当前列号
         *
         * 指示下一个读取位置的列号。
         */
        virtual uint32_t GetColumn()const noexcept = 0;

        /**
         * @brief 判断是否达到了结尾
         */
        virtual bool IsEof()const noexcept = 0;

        /**
         * @brief 读取一个字符，并提升读取的位置
         * @exception IOException 遇到IO错误时抛出异常
         * @return 若遇到EOF则返回'\0'，否则返回读取的字符
         */
        virtual char Read() = 0;

        /**
         * @brief 读取一个字符
         * @exception IOException 遇到IO错误时抛出异常
         * @return 若遇到EOF则返回'\0'，否则返回读取的字符
         */
        virtual char Peek() = 0;
    };

    /**
     * @brief 基于ArrayView的TextReader
     */
    class TextReaderFromView :
        public TextReader
    {
    public:
        TextReaderFromView(const ArrayView<char>& view, const char* sourceName="Unknown");

    public:
        const char* GetSourceName()const noexcept override;
        size_t GetLength()const noexcept override;
        size_t GetPosition()const noexcept override;
        uint32_t GetLine()const noexcept override;
        uint32_t GetColumn()const noexcept override;
        bool IsEof()const noexcept override;
        char Read()override;
        char Peek()override;

    private:
        size_t m_uPosition = 0;
        uint32_t m_uLine = 1;
        uint32_t m_uColumn = 1;
        ArrayView<char> m_stView;
        std::string m_stSourceName;
    };
}
