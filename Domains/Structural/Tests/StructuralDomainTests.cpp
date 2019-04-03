/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/StructuralDomainTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#ifdef NO_DGNVIEW_IMODEL02

#include "StructuralDomainTestFixture.h"
#include <StructuralDomain/StructuralDomainApi.h>
// #include <FormsDomain/FormsDomainApi.h>
// #include <ProfilesDomain/ProfilesDomainApi.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <BeSQLite/BeSQLite.h>
#include <Json/Json.h>
#include <Bentley/BeAssert.h>
#include <DgnPlatform/DgnCoreAPI.h>


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
    //Structural::StructuralDomainUtilities::RegisterDomainHandlers();

    DgnDomainCP structuralPhysical = db->Domains().FindDomain(Structural::StructuralPhysicalDomain::GetDomain().GetDomainName());
    ASSERT_TRUE(NULL != structuralPhysical);

    // DgnDomainCP formDomain = db->Domains().FindDomain(Forms::FormsDomain::GetDomain().GetDomainName());
    // ASSERT_TRUE(NULL != formDomain);
    }

BE_JSON_NAME(StructuralDomain)


TEST_F(StructuralDomainTestFixture, ValidateSchema)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext(true, true);
    context->AddSchemaLocater((*db).GetSchemaLocater());

    ECN::SchemaKey refKey = ECN::SchemaKey(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, 1, 0);

    ECN::ECSchemaPtr refSchema = context->LocateSchema(refKey, ECN::SchemaMatchType::LatestWriteCompatible);
    ASSERT_TRUE(refSchema.IsValid());

    ECN::ECSchemaValidator validator;
    ASSERT_TRUE(validator.Validate(*refSchema));
    }


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

#define STRUCTURALMEMBER_CODE_VALUE1       "STRUCTURAL_MEMBER-001"
TEST_F(StructuralDomainTestFixture, StructuralMemberClassTest_1)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_StructuralMember, *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, STRUCTURALMEMBER_CODE_VALUE1);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = physicalElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, STRUCTURALMEMBER_CODE_VALUE1);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define STRUCTURALMEMBER_CODE_VALUE2      "STRUCTURAL_MEMBER-002"
TEST_F(StructuralDomainTestFixture, StructuralMemberClassTest_2)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());
    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, STRUCTURALMEMBER_CODE_VALUE2);

    Dgn::DgnDbStatus status;
    StructuralMemberPtr pw = StructuralMember::Create(*physicalModel);
    status = pw->SetCode(code);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
    pw->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, STRUCTURALMEMBER_CODE_VALUE2);
    ASSERT_TRUE(queriedElement.IsValid());
    }

