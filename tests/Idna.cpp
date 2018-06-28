/**
 * @file
 * @author chu
 * @date 2018/6/9
 */
#include <gtest/gtest.h>
#include <cstdarg>

#include <Moe.Core/Idna.hpp>

using namespace std;
using namespace moe;

// https://github.com/bestiejs/punycode.js/blob/master/tests/tests.js
static const u32string kPunycodeTestCases[] = {
    // decoded, encoded
    U"Bach", U"Bach-",
    U"\u00FC", U"tda",
    U"\u00FC\u00EB\u00E4\u00F6\u2665", U"4can8av2009b",
    U"b\u00FCcher", U"bcher-kva",
    U"\u0644\u064A\u0647\u0645\u0627\u0628\u062A\u0643\u0644\u0645\u0648\u0634\u0639\u0631\u0628\u064A\u061F",
        U"egbpdaj6bu4bxfgehfvwxn",
    U"\u4ED6\u4EEC\u4E3A\u4EC0\u4E48\u4E0D\u8BF4\u4E2d\u6587", U"ihqwcrb4cv8a8dqg056pqjye",
    U"\u4ED6\u5011\u7232\u4EC0\u9EBD\u4E0D\u8AAA\u4E2D\u6587", U"ihqwctvzc91f659drss3x8bo0yb",
    U"Pro\u010Dprost\u011Bnemluv\xED\u010Desky", U"Proprostnemluvesky-uyb24dma41a",
    U"\u05DC\u05DE\u05D4\u05D4\u05DD\u05E4\u05E9\u05D5\u05D8\u05DC\u05D0\u05DE\u05D3\u05D1\u05E8\u05D9\u05DD\u05E2"
        U"\u05D1\u05E8\u05D9\u05EA", U"4dbcagdahymbxekheh6e0a7fei0b",
    U"\u092F\u0939\u0932\u094B\u0917\u0939\u093F\u0928\u094D\u0926\u0940\u0915\u094D\u092F\u094B\u0902\u0928\u0939"
        U"\u0940\u0902\u092C\u094B\u0932\u0938\u0915\u0924\u0947\u0939\u0948\u0902",
        U"i1baa7eci9glrd9b2ae1bj0hfcgg6iyaf8o0a1dig0cd",
    U"\u306A\u305C\u307F\u3093\u306A\u65E5\u672C\u8A9E\u3092\u8A71\u3057\u3066\u304F\u308C\u306A\u3044\u306E\u304B",
        U"n8jok5ay5dzabd5bym9f0cm5685rrjetr6pdxa",
    U"\uC138\uACC4\uC758\uBAA8\uB4E0\uC0AC\uB78C\uB4E4\uC774\uD55C\uAD6D\uC5B4\uB97C\uC774\uD574\uD55C\uB2E4\uBA74"
        U"\uC5BC\uB9C8\uB098\uC88B\uC744\uAE4C",
        U"989aomsvi5e83db1d2a355cv1e0vak1dwrv93d5xbh15a0dt30a5jpsd879ccm6fea98c",
    U"\u043F\u043E\u0447\u0435\u043C\u0443\u0436\u0435\u043E\u043D\u0438\u043D\u0435\u0433\u043E\u0432\u043E\u0440"
        U"\u044F\u0442\u043F\u043E\u0440\u0443\u0441\u0441\u043A\u0438",
        U"b1abfaaepdrnnbgefbadotcwatmq2g4l",
    U"Porqu\u00E9nopuedensimplementehablarenEspa\u00F1ol", U"PorqunopuedensimplementehablarenEspaol-fmd56a",
    U"T\u1EA1isaoh\u1ECDkh\u00F4ngth\u1EC3ch\u1EC9n\u00F3iti\u1EBFngVi\u1EC7t",
        U"TisaohkhngthchnitingVit-kjcr8268qyxafd2f1b9g",
    U"3\u5E74B\u7D44\u91D1\u516B\u5148\u751F", U"3B-ww4c5e180e575a65lsy2b",
    U"\u5B89\u5BA4\u5948\u7F8E\u6075-with-SUPER-MONKEYS", U"-with-SUPER-MONKEYS-pc58ag80a8qai00g7n9n",
    U"Hello-Another-Way-\u305D\u308C\u305E\u308C\u306E\u5834\u6240", U"Hello-Another-Way--fc4qua05auwb3674vfr0b",
    U"\u3072\u3068\u3064\u5C4B\u6839\u306E\u4E0B2", U"2-u9tlzr9756bt3uc0v",
    U"Maji\u3067Koi\u3059\u308B5\u79D2\u524D", U"MajiKoi5-783gue6qz075azm5e",
    U"\u30D1\u30D5\u30A3\u30FCde\u30EB\u30F3\u30D0", U"de-jg4avhby1noc0d",
    U"\u305D\u306E\u30B9\u30D4\u30FC\u30C9\u3067", U"d9juau41awczczp",
    U"-> $1.00 <-", U"-> $1.00 <--",
};

