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

#define TEST_JS_NAMESPACE    "DgnComponentModelTest"
#define TEST_JS_NAMESPACE_W L"DgnComponentModelTest"
#define TEST_WIDGET_COMPONENT_NAME "Widget"
#define TEST_GADGET_COMPONENT_NAME "Gadget"

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
struct ComponentModelTest : public testing::Test
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

ComponentModelTest();
void AddToFakeScriptLibrary(Utf8CP jns, Utf8CP jtext);
DgnCategoryId Developer_CreateCategory(Utf8CP code, ColorDef const&);
void Developer_CreateCMs();
void OpenComponentDb(DgnDb::OpenMode mode) {openDb(m_componentDb, m_componentDbName, mode);}
void CloseComponentDb() {m_componentDb->CloseDb(); m_componentDb=nullptr;}
void OpenClientDb(DgnDb::OpenMode mode) {openDb(m_clientDb, m_clientDbName, mode);}
void CloseClientDb() {m_clientDb->CloseDb(); m_clientDb=nullptr;}
void Developer_TestWidgetSolver();
void Developer_TestGadgetSolver();
void Client_CreateTargetModel(Utf8CP targetModelName);
void Client_SolveAndCapture(PhysicalElementCPtr& solutionEl, Utf8CP componentName, Json::Value const& parms, bool solutionAlreadyExists);
void Client_PlaceInstanceOfSolution(Utf8CP targetModelName, PhysicalElementCR);
void Client_SolveAndPlaceInstance(Utf8CP targetModelName, Utf8CP componentName, Json::Value const& parms, bool solutionAlreadyExists);
void GenerateCMSchema();
void Client_CreateProxyCM(Utf8CP componentName);

