/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/StructuralDomainTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StructuralDomainTestFixture.h"
#include <StructuralDomain/StructuralDomainApi.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Json/Json.h>


#define MODEL_TEST_NAME              "SampleModel"
#define MODEL_TEST_NAME1             "SampleModelAAA"
#define MODEL_TEST_NAME2             "SampleModelBBB"
#define MODEL_TEST_NAME3             "SampleModelCCC"
#define DYNAMIC_SCHEMA_NAME          "SampleDynamic" 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StructuralDomainTestFixture, EnsureDomainsAreRegistered)
    {
    ASSERT_TRUE(true);
    DgnDbPtr db = CreateDgnDb();

    ASSERT_TRUE(db.IsValid());

    // //This should create a DGN db with building domain.
    Structural::StructuralDomainUtilities::RegisterDomainHandlers();

    DgnDomainCP structuralCommon = db->Domains().FindDomain(Structural::StructuralCommonDomain::GetDomain().GetDomainName());
    ASSERT_TRUE(NULL != structuralCommon);

    DgnDomainCP structuralPhysical = db->Domains().FindDomain(Structural::StructuralPhysicalDomain::GetDomain().GetDomainName());
    ASSERT_TRUE(NULL != structuralPhysical);

    DgnDomainCP structuralProfiles = db->Domains().FindDomain(Structural::StructuralProfilesDomain::GetDomain().GetDomainName());
    ASSERT_TRUE(NULL != structuralProfiles);
    }

BE_JSON_NAME(StructuralDomain)

TEST_F(StructuralDomainTestFixture, CreatePhysicalPartition)
    {
    DgnDbPtr db = CreateDgnDb();

    ASSERT_TRUE(db.IsValid());

    // Testing the minimal arguments for CreateBuildingModels
    ASSERT_FALSE(Structural::StructuralDomainUtilities::CreateStructuralModels(MODEL_TEST_NAME, *db));

    Structural::StructuralPhysicalModelCPtr physModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel (MODEL_TEST_NAME, *db);

    ASSERT_TRUE(physModel.IsValid());

    // Testing the minimal arguments for CreateBuildingModels with supplied subject

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::SubjectPtr subject = Dgn::Subject::Create(*parentSubject, MODEL_TEST_NAME1);

    ASSERT_TRUE(subject.IsValid());

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr element = subject->Insert(&status);

    ASSERT_TRUE(element.IsValid() && Dgn::DgnDbStatus::Success == status);

    ASSERT_FALSE(Structural::StructuralDomainUtilities::CreateStructuralModels(MODEL_TEST_NAME1, *db, subject));

    physModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME1, *db);


    ASSERT_FALSE(Structural::StructuralDomainUtilities::CreateStructuralModels(MODEL_TEST_NAME2, *db, nullptr, false));

    physModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME2, *db);

    ECN::ECSchemaCP schema = Structural::StructuralDomainUtilities::GetStructuralDynamicSchema(physModel);

    ASSERT_TRUE(schema == nullptr);

    // Test the creation with a supplied schema

    ECN::ECSchemaPtr dynSchema;

    ASSERT_TRUE(ECN::ECObjectsStatus::Success == ECN::ECSchema::CreateSchema(dynSchema, DYNAMIC_SCHEMA_NAME, "DYN", 1, 1, 0));

    ECN::ECSchemaCP bisSchema = db->Schemas().GetSchema(BIS_ECSCHEMA_NAME);

    ASSERT_FALSE(nullptr == bisSchema);

    ASSERT_TRUE(ECN::ECObjectsStatus::Success == dynSchema->AddReferencedSchema((ECN::ECSchemaR)(*bisSchema)));

    ASSERT_FALSE(Structural::StructuralDomainUtilities::CreateStructuralModels(MODEL_TEST_NAME3, *db, nullptr, false, dynSchema));

    physModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME3, *db);

    schema = Structural::StructuralDomainUtilities::GetStructuralDynamicSchema(physModel);

    ASSERT_TRUE(schema != nullptr);

    ASSERT_TRUE(schema->GetName() == DYNAMIC_SCHEMA_NAME);
}

#define BEAM_CODE_VALUE       "BEAM-001"

TEST_F(StructuralDomainTestFixture, BeamClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr beam = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_Beam, *physicalModel);
    ASSERT_TRUE(beam.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, BEAM_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == beam->SetCode(code));
    
    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = beam->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedBeam = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, BEAM_CODE_VALUE);
    ASSERT_TRUE(queriedBeam.IsValid());
    }

