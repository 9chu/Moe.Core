/**
 * @file
 * @author chu
 * @date 2018/6/7
 * @see http://ietf.org/rfc/rfc3492.txt
 * @see https://tools.ietf.org/html/rfc5892#appendix-A
 * @see https://github.com/bestiejs/punycode.js/blob/master/punycode.js
 * @see https://github.com/jcranmer/idna-uts46
 * @see https://github.com/kjd/idna
 */
#include <Moe.Core/Idna.hpp>
#include <Moe.Core/Unicode.hpp>

#include <algorithm>

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
            MOE_THROW(BadFormatException, "Invalid punycode input near position {0}: {1}", i, StringUtils::Repr(out));
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
            MOE_THROW(BadFormatException, "Punycode overflowed: {0}", StringUtils::Repr(out));
        delta += (m - n) * (h + 1);
        n = m;

        for (size_t i = 0; i < input.GetSize(); ++i)
        {
            if (input[i] < n)
            {
                if (++delta == 0)
                    MOE_THROW(BadFormatException, "Punycode overflowed: {0}", StringUtils::Repr(out));
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
        {
            MOE_THROW(BadFormatException, "Invalid punycode character near position {0}: {1}", i,
                StringUtils::Repr(out));
        }
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
                MOE_THROW(BadFormatException, "Invalid input punycode: {0}", StringUtils::Repr(out));
            auto digit = PunycodeDecodeDigit(static_cast<uint32_t>(input[in++]));
            if (digit >= kPunycodeBase)
                MOE_THROW(BadFormatException, "Invalid input punycode: {0}", StringUtils::Repr(out));
            if (digit > (numeric_limits<uint32_t>::max() - i) / w)
                MOE_THROW(BadFormatException, "Punycode overflowed: {0}", StringUtils::Repr(out));
            i += digit * w;
            uint32_t t = k <= bias ? kPunycodeTmin : (k >= bias + kPunycodeTmax ? kPunycodeTmax : k - bias);
            if (digit < t)
                break;
            if (w > numeric_limits<uint32_t>::max() / (kPunycodeBase - t))
                MOE_THROW(BadFormatException, "Punycode overflowed: {0}", StringUtils::Repr(out));
            w *= (kPunycodeBase - t);
        }

        auto outlen = static_cast<uint32_t>(out.length() + 1);
        bias = PunycodeAdapt(i - oldi, outlen, oldi == 0);

        if (i / outlen > numeric_limits<uint32_t>::max() - n)
            MOE_THROW(BadFormatException, "Punycode overflowed: {0}", StringUtils::Repr(out));
        n += i / outlen;
        i %= outlen;

        out.insert(i++, 1, static_cast<char32_t>(n));
    }
}

//////////////////////////////////////////////////////////////////////////////// IDNA

enum {
    IDNA_STATUS_VALID = 0,
    IDNA_STATUS_VALID_NV8 = 1,
    IDNA_STATUS_VALID_XV8 = 2,
    IDNA_STATUS_DISALLOWED = 3,
    IDNA_STATUS_IGNORED = 4,
    IDNA_STATUS_MAPPED = 5,
    IDNA_STATUS_DEVIATION = 6,
    IDNA_STATUS_DISALLOWED_STD3_VALID = 7,
    IDNA_STATUS_DISALLOWED_STD3_MAPPED = 8,
    IDNA_STATUS_MASK = 15,
    IDNA_PVALID = 1 << 4,
    IDNA_CONTEXT_J = 2 << 4,
    IDNA_CONTEXT_O = 3 << 4,
    IDNA_JOINER_CHECK_MASK = 3 << 4,
    IDNA_EXTRA_MAPPING = 1 << 7,
};

struct IdnaRecord
{
    uint8_t Flags;
    uint32_t Mapping;
};

#include "IdnaData.inl"

static const char32_t kDeliminators[] = { '.' };
static const char32_t kPunycodePrefix[] = { 'x', 'n', '-', '-', '\0' };

static const uint32_t kViramaCombiningClass = 9;

