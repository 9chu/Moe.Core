/**
 * @file
 * @author chu
 * @date 2018/5/17
 */
#pragma once
#include "ArrayView.hpp"

#include <string>

namespace moe
{
    namespace Unicode
    {
        /**
         * @brief 规格化形式
         */
        enum class NormalizationFormType
        {
            NFC,
            NFKC,
            NFD,
            NFKD,
        };

        /**
         * @brief 获取数据库的版本号
         */
        const char* GetVersion()noexcept;

        /**
         * @brief 查询一个字符是否具有XID_Start属性
         * @param ch 字符
         * @return 若具备该属性则返回true，否则返回false。
         */
        bool IsXidStart(char32_t ch)noexcept;

        /**
         * @brief 查询一个字符是否具有XID_Continue属性
         * @param ch 字符
         * @return 若具备该属性则返回true，否则返回false。
         */
        bool IsXidContinue(char32_t ch)noexcept;

        /**
         * @brief 查询一个字符是否具有十进制数字属性(Decimal,0-9)
         * @param ch 字符
         * @param[out] out 若不为空则返回对应的整数
         * @return 若具备Decimal属性则返回true，否则返回false。
         */
        bool IsDecimalDigit(char32_t ch, unsigned *out = nullptr)noexcept;

        /**
         * @brief 查询一个字符是否具有数字属性(Digit,0-9)
         * @param ch 字符
         * @param[out] out 若不为空则返回对应的整数
         * @return 若具备Digit属性则返回true，否则返回false。
         */
        bool IsDigit(char32_t ch, unsigned *out = nullptr)noexcept;

        /**
         * @brief 查询一个字符是否是数字
         * @param ch Unicode字符
         * @param[out] out 若非空，则返回对应表达的数字
         * @return 若为数字，则返回true，否则返回false
         */
        bool IsNumeric(char32_t ch, double *out = nullptr)noexcept;

        /**
         * @brief 查询一个字符是否是字母
         * @param ch 字符
         * @return 若为字母，则返回true，否则返回false
         *
         * 字母包括：分类为Ll、Lu、Lt、Lo或者Lm的字符。
         */
        bool IsAlpha(char32_t ch)noexcept;

        /**
         * @brief 查询一个字符是否可打印
         * @param ch 字符
         * @return 若可打印，则返回true，否则返回false
         *
         * 除去以下分类的字符：Cc、Cf、Cs、Co、Cn、Zl、Zp及Zs（不含空格），都为可打印字符。
         */
        bool IsPrintable(char32_t ch)noexcept;

        /**
         * @brief 查询一个字符是否是空白符
         * @param ch 字符
         * @return 若为空白符，则返回true，否则返回false
         *
         * 空白符包括：Bidirectional为WS、B或S的字符，或者其分类为Zs的字符。
         */
        bool IsWhitespace(char32_t ch)noexcept;

        /**
         * @brief 查询一个字符是否是行分隔符
         * @param ch 字符
         * @return 若为行分隔符，则返回true，否则返回false
         *
         * 行分隔符包括：Bidirectional为B的，或者属性中包含BK、CR、LF或NL的字符。
         */
        bool IsLinebreak(char32_t ch)noexcept;

        /**
         * @brief 查询一个字符是否是词首大写字符
         * @param ch 字符
         * @return 若字符带有Lt分类，则返回true，否则返回false。
         */
        bool IsTitlecase(char32_t ch)noexcept;

        /**
         * @brief 查询一个字符是否是小写字符
         * @param ch 字符
         * @return 若字符带有Ll分类，则返回true，否则返回false。
         */
        bool IsLowercase(char32_t ch)noexcept;

        /**
         * @brief 查询一个字符是否是大写字符
         * @param ch 字符
         * @return 若字符带有Lu分类，则返回true，否则返回false。
         */
        bool IsUppercase(char32_t ch)noexcept;

        /**
         * @brief 查询一个字符是否是大小写转换的
         * @param ch 字符
         * @return 若是大小写转换的，返回true，否则返回false。
         */
        bool IsCased(char32_t ch)noexcept;

        /**
         * @brief 查询一个字符是否是不区分大小写的
         * @param ch 字符
         * @return 若是不区分大小写的，返回true，否则返回false。
         */
        bool IsCaseIgnorable(char32_t ch)noexcept;

        /**
         * @brief 转换字符到对应的词首大写字符
         * @param ch 字符
         * @return 返回对应的词首大写字符，若无，则返回自身
         */
        char32_t ToTitlecase(char32_t ch)noexcept;

        /**
         * @brief 获取所有对应的词首大写字符形式
         * @param[in,out] out 输出缓冲区
         * @param ch 字符
         * @return 返回映射的数量
         */
        size_t ToTitlecaseFull(MutableArrayView<char32_t> out, char32_t ch)noexcept;

        /**
         * @brief 转换字符到对应的大写字符
         * @param ch 字符
         * @return 返回对应的大写字符，若无，则返回自身
         */
        char32_t ToUppercase(char32_t ch)noexcept;

        /**
         * @brief 获取所有对应的大写字符形式
         * @param[in,out] out 输出缓冲区
         * @param ch 字符
         * @return 返回映射的数量
         */
        size_t ToUppercaseFull(MutableArrayView<char32_t> out, char32_t ch)noexcept;