TEST(Idna, Punycode)
{
    u32string tmp;
    for (size_t i = 0; i < CountOf(kPunycodeTestCases) / 2; ++i)
    {
        const auto& decoded = kPunycodeTestCases[i * 2];
        const auto& encoded = kPunycodeTestCases[i * 2 + 1];

        Idna::PunycodeEncode(tmp, ArrayView<char32_t>(decoded.c_str(), decoded.length()));
        EXPECT_EQ(encoded, tmp);

        Idna::PunycodeDecode(tmp, ArrayView<char32_t>(encoded.c_str(), encoded.length()));
        EXPECT_EQ(decoded, tmp);
    }
}

TEST(Idna, CheckBidi)
{
    u32string l = U"\u0061";
    u32string r = U"\u05d0";
    u32string al = U"\u0627";
    u32string an = U"\u0660";
    u32string en = U"\u0030";
    u32string es = U"\u002d";
    u32string cs = U"\u002c";
    u32string et = U"\u0024";
    u32string on = U"\u0021";
    u32string bn = U"\u200c";
    u32string nsm = U"\u0610";
    u32string ws = U"\u0020";

    // RFC 5893 Rule 1
    EXPECT_EQ(l, Idna::ToUnicode(l, false, true, false, false, false));
    EXPECT_EQ(r, Idna::ToUnicode(r, false, true, false, false, false));
    EXPECT_EQ(al, Idna::ToUnicode(al, false, true, false, false, false));
    EXPECT_THROW(Idna::ToUnicode(an, false, true, false, false, false), BadFormatException);

    // RFC 5893 Rule 2
    EXPECT_EQ(r + al, Idna::ToUnicode(r + al, false, true, false, false, false));
    EXPECT_EQ(r + an, Idna::ToUnicode(r + an, false, true, false, false, false));
    EXPECT_EQ(r + en, Idna::ToUnicode(r + en, false, true, false, false, false));
    EXPECT_EQ(r + es + al, Idna::ToUnicode(r + es + al, false, true, false, false, false));
    EXPECT_EQ(r + cs + al, Idna::ToUnicode(r + cs + al, false, true, false, false, false));
    EXPECT_EQ(r + et + al, Idna::ToUnicode(r + et + al, false, true, false, false, false));
    EXPECT_EQ(r + on + al, Idna::ToUnicode(r + on + al, false, true, false, false, false));
    EXPECT_EQ(r + bn + al, Idna::ToUnicode(r + bn + al, false, true, false, false, false));
    EXPECT_EQ(r + nsm, Idna::ToUnicode(r + nsm, false, true, false, false, false));
    EXPECT_THROW(Idna::ToUnicode(r + l, false, true, false, false, false), BadFormatException);
    EXPECT_THROW(Idna::ToUnicode(r + ws, false, true, false, false, false), BadFormatException);

    // RFC 5893 Rule 3
    EXPECT_EQ(r + al, Idna::ToUnicode(r + al, false, true, false, false, false));
    EXPECT_EQ(r + en, Idna::ToUnicode(r + en, false, true, false, false, false));
    EXPECT_EQ(r + an, Idna::ToUnicode(r + an, false, true, false, false, false));
    EXPECT_EQ(r + nsm, Idna::ToUnicode(r + nsm, false, true, false, false, false));
    EXPECT_EQ(r + nsm + nsm, Idna::ToUnicode(r + nsm + nsm, false, true, false, false, false));
    EXPECT_THROW(Idna::ToUnicode(r + on, false, true, false, false, false), BadFormatException);

    // RFC 5893 Rule 4
    EXPECT_EQ(r + en, Idna::ToUnicode(r + en));
    EXPECT_EQ(r + an, Idna::ToUnicode(r + an));
    EXPECT_THROW(Idna::ToUnicode(r + en + an), BadFormatException);
    EXPECT_THROW(Idna::ToUnicode(r + an + en), BadFormatException);
}

TEST(Idna, InitialCombiner)
{
    u32string m = U"\u0300";
    u32string a = U"\u0061";

    EXPECT_EQ(a, Idna::ToUnicode(a, false, false, false, false, false));
    EXPECT_EQ(U"\u00E0", Idna::ToUnicode(a + m, false, false, false, false, false));
    EXPECT_THROW(Idna::ToUnicode(m + a), BadFormatException);
}

