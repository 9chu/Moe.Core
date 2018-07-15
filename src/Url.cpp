/**
 * @file
 * @author chu
 * @date 2018/3/9
 */
#include <Moe.Core/Url.hpp>
#include <Moe.Core/Exception.hpp>
#include <Moe.Core/StringUtils.hpp>
#include <Moe.Core/Encoding.hpp>
#include <Moe.Core/Idna.hpp>

#include <cassert>
#include <cmath>

using namespace std;
using namespace moe;

static const char* kHexTable[256] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "%21", "%22", "%23", "%24", "%25", "%26", "%27",
    "%28", "%29", "%2A", "%2B", "%2C", "%2D", "%2E", "%2F",
    "%30", "%31", "%32", "%33", "%34", "%35", "%36", "%37",
    "%38", "%39", "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
    "%40", "%41", "%42", "%43", "%44", "%45", "%46", "%47",
    "%48", "%49", "%4A", "%4B", "%4C", "%4D", "%4E", "%4F",
    "%50", "%51", "%52", "%53", "%54", "%55", "%56", "%57",
    "%58", "%59", "%5A", "%5B", "%5C", "%5D", "%5E", "%5F",
    "%60", "%61", "%62", "%63", "%64", "%65", "%66", "%67",
    "%68", "%69", "%6A", "%6B", "%6C", "%6D", "%6E", "%6F",
    "%70", "%71", "%72", "%73", "%74", "%75", "%76", "%77",
    "%78", "%79", "%7A", "%7B", "%7C", "%7D", "%7E", "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF",
};

static const uint8_t kC0ControlEncodeSet[32] = {
    // 00     01      02      03      04      05      06      07
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 08     09      0A      0B      0C      0D      0E      0F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 10     11      12      13      14      15      16      17
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 18     19      1A      1B      1C      1D      1E      1F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 20     21      22      23      24      25      26      27
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 28     29      2A      2B      2C      2D      2E      2F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 30     31      32      33      34      35      36      37
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 38     39      3A      3B      3C      3D      3E      3F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 40     41      42      43      44      45      46      47
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 48     49      4A      4B      4C      4D      4E      4F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 50     51      52      53      54      55      56      57
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 58     59      5A      5B      5C      5D      5E      5F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 60     61      62      63      64      65      66      67
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 68     69      6A      6B      6C      6D      6E      6F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 70     71      72      73      74      75      76      77
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 78     79      7A      7B      7C      7D      7E      7F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x80u,
    // 80     81      82      83      84      85      86      87
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 88     89      8A      8B      8C      8D      8E      8F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 90     91      92      93      94      95      96      97
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 98     99      9A      9B      9C      9D      9E      9F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // A0     A1      A2      A3      A4      A5      A6      A7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // A8     A9      AA      AB      AC      AD      AE      AF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // B0     B1      B2      B3      B4      B5      B6      B7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // B8     B9      BA      BB      BC      BD      BE      BF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // C0     C1      C2      C3      C4      C5      C6      C7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // C8     C9      CA      CB      CC      CD      CE      CF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // D0     D1      D2      D3      D4      D5      D6      D7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // D8     D9      DA      DB      DC      DD      DE      DF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // E0     E1      E2      E3      E4      E5      E6      E7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // E8     E9      EA      EB      EC      ED      EE      EF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // F0     F1      F2      F3      F4      F5      F6      F7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // F8     F9      FA      FB      FC      FD      FE      FF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
};

static const uint8_t kPathEncodeSet[32] = {
    // 00     01      02      03      04      05      06      07
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 08     09      0A      0B      0C      0D      0E      0F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 10     11      12      13      14      15      16      17
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 18     19      1A      1B      1C      1D      1E      1F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 20     21      22      23      24      25      26      27
    0x01u | 0x00u | 0x04u | 0x08u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 28     29      2A      2B      2C      2D      2E      2F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 30     31      32      33      34      35      36      37
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 38     39      3A      3B      3C      3D      3E      3F
    0x00u | 0x00u | 0x00u | 0x00u | 0x10u | 0x00u | 0x40u | 0x80u,
    // 40     41      42      43      44      45      46      47
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 48     49      4A      4B      4C      4D      4E      4F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 50     51      52      53      54      55      56      57
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 58     59      5A      5B      5C      5D      5E      5F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 60     61      62      63      64      65      66      67
    0x01u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 68     69      6A      6B      6C      6D      6E      6F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 70     71      72      73      74      75      76      77
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 78     79      7A      7B      7C      7D      7E      7F
    0x00u | 0x00u | 0x00u | 0x08u | 0x00u | 0x20u | 0x00u | 0x80u,
    // 80     81      82      83      84      85      86      87
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 88     89      8A      8B      8C      8D      8E      8F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 90     91      92      93      94      95      96      97
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 98     99      9A      9B      9C      9D      9E      9F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // A0     A1      A2      A3      A4      A5      A6      A7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // A8     A9      AA      AB      AC      AD      AE      AF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // B0     B1      B2      B3      B4      B5      B6      B7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // B8     B9      BA      BB      BC      BD      BE      BF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // C0     C1      C2      C3      C4      C5      C6      C7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // C8     C9      CA      CB      CC      CD      CE      CF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // D0     D1      D2      D3      D4      D5      D6      D7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // D8     D9      DA      DB      DC      DD      DE      DF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // E0     E1      E2      E3      E4      E5      E6      E7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // E8     E9      EA      EB      EC      ED      EE      EF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // F0     F1      F2      F3      F4      F5      F6      F7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // F8     F9      FA      FB      FC      FD      FE      FF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
};

static const uint8_t kUserInfoEncodeSet[32] = {
    // 00     01      02      03      04      05      06      07
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 08     09      0A      0B      0C      0D      0E      0F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 10     11      12      13      14      15      16      17
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 18     19      1A      1B      1C      1D      1E      1F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 20     21      22      23      24      25      26      27
    0x01u | 0x00u | 0x04u | 0x08u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 28     29      2A      2B      2C      2D      2E      2F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x80u,
    // 30     31      32      33      34      35      36      37
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 38     39      3A      3B      3C      3D      3E      3F
    0x00u | 0x00u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 40     41      42      43      44      45      46      47
    0x01u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 48     49      4A      4B      4C      4D      4E      4F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 50     51      52      53      54      55      56      57
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 58     59      5A      5B      5C      5D      5E      5F
    0x00u | 0x00u | 0x00u | 0x08u | 0x10u | 0x20u | 0x40u | 0x00u,
    // 60     61      62      63      64      65      66      67
    0x01u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 68     69      6A      6B      6C      6D      6E      6F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 70     71      72      73      74      75      76      77
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 78     79      7A      7B      7C      7D      7E      7F
    0x00u | 0x00u | 0x00u | 0x08u | 0x10u | 0x20u | 0x00u | 0x80u,
    // 80     81      82      83      84      85      86      87
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 88     89      8A      8B      8C      8D      8E      8F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 90     91      92      93      94      95      96      97
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 98     99      9A      9B      9C      9D      9E      9F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // A0     A1      A2      A3      A4      A5      A6      A7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // A8     A9      AA      AB      AC      AD      AE      AF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // B0     B1      B2      B3      B4      B5      B6      B7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // B8     B9      BA      BB      BC      BD      BE      BF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // C0     C1      C2      C3      C4      C5      C6      C7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // C8     C9      CA      CB      CC      CD      CE      CF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // D0     D1      D2      D3      D4      D5      D6      D7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // D8     D9      DA      DB      DC      DD      DE      DF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // E0     E1      E2      E3      E4      E5      E6      E7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // E8     E9      EA      EB      EC      ED      EE      EF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // F0     F1      F2      F3      F4      F5      F6      F7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // F8     F9      FA      FB      FC      FD      FE      FF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
};

static const uint8_t kQueryEncodeSet[32] = {
    // 00     01      02      03      04      05      06      07
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 08     09      0A      0B      0C      0D      0E      0F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 10     11      12      13      14      15      16      17
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 18     19      1A      1B      1C      1D      1E      1F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 20     21      22      23      24      25      26      27
    0x01u | 0x00u | 0x04u | 0x08u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 28     29      2A      2B      2C      2D      2E      2F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 30     31      32      33      34      35      36      37
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 38     39      3A      3B      3C      3D      3E      3F
    0x00u | 0x00u | 0x00u | 0x00u | 0x10u | 0x00u | 0x40u | 0x00u,
    // 40     41      42      43      44      45      46      47
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 48     49      4A      4B      4C      4D      4E      4F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 50     51      52      53      54      55      56      57
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 58     59      5A      5B      5C      5D      5E      5F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 60     61      62      63      64      65      66      67
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 68     69      6A      6B      6C      6D      6E      6F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 70     71      72      73      74      75      76      77
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 78     79      7A      7B      7C      7D      7E      7F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x80u,
    // 80     81      82      83      84      85      86      87
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 88     89      8A      8B      8C      8D      8E      8F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 90     91      92      93      94      95      96      97
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 98     99      9A      9B      9C      9D      9E      9F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // A0     A1      A2      A3      A4      A5      A6      A7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // A8     A9      AA      AB      AC      AD      AE      AF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // B0     B1      B2      B3      B4      B5      B6      B7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // B8     B9      BA      BB      BC      BD      BE      BF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // C0     C1      C2      C3      C4      C5      C6      C7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // C8     C9      CA      CB      CC      CD      CE      CF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // D0     D1      D2      D3      D4      D5      D6      D7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // D8     D9      DA      DB      DC      DD      DE      DF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // E0     E1      E2      E3      E4      E5      E6      E7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // E8     E9      EA      EB      EC      ED      EE      EF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // F0     F1      F2      F3      F4      F5      F6      F7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // F8     F9      FA      FB      FC      FD      FE      FF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
};

