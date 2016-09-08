/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ComponentModelTest.cpp $
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
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

// *** WARNING: Keep this consistent with ComponentModelTest.ts
#define TEST_JS_NAMESPACE    "ComponentModelTest"
#define TEST_JS_NAMESPACE_W L"ComponentModelTest"
#define TEST_WIDGET_COMPONENT_NAME "Widget"
#define TEST_GADGET_COMPONENT_NAME "Gadget"
#define TEST_THING_COMPONENT_NAME "Thing"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void openDb (DgnDbPtr& db, BeFileNameCR name, DgnDb::OpenMode mode)
    {
    db = DgnDbTestUtils::OpenDgnDb(name, mode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus createSpatialModel(SpatialModelPtr& catalogModel, DgnDbR db, DgnCode const& code)
    {
    catalogModel = DgnDbTestUtils::InsertSpatialModel(db, code);
    return catalogModel.IsValid()? DgnDbStatus::Success : DgnDbStatus::NotFound;
    }

/*=================================================================================**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+===============+===============+===============+===============+===============+======*/
namespace 
{
struct ComponentModelTest_DetectJsErrors : DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler
    {
    void _HandleScriptError(BeJsContextR, Category category, Utf8CP description, Utf8CP details) override
        {
        FAIL() << Utf8PrintfString("JS error %x: %s , %s", (int)category, description, details).c_str();
        }

    void _HandleLogMessage(Utf8CP category, DgnPlatformLib::Host::ScriptAdmin::LoggingSeverity sev, Utf8CP msg) override
        {
        ScriptNotificationHandler::_HandleLogMessage(category, sev, msg);  // logs it
        printf ("%s\n", msg);
        }
    };
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static void checkGeometryStream(GeometrySourceCR gel, GeometricPrimitive::GeometryType exptectedType, size_t expectedCount)
    {
    // Verify that we have 1 part containing two solid primitives
    size_t partCount = 0;
    size_t primitiveCount = 0;
    for (auto outer : GeometryCollection (gel))
        {
        EXPECT_FALSE(outer.GetGeometryPtr().IsValid());
        DgnGeometryPartId partId = outer.GetGeometryPartId();
        EXPECT_TRUE(partId.IsValid());
        if (partId.IsValid())
            {
            ++partCount;
            auto part = gel.GetSourceDgnDb().Elements().Get<DgnGeometryPart>(partId);
            ASSERT_TRUE(part.IsValid());
            for (auto inner : GeometryCollection(part->GetGeometryStream(), gel.GetSourceDgnDb()))
                {
                auto geom = inner.GetGeometryPtr();
                ASSERT_TRUE(geom.IsValid());
                EXPECT_EQ(exptectedType, geom->GetGeometryType());
                if (exptectedType == geom->GetGeometryType())
                    ++primitiveCount;
                }
            }
        }

    ASSERT_EQ( expectedCount , primitiveCount );
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void checkSlabDimensions(GeometrySourceCR el, double expectedX, double expectedY, double expectedZ)
    {
    DgnGeometryPartId partId = (*GeometryCollection(el).begin()).GetGeometryPartId();
    DgnGeometryPartCPtr part = el.GetSourceDgnDb().Elements().Get<DgnGeometryPart>(partId);
    ASSERT_TRUE(part.IsValid());

    DgnBoxDetail box;
    ASSERT_TRUE( (*(GeometryCollection(part->GetGeometryStream(), el.GetSourceDgnDb()).begin())).GetGeometryPtr()->GetAsISolidPrimitive()->TryGetDgnBoxDetail(box) ) << "Geometry should be a slab";
    EXPECT_EQ( expectedX, box.m_baseX );
    EXPECT_EQ( expectedY, box.m_baseY );
    EXPECT_DOUBLE_EQ( expectedZ, box.m_topOrigin.Distance(box.m_baseOrigin) );
    }

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson     12/2015
+===============+===============+===============+===============+===============+======*/
struct VariationSpec
{
    Utf8String m_componentName;
    Utf8String m_name;
    TsComponentParameterSet m_params;
    bvector<Utf8String> m_slabDimensions;

    VariationSpec() {;}
    VariationSpec(Utf8StringCR cn, Utf8StringCR n) : m_componentName(cn), m_name(n) {;}
    //VariationSpec(VariationSpec const& rhs) : m_componentName(rhs.m_componentName), m_name(rhs.m_name), m_params(rhs.m_params) {;}

    ECN::IECInstancePtr MakeVariationSpec(DgnDbR db) const;
    void CheckInstance(DgnElementCR el, size_t expectedSolidCount) const;
    void MakeUniqueInstance(DgnElementCPtr&, DgnModelR destModel, size_t expectedSolidCount);
    void MakeVariation(DgnElementCPtr&, SpatialModelR destModel);

    void SetValue(Utf8CP name, ECN::ECValueCR v) {m_params[name].m_value = v;}

    void Dump(Utf8CP title) const;
};

void VariationSpec::Dump(Utf8CP title) const
    {
    BeTest::Log("ComponentModelTest", BeTest::LogPriority::PRIORITY_FATAL, title);
    for (auto entry : m_params)
        {
        BeTest::Log("ComponentModelTest", BeTest::LogPriority::PRIORITY_FATAL, Utf8PrintfString("%s - %s", entry.first.c_str(), entry.second.m_value.ToString().c_str()).c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr VariationSpec::MakeVariationSpec(DgnDbR db) const
    {
#ifdef DEBUG_COMPONENT_MODEL_TEST
    BeTest::Log("ComponentModelTest", BeTest::LogPriority::PRIORITY_FATAL, Utf8PrintfString("%s - MakeVariationSpec", m_name.c_str()).c_str());
    Dump("The VariationSpec");
#endif

    ComponentDefPtr cdef = ComponentDef::FromECClass(nullptr, db, *db.Schemas().GetECClass(TEST_JS_NAMESPACE, m_componentName.c_str()));
    if (!cdef.IsValid())
        return nullptr;
    ECN::IECInstancePtr instance = cdef->MakeVariationSpec();
    m_params.ToECProperties(*instance);

#ifdef DEBUG_COMPONENT_MODEL_TEST
    ComponentDef::DumpScriptOnlyParameters(*instance, "Test - set my properties");
#endif

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void VariationSpec::CheckInstance(DgnElementCR el, size_t expectedSolidCount) const
    {
    ComponentDefPtr cdef = ComponentDef::FromECClass(nullptr, el.GetDgnDb(), *el.GetElementClass());
    ASSERT_TRUE(cdef.IsValid());
    ASSERT_STREQ(cdef->GetName().c_str(), m_componentName.c_str());
    checkGeometryStream(*el.ToGeometrySource(), GeometricPrimitive::GeometryType::SolidPrimitive, expectedSolidCount);
    checkSlabDimensions(*el.ToGeometrySource(), m_params.find(m_slabDimensions[0])->second.m_value.GetDouble(), 
                                                m_params.find(m_slabDimensions[1])->second.m_value.GetDouble(), 
                                                m_params.find(m_slabDimensions[2])->second.m_value.GetDouble());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void VariationSpec::MakeUniqueInstance(DgnElementCPtr& inst, DgnModelR destModel, size_t expectedSolidCount)
    {
#ifdef DEBUG_COMPONENT_MODEL_TEST
    BeTest::Log("ComponentModelTest", BeTest::LogPriority::PRIORITY_FATAL, Utf8PrintfString("%s MakeUniqueInstance", m_name.c_str()).c_str());
#endif

    DgnDbR db = destModel.GetDgnDb();
    inst = ComponentDef::MakeUniqueInstance(nullptr, destModel, *MakeVariationSpec(db));
    ASSERT_TRUE(inst.IsValid()); 
    CheckInstance(*inst, expectedSolidCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void VariationSpec::MakeVariation(DgnElementCPtr& variation, SpatialModelR destModel)
    {
    DgnDbStatus status;
    auto vspec = MakeVariationSpec(destModel.GetDgnDb());
    ASSERT_TRUE(vspec.IsValid());
    variation = ComponentDef::MakeVariation(&status, destModel, *vspec, m_name);
    ASSERT_TRUE(variation.IsValid()) << Utf8PrintfString("MakeVariation failed with error %x", status);
    }

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson     02/2012
+===============+===============+===============+===============+===============+======*/
struct ComponentModelTest : public testing::Test, ScopedDgnHost::FetchScriptCallback
{
BeFileName         m_componentDbName;
BeFileName         m_componentSchemaFileName;
BeFileName         m_clientDbName;
DgnDbPtr           m_componentDb;
DgnDbPtr           m_clientDb;
Dgn::ScopedDgnHost m_host;
VariationSpec m_wsln1, m_wsln3, m_wsln4, m_wsln44, m_gsln1, m_nsln1;

Dgn::DgnDbStatus _FetchScript(Utf8StringR sText, DgnScriptType& stypeFound, DateTime& lmt, DgnDbR, Utf8CP sName, DgnScriptType stypePreferred) override;

ComponentModelTest();
void OpenComponentDb(DgnDb::OpenMode mode) {openDb(m_componentDb, m_componentDbName, mode);}
void CloseComponentDb() {if (m_componentDb.IsValid()) m_componentDb->CloseDb(); m_componentDb=nullptr;}
void OpenClientDb(DgnDb::OpenMode mode) {openDb(m_clientDb, m_clientDbName, mode);}
void CloseClientDb() {if (m_clientDb.IsValid()) m_clientDb->CloseDb(); m_clientDb=nullptr;}
DgnCategoryId Developer_CreateCategory(Utf8CP code, ColorDef const&);
void Developer_DefineSchema();
void Client_ImportComponentDef(Utf8CP componentName);
void Client_InsertNonInstanceElement(Utf8CP modelName, Utf8CP code = nullptr);
void Client_PlaceInstanceOfVariation(DgnElementId&, Utf8StringCR targetModelName, DgnElementCR variation);
void Client_PlaceInstance(DgnElementId&, Utf8CP targetModelName, SpatialModelR catalogModel, Utf8StringCR componentName, Utf8StringCR variationName, bool expectToFindVariation);
void Client_CheckComponentInstance(DgnElementId eid, size_t expectedSolidCount, VariationSpec const& vc);
void Client_CheckNestedInstance(DgnElementCR instanceElement, Utf8CP expectedChildComponentName, int nChildrenExpected);

void SimulateDeveloper();
void SimulateClient();
};

struct AutoCloseClientDb
{
ComponentModelTest& m_test;

AutoCloseClientDb(ComponentModelTest& t) : m_test(t) {;}
~AutoCloseClientDb() {m_test.CloseClientDb();}
};

struct AutoCloseComponentDb
{
ComponentModelTest& m_test;

AutoCloseComponentDb(ComponentModelTest& t) : m_test(t) {;}
~AutoCloseComponentDb() {m_test.CloseComponentDb();}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentModelTest::ComponentModelTest()
    {
    T_HOST.GetScriptAdmin().RegisterScriptNotificationHandler(*new ComponentModelTest_DetectJsErrors);
    m_host.SetFetchScriptCallback(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Developer_DefineSchema()
    {
    OpenComponentDb(Db::OpenMode::ReadWrite);
    AutoCloseComponentDb closeComponentDb(*this);
    
    ECN::ECSchemaPtr testSchema = ComponentDefCreator::GenerateSchema(*m_componentDb, TEST_JS_NAMESPACE);
    ASSERT_TRUE(testSchema.IsValid());

    ECN::ECClassCP baseClass = m_componentDb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement);

    // Widget
        {
        Developer_CreateCategory("WidgetCategory", ColorDef(0xff0000FF));

        TsComponentParameterSet params;
        params["X"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));
        params["Y"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));
        params["Z"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));

        ComponentDefCreator creator(*m_componentDb, *testSchema, TEST_WIDGET_COMPONENT_NAME, *baseClass, TEST_JS_NAMESPACE "." TEST_WIDGET_COMPONENT_NAME, "WidgetCategory", "", params);
        ECN::ECClassCP ecClass = creator.GenerateECClass();
        ASSERT_TRUE(nullptr != ecClass);

        m_wsln1 = VariationSpec(TEST_WIDGET_COMPONENT_NAME, "wsln1");
        m_wsln1.m_params = params;
        m_wsln1.m_slabDimensions.push_back("X");
        m_wsln1.m_slabDimensions.push_back("Y");
        m_wsln1.m_slabDimensions.push_back("Z");

        m_wsln3 = VariationSpec(TEST_WIDGET_COMPONENT_NAME, "wsln3");
        m_wsln3.m_params = m_wsln1.m_params;
        m_wsln3.m_slabDimensions = m_wsln1.m_slabDimensions;
        m_wsln3.m_params["X"].m_value = ECN::ECValue(100.0);
        ASSERT_DOUBLE_EQ(100.0, m_wsln3.m_params["X"].m_value.GetDouble());
        ASSERT_STREQ("100", m_wsln3.m_params["X"].m_value.ToString().c_str());

        m_wsln4 = VariationSpec(TEST_WIDGET_COMPONENT_NAME, "wsln4");
        m_wsln4.m_params = m_wsln3.m_params;
        m_wsln4.m_slabDimensions = m_wsln1.m_slabDimensions;
        m_wsln4.m_params["X"].m_value = ECN::ECValue(2.0);
        ASSERT_DOUBLE_EQ(2.0, m_wsln4.m_params["X"].m_value.GetDouble());

        m_wsln44 = VariationSpec(TEST_WIDGET_COMPONENT_NAME, "wsln44");
        m_wsln44.m_params = m_wsln4.m_params;
        m_wsln44.m_slabDimensions = m_wsln1.m_slabDimensions;
        m_wsln44.m_params["X"].m_value = ECN::ECValue(44.0);
        ASSERT_DOUBLE_EQ(44.0, m_wsln44.m_params["X"].m_value.GetDouble());
    }

    // Gadget
        {
        Developer_CreateCategory("GadgetCategory", ColorDef(0x00ff00FF));

        TsComponentParameterSet params;
        params["Q"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));
        params["W"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));
        params["R"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));
        params["T"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue("text"));

        ComponentDefCreator creator(*m_componentDb, *testSchema, TEST_GADGET_COMPONENT_NAME, *baseClass, TEST_JS_NAMESPACE "." TEST_GADGET_COMPONENT_NAME, "GadgetCategory", "", params);
        ECN::ECClassCP ecClass = creator.GenerateECClass();
        ASSERT_TRUE(nullptr != ecClass);

        m_gsln1 = VariationSpec(TEST_GADGET_COMPONENT_NAME, "gsln1");
        m_gsln1.m_params = params;
        m_gsln1.m_slabDimensions.push_back("Q");
        m_gsln1.m_slabDimensions.push_back("W");
        m_gsln1.m_slabDimensions.push_back("R");
        }

    // Thing
        {
        Developer_CreateCategory("ThingCategory", ColorDef(0x0000ffFF));

        TsComponentParameterSet params;
        params["A"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));
        params["B"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));
        params["C"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));

        ComponentDefCreator creator(*m_componentDb, *testSchema, TEST_THING_COMPONENT_NAME, *baseClass, TEST_JS_NAMESPACE "." TEST_THING_COMPONENT_NAME, "ThingCategory", "", params);
        ECN::ECClassCP ecClass = creator.GenerateECClass();
        ASSERT_TRUE(nullptr != ecClass);

        m_nsln1 = VariationSpec(TEST_THING_COMPONENT_NAME, "nsln1");
        m_nsln1.m_params = params;
        m_nsln1.m_slabDimensions.push_back("A");
        m_nsln1.m_slabDimensions.push_back("B");
        m_nsln1.m_slabDimensions.push_back("C");
        }

    StopWatch timer;
    timer.Start();
    ASSERT_TRUE(ComponentDefCreator::ImportSchema(*m_componentDb, *testSchema, false) != nullptr);
    timer.Stop();
    BeTest::Log("Performance", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("ComponentModelTest - Import component definition schema: %lf seconds", timer.GetElapsedSeconds()).c_str());

    //  Verify that we can look up an existing component
    //ComponentDefPtr widgetCDef = ComponentDef::FromECSqlName(nullptr, *m_componentDb, TEST_JS_NAMESPACE "." TEST_WIDGET_COMPONENT_NAME);
    //ASSERT_TRUE(widgetCDef.IsValid());
    //ASSERT_STREQ(TEST_WIDGET_COMPONENT_NAME, widgetCDef->GetName().c_str());
    //ASSERT_STREQ("WidgetCategory", widgetCDef->GetCategoryName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus ComponentModelTest::_FetchScript(Utf8StringR sText, DgnScriptType& stypeFound, DateTime& lmt, DgnDbR, Utf8CP sName, DgnScriptType stypePreferred)
    {
    BeFileName jsFileName;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(jsFileName);
    jsFileName.AppendToPath(L"Script");
    jsFileName.AppendToPath(WString(sName,BentleyCharEncoding::Utf8).c_str());
    if (jsFileName.find(L".js") == WString::npos)
        jsFileName.append(L".js");
    stypeFound = DgnScriptType::JavaScript;
    lmt = DateTime();
    return DgnScriptLibrary::ReadText(sText, jsFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId ComponentModelTest::Developer_CreateCategory(Utf8CP code, ColorDef const& color)
    {
    DgnCategory cat(DgnCategory::CreateParams(*m_componentDb, code, DgnCategory::Scope::Any));
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);
    cat.Insert(appearance);
    return cat.GetCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_ImportComponentDef(Utf8CP componentName)
    {
    OpenComponentDb(Db::OpenMode::Readonly);
    AutoCloseComponentDb closeComponentDb(*this);

    m_componentDb->Schemas().GetECSchema(TEST_JS_NAMESPACE, true);

    //  ------------------------
    //  Copy in the ComponentDef's ECClass
    //  ------------------------
    m_clientDb->SaveChanges();

    ComponentDefPtr sourceCdef = ComponentDef::FromECSqlName(nullptr, *m_componentDb, Utf8PrintfString("%s.%s", TEST_JS_NAMESPACE, componentName));

    DgnImportContext ctx(*m_componentDb, *m_clientDb);
    StopWatch timer;
    timer.Start();
    ASSERT_EQ( DgnDbStatus::Success , sourceCdef->Export(ctx));
    timer.Stop();
    BeTest::Log("Performance", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("ComponentModelTest - Import component model and schema: %lf seconds", timer.GetElapsedSeconds()).c_str());

    m_clientDb->SaveChanges();

    ComponentDefPtr importedCdef = ComponentDef::FromECSqlName(nullptr, *m_clientDb, Utf8PrintfString("%s.%s", TEST_JS_NAMESPACE, componentName));
    ASSERT_TRUE(importedCdef.IsValid());
    
    //  ------------------------
    //  Copy in the variations
    //  ------------------------
    SpatialModelPtr sourceCatalogModel = DgnDbTestUtils::GetModelByName<SpatialModel>(*m_componentDb, "Catalog");
    ASSERT_TRUE(sourceCatalogModel.IsValid());

    SpatialModelPtr catalogModel = DgnDbTestUtils::GetModelByName<SpatialModel>(*m_clientDb, "Catalog");
    if (!catalogModel.IsValid())
        createSpatialModel(catalogModel, *m_clientDb, DgnModel::CreateModelCode("Catalog"));

    ASSERT_TRUE(catalogModel.IsValid());

    bvector<DgnElementId> originalVariations, importedVariations;
    sourceCdef->QueryVariations(originalVariations, sourceCatalogModel->GetModelId());
    importedCdef->QueryVariations(importedVariations, catalogModel->GetModelId());
    ASSERT_EQ( 0 , importedVariations.size() );

    ASSERT_EQ( DgnDbStatus::Success , sourceCdef->ExportVariations(*catalogModel, sourceCatalogModel->GetModelId(), ctx) );
    
    importedCdef->QueryVariations(importedVariations, catalogModel->GetModelId());
    ASSERT_NE( 0 , importedVariations.size() );
    ASSERT_EQ( originalVariations.size() , importedVariations.size() );

    m_clientDb->SaveChanges();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_CheckComponentInstance(DgnElementId eid, size_t expectedSolidCount, VariationSpec const& vc)
    {
    DgnElementCPtr el = m_clientDb->Elements().Get<DgnElement>(eid);
    vc.CheckInstance(*el, expectedSolidCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_PlaceInstanceOfVariation(DgnElementId& ieid, Utf8StringCR targetModelName, DgnElementCR variation)
    {
    ASSERT_TRUE(m_clientDb.IsValid() && "Caller must have already opened the Client DB");

    SpatialModelPtr targetModel = DgnDbTestUtils::GetModelByName<SpatialModel>(*m_clientDb, targetModelName);
    ASSERT_TRUE( targetModel.IsValid() );

    DgnDbStatus status;
    DgnElementCPtr instanceElement = ComponentDef::MakeInstanceOfVariation(&status, *targetModel, variation, nullptr);
    ASSERT_TRUE(instanceElement.IsValid()) << Utf8PrintfString("MakeInstanceOfVariation failed with error code %x", status);

    ieid = instanceElement->GetElementId();

    ASSERT_EQ( BE_SQLITE_OK , m_clientDb->SaveChanges() );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_PlaceInstance(DgnElementId& ieid, Utf8CP targetModelName, SpatialModelR catalogModel, Utf8StringCR componentName, Utf8StringCR variationName, bool expectToFindVariation)
    {
    ASSERT_TRUE(m_clientDb.IsValid() && "Caller must have already opened the Client DB");


    ComponentDefPtr cdef = ComponentDef::FromECClass(nullptr, catalogModel.GetDgnDb(), *catalogModel.GetDgnDb().Schemas().GetECClass(TEST_JS_NAMESPACE, componentName.c_str()));
    ASSERT_TRUE(cdef.IsValid());

    DgnElementCPtr variation = cdef->QueryVariationByName(variationName);

    if (!expectToFindVariation)
        {
        ASSERT_FALSE(variation.IsValid());
        return;
        }

    ASSERT_TRUE(variation.IsValid());
    Client_PlaceInstanceOfVariation(ieid, targetModelName, *variation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_InsertNonInstanceElement(Utf8CP modelName, Utf8CP code)
    {
    SpatialModelPtr targetModel = DgnDbTestUtils::GetModelByName<SpatialModel>(*m_clientDb, modelName);
    ASSERT_TRUE( targetModel.IsValid() );
    DgnCategoryId catid = DgnCategory::QueryHighestCategoryId(*m_clientDb);
    auto el = GenericPhysicalObject::Create(*targetModel, catid);
    ASSERT_TRUE( el.IsValid() );
    ASSERT_TRUE( el->Insert().IsValid() );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::SimulateDeveloper()
    {
    //  Simulate a customizer who creates a component definition 
    Developer_DefineSchema();   // first, the customizer defines his schema

    // Create catalogs of components
    DgnElementId wsln1UniqueInstanceId, wsln1VariationId;
        {
        OpenComponentDb(Db::OpenMode::ReadWrite);
        AutoCloseComponentDb closeComponentDb(*this);

        //  Test the components
        SpatialModelPtr tmpModel;
        createSpatialModel(tmpModel, *m_componentDb, DgnModel::CreateModelCode("tmp"));

        DgnElementCPtr inst;
        m_wsln1.MakeUniqueInstance(inst, *tmpModel, 2);
        wsln1UniqueInstanceId = inst->GetElementId();
        m_wsln3.MakeUniqueInstance(inst, *tmpModel, 2);
        m_wsln4.MakeUniqueInstance(inst, *tmpModel, 2);
        m_wsln44.MakeUniqueInstance(inst, *tmpModel, 2);
        m_gsln1.MakeUniqueInstance(inst, *tmpModel, 1);
        m_nsln1.MakeUniqueInstance(inst, *tmpModel, 1);
        inst = nullptr;

        tmpModel->Delete();

        //  Create catalogs

        SpatialModelPtr catalogModel;
        ASSERT_EQ( DgnDbStatus::Success , createSpatialModel(catalogModel, *m_componentDb, DgnModel::CreateModelCode("Catalog")) );

        if (true)
            {
            ComponentDefPtr thingCdef = ComponentDef::FromECSqlName(nullptr, *m_componentDb, TEST_JS_NAMESPACE "." TEST_THING_COMPONENT_NAME);
            ASSERT_TRUE(thingCdef.IsValid());
            bvector<DgnElementId> thingVariations;
            thingCdef->QueryVariations(thingVariations, catalogModel->GetModelId());
            ASSERT_EQ(0, thingVariations.size()) << "no variations should exist yet";
            }

        DgnElementCPtr var;
        m_wsln1.MakeVariation(var, *catalogModel); 
        wsln1VariationId = var->GetElementId();
        m_wsln3.MakeVariation(var, *catalogModel); 
        m_wsln4.MakeVariation(var, *catalogModel); 
        m_wsln44.MakeVariation(var, *catalogModel); 
        m_gsln1.MakeVariation(var, *catalogModel); 
        m_nsln1.MakeVariation(var, *catalogModel);

        if (true)
            {
            ComponentDefPtr thingCdef = ComponentDef::FromECSqlName(nullptr, *m_componentDb, TEST_JS_NAMESPACE "." TEST_THING_COMPONENT_NAME);
            ASSERT_TRUE(thingCdef.IsValid());
            bvector<DgnElementId> thingVariations;
            thingCdef->QueryVariations(thingVariations, catalogModel->GetModelId());
            ASSERT_EQ(1, thingVariations.size()) << "1 variation of thing should have been created";
            }

        m_componentDb->SaveChanges();
        }

    //  Double-check the class of the instances created by the component def 
        {
        OpenComponentDb(Db::OpenMode::Readonly);
        AutoCloseComponentDb closeComponentDb(*this);

        ComponentDefPtr widgetCDef = ComponentDef::FromECSqlName(nullptr, *m_componentDb, TEST_JS_NAMESPACE "." TEST_WIDGET_COMPONENT_NAME);
        ASSERT_TRUE(widgetCDef.IsValid());

        DgnElementCPtr var = m_componentDb->Elements().GetElement(wsln1VariationId);
        ASSERT_EQ(&widgetCDef->GetECClass(), var->GetElementClass());
        }

    //  Exercise QueryComponentDefs
        {
        OpenComponentDb(Db::OpenMode::Readonly);
        AutoCloseComponentDb closeComponentDb(*this);

        DgnClassId widgetClassId = DgnClassId(m_componentDb->Schemas().GetECClassId(TEST_JS_NAMESPACE, TEST_WIDGET_COMPONENT_NAME));
        DgnClassId gadgetClassId = DgnClassId(m_componentDb->Schemas().GetECClassId(TEST_JS_NAMESPACE, TEST_GADGET_COMPONENT_NAME));
        DgnClassId  thingClassId = DgnClassId(m_componentDb->Schemas().GetECClassId(TEST_JS_NAMESPACE,  TEST_THING_COMPONENT_NAME));

        bvector<DgnClassId> componentClassIds;
        ComponentDef::QueryComponentDefs(componentClassIds, *m_componentDb, *m_componentDb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement));
        ASSERT_EQ(3, componentClassIds.size());
        ASSERT_TRUE(std::find(componentClassIds.begin(), componentClassIds.end(), widgetClassId) != componentClassIds.end());
        ASSERT_TRUE(std::find(componentClassIds.begin(), componentClassIds.end(), gadgetClassId) != componentClassIds.end());
        ASSERT_TRUE(std::find(componentClassIds.begin(), componentClassIds.end(), thingClassId) != componentClassIds.end());
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_CheckNestedInstance(DgnElementCR instanceElement, Utf8CP expectedChildComponentName, int nChildrenExpected)
    {
    auto instanceChildren = instanceElement.QueryChildren();
    ASSERT_EQ(nChildrenExpected, instanceChildren.size());
    if (0 == nChildrenExpected)
        return;

    DgnElementCPtr nestedInstance = m_clientDb->Elements().GetElement(*instanceChildren.begin());
    ASSERT_TRUE(nestedInstance.IsValid());

    ComponentDefPtr cdef = ComponentDef::FromECClass(nullptr, nestedInstance->GetDgnDb(), *nestedInstance->GetElementClass());
    ASSERT_TRUE(cdef.IsValid());

    ASSERT_STREQ(expectedChildComponentName, cdef->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* Simulate a client who imports a ComponentDef and then places instances of solutions to it.
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::SimulateClient()
    {
    DgnElementId w1, w2, w3;

    OpenClientDb(Db::OpenMode::ReadWrite);
        {
        AutoCloseClientDb closeClientDbAtEnd(*this);
        //  Create the target model in the client. (Do this first, so that the first imported CM's will get a model id other than 1. Hopefully, that will help us catch more bugs.)
        SpatialModelPtr targetModel;
        ASSERT_EQ( DgnDbStatus::Success , createSpatialModel(targetModel, *m_clientDb, DgnModel::CreateModelCode("Instances")) );

        //  Add a few unrelated elements to the target model. That way, the first placed CM instance will get an element id other than 1. Hopefully, that will help us catch more bugs.
        for (int i=0; i<10; ++i)
            Client_InsertNonInstanceElement("Instances");

        //  Once per component, import the component def
        Client_ImportComponentDef(TEST_WIDGET_COMPONENT_NAME);

        //  Exercise QueryComponentDefs
            {
            // Note that when we import the schema, all 3 ecclasses are imported. Therefore, we find 3 components, not just one.
            // We have not yet imported the variations or the models of the other components, however.
            OpenComponentDb(Db::OpenMode::Readonly);
            AutoCloseComponentDb closeComponentDb(*this);

            DgnClassId widgetClassId = DgnClassId(m_componentDb->Schemas().GetECClassId(TEST_JS_NAMESPACE, TEST_WIDGET_COMPONENT_NAME));
            DgnClassId gadgetClassId = DgnClassId(m_componentDb->Schemas().GetECClassId(TEST_JS_NAMESPACE, TEST_GADGET_COMPONENT_NAME));
            DgnClassId  thingClassId = DgnClassId(m_componentDb->Schemas().GetECClassId(TEST_JS_NAMESPACE,  TEST_THING_COMPONENT_NAME));

            bvector<DgnClassId> componentClassIds;
            ComponentDef::QueryComponentDefs(componentClassIds, *m_componentDb, *m_componentDb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement));
            ASSERT_EQ(3, componentClassIds.size());
            ASSERT_TRUE(std::find(componentClassIds.begin(), componentClassIds.end(), widgetClassId) != componentClassIds.end());
            ASSERT_TRUE(std::find(componentClassIds.begin(), componentClassIds.end(), gadgetClassId) != componentClassIds.end());
            ASSERT_TRUE(std::find(componentClassIds.begin(), componentClassIds.end(), thingClassId)  != componentClassIds.end());
            }

        SpatialModelPtr catalogModel = DgnDbTestUtils::GetModelByName<SpatialModel>(*m_clientDb, "Catalog");
        ASSERT_TRUE( catalogModel.IsValid() ) << "importing component should also import its catalog";

        // Now start placing instances of Widgets
        Client_PlaceInstance(w1, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, m_wsln1.m_name, true);
        Client_PlaceInstance(w2, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, m_wsln1.m_name, true);
        ASSERT_TRUE( w1.IsValid() );
        ASSERT_TRUE( w2.IsValid() );
        ASSERT_NE( w1.GetValue() , w2.GetValue() );
        Client_CheckComponentInstance(w1, 2, m_wsln1);
        Client_CheckComponentInstance(w2, 2, m_wsln1); // 2nd instance of same solution should have the same instance geometry
    
        DgnElementId noidexpected;
        Client_PlaceInstance(noidexpected, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, "no_such_solution", false);
        ASSERT_FALSE(noidexpected.IsValid());

        //  Add a few unrelated elements to the target model. That way, the first placed CM instance will get an element id other than 1. Hopefully, that will help us catch more bugs.
        for (int i=0; i<5; ++i)
            Client_InsertNonInstanceElement("Instances");

        Client_PlaceInstance(w3, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, m_wsln3.m_name, true);
    
        Client_CheckComponentInstance(w3, 2, m_wsln3);
        Client_CheckComponentInstance(w1, 2, m_wsln1);  // new instance of new solution should not affect existing instances of other solutions

        //  new instance of new solution should not affect existing instances of other solutions
        if (true)
            {
            DgnElementId w1_second_time;
            Client_PlaceInstance(w1_second_time, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, m_wsln1.m_name, true);
            ASSERT_TRUE(w1_second_time.IsValid());
            Client_CheckComponentInstance(w1_second_time, 2, m_wsln1);  // new instance of new solution should not affect existing instances of other solutions
            }

        // Just for a little variety, close the client Db and re-open it
        catalogModel = nullptr;
        }

    ASSERT_TRUE(w1.IsValid());
    ASSERT_TRUE(w2.IsValid());
    ASSERT_TRUE(w3.IsValid());

    OpenClientDb(Db::OpenMode::ReadWrite);
        {
        AutoCloseClientDb closeClientDbAtEnd(*this);
        SpatialModelPtr catalogModel = DgnDbTestUtils::GetModelByName<SpatialModel>(*m_clientDb, "Catalog");

        DgnElementId w4;
        Client_PlaceInstance(w4, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, m_wsln4.m_name, true);
        ASSERT_TRUE(w4.IsValid());

        Client_CheckComponentInstance(w4, 2, m_wsln4);
        Client_CheckComponentInstance(w1, 2, m_wsln1);  // new instance of new solution should not affect existing instances of other solutions

        // Now start placing instances of Gadgets
        Client_ImportComponentDef(TEST_GADGET_COMPONENT_NAME);

        DgnElementId g1, g2;
        Client_PlaceInstance(g1, "Instances", *catalogModel, TEST_GADGET_COMPONENT_NAME, m_gsln1.m_name, true);
        Client_PlaceInstance(g2, "Instances", *catalogModel, TEST_GADGET_COMPONENT_NAME, m_gsln1.m_name, true);
        ASSERT_TRUE(g1.IsValid());
        ASSERT_TRUE(g2.IsValid());

        Client_CheckComponentInstance(g1, 1, m_gsln1);
        Client_CheckComponentInstance(g2, 1, m_gsln1);

        //  And place another Widget
        DgnElementId w44;
        Client_PlaceInstance(w44, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, m_wsln44.m_name, true);
        ASSERT_TRUE(w44.IsValid());

        Client_CheckComponentInstance(w3, 2, m_wsln3);
        Client_CheckComponentInstance(w1, 2, m_wsln1);
        Client_CheckComponentInstance(g1, 1, m_gsln1);
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelTest, SimulateDeveloperAndClient)
    {
    StopWatch timer;
    timer.Start();

    // For the purposes of this test, we'll put the Component and Client models in different DgnDbs
    DgnDbTestUtils::SeedDbInfo rootInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, false));

    m_componentDbName.SetName(L"ComponentModelTest/SimulateDeveloperAndClient_Component.dgndb");
    DgnDbTestUtils::MakeSeedDbCopy(m_componentDbName, rootInfo.fileName, m_componentDbName);
    
    m_clientDbName.SetName(L"ComponentModelTest/SimulateDeveloperAndClient_Client.dgndb");
    DgnDbTestUtils::MakeSeedDbCopy(m_clientDbName, rootInfo.fileName, m_clientDbName);
    
    timer.Stop();
    BeTest::Log("Performance", BeTest::LogPriority::PRIORITY_INFO, Utf8PrintfString("ComponentModelTest - Create seed Dbs (with imported schemas): %lf seconds", timer.GetElapsedSeconds()).c_str());

    BeTest::GetHost().GetOutputRoot(m_componentSchemaFileName);
    m_componentSchemaFileName.AppendToPath(TEST_JS_NAMESPACE_W L"0.0.ECSchema.xml");

    SimulateDeveloper();
    SimulateClient();
    }

/*---------------------------------------------------------------------------------**//**
* Make a unique/singleton instance of the 'Thing' component.
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelTest, SimulateDeveloperAndClientWithNestingSingleton)
    {
    // For the purposes of this test, we'll put the Component and Client models in different DgnDbs
    DgnDbTestUtils::SeedDbInfo rootInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, false));

    m_componentDbName.SetName(L"ComponentModelTest/SimulateDeveloperAndClientWithNestingSingleton_Component.dgndb");
    DgnDbTestUtils::MakeSeedDbCopy(m_componentDbName, rootInfo.fileName, m_componentDbName);
    
    m_clientDbName.SetName(L"ComponentModelTest/SimulateDeveloperAndClientWithNestingSingleton_Client.dgndb");
    DgnDbTestUtils::MakeSeedDbCopy(m_clientDbName, rootInfo.fileName, m_clientDbName);

    BeTest::GetHost().GetOutputRoot(m_componentSchemaFileName);
    m_componentSchemaFileName.AppendToPath(TEST_JS_NAMESPACE_W L"0.0.ECSchema.xml");

    SimulateDeveloper();

    //  Create the target model in the client. (Do this first, so that the first imported CM's will get a model id other than 1. Hopefully, that will help us catch more bugs.)
    if(true)
        {
        OpenClientDb(Db::OpenMode::ReadWrite);
        AutoCloseClientDb closeClientDbAtEnd(*this);
        
        Client_ImportComponentDef(TEST_GADGET_COMPONENT_NAME);
        }

    OpenClientDb(Db::OpenMode::ReadWrite);
    AutoCloseClientDb closeClientDbAtEnd(*this);

    Client_ImportComponentDef(TEST_THING_COMPONENT_NAME);

    SpatialModelPtr targetModel;
    ASSERT_EQ( DgnDbStatus::Success , createSpatialModel(targetModel, *m_clientDb, DgnModel::CreateModelCode("Instances")) );

    VariationSpec nparms = m_nsln1;
    nparms.m_params["A"].m_value.SetDouble(9999);

    DgnElementCPtr instanceElement;
    nparms.MakeUniqueInstance(instanceElement, *targetModel, 1);
    ASSERT_TRUE(instanceElement.IsValid());

    Client_CheckNestedInstance(*instanceElement, TEST_GADGET_COMPONENT_NAME, 1);

    ASSERT_EQ( BE_SQLITE_OK , m_clientDb->SaveChanges() );
    }

/*---------------------------------------------------------------------------------**//**
* Make an instance of a pre-captured solution to the 'Thing' component.
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelTest, SimulateDeveloperAndClientWithNesting)
    {
    // For the purposes of this test, we'll put the Component and Client models in different DgnDbs
    DgnDbTestUtils::SeedDbInfo rootInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, false));

    m_componentDbName.SetName(L"ComponentModelTest/SimulateDeveloperAndClientWithNesting_Component.dgndb");
    DgnDbTestUtils::MakeSeedDbCopy(m_componentDbName, rootInfo.fileName, m_componentDbName);
    
    m_clientDbName.SetName(L"ComponentModelTest/SimulateDeveloperAndClientWithNesting_Client.dgndb");
    DgnDbTestUtils::MakeSeedDbCopy(m_clientDbName, rootInfo.fileName, m_clientDbName);

    BeTest::GetHost().GetOutputRoot(m_componentSchemaFileName);
    m_componentSchemaFileName.AppendToPath(TEST_JS_NAMESPACE_W L"0.0.ECSchema.xml");

    SimulateDeveloper();
    
    if (true)
        {
        OpenClientDb(Db::OpenMode::ReadWrite);
        AutoCloseClientDb closeClientDbAtEnd(*this);

        Client_ImportComponentDef(TEST_THING_COMPONENT_NAME);

        ComponentDefPtr thingCdef = ComponentDef::FromECSqlName(nullptr, *m_clientDb, TEST_JS_NAMESPACE "." TEST_THING_COMPONENT_NAME);
        ASSERT_TRUE(thingCdef.IsValid());
        }

    OpenClientDb(Db::OpenMode::ReadWrite);
    AutoCloseClientDb closeClientDbAtEnd(*this);

    Client_ImportComponentDef(TEST_GADGET_COMPONENT_NAME);

    SpatialModelPtr targetModel;
    ASSERT_EQ( DgnDbStatus::Success , createSpatialModel(targetModel, *m_clientDb, DgnModel::CreateModelCode("Instances")) );

    DgnElementCPtr instanceElement;
    m_nsln1.MakeUniqueInstance(instanceElement, *targetModel, 1);
    ASSERT_TRUE(instanceElement.IsValid());

    Client_CheckNestedInstance(*instanceElement, TEST_GADGET_COMPONENT_NAME, 1);
    }

//#ifdef COMMENT_OUT // *** SchemaImportTest, SelectAfterImport will fail with an assertion failure in ECDbMap::TryGetClassMap
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
static ECN::ECClassCP generateECClass(DgnDbR db, ECN::ECSchemaR schema, Utf8CP className, ECN::ECClassCR baseClass)
    {
    schema.AddReferencedSchema(const_cast<ECN::ECSchemaR>(baseClass.GetSchema()));

    ECN::ECEntityClassP ecclass;      
    if (ECN::ECObjectsStatus::Success != schema.CreateEntityClass(ecclass, className))
        return nullptr;

    if (ECN::ECObjectsStatus::Success != ecclass->AddBaseClass(baseClass))
        return nullptr;

    ECN::PrimitiveECPropertyP ecprop;
    if (ECN::ECObjectsStatus::Success != ecclass->CreatePrimitiveProperty(ecprop, "X"))
        return nullptr;

    ecprop->SetType(ECN::PRIMITIVETYPE_Double);

    return ecclass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaImportTest, SelectAfterImport)
    {
    Dgn::ScopedDgnHost host;

    // For the purposes of this test, we'll put the Component and Client models in different DgnDbs
    DgnDbTestUtils::SeedDbInfo rootInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(false, false));

    DgnDbPtr db = DgnDbTestUtils::OpenSeedDbCopy(rootInfo.fileName, L"ComponentModelTest/SelectAfterImport.dgndb");

    if (true)
        {
        ECN::ECSchemaPtr schema;
        ASSERT_EQ( ECN::ECObjectsStatus::Success , ECN::ECSchema::CreateSchema(schema, "ImportTwoInARow", 0, 0) );
        schema->SetNamespacePrefix("tir");

        ECN::ECClassCP baseClass = db->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement);

        ASSERT_TRUE( nullptr != generateECClass(*db, *schema, "C1", *baseClass) );

        ComponentDefCreator::ImportSchema(*db, *schema, false);
        }
    db->ClearECDbCache();
    EC::ECSqlStatement selectC1after;
    selectC1after.Prepare(*db, "SELECT ECInstanceId FROM tir.C1");
    selectC1after.Step();

    db->SaveChanges();
    }
//#endif

#endif //ndef BENTLEYCONFIG_NO_JAVASCRIPT
