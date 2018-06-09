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