void SimulateDeveloper();
void SimulateClient();
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentModelTest::ComponentModelTest()
    {
    m_host.SetFetchScriptCallback(&m_scriptLibrary);// In this test, we redirect all requests for JS programs to our fake library
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::AddToFakeScriptLibrary(Utf8CP jns, Utf8CP jtext)
    {
    // In this test, there is only one JS program in the fake library at a time.
    m_scriptLibrary.m_jsProgramName = jns;
    m_scriptLibrary.m_jsProgramText = jtext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId ComponentModelTest::Developer_CreateCategory(Utf8CP code, ColorDef const& color)
    {
    DgnCategories::Category category(code, DgnCategories::Scope::Any);
    DgnCategories::SubCategory::Appearance appearance;
    appearance.SetColor(color);
    m_componentDb->Categories().Insert(category, appearance);
    return category.GetCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* This function defines 2 ComponenModels: a Widget and a Gadget.
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Developer_CreateCMs()
    {
    OpenComponentDb(Db::OpenMode::ReadWrite);

    ASSERT_TRUE(m_componentDb.IsValid());

    // Define the CM's Element Category (in the CM's DgnDb). Use the same name as the component model. 
    ASSERT_TRUE( Developer_CreateCategory("Widget", ColorDef(0xff,0x00,0x00)).IsValid() );
    ASSERT_TRUE( Developer_CreateCategory("Gadget", ColorDef(0x00,0xff,0x00)).IsValid() );

    // Define the Solver wparameters for use by this model.
    Json::Value wparameters(Json::objectValue);
    wparameters["X"] = 1;
    wparameters["Y"] = 1;
    wparameters["Z"] = 1;
    wparameters["Other"] = "Something else";
    DgnModel::Solver wsolver(DgnModel::Solver::Type::Script, TEST_JS_NAMESPACE ".Widget", wparameters); // Identify the JS solver that should be used. Note: this JS program must be in the script library
    Json::Value gparameters(Json::objectValue);
    gparameters["Q"] = 1;
    gparameters["W"] = 1;
    gparameters["R"] = 1;
    gparameters["T"] = "Some other parm";
    DgnModel::Solver gsolver(DgnModel::Solver::Type::Script, TEST_JS_NAMESPACE ".Gadget", gparameters); // Identify the JS solver that should be used. Note: this JS program must be in the script library

    // Create the models
    DgnClassId mclassId = DgnClassId(m_componentDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ComponentModel));
    ComponentModelPtr wcm = new ComponentModel(ComponentModel::CreateParams(*m_componentDb, mclassId, TEST_WIDGET_COMPONENT_NAME, "Widget", wsolver));
    ASSERT_EQ( DgnDbStatus::Success , wcm->Insert() );       /* Insert the new model into the DgnDb */
    ComponentModelPtr gcm = new ComponentModel(ComponentModel::CreateParams(*m_componentDb, mclassId, TEST_GADGET_COMPONENT_NAME, "Gadget", gsolver));
    ASSERT_EQ( DgnDbStatus::Success , gcm->Insert() );       /* Insert the new model into the DgnDb */

    // Here is the model solver that should be used. 
    // Note that we must put it into the Script library under the same name that was used in the model definition above.
    // It must also register itself as a model solver under the same name as was used in the model definition above
    // Note that a script will generally create elements from scratch. That's why it starts by deleting all elements in the model. They would have been the outputs of the last run.
    AddToFakeScriptLibrary(TEST_JS_NAMESPACE, 
"(function () { \
    function widgetSolver(model, params) { \
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
    function gadgetSolver(model, params) { \
        model.DeleteAllElements();\
        var element = model.CreateElement('dgn.PhysicalElement', 'Gadget');\
        var origin = BentleyApi.Dgn.JsDPoint3d.Create(0,0,0);\
        var angles = BentleyApi.Dgn.JsYawPitchRollAngles.Create(0,0,45);\
        var builder = BentleyApi.Dgn.JsElementGeometryBuilder.Create(element, origin, angles); \
        builder.AppendBox(params['Q'], params['W'], params['R']); \
        builder.SetGeomStreamAndPlacement(element); \
        model.InsertElement(element); \
        return 0;\
    } \
    BentleyApi.Dgn.RegisterModelSolver('" TEST_JS_NAMESPACE ".Widget" "', widgetSolver); \
    BentleyApi.Dgn.RegisterModelSolver('" TEST_JS_NAMESPACE ".Gadget" "', gadgetSolver); \
})();\
");
    ASSERT_TRUE( wcm.IsValid() );
    ASSERT_TRUE( gcm.IsValid() );

    m_componentDb->SaveChanges(); // should trigger validation

    CloseComponentDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Developer_TestWidgetSolver()
    {
    OpenComponentDb(Db::OpenMode::ReadWrite);

    ComponentModelPtr cm = getModelByName<ComponentModel>(*m_componentDb, TEST_WIDGET_COMPONENT_NAME);
    ASSERT_TRUE( cm.IsValid() );

    Json::Value parms = cm->GetSolver().GetParameters();  // make a copy

    for (int i=0; i<10; ++i)
        {
        parms["X"] = parms["X"].asDouble() + 1*i;
        parms["Y"] = parms["Y"].asDouble() + 2*i;
        parms["Z"] = parms["Z"].asDouble() + 3*i;

        ASSERT_EQ( DgnDbStatus::Success , cm->Solve(parms) );
    
        cm->FillModel();
        ASSERT_EQ( 2 , countElementsInModel(*cm) );

        RefCountedCPtr<DgnElement> el = cm->begin()->second;
        checkGeomStream(*el->ToGeometricElement(), ElementGeometry::GeometryType::SolidPrimitive, 1);
        checkSlabDimensions(*el->ToGeometricElement(), parms["X"].asDouble(), parms["Y"].asDouble(), parms["Z"].asDouble());
        }

    CloseComponentDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Developer_TestGadgetSolver()
    {
    OpenComponentDb(Db::OpenMode::ReadWrite);

    ComponentModelPtr cm = getModelByName<ComponentModel>(*m_componentDb, TEST_GADGET_COMPONENT_NAME);
    ASSERT_TRUE( cm.IsValid() );

    Json::Value parms = cm->GetSolver().GetParameters();  // make a copy

    for (int i=0; i<10; ++i)
        {
        parms["Q"] = parms["Q"].asDouble() + 1*i;
        parms["W"] = parms["W"].asDouble() + 2*i;
        parms["R"] = parms["R"].asDouble() + 3*i;

        ASSERT_EQ( DgnDbStatus::Success , cm->Solve(parms) );
    
        cm->FillModel();
        ASSERT_EQ( 1 , countElementsInModel(*cm) );

        RefCountedCPtr<DgnElement> el = cm->begin()->second;
        checkGeomStream(*el->ToGeometricElement(), ElementGeometry::GeometryType::SolidPrimitive, 1);
        checkSlabDimensions(*el->ToGeometricElement(), parms["Q"].asDouble(), parms["W"].asDouble(), parms["R"].asDouble());
        }

    CloseComponentDb();
    }

/*---------------------------------------------------------------------------------**//**
// Generate the ECSchema for all related CM's -- ONLY DO THIS ONCE.
// Note: The client or the CM developer could do this. If the CM developer does this, he
// would then have to deliver the ecschema.xml file along with the CM dgndb.
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::GenerateCMSchema()
    {
    OpenComponentDb(Db::OpenMode::ReadWrite);
    ECN::ECSchemaPtr schema;
    ASSERT_EQ( ECN::ECOBJECTS_STATUS_Success , ECN::ECSchema::CreateSchema(schema, TEST_JS_NAMESPACE_W, 0, 0) );
    ASSERT_EQ( DgnDbStatus::Success , ComponentModel::AddAllToECSchema(*schema, *m_componentDb) );
    ASSERT_EQ( ECN::SCHEMA_WRITE_STATUS_Success , schema->WriteToXmlFile(m_componentSchemaFileName) );
    CloseComponentDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_CreateProxyCM(Utf8CP componentName)
    {
    OpenComponentDb(Db::OpenMode::Readonly);
    
    ComponentModelPtr componentModel = getModelByName<ComponentModel>(*m_componentDb, componentName);

    // Create the CMProxyModel -- ONLY DO THIS ONCE per CM. This might be done on demand, the first time that an instance of a particular CM is placed.
    ASSERT_TRUE( componentModel.IsValid() );

    ComponentProxyModelPtr existingProxy = ComponentProxyModel::Get(*m_clientDb, *componentModel);
    
    ASSERT_FALSE( existingProxy.IsValid() ) << "We have not imported the CM yet, so no proxy for it should be found";
    
    ComponentProxyModelPtr proxy = ComponentProxyModel::Create(*m_clientDb, *componentModel, TEST_JS_NAMESPACE);

    ASSERT_TRUE( proxy.IsValid() );
    ASSERT_EQ( DgnDbStatus::Success , proxy->Insert() ) << "We have to be able to create a proxy with a prescribed name -- the name is how we look it up later";

    //  Verify that we can look up an existing proxy
    existingProxy = ComponentProxyModel::Get(*m_clientDb, *componentModel);
    ASSERT_TRUE( existingProxy.IsValid() );
    ASSERT_EQ( proxy.get() , existingProxy.get() );

    CloseComponentDb();
    }

/*---------------------------------------------------------------------------------**//**
*  Create a model in the client DgnDb where we will place instances    
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_CreateTargetModel(Utf8CP targetModelName)
    {
    DgnClassId mclassId = DgnClassId(m_clientDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr targetModel = new PhysicalModel(PhysicalModel::CreateParams(*m_clientDb, mclassId, "Instances"));
    ASSERT_EQ( DgnDbStatus::Success , targetModel->Insert() );       /* Insert the new model into the DgnDb */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_SolveAndCapture(PhysicalElementCPtr& solutionEl, Utf8CP componentName, Json::Value const& parms, bool solutionAlreadyExists)
    {
    //  -------------------------------------------------------
    //  ComponentModel - Solve for the given parameter values
    //  -------------------------------------------------------
    OpenComponentDb(Db::OpenMode::ReadWrite);   // Note that we must open CM read-write, since we'll be updating it.
        
    ComponentModelPtr componentModel = getModelByName<ComponentModel>(*m_componentDb, componentName);
    ASSERT_TRUE( componentModel.IsValid() );

    ASSERT_EQ( DgnDbStatus::Success , componentModel->Solve(parms) );

    Json::Value solvedParms = componentModel->GetSolver().GetParameters();
    for (auto pn : parms.getMemberNames())
        {
        ASSERT_TRUE( parms[pn] == solvedParms[pn] ) << "Solving a CM should result in saving the specified parameter values";
        }

    //  -------------------------------------------------------
    //  ComponentProxyModel - Capture (or look up) the solution geometry
    //  -------------------------------------------------------
    BeAssert(m_clientDb.IsValid() && "Caller must have already opened the Client DB");

    ComponentProxyModelPtr proxy = ComponentProxyModel::Get(*m_clientDb, *componentModel);
    ASSERT_TRUE( proxy.IsValid() ) << "We should have imported the CM and created a proxy in a previous step";

    PhysicalElementCPtr existingSolution = proxy->QuerySolution(ComponentProxyModel::ComputeSolutionName(parms));
    ASSERT_EQ( solutionAlreadyExists , existingSolution.IsValid() );
    
    solutionEl = proxy->CaptureSolution(*componentModel);
    ASSERT_TRUE( solutionEl.IsValid() );

    if (solutionAlreadyExists)
        ASSERT_EQ( existingSolution.get(), solutionEl.get() );

    componentModel = nullptr;
    CloseComponentDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_PlaceInstanceOfSolution(Utf8CP targetModelName, PhysicalElementCR solutionEl)
    {
    BeAssert(m_clientDb.IsValid() && "Caller must have already opened the Client DB");

    PhysicalModelPtr targetModel = getModelByName<PhysicalModel>(*m_clientDb, targetModelName);
    ASSERT_TRUE( targetModel.IsValid() );
    
    PhysicalElementPtr instance = ComponentProxyModel::CreateSolutionInstance(*targetModel, solutionEl, DPoint3d::FromZero(), YawPitchRollAngles());
    ASSERT_TRUE( instance.IsValid() );
    ASSERT_TRUE( instance->Insert().IsValid() );

    ASSERT_EQ( BE_SQLITE_OK , m_clientDb->SaveChanges() );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_SolveAndPlaceInstance(Utf8CP targetModelName, Utf8CP componentName, Json::Value const& parms, bool solutionAlreadyExists)
    {
    BeAssert(m_clientDb.IsValid() && "Caller must have already opened the Client DB");

    PhysicalElementCPtr solutionEl;
    Client_SolveAndCapture(solutionEl, componentName, parms, solutionAlreadyExists);
    
    if (solutionEl.IsValid())
        Client_PlaceInstanceOfSolution(targetModelName, *solutionEl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::SimulateDeveloper()
    {
    //  Simulate a customizer who creates a component definition 
    Developer_CreateCMs();
    Developer_TestWidgetSolver();
    Developer_TestGadgetSolver();
    }

/*---------------------------------------------------------------------------------**//**
* Simulate a client who receives a ComponentModel and then places instances of solutions to it.
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::SimulateClient()
    {
    GenerateCMSchema(); // Note: either the component developer or the client could generate the ECSchema.

    OpenClientDb(Db::OpenMode::ReadWrite);

    ASSERT_EQ( DgnDbStatus::Success , ComponentProxyModel::ImportSchema(*m_clientDb, m_componentSchemaFileName) );
    Client_CreateProxyCM(TEST_WIDGET_COMPONENT_NAME);

    Client_CreateTargetModel("Instances");

    // Now start placing instances of Widgets
    Json::Value wparms(Json::objectValue);
    wparms["X"] = 10;
    wparms["Y"] = 11;
    wparms["Z"] = 12;
    Client_SolveAndPlaceInstance("Instances", TEST_WIDGET_COMPONENT_NAME, wparms, false);
    BeTest::SetFailOnAssert(false);
    Client_SolveAndPlaceInstance("Instances", TEST_WIDGET_COMPONENT_NAME, wparms, true);
    BeTest::SetFailOnAssert(true);
    
    wparms["X"] = 100;
    Client_SolveAndPlaceInstance("Instances", TEST_WIDGET_COMPONENT_NAME, wparms, false);
    
    // Just for a little variety, close the client Db and re-open it
    CloseClientDb();

    OpenClientDb(Db::OpenMode::ReadWrite);
    wparms["X"] = 2;
    Client_SolveAndPlaceInstance("Instances", TEST_WIDGET_COMPONENT_NAME, wparms, false);

    // Now start placing instances of Gadgets
    Client_CreateProxyCM(TEST_GADGET_COMPONENT_NAME);
    Json::Value gparms(Json::objectValue);
    gparms["Q"] = 3;
    gparms["W"] = 2;
    gparms["R"] = 1;
    gparms["T"] = "text";
    Client_SolveAndPlaceInstance("Instances", TEST_GADGET_COMPONENT_NAME, gparms, false);
    BeTest::SetFailOnAssert(false);
    Client_SolveAndPlaceInstance("Instances", TEST_GADGET_COMPONENT_NAME, gparms, true);
    BeTest::SetFailOnAssert(true);

    //  And place another Widget
    wparms["X"] = 44;
    Client_SolveAndPlaceInstance("Instances", TEST_WIDGET_COMPONENT_NAME, wparms, false);

    CloseClientDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelTest, SimulateDeveloperAndClient)
    {
    // For the purposes of this test, we'll put the Component and Client models in different DgnDbs
    m_componentDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"DgnComponentModelTest_Component.idgndb");
    m_clientDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"DgnComponentModelTest_Client.idgndb");
    BeTest::GetHost().GetOutputRoot(m_componentSchemaFileName);
    m_componentSchemaFileName.AppendToPath(TEST_JS_NAMESPACE_W L"0.0.ECSchema.xml");

    SimulateDeveloper();
    SimulateClient();
    }
