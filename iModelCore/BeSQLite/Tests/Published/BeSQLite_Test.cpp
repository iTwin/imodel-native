/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeSQLite_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BeSQLitePublishedTests.h"

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   06/16
//=======================================================================================
struct BeIdSetTests : public ::testing::Test
{
public:
    void ExpectRoundTrip(BeIdSet const& ids, Utf8CP expected);
    BeIdSet MakeIdSet(std::initializer_list<int> values)
        {
        BeIdSet ids;
        for (auto value : values)
            ids.insert(BeInt64Id(static_cast<int64_t>(value)));

        return ids;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BeIdSetTests::ExpectRoundTrip(BeIdSet const& ids, Utf8CP expected)
    {
    Utf8String actual = ids.ToString();
    EXPECT_TRUE(actual.Equals(expected)) << "Expected: " << expected << " Actual: " << actual.c_str();
    BeIdSet roundtripped;
    roundtripped.FromString(actual);
    EXPECT_TRUE(roundtripped == ids) << " Expected: " << ids.ToString(BeIdSet::StringFormat::Readable).c_str() << " Actual: " << roundtripped.ToString(BeIdSet::StringFormat::Readable).c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeIdSetTests, ToString)
    {
    ExpectRoundTrip(MakeIdSet({2}), "+2");
    ExpectRoundTrip(MakeIdSet({1,5}), "+1+4");
    ExpectRoundTrip(MakeIdSet({3,7,8,10}), "+3+4+1+2");
    ExpectRoundTrip(MakeIdSet({0xFF, 0x150}), "+FF+51");
    
    ExpectRoundTrip(MakeIdSet({1,2,3,4,5}), "+1*5");
    ExpectRoundTrip(MakeIdSet({2,4,6,8}), "+2*4");
    ExpectRoundTrip(MakeIdSet({1,2,3,4,8,12,16}), "+1*4+4*3");
    ExpectRoundTrip(MakeIdSet({1,2,3,4,8,12,16,17}), "+1*4+4*3+1");

    ExpectRoundTrip(MakeIdSet({100,200,300,400,500,600,700,800,900,1000,1100,1200,1300,1400,1500,1600,1700,1800,1900,2000,2100,2200,2300}), "+64*17");
    ExpectRoundTrip(MakeIdSet({1,10001,20001,30001,40001,50001,60001,70001,80001,90001,100001,110001,120001,130001,140001,150001,160001,170001,180001,190001,200001,210001,220001,230001, 230002}), "+1+2710*17+1");
    }

