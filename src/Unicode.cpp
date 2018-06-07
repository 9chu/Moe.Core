/**
 * @file
 * @author chu
 * @date 2018/5/17
 */
#include <Moe.Core/Unicode.hpp>
#include <Moe.Core/Convert.hpp>
#include <Moe.Core/Utils.hpp>

#include <cstring>

#include "UnicodeData.inl"

using namespace std;
using namespace moe;

enum UnicodeTypeMask
{
    UNICODE_TYPEMASK_ALPHA = 0x01,
    UNICODE_TYPEMASK_DECIMAL = 0x02,
    UNICODE_TYPEMASK_DIGIT = 0x04,
    UNICODE_TYPEMASK_LOWER = 0x08,
    UNICODE_TYPEMASK_LINEBREAK = 0x10,
    UNICODE_TYPEMASK_SPACE = 0x20,
    UNICODE_TYPEMASK_TITLE = 0x40,
    UNICODE_TYPEMASK_UPPER = 0x80,
    UNICODE_TYPEMASK_XID_START = 0x100,
    UNICODE_TYPEMASK_XID_CONTINUE = 0x200,
    UNICODE_TYPEMASK_PRINTABLE = 0x400,
    UNICODE_TYPEMASK_NUMERIC = 0x800,
    UNICODE_TYPEMASK_CASE_IGNORABLE = 0x1000,
    UNICODE_TYPEMASK_CASED = 0x2000,
    UNICODE_TYPEMASK_EXTENDED_CASE = 0x4000,
};

static const UnicodeDatabaseRecord& GetUnicodeDatabaseRecord(char32_t ch)noexcept
{
    unsigned index = 0;
    const auto mask = (1u << kUnicodeDatabaseRecordsIndexShift) - 1;
    if (static_cast<uint32_t>(ch) < kUnicodeCodePointCount)
    {
        index = kUnicodeDatabaseRecordsIndex1[(ch >> kUnicodeDatabaseRecordsIndexShift)];
        index = kUnicodeDatabaseRecordsIndex2[(index << kUnicodeDatabaseRecordsIndexShift) + (ch & mask)];
    }
    return kUnicodeDatabaseRecords[index];
}

static const UnicodeTypeRecord& GetUnicodeTypeRecord(char32_t ch)noexcept
{
    unsigned index = 0;
    const auto mask = (1u << kUnicodeTypeRecordsIndexShift) - 1;
    if (static_cast<uint32_t>(ch) < kUnicodeCodePointCount)
    {
        index = kUnicodeTypeRecordsIndex1[(ch >> kUnicodeTypeRecordsIndexShift)];
        index = kUnicodeTypeRecordsIndex2[(index << kUnicodeTypeRecordsIndexShift) + (ch & mask)];
    }
    return kUnicodeTypeRecords[index];
}

//////////////////////////////////////////////////////////////////////////////// Normalization

static const int kHangulSBase = 0xAC00;
static const int kHangulLBase = 0x1100;
static const int kHangulVBase = 0x1161;
static const int kHangulTBase = 0x11A7;
static const int kHangulLCount = 19;
static const int kHangulVCount = 21;
static const int kHangulTCount = 28;
static const int kHangulNCount = kHangulVCount * kHangulTCount;
static const int kHangulSCount = kHangulLCount * kHangulNCount;

/**
 * @brief 获取解组合的记录
 * @param[out] index 返回映射元素的起始下标，映射元素位于[index, index+count)
 * @param[out] count 映射数量
 * @param[out] prefixIndex 前缀下标
 * @param ch 字符
 * @param oldVersion 是否为老版本
 */
