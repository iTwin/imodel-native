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
* @bsiclass                                                     Umar.Hayat     01/16
+===============+===============+===============+===============+===============+======*/
struct ComponentModelBasicTest : public testing::Test
{
    Dgn::ScopedDgnHost m_host;
    DgnCategoryId CreateCategory(DgnDbPtr& db, Utf8CP code, ColorDef const& color);
    void CreateAndImportComponentSchema(DgnDbPtr& db);
};

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
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelBasicTest::CreateAndImportComponentSchema(DgnDbPtr& db)
    {
    ECN::ECSchemaPtr testSchema = ComponentDefCreator::GenerateSchema(*db, TEST_JS_NAMESPACE);
    ASSERT_TRUE(testSchema.IsValid());

    ECN::ECClassCP baseClass = db->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement);

    ComponentDefCreator creator(*db, *testSchema, TEST_BOXES_COMPONENT_NAME, *baseClass, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME, "Boxes", "");
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("H", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("W", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("D", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("box_count", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    ECN::ECClassCP ecClass = creator.GenerateECClass();
    EXPECT_TRUE(nullptr != ecClass);

    ASSERT_TRUE(nullptr != ComponentDefCreator::ImportSchema(*db, *testSchema, false));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, FromECClass)
    {
    BeFileName componentDbName = DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_FromECSqlName.idgndb");
    DgnDbPtr db;
    DgnDbTestFixture::OpenDb(db, componentDbName, Db::OpenMode::ReadWrite);
    
    ECN::ECSchemaPtr testSchema = ComponentDefCreator::GenerateSchema(*db, TEST_JS_NAMESPACE);
    ASSERT_TRUE(testSchema.IsValid());

    ECN::ECClassCP baseClass = db->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement);

    ComponentDefCreator creator(*db, *testSchema, TEST_BOXES_COMPONENT_NAME, *baseClass, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME, "Boxes", "");
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("H", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("W", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("D", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("box_count", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    ECN::ECClassCP ecClass = creator.GenerateECClass();
    EXPECT_TRUE(nullptr != ecClass);

    ASSERT_TRUE(nullptr != ComponentDefCreator::ImportSchema(*db, *testSchema, false));

    DgnDbStatus status;
    ComponentDefPtr def1 = ComponentDef::FromECClass(&status, *db, *ecClass);
    EXPECT_FALSE(def1.IsValid());
    // Doesn't return status right now, uncomment once fixed
    //EXPECT_TRUE(status == DgnDbStatus::InvalidCategory);

    ASSERT_TRUE(CreateCategory(db, "Boxes", ColorDef(255, 0, 0)).IsValid());

    ComponentDefPtr def2 = ComponentDef::FromECClass(&status, *db, *ecClass);
    EXPECT_TRUE(def2.IsValid());

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, FromECClassId)
    {
    BeFileName componentDbName = DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_FromECSqlName.idgndb");
    DgnDbPtr db;
    DgnDbTestFixture::OpenDb(db, componentDbName, Db::OpenMode::ReadWrite);
    
    ECN::ECSchemaPtr testSchema = ComponentDefCreator::GenerateSchema(*db, TEST_JS_NAMESPACE);
    ASSERT_TRUE(testSchema.IsValid());

    ECN::ECClassCP baseClass = db->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement);

    ComponentDefCreator creator(*db, *testSchema, TEST_BOXES_COMPONENT_NAME, *baseClass, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME, "Boxes", "");
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("H", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("W", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("D", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("box_count", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    ECN::ECClassCP ecClass = creator.GenerateECClass();
    EXPECT_TRUE(nullptr != ecClass);

    ASSERT_TRUE(nullptr != ComponentDefCreator::ImportSchema(*db, *testSchema, false));

    DgnDbStatus status;
    ComponentDefPtr def1 = ComponentDef::FromECClassId(&status, *db, DgnClassId(ecClass->GetId()));
    EXPECT_FALSE(def1.IsValid());
    // Doesn't return status right now, uncomment once fixed
    //EXPECT_TRUE(status == DgnDbStatus::InvalidCategory);

    ASSERT_TRUE(CreateCategory(db, "Boxes", ColorDef(255, 0, 0)).IsValid());

    ComponentDefPtr def2 = ComponentDef::FromECClassId(&status, *db, DgnClassId(ecClass->GetId()));
    EXPECT_TRUE(def2.IsValid());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, FromECSqlName)
    {
    BeFileName componentDbName = DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_FromECSqlName.idgndb");
    DgnDbPtr db;
    DgnDbTestFixture::OpenDb(db, componentDbName, Db::OpenMode::ReadWrite);

    DgnDbStatus status;
    ComponentDefPtr def0 = ComponentDef::FromECSqlName(&status, *db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_FALSE(def0.IsValid());
    // Doesn't return status right now, uncomment once fixed
    //EXPECT_TRUE(status == DgnDbStatus::InvalidCategory);

    // With outstatus Status Nullptr
    def0 = ComponentDef::FromECSqlName(nullptr, *db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_FALSE(def0.IsValid());

    ASSERT_NO_FATAL_FAILURE(CreateAndImportComponentSchema(db));

    ComponentDefPtr def1 = ComponentDef::FromECSqlName(&status, *db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_FALSE(def1.IsValid());

    // Doesn't return status right now, uncomment once fixed
    //EXPECT_TRUE(status == DgnDbStatus::InvalidCategory);
    ASSERT_TRUE(CreateCategory(db, "Boxes", ColorDef(255, 0, 0)).IsValid());

    ComponentDefPtr def2 = ComponentDef::FromECSqlName(&status, *db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(def2.IsValid());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, CreateComponentModel)
    {
    BeFileName componentDbName = DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.idgndb", L"CreateComponentModel.idgndb");
    DgnDbPtr db;
    DgnDbTestFixture::OpenDb(db, componentDbName, Db::OpenMode::ReadWrite);

    ASSERT_NO_FATAL_FAILURE(CreateAndImportComponentSchema(db));

    ComponentModelPtr CM2 = ComponentModel::Create(*db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(CM2.IsValid());

    ASSERT_TRUE(DgnDbStatus::Success == CM2->Insert());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, CreateComponentModel_NoSchema)
    {
    BeFileName componentDbName = DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.idgndb", L"CreateComponentModel_NoSchema.idgndb");
    DgnDbPtr db;
    DgnDbTestFixture::OpenDb(db, componentDbName, Db::OpenMode::ReadWrite);

    ComponentModelPtr CM = ComponentModel::Create(*db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(CM.IsValid());

    ASSERT_TRUE(DgnDbStatus::Success == CM->Insert());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, CreateComponentModel_Redundant)
    {
    BeFileName componentDbName = DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.idgndb", L"CreateComponentModel_Redundant.idgndb");
    DgnDbPtr db;
    DgnDbTestFixture::OpenDb(db, componentDbName, Db::OpenMode::ReadWrite);

    ComponentModelPtr CM = ComponentModel::Create(*db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(CM.IsValid());

    ASSERT_TRUE(DgnDbStatus::Success == CM->Insert());

    ComponentModelPtr CM2 = ComponentModel::Create(*db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(CM2.IsValid());

    ASSERT_TRUE(DgnDbStatus::Success == CM2->Insert());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, FromComponentModel)
    {
    BeFileName componentDbName = DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.idgndb", L"FromComponentModel.idgndb");
    DgnDbPtr db;
    DgnDbTestFixture::OpenDb(db, componentDbName, Db::OpenMode::ReadWrite);

    ComponentModelPtr CM = ComponentModel::Create(*db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(CM.IsValid());

    ASSERT_TRUE(DgnDbStatus::Success == CM->Insert());
    
    DgnDbStatus status;
    // When component class does not exist
    ComponentDefPtr def = ComponentDef::FromComponentModel(&status, *CM);
    EXPECT_FALSE(def.IsValid());
    // TODO: Check status

    ASSERT_NO_FATAL_FAILURE(CreateAndImportComponentSchema(db));

    // Still category does not exist
    ComponentDefPtr def2 = ComponentDef::FromComponentModel(&status, *CM);
    EXPECT_FALSE(def2.IsValid());
    // Doesn't return status right now, uncomment once fixed
    //EXPECT_TRUE(status == DgnDbStatus::InvalidCategory);

    ASSERT_TRUE(CreateCategory(db, "Boxes", ColorDef(255, 0, 0)).IsValid());
    ComponentDefPtr def3 = ComponentDef::FromComponentModel(&status, *CM);
    EXPECT_TRUE(def3.IsValid());

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, DeleteComponentDef)
    {
    BeFileName componentDbName = DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_DeleteComponentDef.idgndb");
    DgnDbPtr db;
    DgnDbTestFixture::OpenDb(db, componentDbName, Db::OpenMode::ReadWrite);

    // Create Category
    ASSERT_TRUE(CreateCategory(db, "Boxes", ColorDef(255, 0, 0)).IsValid());
    DgnDbStatus status = ComponentDef::DeleteComponentDef(*db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(DgnDbStatus::Success != status);

    ASSERT_NO_FATAL_FAILURE(CreateAndImportComponentSchema(db));

    ComponentDefPtr def2 = ComponentDef::FromECSqlName(&status, *db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(def2.IsValid());

    status = ComponentDef::DeleteComponentDef(*db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    ASSERT_TRUE(DgnDbStatus::Success == status);

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, Export)
    {
    BeFileName componentDbName = DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_Export.idgndb");
    BeFileName clientDbName = DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_Export_Client.idgndb");
    DgnDbPtr db,clientDb;
    DgnDbTestFixture::OpenDb(db, componentDbName, Db::OpenMode::ReadWrite);
    DgnDbTestFixture::OpenDb(clientDb, clientDbName, Db::OpenMode::ReadWrite);

    // Create Category and Component Schema
    ASSERT_TRUE(CreateCategory(db, "Boxes", ColorDef(255, 0, 0)).IsValid());
    ASSERT_NO_FATAL_FAILURE(CreateAndImportComponentSchema(db));
    db->SaveChanges();

    DgnDbStatus status;
    ComponentDefPtr def2 = ComponentDef::FromECSqlName(&status, *db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(def2.IsValid());
    
    // Exporting in Same DgnDb
    DgnImportContext context(*db,*db);
    //DgnDbStatus Export(DgnImportContext& context, bool exportSchema = true, bool exportCategory = false);
    status = def2->Export(context, true,true);
    ASSERT_TRUE(DgnDbStatus::Success == status);

    // Exporting in second DgnDb
    DgnImportContext context2(*db, *clientDb);
    status = def2->Export(context2, true, true);
    ASSERT_TRUE(DgnDbStatus::Success == status);
    clientDb->SaveChanges();

    // Export redundantly 
    status = def2->Export(context2, true, true);
    ASSERT_TRUE(DgnDbStatus::Success == status);

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelBasicTest, QueryComponentDefs)
    {
    BeFileName componentDbName = DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_QueryComponentDefs.idgndb");
    DgnDbPtr db;
    DgnDbTestFixture::OpenDb(db, componentDbName, Db::OpenMode::ReadWrite);

    DgnDbStatus status;
    ASSERT_TRUE(CreateCategory(db, "Boxes", ColorDef(255, 0, 0)).IsValid());
    ECN::ECSchemaPtr testSchema = ComponentDefCreator::GenerateSchema(*db, TEST_JS_NAMESPACE);
    ASSERT_TRUE(testSchema.IsValid());

    ECN::ECClassCP baseClass = db->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement);

    ComponentDefCreator creator(*db, *testSchema, TEST_BOXES_COMPONENT_NAME, *baseClass, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME, "Boxes", "");
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("H", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("W", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("D", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    creator.AddPropertySpec(ComponentDefCreator::PropertySpec("box_count", ECN::PrimitiveType::PRIMITIVETYPE_Double, ComponentDef::ParameterVariesPer::Instance));
    ECN::ECClassCP ecClass = creator.GenerateECClass();
    EXPECT_TRUE(nullptr != ecClass);

    ASSERT_TRUE(nullptr != ComponentDefCreator::ImportSchema(*db, *testSchema, false));

    db->SaveChanges();

    ComponentDefPtr def2 = ComponentDef::FromECSqlName(&status, *db, TEST_BOXES_COMPONENT_ECSQLCLASS_NAME);
    EXPECT_TRUE(def2.IsValid());

    DgnImportContext context(*db, *db);
    //DgnDbStatus Export(DgnImportContext& context, bool exportSchema = true, bool exportCategory = false);
    status = def2->Export(context, true, true);
    ASSERT_TRUE(DgnDbStatus::Success == status);
   db->SaveChanges();

    bvector<DgnClassId> componentDefs;
    ComponentDef::QueryComponentDefs(componentDefs, *db, *ecClass); 
    ASSERT_EQ(1 , componentDefs.size());

    }


#endif //ndef BENTLEYCONFIG_NO_JAVASCRIPT