const static uint8_t kFragmentEncodeSet[32] = {
    // 00     01      02      03      04      05      06      07
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 08     09      0A      0B      0C      0D      0E      0F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 10     11      12      13      14      15      16      17
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 18     19      1A      1B      1C      1D      1E      1F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 20     21      22      23      24      25      26      27
    0x01u | 0x00u | 0x04u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 28     29      2A      2B      2C      2D      2E      2F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 30     31      32      33      34      35      36      37
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 38     39      3A      3B      3C      3D      3E      3F
    0x00u | 0x00u | 0x00u | 0x00u | 0x10u | 0x00u | 0x40u | 0x00u,
    // 40     41      42      43      44      45      46      47
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 48     49      4A      4B      4C      4D      4E      4F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 50     51      52      53      54      55      56      57
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 58     59      5A      5B      5C      5D      5E      5F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 60     61      62      63      64      65      66      67
    0x01u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 68     69      6A      6B      6C      6D      6E      6F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 70     71      72      73      74      75      76      77
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u,
    // 78     79      7A      7B      7C      7D      7E      7F
    0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x00u | 0x80u,
    // 80     81      82      83      84      85      86      87
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 88     89      8A      8B      8C      8D      8E      8F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 90     91      92      93      94      95      96      97
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // 98     99      9A      9B      9C      9D      9E      9F
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // A0     A1      A2      A3      A4      A5      A6      A7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // A8     A9      AA      AB      AC      AD      AE      AF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // B0     B1      B2      B3      B4      B5      B6      B7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // B8     B9      BA      BB      BC      BD      BE      BF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // C0     C1      C2      C3      C4      C5      C6      C7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // C8     C9      CA      CB      CC      CD      CE      CF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // D0     D1      D2      D3      D4      D5      D6      D7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // D8     D9      DA      DB      DC      DD      DE      DF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // E0     E1      E2      E3      E4      E5      E6      E7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // E8     E9      EA      EB      EC      ED      EE      EF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // F0     F1      F2      F3      F4      F5      F6      F7
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
    // F8     F9      FA      FB      FC      FD      FE      FF
    0x01u | 0x02u | 0x04u | 0x08u | 0x10u | 0x20u | 0x40u | 0x80u,
};

namespace
{
    /**
     * @brief 检查字符是否允许出现在Host中
     */
    bool IsForbiddenHostChar(char ch)noexcept
    {
        return ch == '\0' || ch == '\t' || ch == '\n' || ch == '\r' || ch == ' ' || ch == '#' || ch == '%' ||
               ch == '/' || ch == ':' || ch == '?' || ch == '@' || ch == '[' || ch == '\\' || ch == ']';
    }

    /**
     * @brief 解码UTF-8字符
     */
    std::string PercentDecode(const char* start, const char* end)
    {
        string ret;
        if (start >= end || start == nullptr)
            return ret;

        ret.reserve(end - start);
        while (start < end)
        {
            char ch = *start;
            size_t remaining = end - start - 1;
            unsigned a = 0, b = 0;
            if (remaining >= 2 && ch == '%' && StringUtils::HexDigitToNumber(a, start[1]) &&
                StringUtils::HexDigitToNumber(b, start[2]))
            {
                ret.push_back(static_cast<char>(a * 16 + b));
                start += 3;
            }
            else
            {
                ret.push_back(ch);
                ++start;
            }
        }

        return ret;
    }

    /**
     * @brief 向字符串中追加一个字符或者是转义后的序列
     * @param out 结果串
     * @param ch 字符
     * @param charset 转义序列
     */
    void AppendOrEscape(std::string& out, char ch, const uint8_t charset[])
    {
        auto v = static_cast<uint8_t>(ch);
        if (!!(charset[v >> 3] & (1 << (v & 7))))  // 位图查询
            out.append(kHexTable[v]);
        else
            out.push_back(ch);
    }

    /**
     * @brief 解析IPV4中的数字（十进制、十六进制、八进制）
     */
    bool ParseIpv4Number(uint64_t& result, const char* start, const char* end)noexcept
    {
        unsigned radix = 10;
        if (end - start >= 2)
        {
            if (start[0] == '0' && (start[1] == 'x' || start[1] == 'X'))  // 16进制
            {
                start += 2;
                radix = 16;
            }
            else if (start[0] == '0')  // 8进制
            {
                start += 1;
                radix = 8;
            }
        }

        result = 0;
        if (start >= end)
            return true;

        while (start < end)
        {
            char ch = *(start++);

            if (ch >= 'a' && ch <= 'f')
            {
                if (radix != 16)
                    return false;
                if (result != numeric_limits<uint64_t>::max())
                    result = result * 16 + (ch - 'a') + 10;
            }
            else if (ch >= 'A' && ch <= 'F')
            {
                if (radix != 16)
                    return false;
                if (result != numeric_limits<uint64_t>::max())
                    result = result * 16 + (ch - 'A') + 10;
            }
            else if (ch >= '0' && ch <= '9')
            {
                if ((ch == '8' || ch == '9') && radix == 8)
                    return false;
                if (result != numeric_limits<uint64_t>::max())
                    result = result * radix + (ch - '0');
            }
            else
                return false;

            if (result > numeric_limits<uint32_t>::max())
                result = numeric_limits<uint64_t>::max();
        }
        return true;
    }
}

//////////////////////////////////////////////////////////////////////////////// Url::Host

Url::Host::Host()
{
}

Url::Host::Host(const Host& rhs)
    : m_iType(rhs.m_iType)
{
    switch (m_iType)
    {
        case HostTypes::None:
            break;
        case HostTypes::Domain:
            new(&m_stValue.Domain) string(rhs.m_stValue.Domain);
            break;
        case HostTypes::Ipv4:
            m_stValue.Ipv4 = rhs.m_stValue.Ipv4;
            break;
        case HostTypes::Ipv6:
            new(&m_stValue.Ipv6) Ipv6AddressType(rhs.m_stValue.Ipv6);
            break;
        case HostTypes::Opaque:
            new(&m_stValue.Opaque) string(rhs.m_stValue.Opaque);
            break;
        default:
            assert(false);
            break;
    }
}

Url::Host::Host(Host&& rhs)noexcept
    : m_iType(rhs.m_iType)
{
    switch (m_iType)
    {
        case HostTypes::None:
            break;
        case HostTypes::Domain:
            new(&m_stValue.Domain) string(std::move(rhs.m_stValue.Domain));
            break;
        case HostTypes::Ipv4:
            m_stValue.Ipv4 = rhs.m_stValue.Ipv4;
            break;
        case HostTypes::Ipv6:
            new(&m_stValue.Ipv6) Ipv6AddressType(rhs.m_stValue.Ipv6);
            break;
        case HostTypes::Opaque:
            new(&m_stValue.Opaque) string(std::move(rhs.m_stValue.Opaque));
            break;
        default:
            assert(false);
            break;
    }

    rhs.Reset();
}

Url::Host::~Host()
{
    Reset();
}

Url::Host& Url::Host::operator=(const Host& rhs)
{
    Reset();

    switch (rhs.m_iType)
    {
        case HostTypes::None:
            break;
        case HostTypes::Domain:
            new(&m_stValue.Domain) string(rhs.m_stValue.Domain);
            break;
        case HostTypes::Ipv4:
            m_stValue.Ipv4 = rhs.m_stValue.Ipv4;
            break;
        case HostTypes::Ipv6:
            new(&m_stValue.Ipv6) Ipv6AddressType(rhs.m_stValue.Ipv6);
            break;
        case HostTypes::Opaque:
            new(&m_stValue.Opaque) string(rhs.m_stValue.Opaque);
            break;
        default:
            assert(false);
            break;
    }

    m_iType = rhs.m_iType;
    return *this;
}

Url::Host& Url::Host::operator=(Host&& rhs)noexcept
{
    Reset();

    switch (rhs.m_iType)
    {
        case HostTypes::None:
            break;
        case HostTypes::Domain:
            new(&m_stValue.Domain) string(std::move(rhs.m_stValue.Domain));
            break;
        case HostTypes::Ipv4:
            m_stValue.Ipv4 = rhs.m_stValue.Ipv4;
            break;
        case HostTypes::Ipv6:
            new(&m_stValue.Ipv6) Ipv6AddressType(rhs.m_stValue.Ipv6);
            break;
        case HostTypes::Opaque:
            new(&m_stValue.Opaque) string(std::move(rhs.m_stValue.Opaque));
            break;
        default:
            assert(false);
            break;
    }

    m_iType = rhs.m_iType;
    rhs.Reset();
    return *this;
}

bool Url::Host::operator==(const Host& rhs)const noexcept
{
    if (m_iType != rhs.m_iType)
        return false;

    switch (m_iType)
    {
        case HostTypes::None:
            return true;
        case HostTypes::Domain:
            return m_stValue.Domain == rhs.m_stValue.Domain;
        case HostTypes::Ipv4:
            return m_stValue.Ipv4 == rhs.m_stValue.Ipv4;
        case HostTypes::Ipv6:
            return m_stValue.Ipv6 == rhs.m_stValue.Ipv6;
        case HostTypes::Opaque:
            return m_stValue.Opaque == rhs.m_stValue.Opaque;
        default:
            return false;
    }
}

