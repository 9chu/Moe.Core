/**
 * @file
 * @author chu
 * @date 2018/6/7
 */
#pragma once
#include <Moe.Core/Utils.hpp>
#include <Moe.Core/ArrayView.hpp>

#include <cstdint>

namespace Testing
{
    struct UnicodeNormalizeTestRecord
    {
        char32_t Source[8];
        char32_t Nfc[8];
        char32_t Nfd[8];
        char32_t Nfkc[18];
        char32_t Nfkd[18];
    };

    const moe::ArrayView<UnicodeNormalizeTestRecord> GetUnicodeNormalizeTestRecords();
}
