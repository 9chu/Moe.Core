/**
 * @file
 * @author chu
 * @date 2017/5/28
 */
#pragma once
#include "Exception.hpp"
#include "ArrayView.hpp"

namespace moe
{
    /**
     * @brief 字符编码支持
     *
     * Encoding名字空间提供对编解码器的支持。
     *
     * 约定：
     *   - 所有编解码器需要使用 编码名::Decoder 和 编码名::Encoder 实现相应的编解码器
     *   - 编解码接口必须使用 operator() 实现, 状态由类自行管理
     *   - 编解码器应当可以被拷贝或移动
     *   - 当解码发生错误时, 解码器应当返回Reject, 并重置到起始状态
     *   - 当编码发生错误时, 编码器应当返回Reject, 并重置到起始状态
     *   - 编码器不应当存储状态
     *   - 编解码器不应当抛出异常
     */
    namespace Encoding
    {
        /**
         * @brief 编码结果
         */
        enum class EncodingResult
        {
            Accept = 0,
            Reject = 1,
            Incomplete = 2,
        };

        /**
         * @brief UTF8编解码器
         */
        class UTF8
        {
        public:
            // 字符类型
            using CharType = char;

            // 最大码点大小
            static const uint32_t kMaxCodePointSize = 6;

            class Decoder
            {
            public:
                Decoder()noexcept = default;
                Decoder(const Decoder&)noexcept = default;
                Decoder(Decoder&&)noexcept = default;

            public:
                EncodingResult operator()(char ch, char32_t& out)noexcept;

            private:
                uint32_t m_iState = 0;
            };

            class Encoder
            {
            public:
                Encoder()noexcept = default;
                Encoder(const Encoder&)noexcept = default;
                Encoder(Encoder&&)noexcept = default;

            public:
                EncodingResult operator()(char32_t ch, char out[], uint32_t& cnt)noexcept;
            };
        };

        /**
         * @brief UTF16编解码器
         */
        class UTF16
        {
        public:
            // 字符类型
            using CharType = char16_t;

            // 最大码点大小
            static const uint32_t kMaxCodePointSize = 2;

            class Decoder
            {
            private:
                uint32_t m_iState = 0;
                uint16_t m_iLastWord = 0;

            public:
                Decoder()noexcept = default;
                Decoder(const Decoder&)noexcept = default;
                Decoder(Decoder&&)noexcept = default;

            public:
                EncodingResult operator()(char16_t ch, char32_t& out)noexcept;
            };

            class Encoder
            {
            public:
                Encoder()noexcept = default;
                Encoder(const Encoder&)noexcept = default;
                Encoder(Encoder&&)noexcept = default;

            public:
                EncodingResult operator()(char32_t ch, char16_t out[], uint32_t& cnt)noexcept;
            };
        };

        /**
         * @brief UTF32编解码器
         *
         * 仅作接口匹配使用
         */
        class UTF32
        {
        public:
            // 字符类型
            using CharType = char32_t;

            // 最大码点大小
            static const uint32_t kMaxCodePointSize = 1;

            class Decoder
            {
            public:
                Decoder()noexcept = default;
                Decoder(const Decoder&)noexcept = default;
                Decoder(Decoder&&)noexcept = default;

            public:
                EncodingResult operator()(char32_t ch, char32_t& out)noexcept
                {
                    out = ch;
                    return EncodingResult::Accept;
                }
            };

            class Encoder
            {
            public:
                Encoder()noexcept = default;
                Encoder(const Encoder&)noexcept = default;
                Encoder(Encoder&&)noexcept = default;

            public:
                EncodingResult operator()(char32_t ch, char32_t out[], uint32_t& cnt)noexcept
                {
                    out[0] = ch;
                    cnt = 1;
                    return EncodingResult::Accept;
                }
            };
        };

