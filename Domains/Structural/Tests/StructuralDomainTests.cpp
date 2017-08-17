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

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_Beam, *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, BEAM_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));
    
    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = physicalElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, BEAM_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define WALL_CODE_VALUE       "WALL-001"

TEST_F(StructuralDomainTestFixture, WallClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_Wall, *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, WALL_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = physicalElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, WALL_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define BRACE_CODE_VALUE       "BRACE-001"

TEST_F(StructuralDomainTestFixture, BraceClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_Brace, *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, BRACE_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = physicalElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, BRACE_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define COLUMN_CODE_VALUE       "COLUMN-001"

TEST_F(StructuralDomainTestFixture, ColumnClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_Column, *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, COLUMN_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = physicalElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, COLUMN_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define CURVEMEMBER_CODE_VALUE       "CURVEMEMBER-001"

TEST_F(StructuralDomainTestFixture, CurveMemberClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_CurveMember, *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, CURVEMEMBER_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = physicalElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, CURVEMEMBER_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define FOUNDATIONMEMBER_CODE_VALUE       "FOUNDATIONMEMBER-001"

TEST_F(StructuralDomainTestFixture, FoundationMemberClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_FoundationMember, *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, FOUNDATIONMEMBER_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = physicalElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, FOUNDATIONMEMBER_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define SLAB_CODE_VALUE       "SLAB-001"

TEST_F(StructuralDomainTestFixture, SlabClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_Slab, *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, SLAB_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = physicalElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, SLAB_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define STRIFOOTING_CODE_VALUE       "STRIPFOOTING-001"

TEST_F(StructuralDomainTestFixture, StripFootingClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_StripFooting, *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, STRIFOOTING_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = physicalElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, STRIFOOTING_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define SPREADFOOTING_CODE_VALUE       "SPREADFOOTING-001"

TEST_F(StructuralDomainTestFixture, SpreadFootingClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_SpreadFooting, *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, SPREADFOOTING_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = physicalElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, SPREADFOOTING_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define STRUCTURALELEMENT_CODE_VALUE       "STRUCTURALELEMENT-001"

TEST_F(StructuralDomainTestFixture, StructuralEmenentFootingClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_StructuralElement, *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, STRUCTURALELEMENT_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));
    //ECClass 'StructuralEmenent' is an abstract class which is not instantiable and therefore cannot be used in an ECSQL INSERT statement.
    //so test finished for now
    }

#define STRUCTURALMEMBER_CODE_VALUE       "STRUCTURALMEMBER-001"

TEST_F(StructuralDomainTestFixture, StructuralMemberFootingClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_StructuralMember, *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, STRUCTURALMEMBER_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));
    //ECClass 'StructuralMember' is an abstract class which is not instantiable and therefore cannot be used in an ECSQL INSERT statement.
    //so test finished for now
    }

#define SURFACEMEMBER_CODE_VALUE       "SURFACEMEMBER-001"

TEST_F(StructuralDomainTestFixture, SurfaceMemberClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_SurfaceMember, *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, SURFACEMEMBER_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = physicalElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, SURFACEMEMBER_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define BUILTUPPROFILE_CODE_VALUE       "BUILTUPPROFILE-001"

TEST_F(StructuralDomainTestFixture, BuiltUpProfileClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralTypeDefinitionModelCPtr definitionModel = Structural::StructuralDomainUtilities::GetStructuralTypeDefinitionModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(definitionModel.IsValid());

    Dgn::DefinitionElementPtr definitionElement = Structural::StructuralDomainUtilities::CreateDefinitionElement(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_BuiltUpProfile, *definitionModel);
    ASSERT_TRUE(definitionElement.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, BUILTUPPROFILE_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == definitionElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = definitionElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DefinitionElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::DefinitionElement>(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, BUILTUPPROFILE_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define PROFILE_CODE_VALUE       "PROFILE-001"

TEST_F(StructuralDomainTestFixture, ProfileClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralTypeDefinitionModelCPtr definitionModel = Structural::StructuralDomainUtilities::GetStructuralTypeDefinitionModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(definitionModel.IsValid());

    Dgn::DefinitionElementPtr definitionElement = Structural::StructuralDomainUtilities::CreateDefinitionElement(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_Profile, *definitionModel);
    ASSERT_TRUE(definitionElement.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, PROFILE_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == definitionElement->SetCode(code));
    }

#define CONSTANTPROFILE_CODE_VALUE       "CONSTANTPROFILE-001"

TEST_F(StructuralDomainTestFixture, ConstantProfileClassTests)
{
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralTypeDefinitionModelCPtr definitionModel = Structural::StructuralDomainUtilities::GetStructuralTypeDefinitionModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(definitionModel.IsValid());

    Dgn::DefinitionElementPtr definitionElement = Structural::StructuralDomainUtilities::CreateDefinitionElement(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_ConstantProfile, *definitionModel);
    ASSERT_TRUE(definitionElement.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, CONSTANTPROFILE_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == definitionElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = definitionElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DefinitionElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::DefinitionElement>(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, CONSTANTPROFILE_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
}

#define PARAMETRICPROFILE_CODE_VALUE       "PARAMETRICPROFILE-001"

TEST_F(StructuralDomainTestFixture, ParametricProfileClassTests)
{
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralTypeDefinitionModelCPtr definitionModel = Structural::StructuralDomainUtilities::GetStructuralTypeDefinitionModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(definitionModel.IsValid());

    Dgn::DefinitionElementPtr definitionElement = Structural::StructuralDomainUtilities::CreateDefinitionElement(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_ParametricProfile, *definitionModel);
    ASSERT_TRUE(definitionElement.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, PARAMETRICPROFILE_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == definitionElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = definitionElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DefinitionElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::DefinitionElement>(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, PARAMETRICPROFILE_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
}

#define PUBLISHEDPROFILE_CODE_VALUE       "PUBLISHEDPROFILE-001"

TEST_F(StructuralDomainTestFixture, PublishedProfileClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralTypeDefinitionModelCPtr definitionModel = Structural::StructuralDomainUtilities::GetStructuralTypeDefinitionModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(definitionModel.IsValid());

    Dgn::DefinitionElementPtr definitionElement = Structural::StructuralDomainUtilities::CreateDefinitionElement(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_PublishedProfile, *definitionModel);
    ASSERT_TRUE(definitionElement.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, PUBLISHEDPROFILE_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == definitionElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = definitionElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DefinitionElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::DefinitionElement>(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, PUBLISHEDPROFILE_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define VARYINGPROFILE_CODE_VALUE       "VARYINGPROFILE-001"

TEST_F(StructuralDomainTestFixture, VaryingProfileClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralTypeDefinitionModelCPtr definitionModel = Structural::StructuralDomainUtilities::GetStructuralTypeDefinitionModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(definitionModel.IsValid());

    Dgn::DefinitionElementPtr definitionElement = Structural::StructuralDomainUtilities::CreateDefinitionElement(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_VaryingProfile, *definitionModel);
    ASSERT_TRUE(definitionElement.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, VARYINGPROFILE_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == definitionElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = definitionElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DefinitionElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::DefinitionElement>(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, VARYINGPROFILE_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define VARYINGPROFILEZONE_CODE_VALUE       "VARYINGPROFILEZONE-001"

TEST_F(StructuralDomainTestFixture, VaryingProfileZoneClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralTypeDefinitionModelCPtr definitionModel = Structural::StructuralDomainUtilities::GetStructuralTypeDefinitionModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(definitionModel.IsValid());

    Dgn::DefinitionElementPtr definitionElement = Structural::StructuralDomainUtilities::CreateDefinitionElement(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_VaryingProfileZone, *definitionModel);
    ASSERT_TRUE(definitionElement.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, VARYINGPROFILEZONE_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == definitionElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = definitionElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DefinitionElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::DefinitionElement>(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, VARYINGPROFILEZONE_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

TEST_F(StructuralDomainTestFixture, BuiltupProfileComponentClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    ECN::ECClassCP  aspectClass = db->Schemas().GetClass(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_BuiltUpProfileComponent);
    ECN::StandaloneECEnablerPtr aspectEnabler = aspectClass->GetDefaultStandaloneEnabler();

    ASSERT_TRUE(aspectEnabler.IsValid());

    ECN::StandaloneECInstancePtr p = aspectEnabler->CreateInstance();
    }

/*TEST_F(StructuralDomainTestFixture, VaryingProfileByZoneClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    ECN::ECClassCP  aspectClass = db->Schemas().GetClass(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_VaryingProfileByZone);
    ECN::StandaloneECEnablerPtr aspectEnabler = aspectClass->GetDefaultStandaloneEnabler();

    ASSERT_TRUE(aspectEnabler.IsValid());

    ECN::StandaloneECInstancePtr p = aspectEnabler->CreateInstance();
    }*/