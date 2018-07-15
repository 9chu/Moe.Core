/**
 * @file
 * @author chu
 * @date 2018/7/11
 */
#include <gtest/gtest.h>

#include <Moe.Core/Url.hpp>
#include "Data/UrlTestData.hpp"

using namespace std;
using namespace moe;

TEST(Url, Parse)
{
    const auto& data = Testing::GetUrlDataTestRecords();
    for (size_t i = 0; i < data.GetSize(); ++i)
    {
        const auto& record = data[i];

        Url url;
        try
        {
            Url base(record.Base);
            url.Parse(record.Input, &base);
        }
        catch (...)
        {
            url.Reset();
        }

        EXPECT_EQ(record.Protocol, url.GetScheme());
        EXPECT_EQ(record.Username, url.GetUsername());
        EXPECT_EQ(record.Password, url.GetPassword());
        EXPECT_EQ(record.Hostname, url.GetHost().ToString());
        EXPECT_EQ(record.Port, url.GetPortStandard());
        EXPECT_EQ(record.Path, url.GetPathStandard());
        EXPECT_EQ(record.Query, url.GetQueryStandard());
        EXPECT_EQ(record.Fragment, url.GetFragmentStandard());
    }
}
