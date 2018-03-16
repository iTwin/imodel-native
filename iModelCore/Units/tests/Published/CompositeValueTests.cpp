#include "../FormattingTestsPch.h"
#include "../TestFixture/FormattingTestFixture.h"

USING_BENTLEY_FORMATTING

BEGIN_BENTLEY_FORMATTEST_NAMESPACE

static BEU::UnitCP s_mile = nullptr;
static BEU::UnitCP s_yrd = nullptr;
static BEU::UnitCP s_ft = nullptr;
static BEU::UnitCP s_inch = nullptr;

struct CompositeValueSpecTest : FormattingTestFixture
    {
    void SetUp() override
        {
        FormattingTestFixture::SetUp();

        if (nullptr == s_mile)
            s_mile = s_unitsContext->LookupUnit("MILE");
        if (nullptr == s_yrd)
            s_yrd = s_unitsContext->LookupUnit("YRD");
        if (nullptr == s_ft)
            s_ft = s_unitsContext->LookupUnit("FT");
        if (nullptr == s_inch)
            s_inch = s_unitsContext->LookupUnit("IN");
        }

    void VerifyCompositeJson(Utf8CP jsonString, Utf8CP errorMsg = nullptr);
    };
struct CompositeValueTest : FormattingTestFixture {};

//===================================================
// CompositeValueSpec
//===================================================

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
    // Single Unit
    CompositeValueSpec cvs1unit(s_mile);
    ASSERT_EQ(1, cvs1unit.GetUnitCount());
    ASSERT_EQ(CompositeSpecType::Single, cvs1unit.GetType());
    ASSERT_FALSE(cvs1unit.IsProblem());
    EXPECT_STREQ("mi", cvs1unit.GetMajorUnit()->GetLabel().c_str());
    EXPECT_EQ(nullptr, cvs1unit.GetMiddleUnit());
    EXPECT_EQ(nullptr, cvs1unit.GetMinorUnit());
    EXPECT_EQ(nullptr, cvs1unit.GetSubUnit());

    // Two Units
    CompositeValueSpec cvs2unit(s_mile, s_yrd);
    ASSERT_EQ(2, cvs2unit.GetUnitCount());
    ASSERT_EQ(CompositeSpecType::Double, cvs2unit.GetType());
    ASSERT_FALSE(cvs2unit.IsProblem());
    EXPECT_STREQ("mi", cvs2unit.GetMajorUnit()->GetLabel().c_str());
    EXPECT_STREQ("yd", cvs2unit.GetMiddleUnit()->GetLabel().c_str());
    EXPECT_EQ(nullptr, cvs2unit.GetMinorUnit());
    EXPECT_EQ(nullptr, cvs2unit.GetSubUnit());
    EXPECT_EQ(1760, cvs2unit.GetMajorToMiddleRatio());

    // Three Units
    CompositeValueSpec cvs3unit(s_mile, s_yrd, s_ft);
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
    CompositeValueSpec cvs4unit(s_mile, s_yrd, s_ft, s_inch);
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
TEST_F(CompositeValueSpecTest, None_SetUnitLabels)
    {
    CompositeValueSpec cvs1Unit;
    cvs1Unit.SetUnitLabels(nullptr, nullptr, nullptr, nullptr);
    EXPECT_STREQ("", cvs1Unit.GetMajorLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetMiddleLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetMinorLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(CompositeValueSpecTest, Single_SetUnitLabels)
    {
    { // Set labels on none of the units
    CompositeValueSpec cvs1Unit(s_mile);
    cvs1Unit.SetUnitLabels(nullptr, nullptr, nullptr, nullptr);
    EXPECT_STREQ("MILE", cvs1Unit.GetMajorLabel().c_str()) << "Expected the display label that is set on the unit: " << s_mile->GetLabel().c_str();
    EXPECT_STREQ("", cvs1Unit.GetMiddleLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetMinorLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    { // Set labels on the only unit
    CompositeValueSpec cvs1Unit(s_mile);
    cvs1Unit.SetUnitLabels("sillyLabel");
    EXPECT_STREQ("sillyLabel", cvs1Unit.GetMajorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("", cvs1Unit.GetMiddleLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetMinorLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(CompositeValueSpecTest, Double_SetUnitLabels)
    {
    { // Set labels on none of the units
    CompositeValueSpec cvs1Unit(s_mile, s_yrd);
    cvs1Unit.SetUnitLabels(nullptr, nullptr, nullptr, nullptr);
    EXPECT_STREQ("MILE", cvs1Unit.GetMajorLabel().c_str()) << "Expected the display label that is set on the unit: " << s_mile->GetLabel().c_str();
    EXPECT_STREQ("YRD", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the display label that is set on the unit: " << s_yrd->GetLabel().c_str();
    EXPECT_STREQ("", cvs1Unit.GetMinorLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    { // Set labels on both set Units
    CompositeValueSpec cvs1Unit(s_mile, s_yrd);
    cvs1Unit.SetUnitLabels("sillyLabel1", "sillyLabel2");
    EXPECT_STREQ("sillyLabel1", cvs1Unit.GetMajorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel2", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("", cvs1Unit.GetMinorLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    { // Attempt setting a label without a Unit being set
    CompositeValueSpec cvs1Unit(s_mile, s_yrd);
    cvs1Unit.SetUnitLabels("sillyLabel1", "sillyLabel2", "sillyLabel3", "sillyLabel4");
    EXPECT_STREQ("sillyLabel1", cvs1Unit.GetMajorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel2", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("", cvs1Unit.GetMinorLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(CompositeValueSpecTest, Triple_SetUnitLabels)
    {
    { // Set labels on none of the units
    CompositeValueSpec cvs1Unit(s_mile, s_yrd, s_ft);
    cvs1Unit.SetUnitLabels(nullptr, nullptr, nullptr, nullptr);
    EXPECT_STREQ("MILE", cvs1Unit.GetMajorLabel().c_str()) << "Expected the display label that is set on the unit: " << s_mile->GetLabel().c_str();
    EXPECT_STREQ("YRD", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the display label that is set on the unit: " << s_yrd->GetLabel().c_str();
    EXPECT_STREQ("FT", cvs1Unit.GetMinorLabel().c_str()) << "Expected the display label that is set on the unit: " << s_ft->GetLabel().c_str();
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    { // Set labels on all Units
    CompositeValueSpec cvs1Unit(s_mile, s_yrd, s_ft);
    cvs1Unit.SetUnitLabels("sillyLabel1", "sillyLabel2", "sillyLabel3");
    EXPECT_STREQ("sillyLabel1", cvs1Unit.GetMajorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel2", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel3", cvs1Unit.GetMinorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    { // Attempt setting a label without a Unit.
    CompositeValueSpec cvs1Unit(s_mile, s_yrd, s_ft);
    cvs1Unit.SetUnitLabels("sillyLabel1", "sillyLabel2", "sillyLabel3", "sillyLabel4");
    EXPECT_STREQ("sillyLabel1", cvs1Unit.GetMajorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel2", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel3", cvs1Unit.GetMinorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(CompositeValueSpecTest, Quatro_SetUnitLabels)
    {
    {
    CompositeValueSpec cvs1Unit(s_mile, s_yrd, s_ft, s_inch);
    cvs1Unit.SetUnitLabels(nullptr, nullptr, nullptr, nullptr);
    EXPECT_STREQ("MILE", cvs1Unit.GetMajorLabel().c_str());
    EXPECT_STREQ("YRD", cvs1Unit.GetMiddleLabel().c_str());
    EXPECT_STREQ("FT", cvs1Unit.GetMinorLabel().c_str());
    EXPECT_STREQ("IN", cvs1Unit.GetSubLabel().c_str());
    }
    { // Attempt setting a label without a Unit.
    CompositeValueSpec cvs1Unit(s_mile, s_yrd, s_ft, s_inch);
    cvs1Unit.SetUnitLabels("sillyLabel1", "sillyLabel2", "sillyLabel3", "sillyLabel4");
    EXPECT_STREQ("sillyLabel1", cvs1Unit.GetMajorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel2", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel3", cvs1Unit.GetMinorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel4", cvs1Unit.GetSubLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    }
    }

//===================================================
// CompositeValue
//===================================================

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