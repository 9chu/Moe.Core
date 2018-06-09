/**
 * @file
 * @author chu
 * @date 2018/6/7
 * @see http://ietf.org/rfc/rfc3492.txt
 * @see https://github.com/bestiejs/punycode.js/blob/master/punycode.js
 * @see https://github.com/jcranmer/idna-uts46
 */
#include <Moe.Core/Idna.hpp>

using namespace std;
using namespace moe;

//////////////////////////////////////////////////////////////////////////////// Punycode

// Punycode所用的Bootstring参数
static const unsigned kPunycodeBase = 36;
static const unsigned kPunycodeTmin = 1;
static const unsigned kPunycodeTmax = 26;
static const unsigned kPunycodeSkew = 38;
static const unsigned kPunycodeDamp = 700;
static const unsigned kPunycodeInitialBias = 72;
static const unsigned kPunycodeInitialN = 0x80;
static const unsigned kPunycodeDelimiter = 0x2D;  // ascii: '-'

static constexpr bool IsPunycodeDelim(char32_t ch)noexcept
{
    return ch == kPunycodeDelimiter;
}

static constexpr bool IsPunycodeBasicCodepoint(char32_t ch)noexcept
{
    return static_cast<uint32_t>(ch) < 0x80;
}

static constexpr uint32_t PunycodeEncodeDigit(uint32_t ch)noexcept
{
    /*  0..25 map to ASCII a..z or A..Z */
    /* 26..35 map to ASCII 0..9         */
    return ch + 22 + 75 * (ch < 26);
}

static constexpr uint32_t PunycodeDecodeDigit(uint32_t ch)noexcept
{
    return ch - 48 < 10 ? ch - 22 : (ch - 65 < 26 ? ch - 65 : (ch - 97 < 26 ? ch - 97 : kPunycodeBase));
}

static char32_t PunycodeAdapt(uint32_t delta, uint32_t numpoints, bool firsttime)noexcept
{
    uint32_t k = 0;
    delta = firsttime ? delta / kPunycodeDamp : delta >> 1;
    delta += delta / numpoints;

    for (; delta > ((kPunycodeBase - kPunycodeTmin) * kPunycodeTmax) / 2; k += kPunycodeBase)
        delta /= (kPunycodeBase - kPunycodeTmin);

    return k + (kPunycodeBase - kPunycodeTmin + 1) * delta / (delta + kPunycodeSkew);
}

void Idna::PunycodeEncode(u32string& out, ArrayView<char32_t> input)
{
    out.clear();
    out.reserve(input.GetSize());

    // 处理基本码点
    for (size_t i = 0; i < input.GetSize(); ++i)
    {
        if (IsPunycodeBasicCodepoint(input[i]))
            out.push_back(input[i]);
        else if (input[i] < kPunycodeInitialN)  // 当char32_t为signed类型时
            MOE_THROW(BadFormatException, "Invalid punycode input near position {0}", i);
    }

    size_t h = out.length();  // 已处理的码点数量
    size_t b = h;  // 基本码点数量

    // 基本码点和后面编码的部分之间用'-'分割
    if (b > 0)
        out.push_back(kPunycodeDelimiter);

    uint32_t n = kPunycodeInitialN, delta = 0, bias = kPunycodeInitialBias;
    while (h < input.GetSize())
    {
        // 所有`码点 < n`的字符都已经处理了，找下一个更大的
        auto m = numeric_limits<uint32_t>::max();
        for (size_t i = 0; i < input.GetSize(); ++i)
        {
            if (input[i] >= n && input[i] < m)
                m = static_cast<uint32_t>(input[i]);
        }

        if (m - n > (numeric_limits<uint32_t>::max() - delta) / (h + 1))
            MOE_THROW(BadFormatException, "Punycode overflowed");
        delta += (m - n) * (h + 1);
        n = m;

        for (size_t i = 0; i < input.GetSize(); ++i)
        {
            if (input[i] < n)
            {
                if (++delta == 0)
                    MOE_THROW(BadFormatException, "Punycode overflowed");
            }

            if (input[i] == n)
            {
                // 将delta编码成变长整数
                uint32_t q = delta;
                for (uint32_t k = kPunycodeBase; ; k += kPunycodeBase)
                {
                    uint32_t t = k <= bias ? kPunycodeTmin : (k >= bias + kPunycodeTmax ? kPunycodeTmax : k - bias);
                    if (q < t)
                        break;
                    out.push_back(static_cast<char32_t>(PunycodeEncodeDigit(t + (q - t) % (kPunycodeBase - t))));
                    q = (q - t) / (kPunycodeBase - t);
                }

                out.push_back(static_cast<char32_t>(PunycodeEncodeDigit(q)));
                bias = PunycodeAdapt(delta, static_cast<uint32_t>(h + 1), h == b);
                delta = 0;
                ++h;
            }
        }

        ++delta, ++n;
    }
}

void Idna::PunycodeDecode(u32string& out, ArrayView<char32_t> input)
{
    out.clear();
    out.reserve(input.GetSize());

    // 寻找简单串(Basic string)中的基本码点(Basic code points)分隔符
    size_t b = 0;
    for (size_t i = 0; i < input.GetSize(); ++i)
    {
        if (IsPunycodeDelim(input[i]))
            b = i;
    }

    for (size_t i = 0; i < b; ++i)
    {
        if (!IsPunycodeBasicCodepoint(input[i]))
            MOE_THROW(BadFormatException, "Invalid punycode character near position {0}", i);
        out.push_back(input[i]);
    }

    // 从非基本码点开始进行处理
    uint32_t i = 0, bias = kPunycodeInitialBias, n = kPunycodeInitialN;
    for (size_t in = b > 0 ? b + 1 : 0; in < input.GetSize(); )
    {
        // 解码一个变长整数成索引，然后加到i上
        auto oldi = i;
        for (uint32_t w = 1, k = kPunycodeBase; ; k += kPunycodeBase)
        {
            if (in >= input.GetSize())
                MOE_THROW(BadFormatException, "Invalid input punycode");
            auto digit = PunycodeDecodeDigit(static_cast<uint32_t>(input[in++]));
            if (digit >= kPunycodeBase)
                MOE_THROW(BadFormatException, "Invalid input punycode");
            if (digit > (numeric_limits<uint32_t>::max() - i) / w)
                MOE_THROW(BadFormatException, "Punycode overflowed");
            i += digit * w;
            uint32_t t = k <= bias ? kPunycodeTmin : (k >= bias + kPunycodeTmax ? kPunycodeTmax : k - bias);
            if (digit < t)
                break;
            if (w > numeric_limits<uint32_t>::max() / (kPunycodeBase - t))
                MOE_THROW(BadFormatException, "Punycode overflowed");
            w *= (kPunycodeBase - t);
        }

        auto outlen = static_cast<uint32_t>(out.length() + 1);
        bias = PunycodeAdapt(i - oldi, outlen, oldi == 0);

        if (i / outlen > numeric_limits<uint32_t>::max() - n)
            MOE_THROW(BadFormatException, "Punycode overflowed");
        n += i / outlen;
        i %= outlen;

        out.insert(i++, 1, static_cast<char32_t>(n));
    }
}

//////////////////////////////////////////////////////////////////////////////// IDNA
