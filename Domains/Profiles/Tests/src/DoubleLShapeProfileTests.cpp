/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/DoubleLShapeProfileTests.cpp $
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
struct DoubleLShapeProfileTestCase : ProfileValidationTestCase<DoubleLShapeProfile>
    {
public:
    typedef DoubleLShapeProfile::CreateParams CreateParams;

protected:
    LShapeProfileCPtr InsertLShapeProfile()
        {
        LShapeProfile::CreateParams params (GetModel(), "L", 10.0, 6.0, 1.0);
        LShapeProfilePtr profilePtr = LShapeProfile::Create (params);

        DgnDbStatus status;
        profilePtr->Insert (&status);
        BeAssert (status == DgnDbStatus::Success);

        return profilePtr;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleLShapeProfileTestCase, Create_ValidLShapeProfile_ValidInstance)
    {
    LShapeProfileCPtr singleProfilePtr = InsertLShapeProfile();
    CreateParams params (GetModel(), "DoubleL", 1.0, singleProfilePtr->GetElementId());

    DoubleLShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleLShapeProfileTestCase, Insert_ValidLShapeProfile_SuccessfulInsert)
    {
    LShapeProfileCPtr singleProfilePtr = InsertLShapeProfile();

    CreateParams requiredParams (GetModel(), "DoubleL", 1.0, *singleProfilePtr);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleLShapeProfileTestCase, Insert_ValidLShapeProfileId_SuccessfulInsert)
    {
    LShapeProfileCPtr singleProfilePtr = InsertLShapeProfile();

    CreateParams requiredParams (GetModel(), "DoubleL", 1.0, singleProfilePtr->GetElementId());
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleLShapeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    LShapeProfileCPtr singleProfilePtr = InsertLShapeProfile();
    CreateParams params (GetModel(), "DoubleL", 1.0, singleProfilePtr->GetElementId(), DoubleLShapeProfileType::SLBB);

    DoubleLShapeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("DoubleL", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetSpacing());
    EXPECT_EQ (singleProfilePtr->GetElementId(), profilePtr->GetSingleProfile()->GetElementId());
    EXPECT_EQ (DoubleLShapeProfileType::SLBB, profilePtr->GetType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleLShapeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    LShapeProfileCPtr singleProfilePtr = InsertLShapeProfile();
    CreateParams params (GetModel(), "DoubleL", 1.0, DgnElementId ((uint64_t)0), DoubleLShapeProfileType::LLBB);

    DoubleLShapeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("DoubleL");
    profilePtr->SetSpacing (1.0);
    profilePtr->SetSingleProfile (singleProfilePtr->GetElementId());
    profilePtr->SetType (DoubleLShapeProfileType::SLBB);

    EXPECT_EQ ("DoubleL", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetSpacing());
    EXPECT_EQ (singleProfilePtr->GetElementId(), profilePtr->GetSingleProfile()->GetElementId());
    EXPECT_EQ (DoubleLShapeProfileType::SLBB, profilePtr->GetType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleLShapeProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    LShapeProfileCPtr singleProfilePtr = InsertLShapeProfile();
    CreateParams params (GetModel(), nullptr, 1.0, singleProfilePtr->GetElementId());

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleLShapeProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    LShapeProfileCPtr singleProfilePtr = InsertLShapeProfile();
    CreateParams params (GetModel(), nullptr, 1.0, singleProfilePtr->GetElementId());

    params.name = "DoubleL";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleLShapeProfileTestCase, Insert_InvalidSpacing_FailedInsert)
    {
    LShapeProfileCPtr singleProfilePtr = InsertLShapeProfile();
    CreateParams params (GetModel(), "DoubleL", INFINITY, singleProfilePtr->GetElementId());

    TestParameterToBeFiniteAndPositive (params, params.spacing, "Spacing", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleLShapeProfileTestCase, Insert_InvalidType_FailedInsert)
    {
    LShapeProfileCPtr singleProfilePtr = InsertLShapeProfile();
    CreateParams params (GetModel(), "DoubleL", 1.0, singleProfilePtr->GetElementId());

    params.type = (DoubleLShapeProfileType)-1;
    EXPECT_FAIL_Insert (params);

    params.type = (DoubleLShapeProfileType)((int)DoubleLShapeProfileType::SLBB + 1);
    EXPECT_FAIL_Insert (params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleLShapeProfileTestCase, Insert_ValidType_SuccessfulInsert)
    {
    LShapeProfileCPtr singleProfilePtr = InsertLShapeProfile();
    CreateParams params (GetModel(), "DoubleL", 1.0, singleProfilePtr->GetElementId());

    params.type = DoubleLShapeProfileType::LLBB;
    EXPECT_SUCCESS_Insert (params);

    params.type = DoubleLShapeProfileType::SLBB;
    EXPECT_SUCCESS_Insert (params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleLShapeProfileTestCase, Insert_InvalidSingleProfileId_FailedInsert)
    {
    CreateParams params (GetModel(), "DoubleL", 1.0, DgnElementId());
    EXPECT_FAIL_Insert (params) << "Profile should fail with invalid SinglieProfile id.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleLShapeProfileTestCase, Insert_InvalidSingleProfileClass_FailedInsert)
    {
    CShapeProfile::CreateParams params (GetModel(), "C", 10.0, 6.0, 1.0, 1.0);
    CShapeProfilePtr profilePtr = CShapeProfile::Create (params);

    DgnDbStatus status;
    profilePtr->Insert (&status);
    EXPECT_EQ (status, DgnDbStatus::Success);

    CreateParams requiredParams (GetModel(), "DoubleL", 1.0, profilePtr->GetElementId());
    EXPECT_FAIL_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleLShapeProfileTestCase, SetSingleProfile_ValidLShapeProfileId_Success)
    {
    LShapeProfileCPtr singleProfilePtr = InsertLShapeProfile();

    CreateParams requiredParams (GetModel(), "DoubleL", 1.0, singleProfilePtr->GetElementId());
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    DgnElementIdSet children = singleProfilePtr->QueryChildren();
    ASSERT_EQ (1, children.size());

    DoubleLShapeProfilePtr doubleProfilePtr = DoubleLShapeProfile::GetForEdit (GetDb(), *children.begin());
    ASSERT_TRUE (doubleProfilePtr.IsValid());

    LShapeProfileCPtr otherSingleProfilePtr = InsertLShapeProfile();
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
TEST_F (DoubleLShapeProfileTestCase, SetSingleProfile_InValidLShapeProfileId_Fail)
    {
    LShapeProfileCPtr singleProfilePtr = InsertLShapeProfile();

    CreateParams requiredParams (GetModel(), "DoubleL", 1.0, singleProfilePtr->GetElementId());
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    DgnElementIdSet children = singleProfilePtr->QueryChildren();
    ASSERT_EQ (1, children.size());

    DoubleLShapeProfilePtr doubleProfilePtr = DoubleLShapeProfile::GetForEdit (GetDb(), *children.begin());
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
TEST_F (DoubleLShapeProfileTestCase, SetSingleProfile_InValidSingleProfileClass_Fail)
    {
    LShapeProfileCPtr singleProfilePtr = InsertLShapeProfile();

    CreateParams requiredParams (GetModel(), "DoubleL", 1.0, singleProfilePtr->GetElementId());
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    DgnElementIdSet children = singleProfilePtr->QueryChildren();
    ASSERT_EQ (1, children.size());

    DoubleLShapeProfilePtr doubleProfilePtr = DoubleLShapeProfile::GetForEdit (GetDb(), *children.begin());
    ASSERT_TRUE (doubleProfilePtr.IsValid());

    CShapeProfile::CreateParams params (GetModel(), "C", 10.0, 6.0, 1.0, 1.0);
    CShapeProfilePtr cProfilePtr = CShapeProfile::Create (params);

    DgnDbStatus status;
    cProfilePtr->Insert (&status);
    EXPECT_EQ (status, DgnDbStatus::Success);
    ASSERT_EQ (DgnDbStatus::Success, doubleProfilePtr->SetSingleProfile (cProfilePtr->GetElementId()));

    doubleProfilePtr->Update (&status);
    ASSERT_NE (DgnDbStatus::Success, status);

    EXPECT_EQ (1, singleProfilePtr->QueryChildren().size());
    EXPECT_EQ (0, cProfilePtr->QueryChildren().size());
    }
