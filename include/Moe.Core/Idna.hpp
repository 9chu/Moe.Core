/**
 * @file
 * @author chu
 * @date 2018/6/7
 */
#pragma once
#include "ArrayView.hpp"
#include "Exception.hpp"

#include <string>

namespace moe
{
    namespace Idna
    {
        /**
         * @brief 将Unicode转换到Punycode
         * @exception BadFormatException 当输入串无效时抛出
         * @param[out] out 输出串
         * @param input 输入串
         *
         * 注意：这里的Punycode实现不处理大小写，考虑到IDNA实现使用Unicode::Normalize，实现大小写在这里没有意义。
         */
        void PunycodeEncode(std::u32string& out, ArrayView<char32_t> input);

        /**
         * @brief 从Punycode解码出Unicode
         * @param[out] out 输出串
         * @param input 输入串
         */
        void PunycodeDecode(std::u32string& out, ArrayView<char32_t> input);
    }
}