/*#define BEAM_CODE_VALUE       "BEAM-001"

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


    bool bTrue = true;
    Json::Value jsonObj1 = physicalElement->GetJsonProperties("TEST1");
    jsonObj1["Is it true?"] = bTrue;
    physicalElement->SetJsonProperties("TEST1", jsonObj1);

    Json::Value jsonObj2 = physicalElement->GetJsonProperties("TEST2");
    jsonObj2["Wall Thickness"] = 0.1457824578;
    physicalElement->SetJsonProperties("TEST2", jsonObj2);

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = physicalElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, WALL_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define WALL_CODE_VALUE2       "WALL2-001"

const double COLUMN_WIDTH = 12.0;
const double COLUMN_DEPTH = 12.0;
const double COLUMN_HEIGHT = 120.0;
const double COLUMN_OFFSET = COLUMN_WIDTH / 2.0;
const double WALL_WIDTH = 216.0;
const double WALL_DEPTH = 120.0;
const double WALL_THICKNESS = 8.0;
const double WALL_OFFSET = (COLUMN_DEPTH - WALL_THICKNESS) / 2.0;

TEST_F(StructuralDomainTestFixture, WallClassTests2)
    {
    DgnDbPtr db = OpenDgnDb();
    Dgn::DgnDbStatus status;

    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, WALL_CODE_VALUE2);
    WallPtr pw = Wall::Create(physicalModel);
    status = pw->SetCode(code);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    //test geometry
    Dgn::Placement3d placement;
    placement.GetOriginR() = DPoint3d::From(10.0, 10.0, -50.0);
    pw->SetPlacement(placement);

    DPoint3d blockSize = DPoint3d::From(25.0, 10.0, 0.5);
    DPoint3d blockCenter = DPoint3d::From(blockSize.x / 2.0, blockSize.y / 2.0, blockSize.z / 2.0);
    DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(blockCenter, blockSize, true);

    ISolidPrimitivePtr blockGeom = ISolidPrimitive::CreateDgnBox(blockDetail);

    ASSERT_TRUE(blockGeom.IsValid());

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*pw->ToGeometrySourceP());
    
    GeometrySourceP geomElem = pw->ToGeometrySourceP();
    geomElem->SetCategoryId(pw->GetCategoryId());

    builder->Append(pw->GetCategoryId(), GeometryBuilder::CoordSystem::World);

    Dgn::Render::GeometryParams params;

    DgnCategoryId c = pw->GetCategoryId();

    params.SetCategoryId(pw->GetCategoryId());
    params.SetLineColor(ColorDef::Red());
    params.SetWeight(2);
    builder->Append(params, GeometryBuilder::CoordSystem::World);

    builder->Append(*blockGeom, GeometryBuilder::CoordSystem::World);
    builder->Finish(*pw->ToGeometrySourceP());

    //end test geometry

    pw->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    bool bTrue = true;
    Json::Value jsonObj1 = pw->GetJsonProperties("TEST1");
    jsonObj1["Is it true?"] = bTrue;
    pw->SetJsonProperties("TEST1", jsonObj1);

    Json::Value jsonObj2 = pw->GetJsonProperties("TEST2");
    jsonObj2["Wall Thickness"] = 0.1457824578;
    pw->SetJsonProperties("TEST2", jsonObj2);

    //pw->SetThickness(5.5);

    //double thickness = pw->GetThickness();

    //ASSERT_TRUE(5.5 == thickness);

    pw->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, WALL_CODE_VALUE2);
    ASSERT_TRUE(queriedElement.IsValid());

    //create a view, it is neccessary if you like to see geoemetry with Gist
    Dgn::DefinitionModelR dictionary = db->GetDictionaryModel();
    Dgn::CategorySelectorPtr categorySelector = new Dgn::CategorySelector(dictionary, "Default");

    Dgn::ModelSelectorPtr modelSelector = new Dgn::ModelSelector(dictionary, "Default");
    modelSelector->AddModel(physicalModel->GetModelId());

    Dgn::DisplayStyle3dPtr displayStyle = new Dgn::DisplayStyle3d(dictionary, "Default");

    displayStyle->SetBackgroundColor(Dgn::ColorDef::DarkYellow());
    displayStyle->SetSkyBoxEnabled(false);
    displayStyle->SetGroundPlaneEnabled(false);

    Dgn::Render::ViewFlags viewFlags = displayStyle->GetViewFlags();
    viewFlags.SetRenderMode(Dgn::Render::RenderMode::SmoothShade);
    viewFlags.SetShowTransparency(true);
    viewFlags.ShowTransparency();

    displayStyle->SetViewFlags(viewFlags);

    //create view 
    Dgn::OrthographicViewDefinition view(dictionary, "Structure View", *categorySelector, *displayStyle, *modelSelector);
    view.SetStandardViewRotation(Dgn::StandardView::Iso); // Default to a rotated view
    view.LookAtVolume(db->GeoLocation().GetProjectExtents());
    view.Insert();
    Dgn::DgnViewId viewId = view.GetViewId();
    db->SaveProperty(Dgn::DgnViewProperty::DefaultView(), &viewId, (uint32_t) sizeof(viewId));
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


#define STRUCTURALSUBTRACTION_CODE_VALUE       "STRUCTURALSUBTRACTION-001"
TEST_F(StructuralDomainTestFixture, StructuralSubtractionClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::DgnDbStatus status;

    StructuralSubtractionPtr subtraction = StructuralSubtraction::Create(physicalModel);
    subtraction->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, STRUCTURALSUBTRACTION_CODE_VALUE);
    subtraction->SetCode(code);
    subtraction->SetUserLabel("Structural Subtraction label");
    subtraction->SetFederationGuid(BeSQLite::BeGuid(true));

    subtraction->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::SpatialLocationElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::SpatialLocationElement>(*physicalModel, STRUCTURALSUBTRACTION_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define STRUCTURALADDITION_CODE_VALUE       "STRUCTURALADDITION-001"
TEST_F(StructuralDomainTestFixture, StructuralAdditionClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::DgnDbStatus status;

    StructuralAdditionPtr addition = StructuralAddition::Create(physicalModel);
    addition->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, STRUCTURALADDITION_CODE_VALUE);
    addition->SetCode(code);
    addition->SetUserLabel("Structural Addition label");
    addition->SetFederationGuid(BeSQLite::BeGuid(true));

    addition->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, STRUCTURALADDITION_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

TEST_F(StructuralDomainTestFixture, StraightExtrusionTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::DgnDbStatus status;
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, "StraightExtrusionTests");

    WallPtr pw = Wall::Create(physicalModel);
    status = pw->SetCode(code);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Forms::StraightExtrusionPtr se = Forms::StraightExtrusion::Create();

    status = se->SetPropertyValue("Length", ECN::ECValue(0.58));
    ASSERT_TRUE(Dgn::DgnDbStatus::NotEnabled == status);

    Forms::StraightExtrusion::SetAspect(*pw, *se); //set aspect to wall

    se->SetLength(78.0);

    IGeometryPtr geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    se->SetShape(geom);

    pw->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Forms::StraightExtrusionP se2 = Forms::StraightExtrusion::GetAspectP(*pw);
    double b = se2->GetLength();

    ASSERT_TRUE(se->GetLength() ==  se2->GetLength());

    DgnElement::UniqueAspect* aspect = DgnElement::UniqueAspect::GetAspectP(*pw, *(se->GetECClass(*db)));
    ECN::ECValue assValue;
    status = aspect->GetPropertyValue(assValue, "Length");
    ASSERT_TRUE(Dgn::DgnDbStatus::NotEnabled == status);
    }

TEST_F(StructuralDomainTestFixture, CurvedExtrusionTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::DgnDbStatus status;
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, "CurvedExtrusionTests");

    WallPtr pw = Wall::Create(physicalModel);
    status = pw->SetCode(code);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Forms::CurvedExtrusionPtr ce = Forms::CurvedExtrusion::Create();

    IGeometryPtr geom1 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    IGeometryPtr geom2 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 1.0, 0.0, 1.0, 1.0, 1.0)));
    IGeometryPtr geom3 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 1.0, 1.0, 1.0, 1.0)));

    Forms::CurvedExtrusion::SetAspect(*pw, *ce); //set aspect to wall

    ce->SetStartShape(geom1);
    ce->SetEndShape(geom2);
    ce->SetCurve(geom3);

    pw->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Forms::CurvedExtrusionP ce2 = Forms::CurvedExtrusion::GetAspectP(*pw);
    IGeometryPtr geomCurve = ce2->GetCurve();

    ASSERT_TRUE(geomCurve->IsSameStructureAndGeometry(*ce->GetCurve()));

    DgnElement::UniqueAspect* aspect = DgnElement::UniqueAspect::GetAspectP(*pw, *(ce->GetECClass(*db)));
    ECN::ECValue assValue;
    status = aspect->GetPropertyValue(assValue, "Curve");
    ASSERT_TRUE(Dgn::DgnDbStatus::NotEnabled == status);
    }

TEST_F(StructuralDomainTestFixture, StraightProfileExtrusionTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::DgnDbStatus status;
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, "StraightProfiledExtrusionTests");

    WallPtr pw = Wall::Create(physicalModel);
    status = pw->SetCode(code);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);


    Dgn::DgnCode code2 = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, "StraightProfiledExtrusionTests 2");

    WallPtr pw2 = Wall::Create(physicalModel);
    status = pw2->SetCode(code2);
    pw2->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    //create constant profile

    Profiles::ProfileDefinitionModelCPtr profilesModel = Profiles::ProfilesDomainUtilities::CreateProfilesModel(MODEL_TEST_NAME, *db, nullptr);
    ASSERT_TRUE(profilesModel.IsValid());

    Dgn::DgnCode codeConstProfile = Dgn::CodeSpec::CreateCode(BENTLEY_PROFILES_AUTHORITY, *profilesModel, "ConstProfile");

    Profiles::ConstantProfilePtr constantProfile = Profiles::ConstantProfile::Create(profilesModel);
    ASSERT_TRUE(constantProfile.IsValid());
    constantProfile->SetCode(codeConstProfile);

   
    Dgn::DgnElementCPtr persistentElement = constantProfile->Insert(&status);
    ASSERT_TRUE(persistentElement.IsValid());


    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Forms::StraightProfiledExtrusionPtr se = Forms::StraightProfiledExtrusion::Create();

    status = se->SetPropertyValue("Length", ECN::ECValue(0.58));
    ASSERT_TRUE(Dgn::DgnDbStatus::NotEnabled == status);
    se->SetLength(78.0);
    se->SetProfile(*constantProfile);
    Forms::StraightProfiledExtrusion::SetAspect(*pw, *se); //set aspect to wall

    pw->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
    }

TEST_F(StructuralDomainTestFixture, StructuralMemberRefersToStructuralSubtractionTest)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::DgnDbStatus status;
    Dgn::DgnCode subtrCode1 = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, "StructuralSubtractionTests - code for subtraction 1");
    StructuralSubtractionPtr psub1 = StructuralSubtraction::Create(physicalModel);
    status = psub1->SetCode(subtrCode1);
    psub1->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DgnCode subtrCode2 = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, "StructuralAdditionTests - code for addition 2");
    StructuralSubtractionPtr psub2 = StructuralSubtraction::Create(physicalModel);
    status = psub2->SetCode(subtrCode2);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
    psub2->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DgnCode memberCode1 = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, "StructuralSubtractionTests - code for structural member 1");
    WallPtr pstructMember1 = Wall::Create(physicalModel);
    pstructMember1->SetCode(memberCode1);
    pstructMember1->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DgnCode memberCode2 = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, "StructuralSubtractionTests - code for structural member 2");
    WallPtr pstructMember2 = Wall::Create(physicalModel);
    pstructMember2->SetCode(memberCode2);
    pstructMember2->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
    
    //many to many relationship
    BeSQLite::EC::ECInstanceKey key;
    BeSQLite::DbResult sqliteStatus = StructuralDomainUtilities::InsertLinkTableRelationship(key, *db, BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, "StructuralMemberRefersToStructuralSubtraction", pstructMember1->GetECInstanceKey(), psub1->GetECInstanceKey());
    ASSERT_TRUE(BeSQLite::DbResult::BE_SQLITE_ERROR != sqliteStatus);

    sqliteStatus = StructuralDomainUtilities::InsertLinkTableRelationship(key, *db, BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, "StructuralMemberRefersToStructuralSubtraction", pstructMember2->GetECInstanceKey(), psub1->GetECInstanceKey());
    ASSERT_TRUE(BeSQLite::DbResult::BE_SQLITE_ERROR != sqliteStatus);

    sqliteStatus = StructuralDomainUtilities::InsertLinkTableRelationship(key, *db, BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, "StructuralMemberRefersToStructuralSubtraction", pstructMember1->GetECInstanceKey(), psub2->GetECInstanceKey());
    ASSERT_TRUE(BeSQLite::DbResult::BE_SQLITE_ERROR != sqliteStatus);

    sqliteStatus = StructuralDomainUtilities::InsertLinkTableRelationship(key, *db, BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, "StructuralMemberRefersToStructuralSubtraction", pstructMember2->GetECInstanceKey(), psub2->GetECInstanceKey());
    ASSERT_TRUE(BeSQLite::DbResult::BE_SQLITE_ERROR != sqliteStatus);

    }

TEST_F(StructuralDomainTestFixture, StructuralMemberOwnsStructuralAdditionTest)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::DgnDbStatus status;
    Dgn::DgnCode addCode1 = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, "StructuralAdditionTests - code for subtraction 1");
    StructuralAdditionPtr padd1 = StructuralAddition::Create(physicalModel);
    status = padd1->SetCode(addCode1);
    padd1->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DgnCode memberCode1 = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, "StructuralAdditionTests - code for structural member 1");
    WallPtr pstructMember1 = Wall::Create(physicalModel);
    pstructMember1->SetCode(memberCode1);
    pstructMember1->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    status = padd1->SetParentId(pstructMember1->GetElementId(), pstructMember1->GetElementClassId());
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
    }

#define PILECAP_CODE_VALUE       "PILECAP-001"
TEST_F(StructuralDomainTestFixture, PileCapTest)
    {
    DgnDbPtr db = OpenDgnDb();
    Dgn::DgnDbStatus status;

    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, PILECAP_CODE_VALUE);
    PileCapPtr pileCap = PileCap::Create(physicalModel);
    status = pileCap->SetCode(code);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    pileCap->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, PILECAP_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define PILE_CODE_VALUE       "PILE-001"
TEST_F(StructuralDomainTestFixture, PileTest)
    {
    DgnDbPtr db = OpenDgnDb();
    Dgn::DgnDbStatus status;

    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, PILE_CODE_VALUE);
    PilePtr pile = Pile::Create(physicalModel);
    status = pile->SetCode(code);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    pile->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, PILE_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }

#define PILE_CODE_VALUE2       "PILE-002"
TEST_F(StructuralDomainTestFixture, PileMixinTest)
{
    DgnDbPtr db = OpenDgnDb();
    Dgn::DgnDbStatus status;

    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, PILE_CODE_VALUE2);
    PilePtr pile = Pile::Create(physicalModel);
    status = pile->SetCode(code);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    ASSERT_TRUE(pile->IsCurveMember());
    ASSERT_FALSE(pile->IsSurfaceMember());

    pile->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*physicalModel, PILE_CODE_VALUE2);
    ASSERT_TRUE(queriedElement.IsValid());
    }

TEST_F(StructuralDomainTestFixture, CurvedProfileExtrusionTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::DgnDbStatus status;
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, "CurvedProfileExtrusionTests");

    WallPtr pw = Wall::Create(physicalModel);
    status = pw->SetCode(code);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Profiles::ProfileDefinitionModelCPtr profilesModel = Profiles::ProfilesDomainUtilities::GetProfilesModel(MODEL_TEST_NAME, *db, nullptr);
    ASSERT_TRUE(profilesModel.IsValid());

    Dgn::DgnCode codeConstProfile1 = Dgn::CodeSpec::CreateCode(BENTLEY_PROFILES_AUTHORITY, *profilesModel, "CurvedProfileExtrusionTests_ConstProfile_1");
    Dgn::DgnCode codeConstProfile2 = Dgn::CodeSpec::CreateCode(BENTLEY_PROFILES_AUTHORITY, *profilesModel, "CurvedProfileExtrusionTests_ConstProfile_2");

    Profiles::ConstantProfilePtr constantProfile1 = Profiles::ConstantProfile::Create(profilesModel);
    ASSERT_TRUE(constantProfile1.IsValid());
    constantProfile1->SetCode(codeConstProfile1);
    Dgn::DgnElementCPtr persistentElement1 = constantProfile1->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
    ASSERT_TRUE(persistentElement1.IsValid());

    Profiles::ConstantProfilePtr constantProfile2 = Profiles::ConstantProfile::Create(profilesModel);
    ASSERT_TRUE(constantProfile2.IsValid());
    constantProfile2->SetCode(codeConstProfile2);
    Dgn::DgnElementCPtr persistentElement2 = constantProfile2->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
    ASSERT_TRUE(persistentElement2.IsValid());


    Forms::CurvedProfiledExtrusionPtr se = Forms::CurvedProfiledExtrusion::Create();
    se->SetStartProfile(*constantProfile1);
    se->SetEndProfile(*constantProfile2);
    IGeometryPtr geom1 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    se->SetCurve(geom1);

    Forms::CurvedProfiledExtrusion::SetAspect(*pw, *se); //set aspect to wall

    pw->Insert(&status);

    Forms::CurvedProfiledExtrusionP se2 = Forms::CurvedProfiledExtrusion::GetAspectP(*pw);


    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
    }

TEST_F(StructuralDomainTestFixture, EnsureCanContainAnyPhysicalElement)
    {
    Utf8CP schemaXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName = \"TestSchema\" alias = \"tt\" version = \"01.00.00\" xmlns = \"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
        "    <ECSchemaReference name = \"BisCore\" version = \"01.00\" alias = \"bis\" />"
        "    <ECEntityClass typeName = \"TestElement\">"
        "        <BaseClass>bis:PhysicalElement</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";

    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater(db->GetSchemaLocater());

    ECN::ECSchemaPtr schema;
    ECN::SchemaReadStatus status = ECN::ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);

    ASSERT_TRUE(ECN::SchemaReadStatus::Success == status);

    Dgn::SchemaStatus dgnSchemaStatus = db->ImportSchemas(schemaContext->GetCache().GetSchemas());

    ASSERT_TRUE(SchemaStatus::Success == dgnSchemaStatus);

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::PhysicalElementPtr physicalElement = Structural::StructuralDomainUtilities::CreatePhysicalElement("TestSchema", "TestElement", *physicalModel);
    ASSERT_TRUE(physicalElement.IsValid());

    Dgn::CodeSpecPtr codeSpec = Dgn::CodeSpec::Create(*db, "TestSchema", Dgn::CodeScopeSpec::CreateModelScope());

    if (codeSpec.IsValid())
        {
        codeSpec->Insert();
        }

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode("TestSchema", *physicalModel, "testPhysicalElement");
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == physicalElement->SetCode(code));

    Dgn::DgnDbStatus dbStatus;
    physicalElement->Insert(&dbStatus);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == dbStatus);

    DgnElementId id = db->Elements().QueryElementIdByCode(code);
    ASSERT_TRUE(id.IsValid());
    }
*/

#endif // NO_DGNVIEW_IMODEL02
