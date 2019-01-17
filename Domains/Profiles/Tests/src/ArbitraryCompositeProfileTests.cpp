/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/ArbitraryCompositeProfileTests.cpp $
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
struct ArbitraryCompositeProfileTestCase : ProfileValidationTestCase<ArbitraryCompositeProfile>
    {
public:
    typedef ArbitraryCompositeProfile::CreateParams CreateParams;

protected:
    ArbitraryCompositeProfileComponent CreateComponent (DgnElementId const& profileId)
        {
        return ArbitraryCompositeProfileComponent (profileId, DPoint2d::From (0.0, 0.0));
        }

    ArbitraryCompositeProfileComponent CreateDefaultComponent()
        {
        CircleProfilePtr profilePtr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
        return CreateComponent (profilePtr->GetElementId());
        }

    bvector<ArbitraryCompositeProfileComponent> DefaultComponentVector()
        {
        return bvector<ArbitraryCompositeProfileComponent> { CreateDefaultComponent(), CreateDefaultComponent() };
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Create_CreateParams_NewInstance)
    {
    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent>());

    ArbitraryCompositeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Insert_TwoDifferentProfileInstances_SuccessfulInsert)
    {
    ProfilePtr profile1Ptr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
    ProfilePtr profile2Ptr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
    ArbitraryCompositeProfileComponent component1 = CreateComponent (profile1Ptr->GetElementId());
    ArbitraryCompositeProfileComponent component2 = CreateComponent (profile2Ptr->GetElementId());

    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { component1, component2 });
    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert two components referencing different profiles.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Insert_TwoSameProfileInstances_SuccessfulInsert)
    {
    ProfilePtr profilePtr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
    ArbitraryCompositeProfileComponent component1 = CreateComponent (profilePtr->GetElementId());
    ArbitraryCompositeProfileComponent component2 = CreateComponent (profilePtr->GetElementId());

    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { component1, component2 });
    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert with two components referencing same profile.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Insert_SingleProfileInstances_FailedInsert)
    {
    ProfilePtr profilePtr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
    ArbitraryCompositeProfileComponent component = CreateComponent (profilePtr->GetElementId());

    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { component });
    EXPECT_FAIL_Insert (params) << "Profile should succeed to insert with two components referencing same profile.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    ArbitraryCompositeProfileComponent component = CreateDefaultComponent();
    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { component });

    ArbitraryCompositeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    bvector<ArbitraryCompositeProfileComponent> components = profilePtr->GetComponents();
    ArbitraryCompositeProfileComponent& firstComponent = components[0];
    EXPECT_EQ ("Composite", profilePtr->GetName());
    EXPECT_EQ (component.singleProfileId, firstComponent.singleProfileId);
    EXPECT_EQ (component.offset, firstComponent.offset);
    EXPECT_EQ (component.mirrorAboutYAxis, firstComponent.mirrorAboutYAxis);
    EXPECT_EQ (component.rotation.Radians(), firstComponent.rotation.Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    ArbitraryCompositeProfileComponent component = CreateDefaultComponent();
    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent>());

    ArbitraryCompositeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("Composite");
    profilePtr->SetComponents (bvector<ArbitraryCompositeProfileComponent> { component });

    bvector<ArbitraryCompositeProfileComponent> components = profilePtr->GetComponents();
    ArbitraryCompositeProfileComponent& firstComponent = components[0];
    EXPECT_EQ ("Composite", profilePtr->GetName());
    EXPECT_EQ (component.singleProfileId, firstComponent.singleProfileId);
    EXPECT_EQ (component.offset, firstComponent.offset);
    EXPECT_EQ (component.mirrorAboutYAxis, firstComponent.mirrorAboutYAxis);
    EXPECT_EQ (component.rotation.Radians(), firstComponent.rotation.Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), "Composite", DefaultComponentVector());

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "Composite", DefaultComponentVector());

    params.name = "Composite";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Insert_InvalidSingleProfileId_FailedInsert)
    {
    ArbitraryCompositeProfileComponent component1 (DgnElementId ((uint64_t)0), DPoint2d::From (0.0, 0.0));
    ArbitraryCompositeProfileComponent component2 (DgnElementId ((uint64_t)0), DPoint2d::From (0.0, 0.0));

    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { component1, component2 });
    EXPECT_FAIL_Insert (params) << "Profile should fail with invalid SinglieProfile id.";
    }