        /**
         * @brief 转换字符到对应的小写字符
         * @param ch 字符
         * @return 返回对应的小写字符，若无，则返回自身
         */
        char32_t ToLowercase(char32_t ch)noexcept;

        /**
         * @brief 获取所有对应的小写字符形式
         * @param[in,out] out 输出缓冲区
         * @param ch 字符
         * @return 返回映射的数量
         */
        size_t ToLowercaseFull(MutableArrayView<char32_t> out, char32_t ch)noexcept;

        /**
         * @brief 获取所有对应的大小写折叠字符形式
         * @param[in,out] out 输出缓冲区
         * @param ch 字符
         * @return 返回映射的数量
         *
         * 通常，字符的大小写折叠形式和小写形式相同，但是也有例外。
         */
        size_t ToFoldedFull(MutableArrayView<char32_t> out, char32_t ch)noexcept;

        /**
         * @brief 获取字符的分类
         */
        const char* GetCategory(char32_t ch)noexcept;

        /**
         * @brief 获取字符的书写方向
         */
        const char* GetBidirectional(char32_t ch)noexcept;

        /**
         * @brief 获取字符的组合方式
         */
        int GetCombining(char32_t ch)noexcept;

        /**
         * @brief 检查一个字符在Bidi中是否具备Mirrored属性
         */
        bool IsMirrored(char32_t ch)noexcept;

        /**
         * @brief 获取字符的EastAsian宽度
         */
        const char* GetEastAsianWidth(char32_t ch)noexcept;

        /**
         * @brief 解组合一个字符并返回对应的映射串
         * @param[out] out 输出串
         * @param ch 被解组合字符
         * @return 映射数量
         *
         * 若没有这样的映射，则返回空串。
         * 其中串格式如下：{prefix} XXXX XXXX ...
         */
        size_t Decomposition(std::string& out, char32_t ch);

        inline std::string Decomposition(char32_t ch)
        {
            std::string ret;
            Decomposition(ret, ch);
            return ret;
        }

        /**
         * @brief 检查是否已经是规格化文本
         * @param input 输入串
         * @param form 规格化形式
         * @return 是否已经规格化
         *
         * 方法会进行一次快速规格化判断，倘若无法得出可信结果，则依旧会进行一次规格化转换操作而后再进行比较。
         */
        bool IsNormalized(ArrayView<char32_t> input, Unicode::NormalizationFormType form);

        /**
         * @brief Unicode规格化
         * @param[out] out 输出结果
         * @param input 输入字符串
         * @param form 规格化形式
         */
        void Normalize(std::u32string& out, ArrayView<char32_t> input, Unicode::NormalizationFormType form);

        inline void Normalize(std::u32string& out, const char32_t* input, Unicode::NormalizationFormType form)
        {
            Normalize(out, ArrayView<char32_t>(input, std::char_traits<char32_t>::length(input)), form);
        }

        inline void Normalize(std::u32string& out, const std::u32string& input, Unicode::NormalizationFormType form)
        {
            Normalize(out, ArrayView<char32_t>(input.data(), input.length()), form);
        }

        /**
         * @brief Unicode 3.2.0版本
         *
         * 用于一些特殊场景。
         */
        namespace Ucd_3_2_0
        {
            /**
             * @brief 获取字符的分类
             */
            const char* GetCategory(char32_t ch)noexcept;

            /**
             * @brief 获取字符的书写方向
             */
            const char* GetBidirectional(char32_t ch)noexcept;

            /**
             * @brief 获取字符的组合方式
             */
            int GetCombining(char32_t ch)noexcept;

            /**
             * @brief 检查一个字符在Bidi中是否具备Mirrored属性
             */
            bool IsMirrored(char32_t ch)noexcept;

            /**
             * @brief 获取字符的EastAsian宽度
             */
            const char* GetEastAsianWidth(char32_t ch)noexcept;

            /**
             * @brief 解组合一个字符并返回对应的映射串
             * @param[out] out 输出串
             * @param ch 被解组合字符
             * @return 映射数量
             *
             * 若没有这样的映射，则返回空串。
             * 其中串格式如下：{prefix} XXXX XXXX ...
             */
            size_t Decomposition(std::string& out, char32_t ch);

            inline std::string Decomposition(char32_t ch)
            {
                std::string ret;
                Ucd_3_2_0::Decomposition(ret, ch);
                return ret;
            }

            /**
             * @brief Unicode规格化
             * @param[out] out 输出结果
             * @param input 输入字符串
             * @param form 规格化形式
             */
            void Normalize(std::u32string& out, ArrayView<char32_t> input, Unicode::NormalizationFormType form);

            inline void Normalize(std::u32string& out, const char32_t* input, Unicode::NormalizationFormType form)
            {
                Ucd_3_2_0::Normalize(out, ArrayView<char32_t>(input, std::char_traits<char32_t>::length(input)), form);
            }

            inline void Normalize(std::u32string& out, const std::u32string& input, Unicode::NormalizationFormType form)
            {
                Ucd_3_2_0::Normalize(out, ArrayView<char32_t>(input.data(), input.length()), form);
            }
        }
    }
}
