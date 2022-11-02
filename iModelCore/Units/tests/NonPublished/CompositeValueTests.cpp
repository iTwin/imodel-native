/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../FormattingTestsPch.h"
#include "../TestFixture/FormattingTestFixture.h"

USING_BENTLEY_FORMATTING

BEGIN_BENTLEY_FORMATTEST_NAMESPACE

static BEU::UnitCP s_mile = nullptr;
static BEU::UnitCP s_yrd = nullptr;
static BEU::UnitCP s_ft = nullptr;
static BEU::UnitCP s_inch = nullptr;
static BEU::UnitCP s_meter = nullptr;

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
        if (nullptr == s_meter)
            s_meter = s_unitsContext->LookupUnit("M");
        }
    };
struct CompositeValueSpecJsonTest : CompositeValueSpecTest {};
struct CompositeValueTest : FormattingTestFixture {};
struct FormatCompositeStringTest : CompositeValueSpecTest {};

//===================================================
// CompositeValueSpec
//===================================================

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(CompositeValueSpecTest, DefaultConstructor)
    {
    CompositeValueSpec cvs;
    EXPECT_TRUE(cvs.IsProblem());

    EXPECT_EQ(0, cvs.GetUnitCount());

    EXPECT_EQ(nullptr, cvs.GetMajorUnit());
    EXPECT_STREQ("", cvs.GetMajorLabel().c_str());

    EXPECT_EQ(nullptr, cvs.GetMiddleUnit());
    EXPECT_STREQ("", cvs.GetMiddleLabel().c_str());

    EXPECT_EQ(nullptr, cvs.GetMinorUnit());
    EXPECT_STREQ("", cvs.GetMinorLabel().c_str());

    EXPECT_EQ(nullptr, cvs.GetSubUnit());
    EXPECT_STREQ("", cvs.GetSubLabel().c_str());

    EXPECT_DOUBLE_EQ(0.0, cvs.GetMajorToMiddleRatio());
    EXPECT_DOUBLE_EQ(0.0, cvs.GetMiddleToMinorRatio());
    EXPECT_DOUBLE_EQ(0.0, cvs.GetMinorToSubRatio());

    EXPECT_STREQ(" ", cvs.GetSpacer().c_str());
    EXPECT_TRUE(cvs.IsIncludeZero());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeValueSpecTest, Constructors)
    {
    // Single Unit
    CompositeValueSpec cvs1unit(*s_mile);
    ASSERT_EQ(1, cvs1unit.GetUnitCount());
    ASSERT_FALSE(cvs1unit.IsProblem());
    EXPECT_STREQ("MILE", cvs1unit.GetMajorUnit()->GetDisplayLabel().c_str());
    EXPECT_EQ(nullptr, cvs1unit.GetMiddleUnit());
    EXPECT_EQ(nullptr, cvs1unit.GetMinorUnit());
    EXPECT_EQ(nullptr, cvs1unit.GetSubUnit());

    // Two Units
    CompositeValueSpec cvs2unit(*s_mile, *s_yrd);
    ASSERT_EQ(2, cvs2unit.GetUnitCount());
    ASSERT_FALSE(cvs2unit.IsProblem());
    EXPECT_STREQ("MILE", cvs2unit.GetMajorUnit()->GetDisplayLabel().c_str());
    EXPECT_STREQ("YRD", cvs2unit.GetMiddleUnit()->GetDisplayLabel().c_str());
    EXPECT_EQ(nullptr, cvs2unit.GetMinorUnit());
    EXPECT_EQ(nullptr, cvs2unit.GetSubUnit());
    EXPECT_DOUBLE_EQ(1760.0, cvs2unit.GetMajorToMiddleRatio());

    // Three Units
    CompositeValueSpec cvs3unit(*s_mile, *s_yrd, *s_ft);
    ASSERT_EQ(3, cvs3unit.GetUnitCount());
    ASSERT_FALSE(cvs3unit.IsProblem());
    EXPECT_STREQ("MILE", cvs3unit.GetMajorUnit()->GetDisplayLabel().c_str());
    EXPECT_STREQ("YRD", cvs3unit.GetMiddleUnit()->GetDisplayLabel().c_str());
    EXPECT_STREQ("FT", cvs3unit.GetMinorUnit()->GetDisplayLabel().c_str());
    EXPECT_EQ(nullptr, cvs3unit.GetSubUnit());
    EXPECT_DOUBLE_EQ(1760.0, cvs3unit.GetMajorToMiddleRatio());
    EXPECT_DOUBLE_EQ(3.0, cvs3unit.GetMiddleToMinorRatio());

    // Four Units
    CompositeValueSpec cvs4unit(*s_mile, *s_yrd, *s_ft, *s_inch);
    ASSERT_EQ(4, cvs4unit.GetUnitCount());
    ASSERT_FALSE(cvs4unit.IsProblem());
    EXPECT_STREQ("MILE", cvs4unit.GetMajorUnit()->GetDisplayLabel().c_str());
    EXPECT_STREQ("YRD", cvs4unit.GetMiddleUnit()->GetDisplayLabel().c_str());
    EXPECT_STREQ("FT", cvs4unit.GetMinorUnit()->GetDisplayLabel().c_str());
    EXPECT_STREQ("IN", cvs4unit.GetSubUnit()->GetDisplayLabel().c_str());
    EXPECT_DOUBLE_EQ(1760.0, cvs4unit.GetMajorToMiddleRatio());
    EXPECT_DOUBLE_EQ(3.0, cvs4unit.GetMiddleToMinorRatio());
    EXPECT_DOUBLE_EQ(12.0, cvs4unit.GetMinorToSubRatio());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeValueSpecTest, NonIntegerRatios)
    {
    CompositeValueSpec cvs1unit(*s_mile, *s_meter);
    ASSERT_EQ(2, cvs1unit.GetUnitCount());
    ASSERT_FALSE(cvs1unit.IsProblem());
    EXPECT_STREQ("MILE", cvs1unit.GetMajorUnit()->GetDisplayLabel().c_str());
    EXPECT_STREQ("M", cvs1unit.GetMiddleUnit()->GetDisplayLabel().c_str());
    EXPECT_EQ(nullptr, cvs1unit.GetMinorUnit());
    EXPECT_EQ(nullptr, cvs1unit.GetSubUnit());
    EXPECT_DOUBLE_EQ(1609.344, cvs1unit.GetMajorToMiddleRatio());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(CompositeValueSpecTest, Single_SetUnitLabels)
    {
    { // Set labels on none of the units
    CompositeValueSpec cvs1Unit(*s_mile);
    cvs1Unit.SetUnitLabels(nullptr, nullptr, nullptr, nullptr);
    EXPECT_STREQ("MILE", cvs1Unit.GetMajorLabel().c_str()) << "Expected the display label that is set on the unit: " << s_mile->GetDisplayLabel().c_str();
    EXPECT_STREQ("", cvs1Unit.GetMiddleLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetMinorLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    { // Set labels on the only unit
    CompositeValueSpec cvs1Unit(*s_mile);
    cvs1Unit.SetUnitLabels("sillyLabel");
    EXPECT_STREQ("sillyLabel", cvs1Unit.GetMajorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("", cvs1Unit.GetMiddleLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetMinorLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(CompositeValueSpecTest, Double_SetUnitLabels)
    {
    { // Set labels on none of the units
    CompositeValueSpec cvs1Unit(*s_mile, *s_yrd);
    cvs1Unit.SetUnitLabels(nullptr, nullptr, nullptr, nullptr);
    EXPECT_STREQ("MILE", cvs1Unit.GetMajorLabel().c_str()) << "Expected the display label that is set on the unit: " << s_mile->GetDisplayLabel().c_str();
    EXPECT_STREQ("YRD", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the display label that is set on the unit: " << s_yrd->GetDisplayLabel().c_str();
    EXPECT_STREQ("", cvs1Unit.GetMinorLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    { // Set labels on both set Units
    CompositeValueSpec cvs1Unit(*s_mile, *s_yrd);
    cvs1Unit.SetUnitLabels("sillyLabel1", "sillyLabel2");
    EXPECT_STREQ("sillyLabel1", cvs1Unit.GetMajorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel2", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("", cvs1Unit.GetMinorLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    { // Attempt setting a label without a Unit being set
    CompositeValueSpec cvs1Unit(*s_mile, *s_yrd);
    cvs1Unit.SetUnitLabels("sillyLabel1", "sillyLabel2", "sillyLabel3", "sillyLabel4");
    EXPECT_STREQ("sillyLabel1", cvs1Unit.GetMajorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel2", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("", cvs1Unit.GetMinorLabel().c_str());
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(CompositeValueSpecTest, Triple_SetUnitLabels)
    {
    { // Set labels on none of the units
    CompositeValueSpec cvs1Unit(*s_mile, *s_yrd, *s_ft);
    cvs1Unit.SetUnitLabels(nullptr, nullptr, nullptr, nullptr);
    EXPECT_STREQ("MILE", cvs1Unit.GetMajorLabel().c_str()) << "Expected the display label that is set on the unit: " << s_mile->GetDisplayLabel().c_str();
    EXPECT_STREQ("YRD", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the display label that is set on the unit: " << s_yrd->GetDisplayLabel().c_str();
    EXPECT_STREQ("FT", cvs1Unit.GetMinorLabel().c_str()) << "Expected the display label that is set on the unit: " << s_ft->GetDisplayLabel().c_str();
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    { // Set labels on all Units
    CompositeValueSpec cvs1Unit(*s_mile, *s_yrd, *s_ft);
    cvs1Unit.SetUnitLabels("sillyLabel1", "sillyLabel2", "sillyLabel3");
    EXPECT_STREQ("sillyLabel1", cvs1Unit.GetMajorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel2", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel3", cvs1Unit.GetMinorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    { // Attempt setting a label without a Unit.
    CompositeValueSpec cvs1Unit(*s_mile, *s_yrd, *s_ft);
    cvs1Unit.SetUnitLabels("sillyLabel1", "sillyLabel2", "sillyLabel3", "sillyLabel4");
    EXPECT_STREQ("sillyLabel1", cvs1Unit.GetMajorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel2", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel3", cvs1Unit.GetMinorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("", cvs1Unit.GetSubLabel().c_str());
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(CompositeValueSpecTest, Quatro_SetUnitLabels)
    {
    {
    CompositeValueSpec cvs1Unit(*s_mile, *s_yrd, *s_ft, *s_inch);
    cvs1Unit.SetUnitLabels(nullptr, nullptr, nullptr, nullptr);
    EXPECT_STREQ("MILE", cvs1Unit.GetMajorLabel().c_str());
    EXPECT_STREQ("YRD", cvs1Unit.GetMiddleLabel().c_str());
    EXPECT_STREQ("FT", cvs1Unit.GetMinorLabel().c_str());
    EXPECT_STREQ("IN", cvs1Unit.GetSubLabel().c_str());
    }
    { // Attempt setting a label without a Unit.
    CompositeValueSpec cvs1Unit(*s_mile, *s_yrd, *s_ft, *s_inch);
    cvs1Unit.SetUnitLabels("sillyLabel1", "sillyLabel2", "sillyLabel3", "sillyLabel4");
    EXPECT_STREQ("sillyLabel1", cvs1Unit.GetMajorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel2", cvs1Unit.GetMiddleLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel3", cvs1Unit.GetMinorLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    EXPECT_STREQ("sillyLabel4", cvs1Unit.GetSubLabel().c_str()) << "Expected the label set in the CompositeValueSpec.";
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(CompositeValueSpecTest, ConstantAsUnit)
    {
    auto pi = s_unitsContext->LookupConstant("PI");
    auto expectedProblemCode = FormatProblemCode::CVS_ConstantAsUnit;
    FormatProblemDetail detail = FormatProblemDetail(expectedProblemCode);
    auto expectedProblemDescription = detail.GetProblemDescription();

    {
    CompositeValueSpec cvs1Unit(*pi);
    EXPECT_TRUE(cvs1Unit.IsProblem());
    EXPECT_STRCASEEQ(expectedProblemDescription.c_str(), cvs1Unit.GetProblemDescription().c_str());
    }

    {
    CompositeValueSpec cvs1Unit(*s_mile, *s_yrd, *pi, *s_inch);
    EXPECT_TRUE(cvs1Unit.IsProblem());
    EXPECT_STRCASEEQ(expectedProblemDescription.c_str(), cvs1Unit.GetProblemDescription().c_str());
    }

    {
    bvector<Units::UnitCP> units = {s_mile, s_yrd, s_ft, pi};
    CompositeValueSpec spec;
    ASSERT_FALSE(CompositeValueSpec::CreateCompositeSpec(spec, units));
    EXPECT_TRUE(spec.IsProblem());
    EXPECT_STRCASEEQ(expectedProblemDescription.c_str(), spec.GetProblemDescription().c_str());
    }

    {
    bvector<Units::UnitCP> units = {pi};
    CompositeValueSpec spec;
    ASSERT_FALSE(CompositeValueSpec::CreateCompositeSpec(spec, units));
    EXPECT_TRUE(spec.IsProblem());
    EXPECT_STRCASEEQ(expectedProblemDescription.c_str(), spec.GetProblemDescription().c_str());
    }
    }

//===================================================
// FormatCompositeStringTest
//===================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatCompositeStringTest, CompositeValueUsesThousandSeparatorForLastUnit)
    {
    NumericFormatSpec numericFormatSpec;
    numericFormatSpec.SetThousandSeparator('\'');
    numericFormatSpec.SetUse1000Separator(true);
    numericFormatSpec.SetKeepSingleZero(true);

    CompositeValueSpec compositeValueSpec(*s_mile, *s_inch);
    ASSERT_EQ(2, compositeValueSpec.GetUnitCount());
    Format format(numericFormatSpec, compositeValueSpec);

    // 1500.5 miles == 1,500 miles and 31,680 inches
    BEU::Quantity quantity(1500.5, *compositeValueSpec.GetMajorUnit());
    EXPECT_STREQ("1500 31'680.0", format.FormatQuantity(quantity).c_str());
    }

//===================================================
// CompositeValueSpecJsonTest
//===================================================

void validateSpecJson(CompositeValueSpecCP spec, Utf8StringCR expectedJson, Units::IUnitsContextCP unitsContext, bool verbose = false)
    {
    Json::Value root;
    Json::Reader::Parse(expectedJson, root);
    Json::Value comp;
    spec->ToJson(BeJsValue(comp), verbose);
    EXPECT_TRUE(root.ToString() == comp.ToString()) << FormattingTestUtils::JsonComparisonString(comp, root);

    //FromJson
    CompositeValueSpec compSpec;
    CompositeValueSpec::FromJson(compSpec, root, unitsContext);
    Json::Value roundTrip;
    compSpec.ToJson(BeJsValue(roundTrip), verbose);
    EXPECT_TRUE(roundTrip.ToString() == root.ToString()) << FormattingTestUtils::JsonComparisonString(roundTrip, root);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeValueSpecJsonTest, TestDefaultAndEmptySpacerRoundTrips)
    {
    CompositeValueSpec spec(*s_mile); // Default Spacer (is space)
    Json::Value json;
    spec.ToJson(BeJsValue(json));

    auto expectedJson = R"json({
                                "includeZero": true,
                                "units": [
                                        {
                                        "name": "MILE"
                                        }
                                    ]
                                })json";
    validateSpecJson(&spec, expectedJson, s_unitsContext);

    spec.SetSpacer(""); // Set so no spacer is used
    expectedJson = R"json({
                        "includeZero": true,
                        "spacer": "",
                        "units": [
                                {
                                "name": "MILE"
                                }
                            ]
                        })json";
    validateSpecJson(&spec, expectedJson, s_unitsContext);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeValueSpecJsonTest, JsonTest)
    {
    CompositeValueSpec spec(*s_mile, *s_yrd, *s_ft, *s_inch);
    spec.SetMajorLabel("apple");
    spec.SetMiddleLabel("banana");
    spec.SetMinorLabel("cactus pear");
    spec.SetSubLabel("dragonfruit");
    spec.SetSpacer("-");
    Json::Value json;
    spec.ToJson(BeJsValue(json));

    auto expectedJson = R"json({
                                "includeZero": true,
                                "spacer": "-",
                                "units": [
                                        {
                                        "name": "MILE",
                                        "label": "apple"
                                        },
                                        {
                                        "name": "YRD",
                                        "label": "banana"
                                        },
                                        {
                                        "name": "FT",
                                        "label": "cactus pear"
                                        },
                                        {
                                        "name": "IN",
                                        "label": "dragonfruit"
                                        }
                                    ]
                                })json";
    validateSpecJson(&spec, expectedJson, s_unitsContext);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeValueSpecJsonTest, JsonVerboseTest)
    {
    CompositeValueSpec spec(*s_mile, *s_yrd, *s_ft, *s_inch);
    spec.SetSpacer("-");
    Json::Value json;
    spec.ToJson(BeJsValue(json));

    auto expectedJson = R"json({
                                "includeZero": true,
                                "spacer": "-",
                                "units": [
                                        {
                                        "name": "MILE",
                                        "label": "MILE"
                                        },
                                        {
                                        "name": "YRD",
                                        "label": "YRD"
                                        },
                                        {
                                        "name": "FT",
                                        "label": "FT"
                                        },
                                        {
                                        "name": "IN",
                                        "label": "IN"
                                        }
                                    ]
                                })json";

    validateSpecJson(&spec, expectedJson, s_unitsContext, true);
    }


END_BENTLEY_FORMATTEST_NAMESPACE
