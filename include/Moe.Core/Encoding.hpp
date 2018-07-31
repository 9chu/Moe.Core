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
         * @brief 用于指示编码输入流终止
         */
        struct EndOfInputTag {};

        /**
         * @brief UTF8编解码器
         */
        class Utf8
        {
        public:
            static const char* const kName;

            class Decoder
            {
            public:
                using InputType = char;
                using OutputType = char32_t;
                static const uint32_t kMaxOutputCount = 1;

            public:
                Decoder()noexcept {}
                Decoder(const Decoder&)noexcept = default;
                Decoder(Decoder&&)noexcept = default;

                Decoder& operator=(const Decoder&)noexcept = default;
                Decoder& operator=(Decoder&&)noexcept = default;

            public:
                bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count)noexcept
                {
                    count = 0;
                    auto ret = (m_iState == 0);

                    m_iState = 0;
                    m_iTmp = 0;
                    return ret;
                }

                EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
                    uint32_t& count)noexcept;

            private:
                uint32_t m_iState = 0;
                uint32_t m_iTmp = 0;
            };

            class Encoder
            {
            public:
                using InputType = char32_t;
                using OutputType = char;
                static const uint32_t kMaxOutputCount = 6;

            public:
                Encoder()noexcept {}
                Encoder(const Encoder&)noexcept = default;
                Encoder(Encoder&&)noexcept = default;

                Encoder& operator=(const Encoder&)noexcept = default;
                Encoder& operator=(Encoder&&)noexcept = default;

            public:
                bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count)noexcept
                {
                    count = 0;
                    return true;
                }

                EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
                    uint32_t& count)noexcept;
            };
        };

        /**
         * @brief UTF16编解码器
         */
        class Utf16
        {
        public:
            static const char* const kName;

            class Decoder
            {
            public:
                using InputType = char16_t;
                using OutputType = char32_t;
                static const uint32_t kMaxOutputCount = 1;

            public:
                Decoder()noexcept {}
                Decoder(const Decoder&)noexcept = default;
                Decoder(Decoder&&)noexcept = default;

                Decoder& operator=(const Decoder&)noexcept = default;
                Decoder& operator=(Decoder&&)noexcept = default;

            public:
                bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count)noexcept
                {
                    count = 0;
                    auto ret = (m_iState == 0);

                    m_iState = 0;
                    m_iLastWord = 0;
                    return ret;
                }

                EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
                    uint32_t& count)noexcept;

            private:
                uint32_t m_iState = 0;
                uint16_t m_iLastWord = 0;
            };

            class Encoder
            {
            public:
                using InputType = char32_t;
                using OutputType = char16_t;
                static const uint32_t kMaxOutputCount = 2;

            public:
                Encoder()noexcept {}
                Encoder(const Encoder&)noexcept = default;
                Encoder(Encoder&&)noexcept = default;

                Encoder& operator=(const Encoder&)noexcept = default;
                Encoder& operator=(Encoder&&)noexcept = default;

            public:
                bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count)noexcept
                {
                    count = 0;
                    return true;
                }

                EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
                    uint32_t& count)noexcept;
            };
        };

        /**
         * @brief UTF32编解码器
         *
         * 仅作接口匹配使用
         */
        class Utf32
        {
        public:
            static const char* const kName;

            class Decoder
            {
            public:
                using InputType = char32_t;
                using OutputType = char32_t;
                static const uint32_t kMaxOutputCount = 1;

            public:
                Decoder()noexcept {}
                Decoder(const Decoder&)noexcept = default;
                Decoder(Decoder&&)noexcept = default;

                Decoder& operator=(const Decoder&)noexcept = default;
                Decoder& operator=(Decoder&&)noexcept = default;

            public:
                bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count)noexcept
                {
                    count = 0;
                    return true;
                }

                EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
                    uint32_t& count)noexcept
                {
                    out[0] = ch;
                    count = 1;
                    return EncodingResult::Accept;
                }
            };

            class Encoder
            {
            public:
                using InputType = char32_t;
                using OutputType = char32_t;
                static const uint32_t kMaxOutputCount = 1;

            public:
                Encoder()noexcept {}
                Encoder(const Encoder&)noexcept = default;
                Encoder(Encoder&&)noexcept = default;

                Encoder& operator=(const Encoder&)noexcept = default;
                Encoder& operator=(Encoder&&)noexcept = default;

            public:
                bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>&, uint32_t& count)noexcept
                {
                    count = 0;
                    return true;
                }

                EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
                    uint32_t& count)noexcept
                {
                    out[0] = ch;
                    count = 1;
                    return EncodingResult::Accept;
                }
            };
        };

        /**
         * @brief BASE64编解码器
         */
        class Base64
        {
        public:
            static const char* const kName;

            class Decoder
            {
            public:
                using InputType = char;
                using OutputType = uint8_t;
                static const uint32_t kMaxOutputCount = 3;

            public:
                Decoder()noexcept {}
                Decoder(const Decoder&)noexcept = default;
                Decoder(Decoder&&)noexcept = default;

                Decoder& operator=(const Decoder&)noexcept = default;
                Decoder& operator=(Decoder&&)noexcept = default;

            public:
                bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count)noexcept;

                EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
                    uint32_t& count)noexcept;

            private:
                uint32_t m_iState = 0;
                std::array<uint8_t, 3> m_stBuf;
            };

            class Encoder
            {
            public:
                using InputType = uint8_t;
                using OutputType = char;
                static const uint32_t kMaxOutputCount = 4;

            public:
                Encoder()noexcept {}
                Encoder(const Encoder&)noexcept = default;
                Encoder(Encoder&&)noexcept = default;

                Encoder& operator=(const Encoder&)noexcept = default;
                Encoder& operator=(Encoder&&)noexcept = default;

            public:
                bool operator()(EndOfInputTag, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count)noexcept;

                EncodingResult operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out,
                    uint32_t& count)noexcept;

            private:
                uint32_t m_iState = 0;
                std::array<char, 2> m_stBuf;
            };
        };

        /**
         * @brief 错误回退处理函数回调类型
         *
         * 该方法不应当抛出异常。
         */
        template <typename EncoderOrDecoder>
        using FailureFallbackCallbackType = bool(std::array<typename EncoderOrDecoder::OutputType,
            EncoderOrDecoder::kMaxOutputCount>& out, uint32_t& count) /* noexcept */;

        /**
         * @brief 默认Unicode回退处理函数
         */
        inline bool DefaultUnicodeFallbackHandler(std::array<char32_t, 1>& out, uint32_t& count)noexcept
        {
            out[0] = 0xFFFD;
            count = 1;
            return true;
        }

        /**
         * @brief 编码转换
         * @throw InvalidEncodingException 当转换无效时抛出
         * @tparam InputEncoding 输入编码
         * @tparam OutputEncoding 输出编码
         * @tparam InputType 输入字符类型
         * @tparam OutputType 输出字符类型
         * @tparam ContainerType 容器类型
         * @param out 转码输出
         * @param src 转码输入
         * @param decoderFailureFallback 当解码失败时的处理
         * @param encoderFailureFallback 当编码失败时的处理
         * @return out对象的引用
         */
        template <typename InputEncoding, typename OutputEncoding,
            typename InputType = typename InputEncoding::Decoder::InputType,
            typename OutputType = typename OutputEncoding::Encoder::OutputType,
            typename ContainerType = std::basic_string<OutputType>>
        ContainerType& Convert(ContainerType& out, ArrayView<InputType> src,
            FailureFallbackCallbackType<typename InputEncoding::Decoder> decoderFailureFallback = nullptr,
            FailureFallbackCallbackType<typename OutputEncoding::Encoder> encoderFailureFallback = nullptr)
        {
            using DecoderType = typename InputEncoding::Decoder;
            using EncoderType = typename OutputEncoding::Encoder;
            using DecoderInputType = typename InputEncoding::Decoder::InputType;
            using DecoderOutputType = typename InputEncoding::Decoder::OutputType;
            using EncoderInputType = typename OutputEncoding::Encoder::InputType;
            using EncoderOutputType = typename OutputEncoding::Encoder::OutputType;

            static_assert(std::is_same<DecoderOutputType, EncoderInputType>::value,
                "InputEncoder and OutputEncoder mismatched");

            DecoderType decoder;
            EncoderType encoder;
            EncodingResult decoderResult = EncodingResult::Accept;
            uint32_t decoderOutCount = 0, encoderOutCount = 0;
            std::array<DecoderOutputType, InputEncoding::Decoder::kMaxOutputCount> decoderBuffer;
            std::array<EncoderOutputType, OutputEncoding::Encoder::kMaxOutputCount> encoderBuffer;

            out.clear();
            out.reserve(src.GetSize());

            for (size_t i = 0; i <= src.GetSize(); ++i)
            {
                if (i < src.GetSize())
                    decoderResult = decoder(static_cast<DecoderInputType>(src[i]), decoderBuffer, decoderOutCount);
                else
                {
                    // 此处需要放置EOS的标志
                    decoderResult = decoder(EndOfInputTag(), decoderBuffer, decoderOutCount) ? EncodingResult::Accept :
                        EncodingResult::Reject;
                }

                switch (decoderResult)
                {
                    case EncodingResult::Reject:
                        if (!decoderFailureFallback || !decoderFailureFallback(decoderBuffer, decoderOutCount))
                        {
                            MOE_THROW(InvalidEncodingException, "{0} decoder cannot accept character near {1}",
                                InputEncoding::kName, i);
                        }
                        encoder = EncoderType();  // 需要重置编码器的状态
                        decoderResult = EncodingResult::Accept;
                    case EncodingResult::Accept:
                        for (size_t j = 0; j < decoderOutCount || (j == decoderOutCount && i >= src.GetSize()); ++j)
                        {
                            EncodingResult encoderResult;
                            if (j < decoderOutCount)
                                encoderResult = encoder(decoderBuffer[j], encoderBuffer, encoderOutCount);
                            else
                            {
                                encoderResult = encoder(EndOfInputTag(), encoderBuffer, encoderOutCount) ?
                                    EncodingResult::Accept : EncodingResult::Reject;
                            }

                            switch (encoderResult)
                            {
                                case EncodingResult::Reject:
                                    if (!encoderFailureFallback ||
                                        !encoderFailureFallback(encoderBuffer, encoderOutCount))
                                    {
                                        MOE_THROW(InvalidEncodingException,
                                            "{0} encoder cannot accept character near {1}", OutputEncoding::kName, i);
                                    }
                                case EncodingResult::Accept:
                                    for (uint32_t k = 0; k < encoderOutCount; ++k)
                                        out.push_back(static_cast<OutputType>(encoderBuffer[k]));
                                    break;
                                case EncodingResult::Incomplete:
                                default:
                                    break;
                            }
                        }
                        break;
                    case EncodingResult::Incomplete:
                    default:
                        break;
                }
            }

            assert(decoderResult == EncodingResult::Accept);  // 此时解码过程必然已经全部被接受
            return out;
        }

        /**
         * @brief 编码转换
         * @throw InvalidEncodingException 当转换无效时抛出
         * @tparam InputEncoding 输入编码
         * @tparam OutputEncoding 输出编码
         * @tparam InputType 输入字符类型
         * @tparam OutputType 输出字符类型
         * @tparam ContainerType 容器类型
         * @param src 转码输入
         * @param decoderFailureFallback 当解码失败时的处理
         * @param encoderFailureFallback 当编码失败时的处理
         * @return 转码输出
         */
        template <typename InputEncoding, typename OutputEncoding,
            typename InputType = typename InputEncoding::Decoder::InputType,
            typename OutputType = typename OutputEncoding::Encoder::OutputType,
            typename ContainerType = std::basic_string<OutputType>>
        ContainerType Convert(ArrayView<InputType> src,
            FailureFallbackCallbackType<typename InputEncoding::Decoder> decoderFailureFallback = nullptr,
            FailureFallbackCallbackType<typename OutputEncoding::Encoder> encoderFailureFallback = nullptr)
        {
            ContainerType out;
            Convert<InputEncoding, OutputEncoding, InputType, OutputType, ContainerType>(out, src,
                decoderFailureFallback, encoderFailureFallback);
            return out;
        }

        /**
         * @brief 编码转换
         * @throw InvalidEncodingException 当转换无效时抛出
         * @tparam InputEncoding 输入编码
         * @tparam OutputEncoding 输出编码
         * @tparam InputType 输入字符类型
         * @tparam OutputType 输出字符类型
         * @tparam ContainerType 容器类型
         * @param src 转码输入
         * @param decoderFailureFallback 当解码失败时的处理
         * @param encoderFailureFallback 当编码失败时的处理
         * @return 转码输出
         */
        template <typename InputEncoding, typename OutputEncoding,
            typename InputType = typename InputEncoding::Decoder::InputType,
            typename OutputType = typename OutputEncoding::Encoder::OutputType,
            typename ContainerType = std::basic_string<OutputType>>
        ContainerType Convert(const std::basic_string<InputType>& src,
            FailureFallbackCallbackType<typename InputEncoding::Decoder> decoderFailureFallback = nullptr,
            FailureFallbackCallbackType<typename OutputEncoding::Encoder> encoderFailureFallback = nullptr)
        {
            ContainerType out;
            Convert<InputEncoding, OutputEncoding, InputType, OutputType, ContainerType>(out, ArrayView<InputType>(
                src.data(), src.length()), decoderFailureFallback, encoderFailureFallback);
            return out;
        }

        /**
         * @brief 编码转换
         * @throw InvalidEncodingException 当转换无效时抛出
         * @tparam InputEncoding 输入编码
         * @tparam OutputEncoding 输出编码
         * @tparam InputType 输入字符类型
         * @tparam OutputType 输出字符类型
         * @tparam ContainerType 容器类型
         * @param src 转码输入
         * @param decoderFailureFallback 当解码失败时的处理
         * @param encoderFailureFallback 当编码失败时的处理
         * @return 转码输出
         */
        template <typename InputEncoding, typename OutputEncoding,
            typename InputType = typename InputEncoding::Decoder::InputType,
            typename OutputType = typename OutputEncoding::Encoder::OutputType,
            typename ContainerType = std::basic_string<OutputType>>
        ContainerType Convert(const InputType* src,
            FailureFallbackCallbackType<typename InputEncoding::Decoder> decoderFailureFallback = nullptr,
            FailureFallbackCallbackType<typename OutputEncoding::Encoder> encoderFailureFallback = nullptr)
        {
            ContainerType out;
            Convert<InputEncoding, OutputEncoding, InputType, OutputType, ContainerType>(out, ArrayView<InputType>(src,
                std::char_traits<InputType>::length(src)), decoderFailureFallback, encoderFailureFallback);
            return out;
        }

        /**
         * @brief 编码转换
         * @warning 编码输出不保证以'\0'结尾
         * @tparam InputEncoding 输入编码
         * @tparam OutputEncoding 输出编码
         * @tparam InputType 输入字符类型
         * @tparam OutputType 输出字符类型
         * @param out 转码输出
         * @param src 转码输入
         * @param[in,out] converted 记录转换的个数
         * @param decoderFailureFallback 当解码失败时的处理
         * @param encoderFailureFallback 当编码失败时的处理
         * @return 转换是否成功
         *
         * 当转换遇到失败时，方法会直接退出并返回已经转换的个数。
         */
        template <typename InputEncoding, typename OutputEncoding,
            typename InputType = typename InputEncoding::Decoder::InputType,
            typename OutputType = typename OutputEncoding::Encoder::OutputType>
        bool Convert(MutableArrayView<OutputType> out, ArrayView<InputType> src, size_t* converted = nullptr,
            FailureFallbackCallbackType<typename InputEncoding::Decoder> decoderFailureFallback = nullptr,
            FailureFallbackCallbackType<typename OutputEncoding::Encoder> encoderFailureFallback = nullptr)noexcept
        {
            using DecoderType = typename InputEncoding::Decoder;
            using EncoderType = typename OutputEncoding::Encoder;
            using DecoderInputType = typename InputEncoding::Decoder::InputType;
            using DecoderOutputType = typename InputEncoding::Decoder::OutputType;
            using EncoderInputType = typename OutputEncoding::Encoder::InputType;
            using EncoderOutputType = typename OutputEncoding::Encoder::OutputType;

            static_assert(std::is_same<DecoderOutputType, EncoderInputType>::value,
                "InputEncoder and OutputEncoder mismatched");

            DecoderType decoder;
            EncoderType encoder;
            EncodingResult decoderResult = EncodingResult::Accept;
            uint32_t decoderOutCount = 0, encoderOutCount = 0;
            std::array<DecoderOutputType, InputEncoding::Decoder::kMaxOutputCount> decoderBuffer;
            std::array<EncoderOutputType, OutputEncoding::Encoder::kMaxOutputCount> encoderBuffer;

            size_t processed = 0;

            for (size_t i = 0; i <= src.GetSize(); ++i)
            {
                if (i < src.GetSize())
                    decoderResult = decoder(static_cast<DecoderInputType>(src[i]), decoderBuffer, decoderOutCount);
                else
                {
                    // 此处需要放置EOS的标志
                    decoderResult = decoder(EndOfInputTag(), decoderBuffer, decoderOutCount) ? EncodingResult::Accept :
                        EncodingResult::Reject;
                }

                switch (decoderResult)
                {
                    case EncodingResult::Reject:
                        if (!decoderFailureFallback || !decoderFailureFallback(decoderBuffer, decoderOutCount))
                        {
                            if (converted)
                                *converted = processed;
                            return false;
                        }
                        encoder = EncoderType();  // 需要重置编码器的状态
                        decoderResult = EncodingResult::Accept;
                    case EncodingResult::Accept:
                        for (size_t j = 0; j < decoderOutCount || (j == decoderOutCount && i >= src.GetSize()); ++j)
                        {
                            EncodingResult encoderResult;
                            if (j < decoderOutCount)
                                encoderResult = encoder(decoderBuffer[j], encoderBuffer, encoderOutCount);
                            else
                            {
                                encoderResult = encoder(EndOfInputTag(), encoderBuffer, encoderOutCount) ?
                                    EncodingResult::Accept : EncodingResult::Reject;
                            }

                            switch (encoderResult)
                            {
                                case EncodingResult::Reject:
                                    if (!encoderFailureFallback ||
                                        !encoderFailureFallback(encoderBuffer, encoderOutCount))
                                    {
                                        if (converted)
                                            *converted = processed;
                                        return false;
                                    }
                                case EncodingResult::Accept:
                                    for (uint32_t k = 0; k < encoderOutCount; ++k)
                                    {
                                        if (processed >= out.GetSize())
                                        {
                                            if (converted)
                                                *converted = processed;
                                            return false;
                                        }
                                        out[processed++] = static_cast<OutputType>(encoderBuffer[k]);
                                    }
                                    break;
                                case EncodingResult::Incomplete:
                                default:
                                    break;
                            }
                        }
                        break;
                    case EncodingResult::Incomplete:
                    default:
                        break;
                }
            }

            assert(decoderResult == EncodingResult::Accept);  // 此时解码过程必然已经全部被接受

            if (converted)
                *converted = processed;
            return true;
        }

        /**
         * @brief 编码转换
         * @throw InvalidEncodingException 当转换无效时抛出
         * @tparam Encoder 目标编码器（或者解码器）
         * @tparam InputType 输入字符类型
         * @tparam OutputType 输出字符类型
         * @tparam ContainerType 容器类型
         * @param out 转码输出
         * @param src 转码输入
         * @param failureFallback 当解码失败时的处理
         * @return out对象的引用
         *
         * 该函数为单向转换，模板参数Encoding可以为某个编码器或者解码器。
         */
        template <typename Encoder, typename InputType = typename Encoder::InputType,
            typename OutputType = typename Encoder::OutputType, typename ContainerType = std::basic_string<OutputType>>
        ContainerType& Convert(ContainerType& out, ArrayView<InputType> src,
            FailureFallbackCallbackType<Encoder> failureFallback = nullptr)
        {
            using EncoderInputType = typename Encoder::InputType;
            using EncoderOutputType = typename Encoder::OutputType;

            Encoder encoder;
            EncodingResult result = EncodingResult::Accept;
            uint32_t encoded = 0;
            std::array<EncoderOutputType, Encoder::kMaxOutputCount> buffer;

            out.clear();
            out.reserve(src.GetSize());

            for (size_t i = 0; i <= src.GetSize(); ++i)
            {
                if (i < src.GetSize())
                    result = encoder(static_cast<EncoderInputType>(src[i]), buffer, encoded);
                else
                {
                    // 此处需要放置EOS的标志
                    result = encoder(EndOfInputTag(), buffer, encoded) ? EncodingResult::Accept :
                        EncodingResult::Reject;
                }

                switch (result)
                {
                    case EncodingResult::Reject:
                        if (!failureFallback || !failureFallback(buffer, encoded))
                            MOE_THROW(InvalidEncodingException, "Cannot encode character near {0}", i);
                        result = EncodingResult::Accept;
                    case EncodingResult::Accept:
                        for (uint32_t j = 0; j < encoded; ++j)
                            out.push_back(static_cast<OutputType>(buffer[j]));
                        break;
                    case EncodingResult::Incomplete:
                    default:
                        break;
                }
            }

            assert(result == EncodingResult::Accept);  // 此时解码过程必然已经全部被接受
            return out;
        }

        /**
         * @brief 编码转换
         * @throw InvalidEncodingException 当转换无效时抛出
         * @tparam Encoder 目标编码器（或者解码器）
         * @tparam InputType 输入字符类型
         * @tparam OutputType 输出字符类型
         * @tparam ContainerType 容器类型
         * @param out 转码输出
         * @param src 转码输入
         * @param failureFallback 当解码失败时的处理
         * @return out对象的引用
         *
         * 该函数为单向转换，模板参数Encoding可以为某个编码器或者解码器。
         */
        template <typename Encoder, typename InputType = typename Encoder::InputType,
            typename OutputType = typename Encoder::OutputType, typename ContainerType = std::basic_string<OutputType>>
        ContainerType Convert(ArrayView<InputType> src,
            FailureFallbackCallbackType<Encoder> failureFallback = nullptr)
        {
            ContainerType out;
            Convert<Encoder, InputType, OutputType, ContainerType>(out, src, failureFallback);
            return out;
        }

        /**
         * @brief 编码转换
         * @throw InvalidEncodingException 当转换无效时抛出
         * @tparam Encoder 目标编码器（或者解码器）
         * @tparam InputType 输入字符类型
         * @tparam OutputType 输出字符类型
         * @tparam ContainerType 容器类型
         * @param out 转码输出
         * @param src 转码输入
         * @param failureFallback 当解码失败时的处理
         * @return out对象的引用
         *
         * 该函数为单向转换，模板参数Encoding可以为某个编码器或者解码器。
         */
        template <typename Encoder, typename InputType = typename Encoder::InputType,
            typename OutputType = typename Encoder::OutputType, typename ContainerType = std::basic_string<OutputType>>
        ContainerType Convert(const std::basic_string<InputType>& src,
            FailureFallbackCallbackType<Encoder> failureFallback = nullptr)
        {
            ContainerType out;
            Convert<Encoder, InputType, OutputType, ContainerType>(out, ArrayView<InputType>(src.data(), src.length()),
                failureFallback);
            return out;
        }

        /**
         * @brief 编码转换
         * @throw InvalidEncodingException 当转换无效时抛出
         * @tparam Encoder 目标编码器（或者解码器）
         * @tparam InputType 输入字符类型
         * @tparam OutputType 输出字符类型
         * @tparam ContainerType 容器类型
         * @param out 转码输出
         * @param src 转码输入
         * @param failureFallback 当解码失败时的处理
         * @return out对象的引用
         *
         * 该函数为单向转换，模板参数Encoding可以为某个编码器或者解码器。
         */
        template <typename Encoder, typename InputType = typename Encoder::InputType,
            typename OutputType = typename Encoder::OutputType, typename ContainerType = std::basic_string<OutputType>>
        ContainerType Convert(const InputType* src,
            FailureFallbackCallbackType<Encoder> failureFallback = nullptr)
        {
            ContainerType out;
            Convert<Encoder, InputType, OutputType, ContainerType>(out, ArrayView<InputType>(src,
                std::char_traits<InputType>::length(src)), failureFallback);
            return out;
        }
    }
}
