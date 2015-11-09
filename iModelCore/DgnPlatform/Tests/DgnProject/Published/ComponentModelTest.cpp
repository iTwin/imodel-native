/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ComponentModelTest.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE

#define TEST_JS_NAMESPACE    "ComponentModelTest"
#define TEST_JS_NAMESPACE_W L"ComponentModelTest"
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

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile (fullInputFileName, fullOutputFileName))
        return BeFileName();

    return fullOutputFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void openDb (DgnDbPtr& db, BeFileNameCR name, DgnDb::OpenMode mode)
    {
    DbResult result = BE_SQLITE_OK;
    db = DgnDb::OpenDgnDb(&result, name, DgnDb::OpenParams(mode));
    ASSERT_TRUE( db.IsValid() ) << (WCharCP)WPrintfString(L"Failed to open %ls in mode %d => result=%x", name.c_str(), (int)mode, (int)result);
    ASSERT_EQ( BE_SQLITE_OK , result );
    db->Txns().EnableTracking(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus createPhysicalModel(PhysicalModelPtr& catalogModel, DgnDbR db, DgnModel::Code const& code)
    {
    DgnClassId mclassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    catalogModel = new PhysicalModel(DgnModel3d::CreateParams(db, mclassId, code));
    return catalogModel->Insert("", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
RefCountedPtr<T> getModelByName(DgnDbR db, Utf8CP cmname)
    {
    return db.Models().Get<T>(db.Models().QueryModelId(DgnModel::CreateModelCode(cmname)));
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void checkElementClassesInModel(DgnModelCR model, bset<DgnClassId> const& allowedClasses)
    {
    Statement statement(model.GetDgnDb(), "SELECT ECClassId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE ModelId=?");
    statement.BindId(1, model.GetModelId());
    while (BE_SQLITE_ROW == statement.Step())
        {
        DgnClassId foundClassId = statement.GetValueId<DgnClassId>(0);
        ASSERT_TRUE( allowedClasses.find(foundClassId) != allowedClasses.end() ) << Utf8PrintfString("Did not expect to find an instance of class %s", model.GetDgnDb().Schemas().GetECClass(ECN::ECClassId(foundClassId.GetValue()))->GetName().c_str()).c_str();
        }
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
void Client_ImportCM(Utf8CP componentName);
void Client_SolveAndCapture(PhysicalElementCPtr&, PhysicalModelR catalogModel, Utf8CP componentName, Json::Value const& parms, bool solutionAlreadyExists);
void Client_InsertNonInstanceElement(Utf8CP modelName, Utf8CP code = nullptr);
void Client_PlaceInstanceOfSolution(DgnElementId&, Utf8CP targetModelName, PhysicalElementCR catalogItem);
void Client_SolveAndPlaceInstance(DgnElementId&, Utf8CP targetModelName, PhysicalModelR catalogModel, Utf8CP componentName, Json::Value const& parms, bool solutionAlreadyExists);
void Client_CheckComponentInstance(DgnElementId, size_t expectedCount, double x, double y, double z);

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
    DgnCategory cat(DgnCategory::CreateParams(*m_componentDb, code, DgnCategory::Scope::Any));
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);
    cat.Insert(appearance);
    return cat.GetCategoryId();
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
    ModelSolverDef::Parameter::Scope ip = ModelSolverDef::Parameter::Scope::Instance;
    ModelSolverDef::Parameter::Scope tp = ModelSolverDef::Parameter::Scope::Type;
    bvector<ModelSolverDef::Parameter> wparameters;
    wparameters.push_back(ModelSolverDef::Parameter("X", tp, ECN::ECValue(1.0))); 
    wparameters.push_back(ModelSolverDef::Parameter("Y", tp, ECN::ECValue(1.0))); 
    wparameters.push_back(ModelSolverDef::Parameter("Z", tp, ECN::ECValue(1.0))); 
    wparameters.push_back(ModelSolverDef::Parameter("Other", ip, ECN::ECValue("Something else")));
    ModelSolverDef wsolver(ModelSolverDef::Type::Script, TEST_JS_NAMESPACE ".Widget", wparameters); // Identify the JS solver that should be used. Note: this JS program must be in the script library
    bvector<ModelSolverDef::Parameter> gparameters; 
    gparameters.push_back(ModelSolverDef::Parameter("Q", tp, ECN::ECValue(1.0))); 
    gparameters.push_back(ModelSolverDef::Parameter("W", tp, ECN::ECValue(1.0))); 
    gparameters.push_back(ModelSolverDef::Parameter("R", tp, ECN::ECValue(1.0))); 
    gparameters.push_back(ModelSolverDef::Parameter("T", ip, ECN::ECValue("Some other parm")));
    ModelSolverDef gsolver(ModelSolverDef::Type::Script, TEST_JS_NAMESPACE ".Gadget", gparameters); // Identify the JS solver that should be used. Note: this JS program must be in the script library

    // Create the models
    ComponentModel::CreateParams wparms(*m_componentDb, TEST_WIDGET_COMPONENT_NAME, "dgn.PhysicalElement", "Widget", "", wsolver);     // *** WIP_COMPONENT_MODEL Authority
    ComponentModelPtr wcm = new ComponentModel(wparms);
    ASSERT_TRUE( wcm->IsValid() );
    ASSERT_EQ( DgnDbStatus::Success , wcm->Insert() );       /* Insert the new model into the DgnDb */

    ComponentModel::CreateParams gparms(*m_componentDb, TEST_GADGET_COMPONENT_NAME, "dgn.PhysicalElement", "Widget", "", gsolver);     // *** WIP_COMPONENT_MODEL Authority
    gparms.SetSolver(gsolver);
    ComponentModelPtr gcm = new ComponentModel(gparms);
    ASSERT_TRUE( gcm->IsValid() );
    ASSERT_EQ( DgnDbStatus::Success , gcm->Insert() );       /* Insert the new model into the DgnDb */

    // Here is the model solver that should be used. 
    // Note that we must put it into the Script library under the same name that was used in the model definition above.
    // It must also register itself as a model solver under the same name as was used in the model definition above
    // Note that a script will generally create elements from scratch. That's why it starts by deleting all elements in the model. They would have been the outputs of the last run.
    AddToFakeScriptLibrary(TEST_JS_NAMESPACE, 
"(function () { \
    function widgetSolver(model, params, options) { \
        model.DeleteAllElements();\
        var element = model.CreateElement('dgn.PhysicalElement', options.Category);\
        var origin = new BentleyApi.Dgn.JsDPoint3d(1,2,3);\
        var angles = new BentleyApi.Dgn.JsYawPitchRollAngles(0,0,0);\
        var builder = new BentleyApi.Dgn.JsElementGeometryBuilder(element, origin, angles); \
        builder.AppendBox(params['X'], params['Y'], params['Z']); \
        builder.SetGeomStreamAndPlacement(element); \
        element.Insert(); \
        var element2 = model.CreateElement('dgn.PhysicalElement', options.Category);\
        var origin2 = new BentleyApi.Dgn.JsDPoint3d(10,12,13);\
        var angles2 = new BentleyApi.Dgn.JsYawPitchRollAngles(0,0,0);\
        var builder2 = new BentleyApi.Dgn.JsElementGeometryBuilder(element2, origin2, angles2); \
        builder2.AppendBox(params['X'], params['Y'], params['Z']); \
        builder2.SetGeomStreamAndPlacement(element2); \
        element2.Insert(); \
        element.SetParent(element2);\
        element.Update();\
        return 0;\
    } \
    function gadgetSolver(model, params, options) { \
        model.DeleteAllElements();\
        var element = model.CreateElement('dgn.PhysicalElement', options.Category);\
        var origin = new BentleyApi.Dgn.JsDPoint3d(0,0,0);\
        var angles = new BentleyApi.Dgn.JsYawPitchRollAngles(0,0,45);\
        var builder = new BentleyApi.Dgn.JsElementGeometryBuilder(element, origin, angles); \
        builder.AppendBox(params['Q'], params['W'], params['R']); \
        builder.SetGeomStreamAndPlacement(element); \
        element.Insert(); \
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

    ModelSolverDef::ParameterSet params = cm->GetSolver().GetParameters();

    for (int i=0; i<10; ++i)
        {
        params.GetParameterP("X")->SetValue(ECN::ECValue(1*i));
        params.GetParameterP("Y")->SetValue(ECN::ECValue(2*i));
        params.GetParameterP("Z")->SetValue(ECN::ECValue(3*i));

        ASSERT_EQ( DgnDbStatus::Success , cm->Solve(params) );
    
        cm->FillModel();
        ASSERT_EQ( 2 , countElementsInModel(*cm) );

        RefCountedCPtr<DgnElement> el = cm->begin()->second;
        checkGeomStream(*el->ToGeometricElement(), ElementGeometry::GeometryType::SolidPrimitive, 1);
        checkSlabDimensions(*el->ToGeometricElement(),  params.GetParameter("X")->GetValue().GetDouble(), 
                                                        params.GetParameter("Y")->GetValue().GetDouble(),
                                                        params.GetParameter("Z")->GetValue().GetDouble());
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

    ModelSolverDef::ParameterSet params = cm->GetSolver().GetParameters();

    for (int i=0; i<10; ++i)
        {
        params.GetParameterP("Q")->SetValue(ECN::ECValue(1*i));
        params.GetParameterP("W")->SetValue(ECN::ECValue(2*i));
        params.GetParameterP("R")->SetValue(ECN::ECValue(3*i));

        ASSERT_EQ( DgnDbStatus::Success , cm->Solve(params) );
    
        cm->FillModel();
        ASSERT_EQ( 1 , countElementsInModel(*cm) );

        RefCountedCPtr<DgnElement> el = cm->begin()->second;
        checkGeomStream(*el->ToGeometricElement(), ElementGeometry::GeometryType::SolidPrimitive, 1);
        checkSlabDimensions(*el->ToGeometricElement(),  params.GetParameter("Q")->GetValue().GetDouble(), 
                                                        params.GetParameter("W")->GetValue().GetDouble(),
                                                        params.GetParameter("R")->GetValue().GetDouble());
        }

    CloseComponentDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_ImportCM(Utf8CP componentName)
    {
    OpenComponentDb(Db::OpenMode::Readonly);
    
    ComponentModelPtr componentModel = getModelByName<ComponentModel>(*m_componentDb, componentName);

    // ONLY DO THIS ONCE per CM. This might be done on demand, the first time that an instance of a particular CM is placed.
    ASSERT_TRUE( componentModel.IsValid() );

    DgnImportContext importer(*m_componentDb, *m_clientDb);

    DgnDbStatus status;
    ComponentModelPtr cmCopy = DgnModel::Import(&status, *componentModel, importer);
    
    ASSERT_TRUE( cmCopy.IsValid() );

    ASSERT_EQ( countElementsInModel(*componentModel), countElementsInModel(*cmCopy) ); // at least make sure the copy has the same number of elements.

    // Original ComponentModel and the copy should contain only PhysicalElements (in this test)
    bset<DgnClassId> cmModelElementClasses;
    cmModelElementClasses.insert(DgnClassId(m_componentDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement)));
    checkElementClassesInModel(*componentModel, cmModelElementClasses);

    bset<DgnClassId> cmCopyModelElementClasses;
    cmCopyModelElementClasses.insert(DgnClassId(m_clientDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement)));
    checkElementClassesInModel(*cmCopy, cmCopyModelElementClasses);

    m_clientDb->SaveChanges();

    //  Verify that we can look up an existing cmCopy
    DgnModelId ccId = m_clientDb->Models().QueryModelId(DgnModel::CreateModelCode(componentName));
    ASSERT_EQ( ccId.GetValue(), cmCopy->GetModelId().GetValue() );

    CloseComponentDb();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_CheckComponentInstance(DgnElementId eid, size_t expectedCount, double x, double y, double z)
    {
    GeometricElementCPtr el = m_clientDb->Elements().Get<GeometricElement>(eid);
    checkGeomStream(*el, ElementGeometry::GeometryType::SolidPrimitive, expectedCount);
    checkSlabDimensions(*el, x, y, z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_SolveAndCapture(PhysicalElementCPtr& catalogItem, PhysicalModelR catalogModel, Utf8CP componentName, Json::Value const& parmsToChange, bool solutionAlreadyExists)
    {
    ComponentModelPtr componentModel = getModelByName<ComponentModel>(*m_clientDb, componentName);  // Open the client's imported copy
    ASSERT_TRUE( componentModel.IsValid() );

    ModelSolverDef::ParameterSet newParameterValues = componentModel->GetSolver().GetParameters();
    for (auto pname : parmsToChange.getMemberNames())
        {
        ModelSolverDef::Parameter* sparam = newParameterValues.GetParameterP(pname.c_str());
        ASSERT_NE( nullptr , sparam );
        ECN::ECValue ecv;
        ECUtils::ConvertJsonToECValue(ecv, parmsToChange[pname], sparam->GetValue().GetPrimitiveType());
        sparam->SetValue(ecv);
        }

    DgnDbStatus status;
    catalogItem = componentModel->GetSolution(&status, catalogModel, newParameterValues);
    ASSERT_TRUE(catalogItem.IsValid()) << Utf8PrintfString("ComponentModel::GetSolution failed with error %x", status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_PlaceInstanceOfSolution(DgnElementId& ieid, Utf8CP targetModelName, PhysicalElementCR catalogItem)
    {
    ASSERT_TRUE(m_clientDb.IsValid() && "Caller must have already opened the Client DB");

    PhysicalModelPtr targetModel = getModelByName<PhysicalModel>(*m_clientDb, targetModelName);
    ASSERT_TRUE( targetModel.IsValid() );

    DgnDbStatus status;
    PhysicalElementCPtr instanceElement = ComponentModel::MakeInstanceOfSolution(&status, *targetModel, catalogItem, DPoint3d::From(1, 2, 3), YawPitchRollAngles::FromDegrees(4, 5, 6), DgnElement::Code());
    ASSERT_TRUE(instanceElement.IsValid()) << Utf8PrintfString("CreateInstanceItem failed with error code %x", status);

    ieid = instanceElement->GetElementId();

    ASSERT_EQ( BE_SQLITE_OK , m_clientDb->SaveChanges() );

    // *** TBD: check that instance matches catalog item

    // Make sure that no component model elements are accidentally copied into the instances model
    bset<DgnClassId> targetModelElementClasses;
    // *** TBD: These are now Item classes targetModelElementClasses.insert(DgnClassId(m_clientDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Element)));
    // *** TBD: These are now Item classes targetModelElementClasses.insert(DgnClassId(m_clientDb->Schemas().GetECClassId(TEST_JS_NAMESPACE, "Widget")));
    // *** TBD: These are now Item classes targetModelElementClasses.insert(DgnClassId(m_clientDb->Schemas().GetECClassId(TEST_JS_NAMESPACE, "Gadget")));
    targetModelElementClasses.insert(DgnClassId(m_clientDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Element)));
    targetModelElementClasses.insert(DgnClassId(m_clientDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement)));
    checkElementClassesInModel(*targetModel, targetModelElementClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_SolveAndPlaceInstance(DgnElementId& ieid, Utf8CP targetModelName, PhysicalModelR catalogModel, Utf8CP componentName, Json::Value const& parms, bool solutionAlreadyExists)
    {
    ASSERT_TRUE(m_clientDb.IsValid() && "Caller must have already opened the Client DB");

    PhysicalElementCPtr solution;
    Client_SolveAndCapture(solution, catalogModel, componentName, parms, solutionAlreadyExists);
    
    if (solution.IsValid())
        Client_PlaceInstanceOfSolution(ieid, targetModelName, *solution);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_InsertNonInstanceElement(Utf8CP modelName, Utf8CP code)
    {
    PhysicalModelPtr targetModel = getModelByName<PhysicalModel>(*m_clientDb, modelName);
    ASSERT_TRUE( targetModel.IsValid() );
    DgnClassId classid = DgnClassId(m_clientDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement));
    DgnCategoryId catid = DgnCategory::QueryHighestCategoryId(*m_clientDb);
    auto el = PhysicalElement::Create(PhysicalElement::CreateParams(*m_clientDb, targetModel->GetModelId(), classid, catid));
    ASSERT_TRUE( el.IsValid() );
    ASSERT_TRUE( el->Insert().IsValid() );
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
    Json::Value wsln1(Json::objectValue);
    wsln1["X"] = 10;
    wsln1["Y"] = 11;
    wsln1["Z"] = 12;

    Json::Value wsln3 = wsln1;
    wsln3["X"] = 100;

    DgnElementId w1, w2, w3;

    OpenClientDb(Db::OpenMode::ReadWrite);
        {
        //  Create the target model in the client. (Do this first, so that the first imported CM's will get a model id other than 1. Hopefully, that will help us catch more bugs.)
        PhysicalModelPtr targetModel;
        ASSERT_EQ( DgnDbStatus::Success , createPhysicalModel(targetModel, *m_clientDb, DgnModel::CreateModelCode("Instances")) );

        //  Add a few unrelated elements to the target model. That way, the first placed CM instance will get an element id other than 1. Hopefully, that will help us catch more bugs.
        for (int i=0; i<10; ++i)
            Client_InsertNonInstanceElement("Instances");

        //  Once per component, import the component model
        Client_ImportCM(TEST_WIDGET_COMPONENT_NAME);

        PhysicalModelPtr catalogModel;
        ASSERT_EQ( DgnDbStatus::Success , createPhysicalModel(catalogModel, *m_clientDb, DgnModel::CreateModelCode("Catalog")) );

        // Now start placing instances of Widgets
        Client_SolveAndPlaceInstance(w1, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, wsln1, false);
        Client_SolveAndPlaceInstance(w2, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, wsln1, true);
        ASSERT_TRUE( w1.IsValid() );
        ASSERT_TRUE( w2.IsValid() );
        ASSERT_NE( w1.GetValue() , w2.GetValue() );
        Client_CheckComponentInstance(w1, 2, wsln1["X"].asDouble(), wsln1["Y"].asDouble(), wsln1["Z"].asDouble());
        Client_CheckComponentInstance(w2, 2, wsln1["X"].asDouble(), wsln1["Y"].asDouble(), wsln1["Z"].asDouble()); // 2nd instance of same solution should have the same instance geometry
    
        //  Add a few unrelated elements to the target model. That way, the first placed CM instance will get an element id other than 1. Hopefully, that will help us catch more bugs.
        for (int i=0; i<5; ++i)
            Client_InsertNonInstanceElement("Instances");

        Client_SolveAndPlaceInstance(w3, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, wsln3, false);
    
        Client_CheckComponentInstance(w3, 2, wsln3["X"].asDouble(), wsln3["Y"].asDouble(), wsln3["Z"].asDouble());
        Client_CheckComponentInstance(w1, 2, wsln1["X"].asDouble(), wsln1["Y"].asDouble(), wsln1["Z"].asDouble());  // new instance of new solution should not affect existing instances of other solutions

        //  new instance of new solution should not affect existing instances of other solutions
        if (true)
            {
            DgnElementId w1_second_time;
            Client_SolveAndPlaceInstance(w1_second_time, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, wsln1, true);
            Client_CheckComponentInstance(w1_second_time, 2, wsln1["X"].asDouble(), wsln1["Y"].asDouble(), wsln1["Z"].asDouble());  // new instance of new solution should not affect existing instances of other solutions
            }

        // Just for a little variety, close the client Db and re-open it
        catalogModel = nullptr;
        CloseClientDb();
        }

    ASSERT_TRUE(w1.IsValid());
    ASSERT_TRUE(w2.IsValid());
    ASSERT_TRUE(w3.IsValid());

    OpenClientDb(Db::OpenMode::ReadWrite);
        {
        PhysicalModelPtr catalogModel = getModelByName<PhysicalModel>(*m_clientDb, "Catalog");

        Json::Value wsln4 = wsln3;
        wsln4["X"] = 2;
        DgnElementId w4;
        Client_SolveAndPlaceInstance(w4, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, wsln4, false);

        Client_CheckComponentInstance(w4, 2, wsln4["X"].asDouble(), wsln4["Y"].asDouble(), wsln4["Z"].asDouble());
        Client_CheckComponentInstance(w1, 2, wsln1["X"].asDouble(), wsln1["Y"].asDouble(), wsln1["Z"].asDouble());  // new instance of new solution should not affect existing instances of other solutions

        // Now start placing instances of Gadgets
        Client_ImportCM(TEST_GADGET_COMPONENT_NAME);
        Json::Value gsln1(Json::objectValue);
        gsln1["Q"] = 3;
        gsln1["W"] = 2;
        gsln1["R"] = 1;
        gsln1["T"] = "text";
        DgnElementId g1, g2;
        Client_SolveAndPlaceInstance(g1, "Instances", *catalogModel, TEST_GADGET_COMPONENT_NAME, gsln1, false);
        BeTest::SetFailOnAssert(false);
        Client_SolveAndPlaceInstance(g2, "Instances", *catalogModel, TEST_GADGET_COMPONENT_NAME, gsln1, true);
        BeTest::SetFailOnAssert(true);

        Client_CheckComponentInstance(g1, 1, gsln1["Q"].asDouble(), gsln1["W"].asDouble(), gsln1["R"].asDouble());
        Client_CheckComponentInstance(g2, 1, gsln1["Q"].asDouble(), gsln1["W"].asDouble(), gsln1["R"].asDouble());

        //  And place another Widget
        Json::Value wsln44 = wsln4;
        wsln44["X"] = 44;
        DgnElementId w44;
        Client_SolveAndPlaceInstance(w44, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, wsln44, false);

        Client_CheckComponentInstance(w3, 2, wsln3["X"].asDouble(), wsln3["Y"].asDouble(), wsln3["Z"].asDouble());
        Client_CheckComponentInstance(w1, 2, wsln1["X"].asDouble(), wsln1["Y"].asDouble(), wsln1["Z"].asDouble());
        Client_CheckComponentInstance(g1, 1, gsln1["Q"].asDouble(), gsln1["W"].asDouble(), gsln1["R"].asDouble());

        CloseClientDb();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelTest, SimulateDeveloperAndClient)
    {
    // For the purposes of this test, we'll put the Component and Client models in different DgnDbs
    m_componentDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_Component.idgndb");
    m_clientDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_Client.idgndb");
    BeTest::GetHost().GetOutputRoot(m_componentSchemaFileName);
    m_componentSchemaFileName.AppendToPath(TEST_JS_NAMESPACE_W L"0.0.ECSchema.xml");

    SimulateDeveloper();
    SimulateClient();
    }

#endif //ndef BENTLEYCONFIG_NO_JAVASCRIPT