bool Url::Host::operator!=(const Host& rhs)const noexcept
{
    return !operator==(rhs);
}

Url::Host::operator bool()const noexcept
{
    return m_iType != HostTypes::None;
}

Url::Host::HostTypes Url::Host::GetType()const noexcept
{
    return m_iType;
}

const std::string& Url::Host::GetDomain()const noexcept
{
    return m_iType == HostTypes::Domain ? m_stValue.Domain : EmptyRefOf<string>();
}

void Url::Host::SetDomain(const std::string& value)
{
    Reset();

    new(&m_stValue.Domain) string(value);
    m_iType = HostTypes::Domain;
}

void Url::Host::SetDomain(std::string&& value)noexcept
{
    Reset();

    new(&m_stValue.Domain) string(std::move(value));
    m_iType = HostTypes::Domain;
}

uint32_t Url::Host::GetIpv4()const noexcept
{
    return m_iType == HostTypes::Ipv4 ? m_stValue.Ipv4 : 0u;
}

void Url::Host::SetIpv4(uint32_t value)noexcept
{
    Reset();

    m_iType = HostTypes::Ipv4;
    m_stValue.Ipv4 = value;
}

const Url::Host::Ipv6AddressType& Url::Host::GetIpv6()const noexcept
{
    return m_iType == HostTypes::Ipv6 ? m_stValue.Ipv6 : EmptyRefOf<Ipv6AddressType>();
}

void Url::Host::SetIpv6(const Ipv6AddressType& value)noexcept
{
    Reset();

    m_iType = HostTypes::Ipv6;
    m_stValue.Ipv6 = value;
}

const std::string& Url::Host::GetOpaque()const noexcept
{
    return m_iType == HostTypes::Opaque ? m_stValue.Opaque : EmptyRefOf<string>();
}

void Url::Host::SetOpaque(const std::string& value)
{
    Reset();

    new(&m_stValue.Opaque) string(value);
    m_iType = HostTypes::Opaque;
}

void Url::Host::SetOpaque(std::string&& value)noexcept
{
    Reset();

    new(&m_stValue.Opaque) string(std::move(value));
    m_iType = HostTypes::Opaque;
}

void Url::Host::Reset()
{
    switch (m_iType)
    {
        case HostTypes::Domain:
            m_stValue.Domain.~string();
            break;
        case HostTypes::Ipv4:
            break;
        case HostTypes::Ipv6:
            m_stValue.Ipv6.~Ipv6AddressType();
            break;
        case HostTypes::Opaque:
            m_stValue.Opaque.~string();
            break;
        default:
            break;
    }

    m_iType = HostTypes::None;
}

void Url::Host::Parse(const std::string& text, bool special, bool unicode)
{
    Parse(text.c_str(), text.c_str() + text.length(), special, unicode);
}

void Url::Host::Parse(const char* text, bool special, bool unicode)
{
    Parse(text, text + std::strlen(text), special, unicode);
}

void Url::Host::Parse(const char* text, size_t length, bool special, bool unicode)
{
    Parse(text, text + length, special, unicode);
}

std::string Url::Host::ToString()const
{
    string ret;
    switch (m_iType)
    {
        case HostTypes::Domain:
            return m_stValue.Domain;
        case HostTypes::Opaque:
            return m_stValue.Opaque;
        case HostTypes::Ipv4:
            ret.reserve(15);
            for (uint32_t value = m_stValue.Ipv4, n = 0; n < 4; n++)
            {
                char buf[4];
                Convert::ToDecimalString(static_cast<uint8_t>(value % 256), buf);

                ret.insert(0, buf);
                if (n < 3)
                    ret.insert(0, 1, '.');
                value /= 256;
            }
            break;
        case HostTypes::Ipv6:
            ret.reserve(41);
            ret.push_back('[');
            {
                uint32_t start = 0;
                uint32_t compress = 0xFFFFFFFFu;

                // 找到最长的0的部分
                uint32_t cur = 0xFFFFFFFFu, count = 0, longest = 0;
                while (start < 8)
                {
                    if (m_stValue.Ipv6[start] == 0)
                    {
                        if (cur == 0xFFFFFFFFu)
                            cur = start;
                        ++count;
                    }
                    else
                    {
                        if (count > longest && count > 1)
                        {
                            longest = count;
                            compress = cur;
                        }
                        count = 0;
                        cur = 0xFFFFFFFFu;
                    }
                    ++start;
                }
                if (count > longest && count > 1)
                    compress = cur;

                // 序列化过程
                bool ignore0 = false;
                for (unsigned n = 0; n < 8; ++n)
                {
                    auto piece = m_stValue.Ipv6[n];
                    if (ignore0 && piece == 0)
                        continue;
                    else if (ignore0)
                        ignore0 = false;
                    if (compress == n)
                    {
                        ret.append(n == 0 ? "::" : ":");
                        ignore0 = true;
                        continue;
                    }

                    char buf[5];
                    Convert::ToHexStringLower(piece, buf);
                    ret.append(buf);
                    if (n < 7)
                        ret.push_back(':');
                }
            }
            ret.push_back(']');
            break;
        default:
            break;
    }
    return ret;
}

bool Url::Host::IsAsciiFastPath(const std::string& domain)noexcept
{
    static const char kDeliminators[] = { '.' };
    static const char kPunycodePrefix[] = { 'x', 'n', '-', '-', '\0' };

    // 按dot对domain进行分割
    auto it = StringUtils::SplitByCharsBegin<char>(ToArrayView<char>(domain), ArrayView<char>(kDeliminators, 1));
    while (it != StringUtils::SplitByCharsEnd<char>())
    {
        auto label = *it;

        if (label.GetSize() >= sizeof(kPunycodePrefix) - 1 &&
            ::strncmp(kPunycodePrefix, label.GetBuffer(), sizeof(kPunycodePrefix) - 1) == 0)
        {
            return false;
        }

        for (size_t i = 0; i < label.GetSize(); ++i)
        {
            auto ch = label[i];

            if (ch <= 0x002C)
                return false;
            else if (ch == 0x002F)
                return false;
            else if (0x003A <= ch && ch <= 0x0040)
                return false;
            else if (0x005B <= ch && ch <= 0x0060)
                return false;
            else if (0x007B <= ch)
                return false;
        }

        ++it;
    }

    return true;
}

void Url::Host::Parse(const char* start, const char* end, bool special, bool unicode)
{
    if (start >= end || start == nullptr)
    {
        if (start)
        {
            SetDomain(string());
            return;
        }
        MOE_THROW(BadArgumentException, "Invalid argument");
    }

    if (*start == '[')
    {
        if (*(end - 1) != ']')
            MOE_THROW(BadFormatException, "Expected ']' but found {0}", *(end - 1));
        if (!ParseIpv6(start + 1, end - 1))
            MOE_THROW(BadFormatException, "Bad Ipv6 address: {0}", string(start + 1, end - 1));
        return;
    }
    if (!special)
    {
        ParseOpaque(start, end);
        return;
    }

    auto decoded = PercentDecode(start, end);

    u32string temp, temp2;
    auto isFastPath = IsAsciiFastPath(decoded);
    if (!isFastPath)
    {
        temp.reserve(decoded.length());
        temp2.reserve(decoded.length());

        // Punycode ToAscii
        Encoding::Convert<Encoding::UTF32, Encoding::UTF8>(temp, ArrayView<char>(decoded.data(), decoded.length()));
        Idna::ToAscii(temp2, ArrayView<char32_t>(temp.data(), temp.length()), false, true, true, true, false, true);
        Encoding::Convert<Encoding::UTF8, Encoding::UTF32>(decoded, ArrayView<char32_t>(temp2.data(), temp2.length()));
    }
    else
    {
        for (size_t i = 0; i < decoded.length(); ++i)
        {
            auto ch = decoded[i];
            if ('A' <= ch && ch <= 'Z')
                decoded[i] = ch - 'A' + 'a';
        }
    }

    // 检查字符合法性
    for (size_t n = 0; n < decoded.size(); ++n)
    {
        char ch = decoded[n];
        if (IsForbiddenHostChar(ch))
            MOE_THROW(BadFormatException, "Forbidden host character {0}", ch);
    }

    // 检查是否是IPV4地址
    if (ParseIpv4(decoded.c_str(), decoded.c_str() + decoded.length()))
        return;

    if (!isFastPath && unicode)
    {
        // Punycode ToUnicode
        Encoding::Convert<Encoding::UTF32, Encoding::UTF8>(temp, ArrayView<char>(decoded.data(), decoded.length()));
        Idna::ToUnicode(temp2, ArrayView<char32_t>(temp.data(), temp.length()), false, true, true, false, false);
        Encoding::Convert<Encoding::UTF8, Encoding::UTF32>(decoded, ArrayView<char32_t>(temp2.data(), temp2.length()));
    }

    // 不是IPV6或者IPV4，那么就是域名
    SetDomain(std::move(decoded));
}