        /**
         * @brief 执行编码转换
         * @throw InvalidEncodingException 当转换无效时抛出
         * @tparam DestEncoding 目标编码
         * @tparam SrcEncoding 原始编码
         * @tparam TDestChar 目标字符类型
         * @tparam TSrcChar 原始字符类型
         * @tparam DestContainer 目的容器
         * @param[out] out 转换输出
         * @param src 原始输入
         * @param replacer 非法字符替换字符, 若为0则在错误时抛出异常
         */
        template <typename DestEncoding, typename SrcEncoding, typename TDestChar = typename DestEncoding::CharType,
            typename TSrcChar = typename SrcEncoding::CharType, typename DestContainer = std::basic_string<TDestChar>>
        DestContainer& Convert(DestContainer& out, ArrayView<TSrcChar> src, char32_t replacer=0xFFFD)
        {
            static_assert(DestEncoding::kMaxCodePointSize > 0,
                "Invalid kMaxCodePointSize, which must bigger than zero.");
            static_assert(sizeof(TSrcChar) == sizeof(typename SrcEncoding::CharType), "Type size mismatched.");
            static_assert(sizeof(typename DestContainer::value_type) ==
                sizeof(typename DestEncoding::CharType), "Type size mismatched.");

            // 预分配空间
            out.clear();
            out.reserve(src.GetSize() > 10 ? src.GetSize() / 2 : src.GetSize());

            typename SrcEncoding::Decoder decoder;
            typename DestEncoding::Encoder encoder;

            // 保存解码器的输出
            EncodingResult lastDecoderResult = EncodingResult::Accept;
            char32_t decoderOutput = 0;

            // 保存编码器的输出
            typename DestEncoding::CharType destOutput[DestEncoding::kMaxCodePointSize] = { 0 };
            uint32_t destOutputCount = 0;

            for (size_t i = 0; i < src.GetSize(); ++i)
            {
                auto ch = src[i];

                switch (lastDecoderResult = decoder(static_cast<typename SrcEncoding::CharType>(ch), decoderOutput))
                {
                    case EncodingResult::Reject:  // 将错误的序列进行替换
                        if (replacer == 0)
                            MOE_THROW(InvalidEncodingException, "Bad input character near {0}", i);
                        decoderOutput = replacer;
                        lastDecoderResult = EncodingResult::Accept;
                    case EncodingResult::Accept:
                        switch (encoder(decoderOutput, destOutput, destOutputCount))
                        {
                            case EncodingResult::Reject:
                            case EncodingResult::Incomplete:
                                if (replacer == 0)
                                    MOE_THROW(InvalidEncodingException, "Cannot encode character near {0}", i);

                                // 试图使用replacer进行编码
                                switch (encoder(replacer, destOutput, destOutputCount))
                                {
                                    case EncodingResult::Reject:
                                    case EncodingResult::Incomplete:  // 若替换为替换字符依旧无法编码,则抛出异常
                                        MOE_THROW(InvalidEncodingException, "Cannot encode character near {0}", i);
                                    case EncodingResult::Accept:
                                        break;
                                    default:
                                        assert(false);
                                        break;
                                }
                            case EncodingResult::Accept:
                                for (uint32_t j = 0; j < destOutputCount; ++j)
                                    out.push_back(static_cast<TDestChar>(destOutput[j]));
                                break;
                            default:
                                assert(false);
                                break;
                        }
                        break;
                    default:
                        break;
                }
            }

            // 检查是否所有字符都被解码
            if (lastDecoderResult != EncodingResult::Accept)
            {
                if (replacer == 0)
                    MOE_THROW(InvalidEncodingException, "Not all character has been decoded");

                switch (encoder(replacer, destOutput, destOutputCount))
                {
                    case EncodingResult::Reject:
                    case EncodingResult::Incomplete:  // 若替换为替换字符依旧无法编码,则抛出异常
                        MOE_THROW(InvalidEncodingException, "Cannot encode character");
                    case EncodingResult::Accept:
                        break;
                    default:
                        assert(false);
                        break;
                }

                for (uint32_t j = 0; j < destOutputCount; ++j)
                    out.push_back(static_cast<TDestChar>(destOutput[j]));
            }

            return out;
        }

        /**
         * @brief 执行编码转换
         * @throw InvalidEncodingException 当转换无效时抛出
         * @tparam DestEncoding 目标编码
         * @tparam SrcEncoding 原始编码
         * @tparam TDestChar 目标字符类型
         * @tparam TSrcChar 原始字符类型
         * @tparam DestContainer 目的容器
         * @tparam SrcContainer 原始容器
         * @param src 原始字符串
         * @param replacer 非法字符替换字符, 若为0则在错误时抛出异常
         * @return 目标转换输出
         */
        template <typename DestEncoding, typename SrcEncoding, typename TDestChar = typename DestEncoding::CharType,
            typename TSrcChar = typename SrcEncoding::CharType, typename DestContainer = std::basic_string<TDestChar>,
            typename SrcContainer = std::basic_string<TSrcChar>>
        DestContainer Convert(const SrcContainer& src, char32_t replacer=0xFFFD)
        {
            DestContainer out;
            Convert<DestEncoding, SrcEncoding, TDestChar, TSrcChar, DestContainer>(out,
                ArrayView<typename SrcContainer::value_type>(src.c_str(), src.length()), replacer);
            return out;
        }

        /**
         * @brief 执行编码转换
         * @throw InvalidEncodingException 当转换无效时抛出
         * @tparam DestEncoding 目标编码
         * @tparam SrcEncoding 原始编码
         * @tparam TDestChar 目标字符类型
         * @tparam TSrcChar 原始字符类型
         * @tparam DestContainer 目的容器
         * @tparam SrcContainer 原始容器
         * @param src 原始字符串
         * @param replacer 非法字符替换字符, 若为0则在错误时抛出异常
         * @return 目标转换输出
         */
        template <typename DestEncoding, typename SrcEncoding, typename TDestChar = typename DestEncoding::CharType,
            typename TSrcChar = typename SrcEncoding::CharType, typename DestContainer = std::basic_string<TDestChar>,
            typename SrcContainer = std::basic_string<TSrcChar>>
        DestContainer Convert(ArrayView<TSrcChar> src, char32_t replacer=0xFFFD)
        {
            DestContainer out;
            Convert<DestEncoding, SrcEncoding, TDestChar, TSrcChar, DestContainer>(out, src, replacer);
            return out;
        }

