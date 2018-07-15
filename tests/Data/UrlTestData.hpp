/**
 * @file
 * @author chu
 * @date 2018/7/11
 */
#pragma once
#include <Moe.Core/Utils.hpp>
#include <Moe.Core/ArrayView.hpp>

#include <cstdint>

namespace Testing
{
    struct UrlDataTestRecord
    {
        const char* Input;
        const char* Base;
        const char* Protocol;
        const char* Username;
        const char* Password;
        const char* Hostname;
        const char* Port;
        const char* Path;
        const char* Query;
        const char* Fragment;
    };

    const moe::ArrayView<UrlDataTestRecord> GetUrlDataTestRecords();
}