bool Url::Host::ParseIpv4(const char* start, const char* end)
{
    auto input = start;
    auto mark = start;
    unsigned parts = 0;
    unsigned tooBigParts = 0;
    uint64_t numbers[4] = { 0, 0, 0, 0 };

    while (start <= end)
    {
        auto ch = static_cast<char>(start < end ? *start : '\0');
        if (ch == '.' || ch == '\0')
        {
            if (++parts > 4)
                return false;  // IPV4地址至多只有4个部分
            if (start == mark)
                return false;  // 无效的空的点分部分

            // 解析[mark, start)部分的数字
            uint64_t n = 0;
            if (!ParseIpv4Number(n, mark, start))
                return false;
            if (n > 255)
                ++tooBigParts;

            numbers[parts - 1] = n;
            mark = start + 1;
            if (ch == '.' && start + 1 >= end)
                break;  // 允许输入的最后一个字符是'.'
        }
        ++start;
    }
    assert(parts > 0);

    if (tooBigParts > 1 || (tooBigParts == 1 && numbers[parts - 1] <= 255) ||
        numbers[parts - 1] >= std::pow(256, static_cast<double>(5 - parts)))
    {
        // 规范要求每个点分部分不能超过255，但是最后一个元素例外
        // 此外，整个IPV4的解析结果也要保证不能溢出
        MOE_THROW(BadFormatException, "Bad Ipv4 string: {0}", string(input, end));
    }

    // 计算IPV4值
    auto val = numbers[parts - 1];
    for (unsigned n = 0; n < parts - 1; ++n)
    {
        double b = 3 - n;
        val += numbers[n] * std::pow(256, b);
    }
    SetIpv4(static_cast<uint32_t>(val));
    return true;
}

bool Url::Host::ParseIpv6(const char* start, const char* end)noexcept
{
    auto ch = static_cast<char>(start < end ? *start : '\0');
    unsigned current = 0;  // 指示当前解析的部分
    uint32_t compress = 0xFFFFFFFFu;  // 指示压缩可扩充的位置

    Ipv6AddressType address;
    address.fill(0);

    // 压缩的case '::xxxx'
    if (ch == ':')
    {
        if (end - start < 2 || start[1] != ':')
            return false;  // 无效的':
        start += 2;
        ch = static_cast<char>(start < end ? *start : '\0');
        ++current;
        compress = current;
    }

    while (ch != '\0')
    {
        if (current >= address.max_size())
            return false;  // 无效的地址（至多8个部分）

        // 压缩的case 'fe80::xxxxx'
        if (ch == ':')
        {
            if (compress != 0xFFFFFFFFu)
                return false;  // 不可能同时存在两个压缩部分
            ++start;
            ch = static_cast<char>(start < end ? *start : '\0');
            address[current++] = 0;
            compress = current;
            continue;
        }

        uint32_t value = 0;
        unsigned len = 0;
        while (len < 4 && StringUtils::IsHexDigit(ch))
        {
            value = value * 16 + StringUtils::HexDigitToNumber(ch);

            ++start;
            ch = static_cast<char>(start < end ? *start : '\0');
            ++len;
        }

        switch (ch)
        {
            case '.':  // 内嵌IPV4地址
                if (len == 0)
                    return false;
                start -= len;  // 回退
                ch = static_cast<char>(start < end ? *start : '\0');
                if (current > address.max_size() - 2)  // IPV4地址占两个Piece
                    return false;

                // 解析IPV4部分（这一地址只允许使用十进制点分结构）
                {
                    unsigned numbers = 0;
                    while (ch != '\0')
                    {
                        value = 0xFFFFFFFF;
                        if (numbers > 0)
                        {
                            if (ch == '.' && numbers < 4)
                            {
                                ++start;
                                ch = static_cast<char>(start < end ? *start : '\0');
                            }
                            else
                                return false;
                        }
                        if (!StringUtils::IsDigit(ch))
                            return false;
                        while (StringUtils::IsDigit(ch))
                        {
                            auto number = static_cast<unsigned>(ch - '0');
                            if (value == 0xFFFFFFFF)
                                value = number;
                            else if (value == 0)
                                return false;
                            else
                                value = value * 10 + number;
                            if (value > 255)
                                return false;
                            ++start;
                            ch = static_cast<char>(start < end ? *start : '\0');
                        }
                        address[current] = static_cast<uint16_t>(address[current] * 0x100 + value);
                        ++numbers;
                        if (numbers == 2 || numbers == 4)
                            ++current;
                    }
                    if (numbers != 4)
                        return false;
                }
                continue;
            case ':':
                ++start;
                ch = static_cast<char>(start < end ? *start : '\0');
                if (ch == '\0')
                    return false;
                break;
            case '\0':
                break;
            default:
                return false;
        }
        address[current] = static_cast<uint16_t>(value);
        ++current;
    }

    if (compress != 0xFFFFFFFFu)
    {
        auto swaps = current - compress;
        current = address.max_size() - 1;
        while (current != 0 && swaps > 0)
        {
            auto swap = compress + swaps - 1;
            std::swap(address[current], address[swap]);
            --current;
            --swaps;
        }
    }
    else if (current != address.max_size())  // 没有压缩，则必然读取了所有的部分
        return false;

    SetIpv6(address);
    return true;
}

void Url::Host::ParseOpaque(const char* start, const char* end)
{
    string output;
    output.reserve((end - start) * 3);  // 最坏情况下所有字符都需要转义

    while (start < end)
    {
        char ch = *(start++);
        if (ch != '%' && IsForbiddenHostChar(ch))
            MOE_THROW(BadFormatException, "Forbidden host character {0}", ch);
        AppendOrEscape(output, ch, kC0ControlEncodeSet);
    }

    SetOpaque(std::move(output));
}

//////////////////////////////////////////////////////////////////////////////// Url

namespace
{
    /**
     * @brief 检查是否是控制字符或空格
     */
    bool IsC0ControlOrSpace(char ch)noexcept
    {
        return '\0' <= ch && ch <= ' ';
    }

    /**
     * @brief 检查是否是制表符或者换行符
     */
    bool IsAsciiTabOrNewline(char ch)noexcept
    {
        return ch == '\t' || ch == '\n' || ch == '\r';
    }

    bool IsSpecial(const std::string& str)noexcept
    {
        if (str == "ftp:")
            return true;
        else if (str == "file:")
            return true;
        else if (str == "gopher:")
            return true;
        else if (str == "http:")
            return true;
        else if (str == "https:")
            return true;
        else if (str == "ws:")
            return true;
        else if (str == "wss:")
            return true;
        return false;
    }

    uint16_t GetSpecialPort(const std::string& str)noexcept
    {
        if (str == "ftp:")
            return 21;
        else if (str == "file:")
            return 0;
        else if (str == "gopher:")
            return 70;
        else if (str == "http:")
            return 80;
        else if (str == "https:")
            return 443;
        else if (str == "ws:")
            return 80;
        else if (str == "wss:")
            return 443;
        return 0;
    }

    bool IsWindowsDriveLetter(char a, char b)noexcept
    {
        return StringUtils::IsAlphabet(a) && (b == ':' || b == '|');
    }

    bool IsNormalizedWindowsDriveLetter(char a, char b)noexcept
    {
        return StringUtils::IsAlphabet(a) && b == ':';
    }

    bool IsNormalizedWindowsDriveLetter(const std::string& str)noexcept
    {
        return str.length() >= 2 && StringUtils::IsAlphabet(str[0]) && str[1] == ':';
    }

    bool StartsWithWindowsDriveLetter(const char* p, const char* end)noexcept
    {
        size_t length = end - p;
        return length >= 2 && IsWindowsDriveLetter(p[0], p[1]) && (length == 2 || p[2] == '/' || p[2] == '\\' ||
            p[2] == '?' || p[2] == '#');
    }

    bool IsSingleDotSegment(const std::string& str)noexcept
    {
        switch (str.size())
        {
            case 1:
                return str == ".";
            case 3:
                return str[0] == '%' && str[1] == '2' && StringUtils::ToLower(str[2]) == 'e';
            default:
                return false;
        }
    }

    bool IsDoubleDotSegment(const std::string& str)noexcept
    {
        switch (str.size())
        {
            case 2:
                return str == "..";
            case 4:
                if (str[0] != '.' && str[0] != '%')
                    return false;
                return ((str[0] == '.' && str[1] == '%' && str[2] == '2' && StringUtils::ToLower(str[3]) == 'e') ||
                    (str[0] == '%' && str[1] == '2' && StringUtils::ToLower(str[2]) == 'e' && str[3] == '.'));
            case 6:
                return (str[0] == '%' && str[1] == '2' && StringUtils::ToLower(str[2]) == 'e' && str[3] == '%' &&
                    str[4] == '2' && StringUtils::ToLower(str[5]) == 'e');
            default:
                return false;
        }
    }
}

Url::Url(ArrayView<char> url, ArrayView<char> base)
{
    if (!base.IsEmpty())
    {
        Url parsedBase(base);
        Parse(url, &parsedBase);
    }
    else
        Parse(url);
}

bool Url::operator==(const Url& rhs)const noexcept
{
    if (m_uFlags != rhs.m_uFlags)
        return false;
    if (m_stScheme != rhs.m_stScheme)
        return false;
    if ((m_uFlags & FLAGS_HAS_USERNAME) && m_stUsername != rhs.m_stUsername)
        return false;
    if ((m_uFlags & FLAGS_HAS_PASSWORD) && m_stPassword != rhs.m_stPassword)
        return false;
    if (m_stHost != rhs.m_stHost)
        return false;
    if ((m_uFlags & FLAGS_HAS_PORT) && m_uPort != rhs.m_uPort)
        return false;
    if ((m_uFlags & FLAGS_HAS_QUERY) && m_stQuery != rhs.m_stQuery)
        return false;
    if ((m_uFlags & FLAGS_HAS_FRAGMENT) && m_stFragment != rhs.m_stFragment)
        return false;
    if (m_stPath != rhs.m_stPath)
        return false;
    return true;
}

bool Url::operator!=(const Url& rhs)const noexcept
{
    return !operator==(rhs);
}

