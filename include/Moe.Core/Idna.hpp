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
    /**
     * @brief IDNA支持
     * @ref https://www.unicode.org/reports/tr46/
     */
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

        /**
         * @brief 编码字符串
         * @param[out] out 输出编码后字符串
         * @param domainName 输入域名
         * @param checkHyphens 检查分隔符
         * @param checkBidi 检查书写方向
         * @param checkJoiners 检查Joiner）
         * @param useStd3Rules 采用AsciiSTD3规则
         * @param transitionalProcessing 透传处理
         * @param verifyDnsLength 检查DNS长度
         */
        void ToAscii(std::u32string& out, ArrayView<char32_t> domainName, bool checkHyphens=true,
            bool checkBidi=true, bool checkJoiners=true, bool useStd3Rules=true, bool transitionalProcessing=false,
            bool verifyDnsLength=true);

        inline std::u32string ToAscii(const char32_t* domainName, bool checkHyphens=true, bool checkBidi=true,
            bool checkJoiners=true, bool useStd3Rules=true, bool transitionalProcessing=false,
            bool verifyDnsLength=true)
        {
            std::u32string ret;
            ToAscii(ret, ArrayView<char32_t>(domainName, std::char_traits<char32_t>::length(domainName)), checkHyphens,
                checkBidi, checkJoiners, useStd3Rules, transitionalProcessing, verifyDnsLength);
            return ret;
        }

        inline std::u32string ToAscii(const std::u32string& domainName, bool checkHyphens=true, bool checkBidi=true,
            bool checkJoiners=true, bool useStd3Rules=true, bool transitionalProcessing=false,
            bool verifyDnsLength=true)
        {
            std::u32string ret;
            ToAscii(ret, ArrayView<char32_t>(domainName.data(), domainName.length()), checkHyphens, checkBidi,
                checkJoiners, useStd3Rules, transitionalProcessing, verifyDnsLength);
            return ret;
        }

        /**
         * @brief 转换到Unicode字符串
         * @param[out] out 转换结果
         * @param domainName 输入域名
         * @param checkHyphens 检查分隔符
         * @param checkBidi 检查书写方向
         * @param checkJoiners 检查Joiner）
         * @param useStd3Rules 采用AsciiSTD3规则
         * @param transitionalProcessing 透传处理
         */
        void ToUnicode(std::u32string& out, ArrayView<char32_t> domainName, bool checkHyphens=true,
            bool checkBidi=true, bool checkJoiners=true, bool useStd3Rules=true, bool transitionalProcessing=false);

        inline std::u32string ToUnicode(const char32_t* domainName, bool checkHyphens=true, bool checkBidi=true,
            bool checkJoiners=true, bool useStd3Rules=true, bool transitionalProcessing=false)
        {
            std::u32string ret;
            ToUnicode(ret, ArrayView<char32_t>(domainName, std::char_traits<char32_t>::length(domainName)),
                checkHyphens, checkBidi, checkJoiners, useStd3Rules, transitionalProcessing);
            return ret;
        }

        inline std::u32string ToUnicode(const std::u32string& domainName, bool checkHyphens=true, bool checkBidi=true,
            bool checkJoiners=true, bool useStd3Rules=true, bool transitionalProcessing=false)
        {
            std::u32string ret;
            ToUnicode(ret, ArrayView<char32_t>(domainName.data(), domainName.length()), checkHyphens, checkBidi,
                checkJoiners, useStd3Rules, transitionalProcessing);
            return ret;
        }
    }
}
