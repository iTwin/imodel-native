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
#include <BeSQLite/BeSQLite.h>
#include <Json/Json.h>
#include <Bentley\BeAssert.h>


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


    //newtest
    /*ECN::ECClassCP structuralClass = db->GetClassLocater().LocateClass(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, Wall::MyHandlerECClassName());
    ElementHandlerP elementHandler = dgn_ElementHandler::Element::FindHandler(*db, structuralClass->GetId());
    DgnElementPtr   elem = elementHandler->Create(DgnElement::CreateParams(*db, physicalModel->GetModelId(), structuralClass->GetId(), code));
    GeometrySourceP geomElem = elem.IsValid() ? elem->ToGeometrySourceP() : nullptr;

    Dgn::DgnCategoryId categoryId = Structural::StructuralPhysicalCategory::QueryStructuralPhysicalCategoryId(*db, structuralClass->GetDisplayLabel().c_str());
    geomElem->SetCategoryId(categoryId);

    */


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

    /*pw->SetThickness(5.5);

    double thickness = pw->GetThickness();

    ASSERT_TRUE(5.5 == thickness);*/

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

    /*double queriedThickness = queriedElement->GetPropertyValueDouble("Thickness");

    ASSERT_EQ(queriedThickness, thickness);*/
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

    //abstract class
    }

#define VARYINGPROFILEBYZONE_CODE_VALUE       "VARYINGPROFILEBYZONE-001"

TEST_F(StructuralDomainTestFixture, VaryingProfileByZoneClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralTypeDefinitionModelCPtr definitionModel = Structural::StructuralDomainUtilities::GetStructuralTypeDefinitionModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(definitionModel.IsValid());

    Dgn::DefinitionElementPtr definitionElement = Structural::StructuralDomainUtilities::CreateDefinitionElement(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_VaryingProfileByZone, *definitionModel);
    ASSERT_TRUE(definitionElement.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, VARYINGPROFILEBYZONE_CODE_VALUE);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == definitionElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = definitionElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DefinitionElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::DefinitionElement>(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, VARYINGPROFILEBYZONE_CODE_VALUE);
    ASSERT_TRUE(queriedElement.IsValid());
    }


#define CONSTANTPROFILE_CODE_VALUE2       "CONSTANTPROFILE-002"
TEST_F(StructuralDomainTestFixture, BuiltUpProfileComponentUsesConstantProfileTest)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralTypeDefinitionModelCPtr definitionModel = Structural::StructuralDomainUtilities::GetStructuralTypeDefinitionModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(definitionModel.IsValid());

    Dgn::DefinitionElementPtr definitionElement = Structural::StructuralDomainUtilities::CreateDefinitionElement(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_ConstantProfile, *definitionModel);
    ASSERT_TRUE(definitionElement.IsValid());

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, CONSTANTPROFILE_CODE_VALUE2);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == definitionElement->SetCode(code));

    Dgn::DgnDbStatus status;
    Dgn::DgnElementCPtr element = definitionElement->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::DefinitionElementPtr queriedElement = Structural::StructuralDomainUtilities::QueryByCodeValue<Dgn::DefinitionElement>(BENTLEY_STRUCTURAL_PROFILES_AUTHORITY, *definitionModel, CONSTANTPROFILE_CODE_VALUE2);
    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECClassCP  aspectClass = db->Schemas().GetClass(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_BuiltUpProfileComponent);
    RefCountedPtr<DgnElement::MultiAspect> aspect = DgnElement::MultiAspect::CreateAspect(*db, *aspectClass);
    DgnElement::MultiAspect::AddAspect(*queriedElement, *aspect);

    ASSERT_TRUE(queriedElement->Update().IsValid());
    }

TEST_F(StructuralDomainTestFixture, VaryingProfileZoneClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    ECN::ECClassCP  aspectClass = db->Schemas().GetClass(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_VaryingProfileZone);
    ECN::StandaloneECEnablerPtr aspectEnabler = aspectClass->GetDefaultStandaloneEnabler();

    ASSERT_TRUE(aspectEnabler.IsValid());

    ECN::StandaloneECInstancePtr p = aspectEnabler->CreateInstance();
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

TEST_F(StructuralDomainTestFixture, FormClassTests)
    {
    DgnDbPtr db = OpenDgnDb();
    ASSERT_TRUE(db.IsValid());

    Structural::StructuralPhysicalModelCPtr physicalModel = Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(physicalModel.IsValid());

    Dgn::DgnDbStatus status;
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *physicalModel, "FormClassTests");

    WallPtr pw = Wall::Create(physicalModel);
    status = pw->SetCode(code);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    FormPtr formAspect = Form::Create();
    Form::SetAspect(*pw, *formAspect); //set aspect to wall

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
    pstructMember1->SetCode(memberCode1);//not sure it is correct...
    pstructMember1->Insert(&status);
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    status = padd1->SetParentId(pstructMember1->GetElementId(), pstructMember1->GetElementClassId());
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
    }