static void GetDecompositionRecord(unsigned& index, unsigned& count, unsigned& prefixIndex, char32_t ch,
    bool oldVersion)noexcept
{
    index = 0;

    if (static_cast<unsigned>(ch) < kUnicodeCodePointCount)
    {
        if (!oldVersion || GetChangeRecord_3_2_0(ch).CategoryChanged != 0)  // 保证老版本里面也有这个字符
        {
            const auto mask = (1 << kUnicodeDecompDataShift) - 1;
            index = kUnicodeDecompDataIndex1[(ch >> kUnicodeDecompDataShift)];
            index = kUnicodeDecompDataIndex2[(index << kUnicodeDecompDataShift) + (ch & mask)];
        }
    }

    count = kUnicodeDecompData[index] >> 8;
    prefixIndex = kUnicodeDecompData[index] & 0xFF;
    ++index;
}

/**
 * @brief 快速检查是否已经规格化
 * @param input 输入字符串
 * @param nfc nfc模式开关
 * @param k k模式开关
 * @param oldVersion 是否为老版本
 * @return 返回true表示是已经规格化的，返回false表示可能没有
 */
static bool IsNormalized(ArrayView<char32_t> input, bool nfc, bool k, bool oldVersion)noexcept
{
    if (oldVersion)  // 老版本不可用
        return false;

    // see: http://unicode.org/reports/tr15/#Annex8
    // 0: yes, 1: maybe, 2: no
    auto quickCheckMask = static_cast<uint8_t>(3 << ((nfc ? 4 : 0) + (k ? 2 : 0)));

    unsigned prevCombining = 0;
    for (size_t i = 0; i < input.GetSize(); ++i)
    {
        auto ch = input[i];
        const auto& record = GetUnicodeDatabaseRecord(ch);
        auto combining = record.Combining;
        auto quickcheck = record.NormalizationQuickCheck;

        if (quickcheck & quickCheckMask)
            return false;
        if (combining && prevCombining > combining)
            return false;
        prevCombining = combining;
    }
    return true;
}

static void NormalizeNfdOrNfkd(std::u32string& out, ArrayView<char32_t> input, bool k, bool oldVersion)
{
    out.reserve(input.GetSize() > 10 ? max(input.GetSize() + 10, input.GetSize()) : input.GetSize() * 2);

    char32_t stack[20];
    int sp = 0;
    for (size_t i = 0; i < input.GetSize(); ++i)
    {
        stack[sp++] = input[i];
        while (sp)
        {
            auto ch = stack[--sp];

            // 韩文解组合
            if (kHangulSBase <= ch && ch < (kHangulSBase + kHangulSCount))
            {
                int s = static_cast<int>(ch) - kHangulSBase;
                int l = kHangulLBase + s / kHangulNCount;
                int v = kHangulVBase + (s % kHangulNCount) / kHangulTCount;
                int t = kHangulTBase + s % kHangulTCount;
                out.push_back(static_cast<char32_t>(l));
                out.push_back(static_cast<char32_t>(v));

                if (t != kHangulTBase)
                    out.push_back(static_cast<char32_t>(t));
                continue;
            }

            if (oldVersion)
            {
                auto v = Normalization_3_2_0(ch);
                if (v != 0)
                {
                    stack[sp++] = v;
                    continue;
                }
            }

            unsigned index = 0, prefixIndex = 0, count = 0;
            GetDecompositionRecord(index, count, prefixIndex, ch, oldVersion);

            if (!count || (prefixIndex && !k))
            {
                out.push_back(ch);
                continue;
            }

            while (count)
            {
                ch = kUnicodeDecompData[index + (--count)];
                stack[sp++] = ch;
            }
        }
    }

    size_t i = 0;
    auto prev = GetUnicodeDatabaseRecord(out[i]).Combining;
    for (++i; i < out.length(); ++i)
    {
        auto cur = GetUnicodeDatabaseRecord(out[i]).Combining;
        if (prev == 0 || cur == 0 || prev <= cur)
        {
            prev = cur;
            continue;
        }

        auto o = i - 1;
        while (true)
        {
            auto tmp = out[o + 1];
            out[o + 1] = out[o];
            out[o] = tmp;

            if (o == 0)
                break;
            --o;
            prev = GetUnicodeDatabaseRecord(out[o]).Combining;
            if (prev == 0 || prev <= cur)
                break;
        }
        prev = GetUnicodeDatabaseRecord(out[i]).Combining;
    }
}