static const IdnaRecord& GetIdnaRecord(char32_t ch)noexcept
{
    unsigned index = 0;
    const auto mask = (1u << kIdnaRecordsIndexShift) - 1;
    if (static_cast<uint32_t>(ch) < kIdnaRecordsCount)
    {
        index = kIdnaRecordsIndex1[ch >> kIdnaRecordsIndexShift];
        index = kIdnaRecordsIndex2[(index << kIdnaRecordsIndexShift) + (ch & mask)];
    }
    return kIdnaRecords[index];
}

static ArrayView<char32_t> GetIdnaMapping(const IdnaRecord& record)noexcept
{
    if (record.Flags & IDNA_EXTRA_MAPPING)
    {
        auto index = record.Mapping & 0xFFFF;
        auto count = (record.Mapping >> 16) & 0xFFFF;
        return ArrayView<char32_t>(&kIdnaMappingData[index], count);
    }
    else if (record.Mapping != 0)
    {
        static_assert(sizeof(IdnaRecord::Mapping) == sizeof(char32_t), "Bad condition");
        return ArrayView<char32_t>(reinterpret_cast<const char32_t*>(&record.Mapping), 1);
    }
    return ArrayView<char32_t>();
}

static bool IsPureAscii(ArrayView<char32_t> input)noexcept
{
    for (size_t i = 0; i < input.GetSize(); ++i)
    {
        if (static_cast<uint32_t>(input[i]) >= 0x80)
            return false;
    }
    return true;
}

static void CheckBidi(ArrayView<char32_t> input, bool checkLtr=false)
{
    bool bidiLabel = false;
    for (size_t i = 0; i < input.GetSize(); ++i)
    {
        auto direction = Unicode::GetBidirectional(input[i]);
        if (!direction || direction[0] == '\0')
            MOE_THROW(BadFormatException, "Unknown directionality at position {0}: {1}", i, StringUtils::Repr(input));
        if (::strcmp(direction, "R") == 0 || ::strcmp(direction, "AL") == 0 || ::strcmp(direction, "AN") == 0)
        {
            bidiLabel = true;
            break;
        }
    }
    if (!bidiLabel && !checkLtr)
        return;

    // Bidi rule 1
    bool rtl = false;
    if (input.GetSize() > 0)
    {
        auto direction = Unicode::GetBidirectional(input[0]);
        if (::strcmp(direction, "R") == 0 || ::strcmp(direction, "AL") == 0)
            rtl = true;
        else if (::strcmp(direction, "L") != 0)
        {
            MOE_THROW(BadFormatException, "First codepoint in label must be directionality L, R or AL: {0}",
                StringUtils::Repr(input));
        }
    }

    bool validEnding = false;
    const char* numberType = nullptr;
    for (size_t i = 0; i < input.GetSize(); ++i)
    {
        auto direction = Unicode::GetBidirectional(input[i]);
        if (rtl)
        {
            // Bidi rule 2
            if (!(::strcmp(direction, "R") == 0 || ::strcmp(direction, "AL") == 0 || ::strcmp(direction, "AN") == 0 ||
                ::strcmp(direction, "EN") == 0 || ::strcmp(direction, "ES") == 0 || ::strcmp(direction, "CS") == 0 ||
                ::strcmp(direction, "ET") == 0 || ::strcmp(direction, "ON") == 0 || ::strcmp(direction, "BN") == 0 ||
                ::strcmp(direction, "NSM") == 0))
            {
                MOE_THROW(BadFormatException, "Invalid direction for codepoint at position {0}: {1}", i,
                    StringUtils::Repr(input));
            }

            // Bidi rule 3
            if (::strcmp(direction, "R") == 0 || ::strcmp(direction, "AL") == 0 || ::strcmp(direction, "EN") == 0 ||
                ::strcmp(direction, "AN") == 0)
            {
                validEnding = true;
            }
            else if (::strcmp(direction, "NSM") != 0)
                validEnding = false;

            // Bidi rule 4
            if (::strcmp(direction, "AN") == 0 || ::strcmp(direction, "EN") == 0)
            {
                if (!numberType)
                    numberType = direction;
                else if (direction != numberType)
                {
                    MOE_THROW(BadFormatException, "Can not mix numeral types in a right-to-left label: {0}",
                        StringUtils::Repr(input));
                }
            }
        }
        else
        {
            // Bidi rule 5
            if (!(::strcmp(direction, "L") == 0 || ::strcmp(direction, "EN") == 0 || ::strcmp(direction, "ES") == 0 ||
                ::strcmp(direction, "CS") == 0 || ::strcmp(direction, "ET") == 0 || ::strcmp(direction, "ON") == 0 ||
                ::strcmp(direction, "BN") == 0 || ::strcmp(direction, "NSM") == 0))
            {
                MOE_THROW(BadFormatException, "Invalid direction for codepoint at position {0}: {1}", i,
                    StringUtils::Repr(input));
            }

            // Bidi rule 6
            if (::strcmp(direction, "L") == 0 || ::strcmp(direction, "EN") == 0)
                validEnding = true;
            else if (::strcmp(direction, "NSM") != 0)
                validEnding = false;
        }
    }

    if (!validEnding)
    {
        MOE_THROW(BadFormatException, "Label ends with illegal codepoint directionality: {0}",
            StringUtils::Repr(input));
    }
}

