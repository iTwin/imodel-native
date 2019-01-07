/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/MaterialProfileTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesTestCase.h"
#include "TestHost.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

#define EXPECT_SUCCESS_Insert(createParams) EXPECT_EQ (DgnDbStatus::Success, Insert (createParams))
#define EXPECT_FAIL_Insert(createParams) EXPECT_EQ (DgnDbStatus::ValidationFailed, Insert (createParams))

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct MaterialProfileTestCase : ProfilesTestCase
    {
public:
    typedef MaterialProfile::CreateParams CreateParams;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    Dgn::DgnDbStatus Insert (CreateParams const& createParams)
        {
        MaterialProfilePtr materialProfilePtr = MaterialProfile::Create (createParams);
        BeAssert (materialProfilePtr.IsValid());

        Dgn::DgnDbStatus status;
        materialProfilePtr->Insert (&status);
        if (status != Dgn::DgnDbStatus::Success)
            return status;

        // Perform an Update just to double check same validation is happenning on update.
        materialProfilePtr->Update (&status);
        return status;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MaterialProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel());
    MaterialProfilePtr materialProfilePtr = MaterialProfile::Create (params);
    EXPECT_TRUE(materialProfilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MaterialProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams params(GetModel(), CreateAndGetProfileId(), CreateAndGetMaterialId("0"));
    MaterialProfilePtr materialProfilePtr = MaterialProfile::Create(params);
    EXPECT_SUCCESS_Insert(params) << "MaterialProfile should succeed to insert with valid create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MaterialProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel());
    EXPECT_FAIL_Insert (params) << "MaterialProfile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MaterialProfileTestCase, Insert_InvalidCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), CreateAndGetMaterialId("0"), CreateAndGetProfileId());
    EXPECT_FAIL_Insert (params) << "MaterialProfile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MaterialProfileTestCase, GetProperties_MaterialProfileInstance_ValidProperties)
    {
    DgnElementId profileId = CreateAndGetProfileId();
    DgnElementId materialId = CreateAndGetMaterialId("0");

    CreateParams params (GetModel(), profileId, materialId);
    MaterialProfilePtr materialProfilePtr = MaterialProfile::Create(params);

    EXPECT_EQ (profileId, materialProfilePtr->GetProfile()->GetElementId());
    EXPECT_EQ (materialId, materialProfilePtr->GetMaterial()->GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MaterialProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    DgnElementId profileId = CreateAndGetProfileId();
    DgnElementId materialId = CreateAndGetMaterialId("0");

    CreateParams params (GetModel());
    MaterialProfilePtr materialProfilePtr = MaterialProfile::Create(params);

    materialProfilePtr->SetProfile(profileId);
    materialProfilePtr->SetMaterial(materialId);

    EXPECT_EQ (profileId, materialProfilePtr->GetProfile()->GetElementId());
    EXPECT_EQ (materialId, materialProfilePtr->GetMaterial()->GetElementId());
    }