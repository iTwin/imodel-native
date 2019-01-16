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
    CompositeProfileComponent CreateComponent (SinglePerimeterProfilePtr const& profilePtr)
        {
        return CompositeProfileComponent (*profilePtr, false, DPoint2d::From (0.0, 0.0), Angle::FromRadians (0.0));
        }

    CompositeProfileComponent CreateComponent (DgnElementId const& profileId)
        {
        return CompositeProfileComponent (profileId, false, DPoint2d::From (0.0, 0.0), Angle::FromRadians (0.0));
        }

    CompositeProfileComponent CreateDefaultComponent()
        {
        CircleProfilePtr profilePtr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
        return CreateComponent (profilePtr);
        }

    bvector<CompositeProfileComponent> DefaultComponentVector()
        {
        return bvector<CompositeProfileComponent> { CreateDefaultComponent(), CreateDefaultComponent() };
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Create_CreateParams_NewInstance)
    {
    CreateParams params (GetModel(), "Composite", bvector<CompositeProfileComponent>());

    ArbitraryCompositeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    SinglePerimeterProfilePtr profile1Ptr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
    SinglePerimeterProfilePtr profile2Ptr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
    CompositeProfileComponent component1 = CreateComponent (profile1Ptr);
    CompositeProfileComponent component2 = CreateComponent (profile2Ptr->GetElementId());

    CreateParams requiredParams (GetModel(), "Composite", bvector<CompositeProfileComponent> {component1, component2});
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CompositeProfileComponent component = CreateDefaultComponent();
    CreateParams params (GetModel(), "Composite", bvector<CompositeProfileComponent> { component });

    ArbitraryCompositeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    bvector<CompositeProfileComponent> components = profilePtr->GetComponents();
    CompositeProfileComponent& firstComponent = components[0];
    EXPECT_EQ ("Composite", profilePtr->GetName());
    EXPECT_EQ (component.singleProfileId, firstComponent.singleProfileId);
    EXPECT_EQ (component.singleProfileId, firstComponent.singleProfilePtr->GetElementId());
    EXPECT_EQ (component.offset, firstComponent.offset);
    EXPECT_EQ (component.mirrorAboutYAxis, firstComponent.mirrorAboutYAxis);
    EXPECT_EQ (component.rotation.Radians(), firstComponent.rotation.Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CompositeProfileComponent component = CreateDefaultComponent();
    CreateParams params (GetModel(), "Composite", bvector<CompositeProfileComponent>());

    ArbitraryCompositeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("Composite");
    profilePtr->SetComponents (bvector<CompositeProfileComponent> { component });

    bvector<CompositeProfileComponent> components = profilePtr->GetComponents();
    CompositeProfileComponent& firstComponent = components[0];
    EXPECT_EQ ("Composite", profilePtr->GetName());
    EXPECT_EQ (component.singleProfileId, firstComponent.singleProfileId);
    EXPECT_EQ (component.singleProfileId, firstComponent.singleProfilePtr->GetElementId());
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
    CompositeProfileComponent component1 (DgnElementId ((uint64_t)0), false, DPoint2d::From (0.0, 0.0), Angle::FromRadians (0.0));
    CompositeProfileComponent component2 (DgnElementId ((uint64_t)0), false, DPoint2d::From (0.0, 0.0), Angle::FromRadians (0.0));

    CreateParams params (GetModel(), "Composite", bvector<CompositeProfileComponent> { component1, component2 });
    EXPECT_FAIL_Insert (params) << "Profile should fail with invalid SinglieProfile id.";
    }
