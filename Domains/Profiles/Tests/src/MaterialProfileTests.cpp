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

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct MaterialProfileTestCase : ProfilesTestCase
    {
public:
    typedef MaterialProfile::CreateParams CreateParams;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     01/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    Dgn::DgnDbStatus InsertAndUpdateElement (CreateParams const& createParams)
        {
        return ProfilesTestCase::InsertAndUpdateElement<MaterialProfile> (createParams);
        }

    /*---------------------------------------------------------------------------------**//**
    * Create and insert a TEMPORARY Material element.
    * @bsimethod                                                                     01/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    Dgn::DgnElementId CreateAndGetMaterialId (Utf8CP pCodeValue)
        {
        Dgn::DgnDb& db = GetDb();

        DgnClassId classId (db.Schemas().GetClassId (PRF_SCHEMA_NAME, "TempMaterial"));
        dgn_ElementHandler::Element* pHandler = Dgn::dgn_ElementHandler::Element::FindHandler (db, classId);

        DgnElementPtr elementPtr = pHandler->Create (DgnElement::CreateParams (db, GetModel().GetModelId(), classId));
        elementPtr->SetCode (CodeSpec::CreateCode (db, "ProfilesMaterialCodeSpec", pCodeValue));

        Dgn::DgnDbStatus status;
        elementPtr->Insert (&status);
        if (status != Dgn::DgnDbStatus::Success)
            {
            BeAssert (false && "Failed to insert TempMaterial");
            return DgnElementId();
            }

        return elementPtr->GetElementId();
        }

    /*---------------------------------------------------------------------------------**//**
    * Create and insert a generic profile instnace, used to test MaterialProfile.
    * @bsimethod                                                                     01/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    Dgn::DgnElementId CreateAndGetProfileId()
        {
        CShapeProfile::CreateParams params (GetModel(), "C", 10, 10, 1, 1);
        CShapeProfilePtr profilePtr = InsertElement<CShapeProfile> (params);

        return profilePtr->GetElementId();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                      01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MaterialProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel());
    MaterialProfilePtr materialProfilePtr = MaterialProfile::Create (params);
    EXPECT_TRUE(materialProfilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                      01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MaterialProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams params(GetModel(), CreateAndGetProfileId(), CreateAndGetMaterialId("0"));
    MaterialProfilePtr materialProfilePtr = MaterialProfile::Create(params);
    EXPECT_SUCCESS_Insert(params) << "MaterialProfile should succeed to insert with valid create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MaterialProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel());
    EXPECT_FAIL_Insert (params) << "MaterialProfile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (MaterialProfileTestCase, Insert_InvalidCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), CreateAndGetMaterialId("0"), CreateAndGetProfileId());
    EXPECT_FAIL_Insert (params) << "MaterialProfile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
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
* @bsimethod                                                                     01/2019
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