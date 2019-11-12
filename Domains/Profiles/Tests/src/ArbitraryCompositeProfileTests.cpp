/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct ArbitraryCompositeProfileTestCase : ProfileValidationTestCase<ArbitraryCompositeProfile>
    {
public:
    typedef ArbitraryCompositeProfile::CreateParams CreateParams;

protected:
    ArbitraryCompositeProfileComponent CreateComponent (SinglePerimeterProfile const& profile)
        {
        return ArbitraryCompositeProfileComponent (profile.GetElementId(), DPoint2d::From (0.0, 0.0));
        }

    ArbitraryCompositeProfileComponent CreateDefaultComponent()
        {
        CircleProfilePtr profilePtr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
        BeAssert (profilePtr.IsValid());
        return CreateComponent (*profilePtr);
        }

    bvector<ArbitraryCompositeProfileComponent> DefaultComponentVector()
        {
        return bvector<ArbitraryCompositeProfileComponent> { CreateDefaultComponent(), CreateDefaultComponent() };
        }

    int GetAspectCount (Profile const& profile)
        {
        Utf8CP pSqlString = "SELECT COUNT (*) FROM " PRF_SCHEMA ("ArbitraryCompositeProfileAspect") " WHERE Element.Id=?";
        ECSqlStatement sqlStatement;
        sqlStatement.Prepare (GetDb(), pSqlString);
        sqlStatement.BindId (1, profile.GetElementId());

        if (sqlStatement.Step() != BE_SQLITE_ROW)
            return -1;

        return sqlStatement.GetValueInt (0);
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
    CircleProfilePtr profile1Ptr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
    CircleProfilePtr profile2Ptr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
    ArbitraryCompositeProfileComponent component1 = CreateComponent (*profile1Ptr);
    ArbitraryCompositeProfileComponent component2 = CreateComponent (*profile2Ptr);

    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { component1, component2 });
    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert two components referencing different profiles.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Insert_TwoSameProfileInstances_SuccessfulInsert)
    {
    CircleProfilePtr profilePtr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
    ArbitraryCompositeProfileComponent component1 = CreateComponent (*profilePtr);
    ArbitraryCompositeProfileComponent component2 = CreateComponent (*profilePtr);

    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { component1, component2 });
    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert with two components referencing same profile.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Insert_SingleProfileInstances_FailedInsert)
    {
    CircleProfilePtr profilePtr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
    ArbitraryCompositeProfileComponent component = CreateComponent (*profilePtr);

    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { component });
    EXPECT_FAIL_Insert (params) << "Profile should succeed to insert with two components referencing same profile.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    bvector<ArbitraryCompositeProfileComponent> initialComponents = DefaultComponentVector();
    EXPECT_EQ (-1, initialComponents[0].GetMemberPriority()) << "MemberPriority of a user-constructed ArbitraryCompositeProfileCompoent should be invalid (-1)";

    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { initialComponents });

    ArbitraryCompositeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    bvector<ArbitraryCompositeProfileComponent> components = profilePtr->GetComponents();
    EXPECT_EQ ("Composite", profilePtr->GetName());
    EXPECT_EQ (initialComponents[0].singleProfileId, components[0].singleProfileId);
    EXPECT_EQ (initialComponents[0].offset, components[0].offset);
    EXPECT_EQ (initialComponents[0].mirrorAboutYAxis, components[0].mirrorAboutYAxis);
    EXPECT_EQ (initialComponents[0].rotation.Radians(), components[0].rotation.Radians());
    EXPECT_EQ (0, components[0].GetMemberPriority()) << "MemberPriority of a created profile instance should be valid";

    ArbitraryCompositeProfileCPtr insertedProfilePtr = profilePtr->Insert();
    bvector<ArbitraryCompositeProfileComponent> insertedComponents = insertedProfilePtr->GetComponents();

    EXPECT_EQ (initialComponents[0].singleProfileId, insertedComponents[0].singleProfileId);
    EXPECT_EQ (initialComponents[0].offset, insertedComponents[0].offset);
    EXPECT_EQ (initialComponents[0].mirrorAboutYAxis, insertedComponents[0].mirrorAboutYAxis);
    EXPECT_EQ (initialComponents[0].rotation.Radians(), insertedComponents[0].rotation.Radians());
    EXPECT_EQ (0, insertedComponents[0].GetMemberPriority()) << "MemberPriority of an inserted profile instance should be valid";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, GetProperties_LoadedInstance_ValidProperties)
    {
    bvector<ArbitraryCompositeProfileComponent> initialComponents = DefaultComponentVector();
    CreateParams params (GetModel(), "Composite", initialComponents);

    ArbitraryCompositeProfilePtr profilePtr = InsertElement<ArbitraryCompositeProfile> (params);
    ASSERT_TRUE (profilePtr.IsValid());

    GetDb().Elements().ClearCache();

    ArbitraryCompositeProfileCPtr loadedProfilePtr = ArbitraryCompositeProfile::Get (GetDb(), profilePtr->GetElementId());
    bvector<ArbitraryCompositeProfileComponent> loadedComponents = loadedProfilePtr->GetComponents();

    for (int i = 0; i < 2; ++i)
        {
        EXPECT_EQ (initialComponents[i].singleProfileId, loadedComponents[i].singleProfileId);
        EXPECT_EQ (initialComponents[i].offset, loadedComponents[i].offset);
        EXPECT_EQ (initialComponents[i].rotation.Radians(), loadedComponents[i].rotation.Radians());
        EXPECT_EQ (initialComponents[i].mirrorAboutYAxis, loadedComponents[i].mirrorAboutYAxis);
        EXPECT_EQ (i, loadedComponents[i].GetMemberPriority());
        }
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
    EXPECT_EQ ("Composite", profilePtr->GetName());
    EXPECT_EQ (component.singleProfileId, components[0].singleProfileId);
    EXPECT_EQ (component.offset, components[0].offset);
    EXPECT_EQ (component.mirrorAboutYAxis, components[0].mirrorAboutYAxis);
    EXPECT_EQ (component.rotation.Radians(), components[0].rotation.Radians());
    EXPECT_EQ (0, components[0].GetMemberPriority());
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Insert_QueryAspects_TwoRows)
    {
    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { CreateDefaultComponent(), CreateDefaultComponent() });
    ProfilePtr profilePtr = InsertElement<ArbitraryCompositeProfile> (params);

    ASSERT_EQ (2, GetAspectCount (*profilePtr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Delete_QueryAspects_ZeroRows)
    {
    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { CreateDefaultComponent(), CreateDefaultComponent() });
    ProfilePtr profilePtr = InsertElement<ArbitraryCompositeProfile> (params);

    profilePtr->Delete();
    ASSERT_EQ (0, GetAspectCount (*profilePtr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Update_AddedComponent_AddedAspect)
    {
    RectangleProfilePtr rectanglePtr = InsertElement<RectangleProfile> (RectangleProfile::CreateParams (GetModel(), "r1", 1.0, 1.0));
    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { CreateComponent (*rectanglePtr), CreateComponent (*rectanglePtr) });

    ArbitraryCompositeProfilePtr profilePtr = InsertElement<ArbitraryCompositeProfile> (params);

    ArbitraryCompositeProfile::ComponentVector components = profilePtr->GetComponents();
    components.push_back (CreateComponent (*rectanglePtr));
    profilePtr->SetComponents (components);

    ASSERT_EQ (2, GetAspectCount (*profilePtr));

    profilePtr->Update();
    ASSERT_EQ (3, GetAspectCount (*profilePtr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, Update_RemovedComponent_RemovedAspect)
    {
    RectangleProfilePtr rectanglePtr = InsertElement<RectangleProfile> (RectangleProfile::CreateParams (GetModel(), "r1", 1.0, 1.0));
    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent>
        { CreateComponent (*rectanglePtr), CreateComponent (*rectanglePtr), CreateComponent (*rectanglePtr) });

    ArbitraryCompositeProfilePtr profilePtr = InsertElement<ArbitraryCompositeProfile> (params);

    ArbitraryCompositeProfile::ComponentVector components = profilePtr->GetComponents();
    components.pop_back();

    profilePtr->SetComponents (components);

    ASSERT_EQ (3, GetAspectCount (*profilePtr));

    profilePtr->Update();
    ASSERT_EQ (2, GetAspectCount (*profilePtr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, DeleteReferencedProfile_ExistingCompositeProfile_FailedDelete)
    {
    CircleProfilePtr singleProfilePtr = InsertElement<CircleProfile> (CircleProfile::CreateParams (GetModel(), "c", 1.0));
    bvector<ArbitraryCompositeProfileComponent> components { CreateComponent (*singleProfilePtr), CreateComponent (*singleProfilePtr) };

    CreateParams params (GetModel(), "Composite", components);
    ProfilePtr profilePtr = InsertElement<ArbitraryCompositeProfile> (params);

    ASSERT_EQ (DgnDbStatus::ForeignKeyConstraint, singleProfilePtr->Delete()) << "SingleProfile should fail to delete if it is being referenced by ArbitraryCompositeProfile";
    profilePtr->Delete();
    ASSERT_EQ (DgnDbStatus::Success, singleProfilePtr->Delete()) << "SingleProfile should succeed to delete when it's not referenced by ArbitraryCompositeProfile";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, UpdateReferencedProfile_ExistingCompositeProfile_UpdatedCompositeGeometry)
    {
    RectangleProfilePtr rectanglePtr = InsertElement<RectangleProfile> (RectangleProfile::CreateParams (GetModel(), "r", 1.0, 1.0));

    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { CreateComponent (*rectanglePtr), CreateComponent (*rectanglePtr) });
    ProfilePtr profilePtr = InsertElement<ArbitraryCompositeProfile> (params);
    DgnElementId compositeProfileId = profilePtr->GetElementId();

    DRange3d range;
    ASSERT_TRUE (Profile::Get (GetDb(), compositeProfileId)->GetShape()->TryGetRange (range));
    EXPECT_EQ (1.0, range.XLength());
    EXPECT_EQ (1.0, range.YLength());

    rectanglePtr->SetWidth (2.0);
    rectanglePtr->Update();

    ASSERT_TRUE (Profile::Get (GetDb(), compositeProfileId)->GetShape()->TryGetRange (range));
    EXPECT_EQ (2.0, range.XLength());
    EXPECT_EQ (1.0, range.YLength());

    rectanglePtr->SetDepth (2.0);
    rectanglePtr->Update();

    ASSERT_TRUE (Profile::Get (GetDb(), compositeProfileId)->GetShape()->TryGetRange (range));
    EXPECT_EQ (2.0, range.XLength());
    EXPECT_EQ (2.0, range.YLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ArbitraryCompositeProfileTestCase, UpdateReferencedProfiles_ExistingCompositeProfile_UpdatedCompositeGeometry)
    {
    RectangleProfilePtr rectangle1Ptr = InsertElement<RectangleProfile> (RectangleProfile::CreateParams (GetModel(), "r1", 1.0, 1.0));
    RectangleProfilePtr rectangle2Ptr = InsertElement<RectangleProfile> (RectangleProfile::CreateParams (GetModel(), "r2", 1.0, 1.0));

    CreateParams params (GetModel(), "Composite", bvector<ArbitraryCompositeProfileComponent> { CreateComponent (*rectangle1Ptr), CreateComponent (*rectangle2Ptr) });
    ProfilePtr profilePtr = InsertElement<ArbitraryCompositeProfile> (params);
    DgnElementId compositeProfileId = profilePtr->GetElementId();

    DRange3d range;
    ASSERT_TRUE (Profile::Get (GetDb(), compositeProfileId)->GetShape()->TryGetRange (range));
    EXPECT_EQ (1.0, range.XLength());
    EXPECT_EQ (1.0, range.YLength());

    rectangle1Ptr->SetWidth (2.0);
    rectangle1Ptr->Update();

    ASSERT_TRUE (Profile::Get (GetDb(), compositeProfileId)->GetShape()->TryGetRange (range));
    EXPECT_EQ (2.0, range.XLength());
    EXPECT_EQ (1.0, range.YLength());

    rectangle2Ptr->SetDepth (2.0);
    rectangle2Ptr->Update();

    ASSERT_TRUE (Profile::Get (GetDb(), compositeProfileId)->GetShape()->TryGetRange (range));
    EXPECT_EQ (2.0, range.XLength());
    EXPECT_EQ (2.0, range.YLength());
    }
