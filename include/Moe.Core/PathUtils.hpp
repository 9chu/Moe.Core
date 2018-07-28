/**
 * @file
 * @date 2017/5/28
 */
#pragma once
#include <vector>
#include "ArrayView.hpp"
#include "Exception.hpp"

namespace moe
{
    /**
     * @brief 路径实用函数
     *
     * 用于计算相对路径、绝对路径、获取文件名等操作的实用函数
     */
    namespace PathUtils
    {
        /**
         * @brief 检查是否为路径中的合法字符
         * @tparam TChar 字符类型
         *
         * 由于linux路径中非法字符少于windows，因此选取并集来作为合法或非法的判断
         * 下述字符会被认为不能在路径中出现：
         *   : / \ * ? " < > |
         */
        template <typename TChar = char>
        inline bool IsValidCharacterInPath(TChar x)noexcept
        {
            switch (x) {
                case ':':
                case '/':
                case '\\':
                case '*':
                case '?':
                case '"':
                case '<':
                case '>':
                case '|':
                    return false;
                default:
                    return true;
            }
        }

        /**
         * @brief 路径规范化标识
         */
        enum class PathNormalizeFlags
        {
            Default = 0,
            WindowsDriverPath = 1,  // 允许Windows盘符表述
            RemoveRootSplash = 2,  // 移除首个'/'
        };

        /**
         * @brief 规范化文件路径
         * @tparam TChar 字符类型
         * @throw BadFormat 无效格式
         * @param path 路径
         * @param flags 规范化参数
         * @return 规范化后路径
         *
         * 该方法将文件路径中的'\'替换成'/'
         * 该方法将检查路径中的非法字符并抛出异常
         * 该方法将清除空白的路径项目
         */
        template <typename TChar = char>
        std::basic_string<TChar> Normalize(const TChar* path, PathNormalizeFlags flags=PathNormalizeFlags::Default)
        {
            const bool windowsDriverPath =
                (static_cast<int>(flags) & static_cast<int>(PathNormalizeFlags::WindowsDriverPath)) != 0;
            const bool removeRootSplash =
                (static_cast<int>(flags) & static_cast<int>(PathNormalizeFlags::RemoveRootSplash)) != 0;

            std::basic_string<TChar> ret;
            const TChar* lastSplitPosition = nullptr;
            while (*path)
            {
                TChar c = *path;
                switch (c)
                {
                    case '/':
                    case '\\':
                        if (lastSplitPosition + 1 == path)
                            break;
                        ret.push_back('/');
                        lastSplitPosition = path;
                        break;
                    default:
                        if (!IsValidCharacterInPath(c))
                        {
                            if (c == ':' && windowsDriverPath)
                            {
                                if (lastSplitPosition == nullptr)  // workaround for windows
                                {
                                    ret.push_back(c);
                                    break;
                                }
                            }

                            MOE_THROW(BadFormatException, "Invalid character '{0}'.", c);
                        }
                        ret.push_back(c);
                        break;
                }
                ++path;
            }

            if (removeRootSplash)
            {
                while (!ret.empty() && ret.front() == '/')
                    ret.erase(ret.begin());
            }

            return ret;
        }

        /**
         * @brief 规范化文件夹路径
         * @tparam TChar 字符类型
         * @throw BadFormat 无效格式
         * @param path 路径
         * @param flags 规范化参数
         * @return 规范化后路径
         *
         * 该方法将文件夹路径中的'\'替换成'/'
         * 该方法将检查路径中的非法字符并抛出异常
         * 该方法将清除空白的路径项目
         * 若路径不以'/'结尾，方法将加上'/'
         */
        template <typename TChar = char>
        std::basic_string<TChar> NormalizeAsDirectory(const TChar* path,
            PathNormalizeFlags flags=PathNormalizeFlags::Default)
        {
            const bool removeRootSplash =
                (static_cast<int>(flags) & static_cast<int>(PathNormalizeFlags::RemoveRootSplash)) != 0;

            std::basic_string<TChar> ret = Normalize<TChar>(path, flags);

            if (ret.empty())
            {
                if (!removeRootSplash)
                    ret.push_back('/');
            }
            else if (ret.back() != '/')
                ret.push_back('/');

            return ret;
        }

