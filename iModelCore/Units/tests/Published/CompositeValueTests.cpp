#include "../FormattingTestsPch.h"
#include "../TestFixture/FormattingTestFixture.h"

USING_BENTLEY_FORMATTING

BEGIN_BENTLEY_FORMATTEST_NAMESPACE

struct CompositeValueSpecTest : FormattingTestFixture {};

struct CompositeValueTest : FormattingTestFixture {};

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(CompositeValueSpecTest, DefaultConstructor)
    {
    CompositeValueSpecCP cvs = new CompositeValueSpec();
    EXPECT_NE(nullptr, cvs);
    EXPECT_FALSE(cvs->IsProblem());
    EXPECT_TRUE(cvs->NoProblem());

    EXPECT_EQ(CompositeSpecType::Undefined, cvs->GetType());

    EXPECT_EQ(0, cvs->GetUnitCount());

    EXPECT_EQ(nullptr, cvs->GetMajorUnit());
    EXPECT_STREQ("", cvs->GetMajorLabel().c_str());
    
    EXPECT_EQ(nullptr, cvs->GetMiddleUnit());
    EXPECT_STREQ("", cvs->GetMiddleLabel().c_str());
    
    EXPECT_EQ(nullptr, cvs->GetMinorUnit());
    EXPECT_STREQ("", cvs->GetMinorLabel().c_str());
    
    EXPECT_EQ(nullptr, cvs->GetSubUnit());
    EXPECT_STREQ("", cvs->GetSubLabel().c_str());

    EXPECT_EQ(0, cvs->GetMajorToMiddleRatio());
    EXPECT_EQ(0, cvs->GetMiddleToMinorRatio());
    EXPECT_EQ(0, cvs->GetMinorToSubRatio());

    EXPECT_STREQ("", cvs->GetSpacer().c_str());
    EXPECT_TRUE(cvs->IsIncludeZero());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeValueSpecTest, Constructors)
    {
    BEU::UnitCP mile = BEU::UnitRegistry::Get().LookupUnit("MILE");
    BEU::UnitCP yrd = BEU::UnitRegistry::Get().LookupUnit("YRD");
    BEU::UnitCP ft = BEU::UnitRegistry::Get().LookupUnit("FT");
    BEU::UnitCP inch = BEU::UnitRegistry::Get().LookupUnit("IN");

    // Single Unit
    CompositeValueSpec cvs1unit(mile);
    ASSERT_EQ(1, cvs1unit.GetUnitCount());
    ASSERT_EQ(CompositeSpecType::Single, cvs1unit.GetType());
    ASSERT_FALSE(cvs1unit.IsProblem());
    EXPECT_STREQ("mi", cvs1unit.GetMajorUnit()->GetLabel().c_str());
    EXPECT_EQ(nullptr, cvs1unit.GetMiddleUnit());
    EXPECT_EQ(nullptr, cvs1unit.GetMinorUnit());
    EXPECT_EQ(nullptr, cvs1unit.GetSubUnit());

    // Two Units
    CompositeValueSpec cvs2unit(mile, yrd);
    ASSERT_EQ(2, cvs2unit.GetUnitCount());
    ASSERT_EQ(CompositeSpecType::Double, cvs2unit.GetType());
    ASSERT_FALSE(cvs2unit.IsProblem());
    EXPECT_STREQ("mi", cvs2unit.GetMajorUnit()->GetLabel().c_str());
    EXPECT_STREQ("yd", cvs2unit.GetMiddleUnit()->GetLabel().c_str());
    EXPECT_EQ(nullptr, cvs2unit.GetMinorUnit());
    EXPECT_EQ(nullptr, cvs2unit.GetSubUnit());
    EXPECT_EQ(1760, cvs2unit.GetMajorToMiddleRatio());

    // Three Units
    CompositeValueSpec cvs3unit(mile, yrd, ft);
    ASSERT_EQ(3, cvs3unit.GetUnitCount());
    ASSERT_EQ(CompositeSpecType::Triple, cvs3unit.GetType());
    ASSERT_FALSE(cvs3unit.IsProblem());
    EXPECT_STREQ("mi", cvs3unit.GetMajorUnit()->GetLabel().c_str());
    EXPECT_STREQ("yd", cvs3unit.GetMiddleUnit()->GetLabel().c_str());
    EXPECT_STREQ("ft", cvs3unit.GetMinorUnit()->GetLabel().c_str());
    EXPECT_EQ(nullptr, cvs3unit.GetSubUnit());
    EXPECT_EQ(1760, cvs3unit.GetMajorToMiddleRatio());
    EXPECT_EQ(3, cvs3unit.GetMiddleToMinorRatio());

    // Four Units
    CompositeValueSpec cvs4unit(mile, yrd, ft, inch);
    ASSERT_EQ(4, cvs4unit.GetUnitCount());
    ASSERT_EQ(CompositeSpecType::Quatro, cvs4unit.GetType());
    ASSERT_FALSE(cvs4unit.IsProblem());
    EXPECT_STREQ("mi", cvs4unit.GetMajorUnit()->GetLabel().c_str());
    EXPECT_STREQ("yd", cvs4unit.GetMiddleUnit()->GetLabel().c_str());
    EXPECT_STREQ("ft", cvs4unit.GetMinorUnit()->GetLabel().c_str());
    EXPECT_STREQ("in", cvs4unit.GetSubUnit()->GetLabel().c_str());
    EXPECT_EQ(1760, cvs4unit.GetMajorToMiddleRatio());
    EXPECT_EQ(3, cvs4unit.GetMiddleToMinorRatio());
    EXPECT_EQ(12, cvs4unit.GetMinorToSubRatio());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(CompositeValueSpecTest, LoadFromJson)
    {
    Utf8CP jsonString = R"json(
        "CompositeFormat": {
            "MajorUnit": {
                "unitLabel": "hour(s)",
                "unitName": "HR",
            },
            "MiddleUnit": {
                "unitLabel": "min",
                "unitName": "MIN",
            },
            "MinorUnit": {
                "unitLabel": "sec",
                "unitName": "S",
            }
        }
    )json";

    Json::Value jval(Json::objectValue);
    Json::Reader::Parse(jsonString, jval);

    CompositeValueSpec cvs;
    cvs.LoadJsonData(jval, &BEU::UnitRegistry::Get());

    EXPECT_TRUE(cvs.NoProblem());
    EXPECT_FALSE(cvs.IsProblem());

    EXPECT_EQ(3, cvs.GetUnitCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeValueTest, Constructors)
    {
    CompositeValue cv;
    ASSERT_FALSE(cv.IsProblem());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeValueTest, DataMemberSettersAndGetters)
    {
    CompositeValue cv;

    cv.SetMajor(1.0);
    cv.SetMiddle(2.0);
    cv.SetMinor(3.0);
    cv.SetSub(4.0);
    cv.SetInput(5.0);
    EXPECT_EQ(1.0, cv.GetMajor());
    EXPECT_EQ(2.0, cv.GetMiddle());
    EXPECT_EQ(3.0, cv.GetMinor());
    EXPECT_EQ(4.0, cv.GetSub());
    EXPECT_EQ(5.0, cv.GetInput());

    cv.SetPositive();
    EXPECT_STREQ("", cv.GetSignPrefix().c_str());
    EXPECT_STREQ("", cv.GetSignPrefix(true).c_str());
    EXPECT_STREQ("", cv.GetSignSuffix().c_str());
    EXPECT_STREQ("", cv.GetSignSuffix(true).c_str());

    cv.SetNegative();
    EXPECT_STREQ("-", cv.GetSignPrefix().c_str());
    EXPECT_STREQ("(", cv.GetSignPrefix(true).c_str());
    EXPECT_STREQ("", cv.GetSignSuffix().c_str());
    EXPECT_STREQ(")", cv.GetSignSuffix(true).c_str());
    }

END_BENTLEY_FORMATTEST_NAMESPACE