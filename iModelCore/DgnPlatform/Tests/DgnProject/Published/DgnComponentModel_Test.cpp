/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnComponentModel_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnCore/DgnScriptContext.h>
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE

#define TEST_JS_NAMESPACE    "Acme"
#define TEST_JS_NAMESPACE_W L"Acme"
#define TEST_JS_SOLVER_NAME "WidgetSolver"
#define TEST_COMPONENT_NAME "Widget"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName copyDb (WCharCP inputFileName, WCharCP outputFileName)
    {
    BeFileName fullInputFileName;
    BeTest::GetHost().GetDocumentsRoot (fullInputFileName);
    fullInputFileName.AppendToPath (inputFileName);

    BeFileName fullOutputFileName;
    BeTest::GetHost().GetOutputRoot(fullOutputFileName);
    fullOutputFileName.AppendToPath(outputFileName);

    BeAssert ( BeFileNameStatus::Success == BeFileName::BeCopyFile (fullInputFileName, fullOutputFileName) );
    return fullOutputFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void openDb (DgnDbPtr& db, BeFileNameCR name, DgnDb::OpenMode mode)
    {
    DbResult result = BE_SQLITE_OK;
    db = DgnDb::OpenDgnDb(&result, name, DgnDb::OpenParams(mode));
    ASSERT_TRUE( db.IsValid() );
    ASSERT_EQ( BE_SQLITE_OK , result );
    db->Txns().EnableTracking(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
RefCountedPtr<T> getModelByName(DgnDbR db, Utf8CP cmname)
    {
    return db.Models().Get<T>(db.Models().QueryModelId(cmname));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t countElementsInModel (DgnModelR model)
    {
    return model.GetElements().size();
    }

/*=================================================================================**//**
* This is a stub implementation of a Script library. 
* In a real implementation, we would look in a disk=based cache or go to Bentley Connect.
* @bsiclass                                                     Sam.Wilson     02/2012
+===============+===============+===============+===============+===============+======*/
struct FakeScriptLibrary : ScopedDgnHost::FetchScriptCallback
    {
    Utf8String m_jsProgramName;
    Utf8String m_jsProgramText;

    Dgn::DgnDbStatus _FetchScript(Utf8StringR sText, DgnScriptType& stypeFound, DgnDbR, Utf8CP sName, DgnScriptType stypePreferred) override
        {
        if (!m_jsProgramName.EqualsI(sName))
            return DgnDbStatus::NotFound;
        stypeFound = DgnScriptType::JavaScript;
        sText = m_jsProgramText;
        return DgnDbStatus::Success;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static void checkGeomStream(GeometricElementCR gel, ElementGeometry::GeometryType exptectedType, size_t expectedCount)
    {
    //  Verify that item generated a line
    size_t count=0;
    for (ElementGeometryPtr geom : ElementGeometryCollection (gel))
        {
        ASSERT_EQ( exptectedType , geom->GetGeometryType() );
        ++count;
        }
    ASSERT_EQ( expectedCount , count );
    }
    
/*---------------------------------------------------------------------------------**//**
* Create a 3D component definition model
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void checkSlabDimensions(GeometricElementCR el, double expectedX, double expectedY, double expectedZ)
    {
    DgnBoxDetail box;
    ASSERT_TRUE( (*(ElementGeometryCollection(el).begin()))->GetAsISolidPrimitive()->TryGetDgnBoxDetail(box) ) << "Geometry should be a slab";
    ASSERT_EQ( expectedX, box.m_baseX );
    ASSERT_EQ( expectedY, box.m_baseY );
    ASSERT_EQ( expectedZ, box.m_topOrigin.Distance(box.m_baseOrigin) );
    }

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson     02/2012
+===============+===============+===============+===============+===============+======*/
struct CMTestFixture : public testing::Test
{
protected:
BeFileName         m_componentDbName;
BeFileName         m_componentSchemaFileName;
BeFileName         m_clientDbName;
DgnDbPtr           m_componentDb;
DgnDbPtr           m_clientDb;
Dgn::ScopedDgnHost m_host;
FakeScriptLibrary  m_scriptLibrary;
bmap<DgnSubCategoryId, DgnSubCategoryId> m_subcatxlat;

CMTestFixture();
void AddToScriptLibrary(Utf8CP jns, Utf8CP jtext);
DgnCategoryId CreateCategory(Utf8CP code, ColorDef const&);
void CreateWidgetComponentModel();
void OpenComponentDb(DgnDb::OpenMode mode) {openDb(m_componentDb, m_componentDbName, mode);}
void CloseComponentDb() {m_componentDb->CloseDb(); m_componentDb=nullptr;}
void OpenClientDb(DgnDb::OpenMode mode) {openDb(m_clientDb, m_clientDbName, mode);}
void CloseClientDb() {m_clientDb->CloseDb(); m_clientDb=nullptr;}
void TestWidgetSolverInPlace();
void PlaceInstanceOfWidgetComponent(double x, double y, double z, bool solutionAlreadyExists);
void GenerateTestECSChema();
void ImportCMSchema();
void ImportCM();

void SimulateWidgetDeveloper();
void SimulateWidgetClient();
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
CMTestFixture::CMTestFixture()
    {
    m_host.SetFetchScriptCallback(&m_scriptLibrary);// In this test, we redirect all requests for JS programs to our fake library
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void CMTestFixture::AddToScriptLibrary(Utf8CP jns, Utf8CP jtext)
    {
    // In this test, there is only one JS program in the fake library at a time.
    m_scriptLibrary.m_jsProgramName = jns;
    m_scriptLibrary.m_jsProgramText = jtext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId CMTestFixture::CreateCategory(Utf8CP code, ColorDef const& color)
    {
    DgnCategories::Category category(code, DgnCategories::Scope::Any);
    DgnCategories::SubCategory::Appearance appearance;
    appearance.SetColor(color);
    m_componentDb->Categories().Insert(category, appearance);
    return category.GetCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CMTestFixture::CreateWidgetComponentModel()
    {
    OpenComponentDb(Db::OpenMode::ReadWrite);

    ASSERT_TRUE(m_componentDb.IsValid());

    // Define the CM's Element Category (in the CM's DgnDb). Use the same name as the component model. 
    ASSERT_TRUE( CreateCategory(TEST_COMPONENT_NAME, ColorDef(0xff,0x00,0x00)).IsValid() );

    // Define the Solver parameters for use by this model.
    Json::Value parameters(Json::objectValue);
    parameters["X"] = 1;
    parameters["Y"] = 1;
    parameters["Z"] = 1;
    parameters["Other"] = "Something else";
    DgnModel::Solver solver(DgnModel::Solver::Type::Script, TEST_JS_NAMESPACE "." TEST_JS_SOLVER_NAME, parameters); // Identify the JS solver that should be used. Note: this JS program must be in the script library

    // Create the model
    DgnClassId mclassId = DgnClassId(m_componentDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ComponentModel));
    RefCountedPtr<ComponentModel> cm = new ComponentModel(ComponentModel::CreateParams(*m_componentDb, mclassId, TEST_COMPONENT_NAME, TEST_COMPONENT_NAME, solver));
    ASSERT_EQ( DgnDbStatus::Success , cm->Insert() );       /* Insert the new model into the DgnDb */

    // Here is the model solver that should be used. 
    // Note that we must put it into the Script library under the same name that was used in the model definition above.
    // It must also register itself as a model solver under the same name as was used in the model definition above
    // Note that a script will generally create elements from scratch. That's why it starts by deleting all elements in the model. They would have been the outputs of the last run.
    AddToScriptLibrary(TEST_JS_NAMESPACE, 
"(function () { \
    function testSolver(model, params) { \
        model.DeleteAllElements();\
        var element = model.CreateElement('dgn.PhysicalElement', 'Widget');\
        var origin = BentleyApi.Dgn.JsDPoint3d.Create(1,2,3);\
        var angles = BentleyApi.Dgn.JsYawPitchRollAngles.Create(0,0,0);\
        var builder = BentleyApi.Dgn.JsElementGeometryBuilder.Create(element, origin, angles); \
        builder.AppendBox(params['X'], params['Y'], params['Z']); \
        builder.SetGeomStreamAndPlacement(element); \
        model.InsertElement(element); \
        var element2 = model.CreateElement('dgn.PhysicalElement', 'Widget');\
        var origin2 = BentleyApi.Dgn.JsDPoint3d.Create(10,12,13);\
        var angles2 = BentleyApi.Dgn.JsYawPitchRollAngles.Create(0,0,0);\
        var builder2 = BentleyApi.Dgn.JsElementGeometryBuilder.Create(element2, origin2, angles2); \
        builder2.AppendBox(params['X'], params['Y'], params['Z']); \
        builder2.SetGeomStreamAndPlacement(element2); \
        model.InsertElement(element2); \
        return 0;\
    } \
    BentleyApi.Dgn.RegisterModelSolver('" TEST_JS_NAMESPACE "." TEST_JS_SOLVER_NAME "', testSolver); \
})();\
");
    ASSERT_TRUE( cm.IsValid() );

    m_componentDb->SaveChanges(); // should trigger validation

    CloseComponentDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CMTestFixture::TestWidgetSolverInPlace()
    {
    OpenComponentDb(Db::OpenMode::ReadWrite);

    RefCountedPtr<ComponentModel> cm = getModelByName<ComponentModel>(*m_componentDb, TEST_COMPONENT_NAME);
    ASSERT_TRUE( cm.IsValid() );

    Json::Value parms = cm->GetSolverParametersR();  // make a copy

    for (int i=0; i<10; ++i)
        {
        parms["X"] = parms["X"].asDouble() + 1*i;
        parms["Y"] = parms["Y"].asDouble() + 2*i;
        parms["Z"] = parms["Z"].asDouble() + 3*i;

        cm->GetSolverParametersR() = parms; // set the parameters
        cm->Update();                       // Don't forget to call Update and then ...
        m_componentDb->SaveChanges();       // ... SaveChanges. That's how the txn manager finds out about the change to the model's parameters and then triggers its validation callback.
    
        cm->FillModel();
        ASSERT_EQ( 2 , countElementsInModel(*cm) );

        RefCountedCPtr<DgnElement> el = cm->begin()->second;
        checkGeomStream(*el->ToGeometricElement(), ElementGeometry::GeometryType::SolidPrimitive, 1);
        checkSlabDimensions(*el->ToGeometricElement(), parms["X"].asDouble(), parms["Y"].asDouble(), parms["Z"].asDouble());
        }

    CloseComponentDb();
    }

/*---------------------------------------------------------------------------------**//**
// Generate the ECSchema for all related CM's -- ONLY DO THIS ONCE.
// Note: The client or the CM developer could do this. If the CM developer does this, he
// would then have to deliver the ecschema.xml file along with the CM dgndb.
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CMTestFixture::GenerateTestECSChema()
    {
    ECN::ECSchemaPtr schema;
    ASSERT_EQ( ECN::ECOBJECTS_STATUS_Success , ECN::ECSchema::CreateSchema(schema, TEST_JS_NAMESPACE_W, 0, 0) );

    RefCountedPtr<ComponentModel> componentModel = getModelByName<ComponentModel>(*m_componentDb, TEST_COMPONENT_NAME);
    ASSERT_EQ( DgnDbStatus::Success , componentModel->GenerateECClass(*schema) );

    ASSERT_EQ( ECN::SCHEMA_WRITE_STATUS_Success , schema->WriteToXmlFile(m_componentSchemaFileName) );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CMTestFixture::ImportCMSchema()
    {
    OpenClientDb(Db::OpenMode::ReadWrite);
    OpenComponentDb(Db::OpenMode::ReadWrite);

    // Generate the ECSchema for all related CM's -- ONLY DO THIS ONCE. This might be done when the target Db is created. 
    GenerateTestECSChema();

    // Import the generated ECSchema -- ONLY DO THIS ONCE per generated schema. This might be done when the target Db is created.
    ASSERT_EQ( DgnDbStatus::Success , ComponentProxyModel::ImportSchema(*m_clientDb, m_componentSchemaFileName) );
    
    CloseComponentDb();
    CloseClientDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CMTestFixture::ImportCM()
    {
    OpenComponentDb(Db::OpenMode::Readonly);
    OpenClientDb(Db::OpenMode::ReadWrite);
    
    RefCountedPtr<ComponentModel> componentModel = getModelByName<ComponentModel>(*m_componentDb, TEST_COMPONENT_NAME);

    // Create the CMProxyModel -- ONLY DO THIS ONCE per CM. This might be done on demand, the first time that an instance of a particular CM is placed.
    ASSERT_TRUE( componentModel.IsValid() );

    RefCountedPtr<ComponentProxyModel> existingProxy;
    ASSERT_EQ( DgnDbStatus::NotFound , ComponentProxyModel::Query(existingProxy, *m_clientDb, *componentModel) ) << "We have not imported the CM yet, so no proxy for it should be found";
    
    RefCountedPtr<ComponentProxyModel> proxy = ComponentProxyModel::Create(*m_clientDb, *componentModel, *m_clientDb->Schemas().GetECSchema(TEST_JS_NAMESPACE));

    componentModel = nullptr; // Once we have created the proxy, we don't need the componentModel model itself.
    CloseComponentDb();

    ASSERT_TRUE( proxy.IsValid() );
    ASSERT_EQ( DgnDbStatus::Success , proxy->Insert() ) << "We have to be able to create a proxy with a prescribed name -- the name is how we look it up later";

    CloseClientDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CMTestFixture::PlaceInstanceOfWidgetComponent(double x, double y, double z, bool solutionAlreadyExists)
    {
    //  ----- We start with the component's Db -----

    OpenComponentDb(Db::OpenMode::ReadWrite);   // Note that we must open CM read-write, since we'll be updating it.
    RefCountedPtr<ComponentModel> componentModel = getModelByName<ComponentModel>(*m_componentDb, TEST_COMPONENT_NAME);
    ASSERT_TRUE( componentModel.IsValid() );

    //  1. Solve the CM for the given parameter values
    Json::Value& parms = componentModel->GetSolverParametersR();  // set the parameters
    parms["X"] = x;
    parms["Y"] = y;
    parms["Z"] = z;
    componentModel->Update();                        // Don't forget to call Update and then ...
    m_componentDb->SaveChanges();               // ... SaveChanges. That's how the txn manager finds out about the change to the model's parameters and then triggers its validation callback.
    

    //  ----- Now the client/target Db comes into play -----

    OpenClientDb(Db::OpenMode::ReadWrite);

    // The CM Solution Element is stored in the proxy model (which should have been created in the client DB and CM import time)
    RefCountedPtr<ComponentProxyModel> proxy;
    ASSERT_EQ( DgnDbStatus::Success , ComponentProxyModel::Query(proxy, *m_clientDb, *componentModel) ) << "We should have imported the CM and created a proxy in a prior step";
    ASSERT_TRUE( proxy.IsValid() );

    PhysicalElementCPtr solutionEl;

    if (solutionAlreadyExists)
        solutionEl = proxy->QuerySolution(*componentModel);
    else
        solutionEl = proxy->CaptureSolution(*componentModel);

    ASSERT_TRUE( solutionEl.IsValid() );

    //  3. Place an instance of the solution in the target model
    
    RefCountedPtr<PhysicalModel> client = getModelByName<PhysicalModel>(*m_clientDb, TEST_COMPONENT_NAME);
    ASSERT_TRUE( client.IsValid() );
    
    //FAIL() << "*** TBD: create instance as copy of CM Solution Element";

    CloseComponentDb();
    CloseClientDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CMTestFixture::SimulateWidgetDeveloper()
    {
    //  Simulate a customizer creating a component definition 
    CreateWidgetComponentModel();
    
    TestWidgetSolverInPlace();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CMTestFixture::SimulateWidgetClient()
    {
    //  Simulate a client placing an instance of the component.
    ImportCMSchema();
    ImportCM();
    PlaceInstanceOfWidgetComponent(10,11,12,false);
    PlaceInstanceOfWidgetComponent(10,11,12,true);
    PlaceInstanceOfWidgetComponent(100,11,12,false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CMTestFixture, Test1)
    {
    // For the purposes of this test, we'll put the Component and Client models in different DgnDbs
    m_componentDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"DgnComponentModelTest_Component.idgndb");
    m_clientDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"DgnComponentModelTest_Client.idgndb");
    BeTest::GetHost().GetOutputRoot(m_componentSchemaFileName);
    m_componentSchemaFileName.AppendToPath(TEST_JS_NAMESPACE_W L"0.0.ECSchema.xml");

    SimulateWidgetDeveloper();
    SimulateWidgetClient();
    }