static unsigned FindNfcIndex(char32_t ch, const NfcCharReindex* nfc)
{
    for (unsigned index = 0; nfc[index].Start != 0; ++index)
    {
        unsigned start = nfc[index].Start;
        if (static_cast<unsigned>(ch) < start)
            return static_cast<unsigned>(-1);
        if (static_cast<unsigned>(ch) <= start + nfc[index].Count)
        {
            unsigned delta = static_cast<unsigned>(ch) - start;
            return nfc[index].Index + delta;
        }
    }
    return static_cast<unsigned>(-1);
}

static void NormalizeNfcOrNfkc(std::u32string& out, ArrayView<char32_t> input, bool k, bool oldVersion)
{
    u32string tmp;
    NormalizeNfdOrNfkd(tmp, input, k, oldVersion);
    out.reserve(tmp.length());

    size_t i = 0, skippedCount = 0, skipped[20];

    while (i < tmp.length())
    {
        bool again = false;
        for (size_t index = 0; index < skippedCount; ++index)
        {
            if (skipped[index] == i)
            {
                skipped[index] = skipped[skippedCount - 1];
                --skippedCount;
                ++i;
                again = true;
                break;
            }
        }
        if (again)
            continue;

        auto ch = tmp[i];
        if (kHangulLBase <= ch && ch < (kHangulLBase + kHangulLCount) && i + 1 < tmp.length() &&
            kHangulVBase <= tmp[i + 1] && tmp[i + 1] <= (kHangulVBase + kHangulVCount))
        {
            int l = static_cast<int>(ch) - kHangulLBase;
            int v = static_cast<int>(tmp[i + 1]) - kHangulVBase;
            ch = static_cast<char32_t>(kHangulSBase + (l * kHangulVCount + v) * kHangulTCount);
            i += 2;
            if (i < tmp.length() && kHangulTBase <= tmp[i] && tmp[i] <= (kHangulTBase + kHangulTCount))
            {
                ch += tmp[i] - kHangulTBase;
                ++i;
            }
            out.push_back(ch);
            continue;
        }

        auto f = FindNfcIndex(ch, kUnicodeNfcFirst);
        if (f == static_cast<unsigned>(-1))
        {
            out.push_back(ch);
            ++i;
            continue;
        }

        auto i1 = i + 1;
        auto comb = 0;
        auto o = tmp[i];
        while (i1 < tmp.length())
        {
            auto code1 = tmp[i1];
            auto comb1 = GetUnicodeDatabaseRecord(code1).Combining;
            if (comb)
            {
                if (comb1 == 0)
                    break;
                if (comb >= comb1)
                {
                    ++i1;
                    continue;
                }
            }
            auto l = FindNfcIndex(code1, kUnicodeNfcLast);
            if (l == static_cast<unsigned>(-1))
            {
NOT_COMBINABLE:
                if (comb1 == 0)
                    break;
                comb = comb1;
                ++i1;
                continue;
            }
            auto mask = (1 << kUnicodeCompShift) - 1;
            auto index = f * kUnicodeNfcLastCount + l;
            auto index1 = kUnicodeCompDataIndex1[index >> kUnicodeCompShift];
            ch = kUnicodeCompDataIndex2[(index1 << kUnicodeCompShift) + (index & mask)];
            if (ch == 0)
                goto NOT_COMBINABLE;

            o = ch;
            assert(skippedCount < 20);
            skipped[skippedCount++] = i1;
            ++i1;
            f = FindNfcIndex(o, kUnicodeNfcFirst);
            if (f == static_cast<unsigned>(-1))
                break;
        }
        out.push_back(o);
        ++i;
    }
}