static bool ValidateContextJ(ArrayView<char32_t> input, size_t pos)
{
    auto ch = input[pos];
    if (ch == 0x200C)
    {
        if (pos > 0)
        {
            if (Unicode::GetCombining(input[pos - 1]) == kViramaCombiningClass)
                return true;
        }

        bool ok = false;
        size_t i = pos;
        while (i-- > 0)
        {
            auto joiningType = GetJoiningType(input[i]);
            if (joiningType == 'T')
                continue;
            else if (joiningType == 'L' || joiningType == 'D')
            {
                ok = true;
                break;
            }
        }

        if (!ok)
            return false;

        for (i = pos + 1; i < input.GetSize(); ++i)
        {
            auto joiningType = GetJoiningType(input[i]);
            if (joiningType == 'T')
                continue;
            else if (joiningType == 'R' || joiningType == 'D')
                return true;
        }
    }
    else if (ch == 0x200D)
    {
        if (pos > 0)
            return Unicode::GetCombining(input[pos - 1]) == kViramaCombiningClass;
    }
    return false;
}

static bool ValidateContextO(ArrayView<char32_t> input, size_t pos)
{
    auto ch = input[pos];
    if (ch == 0x00B7)
    {
        if (0 < pos && pos < input.GetSize() - 1)
        {
            if (input[pos - 1] == 0x006C && input[pos + 1] == 0x006C)
                return true;
        }
        return false;
    }
    else if (ch == 0x0375)
    {
        if (pos < input.GetSize() - 1 && input.GetSize() > 1)
            return IsInGreekScript(input[pos + 1]);
        return false;
    }
    else if (ch == 0x05F3 || ch == 0x05F4)
    {
        if (pos > 0)
            return IsInHebrewScript(input[pos - 1]);
        return false;
    }
    else if (ch == 0x30FB)
    {
        for (size_t i = 0; i < input.GetSize(); ++i)
        {
            ch = input[i];
            if (ch == 0x30FB)
                continue;
            else if (IsInHiraganaScript(ch) || IsInKatakanaScript(ch) || IsInHanScript(ch))
                return true;
        }
        return false;
    }
    else if (0x0660 <= ch && ch <= 0x0669)
    {
        for (size_t i = 0; i < input.GetSize(); ++i)
        {
            ch = input[i];
            if (0x06F0 <= ch && ch <= 0x06F9)
                return false;
        }
        return true;
    }
    else if (0x06F0 <= ch && ch <= 0x06F9)
    {
        for (size_t i = 0; i < input.GetSize(); ++i)
        {
            ch = input[i];
            if (0x0660 <= ch && ch <= 0x0669)
                return false;
        }
        return true;
    }
    assert(false);
    return false;
}

