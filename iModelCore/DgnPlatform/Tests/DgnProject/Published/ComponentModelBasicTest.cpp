/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ComponentModelBasicTest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef BENTLEYCONFIG_NO_JAVASCRIPT
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/ECUtils.h>
#include <DgnPlatform/DgnScript.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeNumerical.h>
#include <Logging/bentleylogging.h>
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE

// *** WARNING: Keep this consistent with ComponentModelTest.ts
#define TEST_JS_NAMESPACE    "ComponentModelTest"
#define TEST_JS_NAMESPACE_W L"ComponentModelTest"
#define TEST_BOXES_COMPONENT_NAME "Boxes"
#define TEST_BOXES_COMPONENT_ECSQLCLASS_NAME Utf8PrintfString("%s.%s", TEST_JS_NAMESPACE , TEST_BOXES_COMPONENT_NAME)

/*=================================================================================**//**
* This is a stub implementation of a Script library. 
* In a real implementation, we would look in a disk=based cache or go to Bentley Connect.
* @bsiclass                                                     Sam.Wilson     02/2012
+===============+===============+===============+===============+===============+======*/
struct FakeScriptLibrary : ScopedDgnHost::FetchScriptCallback
    {
    Utf8String m_jsProgramName;
    Utf8String m_jsProgramText;

    Dgn::DgnDbStatus _FetchScript(Utf8StringR sText, DgnScriptType& stypeFound, DateTime& lmt, DgnDbR, Utf8CP sName, DgnScriptType stypePreferred) override
        {
        if (!m_jsProgramName.EqualsI(sName))
            return DgnDbStatus::NotFound;
        stypeFound = DgnScriptType::JavaScript;
        sText = m_jsProgramText;
        lmt = DateTime();
        return DgnDbStatus::Success;
        }
    };
/*=================================================================================**//**
* @bsiclass                                                     Umar.Hayat     01/16
+===============+===============+===============+===============+===============+======*/
struct ComponentModelBasicTest : public DgnDbTestFixture
{
    //Dgn::ScopedDgnHost m_host;
    FakeScriptLibrary  m_library;
    DgnDbPtr initDb(WCharCP fileName, Db::OpenMode mode = Db::OpenMode::ReadWrite);
    DgnCategoryId CreateCategory(DgnDbPtr& db, Utf8CP code, ColorDef const& color);
    void CreateAndImportComponentSchema(DgnDbPtr& db); 
    DgnElementCPtr CreateVariation(ComponentDefPtr& cdef, SpatialModelR targetModel, DPoint3d boxSize, Utf8CP name);
    void SetScriptLibrary();
    void AddScriptToLibrary(Utf8CP ProgramName = nullptr, Utf8CP ProgramText=nullptr);
};

#define MakeBoxTsComponentParameterSet() \
    TsComponentParameterSet params; \
    params["H"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0)); \
    params["W"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0)); \
    params["D"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0)); \
    params["box_count"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue("text"));