static void Normalize(std::u32string& out, ArrayView<char32_t> input, Unicode::NormalizationFormType form,
    bool oldVersion)
{
    out.clear();
    if (input.GetSize() == 0)
        return;

    switch (form)
    {
        case Unicode::NormalizationFormType::NFC:
            if (IsNormalized(input, true, false, oldVersion))
                out.assign(input.GetBuffer(), input.GetSize());
            else
                NormalizeNfcOrNfkc(out, input, false, oldVersion);
            break;
        case Unicode::NormalizationFormType::NFKC:
            if (IsNormalized(input, true, true, oldVersion))
                out.assign(input.GetBuffer(), input.GetSize());
            else
                NormalizeNfcOrNfkc(out, input, true, oldVersion);
            break;
        case Unicode::NormalizationFormType::NFD:
            if (IsNormalized(input, false, false, oldVersion))
                out.assign(input.GetBuffer(), input.GetSize());
            else
                NormalizeNfdOrNfkd(out, input, false, oldVersion);
            break;
        case Unicode::NormalizationFormType::NFKD:
            if (IsNormalized(input, false, true, oldVersion))
                out.assign(input.GetBuffer(), input.GetSize());
            else
                NormalizeNfdOrNfkd(out, input, true, oldVersion);
            break;
        default:
            assert(false);
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////// Unicode

const char* Unicode::GetVersion()noexcept
{
    return kUnicodeDataVersion;
}

bool Unicode::IsXidStart(char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    return (type.Flags & UNICODE_TYPEMASK_XID_START) != 0;
}

bool Unicode::IsXidContinue(char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    return (type.Flags & UNICODE_TYPEMASK_XID_CONTINUE) != 0;
}

bool Unicode::IsDecimalDigit(char32_t ch, unsigned *out)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    if (type.Flags & UNICODE_TYPEMASK_DECIMAL)
    {
        if (out)
            *out = type.Decimal;
        return true;
    }
    else
    {
        if (out)
            *out = 0;
        return false;
    }
}

bool Unicode::IsDigit(char32_t ch, unsigned *out)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    if (type.Flags & UNICODE_TYPEMASK_DIGIT)
    {
        if (out)
            *out = type.Digit;
        return true;
    }
    else
    {
        if (out)
            *out = 0;
        return false;
    }
}

bool Unicode::IsAlpha(char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    return (type.Flags & UNICODE_TYPEMASK_ALPHA) != 0;
}

bool Unicode::IsPrintable(char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    return (type.Flags & UNICODE_TYPEMASK_PRINTABLE) != 0;
}

bool Unicode::IsTitlecase(char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    return (type.Flags & UNICODE_TYPEMASK_TITLE) != 0;
}

bool Unicode::IsLowercase(char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    return (type.Flags & UNICODE_TYPEMASK_LOWER) != 0;
}

bool Unicode::IsUppercase(char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    return (type.Flags & UNICODE_TYPEMASK_UPPER) != 0;
}

bool Unicode::IsCased(char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    return (type.Flags & UNICODE_TYPEMASK_CASED) != 0;
}

bool Unicode::IsCaseIgnorable(char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    return (type.Flags & UNICODE_TYPEMASK_CASE_IGNORABLE) != 0;
}

char32_t Unicode::ToTitlecase(char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    if (type.Flags & UNICODE_TYPEMASK_EXTENDED_CASE)
        return kUnicodeExtendedCase[type.Title & 0xFFFF];
    return ch + type.Title;
}

size_t Unicode::ToTitlecaseFull(MutableArrayView<char32_t> out, char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    if (type.Flags & UNICODE_TYPEMASK_EXTENDED_CASE)
    {
        auto index = type.Title & 0xFFFF;
        auto n = std::min(out.GetSize(), static_cast<size_t>(type.Title >> 24));
        for (unsigned i = 0; i < n; ++i)
            out[i] = kUnicodeExtendedCase[index + i];
        return n;
    }
    if (out.GetSize() >= 1)
    {
        out[0] = ch + type.Title;
        return 1;
    }
    return 0;
}