void Url::SetScheme(ArrayView<char> value)
{
    Parse(value, nullptr, PARSE_STATE_SCHEME_START, false);
}

bool Url::HasUsername()const noexcept
{
    return (m_uFlags & FLAGS_HAS_USERNAME) != 0;
}

const std::string& Url::GetUsername()const noexcept
{
    if (HasUsername())
        return m_stUsername;
    return EmptyRefOf<string>();
}

void Url::SetUsername(ArrayView<char> value)
{
    if (m_stScheme == "file:")
        MOE_THROW(InvalidCallException, "Protocol \"file:\" cannot have username");
    if (!m_stHost)
        MOE_THROW(InvalidCallException, "Host is not set");

    m_uFlags &= ~FLAGS_HAS_USERNAME;
    if (!value.IsEmpty())
    {
        m_stUsername.assign(value.GetBuffer(), value.GetSize());
        m_uFlags |= FLAGS_HAS_USERNAME;
    }
}

bool Url::HasPassword()const noexcept
{
    return (m_uFlags & FLAGS_HAS_PASSWORD) != 0;
}

const std::string& Url::GetPassword()const noexcept
{
    if (HasPassword())
        return m_stPassword;
    return EmptyRefOf<string>();
}

void Url::SetPassword(ArrayView<char> value)
{
    if (m_stScheme == "file:")
        MOE_THROW(InvalidCallException, "Protocol \"file:\" cannot have username");
    if (!m_stHost)
        MOE_THROW(InvalidCallException, "Host is not set");

    m_uFlags &= ~FLAGS_HAS_PASSWORD;
    if (!value.IsEmpty())
    {
        m_stPassword.assign(value.GetBuffer(), value.GetSize());
        m_uFlags |= FLAGS_HAS_PASSWORD;
    }
}

bool Url::HasHost()const noexcept
{
    return (m_uFlags & FLAGS_HAS_HOST) != 0;
}

const Url::Host& Url::GetHost()const noexcept
{
    return m_stHost;
}

void Url::SetHost(const Host& host)
{
    if (IsCannotBeBase())
        MOE_THROW(InvalidCallException, "Cannot be base");

    m_uFlags &= ~FLAGS_HAS_HOST;
    if (host)
    {
        if (!(host.GetType() == Host::HostTypes::Domain && host.GetDomain().empty()) &&
            !(host.GetType() == Host::HostTypes::Opaque && host.GetOpaque().empty()))
        {
            m_uFlags |= FLAGS_HAS_HOST;
            m_stHost = host;
        }
    }
}

void Url::SetHost(Host&& host)
{
    if (IsCannotBeBase())
        MOE_THROW(InvalidCallException, "Cannot be base");

    m_uFlags &= ~FLAGS_HAS_HOST;
    if (host)
    {
        if (!(host.GetType() == Host::HostTypes::Domain && host.GetDomain().empty()) &&
            !(host.GetType() == Host::HostTypes::Opaque && host.GetOpaque().empty()))
        {
            m_uFlags |= FLAGS_HAS_HOST;
            m_stHost = std::move(host);
        }
    }
    host.Reset();
}

void Url::SetHost(ArrayView<char> value)
{
    if (IsCannotBeBase())
        MOE_THROW(InvalidCallException, "Cannot be base");
    m_uFlags &= ~FLAGS_HAS_HOST;
    Parse(value, nullptr, PARSE_STATE_HOST, false);
}

bool Url::HasPort()const noexcept
{
    return (m_uFlags & FLAGS_HAS_PORT) != 0;
}

uint16_t Url::GetPort()const noexcept
{
    if (HasPort())
        return m_uPort;
    return 0;
}

void Url::SetPort(uint16_t value)
{
    if (m_stScheme == "file:")
        MOE_THROW(InvalidCallException, "Protocol \"file:\" cannot have username");
    if (!m_stHost)
        MOE_THROW(InvalidCallException, "Host is not set");

    m_uPort = value;
    m_uFlags |= FLAGS_HAS_PORT;
    NormalizePort();
}

bool Url::HasPath()const noexcept
{
    return (m_uFlags & FLAGS_HAS_PATH) != 0;
}

const std::vector<std::string>& Url::GetPath()const noexcept
{
    if (m_stPath.empty())
        return EmptyRefOf<vector<string>>();
    return m_stPath;
}

void Url::SetPath(ArrayView<char> value)
{
    if (IsCannotBeBase())
        MOE_THROW(InvalidCallException, "Cannot be base");
    m_stPath.clear();
    m_uFlags &= ~FLAGS_HAS_PATH;
    Parse(value, nullptr, PARSE_STATE_PATH_START, false);
}

bool Url::HasQuery()const noexcept
{
    return (m_uFlags & FLAGS_HAS_QUERY) != 0;
}

const std::string& Url::GetQuery()const noexcept
{
    if (HasQuery())
        return m_stQuery;
    return EmptyRefOf<string>();
}

void Url::SetQuery(ArrayView<char> value)
{
    m_uFlags &= ~FLAGS_HAS_QUERY;
    if (!value.IsEmpty())
    {
        if (value[0] == '?')
            value = value.Slice(1, value.GetSize());
        Parse(value, nullptr, PARSE_STATE_QUERY, false);
    }
}

bool Url::HasFragment()const noexcept
{
    return (m_uFlags & FLAGS_HAS_FRAGMENT) != 0;
}

const std::string& Url::GetFragment()const noexcept
{
    if (HasFragment())
        return m_stFragment;
    return EmptyRefOf<string>();
}

void Url::SetFragment(ArrayView<char> value)
{
    m_uFlags &= ~FLAGS_HAS_FRAGMENT;
    if (!value.IsEmpty())
    {
        if (value[0] == '#')
            value = value.Slice(1, value.GetSize());
        Parse(value, nullptr, PARSE_STATE_FRAGMENT, false);
    }
}

std::string Url::GetPortStandard()const
{
    if (HasPort())
        return StringUtils::ToString(m_uPort);
    return EmptyRefOf<string>();
}

std::string Url::GetPathStandard()const
{
    if (m_stPath.empty())
        return EmptyRefOf<string>();
    if (IsCannotBeBase())
        return m_stPath[0];

    string ret;
    for (const auto& str : m_stPath)
    {
        ret.push_back('/');
        ret.append(str);
    }
    return ret;
}

std::string Url::GetQueryStandard()const
{
    string ret = GetQuery();
    if (ret.empty())
        return ret;
    ret.insert(0, 1, '?');
    return ret;
}

std::string Url::GetFragmentStandard()const
{
    string ret = GetFragment();
    if (ret.empty())
        return ret;
    ret.insert(0, 1, '#');
    return ret;
}

void Url::Parse(ArrayView<char> src, Url* base, bool trimWhitespace)
{
    Reset();
    Parse(src, base, PARSE_STATE_UNKNOWN, trimWhitespace);
}

void Url::Reset()noexcept
{
    m_uFlags = 0;
    m_stScheme.clear();
    m_stUsername.clear();
    m_stPassword.clear();
    m_stHost.Reset();
    m_uPort = 0;
    m_stQuery.clear();
    m_stFragment.clear();
    m_stPath.clear();
}

std::string Url::ToString(bool excludeFragmentFlag)const
{
    string ret;
    ret.reserve(m_stScheme.length() + m_stUsername.length() + m_stPassword.length() + m_stQuery.length() +
        m_stFragment.length() + m_stPath.size() * 4 + 4);

    ret.append(m_stScheme);
    assert(ret.empty() || ret.back() == ':');

    if (HasHost())
    {
        ret.append("//");
        if (HasUsername() || HasPassword())
        {
            ret.append(m_stUsername);
            if (HasPassword())
            {
                ret.push_back(':');
                ret.append(m_stPassword);
            }
            ret.push_back('@');
        }

        ret.append(m_stHost.ToString());

        if (HasPort())
        {
            ret.push_back(':');

            char buf[8];
            Convert::ToDecimalString(m_uPort, buf);
            ret.append(buf);
        }
    }
    else if (m_stScheme == "file" || m_stScheme == "file:")
        ret.append("//");

    if (IsCannotBeBase())
    {
        assert(!m_stPath.empty());
        ret.append(m_stPath[0]);
    }
    else
    {
        for (const auto& str : m_stPath)
        {
            ret.push_back('/');
            ret.append(str);
        }
    }

    if (HasQuery())
    {
        ret.push_back('?');
        ret.append(m_stQuery);
    }

    if (!excludeFragmentFlag && HasFragment())
    {
        ret.push_back('#');
        ret.append(m_stFragment);
    }
    return ret;
}

void Url::NormalizePort()noexcept
{
    if (m_uFlags & FLAGS_HAS_PORT)
    {
        auto port = GetSpecialPort(m_stScheme);
        if (port != 0 && port == m_uPort)
            m_uFlags &= ~FLAGS_HAS_PORT;
    }
}

void Url::ShortenUrlPath()noexcept
{
    if (m_stPath.empty())
        return;
    if (m_stPath.size() == 1 && m_stScheme == "file:" && IsNormalizedWindowsDriveLetter(m_stPath[0]))
        return;
    m_stPath.pop_back();
}