        /**
         * @brief 将一个相对路径转换成绝对路径
         * @tparam TChar 字符类型
         * @param path 路径
         * @return 绝对路径输出
         *
         * 该方法将路径中的'\'替换成'/'
         * 该方法将清除空白的路径项目
         */
        template <typename TChar = char>
        std::basic_string<TChar> Absolute(const TChar* path)
        {
            const TChar* lastSplit = path;
            const TChar* p = path;
            std::vector<std::basic_string<TChar>> filenames;

            while (true)
            {
                if (*p == '/' || *p == '\\' || *p == '\0')
                {
                    if (lastSplit != p)
                    {
                        std::basic_string<TChar> filename(lastSplit, p - lastSplit);
                        if (filename == ".")
                        {
                            // do nothing.
                        }
                        else if (filename == "..")
                        {
                            if (filenames.size() > 0)
                                filenames.pop_back();
                            else
                                filenames.push_back("..");
                        }
                        else
                            filenames.emplace_back(std::move(filename));
                    }

                    lastSplit = p + 1;
                }

                if (*p == '\0')
                    break;
                ++p;
            }

            if (path != p && (*(p - 1) == '\\' || *(p - 1) == '/'))
                filenames.emplace_back("");

            return StringUtils::Join(filenames.begin(), filenames.end(), '/');
        }

        /**
         * @brief 获取文件名
         * @tparam TChar 字符类型
         * @param path 路径
         * @return 文件名
         *
         * 该方法用于获取路径中文件的文件名
         * 允许以'/'和'\'作路径分隔符
         * 若最后以'/'或'\'结尾则被认定为文件夹，返回空值
         */
        template <typename TChar = char>
        ArrayView<TChar> GetFileName(const TChar* path)noexcept
        {
            const TChar* lastFilenameStart = path;

            while (*path)
            {
                TChar c = *path;
                if (c == '\\' || c == '/')
                    lastFilenameStart = path + 1;
                ++path;
            }

            auto end = path + std::char_traits<TChar>::length(path);
            return ArrayView<TChar>(lastFilenameStart, end - lastFilenameStart);
        }

        template <typename TChar = char>
        ArrayView<TChar> GetFileName(const std::basic_string<TChar>& path)noexcept
        {
            size_t lastFilenameStart = 0;
            for (size_t i = 0; i < path.length(); ++i)
            {
                TChar c = path[i];
                if (c == '\\' || c == '/')
                    lastFilenameStart = i + 1;
            }
            if (lastFilenameStart >= path.length())
                return ArrayView<TChar>();
            return ArrayView<TChar>(path.data() + lastFilenameStart, path.length() - lastFilenameStart);
        }

        /**
         * @brief 获取文件扩展名
         * @tparam TChar 字符类型
         * @param path
         * @return 扩展名
         *
         * 该方法用于获取路径中文件的扩展名
         * 允许以'/'和'\'作路径分隔符
         * 若最后以'/'或'\'结尾则被认定为文件夹，返回空值
         * 使用'.'作扩展名的分界符，若文件名有多个'.'，则取最后一个
         * 返回扩展名不包含'.'
         */
        template <typename TChar = char>
        ArrayView<TChar> GetExtension(const TChar* path)noexcept
        {
            const TChar* lastFilenameStart = path;
            const TChar* lastExtensionStart = nullptr;

            while (*path)
            {
                TChar c = *path;
                if (c == '\\' || c == '/')
                    lastFilenameStart = path + 1;
                else if (c == '.')
                    lastExtensionStart = path + 1;
                ++path;
            }

            if (lastExtensionStart <= lastFilenameStart)
                return ArrayView<TChar>();
            else
            {
                auto end = path + std::char_traits<TChar>::length(path);
                return ArrayView<TChar>(lastExtensionStart, end - lastExtensionStart);
            }
        }

        template <typename TChar = char>
        ArrayView<TChar> GetExtension(const std::basic_string<TChar>& path)noexcept
        {
            size_t lastFilenameStart = 0;
            size_t lastExtensionStart = static_cast<size_t>(-1);

            for (size_t i = 0; i < path.length(); ++i)
            {
                TChar c = path[i];
                if (c == '\\' || c == '/')
                    lastFilenameStart = i + 1;
                else if (c == '.')
                    lastExtensionStart = i + 1;
            }

            if (lastExtensionStart == static_cast<size_t>(-1) || lastExtensionStart <= lastFilenameStart)
                return ArrayView<TChar>();
            else
                return ArrayView<TChar>(path.data() + lastExtensionStart, path.length() - lastExtensionStart);
        }
    }
}