TEST(Idna, HyphenOk)
{
    EXPECT_EQ(U"abc", Idna::ToUnicode(U"abc", true, false, false, false, false));
    EXPECT_EQ(U"a--b", Idna::ToUnicode(U"a--b", true, false, false, false, false));
    EXPECT_THROW(Idna::ToUnicode(U"aa--", true, false, false, false, false), BadFormatException);
    EXPECT_THROW(Idna::ToUnicode(U"a-", true, false, false, false, false), BadFormatException);
    EXPECT_THROW(Idna::ToUnicode(U"-a", true, false, false, false, false), BadFormatException);
}

TEST(Idna, ValidateContextJ)
{
    u32string start = U"1";
    u32string zwnj = U"\u200c";
    u32string zwj = U"\u200d";
    u32string virama = U"\u094d";
    u32string latin = U"\u0061";

    // RFC 5892 Appendix A.1 (Zero Width Non-Joiner)
    EXPECT_THROW(Idna::ToUnicode(start + zwnj, false, false, true, false, false), BadFormatException);
    EXPECT_THROW(Idna::ToUnicode(start + latin + zwnj, false, false, true, false, false), BadFormatException);
    EXPECT_EQ(start + virama + zwnj, Idna::ToUnicode(start + virama + zwnj, false, false, true, false, false));

    // RFC 5892 Appendix A.2 (Zero Width Joiner)
    EXPECT_THROW(Idna::ToUnicode(start + zwj, false, false, true, false, false), BadFormatException);
    EXPECT_THROW(Idna::ToUnicode(start + latin + zwj, false, false, true, false, false), BadFormatException);
    EXPECT_EQ(start + virama + zwj, Idna::ToUnicode(start + virama + zwj, false, false, true, false, false));
}

TEST(Idna, ValidateContextO)
{
    u32string latin = U"\u0061";
    u32string latinL = U"\u006c";
    u32string greek = U"\u03b1";
    u32string hebrew = U"\u05d0";
    u32string katakana = U"\u30a1";
    u32string hiragana = U"\u3041";
    u32string han = U"\u6f22";
    u32string arabicDigit = U"\u0660";
    u32string extArabicDigit = U"\u06f0";

    // RFC 5892 Rule A.3 (Middle Dot)
    u32string latinMiddleDot = U"\u00b7";
    EXPECT_EQ(latinL + latinMiddleDot + latinL,
        Idna::ToUnicode(latinL + latinMiddleDot + latinL, false, false, true, false, false));
    EXPECT_THROW(Idna::ToUnicode(latinMiddleDot + latinL, false, false, true, false, false), BadFormatException);
    EXPECT_THROW(Idna::ToUnicode(latinL + latinMiddleDot, false, false, true, false, false), BadFormatException);
    EXPECT_THROW(Idna::ToUnicode(latinMiddleDot, false, false, true, false, false), BadFormatException);
    EXPECT_THROW(Idna::ToUnicode(latinL + latinMiddleDot + latin, false, false, true, false, false),
        BadFormatException);

    // RFC 5892 Rule A.4 (Greek Lower Numeral Sign)
    u32string glns = U"\u0375";
    EXPECT_EQ(glns + greek, Idna::ToUnicode(glns + greek, false, false, true, false, false));
    EXPECT_THROW(Idna::ToUnicode(glns + latin, false, false, true, false, false), BadFormatException);
    EXPECT_THROW(Idna::ToUnicode(glns, false, false, true, false, false), BadFormatException);
    EXPECT_THROW(Idna::ToUnicode(greek + glns, false, false, true, false, false), BadFormatException);

    // RFC 5892 Rule A.5 (Hebrew Punctuation Geresh)
    u32string geresh = U"\u05f3";
    EXPECT_EQ(hebrew + geresh, Idna::ToUnicode(hebrew + geresh, false, false, true, false, false));
    EXPECT_THROW(Idna::ToUnicode(latin + geresh, false, false, true, false, false), BadFormatException);

    // RFC 5892 Rule A.6 (Hebrew Punctuation Gershayim)
    u32string gershayim = U"\u05f4";
    EXPECT_EQ(hebrew + gershayim, Idna::ToUnicode(hebrew + gershayim, false, false, true, false, false));
    EXPECT_THROW(Idna::ToUnicode(latin + gershayim, false, false, true, false, false), BadFormatException);

    // RFC 5892 Rule A.7 (Katakana Middle Dot)
    u32string jaMiddleDot = U"\u30fb";
    EXPECT_EQ(katakana + jaMiddleDot + katakana,
        Idna::ToUnicode(katakana + jaMiddleDot + katakana, false, false, true, false, false));
    EXPECT_EQ(hiragana + jaMiddleDot + hiragana,
        Idna::ToUnicode(hiragana + jaMiddleDot + hiragana, false, false, true, false, false));
    EXPECT_EQ(han + jaMiddleDot + han,
        Idna::ToUnicode(han + jaMiddleDot + han, false, false, true, false, false));
    EXPECT_EQ(han + jaMiddleDot + latin,
        Idna::ToUnicode(han + jaMiddleDot + latin, false, false, true, false, false));
    EXPECT_EQ(U"\u6f22\u30fb\u5b57", Idna::ToUnicode(U"\u6f22\u30fb\u5b57", false, false, true, false, false));
    EXPECT_THROW(Idna::ToUnicode(U"\u0061\u30fb\u0061", false, false, true, false, false), BadFormatException);

    // RFC 5892 Rule A.8 (Arabic-Indic Digits)
    EXPECT_EQ(arabicDigit + arabicDigit, Idna::ToUnicode(arabicDigit + arabicDigit, false, false, true, false, false));
    EXPECT_THROW(Idna::ToUnicode(arabicDigit + extArabicDigit, false, false, true, false, false), BadFormatException);

    // RFC 5892 Rule A.9 (Extended Arabic-Indic Digits)
    EXPECT_EQ(extArabicDigit + extArabicDigit,
        Idna::ToUnicode(extArabicDigit + extArabicDigit, false, false, true, false, false));
    EXPECT_THROW(Idna::ToUnicode(extArabicDigit + arabicDigit, false, false, true, false, false), BadFormatException);
}

