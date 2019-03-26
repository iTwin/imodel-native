/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/DoubleCShapeProfileTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct DoubleCShapeProfileTestCase : ProfileValidationTestCase<DoubleCShapeProfile>
    {
public:
    typedef DoubleCShapeProfile::CreateParams CreateParams;

protected:
    CShapeProfileCPtr InsertCShapeProfile()
        {
        CShapeProfile::CreateParams params (GetModel(), "C", 10.0, 6.0, 1.0, 1.0);
        CShapeProfilePtr profilePtr = CShapeProfile::Create (params);

        DgnDbStatus status;
        profilePtr->Insert (&status);
        BeAssert (status == DgnDbStatus::Success);

        return profilePtr;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleCShapeProfileTestCase, Create_ValidCShapeProfile_ValidInstance)
    {
    CShapeProfileCPtr singleProfilePtr = InsertCShapeProfile();
    CreateParams params (GetModel(), "DoubleC", 1.0, *singleProfilePtr);

    DoubleCShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleCShapeProfileTestCase, Insert_ValidCShapeProfile_SuccessfulInsert)
    {
    CShapeProfileCPtr singleProfilePtr = InsertCShapeProfile();

    CreateParams requiredParams (GetModel(), "DoubleC", 1.0, *singleProfilePtr);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleCShapeProfileTestCase, Insert_ValidCShapeProfileId_SuccessfulInsert)
    {
    CShapeProfileCPtr singleProfilePtr = InsertCShapeProfile();

    CreateParams requiredParams (GetModel(), "DoubleC", 1.0, singleProfilePtr->GetElementId());
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleCShapeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CShapeProfileCPtr singleProfilePtr = InsertCShapeProfile();
    CreateParams params (GetModel(), "DoubleC", 1.0, singleProfilePtr->GetElementId());

    DoubleCShapeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("DoubleC", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetSpacing());
    EXPECT_EQ (singleProfilePtr->GetElementId(), profilePtr->GetSingleProfile()->GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleCShapeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CShapeProfileCPtr singleProfilePtr = InsertCShapeProfile();
    CreateParams params (GetModel(), "DoubleC", 1.0, DgnElementId ((uint64_t)0));

    DoubleCShapeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("DoubleC");
    profilePtr->SetSpacing (1.0);
    profilePtr->SetSingleProfile (singleProfilePtr->GetElementId());

    EXPECT_EQ ("DoubleC", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetSpacing());
    EXPECT_EQ (singleProfilePtr->GetElementId(), profilePtr->GetSingleProfile()->GetElementId());

    CShapeProfileCPtr otherSingleProfilePtr = InsertCShapeProfile();
    profilePtr->SetSingleProfile (*otherSingleProfilePtr);
    EXPECT_EQ (otherSingleProfilePtr->GetElementId(), profilePtr->GetSingleProfile()->GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleCShapeProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CShapeProfileCPtr singleProfilePtr = InsertCShapeProfile();
    CreateParams params (GetModel(), nullptr, 1.0, singleProfilePtr->GetElementId());

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleCShapeProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CShapeProfileCPtr singleProfilePtr = InsertCShapeProfile();
    CreateParams params (GetModel(), nullptr, 1.0, singleProfilePtr->GetElementId());

    params.name = "DoubleC";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleCShapeProfileTestCase, Insert_InvalidSpacing_FailedInsert)
    {
    CShapeProfileCPtr singleProfilePtr = InsertCShapeProfile();
    CreateParams params (GetModel(), "DoubleC", INFINITY, singleProfilePtr->GetElementId());

    TestParameterToBeFiniteAndPositive (params, params.spacing, "Spacing", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleCShapeProfileTestCase, Insert_InvalidSingleProfileId_FailedInsert)
    {
    CreateParams params (GetModel(), "DoubleC", 1.0, DgnElementId());
    EXPECT_FAIL_Insert (params) << "Profile should fail with invalid SinglieProfile id.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleCShapeProfileTestCase, Insert_InvalidSingleProfileClass_FailedInsert)
    {
    LShapeProfile::CreateParams params (GetModel(), "L", 10.0, 6.0, 1.0);
    LShapeProfilePtr profilePtr = LShapeProfile::Create (params);

    DgnDbStatus status;
    profilePtr->Insert (&status);
    EXPECT_EQ (status, DgnDbStatus::Success);

    CreateParams requiredParams (GetModel(), "DoubleC", 1.0, profilePtr->GetElementId());
    EXPECT_FAIL_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleCShapeProfileTestCase, SetSingleProfile_ValidCShapeProfileId_Success)
    {
    CShapeProfileCPtr singleProfilePtr = InsertCShapeProfile();

    CreateParams requiredParams (GetModel(), "DoubleC", 1.0, singleProfilePtr->GetElementId());
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    DgnElementIdSet children = singleProfilePtr->QueryChildren();
    ASSERT_EQ (1, children.size());

    DoubleCShapeProfilePtr doubleProfilePtr = DoubleCShapeProfile::GetForEdit (GetDb(), *children.begin());
    ASSERT_TRUE (doubleProfilePtr.IsValid());

    CShapeProfileCPtr otherSingleProfilePtr = InsertCShapeProfile();
    ASSERT_EQ (DgnDbStatus::Success, doubleProfilePtr->SetSingleProfile (otherSingleProfilePtr->GetElementId()));

    DgnDbStatus status;
    doubleProfilePtr->Update (&status);
    ASSERT_EQ (DgnDbStatus::Success, status);

    EXPECT_EQ (0, singleProfilePtr->QueryChildren().size());
    EXPECT_EQ (1, otherSingleProfilePtr->QueryChildren().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleCShapeProfileTestCase, SetSingleProfile_InValidCShapeProfileId_Fail)
    {
    CShapeProfileCPtr singleProfilePtr = InsertCShapeProfile();

    CreateParams requiredParams (GetModel(), "DoubleC", 1.0, singleProfilePtr->GetElementId());
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    DgnElementIdSet children = singleProfilePtr->QueryChildren();
    ASSERT_EQ (1, children.size());

    DoubleCShapeProfilePtr doubleProfilePtr = DoubleCShapeProfile::GetForEdit (GetDb(), *children.begin());
    ASSERT_TRUE (doubleProfilePtr.IsValid());

    ASSERT_EQ (DgnDbStatus::Success, doubleProfilePtr->SetSingleProfile (DgnElementId()));

    DgnDbStatus status;
    doubleProfilePtr->Update (&status);
    ASSERT_NE (DgnDbStatus::Success, status);

    EXPECT_EQ (1, singleProfilePtr->QueryChildren().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleCShapeProfileTestCase, SetSingleProfile_InValidSingleProfileClass_Fail)
    {
    CShapeProfileCPtr singleProfilePtr = InsertCShapeProfile();

    CreateParams requiredParams (GetModel(), "DoubleC", 1.0, singleProfilePtr->GetElementId());
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    DgnElementIdSet children = singleProfilePtr->QueryChildren();
    ASSERT_EQ (1, children.size());

    DoubleCShapeProfilePtr doubleProfilePtr = DoubleCShapeProfile::GetForEdit (GetDb(), *children.begin());
    ASSERT_TRUE (doubleProfilePtr.IsValid());

    LShapeProfile::CreateParams params (GetModel(), "L", 10.0, 6.0, 1.0);
    LShapeProfilePtr lProfilePtr = LShapeProfile::Create (params);

    DgnDbStatus status;
    lProfilePtr->Insert (&status);
    EXPECT_EQ (status, DgnDbStatus::Success);
    ASSERT_EQ (DgnDbStatus::Success, doubleProfilePtr->SetSingleProfile (lProfilePtr->GetElementId()));

    doubleProfilePtr->Update (&status);
    ASSERT_NE (DgnDbStatus::Success, status);

    EXPECT_EQ (1, singleProfilePtr->QueryChildren().size());
    EXPECT_EQ (0, lProfilePtr->QueryChildren().size());
    }
