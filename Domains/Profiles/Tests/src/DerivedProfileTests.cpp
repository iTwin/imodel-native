/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/DerivedProfileTests.cpp $
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
struct DerivedProfileTestCase : ProfileValidationTestCase<DerivedProfile>
    {
public:
    typedef DerivedProfile::CreateParams CreateParams;

    CircleProfilePtr InsertBaseProfile()
        {
        return InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "circle", 1.0));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "Derived", *InsertBaseProfile());

    DerivedProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "Derived", *InsertBaseProfile(),
        DPoint2d::From (0.0, 0.0), DPoint2d::From (2.0, 2.0), Angle::FromRadians (PI), true);

    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert with valid create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, Insert_BadBaseProfileId_FailedInsert)
    {
    CreateParams params (GetModel(), "Derived", DgnElementId());

    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    ProfilePtr baseProfilePtr = InsertBaseProfile();
    CreateParams params (GetModel(), "Derived", baseProfilePtr->GetElementId(),
        DPoint2d::From (1.0, 2.0), DPoint2d::From (3.0, 4.0), Angle::FromRadians (5.0), true);

    DerivedProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("Derived", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetOffset().x);
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetOffset().y);
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetScale().x);
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetScale().y);
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetRotation().Radians());
    EXPECT_EQ (true, profilePtr->GetMirrorAboutYAxis());
    EXPECT_EQ (baseProfilePtr->GetElementId(), profilePtr->GetBaseProfile()->GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    ProfilePtr baseProfilePtr = InsertBaseProfile();
    CreateParams params (GetModel(), "", baseProfilePtr->GetElementId(),
        DPoint2d::From (0.0, 0.0), DPoint2d::From (0.0, 0.0), Angle::FromRadians (0.0), false);

    DerivedProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("Derived");
    profilePtr->SetOffset (DPoint2d::From (1.0, 2.0));
    profilePtr->SetScale (DPoint2d::From (3.0, 4.0));
    profilePtr->SetRotation (Angle::FromRadians (5.0));
    profilePtr->SetMirrorAboutYAxis (true);

    EXPECT_EQ ("Derived", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetOffset().x);
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetOffset().y);
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetScale().x);
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetScale().y);
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetRotation().Radians());
    EXPECT_EQ (true, profilePtr->GetMirrorAboutYAxis());
    EXPECT_EQ (baseProfilePtr->GetElementId(), profilePtr->GetBaseProfile()->GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), nullptr, *InsertBaseProfile());

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), nullptr, *InsertBaseProfile());

    params.name = "Derived";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, Insert_InvalidCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "Derived", *InsertBaseProfile());

    EXPECT_SUCCESS_Insert (params);
    params.offset.x = std::numeric_limits<double>::signaling_NaN();
    EXPECT_FAIL_Insert (params);

    params.offset.x = 0.0;
    EXPECT_SUCCESS_Insert (params);
    params.offset.y = std::numeric_limits<double>::signaling_NaN();
    EXPECT_FAIL_Insert (params);

    params.offset.y = 0.0;
    EXPECT_SUCCESS_Insert (params);
    params.scale.x = std::numeric_limits<double>::signaling_NaN();
    EXPECT_FAIL_Insert (params);

    params.scale.x = 0.0;
    EXPECT_SUCCESS_Insert (params);
    params.scale.y = std::numeric_limits<double>::signaling_NaN();
    EXPECT_FAIL_Insert (params);

    params.scale.y = 0.0;
    EXPECT_SUCCESS_Insert (params);
    params.rotation = Angle::FromRadians (std::numeric_limits<double>::signaling_NaN());
    EXPECT_FAIL_Insert (params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, DeleteBaseProfile_ExistingDerivedProfile_FailedDelete)
    {
    ProfilePtr baseProfilePtr = InsertBaseProfile();
    CreateParams params (GetModel(), "Derived", baseProfilePtr->GetElementId());

    DerivedProfilePtr profilePtr = InsertElement<DerivedProfile> (params);

    ASSERT_EQ (DgnDbStatus::ForeignKeyConstraint, baseProfilePtr->Delete()) << "BaseProfile should fail to delete if it is being referenced by DerivedProfile";
    profilePtr->Delete();
    ASSERT_EQ (DgnDbStatus::Success, baseProfilePtr->Delete()) << "BaseProfile should succeed to delete when it's not referenced by DerivedProfile";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, DeleteDerivedProfile_ExistingDerivedProfile_FailedDelete)
    {
    ProfilePtr baseProfilePtr = InsertBaseProfile();

    CreateParams derivedParams (GetModel(), "Derived", baseProfilePtr->GetElementId());
    DerivedProfilePtr derivedPtr = InsertElement<DerivedProfile> (derivedParams);
    ASSERT_TRUE (derivedPtr.IsValid());

    CreateParams derivedDerivedParams (GetModel(), "Derived of Derived", derivedPtr->GetElementId());
    DerivedProfilePtr derivedDerivedPtr = InsertElement<DerivedProfile> (derivedDerivedParams);
    ASSERT_TRUE (derivedDerivedPtr.IsValid());

    ASSERT_EQ (DgnDbStatus::ForeignKeyConstraint, baseProfilePtr->Delete()) << "base is referenced by derived";
    ASSERT_EQ (DgnDbStatus::ForeignKeyConstraint, derivedPtr->Delete()) << "derived is referenced by derivedDerived";
    ASSERT_EQ (DgnDbStatus::Success, derivedDerivedPtr->Delete()) << "derivedDerived is not referenced";
    ASSERT_EQ (DgnDbStatus::ForeignKeyConstraint, baseProfilePtr->Delete()) << "base is referenced by derived";
    ASSERT_EQ (DgnDbStatus::Success, derivedPtr->Delete()) << "derived is no longer referenced";
    ASSERT_EQ (DgnDbStatus::Success, baseProfilePtr->Delete()) << "base is no longer referenced";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, UpdateBaseProfile_ExistingDerivedProfile_UpdatedDerivedGeometry)
    {
    RectangleProfilePtr rectanglePtr = InsertElement<RectangleProfile> (RectangleProfile::CreateParams (GetModel(), "Rectangle", 1.0, 1.0));
    CreateParams params (GetModel(), "Derived", *rectanglePtr);

    DerivedProfilePtr profilePtr = InsertElement<DerivedProfile> (params);
    DgnElementId derivedProfileId = profilePtr->GetElementId();

    DRange3d range;
    ASSERT_TRUE (Profile::Get (GetDb(), derivedProfileId)->GetShape()->TryGetRange (range));
    EXPECT_EQ (1.0, range.XLength());
    EXPECT_EQ (1.0, range.YLength());

    rectanglePtr->SetWidth (2.0);
    rectanglePtr->SetDepth (3.0);
    rectanglePtr->Update();

    ASSERT_TRUE (Profile::Get (GetDb(), derivedProfileId)->GetShape()->TryGetRange (range));
    EXPECT_EQ (2.0, range.XLength());
    EXPECT_EQ (3.0, range.YLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, Insert_InvalidBaseProfileClass_FailedInsert)
    {
    CShapeProfile::CreateParams singlePerimeterParams (GetModel(), "C", 10.0, 6.0, 1.0, 1.0);
    CShapeProfilePtr singleProfilePtr = CShapeProfile::Create (singlePerimeterParams);

    DgnDbStatus status;
    singleProfilePtr->Insert (&status);
    BeAssert (status == DgnDbStatus::Success);

    DoubleCShapeProfile::CreateParams requiredParams (GetModel(), "DoubleC", 1.0, singleProfilePtr->GetElementId());
    DoubleCShapeProfilePtr baseProfilePtr = InsertElement<DoubleCShapeProfile> (requiredParams, &status);
    ASSERT_EQ (status, Dgn::DgnDbStatus::Success);

    CreateParams params (GetModel(), "Derived", baseProfilePtr->GetElementId());
    EXPECT_FAIL_Insert (params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, SetSingleProfile_ValidSinglePerimeterProfileId_Success)
    {
    SinglePerimeterProfilePtr singleProfilePtr = InsertBaseProfile();

    CreateParams requiredParams (GetModel(), "Derived", singleProfilePtr->GetElementId());
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    DgnElementIdSet children = singleProfilePtr->QueryChildren();
    ASSERT_EQ (1, children.size());

    DerivedProfilePtr derivedProfilePtr = DerivedProfile::GetForEdit (GetDb(), *children.begin());
    ASSERT_TRUE (derivedProfilePtr.IsValid());

    SinglePerimeterProfilePtr otherSingleProfilePtr = InsertBaseProfile();
    ASSERT_EQ (DgnDbStatus::Success, derivedProfilePtr->SetBaseProfile (otherSingleProfilePtr->GetElementId()));

    DgnDbStatus status;
    derivedProfilePtr->Update (&status);
    ASSERT_EQ (DgnDbStatus::Success, status);

    EXPECT_EQ (0, singleProfilePtr->QueryChildren().size());
    EXPECT_EQ (1, otherSingleProfilePtr->QueryChildren().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, SetSingleProfile_InValidBaseProfileId_Fail)
    {
    SinglePerimeterProfilePtr singleProfilePtr = InsertBaseProfile();

    CreateParams requiredParams (GetModel(), "Derived", singleProfilePtr->GetElementId());
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    DgnElementIdSet children = singleProfilePtr->QueryChildren();
    ASSERT_EQ (1, children.size());

    DerivedProfilePtr derivedProfilePtr = DerivedProfile::GetForEdit (GetDb(), *children.begin());
    ASSERT_TRUE (derivedProfilePtr.IsValid());

    ASSERT_EQ (DgnDbStatus::Success, derivedProfilePtr->SetBaseProfile (DgnElementId()));

    DgnDbStatus status;
    derivedProfilePtr->Update (&status);
    ASSERT_NE (DgnDbStatus::Success, status);

    EXPECT_EQ (1, singleProfilePtr->QueryChildren().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DerivedProfileTestCase, SetSingleProfile_InValidBaseProfileClass_Fail)
    {
    SinglePerimeterProfilePtr singleProfilePtr = InsertBaseProfile();

    CreateParams requiredParams (GetModel(), "Derived", singleProfilePtr->GetElementId());
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    DgnElementIdSet children = singleProfilePtr->QueryChildren();
    ASSERT_EQ (1, children.size());

    DerivedProfilePtr derivedProfilePtr = DerivedProfile::GetForEdit (GetDb(), *children.begin());
    ASSERT_TRUE (derivedProfilePtr.IsValid());

    CShapeProfile::CreateParams singlePerimeterParams (GetModel(), "C", 10.0, 6.0, 1.0, 1.0);
    CShapeProfilePtr singleCProfilePtr = CShapeProfile::Create (singlePerimeterParams);

    DgnDbStatus status;
    singleCProfilePtr->Insert (&status);
    ASSERT_EQ (status, DgnDbStatus::Success);

    DoubleCShapeProfile::CreateParams doubleCParams (GetModel(), "DoubleC", 1.0, singleCProfilePtr->GetElementId());
    DoubleCShapeProfilePtr baseProfilePtr = InsertElement<DoubleCShapeProfile> (doubleCParams, &status);
    ASSERT_EQ (status, Dgn::DgnDbStatus::Success);
    ASSERT_EQ (DgnDbStatus::Success, derivedProfilePtr->SetBaseProfile (baseProfilePtr->GetElementId()));

    derivedProfilePtr->Update (&status);
    ASSERT_NE (DgnDbStatus::Success, status);

    EXPECT_EQ (1, singleProfilePtr->QueryChildren().size());
    EXPECT_EQ (0, baseProfilePtr->QueryChildren().size());
    }