static void Validate(ArrayView<char32_t> input, bool useStd3Rules, bool checkHyphens, bool checkBidi, bool checkJoiners,
    bool transitional)
{
    // Step1 检查NFC
    if (!Unicode::IsNormalized(input, Unicode::NormalizationFormType::NFC))
        MOE_THROW(BadFormatException, "Label must be in Normalization Form C: {0}", StringUtils::Repr(input));

    // Step2&3 检查Hyphens
    if (checkHyphens)
    {
        if (input.GetSize() >= 4 && input[2] == '-' && input[3] == '-')
        {
            MOE_THROW(BadFormatException, "Label has disallowed hyphens in 3rd and 4th position: {0}",
                StringUtils::Repr(input));
        }
        if (input.GetSize() > 0 && (input.First() == '-' || input.Last() == '-'))
            MOE_THROW(BadFormatException, "Label must not start or end with a hyphen: {0}", StringUtils::Repr(input));
    }

    // Step4 不能包含'.'
    for (size_t i = 0; i < input.GetSize(); ++i)
    {
        if (input[i] == '.')
        {
            MOE_THROW(BadFormatException, "Label cannot contains a U+002E (.) FULL STOP: {0}",
                StringUtils::Repr(input));
        }
    }

    // Step5 不能以General_Category=Mark打头
    if (input.GetSize() > 0)
    {
        auto category = Unicode::GetCategory(input.First());
        if (category && category[0] == 'M')
        {
            MOE_THROW(BadFormatException, "Label begins with an illegal combining character: {0}",
                StringUtils::Repr(input));
        }
    }

    // Step6 每一个码点必须符合映射关系
    for (size_t i = 0; i < input.GetSize(); ++i)
    {
        char32_t ch = input[i];
        const auto& record = GetIdnaRecord(ch);

        auto status = record.Flags & IDNA_STATUS_MASK;
        if ((status == IDNA_STATUS_VALID || status == IDNA_STATUS_VALID_NV8 || status == IDNA_STATUS_VALID_XV8) ||
            ((status == IDNA_STATUS_DEVIATION) && !transitional) ||
            ((status == IDNA_STATUS_DISALLOWED_STD3_VALID) && !useStd3Rules))
        {
            continue;
        }

        MOE_THROW(BadFormatException, "Label contains bad codepoint at position {0}: {1}", i, StringUtils::Repr(input));
    }

    // Step7 CheckJoiners
    if (checkJoiners)
    {
        for (size_t i = 0; i < input.GetSize(); ++i)
        {
            char32_t ch = input[i];
            const auto& record = GetIdnaRecord(ch);

            if ((record.Flags & IDNA_JOINER_CHECK_MASK) == IDNA_PVALID)
                continue;
            else if ((record.Flags & IDNA_JOINER_CHECK_MASK) == IDNA_CONTEXT_J)
            {
                if (!ValidateContextJ(input, i))
                {
                    MOE_THROW(BadFormatException, "Joiner 0x{0:H} not allowed at position {1}: {2}", ch, i,
                        StringUtils::Repr(input));
                }
            }
            else if ((record.Flags & IDNA_JOINER_CHECK_MASK) == IDNA_CONTEXT_O)
            {
                if (!ValidateContextO(input, i))
                {
                    MOE_THROW(BadFormatException, "Codepoint not allowed at position {0}: {1}", i,
                        StringUtils::Repr(input));
                }
            }
        }
    }

    // Step8 CheckBidi
    if (checkBidi)
        CheckBidi(input);
}

static void Uts46Remap(u32string& out, ArrayView<char32_t> domain, bool useStd3Rules, bool transitional)
{
    out.clear();
    out.reserve(domain.GetSize());

    for (size_t i = 0; i < domain.GetSize(); ++i)
    {
        char32_t ch = domain[i];
        const auto& record = GetIdnaRecord(ch);
        auto status = record.Flags & IDNA_STATUS_MASK;
        if ((status == IDNA_STATUS_VALID || status == IDNA_STATUS_VALID_NV8 || status == IDNA_STATUS_VALID_XV8) ||
            ((status == IDNA_STATUS_DEVIATION) && !transitional) ||
            ((status == IDNA_STATUS_DISALLOWED_STD3_VALID) && !useStd3Rules))
        {
            out.push_back(ch);
        }
        else if ((status == IDNA_STATUS_MAPPED) || ((status == IDNA_STATUS_DEVIATION) && transitional) ||
            ((status == IDNA_STATUS_DISALLOWED_STD3_MAPPED) && !useStd3Rules))
        {
            ArrayView<char32_t> mapped = GetIdnaMapping(record);

            if (!mapped.IsEmpty())
                out.append(mapped.GetBuffer(), mapped.GetSize());
            else
                assert(false);
        }
        else if (!(status == IDNA_STATUS_IGNORED))
            MOE_THROW(BadFormatException, "Invalid codepoint near position {0}: {1}", i, StringUtils::Repr(domain));
    }
}