char32_t Unicode::ToUppercase(char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    if (type.Flags & UNICODE_TYPEMASK_EXTENDED_CASE)
        return kUnicodeExtendedCase[type.Upper & 0xFFFF];
    return ch + type.Upper;
}

size_t Unicode::ToUppercaseFull(MutableArrayView<char32_t> out, char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    if (type.Flags & UNICODE_TYPEMASK_EXTENDED_CASE)
    {
        auto index = type.Upper & 0xFFFF;
        auto n = std::min(out.GetSize(), static_cast<size_t>(type.Upper >> 24));
        for (unsigned i = 0; i < n; ++i)
            out[i] = kUnicodeExtendedCase[index + i];
        return n;
    }
    if (out.GetSize() >= 1)
    {
        out[0] = ch + type.Upper;
        return 1;
    }
    return 0;
}

char32_t Unicode::ToLowercase(char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    if (type.Flags & UNICODE_TYPEMASK_EXTENDED_CASE)
        return kUnicodeExtendedCase[type.Lower & 0xFFFF];
    return ch + type.Lower;
}

size_t Unicode::ToLowercaseFull(MutableArrayView<char32_t> out, char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    if (type.Flags & UNICODE_TYPEMASK_EXTENDED_CASE)
    {
        auto index = type.Lower & 0xFFFF;
        auto n = std::min(out.GetSize(), static_cast<size_t>(type.Lower >> 24));
        for (unsigned i = 0; i < n; ++i)
            out[i] = kUnicodeExtendedCase[index + i];
        return n;
    }
    if (out.GetSize() >= 1)
    {
        out[0] = ch + type.Lower;
        return 1;
    }
    return 0;
}

size_t Unicode::ToFoldedFull(MutableArrayView<char32_t> out, char32_t ch)noexcept
{
    const auto& type = GetUnicodeTypeRecord(ch);
    if ((type.Flags & UNICODE_TYPEMASK_EXTENDED_CASE) && ((type.Lower >> 20) & 7))
    {
        auto index = (type.Lower & 0xFFFF) + (type.Lower >> 24);
        auto n = std::min(out.GetSize(), static_cast<size_t>((type.Lower >> 20) & 7));
        for (unsigned i = 0; i < n; ++i)
            out[i] = kUnicodeExtendedCase[index + i];
        return n;
    }
    return ToLowercaseFull(out, ch);
}

const char* Unicode::GetCategory(char32_t ch)noexcept
{
    auto index = GetUnicodeDatabaseRecord(ch).Category;
    return kUnicodeCategoryNames[index];
}

const char* Unicode::GetBidirectional(char32_t ch)noexcept
{
    auto index = GetUnicodeDatabaseRecord(ch).Bidirectional;
    return kUnicodeBidirectionalNames[index];
}

int Unicode::GetCombining(char32_t ch)noexcept
{
    auto combining = GetUnicodeDatabaseRecord(ch).Combining;
    return combining;
}

bool Unicode::IsMirrored(char32_t ch)noexcept
{
    auto mirrored = GetUnicodeDatabaseRecord(ch).Mirrored;
    return mirrored != 0;
}

const char* Unicode::GetEastAsianWidth(char32_t ch)noexcept
{
    auto index = GetUnicodeDatabaseRecord(ch).EastAsianWidth;
    return kUnicodeEastAsianWidthNames[index];
}