        /**
         * @brief 执行编码转换
         * @throw InvalidEncodingException 当转换无效时抛出
         * @tparam DestEncoding 目标编码
         * @tparam SrcEncoding 原始编码
         * @tparam TDestChar 目标字符类型
         * @tparam TSrcChar 原始字符类型
         * @tparam DestContainer 目的容器
         * @tparam SrcContainer 原始容器
         * @param src 原始字符串
         * @param replacer 非法字符替换字符, 若为0则在错误时抛出异常
         * @return 目标转换输出
         */
        template <typename DestEncoding, typename SrcEncoding, typename TDestChar = typename DestEncoding::CharType,
            typename TSrcChar = typename SrcEncoding::CharType, typename DestContainer = std::basic_string<TDestChar>,
            typename SrcContainer = std::basic_string<TSrcChar>>
        DestContainer Convert(const TSrcChar* src, char32_t replacer=0xFFFD)
        {
            DestContainer out;
            Convert<DestEncoding, SrcEncoding, TDestChar, TSrcChar, DestContainer>(out,
                ArrayView<TSrcChar>(src, std::char_traits<TSrcChar>::length(src)), replacer);
            return out;
        }

        /**
         * @brief 执行编码转换
         * @tparam DestEncoding 目标编码
         * @tparam SrcEncoding 原始编码
         * @tparam TDestChar 目标字符类型
         * @tparam TSrcChar 原始字符类型
         * @param dest 目的字符串缓冲区
         * @param destSize 目的字符串缓冲区大小
         * @param src 原始字符串缓冲区
         * @param srcSize 原始字符串缓冲区大小
         * @param replacer 非法字符替换字符, 若为0则在错误时直接退出
         * @return 转换的字符数量（个数）
         *
         * @warning 方法不会在缓冲区结尾加入'\0'。
         */
        template <typename DestEncoding, typename SrcEncoding, typename TDestChar = typename DestEncoding::CharType,
            typename TSrcChar = typename SrcEncoding::CharType>
        size_t Convert(TDestChar* dest, size_t destSize, const TSrcChar* src, size_t srcSize,
            char32_t replacer=0xFFFD)noexcept
        {
            static_assert(DestEncoding::kMaxCodePointSize > 0,
                "Invalid kMaxCodePointSize, which must bigger than zero.");

            typename SrcEncoding::Decoder decoder;
            typename DestEncoding::Encoder encoder;

            // 保存解码器的输出
            EncodingResult lastDecoderResult = EncodingResult::Accept;
            char32_t decoderOutput = 0;

            // 保存编码器的输出
            typename DestEncoding::CharType destOutput[DestEncoding::kMaxCodePointSize] = { 0 };
            uint32_t destOutputCount = 0;

            size_t encodedCount = 0;
            for (size_t i = 0; i < srcSize; ++i)
            {
                auto ch = src[i];
                if (ch == '\0')
                    break;

                switch (lastDecoderResult = decoder(static_cast<typename SrcEncoding::CharType>(ch), decoderOutput))
                {
                    case EncodingResult::Reject:  // 将错误的序列进行替换
                        if (replacer == 0)
                            return encodedCount;
                        decoderOutput = replacer;
                        lastDecoderResult = EncodingResult::Accept;
                    case EncodingResult::Accept:
                        switch (encoder(decoderOutput, destOutput, destOutputCount))
                        {
                            case EncodingResult::Reject:
                            case EncodingResult::Incomplete:
                                if (replacer == 0)
                                    return encodedCount;

                                // 试图使用replacer进行编码
                                switch (encoder(replacer, destOutput, destOutputCount))
                                {
                                    case EncodingResult::Reject:
                                    case EncodingResult::Incomplete:  // 若替换为替换字符依旧无法编码,则抛出异常
                                        return encodedCount;
                                    case EncodingResult::Accept:
                                        break;
                                }
                            case EncodingResult::Accept:
                                if (destOutputCount + encodedCount > destSize)
                                    return encodedCount;  // 此时会导致截断
                                for (uint32_t j = 0; j < destOutputCount; ++j)
                                    dest[encodedCount++] = destOutput[j];
                                break;
                            default:
                                assert(false);
                                break;
                        }
                        break;
                    default:
                        break;
                }
            }

            // 检查是否所有字符都被解码
            if (lastDecoderResult != EncodingResult::Accept)
            {
                if (replacer != 0)
                {
                    switch (encoder(replacer, destOutput, destOutputCount))
                    {
                        case EncodingResult::Reject:
                        case EncodingResult::Incomplete:
                            return encodedCount;
                        case EncodingResult::Accept:
                            break;
                        default:
                            assert(false);
                            break;
                    }

                    if (destOutputCount + encodedCount > destSize)
                        return encodedCount;  // 此时会导致截断
                    for (uint32_t j = 0; j < destOutputCount; ++j)
                        dest[encodedCount++] = destOutput[j];
                }
            }

            return encodedCount;
        }
    }
}