void Url::Parse(ArrayView<char> input, Url* base, URL_PARSE_STATES stateOverride, bool trimWhitespace)
{
    const char* start = input.GetBuffer();
    const char* end = input.GetBuffer() + input.GetSize();
    size_t length = end - start;

    // 剔除首尾的换行符和控制字符
    if (trimWhitespace)
    {
        for (const char* p = start; start < end; ++p)
        {
            if (IsC0ControlOrSpace(*p))
                ++start;
            else
                break;
        }
        for (const char* p = end - 1; p >= start; --p)
        {
            if (IsC0ControlOrSpace(*p))
                --end;
            else
                break;
        }
        length = end - start;
    }

    // 标准要求从字符串中去掉ASCII的TAB或者换行符
    // 当且仅当出现这种情况时才花费额外代价
    string whitespaceStripped;
    for (const char* p = start; p < end; ++p)
    {
        if (!IsAsciiTabOrNewline(*p))
            continue;

        // 如果遇到了TAB和换行，分配空间
        whitespaceStripped.reserve(length);
        whitespaceStripped.assign(start, p - start);

        // 从 p + 1 继续
        for (p = p + 1; p < end; ++p)
        {
            if (!IsAsciiTabOrNewline(*p))
                whitespaceStripped.push_back(*p);
        }

        start = whitespaceStripped.data();
        end  = whitespaceStripped.data() + whitespaceStripped.length();
        length = end - start;
        break;
    }

    // 决定解析器初始状态
    bool hasStateOverride = stateOverride != PARSE_STATE_UNKNOWN;
    URL_PARSE_STATES state = hasStateOverride ? stateOverride : PARSE_STATE_SCHEME_START;
    if (state < PARSE_STATE_SCHEME_START || state > PARSE_STATE_FRAGMENT)
        MOE_THROW(BadArgumentException, "Invalid parse state");

    // 驱动状态机
    string buffer;
    auto p = start;
    bool atFlag = false;  // 遇到'@'时设为true
    bool squareBracketFlag = false;  // 在'[...]'中时设为true
    bool passwordTokenSeenFlag = false;  // 在用户名后跟密码时设为true
    while (p <= end)
    {
        auto ch = static_cast<char>(p < end ? *p : '\0');

        switch (state)
        {
            case PARSE_STATE_SCHEME_START:
                if (StringUtils::IsAlphabet(ch))
                {
                    buffer.push_back(StringUtils::ToLower(ch));
                    state = PARSE_STATE_SCHEME;
                }
                else if (!hasStateOverride)
                {
                    state = PARSE_STATE_NO_SCHEME;
                    continue;
                }
                else
                {
                    MOE_THROW(BadFormatException, "Unexpected character {0} near position {1}", ch,
                        p - input.GetBuffer());
                }
                break;
            case PARSE_STATE_SCHEME:
                if (StringUtils::IsAlphabet(ch) || StringUtils::IsDigit(ch) || ch == '+' || ch == '-' || ch == '.')
                    buffer.push_back(StringUtils::ToLower(ch));
                else if (ch == ':' || (hasStateOverride && ch == '\0'))
                {
                    if (hasStateOverride && buffer.size() == 0)
                        return;

                    buffer.push_back(':');

                    bool special = static_cast<bool>(m_uFlags & FLAGS_SPECIAL);
                    bool newIsSpecial = ::IsSpecial(buffer);

                    if (hasStateOverride)
                    {
                        if (special != newIsSpecial || (buffer == "file:" && ((m_uFlags & FLAGS_HAS_USERNAME) ||
                            (m_uFlags & FLAGS_HAS_PASSWORD) || (m_uFlags & FLAGS_HAS_PORT))))
                        {
                            MOE_THROW(BadFormatException, "Invalid protocol at this time");
                        }
                    }

                    m_stScheme = std::move(buffer);
                    NormalizePort();
                    if (newIsSpecial)
                    {
                        m_uFlags |= FLAGS_SPECIAL;
                        special = true;
                    }
                    else
                    {
                        m_uFlags &= ~FLAGS_SPECIAL;
                        special = false;
                    }

                    if (hasStateOverride)
                        return;

                    buffer.clear();
                    if (m_stScheme == "file:")
                        state = PARSE_STATE_FILE;
                    else if (special && base && m_stScheme == base->m_stScheme)
                        state = PARSE_STATE_SPECIAL_RELATIVE_OR_AUTHORITY;
                    else if (special)
                        state = PARSE_STATE_SPECIAL_AUTHORITY_SLASHES;
                    else if (p < end && p[1] == '/')
                    {
                        state = PARSE_STATE_PATH_OR_AUTHORITY;
                        ++p;
                    }
                    else
                    {
                        m_uFlags |= FLAGS_CANNOT_BE_BASE;
                        m_uFlags |= FLAGS_HAS_PATH;
                        m_stPath.emplace_back("");
                        state = PARSE_STATE_CANNOT_BE_BASE;
                    }
                }
                else if (!hasStateOverride)
                {
                    buffer.clear();
                    state = PARSE_STATE_NO_SCHEME;
                    p = start;
                    continue;
                }
                else
                {
                    MOE_THROW(BadFormatException, "Unexpected character {0} near position {1}", ch,
                        p - input.GetBuffer());
                }
                break;
            case PARSE_STATE_NO_SCHEME:
                {
                    bool cannotBeBase = base && (base->m_uFlags & FLAGS_CANNOT_BE_BASE);
                    if (!base || (cannotBeBase && ch != '#'))
                        MOE_THROW(BadFormatException, "A base URL is required");
                    else if (cannotBeBase && ch == '#')
                    {
                        m_stScheme = base->m_stScheme;
                        if (::IsSpecial(m_stScheme))
                            m_uFlags |= FLAGS_SPECIAL;
                        else
                            m_uFlags &= ~FLAGS_SPECIAL;

                        if (base->m_uFlags & FLAGS_HAS_PATH)
                        {
                            m_uFlags |= FLAGS_HAS_PATH;
                            m_stPath = base->m_stPath;
                        }
                        if (base->m_uFlags & FLAGS_HAS_QUERY)
                        {
                            m_uFlags |= FLAGS_HAS_QUERY;
                            m_stQuery = base->m_stQuery;
                        }
                        if (base->m_uFlags & FLAGS_HAS_FRAGMENT)
                        {
                            m_uFlags |= FLAGS_HAS_FRAGMENT;
                            m_stFragment = base->m_stFragment;
                        }

                        m_uFlags |= FLAGS_CANNOT_BE_BASE;
                        state = PARSE_STATE_FRAGMENT;
                    }
                    else if (base && base->m_stScheme != "file:")
                    {
                        state = PARSE_STATE_RELATIVE;
                        continue;
                    }
                    else
                    {
                        m_stScheme = "file:";
                        m_uFlags |= FLAGS_SPECIAL;
                        state = PARSE_STATE_FILE;
                        continue;
                    }
                }
                break;
            case PARSE_STATE_SPECIAL_RELATIVE_OR_AUTHORITY:
                if (ch == '/' && (p < end && p[1] == '/'))
                {
                    state = PARSE_STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES;
                    p++;
                }
                else
                {
                    state = PARSE_STATE_RELATIVE;
                    continue;
                }
                break;
            case PARSE_STATE_PATH_OR_AUTHORITY:
                if (ch == '/')
                    state = PARSE_STATE_AUTHORITY;
                else
                {
                    state = PARSE_STATE_PATH;
                    continue;
                }
                break;
            case PARSE_STATE_RELATIVE:
                assert(base);
                m_stScheme = base->m_stScheme;
                if (::IsSpecial(m_stScheme))
                    m_uFlags |= FLAGS_SPECIAL;
                else
                    m_uFlags &= ~FLAGS_SPECIAL;

                switch (ch)
                {
                    case '\0':
                        if (base->m_uFlags & FLAGS_HAS_USERNAME)
                        {
                            m_uFlags |= FLAGS_HAS_USERNAME;
                            m_stUsername = base->m_stUsername;
                        }
                        if (base->m_uFlags & FLAGS_HAS_PASSWORD)
                        {
                            m_uFlags |= FLAGS_HAS_PASSWORD;
                            m_stPassword = base->m_stPassword;
                        }
                        if (base->m_uFlags & FLAGS_HAS_HOST)
                        {
                            m_uFlags |= FLAGS_HAS_HOST;
                            m_stHost = base->m_stHost;
                        }
                        if (base->m_uFlags & FLAGS_HAS_PORT)
                        {
                            m_uFlags |= FLAGS_HAS_PORT;
                            m_uPort = base->m_uPort;
                        }
                        if (base->m_uFlags & FLAGS_HAS_QUERY)
                        {
                            m_uFlags |= FLAGS_HAS_QUERY;
                            m_stQuery = base->m_stQuery;
                        }
                        if (base->m_uFlags & FLAGS_HAS_PATH)
                        {
                            m_uFlags |= FLAGS_HAS_PATH;
                            m_stPath = base->m_stPath;
                        }
                        break;
                    case '/':
                        state = PARSE_STATE_RELATIVE_SLASH;
                        break;
                    case '?':
                        if (base->m_uFlags & FLAGS_HAS_USERNAME)
                        {
                            m_uFlags |= FLAGS_HAS_USERNAME;
                            m_stUsername = base->m_stUsername;
                        }
                        if (base->m_uFlags & FLAGS_HAS_PASSWORD)
                        {
                            m_uFlags |= FLAGS_HAS_PASSWORD;
                            m_stPassword = base->m_stPassword;
                        }
                        if (base->m_uFlags & FLAGS_HAS_HOST)
                        {
                            m_uFlags |= FLAGS_HAS_HOST;
                            m_stHost = base->m_stHost;
                        }
                        if (base->m_uFlags & FLAGS_HAS_PORT)
                        {
                            m_uFlags |= FLAGS_HAS_PORT;
                            m_uPort = base->m_uPort;
                        }
                        if (base->m_uFlags & FLAGS_HAS_PATH)
                        {
                            m_uFlags |= FLAGS_HAS_PATH;
                            m_stPath = base->m_stPath;
                        }

                        state = PARSE_STATE_QUERY;
                        break;
                    case '#':
                        if (base->m_uFlags & FLAGS_HAS_USERNAME)
                        {
                            m_uFlags |= FLAGS_HAS_USERNAME;
                            m_stUsername = base->m_stUsername;
                        }
                        if (base->m_uFlags & FLAGS_HAS_PASSWORD)
                        {
                            m_uFlags |= FLAGS_HAS_PASSWORD;
                            m_stPassword = base->m_stPassword;
                        }
                        if (base->m_uFlags & FLAGS_HAS_HOST)
                        {
                            m_uFlags |= FLAGS_HAS_HOST;
                            m_stHost = base->m_stHost;
                        }
                        if (base->m_uFlags & FLAGS_HAS_PORT)
                        {
                            m_uFlags |= FLAGS_HAS_PORT;
                            m_uPort = base->m_uPort;
                        }
                        if (base->m_uFlags & FLAGS_HAS_QUERY)
                        {
                            m_uFlags |= FLAGS_HAS_QUERY;
                            m_stQuery = base->m_stQuery;
                        }
                        if (base->m_uFlags & FLAGS_HAS_PATH)
                        {
                            m_uFlags |= FLAGS_HAS_PATH;
                            m_stPath = base->m_stPath;
                        }

                        state = PARSE_STATE_FRAGMENT;
                        break;
                    default:
                        {
                            auto special = static_cast<bool>(m_uFlags & FLAGS_SPECIAL);
                            bool specialBackSlash = (special && ch == '\\');

                            if (specialBackSlash)
                                state = PARSE_STATE_RELATIVE_SLASH;
                            else
                            {
                                if (base->m_uFlags & FLAGS_HAS_USERNAME)
                                {
                                    m_uFlags |= FLAGS_HAS_USERNAME;
                                    m_stUsername = base->m_stUsername;
                                }
                                if (base->m_uFlags & FLAGS_HAS_PASSWORD)
                                {
                                    m_uFlags |= FLAGS_HAS_PASSWORD;
                                    m_stPassword = base->m_stPassword;
                                }
                                if (base->m_uFlags & FLAGS_HAS_HOST)
                                {
                                    m_uFlags |= FLAGS_HAS_HOST;
                                    m_stHost = base->m_stHost;
                                }
                                if (base->m_uFlags & FLAGS_HAS_PORT)
                                {
                                    m_uFlags |= FLAGS_HAS_PORT;
                                    m_uPort = base->m_uPort;
                                }
                                if (base->m_uFlags & FLAGS_HAS_PATH)
                                {
                                    m_uFlags |= FLAGS_HAS_PATH;
                                    m_stPath = base->m_stPath;
                                    ShortenUrlPath();
                                }

                                state = PARSE_STATE_PATH;
                                continue;
                            }
                        }
                        break;
                }
                break;
            case PARSE_STATE_RELATIVE_SLASH:
                if (::IsSpecial(m_stScheme) && (ch == '/' || ch == '\\'))
                    state = PARSE_STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES;
                else if (ch == '/')
                    state = PARSE_STATE_AUTHORITY;
                else
                {
                    if (base->m_uFlags & FLAGS_HAS_USERNAME)
                    {
                        m_uFlags |= FLAGS_HAS_USERNAME;
                        m_stUsername = base->m_stUsername;
                    }
                    if (base->m_uFlags & FLAGS_HAS_PASSWORD)
                    {
                        m_uFlags |= FLAGS_HAS_PASSWORD;
                        m_stPassword = base->m_stPassword;
                    }
                    if (base->m_uFlags & FLAGS_HAS_HOST)
                    {
                        m_uFlags |= FLAGS_HAS_HOST;
                        m_stHost = base->m_stHost;
                    }
                    if (base->m_uFlags & FLAGS_HAS_PORT)
                    {
                        m_uFlags |= FLAGS_HAS_PORT;
                        m_uPort = base->m_uPort;
                    }

                    state = PARSE_STATE_PATH;
                    continue;
                }
                break;
            case PARSE_STATE_SPECIAL_AUTHORITY_SLASHES:
                state = PARSE_STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES;
                if (ch == '/' && (p < end && p[1] == '/'))
                    ++p;
                else
                    continue;
                break;
            case PARSE_STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES:
                if (ch != '/' && ch != '\\')
                {
                    state = PARSE_STATE_AUTHORITY;
                    continue;
                }
                break;
            case PARSE_STATE_AUTHORITY:
                {
                    auto special = static_cast<bool>(m_uFlags & FLAGS_SPECIAL);
                    bool specialBackSlash = (special && ch == '\\');

                    if (ch == '@')
                    {
                        if (atFlag)
                        {
                            buffer.reserve(buffer.size() + 3);
                            buffer.insert(0, "%40");
                        }
                        atFlag = true;

                        if (buffer.length() > 0 && buffer[0] != ':')
                            m_uFlags |= FLAGS_HAS_USERNAME;

                        for (size_t n = 0; n < buffer.length(); ++n)
                        {
                            char bch = buffer[n];
                            if (bch == ':')
                            {
                                m_uFlags |= FLAGS_HAS_PASSWORD;
                                if (!passwordTokenSeenFlag)
                                {
                                    passwordTokenSeenFlag = true;
                                    continue;
                                }
                            }

                            if (passwordTokenSeenFlag)
                                AppendOrEscape(m_stPassword, bch, kUserInfoEncodeSet);
                            else
                                AppendOrEscape(m_stUsername, bch, kUserInfoEncodeSet);
                        }
                        buffer.clear();
                    }
                    else if (ch == '\0' || ch == '/' || ch == '?' || ch == '#' || specialBackSlash)
                    {
                        if (atFlag && buffer.empty())
                        {
                            MOE_THROW(BadFormatException, "Unexpected character {0} near position {1}", ch,
                                p - input.GetBuffer());
                        }

                        p -= buffer.size() + 1;
                        buffer.clear();
                        state = PARSE_STATE_HOST;
                    }
                    else
                        buffer += ch;
                }
                break;
            case PARSE_STATE_HOST:
            case PARSE_STATE_HOSTNAME:
                {
                    if (hasStateOverride && m_stScheme == "file:")
                    {
                        state = PARSE_STATE_FILE_HOST;
                        continue;
                    }

                    auto special = static_cast<bool>(m_uFlags & FLAGS_SPECIAL);
                    bool specialBackSlash = (special && ch == '\\');

                    if (ch == ':' && !squareBracketFlag)
                    {
                        if (buffer.empty())
                        {
                            MOE_THROW(BadFormatException, "Unexpected character {0} near position {1}", ch,
                                p - input.GetBuffer());
                        }

                        m_stHost.Parse(buffer, special);
                        m_uFlags |= FLAGS_HAS_HOST;

                        if (stateOverride == PARSE_STATE_HOSTNAME)
                            return;

                        buffer.clear();
                        state = PARSE_STATE_PORT;
                    }
                    else if (ch == '\0' || ch == '/' || ch == '?' || ch == '#' || specialBackSlash)
                    {
                        --p;
                        if (special && buffer.empty())
                        {
                            MOE_THROW(BadFormatException, "Unexpected character {0} near position {1}", ch,
                                p - input.GetBuffer());
                        }
                        if (hasStateOverride && buffer.empty() && (!m_stUsername.empty() || !m_stPassword.empty() ||
                            m_uFlags & FLAGS_HAS_PORT))
                        {
                            return;
                        }

                        m_stHost.Parse(buffer, special);
                        m_uFlags |= FLAGS_HAS_HOST;

                        if (hasStateOverride)
                            return;

                        buffer.clear();
                        state = PARSE_STATE_PATH_START;
                    }
                    else
                    {
                        if (ch == '[')
                            squareBracketFlag = true;
                        if (ch == ']')
                            squareBracketFlag = false;
                        buffer += ch;
                    }
                }
                break;
            case PARSE_STATE_PORT:
                {
                    auto special = static_cast<bool>(m_uFlags & FLAGS_SPECIAL);
                    bool specialBackSlash = (special && ch == '\\');

                    if (StringUtils::IsDigit(ch))
                        buffer += ch;
                    else if (hasStateOverride || ch == '\0' || ch == '/' || ch == '?' || ch == '#' || specialBackSlash)
                    {
                        if (!buffer.empty())
                        {
                            unsigned port = 0;

                            for (size_t i = 0; port <= 0xFFFF && i < buffer.size(); ++i)
                                port = port * 10 + buffer[i] - '0';
                            if (port > 0xFFFF)
                            {
                                if (stateOverride == PARSE_STATE_HOST)
                                {
                                    m_uPort = 0;
                                    m_uFlags &= ~FLAGS_HAS_PORT;
                                    return;
                                }

                                MOE_THROW(BadFormatException, "Port number overflowed near position {0}",
                                    p - input.GetBuffer());
                            }

                            m_uPort = static_cast<uint16_t>(port);
                            m_uFlags |= FLAGS_HAS_PORT;
                            NormalizePort();
                            buffer.clear();
                        }
                        else if (hasStateOverride)
                        {
                            if (stateOverride == PARSE_STATE_HOST)
                            {
                                m_uPort = 0;
                                m_uFlags &= ~FLAGS_HAS_PORT;
                            }
                            return;
                        }

                        state = PARSE_STATE_PATH_START;
                        continue;
                    }
                    else
                    {
                        MOE_THROW(BadFormatException, "Unexpected character {0} near position {1}", ch,
                            p - input.GetBuffer());
                    }
                }
                break;
            case PARSE_STATE_FILE:
                m_stScheme = "file:";
                if (ch == '/' || ch == '\\')
                    state = PARSE_STATE_FILE_SLASH;
                else if (base && base->m_stScheme == "file:")
                {
                    switch (ch)
                    {
                        case '\0':
                            if (base->m_uFlags & FLAGS_HAS_HOST)
                            {
                                m_uFlags |= FLAGS_HAS_HOST;
                                m_stHost = base->m_stHost;
                            }
                            if (base->m_uFlags & FLAGS_HAS_PATH)
                            {
                                m_uFlags |= FLAGS_HAS_PATH;
                                m_stPath = base->m_stPath;
                            }
                            if (base->m_uFlags & FLAGS_HAS_QUERY)
                            {
                                m_uFlags |= FLAGS_HAS_QUERY;
                                m_stQuery = base->m_stQuery;
                            }
                            break;
                        case '?':
                            if (base->m_uFlags & FLAGS_HAS_HOST)
                            {
                                m_uFlags |= FLAGS_HAS_HOST;
                                m_stHost = base->m_stHost;
                            }
                            if (base->m_uFlags & FLAGS_HAS_PATH)
                            {
                                m_uFlags |= FLAGS_HAS_PATH;
                                m_stPath = base->m_stPath;
                            }
                            m_uFlags |= FLAGS_HAS_QUERY;
                            m_stQuery.clear();
                            state = PARSE_STATE_QUERY;
                            break;
                        case '#':
                            if (base->m_uFlags & FLAGS_HAS_HOST)
                            {
                                m_uFlags |= FLAGS_HAS_HOST;
                                m_stHost = base->m_stHost;
                            }
                            if (base->m_uFlags & FLAGS_HAS_PATH)
                            {
                                m_uFlags |= FLAGS_HAS_PATH;
                                m_stPath = base->m_stPath;
                            }
                            if (base->m_uFlags & FLAGS_HAS_QUERY)
                            {
                                m_uFlags |= FLAGS_HAS_QUERY;
                                m_stQuery = base->m_stQuery;
                            }
                            m_uFlags |= FLAGS_HAS_FRAGMENT;
                            m_stFragment.clear();
                            state = PARSE_STATE_FRAGMENT;
                            break;
                        default:
                            if (!StartsWithWindowsDriveLetter(p, end))
                            {
                                if (base->m_uFlags & FLAGS_HAS_HOST)
                                {
                                    m_uFlags |= FLAGS_HAS_HOST;
                                    m_stHost = base->m_stHost;
                                }
                                if (base->m_uFlags & FLAGS_HAS_PATH)
                                {
                                    m_uFlags |= FLAGS_HAS_PATH;
                                    m_stPath = base->m_stPath;
                                }
                                ShortenUrlPath();
                            }
                            state = PARSE_STATE_PATH;
                            continue;
                    }
                }
                else
                {
                    state = PARSE_STATE_PATH;
                    continue;
                }
                break;
            case PARSE_STATE_FILE_SLASH:
                if (ch == '/' || ch == '\\')
                    state = PARSE_STATE_FILE_HOST;
                else
                {
                    if (base && base->m_stScheme == "file:" && !StartsWithWindowsDriveLetter(p, end))
                    {
                        assert(base->m_stPath.size() > 0);
                        if (base->m_stPath[0].length() >= 2 &&
                            IsNormalizedWindowsDriveLetter(base->m_stPath[0][0], base->m_stPath[0][1]))
                        {
                            m_uFlags |= FLAGS_HAS_PATH;
                            m_stPath.push_back(base->m_stPath[0]);
                        }
                        else
                        {
                            if (base->m_uFlags & FLAGS_HAS_HOST)
                            {
                                m_uFlags |= FLAGS_HAS_HOST;
                                m_stHost = base->m_stHost;
                            }
                            else
                            {
                                m_uFlags &= ~FLAGS_HAS_HOST;
                                m_stHost.Reset();
                            }
                        }
                    }
                    state = PARSE_STATE_PATH;
                    continue;
                }
                break;
            case PARSE_STATE_FILE_HOST:
                if (ch == '\0' || ch == '/' || ch == '\\' || ch == '?' || ch == '#')
                {
                    if (!hasStateOverride && buffer.size() == 2 && IsWindowsDriveLetter(buffer[0], buffer[1]))
                        state = PARSE_STATE_PATH;
                    else if (buffer.empty())
                    {
                        m_uFlags |= FLAGS_HAS_HOST;
                        m_stHost.Reset();
                        if (hasStateOverride)
                            return;
                        state = PARSE_STATE_PATH_START;
                    }
                    else
                    {
                        auto special = static_cast<bool>(m_uFlags & FLAGS_SPECIAL);

                        m_stHost.Parse(buffer, special);
                        if (m_stHost.GetDomain() == "localhost")
                            m_stHost.Reset();
                        else if (m_stHost.GetOpaque() == "localhost")
                            m_stHost.Reset();

                        m_uFlags |= FLAGS_HAS_HOST;

                        if (hasStateOverride)
                            return;
                        buffer.clear();
                        state = PARSE_STATE_PATH_START;
                    }
                    continue;
                }
                else
                    buffer += ch;
                break;
            case PARSE_STATE_PATH_START:
                if (::IsSpecial(m_stScheme))
                {
                    state = PARSE_STATE_PATH;
                    if (ch != '/' && ch != '\\')
                        continue;
                }
                else if (!hasStateOverride && ch == '?')
                {
                    m_uFlags |= FLAGS_HAS_QUERY;
                    m_stQuery.clear();
                    state = PARSE_STATE_QUERY;
                }
                else if (!hasStateOverride && ch == '#')
                {
                    m_uFlags |= FLAGS_HAS_FRAGMENT;
                    m_stFragment.clear();
                    state = PARSE_STATE_FRAGMENT;
                }
                else if (ch != '\0')
                {
                    state = PARSE_STATE_PATH;
                    if (ch != '/')
                        continue;
                }
                break;
            case PARSE_STATE_PATH:
                {
                    auto special = static_cast<bool>(m_uFlags & FLAGS_SPECIAL);
                    bool specialBackSlash = (special && ch == '\\');

                    if (ch == '\0' || ch == '/' || specialBackSlash || (!hasStateOverride && (ch == '?' || ch == '#')))
                    {
                        if (IsDoubleDotSegment(buffer))
                        {
                            ShortenUrlPath();
                            if (ch != '/' && !specialBackSlash)
                            {
                                m_uFlags |= FLAGS_HAS_PATH;
                                m_stPath.emplace_back("");
                            }
                        }
                        else if (IsSingleDotSegment(buffer) && ch != '/' && !specialBackSlash)
                        {
                            m_uFlags |= FLAGS_HAS_PATH;
                            m_stPath.emplace_back("");
                        }
                        else if (!IsSingleDotSegment(buffer))
                        {
                            if (m_stScheme == "file:" && m_stPath.empty() && buffer.size() == 2 &&
                                IsWindowsDriveLetter(buffer[0], buffer[1]))
                            {
                                if ((m_uFlags & FLAGS_HAS_HOST) && m_stHost)
                                {
                                    m_stHost.Reset();
                                    m_uFlags |= FLAGS_HAS_HOST;
                                }
                                buffer[1] = ':';
                            }
                            m_uFlags |= FLAGS_HAS_PATH;
                            m_stPath.emplace_back(std::move(buffer));
                        }
                        buffer.clear();
                        if (m_stScheme == "file:" && (ch == '\0' || ch == '?' || ch == '#'))
                        {
                            while (m_stPath.size() > 1 && m_stPath[0].length() == 0)
                                m_stPath.erase(m_stPath.begin());
                        }
                        if (ch == '?')
                        {
                            m_uFlags |= FLAGS_HAS_QUERY;
                            state = PARSE_STATE_QUERY;
                        }
                        else if (ch == '#')
                            state = PARSE_STATE_FRAGMENT;
                    }
                    else
                        AppendOrEscape(buffer, ch, kPathEncodeSet);
                }
                break;
            case PARSE_STATE_CANNOT_BE_BASE:
                switch (ch)
                {
                    case '?':
                        state = PARSE_STATE_QUERY;
                        break;
                    case '#':
                        state = PARSE_STATE_FRAGMENT;
                        break;
                    default:
                        if (m_stPath.empty())
                            m_stPath.emplace_back("");
                        if (!m_stPath.empty() && ch != '\0')
                            AppendOrEscape(m_stPath[0], ch, kC0ControlEncodeSet);
                        break;
                }
                break;
            case PARSE_STATE_QUERY:
                if (ch == '\0' || (!hasStateOverride && ch == '#'))
                {
                    m_uFlags |= FLAGS_HAS_QUERY;
                    m_stQuery = std::move(buffer);
                    buffer.clear();
                    if (ch == '#')
                        state = PARSE_STATE_FRAGMENT;
                }
                else
                    AppendOrEscape(buffer, ch, kQueryEncodeSet);
                break;
            case PARSE_STATE_FRAGMENT:
                switch (ch)
                {
                    case '\0':
                        m_uFlags |= FLAGS_HAS_FRAGMENT;
                        m_stFragment = std::move(buffer);
                        buffer.clear();
                        break;
                    default:
                        AppendOrEscape(buffer, ch, kFragmentEncodeSet);
                        break;
                }
                break;
            default:
                assert(false);
                return;
        }

        ++p;
    }
}
