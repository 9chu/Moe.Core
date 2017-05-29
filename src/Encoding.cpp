/**
 * @file
 * @date 2017/5/28
 */
#include <Moe.Core/Encoding.hpp>

using namespace std;
using namespace moe;
using namespace Encoding;

//////////////////////////////////////////////////////////////////////////////// UTF8

EncodingResult UTF8::Decoder::operator()(char ch, char32_t& out)noexcept
{
    // http://bjoern.hoehrmann.de/utf-8/decoder/dfa/
    static const uint8_t utf8dfa[] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
        8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
        0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
        0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
        0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
        1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
        1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
        1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
    };

    uint8_t b = static_cast<uint8_t>(ch);
    uint32_t type = utf8dfa[b];
    out = (m_iState != 0) ? (b & 0x3Fu) | (out << 6) : (0xFFu >> type) & b;
    m_iState = utf8dfa[256 + m_iState * 16 + type];

    switch (m_iState)
    {
        case 0:
            return EncodingResult::Accept;
        case 1:
            m_iState = 0;
            return EncodingResult::Reject;
        default:
            return EncodingResult::Incomplete;
    }
}

EncodingResult UTF8::Encoder::operator()(char32_t ch, char out[], uint32_t& cnt)noexcept
{
    uint32_t cp = static_cast<uint32_t>(ch);

    if (cp <= 0x7Fu)
    {
        out[0] = static_cast<char>(cp & 0xFFu);
        cnt = 1;
    }
    else if (cp <= 0x7FFu)
    {
        out[0] = static_cast<char>(0xC0u | ((cp >> 6) & 0xFFu));
        out[1] = static_cast<char>(0x80u | (cp & 0x3Fu));
        cnt = 2;
    }
    else if (cp <= 0xFFFFu)
    {
        out[0] = static_cast<char>(0xE0u | ((cp >> 12) & 0xFFu));
        out[1] = static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu));
        out[2] = static_cast<char>(0x80u | (cp & 0x3Fu));
        cnt = 3;
    }
    else if (cp <= 0x10FFFFu)
    {
        out[0] = static_cast<char>(0xF0u | ((cp >> 18) & 0xFFu));
        out[1] = static_cast<char>(0x80u | ((cp >> 12) & 0x3Fu));
        out[2] = static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu));
        out[3] = static_cast<char>(0x80u | (cp & 0x3Fu));
        cnt = 4;
    }
    else
        return EncodingResult::Reject;

    return EncodingResult::Accept;
}

//////////////////////////////////////////////////////////////////////////////// UTF16

EncodingResult UTF16::Decoder::operator()(char16_t ch, char32_t& out)noexcept
{
    uint16_t word = static_cast<uint16_t>(ch);

    switch (m_iState)
    {
        case 0:
            if (word < 0xD800u || word > 0xDFFFu)
            {
                out = word;
                return EncodingResult::Accept;
            }
            else if (word <= 0xDBFFu)
            {
                m_iLastWord = word;
                m_iState = 1;
                return EncodingResult::Incomplete;
            }
            else
                return EncodingResult::Reject;
        case 1:
            if (!(word >= 0xDC00u && word <= 0xDFFFu))
            {
                m_iState = 0;
                return EncodingResult::Reject;
            }
            out = (m_iLastWord & 0x3FFu) << 10;
            out |= word & 0x3FFu;
            out += 0x10000u;
            return EncodingResult::Accept;
        default:
            assert(false);
            return EncodingResult::Reject;
    }
}

Encoding::EncodingResult Encoding::UTF16::Encoder::operator()(char32_t ch, char16_t out[], uint32_t& cnt)noexcept
{
    uint32_t cp = static_cast<uint32_t>(ch);
    if (cp <= 0xFFFFu)
    {
        out[0] = static_cast<char16_t>(cp);
        cnt = 1;
    }
    else if (cp <= 0x10FFFFu)
    {
        cp -= 0x10000u;
        out[0] = static_cast<char16_t>(0xD800u | (cp >> 10));
        out[1] = static_cast<char16_t>(0xDC00u | (cp & 0x3FFu));
        cnt = 2;
    }
    else
        return EncodingResult::Reject;

    return EncodingResult::Accept;
}
