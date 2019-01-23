/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/AsymmetricIShapeProfileTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct AsymmetricIShapeProfileTestCase : ProfileValidationTestCase<AsymmetricIShapeProfile>
    {
public:
    typedef AsymmetricIShapeProfile::CreateParams CreateParams;

    AsymmetricIShapeProfile::CreateParams InfinityCreateParams (Utf8CP pName = "AsymmetricI")
        {
        return CreateParams (GetModel(), pName, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY,
            INFINITY, INFINITY, Angle::FromRadians (INFINITY), INFINITY, INFINITY, Angle::FromRadians (INFINITY));;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "AsymmetricI");

    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "AsymmetricI", 10, 10, 10, 1, 1, 1);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    CreateParams fullParams (GetModel(), "AsymmetricI", 10, 10, 10, 1, 1, 1, 1, 0.5, Angle::FromRadians (PI / 18), 1, 0.5, Angle::FromRadians (PI / 18));
    EXPECT_SUCCESS_Insert (fullParams) << "Profile should succeed to insert with valid full create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "AsymmetricI", 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, Angle::FromRadians (9.0), 10.0, 11.0, Angle::FromRadians (12.0));

    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("AsymmetricI", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetTopFlangeWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetBottomFlangeWidth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetTopFlangeThickness());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetBottomFlangeThickness());
    EXPECT_DOUBLE_EQ (6.0, profilePtr->GetWebThickness());
    EXPECT_DOUBLE_EQ (7.0, profilePtr->GetTopFlangeFilletRadius());
    EXPECT_DOUBLE_EQ (8.0, profilePtr->GetTopFlangeEdgeRadius());
    EXPECT_DOUBLE_EQ (9.0, profilePtr->GetTopFlangeSlope().Radians());
    EXPECT_DOUBLE_EQ (10.0, profilePtr->GetBottomFlangeFilletRadius());
    EXPECT_DOUBLE_EQ (11.0, profilePtr->GetBottomFlangeEdgeRadius());
    EXPECT_DOUBLE_EQ (12.0, profilePtr->GetBottomFlangeSlope().Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams = InfinityCreateParams (nullptr);

    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("AsymmetricI");
    profilePtr->SetTopFlangeWidth (1.0);
    profilePtr->SetBottomFlangeWidth (2.0);
    profilePtr->SetDepth (3.0);
    profilePtr->SetTopFlangeThickness (4.0);
    profilePtr->SetBottomFlangeThickness (5.0);
    profilePtr->SetWebThickness (6.0);
    profilePtr->SetTopFlangeFilletRadius (7.0);
    profilePtr->SetTopFlangeEdgeRadius (8.0);
    profilePtr->SetTopFlangeSlope (Angle::FromRadians (9.0));
    profilePtr->SetBottomFlangeFilletRadius (10.0);
    profilePtr->SetBottomFlangeEdgeRadius (11.0);
    profilePtr->SetBottomFlangeSlope (Angle::FromRadians (12.0));

    EXPECT_EQ ("AsymmetricI", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetTopFlangeWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetBottomFlangeWidth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetTopFlangeThickness());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetBottomFlangeThickness());
    EXPECT_DOUBLE_EQ (6.0, profilePtr->GetWebThickness());
    EXPECT_DOUBLE_EQ (7.0, profilePtr->GetTopFlangeFilletRadius());
    EXPECT_DOUBLE_EQ (8.0, profilePtr->GetTopFlangeEdgeRadius());
    EXPECT_DOUBLE_EQ (9.0, profilePtr->GetTopFlangeSlope().Radians());
    EXPECT_DOUBLE_EQ (10.0, profilePtr->GetBottomFlangeFilletRadius());
    EXPECT_DOUBLE_EQ (11.0, profilePtr->GetBottomFlangeEdgeRadius());
    EXPECT_DOUBLE_EQ (12.0, profilePtr->GetBottomFlangeSlope().Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, GetInnerTopFlangeFaceLength_TopFlangeWidthAndWebThickness_CorrectValue)
    {
    CreateParams params = InfinityCreateParams();
    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetTopFlangeWidth (0.0);
    profilePtr->SetWebThickness (0.0);
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetTopFlangeInnerFaceLength());

    profilePtr->SetTopFlangeWidth (3.0);
    profilePtr->SetWebThickness (1.0);
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetTopFlangeInnerFaceLength());

    profilePtr->SetTopFlangeWidth (1.0);
    profilePtr->SetWebThickness (3.0);
    EXPECT_DOUBLE_EQ (-1.0, profilePtr->GetTopFlangeInnerFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, GetInnerBottomFlangeFaceLength_BottomFlangeWidthAndWebThickness_CorrectValue)
    {
    CreateParams params = InfinityCreateParams();
    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetBottomFlangeWidth (0.0);
    profilePtr->SetWebThickness (0.0);
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetBottomFlangeInnerFaceLength());

    profilePtr->SetBottomFlangeWidth (3.0);
    profilePtr->SetWebThickness (1.0);
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetBottomFlangeInnerFaceLength());

    profilePtr->SetBottomFlangeWidth (1.0);
    profilePtr->SetWebThickness (3.0);
    EXPECT_DOUBLE_EQ (-1.0, profilePtr->GetBottomFlangeInnerFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, GetInnerWebFaceLength_DepthAndFlangeThickness_CorrectValue)
    {
    CreateParams params = InfinityCreateParams();
    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetDepth (0.0);
    profilePtr->SetTopFlangeThickness (0.0);
    profilePtr->SetBottomFlangeThickness (0.0);
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetWebInnerFaceLength());

    profilePtr->SetDepth (3.0);
    profilePtr->SetTopFlangeThickness (1.0);
    profilePtr->SetBottomFlangeThickness (1.0);
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWebInnerFaceLength());

    profilePtr->SetDepth (1.0);
    profilePtr->SetTopFlangeThickness (2.0);
    profilePtr->SetBottomFlangeThickness (2.0);
    EXPECT_DOUBLE_EQ (-3.0, profilePtr->GetWebInnerFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, GetTopFlangeSlopeHeight_ProfileWithProperties_CorrectValue)
    {
    CreateParams params = InfinityCreateParams();
    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetTopFlangeWidth (3.0);
    profilePtr->SetWebThickness (1.0);

    profilePtr->SetTopFlangeSlope (Angle::FromRadians (PI / 4.0));
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetTopFlangeSlopeHeight());

    profilePtr->SetTopFlangeSlope (Angle::FromRadians (0.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetTopFlangeSlopeHeight());

    profilePtr->SetTopFlangeSlope (Angle::FromRadians (PI / 2.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetTopFlangeSlopeHeight());

    profilePtr->SetTopFlangeSlope (Angle::FromRadians (PI));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetTopFlangeSlopeHeight());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, GetBottomFlangeSlopeHeight_ProfileWithProperties_CorrectValue)
    {
    CreateParams params = InfinityCreateParams();
    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetBottomFlangeWidth (3.0);
    profilePtr->SetWebThickness (1.0);

    profilePtr->SetBottomFlangeSlope (Angle::FromRadians (PI / 4.0));
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetBottomFlangeSlopeHeight());

    profilePtr->SetBottomFlangeSlope (Angle::FromRadians (0.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetBottomFlangeSlopeHeight());

    profilePtr->SetBottomFlangeSlope (Angle::FromRadians (PI / 2.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetBottomFlangeSlopeHeight());

    profilePtr->SetBottomFlangeSlope (Angle::FromRadians (PI));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetBottomFlangeSlopeHeight());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 10.0, 1.0, 1.0, 1.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 10.0, 1.0, 1.0, 1.0);

    params.name = "AsymmetricI";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_InvalidTopFlangeWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", INFINITY, 10.0, 10.0, 1.0, 1.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.topFlangeWidth, "TopFlangeWidth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_ValidTopFlangeWidth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", INFINITY, 10.0, 10.0, 1.0, 1.0, 1.0);

    params.topFlangeWidth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Top flange width should be of positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_InvalidBottomFlangeWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, INFINITY, 10.0, 1.0, 1.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.bottomFlangeWidth, "BottomFlangeWidth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_ValidBottomFlangeWidth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, INFINITY, 10.0, 1.0, 1.0, 1.0);

    params.bottomFlangeWidth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Bottom flange width should be of positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_InvalidDepth_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, INFINITY, 1.0, 1.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.depth, "Depth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_ValidDepth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, INFINITY, 1.0, 1.0, 1.0);

    params.depth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Depth should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_InvalidTopFlangeThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, 10.0, INFINITY, 1.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.topFlangeThickness, "TopFlangeThickness", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_InvalidBottomFlangeThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, 10.0, 1.0, INFINITY, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.bottomFlangeThickness, "BottomFlangeThickness", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_ValidTopFlangeThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, 10.0, INFINITY, 1.0, 1.0);

    params.topFlangeThickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Top flange thickness should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_ValidBottomFlangeThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, 10.0, 1.0, INFINITY, 1.0);

    params.bottomFlangeThickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Bottom flange thickness should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_TopAndBottomFlangeThicknessGreaterThanDepth_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, 10.0, INFINITY, INFINITY, 1.0);

    double const halfDepth = params.depth / 2.0;

    params.topFlangeThickness = halfDepth;
    params.bottomFlangeThickness = halfDepth;
    EXPECT_FAIL_Insert (params) << "Top flange thickness added with bottom flange thickness should be less than depth.";

    params.topFlangeThickness = BeNumerical::BeNextafter (halfDepth, 0.0);
    params.bottomFlangeThickness = BeNumerical::BeNextafter (halfDepth, 0.0);
    EXPECT_SUCCESS_Insert (params) << "Top flange thickness added with bottom flange thickness should be less than depth.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_InvalidWebThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, 10.0, 1.0, 1.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.webThickness, "WebThickness", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_WebThicknessEqualToTopFlangeWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 20.0, 10.0, 1.0, 1.0, INFINITY);

    params.webThickness = params.topFlangeWidth;
    EXPECT_FAIL_Insert (params) << "Web thickness should be less than top flange width.";

    params.webThickness = params.topFlangeWidth + 1.0;
    EXPECT_FAIL_Insert (params) << "Web thickness should be less than top flange width.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_WebThicknessEqualToBottomFlangeWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 20.0, 10.0, 10.0, 1.0, 1.0, INFINITY);

    params.webThickness = params.bottomFlangeWidth;
    EXPECT_FAIL_Insert (params) << "Web thickness should be less than bottom flange width.";

    params.webThickness = params.bottomFlangeWidth + 1.0;
    EXPECT_FAIL_Insert (params) << "Web thickness should be less than bottom flange width.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_ValidWebThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 20.0, 10.0, 1.0, 1.0, INFINITY);

    params.webThickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Web thickness should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_InvalidTopFlangeFilletRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, 10.0, 1.0, 1.0, 1.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.topFlangeFilletRadius, "TopFlangeFilletRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_InvalidBottomFlangeFilletRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI_ZeroFlangeSlope_ShortWeb", 10.0, 10.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.bottomFlangeFilletRadius, "BototmFlangeFilletRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_TopFlangeFilletRadiusAgainstTheFlange_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "AsymmetricI_ZeroFlangeSlope_ShortTopFlange", 3.0, 10.0, 10.0, 1.0, 1.0, 1.0, INFINITY);
    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);

    params.topFlangeFilletRadius = 0.5;
    EXPECT_DOUBLE_EQ (0.5, profilePtr->GetTopFlangeInnerFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Top flange fillet radius should be less or equal to half of the inner face of the top flange.";

    params.topFlangeFilletRadius = BeNumerical::BeNextafter (0.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Top flange fillet radius should be less or equal to half of the inner face of the top flange.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_BottomFlangeFilletRadiusAgainstTheFlange_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "AsymmetricI_ZeroFlangeSlope_ShortBottomFlange", 10.0, 3.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), INFINITY);
    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);

    params.bottomFlangeFilletRadius = 0.5;
    EXPECT_DOUBLE_EQ (0.5, profilePtr->GetBottomFlangeInnerFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Bottom flange fillet radius should be less or equal to half of the inner face of the bottom flange.";

    params.bottomFlangeFilletRadius = BeNumerical::BeNextafter (0.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Bottom flange fillet radius should be less or equal to half of the inner face of the bottom flange.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_TopFlangeFilletRadiusAgainstTheWeb_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "AsymmetricI_ZeroFlangeSlope_ShortWeb", 10.0, 10.0, 3.0, 1.0, 1.0, 1.0, INFINITY);
    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);

    params.topFlangeFilletRadius = 0.5;
    EXPECT_DOUBLE_EQ (0.5, profilePtr->GetWebInnerFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Top flange fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";

    params.topFlangeFilletRadius = BeNumerical::BeNextafter (0.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Top flange fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_BottomFlangeFilletRadiusAgainstTheWeb_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "AsymmetricI_ZeroFlangeSlope_ShortWeb", 10.0, 10.0, 3.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), INFINITY);
    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);

    params.bottomFlangeFilletRadius = 0.5;
    EXPECT_DOUBLE_EQ (0.5, profilePtr->GetWebInnerFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Bottom flange fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";

    params.bottomFlangeFilletRadius = BeNumerical::BeNextafter (0.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Bottom flange fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_TopFlangeFilletRadiusAgainstTheWebWithSlope_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "AsymmetricI_TopFlangeSlope_ShortWeb", 25.0, 10.0, 10.0, 1.0, 1.0, 1.0, INFINITY);
    params.topFlangeSlope = Angle::FromRadians ((PI / 180.0) * 5.0);

    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);

    double const maximumFilletRadiusForWeb = profilePtr->GetWebInnerFaceLength() / 2.0 - profilePtr->GetTopFlangeSlopeHeight();
    EXPECT_GE (maximumFilletRadiusForWeb, 0.0) << "Top flange slope height cannot be greater than half of the inner web face length";

    params.topFlangeFilletRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Top flange fillet radius should be less or equal to half of the inner face of the web minus top flange slope height.";

    params.topFlangeFilletRadius = maximumFilletRadiusForWeb;
    EXPECT_SUCCESS_Insert (params) << "Top flange fillet radius should be less or equal to half of the inner face of the web minus top flange slope height.";

    params.topFlangeFilletRadius = BeNumerical::BeNextafter (maximumFilletRadiusForWeb, INFINITY);
    EXPECT_FAIL_Insert (params) << "Top flange fillet radius should be less or equal to half of the inner face of the web minus top flange slope height.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_BottomFlangeFilletRadiusAgainstTheWebWithSlope_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "AsymmetricI_BottomFlangeSlope_ShortWeb", 10.0, 25.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), INFINITY);
    params.bottomFlangeSlope = Angle::FromRadians ((PI / 180.0) * 5.0);

    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);

    double const maximumFilletRadiusForWeb = profilePtr->GetWebInnerFaceLength() / 2.0 - profilePtr->GetBottomFlangeSlopeHeight();
    EXPECT_GE (maximumFilletRadiusForWeb, 0.0) << "Bottom flange slope height cannot be greater than half of the inner web face length";

    params.bottomFlangeFilletRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Bottom flange fillet radius should be less or equal to half of the inner face of the web minus bottom flange slope height.";

    params.bottomFlangeFilletRadius = maximumFilletRadiusForWeb;
    EXPECT_SUCCESS_Insert (params) << "Bottom flange fillet radius should be less or equal to half of the inner face of the web minus bottom flange slope height.";

    params.bottomFlangeFilletRadius = BeNumerical::BeNextafter (maximumFilletRadiusForWeb, INFINITY);
    EXPECT_FAIL_Insert (params) << "Bottom flange fillet radius should be less or equal to half of the inner face of the web minus bottom flange slope height.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_InvalidTopFlangeEdgeRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, 10.0, 1.0, 1.0, 1.0, 0.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.topFlangeEdgeRadius, "TopFlangeEdgeRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_InvalidBottomFlangeEdgeRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), 0.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.bottomFlangeEdgeRadius, "BottomFlangeEdgeRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_TopFlangeEdgeRadiusAgainstFlangeThickness_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, 10.0, 1.0, 2.0, 1.0, 0.0, INFINITY);

    params.topFlangeEdgeRadius = 0.5;
    EXPECT_SUCCESS_Insert (params) << "Top flange edge radius should be less or equal to half of the top flange thickness.";

    params.topFlangeEdgeRadius = BeNumerical::BeNextafter (0.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Top flange edge radius should be less or equal to half of the top flange thickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_BottomFlangeEdgeRadiusAgainstFlangeThickness_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, 10.0, 2.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), 0.0, INFINITY);

    params.bottomFlangeEdgeRadius = 0.5;
    EXPECT_SUCCESS_Insert (params) << "Bottom flange edge radius should be less or equal to half of the bottom flange thickness.";

    params.bottomFlangeEdgeRadius = BeNumerical::BeNextafter (0.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Bottom flange edge radius should be less or equal to half of the bottom flange thickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_TopFlangeEdgeRadiusAgainstInnerFlangeFace_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 50.0, 10.0, 1.0, 2.0, 1.0, 0.0, INFINITY);

    params.topFlangeThickness = 4.0;
    params.topFlangeEdgeRadius = 2.0;
    EXPECT_SUCCESS_Insert (params) << "Top flange edge radius should be less or equal to half of the top flange thickness.";

    params.webThickness = 8.0;
    EXPECT_FAIL_Insert (params) << "Top flange edge radius should be less or equal to inner top flange face length.";

    params.topFlangeEdgeRadius = 0.5;
    EXPECT_SUCCESS_Insert (params) << "Top flange edge radius should be less or equal to inner top flange face length.";

    params.topFlangeEdgeRadius = BeNumerical::BeNextafter (0.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Top flange edge radius should be less or equal to inner top flange face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_BottomFlangeEdgeRadiusAgainstInnerFlangeFace_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "AsymmetricI", 50.0, 10.0, 10.0, 2.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), 0.0, INFINITY);

    params.bottomFlangeThickness = 4.0;
    params.bottomFlangeEdgeRadius = 2.0;
    EXPECT_SUCCESS_Insert (params) << "Bottom flange edge radius should be less or equal to half of the bottom flange thickness.";

    params.webThickness = 8.0;
    EXPECT_FAIL_Insert (params) << "Bottom flange edge radius should be less or equal to inner bottom flange face length.";

    params.bottomFlangeEdgeRadius = 0.5;
    EXPECT_SUCCESS_Insert (params) << "Bottom flange edge radius should be less or equal to inner bottom flange face length.";

    params.bottomFlangeEdgeRadius = BeNumerical::BeNextafter (0.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Bottom flange edge radius should be less or equal to inner bottom flange face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_InvalidTopFlangeSlope_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (INFINITY));

    TestParameterToBeFiniteAndPositive (params, params.topFlangeSlope, "TopFlangeSlope", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_InvalidBottomFlangeSlope_FailedInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 10.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), 0.0, 0.0, Angle::FromRadians (INFINITY));

    TestParameterToBeFiniteAndPositive (params, params.bottomFlangeSlope, "BottomFlangeSlope", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_TopFlangeSlopeOf45Degrees_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 9.0, 10.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (INFINITY));

    params.topFlangeSlope = Angle::FromRadians (PI / 4.0);
    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);

    // 45 degree angle means a slope height of 4, when the inner flange face length is 4
    // since inner web face length is 8, a slope of 45 degree should be the maximum allowed value
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetTopFlangeInnerFaceLength());
    EXPECT_DOUBLE_EQ (8.0, profilePtr->GetWebInnerFaceLength());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetTopFlangeSlopeHeight());
    EXPECT_SUCCESS_Insert (params) << "Top flange slope should be such, that the slope height should be less or equal to half of inner web face length.";

    params.topFlangeSlope = Angle::FromRadians (BeNumerical::BeNextafter (PI / 4.0, INFINITY));
    EXPECT_FAIL_Insert (params) << "Top flange slope should be such, that the slope height should be less or equal to half of inner web face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_BottomFlangeSlopeOf45Degrees_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "AsymmetricI", 10.0, 9.0, 10.0, 1.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), 0.0, 0.0, Angle::FromRadians (INFINITY));

    params.bottomFlangeSlope = Angle::FromRadians (PI / 4.0);
    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);

    // 45 degree angle means a slope height of 4, when the inner flange face length is 4
    // since inner web face length is 8, a slope of 45 degree should be the maximum allowed value
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetBottomFlangeInnerFaceLength());
    EXPECT_DOUBLE_EQ (8.0, profilePtr->GetWebInnerFaceLength());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetBottomFlangeSlopeHeight());
    EXPECT_SUCCESS_Insert (params) << "Bottom flange slope should be such, that the slope height should be less or equal to half of inner web face length.";

    params.bottomFlangeSlope = Angle::FromRadians (BeNumerical::BeNextafter (PI / 4.0, INFINITY));
    EXPECT_FAIL_Insert (params) << "Bottom flange slope should be such, that the slope height should be less or equal to half of inner web face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_TopFlangeSlopeOf90Degrees_FailedInsert)
    {
    // Can't use DBL_MAX as it will assert in geometry generation
    double const depth = DBL_MAX / 4.0;
    CreateParams params (GetModel(), "AsymmetricI", 1.0, 1.0, depth, 0.1, 0.1, 0.1, 0.0, 0.0, Angle::FromRadians (INFINITY));

    params.topFlangeSlope = Angle::FromRadians (0.0);
    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert.";

    params.topFlangeSlope = Angle::FromRadians (PI / 2.0);
    EXPECT_FAIL_Insert (params) << "Top flange slope should be less than 90 degrees.";

    params.topFlangeSlope = Angle::FromRadians (PI / 2.0 - PI / 10000);
    EXPECT_SUCCESS_Insert (params) << "Top flange slope should be less than 90 degrees.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Insert_BottomFlangeSlopeOf90Degrees_FailedInsert)
    {
    // Can't use DBL_MAX as it will assert in geometry generation
    double const depth = DBL_MAX / 4.0;
    CreateParams params (GetModel(), "AsymmetricI", 1.0, 1.0, depth, 0.1, 0.1, 0.1, 0.0, 0.0, Angle::FromRadians (0.0), 0.0, 0.0, Angle::FromRadians (INFINITY));

    params.bottomFlangeSlope = Angle::FromRadians (0.0);
    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert.";

    params.bottomFlangeSlope = Angle::FromRadians (PI / 2.0);
    EXPECT_FAIL_Insert (params) << "Bottom flange slope should be less than 90 degrees.";

    params.bottomFlangeSlope = Angle::FromRadians (PI / 2.0 - PI / 10000);
    EXPECT_SUCCESS_Insert (params) << "Bottom flange slope should be less than 90 degrees.";
    }