static void Process(std::u32string& out, ArrayView<char32_t> input, bool useStd3rules, bool checkHyphens,
    bool checkBidi, bool checkJoiners, bool transitional)
{
    out.clear();
    out.reserve(input.GetSize());

    // Step1 进行字符串映射
    u32string tmp;
    Uts46Remap(tmp, input, useStd3rules, transitional);

    // Step2 进行Normalize
    u32string buffer;
    Unicode::Normalize(buffer, tmp, Unicode::NormalizationFormType::NFC);

    // Step3 按照'.'进行分割
    auto i = 0;
    auto cnt = std::count(buffer.begin(), buffer.end(), kDeliminators[0]);
    auto it = StringUtils::SplitByCharsBegin(ToArrayView<char32_t>(buffer), ArrayView<char32_t>(kDeliminators, 1));
    while (it != StringUtils::SplitByCharsEnd<char32_t>())
    {
        auto label = *it;

        if (i != 0 && !(i >= cnt && label.GetSize() == 0))  // 尾随的小数点要去掉
            out.push_back('.');

        // Step4 转换或者校验
        if (label.GetSize() >= 4 && char_traits<char32_t>::compare(label.GetBuffer(), U"xn--", 4) == 0)
        {
            Idna::PunycodeDecode(tmp, ArrayView<char32_t>(label.GetBuffer() + 4, label.GetSize() - 4));
            Validate(ToArrayView<char32_t>(tmp), useStd3rules, checkHyphens, checkBidi, checkJoiners, transitional);
            out.append(tmp);
        }
        else
        {
            Validate(label, useStd3rules, checkHyphens, checkBidi, checkJoiners, transitional);
            out.append(label.GetBuffer(), label.GetSize());
        }

        ++i;
        ++it;
    }
}

void Idna::ToAscii(std::u32string& out, ArrayView<char32_t> domainName, bool checkHyphens, bool checkBidi,
    bool checkJoiners, bool useStd3Rules, bool transitionalProcessing, bool verifyDnsLength)
{
    out.clear();
    out.reserve(domainName.GetSize());

    u32string tmp;
    Process(tmp, domainName, useStd3Rules, checkHyphens, checkBidi, checkJoiners, transitionalProcessing);

    auto i = 0;
    auto cnt = verifyDnsLength ? std::count(tmp.begin(), tmp.end(), kDeliminators[0]) : 0;
    auto totalLength = 0;
    auto it = StringUtils::SplitByCharsBegin(ToArrayView<char32_t>(tmp), ArrayView<char32_t>(kDeliminators, 1));
    while (it != StringUtils::SplitByCharsEnd<char32_t>())
    {
        auto label = *it;

        if (i != 0)
            out.push_back('.');

        if (IsPureAscii(label))
        {
            if (verifyDnsLength)
            {
                auto len = label.GetSize();
                if (len > 63 || len < 1)
                    MOE_THROW(BadFormatException, "Bad label length: {0}", StringUtils::Repr(label));
                if (i + 1 <= cnt)
                    totalLength += len;
            }

            out.append(label.GetBuffer(), label.GetSize());
        }
        else
        {
            u32string punycode;
            PunycodeEncode(punycode, label);

            if (verifyDnsLength)
            {
                auto len = punycode.length() + 4;
                if (len > 63 || len < 1)
                    MOE_THROW(BadFormatException, "Label is too long or empty: {0}", StringUtils::Repr(label));
                if (i + 1 <= cnt)
                    totalLength += len;
            }

            out.append(kPunycodePrefix);
            out.append(punycode);
        }

        ++i;
        ++it;
    }

    if (verifyDnsLength && (totalLength > 253 || totalLength < 1))
        MOE_THROW(BadFormatException, "Domain is too long or empty: {0}", StringUtils::Repr(domainName));
}

void Idna::ToUnicode(std::u32string& out, ArrayView<char32_t> domainName, bool checkHyphens, bool checkBidi,
    bool checkJoiners, bool useStd3Rules, bool transitionalProcessing)
{
    Process(out, domainName, useStd3Rules, checkHyphens, checkBidi, checkJoiners, transitionalProcessing);
}