TEST(Idna, Misc)
{
    static const u32string kCases[][2] = {
        {U"\u6d4b\u8bd5", U"xn--0zwm56d"},
        {U"\u092a\u0930\u0940\u0915\u094d\u0937\u093e", U"xn--11b5bs3a9aj6g"},
        {U"\ud55c\uad6d", U"xn--3e0b707e"},
        {U"\u09ad\u09be\u09b0\u09a4", U"xn--45brj9c"},
        {U"\u09ac\u09be\u0982\u09b2\u09be", U"xn--54b7fta0cc"},
        {U"\u0438\u0441\u043f\u044b\u0442\u0430\u043d\u0438\u0435", U"xn--80akhbyknj4f"},
        {U"\u0441\u0440\u0431", U"xn--90a3ac"},
        {U"\ud14c\uc2a4\ud2b8", U"xn--9t4b11yi5a"},
        {U"\u0b9a\u0bbf\u0b99\u0bcd\u0b95\u0baa\u0bcd\u0baa\u0bc2\u0bb0\u0bcd", U"xn--clchc0ea0b2g2a9gcd"},
        {U"\u05d8\u05e2\u05e1\u05d8", U"xn--deba0ad"},
        {U"\u4e2d\u56fd", U"xn--fiqs8s"},
        {U"\u4e2d\u570b", U"xn--fiqz9s"},
        {U"\u0c2d\u0c3e\u0c30\u0c24\u0c4d", U"xn--fpcrj9c3d"},
        {U"\u0dbd\u0d82\u0d9a\u0dcf", U"xn--fzc2c9e2c"},
        {U"\u6e2c\u8a66", U"xn--g6w251d"},
        {U"\u0aad\u0abe\u0ab0\u0aa4", U"xn--gecrj9c"},
        {U"\u092d\u093e\u0930\u0924", U"xn--h2brj9c"},
        {U"\u0622\u0632\u0645\u0627\u06cc\u0634\u06cc", U"xn--hgbk6aj7f53bba"},
        {U"\u0baa\u0bb0\u0bbf\u0b9f\u0bcd\u0b9a\u0bc8", U"xn--hlcj6aya9esc7a"},
        {U"\u0443\u043a\u0440", U"xn--j1amh"},
        {U"\u9999\u6e2f", U"xn--j6w193g"},
        {U"\u03b4\u03bf\u03ba\u03b9\u03bc\u03ae", U"xn--jxalpdlp"},
        {U"\u0625\u062e\u062a\u0628\u0627\u0631", U"xn--kgbechtv"},
        {U"\u53f0\u6e7e", U"xn--kprw13d"},
        {U"\u53f0\u7063", U"xn--kpry57d"},
        {U"\u0627\u0644\u062c\u0632\u0627\u0626\u0631", U"xn--lgbbat1ad8j"},
        {U"\u0639\u0645\u0627\u0646", U"xn--mgb9awbf"},
        {U"\u0627\u06cc\u0631\u0627\u0646", U"xn--mgba3a4f16a"},
        {U"\u0627\u0645\u0627\u0631\u0627\u062a", U"xn--mgbaam7a8h"},
        {U"\u067e\u0627\u06a9\u0633\u062a\u0627\u0646", U"xn--mgbai9azgqp6j"},
        {U"\u0627\u0644\u0627\u0631\u062f\u0646", U"xn--mgbayh7gpa"},
        {U"\u0628\u06be\u0627\u0631\u062a", U"xn--mgbbh1a71e"},
        {U"\u0627\u0644\u0645\u063a\u0631\u0628", U"xn--mgbc0a9azcg"},
        {U"\u0627\u0644\u0633\u0639\u0648\u062f\u064a\u0629", U"xn--mgberp4a5d4ar"},
        {U"\u10d2\u10d4", U"xn--node"},
        {U"\u0e44\u0e17\u0e22", U"xn--o3cw4h"},
        {U"\u0633\u0648\u0631\u064a\u0629", U"xn--ogbpf8fl"},
        {U"\u0440\u0444", U"xn--p1ai"},
        {U"\u062a\u0648\u0646\u0633", U"xn--pgbs0dh"},
        {U"\u0a2d\u0a3e\u0a30\u0a24", U"xn--s9brj9c"},
        {U"\u0645\u0635\u0631", U"xn--wgbh1c"},
        {U"\u0642\u0637\u0631", U"xn--wgbl6a"},
        {U"\u0b87\u0bb2\u0b99\u0bcd\u0b95\u0bc8", U"xn--xkc2al3hye2a"},
        {U"\u0b87\u0ba8\u0bcd\u0ba4\u0bbf\u0baf\u0bbe", U"xn--xkc2dl3a5ee0h"},
        {U"\u65b0\u52a0\u5761", U"xn--yfro4i67o"},
        {U"\u0641\u0644\u0633\u0637\u064a\u0646", U"xn--ygbi2ammx"},
        {U"\u30c6\u30b9\u30c8", U"xn--zckzah"},
        {U"\u049b\u0430\u0437", U"xn--80ao21a"},
        {U"\u0645\u0644\u064a\u0633\u064a\u0627", U"xn--mgbx4cd0ab"},
        {U"\u043c\u043e\u043d", U"xn--l1acc"},
        {U"\u0633\u0648\u062f\u0627\u0646", U"xn--mgbpl2fh"},
    };

    for (size_t i = 0; i < std::extent<decltype(kCases), 0>::value; ++i)
    {
        const auto& unicode = kCases[i][0];
        const auto& ascii = kCases[i][1];
        EXPECT_EQ(unicode, Idna::ToUnicode(ascii));
        EXPECT_EQ(ascii, Idna::ToAscii(unicode, true, true, true, true, false, false));
    }
    
    EXPECT_EQ(U"python.org", Idna::ToUnicode(U"python.org"));
    EXPECT_EQ(U"python.org", Idna::ToUnicode(U"python.org."));
    EXPECT_EQ(U"pyth\u00F6n.org", Idna::ToUnicode(U"xn--pythn-mua.org"));
    EXPECT_EQ(U"pyth\u00F6n.org", Idna::ToUnicode(U"pyth\u00F6n.org"));

    EXPECT_EQ(U"python.org", Idna::ToAscii(U"python.org"));
    EXPECT_EQ(U"python.org", Idna::ToAscii(U"python.org."));
    EXPECT_EQ(U"xn--pythn-mua.org", Idna::ToAscii(U"xn--pythn-mua.org"));
    EXPECT_EQ(U"xn--pythn-mua.org", Idna::ToAscii(U"pyth\u00F6n.org"));

    EXPECT_THROW(Idna::ToUnicode(U"xn--xam"), BadFormatException);

    EXPECT_EQ(U"xn--zckzah.xn--zckzah", Idna::ToAscii(U"\u30c6\u30b9\u30c8.xn--zckzah"));
    EXPECT_EQ(U"\u30c6\u30b9\u30c8.\u30c6\u30b9\u30c8", Idna::ToUnicode(U"xn--zckzah.xn--zckzah"));

    EXPECT_THROW(Idna::ToUnicode(U"A_"), BadFormatException);
    EXPECT_EQ(U"a_", Idna::ToUnicode(U"A_", true, true, true, false, false));
}
