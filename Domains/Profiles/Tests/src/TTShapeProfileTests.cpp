                                             /*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct TTShapeProfileTestCase : ProfileValidationTestCase<TTShapeProfile>
    {
public:
    typedef TTShapeProfile::CreateParams CreateParams;

    TTShapeProfile::CreateParams InfinityCreateParams (Utf8CP pName = "TT")
        {
        return CreateParams (GetModel(), pName, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY,
            INFINITY, Angle::FromRadians (INFINITY), INFINITY, Angle::FromRadians (INFINITY));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "TT");

    TTShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "TT", 10, 10, 1, 1, 1);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    CreateParams fullParams (GetModel(), "TT", 16, 10, 1, 1, 2, 0.5, 0.5, Angle::FromRadians (PI / 32), 0.5, Angle::FromRadians (PI / 32));
    EXPECT_SUCCESS_Insert (fullParams) << "Profile should succeed to insert with valid full create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "TT");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "TT", 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, Angle::FromRadians (8.0), 9.0, Angle::FromRadians (10.0));

    TTShapeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("TT", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetFlangeThickness());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetWebThickness());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetWebSpacing());
    EXPECT_DOUBLE_EQ (6.0, profilePtr->GetFilletRadius());
    EXPECT_DOUBLE_EQ (7.0, profilePtr->GetFlangeEdgeRadius());
    EXPECT_DOUBLE_EQ (8.0, profilePtr->GetFlangeSlope().Radians());
    EXPECT_DOUBLE_EQ (9.0, profilePtr->GetWebEdgeRadius());
    EXPECT_DOUBLE_EQ (10.0, profilePtr->GetWebSlope().Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams (GetModel(), "", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Angle::FromRadians (0.0), 0.0, Angle::FromRadians (0.0));

    TTShapeProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("TT");
    profilePtr->SetFlangeWidth (1.0);
    profilePtr->SetDepth (2.0);
    profilePtr->SetFlangeThickness (3.0);
    profilePtr->SetWebThickness (4.0);
    profilePtr->SetWebSpacing (5.0);
    profilePtr->SetFilletRadius (6.0);
    profilePtr->SetFlangeEdgeRadius (7.0);
    profilePtr->SetFlangeSlope (Angle::FromRadians (8.0));
    profilePtr->SetWebEdgeRadius (9.0);
    profilePtr->SetWebSlope (Angle::FromRadians (10.0));

    EXPECT_EQ ("TT", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetFlangeThickness());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetWebThickness());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetWebSpacing());
    EXPECT_DOUBLE_EQ (6.0, profilePtr->GetFilletRadius());
    EXPECT_DOUBLE_EQ (7.0, profilePtr->GetFlangeEdgeRadius());
    EXPECT_DOUBLE_EQ (8.0, profilePtr->GetFlangeSlope().Radians());
    EXPECT_DOUBLE_EQ (9.0, profilePtr->GetWebEdgeRadius());
    EXPECT_DOUBLE_EQ (10.0, profilePtr->GetWebSlope().Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, GetInnerFlangeFaceLength_FlangeWidthAndWebThickness_CorrectValue)
    {
    CreateParams params = InfinityCreateParams();
    TTShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetFlangeWidth (0.0);
    profilePtr->SetWebThickness (0.0);
    profilePtr->SetWebSpacing (0.0);
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetFlangeInnerFaceLength());

    profilePtr->SetFlangeWidth (5.0);
    profilePtr->SetWebThickness (1.0);
    profilePtr->SetWebSpacing (1.0);
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeInnerFaceLength());

    profilePtr->SetFlangeWidth (1.0);
    profilePtr->SetWebThickness (1.0);
    profilePtr->SetWebSpacing (1.0);
    EXPECT_DOUBLE_EQ (-1.0, profilePtr->GetFlangeInnerFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, GetOuterWebFaceLength_DepthAndFlangeThickness_CorrectValue)
    {
    CreateParams params = InfinityCreateParams();
    TTShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetDepth (0.0);
    profilePtr->SetFlangeThickness (0.0);
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetWebOuterFaceLength());

    profilePtr->SetDepth (2.0);
    profilePtr->SetFlangeThickness (1.0);
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWebOuterFaceLength());

    profilePtr->SetDepth (1.0);
    profilePtr->SetFlangeThickness (2.0);
    EXPECT_DOUBLE_EQ (-1.0, profilePtr->GetWebOuterFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, GetInnerWebFaceLength_DepthAndFlangeThickness_CorrectValue)
    {
    CreateParams params = InfinityCreateParams();
    TTShapeProfilePtr profilePtr = CreateProfile (params);


    profilePtr->SetDepth (0.0);
    profilePtr->SetFlangeThickness (0.0);
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetWebInnerFaceLength());

    profilePtr->SetDepth (2.0);
    profilePtr->SetFlangeThickness (1.0);
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWebInnerFaceLength());

    profilePtr->SetFlangeWidth (5.0);
    profilePtr->SetWebThickness (1.0);
    profilePtr->SetWebSpacing (1.0);
    profilePtr->SetFlangeSlope (Angle::FromRadians (PI / 4.0));
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeSlopeHeight());

    profilePtr->SetDepth (3.0);
    profilePtr->SetFlangeThickness (1.0);
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWebInnerFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, GetFlangeSlopeHeight_ProfileWithProperties_CorrectValue)
    {
    CreateParams params = InfinityCreateParams();
    TTShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetFlangeWidth (5.0);
    profilePtr->SetWebThickness (1.0);
    profilePtr->SetWebSpacing (1.0);

    profilePtr->SetFlangeSlope (Angle::FromRadians (PI / 4.0));
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeSlopeHeight());

    profilePtr->SetFlangeSlope (Angle::FromRadians (0.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetFlangeSlopeHeight());

    profilePtr->SetFlangeSlope (Angle::FromRadians (PI / 2.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetFlangeSlopeHeight());

    profilePtr->SetFlangeSlope (Angle::FromRadians (PI));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetFlangeSlopeHeight());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, GetWebOuterSlopeHeight_ProfileWithProperties_CorrectValue)
    {
    CreateParams params = InfinityCreateParams();
    TTShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetDepth (2.0);
    profilePtr->SetFlangeThickness (1.0);

    profilePtr->SetWebSlope (Angle::FromRadians (PI / 4.0));
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWebOuterSlopeHeight());

    profilePtr->SetWebSlope (Angle::FromRadians (0.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetWebOuterSlopeHeight());

    profilePtr->SetWebSlope (Angle::FromRadians (PI / 2.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetWebOuterSlopeHeight());

    profilePtr->SetWebSlope (Angle::FromRadians (PI));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetWebOuterSlopeHeight());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, GetWebInnerSlopeHeight_ProfileWithProperties_CorrectValue)
    {
    CreateParams params = InfinityCreateParams();
    TTShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetFlangeWidth (5.0);
    profilePtr->SetWebThickness (1.0);
    profilePtr->SetWebSpacing (1.0);
    profilePtr->SetFlangeSlope (Angle::FromRadians (PI / 4.0));
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeSlopeHeight());

    profilePtr->SetDepth (3.0);
    profilePtr->SetFlangeThickness (1.0);

    profilePtr->SetWebSlope (Angle::FromRadians (PI / 4.0));
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWebInnerSlopeHeight());

    profilePtr->SetWebSlope (Angle::FromRadians (0.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetWebInnerSlopeHeight());

    profilePtr->SetWebSlope (Angle::FromRadians (PI / 2.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetWebInnerSlopeHeight());

    profilePtr->SetWebSlope (Angle::FromRadians (PI));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetWebInnerSlopeHeight());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 1.0, 1.0, 1.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 1.0, 1.0, 1.0);

    params.name = "TT";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_InvalidFlangeWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "TT", INFINITY, 10.0, 1.0, 1.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.flangeWidth, "FlangeWidth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_ValidFlangeWidth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "TT", INFINITY, 10.0, 1.0, 1.0, 1.0);

    params.flangeWidth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Flange width should be of positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_InvalidDepth_FailedInsert)
    {
    CreateParams params (GetModel(), "TT", 10.0, INFINITY, 1.0, 1.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.depth, "Depth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_ValidDepth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "TT", 10.0, INFINITY, 1.0, 1.0, 1.0);

    params.depth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Depth should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_InvalidFlangeThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "TT", 10.0, 10.0, INFINITY, 1.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.flangeThickness, "FlangeThickness", false);

    params.flangeThickness = params.depth;
    EXPECT_FAIL_Insert (params) << "Flange thickness should be less than the depth.";

    params.flangeThickness = params.depth - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "Flange thickness should be less than half of the depth.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_ValidFlangeThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "TT", 10.0, 10.0, INFINITY, 1.0, 1.0);

    params.flangeThickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Flange thickness should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_InvalidWebThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "TT", 10.0, 10.0, 1.0, INFINITY, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.webThickness, "WebThickness", false);

    double const maxThickness = (params.flangeWidth - params.webSpacing) / 2.0;

    params.webThickness = maxThickness;
    EXPECT_FAIL_Insert (params) << "Web thickness should be less than half flange width plus half web spacing.";

    params.webThickness = maxThickness - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "Web thickness should be less than half flange width plus half web spacing.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_ValidWebThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "TT", 10.0, 10.0, 1.0, INFINITY, 1.0);

    params.webThickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Web thickness should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_InvalidWebSpacing_FailedInsert)
    {
    CreateParams params (GetModel(), "TT", 10.0, 10.0, 1.0, 1.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.webSpacing, "WebSpacing", false);

    double const maxSpacing = params.flangeWidth - params.webThickness * 2.0;

    params.webSpacing = maxSpacing;
    EXPECT_FAIL_Insert (params) << "Web spacing should be less than flange width minus two web thicknesses.";

    params.webSpacing = maxSpacing - 0.0000001;
    EXPECT_SUCCESS_Insert (params) << "Web spacing should be less than flange width minus two web thicknesses.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_ValidWebSpacing_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "TT", 10.0, 10.0, 1.0, 1.0, INFINITY);

    params.webSpacing = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Web spacing should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_FilletRadiusAgainstTheFlange_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "TT_ZeroFlangeSlope_ShortFlange", 7.0, 100.0, 1.0, 1.0, 1.0, INFINITY);
    params.flangeSlope = Angle::FromRadians (0.0);

    TTShapeProfilePtr profilePtr = CreateProfile (params);

    params.filletRadius = 1.0;
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeInnerFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange.";

    params.filletRadius = 1.0 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_FilletRadiusAgainstTheWeb_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "TT_ZeroFlangeSlope_ShortWeb", 100.0, 3.0, 1.0, 1.0, 1.0, INFINITY);
    params.flangeSlope = Angle::FromRadians (0.0);

    TTShapeProfilePtr profilePtr = CreateProfile (params);

    params.filletRadius = 1.0;
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWebOuterFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the outer face of the web (when flange slope is zero).";

    params.filletRadius = 1.0 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the outer face of the web (when flange slope is zero).";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_FilletRadiusAgainstTheFlangeWithSlope_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "TT_NonZeroFlangeSlope", 10.0, 20.0, 1.0, 1.0, 1.0, INFINITY);
    params.webSlope = Angle::FromRadians ((PI / 180.0) * 1.0);

    TTShapeProfilePtr profilePtr = CreateProfile (params);

    double const maximumFilletRadiusForFlange = profilePtr->GetFlangeInnerFaceLength() / 2.0 - profilePtr->GetWebOuterSlopeHeight();
    EXPECT_GE (maximumFilletRadiusForFlange, 0.0) << "Web slope height cannot be greater than half of the inner flange face length";

    params.filletRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange minus web outer slope height.";

    params.filletRadius = maximumFilletRadiusForFlange;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange minus web outer slope height.";

    params.filletRadius = maximumFilletRadiusForFlange + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange minus web outer slope height.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_FilletRadiusAgainstTheWebWithSlope_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "TT_NonZeroFlangeSlope", 40.0, 10.0, 1.0, 1.0, 1.0, INFINITY);
    params.flangeSlope = Angle::FromRadians ((PI / 180.0) * 1.0);

    TTShapeProfilePtr profilePtr = CreateProfile (params);

    double const maximumFilletRadiusForWeb = profilePtr->GetWebOuterFaceLength() / 2.0 - profilePtr->GetFlangeSlopeHeight();
    EXPECT_GE (maximumFilletRadiusForWeb, 0.0) << "Flange slope height cannot be greater than half of the outer web face length";

    params.filletRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the outer face of the web minus flange slope height.";

    params.filletRadius = maximumFilletRadiusForWeb;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the outer face of the web minus flange slope height.";

    params.filletRadius = maximumFilletRadiusForWeb + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the outer face of the web minus flange slope height.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_InvalidFlangeEdgeRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "TT", 10.0, 10.0, 1.0, 1.0, 1.0, 0.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.flangeEdgeRadius, "FlangeEdgeRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_FlangeEdgeRadiusAgainstTheFlangeThickness_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "TT", 10.0, 10.0, 1.0, 1.0, 1.0, 0.0, INFINITY);

    params.flangeEdgeRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Flange edge radius should be less or equal to the flange thickness.";

    params.flangeEdgeRadius = 1.0 + 0.0000001;
    EXPECT_FAIL_Insert (params) << "Flange edge radius should be less or equal to the flange thickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_FlangeEdgeRadiusAgainstTheInnerFlangeFace_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "TT", 13.0, 10.0, 1.0, 1.0, 1.0, 0.0, INFINITY);

    params.flangeThickness = 4.0;
    params.flangeEdgeRadius = 2.0;
    EXPECT_SUCCESS_Insert (params) << "Flange edge radius should be less or equal to the flange thickness.";

    params.webThickness = 3.0;
    EXPECT_FAIL_Insert (params) << "Flange edge radius should be less or equal to half of inner flange face length.";

    params.flangeEdgeRadius = 1.5;
    EXPECT_SUCCESS_Insert (params) << "Flange edge radius should be less or equal to half of inner flange face length.";

    params.flangeEdgeRadius = 1.5 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Flange edge radius should be less or equal to inner flange face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_InvalidFlangeSlope_FailedInsert)
    {
    CreateParams params (GetModel(), "TT", 9.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (INFINITY));

    TestParameterToBeFiniteAndPositive (params, params.flangeSlope, "FlangeSlope", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_FlangeSlopeOf45Degrees_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "TT", 11.0, 9.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (INFINITY));

    params.flangeSlope = Angle::FromRadians (PI / 4.0);
    TTShapeProfilePtr profilePtr = CreateProfile (params);

    // 45 degree angle means a slope height of 4, when the inner flange face length is 4
    // since inner web face length is 8, a slope of 45 degree should be the maximum allowed value
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetFlangeInnerFaceLength());
    EXPECT_DOUBLE_EQ (8.0, profilePtr->GetWebOuterFaceLength());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetFlangeSlopeHeight());
    EXPECT_SUCCESS_Insert (params) << "Flange slope should be such, that the slope height should be less or equal to half of inner web face length.";

    params.flangeSlope = Angle::FromRadians (PI / 4.0 + TESTS_EPSILON);
    EXPECT_FAIL_Insert (params) << "Flange slope should be such, that the slope height should be less or equal to half of inner web face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_FlangeSlopeOf90Degrees_FailedInsert)
    {
    CreateParams params (GetModel(), "TT", 10.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (INFINITY));

    params.flangeSlope = Angle::FromRadians (0.0);
    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert.";

    params.flangeSlope = Angle::FromRadians (PI / 2.0);
    EXPECT_FAIL_Insert (params) << "Flange slope should be less than 90 degrees.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_InvalidWebEdgeRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "TT", 10.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.webEdgeRadius, "WebEdgeRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_WebEdgeRadiusAgainstWebThickness_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "TT", 10.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), INFINITY);

    params.webEdgeRadius = 0.5;
    EXPECT_SUCCESS_Insert (params) << "Web edge radius should be less or equal to half of the web thickness.";

    params.webEdgeRadius = 0.5 + 0.0000001;
    EXPECT_FAIL_Insert (params) << "Web edge radius should be less or equal to half of the web thickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_WebEdgeRadiusAgainstTheOuterWebFace_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "TT", 10.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), INFINITY);

    params.webThickness = 4.0;
    params.webEdgeRadius = 2.0;
    EXPECT_SUCCESS_Insert (params) << "Web edge radius should be less or equal to half of the web thickness.";

    params.flangeThickness = 7.0;
    EXPECT_FAIL_Insert (params) << "Web edge radius should be less or equal to half of outer web face length.";

    params.webEdgeRadius = 1.5;
    EXPECT_SUCCESS_Insert (params) << "Web edge radius should be less or equal to half of outer web face length.";

    params.webEdgeRadius = 1.5 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Web edge radius should be less or equal to outer web face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_InvalidWebSlope_FailedInsert)
    {
    CreateParams params (GetModel(), "TT", 50.0, 10.0, 1.0, 1.0, 10.0, 0.0, 0.0, Angle::FromRadians (0.0), 0.0, Angle::FromRadians (INFINITY));

    TestParameterToBeFiniteAndPositive (params, params.webSlope, "WebSlope", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_WebSlopeOf45Degrees_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "TT", 28.0, 5.0, 1.0, 1.0, 10.0, 0.0, 0.0, Angle::FromRadians (0.0), 0.0, Angle::FromRadians (INFINITY));

    params.webSlope = Angle::FromRadians (PI / 4.0);
    TTShapeProfilePtr profilePtr = CreateProfile (params);

    params.webSpacing = 1.0;
    EXPECT_FAIL_Insert (params) << "Web inner slope should be such, that the slope height should be less or equal to half of web spacing.";

    params.webSpacing = 10.0;
    // 45 degree angle means a slope height of 4, when the inner web face length is 4
    // since inner flange face length is 8, a slope of 45 degree should be the maximum allowed value
    EXPECT_DOUBLE_EQ (8.0, profilePtr->GetFlangeInnerFaceLength());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetWebOuterFaceLength());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetWebOuterSlopeHeight());
    EXPECT_SUCCESS_Insert (params) << "Web outer slope should be such, that the slope height should be less or equal to half of inner flange face length.";

    params.flangeSlope = Angle::FromRadians (PI / 4.0 + TESTS_EPSILON);
    EXPECT_FAIL_Insert (params) << "Web slope should be such, that the slope height should be less or equal to half of inner flange face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TTShapeProfileTestCase, Insert_EdgeSlopeOf90Degrees_FailedInsert)
    {
    CreateParams params (GetModel(), "TT", 10.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), 0.0, Angle::FromRadians (INFINITY));

    params.webSlope = Angle::FromRadians (0.0);
    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert.";

    params.webSlope = Angle::FromRadians (PI / 2.0);
    EXPECT_FAIL_Insert (params) << "Web slope should be less than 90 degrees.";
    }
