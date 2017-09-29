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
         * @brief 获取总长度
         * @return 总长度
         */
        virtual size_t GetLength()noexcept = 0;

        /**
         * @brief 获取当前的位置
         * @return 当前读取位置
         */
        virtual size_t GetPosition()noexcept = 0;

        /**
         * @brief 判断是否达到了结尾
         */
        virtual bool IsEof()noexcept = 0;

        /**
         * @brief 读取一个字符，并提升读取的位置
         * @exception IOException 遇到IO错误时抛出异常
         * @return 若遇到EOF则返回-1，否则返回读取的字符
         */
        virtual int Read() = 0;

        /**
         * @brief 读取一个字符
         * @exception IOException 遇到IO错误时抛出异常
         * @return 若遇到EOF则返回-1，否则返回读取的字符
         */
        virtual int Peek() = 0;
    };

    /**
     * @brief 基于ArrayView的TextReader
     */
    class TextReaderFromView :
        public TextReader
    {
    public:
        TextReaderFromView(const ArrayView<char>& view);

    public:
        size_t GetLength()noexcept override;
        size_t GetPosition()noexcept override;
        bool IsEof()noexcept override;
        int Read()override;
        int Peek()override;

    private:
        size_t m_uPosition = 0;
        ArrayView<char> m_stView;
    };
}