size_t Unicode::Decomposition(std::string& out, char32_t ch)
{
    out.clear();

    unsigned index = 0;
    const auto mask = (1u << kUnicodeDecompDataShift) - 1;

    if (static_cast<unsigned>(ch) < kUnicodeCodePointCount)
    {
        index = kUnicodeDecompDataIndex1[ch >> kUnicodeDecompDataShift];
        index = kUnicodeDecompDataIndex2[(index << kUnicodeDecompDataShift) + (ch & mask)];
    }

    const auto count = kUnicodeDecompData[index] >> 8;
    unsigned prefixIndex = kUnicodeDecompData[index] & 0xFF;
    assert(prefixIndex < CountOf(kUnicodeDecompPrefix));

    if (prefixIndex < CountOf(kUnicodeDecompPrefix) && kUnicodeDecompPrefix[prefixIndex] != nullptr)
        out.append(kUnicodeDecompPrefix[prefixIndex]);

    auto i = count;
    while (i-- > 0)
    {
        if (!out.empty())
            out.push_back(' ');

        char buf[16];
        size_t len = Convert::ToHexString(static_cast<uint32_t>(kUnicodeDecompData[++index]), buf);
        if (len < 4)
        {
            for (size_t j = 0; j < (4 - len); ++j)
                out.push_back('0');
        }
        out.append(buf, len);
    }
    return count;
}

void Unicode::Normalize(std::u32string& out, ArrayView<char32_t> input, Unicode::NormalizationFormType form)
{
    ::Normalize(out, input, form, false);
}

//////////////////////////////////////////////////////////////////////////////// Ucd_3_2_0

const char* Unicode::Ucd_3_2_0::GetCategory(char32_t ch)noexcept
{
    auto index = GetUnicodeDatabaseRecord(ch).Category;
    const auto& changed = GetChangeRecord_3_2_0(ch);
    if (changed.CategoryChanged != 0xFF)
        index = changed.CategoryChanged;
    return kUnicodeCategoryNames[index];
}

const char* Unicode::Ucd_3_2_0::GetBidirectional(char32_t ch)noexcept
{
    auto index = GetUnicodeDatabaseRecord(ch).Bidirectional;
    const auto& changed = GetChangeRecord_3_2_0(ch);
    if (changed.CategoryChanged == 0)
        index = 0;
    else if (changed.BidirChanged != 0xFF)
        index = changed.BidirChanged;
    return kUnicodeBidirectionalNames[index];
}

int Unicode::Ucd_3_2_0::GetCombining(char32_t ch)noexcept
{
    auto combining = GetUnicodeDatabaseRecord(ch).Combining;
    const auto& changed = GetChangeRecord_3_2_0(ch);
    if (changed.CategoryChanged == 0)
        combining = 0;
    return combining;
}

bool Unicode::Ucd_3_2_0::IsMirrored(char32_t ch)noexcept
{
    auto mirrored = GetUnicodeDatabaseRecord(ch).Mirrored;
    const auto& changed = GetChangeRecord_3_2_0(ch);
    if (changed.CategoryChanged == 0)
        mirrored = 0;
    else if (changed.MirroredChanged != 0xFF)
        mirrored = changed.MirroredChanged;
    return mirrored != 0;
}

const char* Unicode::Ucd_3_2_0::GetEastAsianWidth(char32_t ch)noexcept
{
    auto index = GetUnicodeDatabaseRecord(ch).EastAsianWidth;
    const auto& changed = GetChangeRecord_3_2_0(ch);
    if (changed.CategoryChanged == 0)
        index = 0;
    else if (changed.EastAsianWidthChanged != 0xFF)
        index = changed.EastAsianWidthChanged;
    return kUnicodeEastAsianWidthNames[index];
}

size_t Unicode::Ucd_3_2_0::Decomposition(std::string& out, char32_t ch)
{
    const auto& changed = GetChangeRecord_3_2_0(ch);
    if (changed.CategoryChanged == 0)
    {
        out.clear();
        return 0;
    }

    return Unicode::Decomposition(out, ch);
}

void Unicode::Ucd_3_2_0::Normalize(std::u32string& out, ArrayView<char32_t> input, Unicode::NormalizationFormType form)
{
    ::Normalize(out, input, form, true);
}
