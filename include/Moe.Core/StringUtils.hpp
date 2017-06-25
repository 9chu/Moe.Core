/**
 * @file
 * @date 2017/5/20
 */
#pragma once
#include <set>
#include <string>

#include "Convert.hpp"

namespace moe
{
    /**
     * @brief 字符串辅助
     *
     * 扩展std::basic_string的功能。
     */
    namespace StringUtils
    {
        //////////////////////////////////////// <editor-fold desc="字符检测类">

        /**
         * @brief 字符集
         */
        template <typename TChar = char>
        using CharSet = std::set<TChar>;

        /**
         * @brief 检查字符是否属于ASCII码中的空白符
         * @tparam TChar 字符类型
         * @param[in] c 待检测字符
         * @return 是否属于空白符
         */
        template <typename TChar = char>
        constexpr bool IsWhitespace(TChar c)
        {
            return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
        }

        /**
         * @brief 检查字符是否属于ASCII码中的数字
         * @tparam TChar 字符类型
         * @param c 待检测字符
         * @return 是否属于数字
         */
        template <typename TChar = char>
        constexpr bool IsDigit(TChar c)
        {
            return c >= '0' && c <= '9';
        }

        /**
         * @brief 检查字符是否属于ASCII码中的字母
         * @param c 待检测字符
         * @return 是否属于字母
         */
        template <typename TChar = char>
        constexpr bool IsAlphabet(TChar c)
        {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        }

        /**
         * @brief 检查字符是否属于Unicode中的空白符
         * @param c 被检测字符
         * @return 是否属于空白符
         */
        bool IsUnicodeWhitespace(char32_t c);

        //////////////////////////////////////// </editor-fold>

        //////////////////////////////////////// <editor-fold desc="大小写转换">