#define RegisterBoxModelResolverScript() \
    AddScriptToLibrary(); \
    SetScriptLibrary();


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr ComponentModelBasicTest::initDb(WCharCP fileName, Db::OpenMode mode)
    {
    BeFileName dbName;
    EXPECT_TRUE(DgnDbStatus::Success == DgnDbTestFixture::GetSeedDbCopy(dbName, fileName));
    DgnDbPtr db;
    DgnDbTestFixture::OpenDb(db, dbName, Db::OpenMode::ReadWrite);
    return db;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus createSpatialModel(SpatialModelPtr& catalogModel, DgnDbR db, DgnCode const& code)
    {
    DgnClassId mclassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel));
    catalogModel = new SpatialModel(SpatialModel::CreateParams(db, mclassId, code));
    catalogModel->SetInGuiList(false);
    return catalogModel->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelBasicTest:: SetScriptLibrary()
    {
    m_host.SetFetchScriptCallback(&m_library);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId ComponentModelBasicTest::CreateCategory(DgnDbPtr& db,Utf8CP code, ColorDef const& color)
    {
    DgnCategory cat(DgnCategory::CreateParams(*db, code, DgnCategory::Scope::Any));
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);
    cat.Insert(appearance);
    return cat.GetCategoryId();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentModelBasicTest::CreateVariation(ComponentDefPtr& cdef, SpatialModelR targetModel, DPoint3d boxSize, Utf8CP name)
    {
    int boxCount = 2;
    ECN::IECInstancePtr params = cdef->MakeVariationSpec();
    params->SetValue("H", ECN::ECValue(boxSize.x));
    params->SetValue("W", ECN::ECValue(boxSize.y));
    params->SetValue("D", ECN::ECValue(boxSize.z));
    params->SetValue("box_count", ECN::ECValue(boxCount));
    DgnDbStatus status;
    DgnElementCPtr variation = cdef->MakeVariation(&status, targetModel, *params, name);
    EXPECT_TRUE(variation.IsValid());
    EXPECT_TRUE(DgnDbStatus::Success == status);
    return variation;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelBasicTest::AddScriptToLibrary(Utf8CP ProgramName, Utf8CP ProgramText)
    {
    m_library.m_jsProgramName = ProgramName ? ProgramName : TEST_JS_NAMESPACE;
    m_library.m_jsProgramText = ProgramText ? ProgramText : "(function () { \
            function makeBoxes(model, params, options) { \
                var angles = new Bentley.Dgn.YawPitchRollAngles(0,0,0);\
                for (var i = 0; i < params.box_count; i++)\
                    {\
                    var boxSize = new be.DPoint3d(params.H, params.W, params.D); \
                    var solid = be.DgnBox.CreateCenteredBox(new be.DPoint3d(0,0,0), boxSize, true); \
                    var element = model.CreateElement('dgn.PhysicalElement', options.Category);\
                    var origin = new Bentley.Dgn.DPoint3d(i,i,i);\
                    var builder = new Bentley.Dgn.ElementGeometryBuilder(element, origin, angles); \
                    builder.AppendGeometry(solid); \
                    builder.SetGeomStreamAndPlacement(element); \
                    element.Insert(); \
                    }\
                return 0;\
            } \
            Bentley.Dgn.RegisterModelSolver('" TEST_JS_NAMESPACE ".Boxes" "', makeBoxes); \
        })();\
        ";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelBasicTest::CreateAndImportComponentSchema(DgnDbPtr& db)
    {
    ECN::ECSchemaPtr testSchema = ComponentDefCreator::GenerateSchema(*db, TEST_JS_NAMESPACE);
    ASSERT_TRUE(testSchema.IsValid());

    ECN::ECClassCP baseClass = db->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement);

    TsComponentParameterSet params;
    params["H"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));
    params["W"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));
    params["D"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));
    params["box_count"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue("text"));

    ComponentDefCreator creator(*db, *testSchema, TEST_BOXES_COMPONENT_NAME, *baseClass, TEST_JS_NAMESPACE "." TEST_BOXES_COMPONENT_NAME, "Boxes", "", params);
    ECN::ECClassCP ecClass = creator.GenerateECClass();
    EXPECT_TRUE(nullptr != ecClass);

    ASSERT_TRUE(nullptr != ComponentDefCreator::ImportSchema(*db, *testSchema, false));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, ComponentDef_FromECClass)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());
    
    ECN::ECSchemaPtr testSchema = ComponentDefCreator::GenerateSchema(*m_db, TEST_JS_NAMESPACE);
    ASSERT_TRUE(testSchema.IsValid());

    ECN::ECClassCP baseClass = m_db->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement);

    MakeBoxTsComponentParameterSet();

    ComponentDefCreator creator(*m_db, *testSchema, TEST_BOXES_COMPONENT_NAME, *baseClass, TEST_JS_NAMESPACE "." TEST_BOXES_COMPONENT_NAME, "Boxes", "", params);
    ECN::ECClassCP ecClass = creator.GenerateECClass();
    EXPECT_TRUE(nullptr != ecClass);

    ASSERT_TRUE(nullptr != ComponentDefCreator::ImportSchema(*m_db, *testSchema, false));

    DgnDbStatus status;
    ComponentDefPtr def1 = ComponentDef::FromECClass(&status, *m_db, *ecClass);
    EXPECT_FALSE(def1.IsValid());
    // Doesn't return status right now, uncomment once fixed
    EXPECT_TRUE(status == DgnDbStatus::InvalidCategory);

    ASSERT_TRUE(CreateCategory(m_db, "Boxes", ColorDef(255, 0, 0)).IsValid());

    ComponentDefPtr cdef = ComponentDef::FromECClass(&status, *m_db, *ecClass);
    EXPECT_TRUE(cdef.IsValid());

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, ComponentDef_FromECClassId)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());
    
    ECN::ECSchemaPtr testSchema = ComponentDefCreator::GenerateSchema(*m_db, TEST_JS_NAMESPACE);
    ASSERT_TRUE(testSchema.IsValid());

    ECN::ECClassCP baseClass = m_db->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement);

    MakeBoxTsComponentParameterSet();

    ComponentDefCreator creator(*m_db, *testSchema, TEST_BOXES_COMPONENT_NAME, *baseClass, TEST_JS_NAMESPACE "." TEST_BOXES_COMPONENT_NAME, "Boxes", "", params);
    ECN::ECClassCP ecClass = creator.GenerateECClass();
    EXPECT_TRUE(nullptr != ecClass);

    ASSERT_TRUE(nullptr != ComponentDefCreator::ImportSchema(*m_db, *testSchema, false));

    DgnDbStatus status;
    ComponentDefPtr def1 = ComponentDef::FromECClassId(&status, *m_db, DgnClassId(ecClass->GetId()));
    EXPECT_FALSE(def1.IsValid());
    // Doesn't return status right now, uncomment once fixed
    EXPECT_TRUE(status == DgnDbStatus::InvalidCategory);

    ASSERT_TRUE(CreateCategory(m_db, "Boxes", ColorDef(255, 0, 0)).IsValid());

    ComponentDefPtr def2 = ComponentDef::FromECClassId(&status, *m_db, DgnClassId(ecClass->GetId()));
    EXPECT_TRUE(def2.IsValid());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, ComponentDef_FromECSqlName)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());

    DgnDbStatus status;
    ComponentDefPtr def0 = ComponentDef::FromECSqlName(&status, *m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_FALSE(def0.IsValid());
    // Doesn't return status right now, uncomment once fixed
    EXPECT_TRUE(status == DgnDbStatus::NotFound);

    // With outstatus Status Nullptr
    def0 = ComponentDef::FromECSqlName(nullptr, *m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_FALSE(def0.IsValid());

    CreateAndImportComponentSchema(m_db);

    ComponentDefPtr def1 = ComponentDef::FromECSqlName(&status, *m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_FALSE(def1.IsValid());

    // Doesn't return status right now, uncomment once fixed
    EXPECT_TRUE(status == DgnDbStatus::InvalidCategory);
    ASSERT_TRUE(CreateCategory(m_db, "Boxes", ColorDef(255, 0, 0)).IsValid());

    ComponentDefPtr def2 = ComponentDef::FromECSqlName(&status, *m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(def2.IsValid());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, CreateComponentModel)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());

    CreateAndImportComponentSchema(m_db);

    ComponentModelPtr CM2 = ComponentModel::Create(*m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(CM2.IsValid());

    ASSERT_TRUE(DgnDbStatus::Success == CM2->Insert());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, CreateComponentModel_NoSchema)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());

    ComponentModelPtr CM = ComponentModel::Create(*m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(CM.IsValid());

    ASSERT_TRUE(DgnDbStatus::Success == CM->Insert());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, CreateComponentModel_Redundant)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());

    ComponentModelPtr CM = ComponentModel::Create(*m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(CM.IsValid());

    ASSERT_TRUE(DgnDbStatus::Success == CM->Insert());

    ComponentModelPtr CM2 = ComponentModel::Create(*m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(CM2.IsValid());

    ASSERT_TRUE(DgnDbStatus::Success == CM2->Insert());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, ComponentDef_FromComponentModel)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());

    ComponentModelPtr CM = ComponentModel::Create(*m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(CM.IsValid());

    ASSERT_TRUE(DgnDbStatus::Success == CM->Insert());
    
    DgnDbStatus status;
    // When component class does not exist
    ComponentDefPtr def = ComponentDef::FromComponentModel(&status, *CM);
    EXPECT_FALSE(def.IsValid());
    // TODO: Check status

    CreateAndImportComponentSchema(m_db);

    // Still category does not exist
    ComponentDefPtr def2 = ComponentDef::FromComponentModel(&status, *CM);
    EXPECT_FALSE(def2.IsValid());
    // Doesn't return status right now, uncomment once fixed
    //EXPECT_TRUE(status == DgnDbStatus::InvalidCategory);

    ASSERT_TRUE(CreateCategory(m_db, "Boxes", ColorDef(255, 0, 0)).IsValid());
    ComponentDefPtr def3 = ComponentDef::FromComponentModel(&status, *CM);
    EXPECT_TRUE(def3.IsValid());

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, DeleteComponentDef)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());

    // Create Category
    ASSERT_TRUE(CreateCategory(m_db, "Boxes", ColorDef(255, 0, 0)).IsValid());
    DgnDbStatus status = ComponentDef::DeleteComponentDef(*m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(DgnDbStatus::Success != status);

    CreateAndImportComponentSchema(m_db);

    ComponentDefPtr def2 = ComponentDef::FromECSqlName(&status, *m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(def2.IsValid());

    status = ComponentDef::DeleteComponentDef(*m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    ASSERT_TRUE(DgnDbStatus::Success == status);

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, ComponentDef_Export)
    {
    SetupSeedProject(L"ComponentDef_Export_Source.dgndb");
    ASSERT_TRUE(m_db.IsValid());
    DgnDbPtr clientDb = initDb(L"ComponentDef_Export_Destination.dgndb");
    ASSERT_TRUE(clientDb.IsValid());

    // Create Category and Component Schema
    ASSERT_TRUE(CreateCategory(m_db, "Boxes", ColorDef(255, 0, 0)).IsValid());
    CreateAndImportComponentSchema(m_db);
    m_db->SaveChanges();

    DgnDbStatus status;
    ComponentDefPtr cdef = ComponentDef::FromECSqlName(&status, *m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(cdef.IsValid());

    // Exporting in second DgnDb
    DgnImportContext context(*m_db, *clientDb);
    status = cdef->Export(context);
    ASSERT_TRUE(DgnDbStatus::Success == status);
    clientDb->SaveChanges();

    // Export redundantly 
    status = cdef->Export(context);
    ASSERT_TRUE(DgnDbStatus::Success == status);
    clientDb->SaveChanges();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, ComponentDef_Export_SameDb)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());

    // Create Category and Component Schema
    ASSERT_TRUE(CreateCategory(m_db, "Boxes", ColorDef(255, 0, 0)).IsValid());
    CreateAndImportComponentSchema(m_db);
    m_db->SaveChanges();

    DgnDbStatus status;
    ComponentDefPtr cdef = ComponentDef::FromECSqlName(&status, *m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(cdef.IsValid());
    
    // Exporting in Same DgnDb
    DgnImportContext context(*m_db,*m_db);
    status = cdef->Export(context);
    ASSERT_TRUE(DgnDbStatus::Success == status);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, QueryComponentDefs)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());

    DgnDbStatus status;
    ASSERT_TRUE(CreateCategory(m_db, "Boxes", ColorDef(255, 0, 0)).IsValid());
    ECN::ECSchemaPtr testSchema = ComponentDefCreator::GenerateSchema(*m_db, TEST_JS_NAMESPACE);
    ASSERT_TRUE(testSchema.IsValid());

    ECN::ECClassCP baseClass = m_db->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement);

    MakeBoxTsComponentParameterSet();

    ComponentDefCreator creator(*m_db, *testSchema, TEST_BOXES_COMPONENT_NAME, *baseClass, TEST_JS_NAMESPACE "." TEST_BOXES_COMPONENT_NAME, "Boxes", "", params);
    ECN::ECClassCP ecClass = creator.GenerateECClass();
    EXPECT_TRUE(nullptr != ecClass);

    ASSERT_TRUE(nullptr != ComponentDefCreator::ImportSchema(*m_db, *testSchema, false));

    m_db->SaveChanges();

    ComponentDefPtr cdef = ComponentDef::FromECSqlName(&status, *m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(cdef.IsValid());

    DgnImportContext context(*m_db, *m_db);
    //DgnDbStatus Export(DgnImportContext& context, bool exportSchema = true, bool exportCategory = false);
    status = cdef->Export(context);
    ASSERT_TRUE(DgnDbStatus::Success == status);
    m_db->SaveChanges();

    bvector<DgnClassId> componentDefs;
    ComponentDef::QueryComponentDefs(componentDefs, *m_db, *baseClass);  // The last argument is the base class, not the component class
    ASSERT_EQ(1 , componentDefs.size());

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, ComponentDef_FromInstance)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());

    DgnDbStatus status;
    // Create Category and Component Schema
    ASSERT_TRUE(CreateCategory(m_db, "Boxes", ColorDef(255, 0, 0)).IsValid());
    CreateAndImportComponentSchema(m_db);
    m_db->SaveChanges();

    ComponentDefPtr cdef = ComponentDef::FromECSqlName(&status, *m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(cdef.IsValid());

    //  Create the target model in the client.
    SpatialModelPtr targetModel;
    ASSERT_EQ(DgnDbStatus::Success, createSpatialModel(targetModel, *m_db, DgnModel::CreateModelCode("Instances")));

    // When Script library is not enabled
    DgnElementCPtr uniqueInstance = ComponentDef::MakeUniqueInstance(&status, *targetModel, *cdef->MakeVariationSpec());
    EXPECT_FALSE(DgnDbStatus::Success == status);

    RegisterBoxModelResolverScript();

    uniqueInstance = ComponentDef::MakeUniqueInstance(&status, *targetModel, *cdef->MakeVariationSpec());
    EXPECT_TRUE(DgnDbStatus::Success == status);

    ComponentDefPtr def3 = ComponentDef::FromInstance(&status, *uniqueInstance);
    EXPECT_TRUE(def3.IsValid());

    m_db->SaveChanges();

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, MakeUniqueInstance)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());

    DgnDbStatus status;
    // Create Category and Component Schema
    ASSERT_TRUE(CreateCategory(m_db, "Boxes", ColorDef(255, 0, 0)).IsValid());
    CreateAndImportComponentSchema(m_db);
    m_db->SaveChanges();

    ComponentDefPtr cdef = ComponentDef::FromECSqlName(&status, *m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(cdef.IsValid());

    //  Create the target model in the client.
    SpatialModelPtr targetModel;
    ASSERT_EQ(DgnDbStatus::Success, createSpatialModel(targetModel, *m_db, DgnModel::CreateModelCode("Instances")));

    // When Script library is not enabled
    DgnElementCPtr uniqueInstance = ComponentDef::MakeUniqueInstance(&status, *targetModel, *cdef->MakeVariationSpec());
    EXPECT_FALSE(DgnDbStatus::Success == status);

    RegisterBoxModelResolverScript();

    uniqueInstance = ComponentDef::MakeUniqueInstance(&status, *targetModel, *cdef->MakeVariationSpec());
    EXPECT_TRUE(DgnDbStatus::Success == status);

    m_db->SaveChanges();

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, Variations)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());

    DgnDbStatus status;
    //---------------------------------------------------------------------------------------------------------------
    // Create Category and Component Schema
    ASSERT_TRUE(CreateCategory(m_db, "Boxes", ColorDef(255, 0, 0)).IsValid());
    CreateAndImportComponentSchema(m_db);
    m_db->SaveChanges();

    ComponentDefPtr cdef = ComponentDef::FromECSqlName(&status, *m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    ASSERT_TRUE(cdef.IsValid());

    //---------------------------------------------------------------------------------------------------------------
    //  Create the catalog model in the client.
    SpatialModelPtr catalogModel1;
    ASSERT_EQ(DgnDbStatus::Success, createSpatialModel(catalogModel1, *m_db, DgnModel::CreateModelCode("CatelogModel")));
    SpatialModelPtr catalogModel2;
    ASSERT_EQ(DgnDbStatus::Success, createSpatialModel(catalogModel2, *m_db, DgnModel::CreateModelCode("OtherCatelogModel")));
    SpatialModelPtr targetModel;
    ASSERT_EQ(DgnDbStatus::Success, createSpatialModel(targetModel, *m_db, DgnModel::CreateModelCode("Instances")));
    
    // Register Script
    RegisterBoxModelResolverScript();

    //---------------------------------------------------------------------------------------------------------------
    // Create Component variations
    // Create 3 variations in catalogModel1
    DgnElementCPtr variation1 = CreateVariation(cdef, *catalogModel1, DPoint3d::From(20, 20, 20), "Cube");
    ASSERT_TRUE(variation1.IsValid());
    DgnElementCPtr variation2 = CreateVariation(cdef, *catalogModel1, DPoint3d::From(20, 20, 10), "Square Prism");
    ASSERT_TRUE(variation2.IsValid());
    DgnElementCPtr variation3 = CreateVariation(cdef, *catalogModel1, DPoint3d::From(20, 10, 5), "Cuboid");
    ASSERT_TRUE(variation3.IsValid());
    // Create 1 variation in catalogModel 2
    DgnElementCPtr variation4 = CreateVariation(cdef, *catalogModel2, DPoint3d::From(40, 40, 40), "Hexahedron");
    ASSERT_TRUE(variation4.IsValid());

    //---------------------------------------------------------------------------------------------------------------
    // Query variations
        // By Name
    DgnElementCPtr query = cdef->QueryVariationByName("Cuboid");
    ASSERT_TRUE(variation3->GetElementId() == query->GetElementId());

        // By Model
    bvector<DgnElementId> variationList1;
    cdef->QueryVariations(variationList1, catalogModel1->GetModelId());
    EXPECT_EQ(3, variationList1.size());

    bvector<DgnElementId> variationList2;
    cdef->QueryVariations(variationList2, catalogModel2->GetModelId());
    EXPECT_EQ(1, variationList2.size());

    //---------------------------------------------------------------------------------------------------------------
    // Delete Variations
    EXPECT_TRUE( DgnDbStatus::Success == cdef->DeleteVariation(*variation1) );
        // Delete variation which does not exist
    EXPECT_FALSE(DgnDbStatus::Success == cdef->DeleteVariation(*variation1));

/* *** WIP - DeleteVariation is not yet checking for existing instances 
        // Deleting variation which is instantiated should fail
    DgnElementCPtr inst = ComponentDef::MakeInstanceOfVariation(&status, *targetModel, *variation2, nullptr);
    EXPECT_TRUE(inst.IsValid());

    EXPECT_FALSE(DgnDbStatus::Success == cdef->DeleteVariation(*variation2));
*/

    //  catalogModel1 had 3 variations. We deleted 1. So, we expect to find 2 left.
    bvector<DgnElementId> variationList3;
    cdef->QueryVariations(variationList3, catalogModel1->GetModelId());
    EXPECT_EQ(2, variationList3.size());

    m_db->SaveChanges();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, MakeVariationInstance)
    {
    SetupSeedProject();
    ASSERT_TRUE(m_db.IsValid());

    DgnDbStatus status;
    //---------------------------------------------------------------------------------------------------------------
    // Create Category and Component Schema
    ASSERT_TRUE(CreateCategory(m_db, "Boxes", ColorDef(255, 0, 0)).IsValid());
    CreateAndImportComponentSchema(m_db);
    m_db->SaveChanges();

    ComponentDefPtr cdef = ComponentDef::FromECSqlName(&status, *m_db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    ASSERT_TRUE(cdef.IsValid());

    //  Create the target model in the client.
    SpatialModelPtr targetModel;
    ASSERT_EQ(DgnDbStatus::Success, createSpatialModel(targetModel, *m_db, DgnModel::CreateModelCode("Instances")));

    //---------------------------------------------------------------------------------------------------------------
    // Register Scripts
    RegisterBoxModelResolverScript();

    //---------------------------------------------------------------------------------------------------------------
    // Create Variation
    DgnElementCPtr variation1 = CreateVariation(cdef, *targetModel, DPoint3d::From(20, 20, 20), "Cube");
    ASSERT_TRUE(variation1.IsValid());

    //---------------------------------------------------------------------------------------------------------------
    // Create Instances
    DgnElementCPtr inst = ComponentDef::MakeInstanceOfVariation(&status, *targetModel, *variation1, nullptr);
    EXPECT_TRUE(inst.IsValid());

    ECN::IECInstancePtr params = cdef->MakeVariationSpec();
    params->SetValue("H", ECN::ECValue(80));
    params->SetValue("W", ECN::ECValue(80));
    params->SetValue("D", ECN::ECValue(20));
    params->SetValue("box_count", ECN::ECValue(1));
    
    DgnElementCPtr inst2 = ComponentDef::MakeInstanceOfVariation(&status, *targetModel, *variation1, params.get());
    EXPECT_TRUE(inst2.IsValid());

    m_db->SaveChanges();
    }


#endif //ndef BENTLEYCONFIG_NO_JAVASCRIPT