        /**
         * @brief 转换字符到大写
         * @tparam TChar 字符类型
         * @param c 待转换字符
         * @return 输出大写字符或者原始字符
         */
        template <typename TChar = char>
        constexpr TChar ToUpper(TChar c)
        {
            return (c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c;
        }

        /**
         * @brief 转换字符到小写
         * @tparam TChar 字符类型
         * @param c 待转换字符
         * @return 输出小写字符或者原始字符
         */
        template <typename TChar = char>
        constexpr TChar ToLower(TChar c)
        {
            return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
        }

        /**
         * @brief 原地转换到大写字符串
         * @tparam TChar 字符类型
         * @param str 被转换字符串
         * @return 转换结果
         */
        template <typename TChar = char>
        std::basic_string<TChar>& ToUpperInPlace(std::basic_string<TChar>& str)
        {
            auto it = str.begin();
            while (it != str.end())
            {
                *it = ToUpper<TChar>(*it);
                ++it;
            }
            return str;
        }

        /**
         * @brief 转换到大写字符串
         * @tparam TChar 字符类型
         * @param str 被转换字符串
         * @return 转换结果
         */
        template <typename TChar = char>
        inline std::basic_string<TChar> ToUpper(const std::basic_string<TChar>& str)
        {
            auto temp(str);
            return ToUpperInPlace(temp);
        }

        template <typename TChar = char>
        inline std::basic_string<TChar> ToUpper(const TChar* str)
        {
            std::basic_string<TChar> temp(str);
            return ToUpperInPlace(temp);
        }

        /**
         * @brief 原地转换到小写字符串
         * @tparam TChar 字符类型
         * @param str 被转换字符串
         * @return 转换结果
         */
        template <typename TChar = char>
        std::basic_string<TChar>& ToLowerInPlace(std::basic_string<TChar>& str)
        {
            auto it = str.begin();
            while (it != str.end())
            {
                *it = ToLower<TChar>(*it);
                ++it;
            }
            return str;
        }

        /**
         * @brief 转换到小写字符串
         * @tparam TChar 字符类型
         * @param str 被转换字符串
         * @return 转换结果
         */
        template <typename TChar = char>
        inline std::basic_string<TChar> ToLower(const std::basic_string<TChar>& str)
        {
            auto temp(str);
            return ToLowerInPlace(temp);
        }

        template <typename TChar = char>
        inline std::basic_string<TChar> ToLower(const TChar* str)
        {
            std::basic_string<TChar> temp(str);
            return ToLowerInPlace(temp);
        }

        //////////////////////////////////////// </editor-fold>

        //////////////////////////////////////// <editor-fold desc="字符串剔除">

        /**
         * @brief 原地剔除字符串开头
         * @tparam TChar 字符类型
         * @param[inout] str 字符串
         * @param set 字符集
         */
        template <typename TChar = char>
        std::basic_string<TChar>& TrimLeftInPlace(std::basic_string<TChar>& str, const CharSet<TChar>& set)
        {
            if (str.empty())
                return str;

            auto it = str.begin();
            while (it != str.end())
            {
                if (set.find(*it) == set.end())
                    break;
                ++it;
            }

            str.erase(str.begin(), it);
            return str;
        }

        /**
         * @brief 剔除字符串开头
         * @tparam TChar 字符类型
         * @param str 字符串
         * @param set 字符集
         * @return 剔除后字符串
         */
        template <typename TChar = char>
        inline std::basic_string<TChar> TrimLeft(const std::basic_string<TChar>& str, const CharSet<TChar>& set)
        {
            auto temp(str);
            return TrimLeftInPlace<TChar>(temp, set);
        }

        template <typename TChar = char>
        inline std::basic_string<TChar> TrimLeft(const TChar* str, const CharSet<TChar>& set)
        {
            std::basic_string<TChar> temp(str);
            return TrimLeftInPlace<TChar>(temp, set);
        }

        /**
         * @brief 原地剔除字符串开头
         * @tparam TChar 字符类型
         * @param[inout] str 字符串
         *
         * 该方法使用Unicode空白字符作为剔除字符集。
         */
        template <typename TChar = char>
        std::basic_string<TChar>& TrimLeftInPlace(std::basic_string<TChar>& str)
        {
            if (str.empty())
                return str;

            auto it = str.begin();
            while (it != str.end())
            {
                if (!IsUnicodeWhitespace(*it))
                    break;
                ++it;
            }

            str.erase(str.begin(), it);
            return str;
        }

        /**
         * @brief 剔除字符串开头
         * @tparam TChar 字符类型
         * @param str 字符串
         * @return 剔除后字符串
         *
         * 该方法使用Unicode空白符作为剔除字符集。
         */
        template <typename TChar = char>
        inline std::basic_string<TChar> TrimLeft(const std::basic_string<TChar>& str)
        {
            auto temp(str);
            return TrimLeftInPlace<TChar>(temp);
        }

        template <typename TChar = char>
        inline std::basic_string<TChar> TrimLeft(const TChar* str)
        {
            std::basic_string<TChar> temp(str);
            return TrimLeftInPlace<TChar>(temp);
        }

        /**
         * @brief 原地剔除字符串末尾
         * @tparam TChar 字符类型
         * @param[inout] str 字符串
         * @param set 字符集
         */
        template <typename TChar = char>
        std::basic_string<TChar>& TrimRightInPlace(std::basic_string<TChar>& str, const CharSet<TChar>& set)
        {
            if (str.empty())
                return str;

            auto it = str.end();
            do
            {
                if (set.find(*(it - 1)) == set.end())
                    break;
                --it;
            } while (it != str.begin());

            str.erase(it, str.end());
            return str;
        }

        /**
         * @brief 剔除字符串末尾
         * @tparam TChar 字符类型
         * @param str 字符串
         * @param set 字符集
         * @return 剔除后字符串
         */
        template <typename TChar = char>
        inline std::basic_string<TChar> TrimRight(const std::basic_string<TChar>& str, const CharSet<TChar>& set)
        {
            auto temp(str);
            return TrimRightInPlace<TChar>(temp, set);
        }

        template <typename TChar = char>
        inline std::basic_string<TChar> TrimRight(const TChar* str, const CharSet<TChar>& set)
        {
            std::basic_string<TChar> temp(str);
            return TrimRightInPlace<TChar>(temp, set);
        }

        /**
         * @brief 原地剔除字符串末尾
         * @tparam TChar 字符类型
         * @param[inout] str 字符串
         *
         * 该方法使用Unicode空白字符作为剔除字符集。
         */
        template <typename TChar = char>
        std::basic_string<TChar>& TrimRightInPlace(std::basic_string<TChar>& str)
        {
            if (str.empty())
                return str;

            auto it = str.end();
            do
            {
                if (!IsUnicodeWhitespace(*(it - 1)))
                    break;
                --it;
            } while (it != str.begin());

            str.erase(it, str.end());
            return str;
        }

        /**
         * @brief 剔除字符串末尾
         * @tparam TChar 字符类型
         * @param str 字符串
         * @return 剔除后字符串
         *
         * 该方法使用Unicode空白符作为剔除字符集。
         */
        template <typename TChar = char>
        inline std::basic_string<TChar> TrimRight(const std::basic_string<TChar>& str)
        {
            auto temp(str);
            return TrimRightInPlace<TChar>(temp);
        }

        template <typename TChar = char>
        inline std::basic_string<TChar> TrimRight(const TChar* str)
        {
            std::basic_string<TChar> temp(str);
            return TrimRightInPlace<TChar>(temp);
        }

        /**
         * @brief 原地剔除字符串首尾空白
         * @tparam TChar 字符类型
         * @param[inout] str 字符串
         * @param set 字符集
         */
        template <typename TChar = char>
        inline std::basic_string<TChar>& TrimInPlace(std::basic_string<TChar>& str, const CharSet<TChar>& set)
        {
            return TrimLeftInPlace<TChar>(TrimRightInPlace<TChar>(str, set));
        }

        /**
         * @brief 剔除字符串首尾空白
         * @tparam TChar 字符类型
         * @param str 字符串
         * @param set 字符集
         * @return 剔除后字符串
         */
        template <typename TChar = char>
        inline std::basic_string<TChar> Trim(const std::basic_string<TChar>& str,
            const CharSet<TChar>& set)
        {
            auto temp(str);
            return TrimInPlace<TChar>(temp, set);
        }

        template <typename TChar = char>
        inline std::basic_string<TChar> Trim(const TChar* str, const CharSet<TChar>& set)
        {
            std::basic_string<TChar> temp(str);
            return TrimInPlace<TChar>(temp, set);
        }

        /**
         * @brief 原地剔除字符串首尾空白
         * @tparam TChar 字符类型
         * @param[inout] str 字符串
         *
         * 该方法使用Unicode空白字符作为剔除字符集。
         */
        template <typename TChar = char>
        inline std::basic_string<TChar>& TrimInPlace(std::basic_string<TChar>& str)
        {
            return TrimLeftInPlace<TChar>(TrimRightInPlace<TChar>(str));
        }

        /**
         * @brief 剔除字符串首尾空白
         * @tparam TChar 字符类型
         * @param str 字符串
         * @return 剔除后字符串
         *
         * 该方法使用Unicode空白符作为剔除字符集。
         */
        template <typename TChar = char>
        inline std::basic_string<TChar> Trim(const std::basic_string<TChar>& str)
        {
            auto temp(str);
            return TrimInPlace<TChar>(temp);
        }

        template <typename TChar = char>
        inline std::basic_string<TChar> Trim(const TChar* str)
        {
            std::basic_string<TChar> temp(str);
            return TrimInPlace<TChar>(temp);
        }

        //////////////////////////////////////// </editor-fold>

        //////////////////////////////////////// <editor-fold desc="其他字符串辅助函数">

        /**
         * @brief 字符串拼接
         * @tparam TIterator 迭代器类型
         * @tparam TChar 字符类型
         * @param begin 起始迭代器
         * @param end 终止迭代器
         * @param c 拼接用字符
         * @return 返回结果
         *
         * 要求*TIterator == TChar。
         */
        template <typename TIterator, typename TChar = char>
        std::basic_string<TChar> Join(TIterator begin, TIterator end, TChar c)
        {
            std::basic_string<TChar> ret;
            auto i = begin;
            while (i != end)
            {
                ret.append(*i);
                ret.push_back(c);
                ++i;
            }

            if (begin != end)
                ret.pop_back();
            return ret;
        }

        /**
         * @brief 字符串拼接
         * @tparam TIterator 迭代器类型
         * @tparam TChar 字符类型
         * @param begin 起始迭代器
         * @param end 终止迭代器
         * @param str 拼接用字符串
         * @return 返回结果
         *
         * 要求*TIterator == TChar。
         */
        template <typename TIterator, typename TChar = char>
        std::basic_string<TChar> Join(TIterator begin, TIterator end, const std::basic_string<TChar>& str)
        {
            std::basic_string<TChar> ret;
            auto i = begin;
            while (i != end)
            {
                ret.append(*i);
                if (i + 1 != end)
                    ret.append(str);
                ++i;
            }

            return ret;
        }

        template <typename TIterator, typename TChar = char>
        std::basic_string<TChar> Join(TIterator begin, TIterator end, const TChar* str)
        {
            std::basic_string<TChar> ret;
            auto i = begin;
            while (i != end)
            {
                ret.append(*i);
                if (i + 1 != end)
                    ret.append(str);
                ++i;
            }

            return ret;
        }

        /**
         * @brief 字符串分割选项
         */
        enum class SplitFlags
        {
            Default = 0,  ///< @brief 默认值
            RemoveEmptyEntries = 1,  ///< @brief 指示是否移除分割结果中的空项目
        };

        /**
         * @brief 分割字符串
         * @tparam TContainer 容器类型
         * @tparam TChar 字符类型
         * @param out[out] 输出容器
         * @param str 待分割字符串
         * @param ch 分割字符
         * @param flags 分割选项
         * @return 被分割数量
         */
        template <class TContainer, typename TChar = char>
        size_t Split(TContainer& out, const std::basic_string<TChar>& str, TChar ch,
            SplitFlags flags = SplitFlags::Default)
        {
            const bool removeEmptyEntries =
                (static_cast<int>(flags) & static_cast<int>(SplitFlags::RemoveEmptyEntries)) != 0;

            out.clear();

            typename std::basic_string<TChar>::size_type last = 0;
            while (last < str.size())
            {
                auto pos = str.find_first_of(ch, last);
                if (pos != std::basic_string<TChar>::npos)
                {
                    if (last == pos)
                    {
                        if (!removeEmptyEntries)
                            out.emplace_back(std::basic_string<TChar>());
                    }
                    else
                        out.emplace_back(str.substr(last, pos - last));

                    last = pos + 1;
                }
                else
                    break;
            }

            if (last < str.size() || !removeEmptyEntries)
            {
                if (last >= str.size())
                    out.emplace_back(std::basic_string<TChar>());
                else
                    out.emplace_back(str.substr(last, std::basic_string<TChar>::npos));
            }

            return out.size();
        }

        template <class TContainer, typename TChar = char>
        inline size_t Split(TContainer& out, const TChar* str, TChar ch, SplitFlags flags = SplitFlags::Default)
        {
            return Split<TContainer, TChar>(out, std::basic_string<TChar>(str), ch, flags);
        }

        /**
         * @brief 分割字符串
         * @tparam TContainer 容器类型
         * @tparam TChar 字符类型
         * @param out[out] 输出容器
         * @param str 待分割字符串
         * @param splitter 分割用字符串
         * @param flags 分割选项
         * @return 被分割数量
         */
        template <class TContainer, typename TChar = char>
        size_t Split(TContainer& out, const std::basic_string<TChar>& str, const std::basic_string<TChar>& splitter,
            SplitFlags flags = SplitFlags::Default)
        {
            const bool removeEmptyEntries =
                (static_cast<int>(flags) & static_cast<int>(SplitFlags::RemoveEmptyEntries)) != 0;

            out.clear();

            if (splitter.empty())
            {
                out.emplace_back(str);
                return 1;
            }

            typename std::basic_string<TChar>::size_type last = 0;
            while (last < str.size())
            {
                auto pos = str.find(splitter, last);
                if (pos != std::basic_string<TChar>::npos)
                {
                    if (last == pos)
                    {
                        if (!removeEmptyEntries)
                            out.emplace_back(std::basic_string<TChar>());
                    }
                    else
                        out.emplace_back(str.substr(last, pos - last));

                    last = pos + splitter.length();
                }
                else
                    break;
            }

            if (last < str.size() || !removeEmptyEntries)
            {
                if (last >= str.size())
                    out.emplace_back(std::basic_string<TChar>());
                else
                    out.emplace_back(str.substr(last, std::basic_string<TChar>::npos));
            }

            return out.size();
        }

        template <class TContainer, typename TChar = char>
        inline size_t Split(TContainer& out, const std::basic_string<TChar>& str, const TChar* splitter,
            SplitFlags flags = SplitFlags::Default)
        {
            return Split<TContainer, TChar>(out, str, std::basic_string<TChar>(splitter), flags);
        }

        template <class TContainer, typename TChar = char>
        inline size_t Split(TContainer& out, const TChar* str, const TChar* splitter,
            SplitFlags flags = SplitFlags::Default)
        {
            return Split<TContainer, TChar>(out, std::basic_string<TChar>(str), std::basic_string<TChar>(splitter),
                flags);
        }

        //////////////////////////////////////// </editor-fold>

        //////////////////////////////////////// <editor-fold desc="字符串格式化">

        namespace details
        {
            template <typename TChar>
            struct StringConstant
            {
                static inline const TChar* GetTrue()
                {
                    static const TChar kTrue[] = { 't', 'r', 'u', 'e', '\0' };
                    return kTrue;
                }

                static inline const TChar* GetFalse()
                {
                    static const TChar kFalse[] = { 'f', 'a', 'l', 's', 'e', '\0' };
                    return kFalse;
                }

                static inline const TChar* GetNull()
                {
                    static const TChar kNull[] = { 'n', 'u', 'l', 'l', '\0' };
                    return kNull;
                }
            };

            template <typename TChar>
            struct HasMethodToStringValidator
            {
                template <class T,
                    typename U = typename std::decay<decltype(std::declval<T>().ToString())>::type,
                    typename = typename std::enable_if<
                        std::is_same<std::basic_string<TChar>, U>::value>::type>
                static std::true_type Test(int);

                template <typename>
                static std::false_type Test(...);
            };

            template <typename TChar>
            struct HasMethodToStringExValidator
            {
                template <class T,
                    typename U = typename std::decay<
                        decltype(std::declval<T>().ToString(std::declval<const ArrayView<TChar>&>()))>::type,
                    typename = typename std::enable_if<
                        std::is_same<std::basic_string<TChar>, U>::value>::type>
                static std::true_type Test(int);

                template <typename>
                static std::false_type Test(...);
            };

            template <typename TChar, typename T, typename U =
                decltype(HasMethodToStringValidator<TChar>::template Test<T>(0))>
            struct HasMethodToString :
                public U
            {};

            template <typename TChar, typename T, typename U =
                decltype(HasMethodToStringExValidator<TChar>::template Test<T>(0))>
            struct HasMethodToStringEx :
                public U
            {};

            template <typename TChar>
            using ToStringFormatter = bool(*)(std::basic_string<TChar>&, const void*, const ArrayView<TChar>&);

            template <typename TChar, typename T>
            struct BoolToStringFormatter
            {
                static const size_t kPreAllocate = 8;

                static bool AppendToString(std::basic_string<TChar>& output, const void* object,
                    const ArrayView<TChar>& format)
                {
                    T value = *static_cast<const T*>(object);

                    output.reserve(output.length() + kPreAllocate);

                    if (format.GetSize() == 0)
                        output.append(value ? StringConstant<TChar>::GetTrue() : StringConstant<TChar>::GetFalse());
                    else
                    {
                        auto where = std::char_traits<TChar>::find(format.GetBuffer(), format.GetSize(), '|');
                        if (where == nullptr)
                            return false;

                        ArrayView<TChar> falseString(format.GetBuffer(), where - format.GetBuffer()),
                            trueString(where + 1, format.GetSize() - (where + 1 - format.GetBuffer()));

                        size_t count = value ? trueString.GetSize() : falseString.GetSize();
                        output.append(value ? trueString.GetBuffer() : falseString.GetBuffer(), count);
                    }

                    return true;
                }
            };

            template <typename TChar, typename T>
            struct IntergeToStringFormatter
            {
                static const size_t kPreAllocate = 32;

                static bool AppendToString(std::basic_string<TChar>& output, const void* object,
                    const ArrayView<TChar>& format)
                {
                    T value = *static_cast<const T*>(object);

                    bool result = true;
                    size_t count = 0;
                    auto pos = output.length();
                    output.resize(pos + kPreAllocate);

                    if (format.GetSize() == 0)
                        count = Convert::ToDecimalString(value, &output[pos], kPreAllocate);
                    else if (format.GetSize() == 1)
                    {
                        if (format[0] == 'H')
                            count = Convert::ToHexString(value, &output[pos], kPreAllocate);
                        else if (format[0] == 'h')
                            count = Convert::ToHexStringLower(value, &output[pos], kPreAllocate);
                        else if (format[0] == 'D')
                            count = Convert::ToDecimalString(value, &output[pos], kPreAllocate);
                        else
                            result = false;
                    }
                    else
                        result = false;

                    if (result)
                        output.resize(pos + count);
                    else
                        output.resize(pos);

                    return result;
                }
            };

            template <typename TChar>
            struct IntergeToStringFormatter<TChar, char>
            {
                static const size_t kPreAllocate = 5;

                static bool AppendToString(std::basic_string<TChar>& output, const void* object,
                    const ArrayView<TChar>& format)
                {
                    MOE_UNUSED(format);

                    char value = *static_cast<const char*>(object);

                    output.reserve(output.length() + kPreAllocate);

                    if (value > 0 && ::isprint(value))
                        output.push_back(value);
                    else
                    {
                        output.push_back('\\');

                        size_t count = 0;
                        auto pos = output.length();
                        count = Convert::ToDecimalString((uint8_t)value, &output[pos], kPreAllocate - 1);

                        output.resize(pos + count);
                    }

                    return true;
                }
            };

            template <typename TChar>
            struct IntergeToStringFormatter<TChar, size_t>
            {
                static const size_t kPreAllocate = 32;

                static bool AppendToString(std::basic_string<TChar>& output, const void* object,
                    const ArrayView<TChar>& format)
                {
                    uint64_t value = static_cast<uint64_t>(*static_cast<const size_t*>(object));

                    bool result = true;
                    size_t count = 0;
                    auto pos = output.length();
                    output.resize(pos + kPreAllocate);

                    if (format.GetSize() == 0)
                        count = Convert::ToDecimalString(value, &output[pos], kPreAllocate);
                    else if (format.GetSize() == 1)
                    {
                        if (format[0] == 'H')
                            count = Convert::ToHexString(value, &output[pos], kPreAllocate);
                        else if (format[0] == 'h')
                            count = Convert::ToHexStringLower(value, &output[pos], kPreAllocate);
                        else if (format[0] == 'D')
                            count = Convert::ToDecimalString(value, &output[pos], kPreAllocate);
                        else
                            result = false;
                    }
                    else
                        result = false;

                    if (result)
                        output.resize(pos + count);
                    else
                        output.resize(pos);

                    return result;
                }
            };

            template <typename TChar, typename T>
            struct DoubleToStringFormatter
            {
                static const size_t kPreAllocate = 128;

                static bool AppendToString(std::basic_string<TChar>& output, const void* object,
                    const ArrayView<TChar>& format)
                {
                    T value = *static_cast<const T*>(object);

                    bool result = true;
                    size_t count = 0;
                    auto pos = output.length();
                    output.resize(pos + kPreAllocate);

                    if (format.GetSize() == 0)
                        count = Convert::ToShortestString(value, &output[pos], kPreAllocate);
                    else if (format.GetSize() == 1)
                    {
                        if (format[0] == 'E')
                            count = Convert::ToExponentialString(value, &output[pos], kPreAllocate);
                        else if (format[0] == 'S')
                            count = Convert::ToShortestString(value, &output[pos], kPreAllocate);
                        else
                            result = false;
                    }
                    else
                    {
                        TChar t = format[0];
                        unsigned digit = 0;
                        for (size_t i = 1; i < format.GetSize(); ++i)
                        {
                            TChar ch = format[i];
                            if (!(ch >= '0' && ch <= '9'))
                                return false;
                            digit = digit * 10 + (ch - '0');
                        }

                        switch (t)
                        {
                            case 'F':
                                if (digit > 20)
                                    digit = 20;
                                count = Convert::ToFixedString(value, digit, &output[pos], kPreAllocate);
                                break;
                            case 'P':
                                if (digit < 1)
                                    digit = 1;
                                else if (digit > 21)
                                    digit = 21;
                                count = Convert::ToPrecisionString(value, digit, &output[pos], kPreAllocate);
                                break;
                            case 'E':
                                if (digit > 20)
                                    digit = 20;
                                count = Convert::ToExponentialString(value, digit, &output[pos], kPreAllocate);
                                break;
                            default:
                                result = false;
                                break;
                        }
                    }

                    if (result)
                        output.resize(pos + count);
                    else
                        output.resize(pos);

                    return result;
                }
            };

            template <typename TChar, typename T>
            struct StringToStringFormatter
            {
                static bool AppendToString(std::basic_string<TChar>& output, const void* object,
                    const ArrayView<TChar>& format)
                {
                    const T& value = *static_cast<const T*>(object);

                    if (format.GetSize() != 0)
                        return false;

                    output.reserve(output.length() + value.length());
                    output.append(value);
                    return true;
                }
            };

            template <typename TChar, typename T, bool = std::is_array<T>::value>
            struct CStringToStringFormatter;

            template <typename TChar, typename T>
            struct CStringToStringFormatter<TChar, T, false>  // const char*
            {
                static bool AppendToString(std::basic_string<TChar>& output, const void* object,
                    const ArrayView<TChar>& format)
                {
                    const T& value = *static_cast<const T*>(object);

                    if (format.GetSize() != 0)
                        return false;

                    if (value == nullptr)
                        output.append(StringConstant<TChar>::GetNull());
                    else
                        output.append(value);

                    return true;
                }
            };

            template <typename TChar, typename T>
            struct CStringToStringFormatter<TChar, T, true>  // const char[]
            {
                static bool AppendToString(std::basic_string<TChar>& output, const void* object,
                    const ArrayView<TChar>& format)
                {
                    const TChar* value = static_cast<const TChar*>(object);

                    if (format.GetSize() != 0)
                        return false;

                    output.append(value);
                    return true;
                }
            };

            template <typename TChar, typename T, bool = std::is_array<T>::value>
            struct PointerToStringFormatter;

            template <typename TChar, typename T>
            struct PointerToStringFormatter<TChar, T, false>  // const T*
            {
                static const size_t kPreAllocate = 32;

                static bool AppendToString(std::basic_string<TChar>& output, const void* object,
                    const ArrayView<TChar>& format)
                {
                    const T& value = *static_cast<const T*>(object);

                    if (format.GetSize() != 0)
                        return false;

                    size_t count = 0;
                    auto pos = output.length();
                    output.resize(pos + kPreAllocate + 2);

                    output[pos] = '0';
                    output[pos + 1] = 'x';
                    count = Convert::ToHexString(reinterpret_cast<uint64_t>(value), &output[pos + 2], kPreAllocate);

                    output.resize(pos + count + 2);
                    return true;
                }
            };

            template <typename TChar, typename T>
            struct PointerToStringFormatter<TChar, T, true>  // const T[]
            {
                static const size_t kPreAllocate = 32;

                static bool AppendToString(std::basic_string<TChar>& output, const void* object,
                    const ArrayView<TChar>& format)
                {
                    if (format.GetSize() != 0)
                        return false;

                    size_t count = 0;
                    auto pos = output.length();
                    output.resize(pos + kPreAllocate + 2);

                    output[pos] = '0';
                    output[pos + 1] = 'x';
                    count = Convert::ToHexString(reinterpret_cast<uint64_t>(object), &output[pos + 2], kPreAllocate);

                    output.resize(pos + count + 2);
                    return true;
                }
            };

            template <typename TChar, typename T>
            struct NullptrToStringFormatter
            {
                static bool AppendToString(std::basic_string<TChar>& output, const void* object,
                    const ArrayView<TChar>& format)
                {
                    MOE_UNUSED(object);

                    if (format.GetSize() != 0)
                        return false;

                    output.append(StringConstant<TChar>::GetNull());
                    return true;
                }
            };

            template <typename TChar, typename T>
            struct CustomExToStringFormatter
            {
                static bool AppendToString(std::basic_string<TChar>& output, const void* object,
                    const ArrayView<TChar>& format)
                {
                    const T& value = *static_cast<const T*>(object);
                    auto str = value.ToString(format);

                    output.append(std::move(str));
                    return true;
                }
            };

            template <typename TChar, typename T>
            struct CustomToStringFormatter
            {
                static bool AppendToString(std::basic_string<TChar>& output, const void* object,
                    const ArrayView<TChar>& format)
                {
                    MOE_UNUSED(format);

                    const T& value = *static_cast<const T*>(object);
                    auto str = value.ToString();

                    output.append(std::move(str));
                    return true;
                }
            };

            template <typename TChar, typename T>
            using ToStringFormatterSelectCustomOrNot = typename std::conditional<
                HasMethodToString<TChar, T>::value,
                CustomToStringFormatter<TChar, T>,
                void>::type;

            template <typename TChar, typename T>
            using ToStringFormatterSelectCustomExOrNot = typename std::conditional<
                HasMethodToStringEx<TChar, T>::value,
                CustomExToStringFormatter<TChar, T>,
                ToStringFormatterSelectCustomOrNot<TChar, T>>::type;

            template <typename TChar, typename T>
            using ToStringFormatterSelectNullptrOrNot = typename std::conditional<
                std::is_same<typename std::remove_cv<typename std::decay<T>::type>::type, std::nullptr_t>::value,
                NullptrToStringFormatter<TChar, T>,
                ToStringFormatterSelectCustomExOrNot<TChar, T>>::type;

            template <typename TChar, typename T>
            using ToStringFormatterSelectPointerOrNot = typename std::conditional<
                std::is_pointer<T>::value || std::is_array<T>::value,
                PointerToStringFormatter<TChar, T>,
                ToStringFormatterSelectNullptrOrNot<TChar, T>>::type;

            template <typename TChar, typename T>
            using ToStringFormatterSelectCStringOrNot = typename std::conditional<
                (std::is_pointer<T>::value || std::is_array<T>::value) &&
                    std::is_same<typename std::remove_cv<typename std::decay<T>::type>::type, TChar*>::value,
                CStringToStringFormatter<TChar, T>,
                ToStringFormatterSelectPointerOrNot<TChar, T>>::type;

            template <typename TChar, typename T>
            using ToStringFormatterSelectStringOrNot = typename std::conditional<
                std::is_same<T, std::basic_string<TChar>>::value,
                StringToStringFormatter<TChar, T>,
                ToStringFormatterSelectCStringOrNot<TChar, T>>::type;

            template <typename TChar, typename T>
            using ToStringFormatterSelectDoubleOrNot = typename std::conditional<
                std::is_floating_point<T>::value,
                DoubleToStringFormatter<TChar, T>,
                ToStringFormatterSelectStringOrNot<TChar, T>>::type;

            template <typename TChar, typename T>
            using ToStringFormatterSelectIntergeOrNot = typename std::conditional<
                std::is_integral<T>::value,
                IntergeToStringFormatter<TChar, T>,
                ToStringFormatterSelectDoubleOrNot<TChar, T>>::type;

            template <typename TChar, typename T>
            using ToStringFormatterSelectBoolOrNot = typename std::conditional<
                std::is_same<typename std::remove_cv<typename std::decay<T>::type>::type, bool>::value,
                BoolToStringFormatter<TChar, T>,
                ToStringFormatterSelectIntergeOrNot<TChar, T>>::type;

            template <typename TChar, typename T>
            using ToStringFormatterSelector = ToStringFormatterSelectBoolOrNot<TChar, T>;
        }

        /**
         * @brief 字符串格式化（原地）
         * @tparam TChar 字符类型
         * @tparam Args 参数类型
         * @param[out] out 输出结果
         * @param format 格式化文本
         * @param args 参数列表
         * @see https://github.com/dotnet/coreclr/blob/master/src/mscorlib/shared/System/Text/StringBuilder.cs
         *
         * 格式化字符串格式（扩展但不完全兼容C#语法）：
         *   Hole := '{' Indexer ws* (',' ws* PaddingCount ('[' PaddingCharacter ']')? ws* )? (':' ws* Format )? ws* '}'
         *   Indexer := [0-9]+
         *   PaddingCount := '-'? [0-9]+
         *   PaddingCharacter := .  // PaddingCharacter为扩展语法
         *   Format := [^} ]  // C#允许在Format中对'{'和'}'进行转义，这个语法被去除了
         *   ws := ' '
         *
         * 正文中，连续的"{{"会被转换为"{"，连续的"}}"同理。
         * 注意到该实现不会抛出异常（但用户自定义ToString函数可以抛出异常），如果格式字符串出现任何问题会原封不同拷贝到结果中。
         * 这也意味着正文中出现的独立'}'不会被当成未配对括号抛出异常（而被直接拷贝到结果中）。
         *
         * 支持的格式化语法如下：
         *   - 布尔类型：
         *     - 默认：true/false
         *     - .* '|' .*：用户定义的假值和真值
         *   - 整数类型：
         *     - 默认：以十进制转换到字符串
         *     - D: 以十进制转换到字符串（同默认）
         *     - H：以大写十六进制转换到字符串（有符号数会被转换到对应的无符号整形表示）
         *     - h：以小写十六进制转换到字符串（有符号数会被转换到对应的无符号整形表示）
         *   - 浮点类型：
         *     - 默认：以浮点数的最短表示进行输出
         *     - S：以浮点数的最短表示进行输出（同默认）
         *     - E：以科学计数法表示，尽可能保留足够多的小数
         *     - E[0-9]+：以科学计数法表示，后接需要的小数位数 [0, 20]
         *     - P[0-9]+: 以有效数字表达，后接需要的有效数字位数 [1, 21]
         *     - F[0-9]+：以定点小数表达，后接需要的定点小数位数 [0, 20]
         *   - const TChar* 或者 const TChar[] / std::basic_string<TChar>：
         *     - 默认：直接拷贝
         *   - 其他指针：
         *     - 默认：以十六进制展示（0x??...）
         *   - nullptr_t：
         *     - 默认：以null展示
         *   - 用户自定义类型：
         *     - 默认：调用 ToString(const ArrayView<TChar>&)，若不可用，调用 ToString()
         */
        template <typename TChar = char, typename... Args>
        void FormatInPlace(std::basic_string<TChar>& out, const ArrayView<TChar>& format, const Args&... args)
        {
            static const unsigned kIndexLimit = 1000000u;
            static const unsigned kWidthLimit = 1000000u;

            out.clear();
            out.reserve(256);

            TChar ch = '\0';
            size_t pos = 0;
            size_t len = format.GetSize();
            const void* objects[] = { static_cast<const void*>(&args)... };
            details::ToStringFormatter<TChar> formatters[] = {
                details::ToStringFormatterSelector<TChar, Args>::AppendToString...
            };
            static_assert(std::extent<decltype(objects)>::value == std::extent<decltype(formatters)>::value,
                "Unexpected condition");

            while (true)
            {
                // 不断读取并寻找 '{'
                while (pos < len)
                {
                    ch = format[pos++];

                    if (ch == '}')
                    {
                        // 将连续的 '}}' 转义成 '}'
                        // 单个情况下在C#中属于错误配对异常，在这直接做容错处理，不管。
                        if (pos < len && format[pos] == '}')
                            ++pos;
                    }
                    else if (ch == '{')
                    {
                        // 将连续的 '{{' 转义成 '{'
                        if (pos < len && format[pos] == '{')
                            ++pos;
                        else
                        {
                            --pos;
                            break;
                        }
                    }

                    out.append(1, ch);
                }

                // 字符串处理完毕
                if (pos == len)
                    break;

                // 开始解析格式化语法
                size_t holeStart = pos++;  // 记录当前开始的位置，可以方便做容错
                unsigned index = 0;
                bool leftJustify = false;
                unsigned padding = 0;
                TChar paddingCharacter = ' ';
                ArrayView<TChar> formatDescriptor;
                size_t outputPos = 0, outputLength = 0;

                // 解析Indexer部分
                if (pos == len)
                    goto badFormat;

                ch = format[pos];
                if (!(ch >= '0' && ch <= '9'))
                    goto badFormat;

                do
                {
                    index = index * 10 + ch - '0';

                    if ((++pos) == len)
                        goto badFormat;

                    ch = format[pos];
                } while (ch >= '0' && ch <= '9' && index < kIndexLimit);

                if (index >= std::extent<decltype(formatters)>::value)  // 索引越界
                    goto badFormat;

                while (pos < len && (ch = format[pos]) == ' ')  // 读取可选的空白
                    ++pos;

                // 解析Padding部分
                if (ch == ',')
                {
                    ++pos;
                    while (pos < len && format[pos] == ' ')  // 读取可选的空白
                        ++pos;

                    if (pos == len)  // 索引越界
                        goto badFormat;

                    if ((ch = format[pos]) == '-')  // 是否存在一个减号
                    {
                        leftJustify = true;  // 此时左对齐

                        if ((++pos) == len)
                            goto badFormat;

                        ch = format[pos];
                    }

                    if (!(ch >= '0' && ch <= '9'))  // 非法字符
                        goto badFormat;

                    do
                    {
                        padding = padding * 10 + ch - '0';

                        if ((++pos) == len)
                            goto badFormat;

                        ch = format[pos];
                    } while (ch >= '0' && ch <= '9' && padding < kWidthLimit);

                    // 扩展语法：如果紧跟一个'['，则读取PaddingCharacter
                    if (ch == '[')
                    {
                        if ((++pos) == len)
                            goto badFormat;

                        // 读取PaddingCharacter
                        paddingCharacter = format[pos];

                        if ((++pos) == len)
                            goto badFormat;

                        // 后面必须紧跟一个']'
                        ch = format[pos];
                        if (ch != ']')
                            goto badFormat;

                        if ((++pos) == len)
                            goto badFormat;

                        ch = format[pos];
                    }

                    while (pos < len && (ch = format[pos]) == ' ')  // 读取可选的空白
                        ++pos;
                }

                // 读取可选的格式化字段
                if (ch == ':')
                {
                    size_t descriptorStart = 0;

                    ++pos;
                    while (pos < len && (ch = format[pos]) == ' ')  // 读取可选的空白
                        ++pos;

                    if (pos == len)
                        goto badFormat;

                    descriptorStart = pos;
                    while (pos < len && !(ch == '}' || ch == ' '))
                    {
                        ++pos;
                        ch = format[pos];
                    }

                    if (pos == len)
                        goto badFormat;

                    if (pos != descriptorStart)
                        formatDescriptor = ArrayView<TChar>(&format[descriptorStart], pos - descriptorStart);

                    while (pos < len && (ch = format[pos]) == ' ')  // 读取可选的空白
                        ++pos;
                }

                // 此时，format[pos]必然为一个'}'
                if (ch != '}')
                    goto badFormat;
                ++pos;

                // 基本格式化参数收集完成，开始进行格式化处理
                // 格式化函数保证在字符串末尾插入，因此先记录当前输出的位置，后续需要用来调整Padding
                outputPos = out.length();

                // 执行格式化函数进行输出
                assert(formatters[index]);
                if (!formatters[index](out, objects[index], formatDescriptor))
                {
                    // 如果转换失败，格式化函数会进行清理
                    assert(out.length() == outputPos);
                    goto badFormat;
                }

                outputLength = out.length() - outputPos;
                if (outputLength < padding)  // 需要进行补齐操作
                {
                    assert(outputPos + padding == out.length() + (padding - outputLength));
                    out.resize(outputPos + padding);

                    size_t paddingPos = 0;
                    if (leftJustify)
                        paddingPos = outputPos + outputLength;
                    else
                    {
                        // 需要对字符串做移动操作
                        ::memmove(&out[out.length() - outputLength], out.data() + outputPos, outputLength *
                            sizeof(TChar));

                        paddingPos = outputPos;
                    }

                    // 填充Padding字符
                    for (size_t i = paddingPos, paddingEnd = paddingPos + padding - outputLength; i < paddingEnd; ++i)
                        out[i] = paddingCharacter;
                }

                continue;  // 完成一个字符串的格式化操作

            badFormat:
                // 错误恢复，直接将从holeStart开始到当前pos位置的所有字符放入结果
                if (pos < len)
                    ++pos;
                out.append(&format[holeStart], pos - holeStart);
            }
        }

        template <typename TChar = char, typename... Args>
        void FormatInPlace(std::basic_string<TChar>& out, const TChar* format, const Args&... args)
        {
            size_t length = std::char_traits<TChar>::length(format);
            FormatInPlace(out, ArrayView<TChar>(format, length), args...);
        }

        template <typename TChar = char, typename... Args>
        std::basic_string<TChar> Format(const TChar* format, const Args&... args)
        {
            size_t length = std::char_traits<TChar>::length(format);

            std::basic_string<TChar> ret;
            ret.reserve(256);

            FormatInPlace(ret, ArrayView<TChar>(format, length), args...);
            return ret;
        }

        template <typename TChar = char, typename... Args>
        std::basic_string<TChar> Format(const std::basic_string<TChar>& format, const Args&... args)
        {
            return Format(format.c_str(), args...);
        }

        //////////////////////////////////////// </editor-fold>

        //////////////////////////////////////// <editor-fold desc="字符串直接转换">

        /**
         * @brief 转到字符串（原地）
         * @tparam T 对象类型
         * @tparam TChar 字符类型
         * @param out 输出字符串
         * @param obj 对象
         * @param format 格式化参数
         */
        template <typename T, typename TChar = char>
        void ToStringInPlace(std::basic_string<TChar>& out, const T& obj, const TChar* format="")
        {
            using Formatter = details::ToStringFormatterSelector<TChar, T>;

            size_t sz = std::char_traits<TChar>::length(format);
            out.clear();

            Formatter::AppendToString(out, static_cast<void*>(&obj), ArrayView<TChar>(format, sz));
        }

        /**
         * @brief 转换到字符串
         * @tparam T 对象类型
         * @tparam TChar 字符类型
         * @param obj 对象
         * @param format 格式化参数
         * @return 结果
         */
        template <typename T, typename TChar = char>
        std::basic_string<TChar> ToString(const T& obj, const TChar* format="")
        {
            std::basic_string<TChar> ret;
            ToStringInPlace<T, TChar>(ret, obj, format);

            return std::move(ret);
        }

        //////////////////////////////////////// </editor-fold>
    }  // StringUtils
}
