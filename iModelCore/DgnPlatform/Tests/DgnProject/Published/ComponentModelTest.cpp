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

// *** WARNING: Keep this consistent with ComponentModelTest.ts
#define TEST_JS_NAMESPACE    "ComponentModelTest"
#define TEST_JS_NAMESPACE_W L"ComponentModelTest"
#define TEST_WIDGET_COMPONENT_NAME "Widget"
#define TEST_GADGET_COMPONENT_NAME "Gadget"
#define TEST_THING_COMPONENT_NAME "Thing"

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
    TestDataManager::MustBeBriefcase(db, mode);
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
* @bsimethod                                    Sam.Wilson                      04/2013
+===============+===============+===============+===============+===============+======*/
struct DetectJsErrors : DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler
    {
    void _HandleScriptError(BeJsContextR, Category category, Utf8CP description, Utf8CP details) override
        {
        FAIL() << (Utf8CP)Utf8PrintfString("JS error %x: %s , %s", (int)category, description, details);
        }

    void _HandleLogMessage(Utf8CP category, DgnPlatformLib::Host::ScriptAdmin::LoggingSeverity sev, Utf8CP msg) override
        {
        ScriptNotificationHandler::_HandleLogMessage(category, sev, msg);  // logs it
        printf ("%s\n", msg);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static void checkGeomStream(GeometrySourceCR gel, ElementGeometry::GeometryType exptectedType, size_t expectedCount)
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
static void checkSlabDimensions(GeometrySourceCR el, double expectedX, double expectedY, double expectedZ)
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
struct ComponentModelTest : public testing::Test, ScopedDgnHost::FetchScriptCallback
{
BeFileName         m_componentDbName;
BeFileName         m_componentSchemaFileName;
BeFileName         m_clientDbName;
DgnDbPtr           m_componentDb;
DgnDbPtr           m_clientDb;
Dgn::ScopedDgnHost m_host;
Json::Value m_wsln1, m_wsln3, m_wsln4, m_wsln44, m_gsln1, m_nsln1;

Dgn::DgnDbStatus _FetchScript(Utf8StringR sText, DgnScriptType& stypeFound, DateTime& lmt, DgnDbR, Utf8CP sName, DgnScriptType stypePreferred) override;

ComponentModelTest();
DgnCategoryId Developer_CreateCategory(Utf8CP code, ColorDef const&);
void Developer_CreateCMs();
void OpenComponentDb(DgnDb::OpenMode mode) {openDb(m_componentDb, m_componentDbName, mode);}
void CloseComponentDb() {if (m_componentDb.IsValid()) m_componentDb->CloseDb(); m_componentDb=nullptr;}
void OpenClientDb(DgnDb::OpenMode mode) {openDb(m_clientDb, m_clientDbName, mode);}
void CloseClientDb() {if (m_clientDb.IsValid()) m_clientDb->CloseDb(); m_clientDb=nullptr;}
void Developer_TestWidgetSolver();
void Developer_TestGadgetSolver();
void Developer_TestThingSolver();
void Developer_CreateCapturedSolution(DgnElementCPtr&, PhysicalModelR catalogModel, Utf8CP componentName, Json::Value const& parms, Utf8CP catalogItemName);
void Client_ImportCM(Utf8CP componentName);
void Client_InsertNonInstanceElement(Utf8CP modelName, Utf8CP code = nullptr);
void Client_PlaceInstanceOfSolution(DgnElementId&, Utf8CP targetModelName, DgnElementCR catalogItem);
void Client_PlaceInstance(DgnElementId&, Utf8CP targetModelName, PhysicalModelR catalogModel, Utf8CP componentName, Utf8CP ciname, bool expectToFindSolution);
void Client_CheckComponentInstance(DgnElementId, size_t expectedSolidCount, double x, double y, double z);
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
    // *** WARNING: Keep these parameters names consistent with ComponentModelTest.ts
    m_wsln1 = Json::objectValue;
    m_wsln1["X"] = 10;
    m_wsln1["Y"] = 11;
    m_wsln1["Z"] = 12;

    m_wsln3 = m_wsln1;
    m_wsln3["X"] = 100;

    m_wsln4 = m_wsln3;
    m_wsln4["X"] = 2;

    m_wsln44 = m_wsln4;
    m_wsln44["X"] = 44;

    m_gsln1 = Json::objectValue;
    m_gsln1["Q"] = 3;
    m_gsln1["W"] = 2;
    m_gsln1["R"] = 1;
    m_gsln1["T"] = "text";

    m_nsln1 = Json::objectValue;
    m_nsln1["A"] = 1;
    m_nsln1["B"] = 1;
    m_nsln1["C"] = 1;

    T_HOST.GetScriptAdmin().RegisterScriptNotificationHandler(*new DetectJsErrors);
    m_host.SetFetchScriptCallback(this);
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
* This function defines 2 ComponenModels: a Widget and a Gadget.
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Developer_CreateCMs()
    {
    OpenComponentDb(Db::OpenMode::ReadWrite);
    AutoCloseComponentDb closeComponentDb(*this);

    ASSERT_TRUE(m_componentDb.IsValid());

    // Define the CM's Element Category (in the CM's DgnDb). Use the same name as the component model. 
    ASSERT_TRUE( Developer_CreateCategory("WidgetCategory", ColorDef(0xff,0x00,0x00)).IsValid() );
    ASSERT_TRUE( Developer_CreateCategory("GadgetCategory", ColorDef(0x00,0xff,0x00)).IsValid() );
    ASSERT_TRUE( Developer_CreateCategory("ThingCategory", ColorDef(0x00,0x00,0xff)).IsValid() );

    ModelSolverDef::Parameter::Scope ip = ModelSolverDef::Parameter::Scope::Instance;
    ModelSolverDef::Parameter::Scope tp = ModelSolverDef::Parameter::Scope::Type;

    // Widget
        {
        bvector<ModelSolverDef::Parameter> wparameters;
        wparameters.push_back(ModelSolverDef::Parameter("X", tp, ECN::ECValue(1.0))); 
        wparameters.push_back(ModelSolverDef::Parameter("Y", tp, ECN::ECValue(1.0))); 
        wparameters.push_back(ModelSolverDef::Parameter("Z", tp, ECN::ECValue(1.0))); 
        wparameters.push_back(ModelSolverDef::Parameter("Other", ip, ECN::ECValue("Something else")));
        ModelSolverDef wsolver(ModelSolverDef::Type::Script, TEST_JS_NAMESPACE "." TEST_WIDGET_COMPONENT_NAME, wparameters);
                                                           // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ This must match the name used in the .ts file in the call to RegisterModelSolver
        ComponentModel::CreateParams wparms(*m_componentDb, TEST_WIDGET_COMPONENT_NAME, DGN_SCHEMA(DGN_CLASSNAME_PhysicalElement), "WidgetCategory", "", wsolver);
        ComponentModelPtr wcm = new ComponentModel(wparms);
        ASSERT_TRUE( wcm->IsValid() );
        ASSERT_EQ( DgnDbStatus::Success , wcm->Insert() );
        }

    //  Gadget
        {
        bvector<ModelSolverDef::Parameter> gparameters; 
        gparameters.push_back(ModelSolverDef::Parameter("Q", tp, ECN::ECValue(1.0))); 
        gparameters.push_back(ModelSolverDef::Parameter("W", tp, ECN::ECValue(1.0))); 
        gparameters.push_back(ModelSolverDef::Parameter("R", tp, ECN::ECValue(1.0))); 
        gparameters.push_back(ModelSolverDef::Parameter("T", ip, ECN::ECValue("Some other parm")));
        ModelSolverDef gsolver(ModelSolverDef::Type::Script, TEST_JS_NAMESPACE "." TEST_GADGET_COMPONENT_NAME, gparameters);

        ComponentModel::CreateParams gparms(*m_componentDb, TEST_GADGET_COMPONENT_NAME, DGN_SCHEMA(DGN_CLASSNAME_PhysicalElement), "GadgetCategory", "", gsolver);
        ComponentModelPtr gcm = new ComponentModel(gparms);
        ASSERT_TRUE( gcm->IsValid() );
        ASSERT_EQ( DgnDbStatus::Success , gcm->Insert() );
        }

    //  Thing
        {
        bvector<ModelSolverDef::Parameter> nparameters; 
        nparameters.push_back(ModelSolverDef::Parameter("A", tp, ECN::ECValue(1.0))); 
        nparameters.push_back(ModelSolverDef::Parameter("B", tp, ECN::ECValue(1.0))); 
        nparameters.push_back(ModelSolverDef::Parameter("C", tp, ECN::ECValue(1.0))); 
        ModelSolverDef nsolver(ModelSolverDef::Type::Script, TEST_JS_NAMESPACE "." TEST_THING_COMPONENT_NAME, nparameters);

        ComponentModel::CreateParams nparms(*m_componentDb, TEST_THING_COMPONENT_NAME, DGN_SCHEMA(DGN_CLASSNAME_PhysicalElement), "ThingCategory", "", nsolver);
        ComponentModelPtr ncm = new ComponentModel(nparms);
        ASSERT_TRUE( ncm->IsValid() );
        ASSERT_EQ( DgnDbStatus::Success , ncm->Insert() );
        }

    m_componentDb->SaveChanges(); // should trigger validation
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Developer_TestWidgetSolver()
    {
    OpenComponentDb(Db::OpenMode::ReadWrite);
    AutoCloseComponentDb closeComponentDb(*this);

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
        checkGeomStream(*el->ToGeometrySource(), ElementGeometry::GeometryType::SolidPrimitive, 1);
        checkSlabDimensions(*el->ToGeometrySource(),  params.GetParameter("X")->GetValue().GetDouble(), 
                                                        params.GetParameter("Y")->GetValue().GetDouble(),
                                                        params.GetParameter("Z")->GetValue().GetDouble());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Developer_TestGadgetSolver()
    {
    OpenComponentDb(Db::OpenMode::ReadWrite);
    AutoCloseComponentDb closeComponentDb(*this);

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
        checkGeomStream(*el->ToGeometrySource(), ElementGeometry::GeometryType::SolidPrimitive, 1);
        checkSlabDimensions(*el->ToGeometrySource(),  params.GetParameter("Q")->GetValue().GetDouble(), 
                                                        params.GetParameter("W")->GetValue().GetDouble(),
                                                        params.GetParameter("R")->GetValue().GetDouble());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Developer_TestThingSolver()
    {
    OpenComponentDb(Db::OpenMode::ReadWrite);
    AutoCloseComponentDb closeComponentDb(*this);

    ComponentModelPtr cm = getModelByName<ComponentModel>(*m_componentDb, TEST_THING_COMPONENT_NAME);
    ASSERT_TRUE( cm.IsValid() );

    ModelSolverDef::ParameterSet params = cm->GetSolver().GetParameters();

    for (int i=0; i<10; ++i)
        {
        params.GetParameterP("A")->SetValue(ECN::ECValue(1*i));
        params.GetParameterP("B")->SetValue(ECN::ECValue(2*i));
        params.GetParameterP("C")->SetValue(ECN::ECValue(3*i));

        ASSERT_EQ( DgnDbStatus::Success , cm->Solve(params) );
    
        cm->FillModel();
        ASSERT_EQ( 2 , countElementsInModel(*cm) );

        RefCountedCPtr<DgnElement> el = cm->begin()->second;
        checkGeomStream(*el->ToGeometrySource(), ElementGeometry::GeometryType::SolidPrimitive, 1);
        checkSlabDimensions(*el->ToGeometrySource(),  params.GetParameter("A")->GetValue().GetDouble(), 
                                                        params.GetParameter("B")->GetValue().GetDouble(),
                                                        params.GetParameter("C")->GetValue().GetDouble());
        }
    }

/*---------------------------------------------------------------------------------**//**
* ONLY DO THIS ONCE per CM. This might be done on demand, the first time that an instance of a particular CM is placed.
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_ImportCM(Utf8CP componentName)
    {
    OpenComponentDb(Db::OpenMode::Readonly);
    AutoCloseComponentDb closeComponentDb(*this);
    
    ComponentModelPtr cmOriginal = getModelByName<ComponentModel>(*m_componentDb, componentName);

    ASSERT_TRUE( cmOriginal.IsValid() );

    ComponentModel::Importer importer(*m_clientDb, *cmOriginal);
    
    //  Import the component model itself
    ComponentModelPtr cmCopy = importer.ImportComponentModel();

    ASSERT_TRUE( cmCopy.IsValid() );

    ASSERT_EQ( countElementsInModel(*cmOriginal), countElementsInModel(*cmCopy) ); // at least make sure the copy has the same number of elements.

    bset<DgnClassId> cmModelElementClasses;
    cmModelElementClasses.insert(DgnClassId(m_componentDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement)));
    checkElementClassesInModel(*cmOriginal, cmModelElementClasses);

    bset<DgnClassId> cmCopyModelElementClasses;
    cmCopyModelElementClasses.insert(DgnClassId(m_clientDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement)));
    checkElementClassesInModel(*cmCopy, cmCopyModelElementClasses);

    // Import the item catalog
    PhysicalModelPtr catalogModel = getModelByName<PhysicalModel>(*m_clientDb, "Catalog");
    if (!catalogModel.IsValid())
        createPhysicalModel(catalogModel, *m_clientDb, DgnModel::CreateModelCode("Catalog"));

    ASSERT_TRUE(catalogModel.IsValid());

    importer.ImportSolutions(*catalogModel);

    bvector<DgnElementId> originalSolutions, importedSolutions;
    cmOriginal->QuerySolutions(originalSolutions);
    cmCopy->QuerySolutions(importedSolutions);
    ASSERT_EQ( originalSolutions.size() , importedSolutions.size() );

    m_clientDb->SaveChanges();

    //  Verify that we can look up an existing component
    DgnModelId ccId = m_clientDb->Models().QueryModelId(DgnModel::CreateModelCode(componentName));
    ASSERT_EQ( ccId.GetValue(), cmCopy->GetModelId().GetValue() );
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_CheckComponentInstance(DgnElementId eid, size_t expectedSolidCount, double x, double y, double z)
    {
    DgnElementCPtr el = m_clientDb->Elements().Get<DgnElement>(eid);
    checkGeomStream(*el->ToGeometrySource(), ElementGeometry::GeometryType::SolidPrimitive, expectedSolidCount);
    checkSlabDimensions(*el->ToGeometrySource(), x, y, z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Developer_CreateCapturedSolution(DgnElementCPtr& catalogItem, PhysicalModelR catalogModel, Utf8CP componentName, Json::Value const& parmsToChange, Utf8CP catalogItemName)
    {
    ComponentModelPtr componentModel = getModelByName<ComponentModel>(*m_componentDb, componentName);  // Open the client's imported copy
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
    catalogItem = componentModel->CaptureSolution(&status, catalogModel, newParameterValues, catalogItemName);
    ASSERT_TRUE(catalogItem.IsValid()) << Utf8PrintfString("ComponentModel::CaptureSolution failed with error %x", status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_PlaceInstanceOfSolution(DgnElementId& ieid, Utf8CP targetModelName, DgnElementCR catalogItem)
    {
    ASSERT_TRUE(m_clientDb.IsValid() && "Caller must have already opened the Client DB");

    PhysicalModelPtr targetModel = getModelByName<PhysicalModel>(*m_clientDb, targetModelName);
    ASSERT_TRUE( targetModel.IsValid() );

    DgnDbStatus status;
    DgnElementCPtr instanceElement = ComponentModel::MakeInstance(&status, *targetModel, catalogItem);
    ASSERT_TRUE(instanceElement.IsValid()) << Utf8PrintfString("CreateInstanceItem failed with error code %x", status);

    ieid = instanceElement->GetElementId();

    ASSERT_EQ( BE_SQLITE_OK , m_clientDb->SaveChanges() );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::Client_PlaceInstance(DgnElementId& ieid, Utf8CP targetModelName, PhysicalModelR catalogModel, Utf8CP componentName, Utf8CP ciname, bool expectToFindSolution)
    {
    ASSERT_TRUE(m_clientDb.IsValid() && "Caller must have already opened the Client DB");

    ComponentModelPtr componentModel = getModelByName<ComponentModel>(*m_clientDb, componentName);  // Open the client's imported copy
    ASSERT_TRUE( componentModel.IsValid() );
    DgnElementCPtr catalogItem;
    ModelSolverDef::ParameterSet cmparams;
    componentModel->QuerySolutionByName(catalogItem, cmparams, ciname);
    if (!expectToFindSolution)
        {
        ASSERT_FALSE(catalogItem.IsValid());
        return;
        }
    ASSERT_TRUE(catalogItem.IsValid());
    Client_PlaceInstanceOfSolution(ieid, targetModelName, *catalogItem);
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
    Developer_TestThingSolver();

    // Create catalogs of components
    OpenComponentDb(Db::OpenMode::ReadWrite);
    AutoCloseComponentDb closeComponentDb(*this);

    PhysicalModelPtr catalogModel;
    ASSERT_EQ( DgnDbStatus::Success , createPhysicalModel(catalogModel, *m_componentDb, DgnModel::CreateModelCode("Catalog")) );

    DgnElementCPtr ci;
    Developer_CreateCapturedSolution(ci, *catalogModel, TEST_WIDGET_COMPONENT_NAME, m_wsln1, "wsln1");
    Developer_CreateCapturedSolution(ci, *catalogModel, TEST_WIDGET_COMPONENT_NAME, m_wsln3, "wsln3");
    Developer_CreateCapturedSolution(ci, *catalogModel, TEST_WIDGET_COMPONENT_NAME, m_wsln4, "wsln4");
    Developer_CreateCapturedSolution(ci, *catalogModel, TEST_WIDGET_COMPONENT_NAME, m_wsln44, "wsln44");

    Developer_CreateCapturedSolution(ci, *catalogModel, TEST_GADGET_COMPONENT_NAME, m_gsln1, "gsln1");

    Developer_CreateCapturedSolution(ci, *catalogModel, TEST_THING_COMPONENT_NAME, m_nsln1, "nsln1");
    
    ci = nullptr;
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

    DgnElementCPtr nestedSln = ComponentModel::QuerySolutionFromInstance(*nestedInstance); 
    ASSERT_TRUE(nestedSln.IsValid());
    
    DgnModelId nestedSlnComponentModelId;
    ModelSolverDef::ParameterSet nestedslnparams;
    ASSERT_EQ(DgnDbStatus::Success, ComponentModel::QuerySolutionInfo(nestedSlnComponentModelId, nestedslnparams, *nestedSln));
    
    ASSERT_EQ(ComponentModel::FindModelByName(*m_clientDb, expectedChildComponentName)->GetModelId(), nestedSlnComponentModelId);
    }

/*---------------------------------------------------------------------------------**//**
* Simulate a client who receives a ComponentModel and then places instances of solutions to it.
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelTest::SimulateClient()
    {
    DgnElementId w1, w2, w3;

    OpenClientDb(Db::OpenMode::ReadWrite);
        {
        AutoCloseClientDb closeClientDbAtEnd(*this);
        //  Create the target model in the client. (Do this first, so that the first imported CM's will get a model id other than 1. Hopefully, that will help us catch more bugs.)
        PhysicalModelPtr targetModel;
        ASSERT_EQ( DgnDbStatus::Success , createPhysicalModel(targetModel, *m_clientDb, DgnModel::CreateModelCode("Instances")) );

        //  Add a few unrelated elements to the target model. That way, the first placed CM instance will get an element id other than 1. Hopefully, that will help us catch more bugs.
        for (int i=0; i<10; ++i)
            Client_InsertNonInstanceElement("Instances");

        //  Once per component, import the component model
        Client_ImportCM(TEST_WIDGET_COMPONENT_NAME);

        PhysicalModelPtr catalogModel = getModelByName<PhysicalModel>(*m_clientDb, "Catalog");
        ASSERT_TRUE( catalogModel.IsValid() ) << "importing component should also import its catalog";

        // Now start placing instances of Widgets
        Client_PlaceInstance(w1, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, "wsln1", true);
        Client_PlaceInstance(w2, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, "wsln1", true);
        ASSERT_TRUE( w1.IsValid() );
        ASSERT_TRUE( w2.IsValid() );
        ASSERT_NE( w1.GetValue() , w2.GetValue() );
        Client_CheckComponentInstance(w1, 2, m_wsln1["X"].asDouble(), m_wsln1["Y"].asDouble(), m_wsln1["Z"].asDouble());
        Client_CheckComponentInstance(w2, 2, m_wsln1["X"].asDouble(), m_wsln1["Y"].asDouble(), m_wsln1["Z"].asDouble()); // 2nd instance of same solution should have the same instance geometry
    
        DgnElementId noidexpected;
        Client_PlaceInstance(noidexpected, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, "no_such_solution", false);
        ASSERT_FALSE(noidexpected.IsValid());

        //  Add a few unrelated elements to the target model. That way, the first placed CM instance will get an element id other than 1. Hopefully, that will help us catch more bugs.
        for (int i=0; i<5; ++i)
            Client_InsertNonInstanceElement("Instances");

        Client_PlaceInstance(w3, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, "wsln3", true);
    
        Client_CheckComponentInstance(w3, 2, m_wsln3["X"].asDouble(), m_wsln3["Y"].asDouble(), m_wsln3["Z"].asDouble());
        Client_CheckComponentInstance(w1, 2, m_wsln1["X"].asDouble(), m_wsln1["Y"].asDouble(), m_wsln1["Z"].asDouble());  // new instance of new solution should not affect existing instances of other solutions

        //  new instance of new solution should not affect existing instances of other solutions
        if (true)
            {
            DgnElementId w1_second_time;
            Client_PlaceInstance(w1_second_time, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, "wsln1", true);
            ASSERT_TRUE(w1_second_time.IsValid());
            Client_CheckComponentInstance(w1_second_time, 2, m_wsln1["X"].asDouble(), m_wsln1["Y"].asDouble(), m_wsln1["Z"].asDouble());  // new instance of new solution should not affect existing instances of other solutions
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
        PhysicalModelPtr catalogModel = getModelByName<PhysicalModel>(*m_clientDb, "Catalog");

        DgnElementId w4;
        Client_PlaceInstance(w4, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, "wsln4", true);
        ASSERT_TRUE(w4.IsValid());

        Client_CheckComponentInstance(w4, 2, m_wsln4["X"].asDouble(), m_wsln4["Y"].asDouble(), m_wsln4["Z"].asDouble());
        Client_CheckComponentInstance(w1, 2, m_wsln1["X"].asDouble(), m_wsln1["Y"].asDouble(), m_wsln1["Z"].asDouble());  // new instance of new solution should not affect existing instances of other solutions

        // Now start placing instances of Gadgets
        Client_ImportCM(TEST_GADGET_COMPONENT_NAME);
        DgnElementId g1, g2;
        Client_PlaceInstance(g1, "Instances", *catalogModel, TEST_GADGET_COMPONENT_NAME, "gsln1", true);
        Client_PlaceInstance(g2, "Instances", *catalogModel, TEST_GADGET_COMPONENT_NAME, "gsln1", true);
        ASSERT_TRUE(g1.IsValid());
        ASSERT_TRUE(g2.IsValid());

        Client_CheckComponentInstance(g1, 1, m_gsln1["Q"].asDouble(), m_gsln1["W"].asDouble(), m_gsln1["R"].asDouble());
        Client_CheckComponentInstance(g2, 1, m_gsln1["Q"].asDouble(), m_gsln1["W"].asDouble(), m_gsln1["R"].asDouble());

        //  And place another Widget
        DgnElementId w44;
        Client_PlaceInstance(w44, "Instances", *catalogModel, TEST_WIDGET_COMPONENT_NAME, "wsln44", true);
        ASSERT_TRUE(w44.IsValid());

        Client_CheckComponentInstance(w3, 2, m_wsln3["X"].asDouble(), m_wsln3["Y"].asDouble(), m_wsln3["Z"].asDouble());
        Client_CheckComponentInstance(w1, 2, m_wsln1["X"].asDouble(), m_wsln1["Y"].asDouble(), m_wsln1["Z"].asDouble());
        Client_CheckComponentInstance(g1, 1, m_gsln1["Q"].asDouble(), m_gsln1["W"].asDouble(), m_gsln1["R"].asDouble());
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

/*---------------------------------------------------------------------------------**//**
* Make a unique/singleton instance of the 'Thing' component.
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelTest, SimulateDeveloperAndClientWithNestingSingleton)
    {
    // For the purposes of this test, we'll put the Component and Client models in different DgnDbs
    m_componentDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_Component.idgndb");
    m_clientDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_ClientWithNestingSingleton.idgndb");
    BeTest::GetHost().GetOutputRoot(m_componentSchemaFileName);
    m_componentSchemaFileName.AppendToPath(TEST_JS_NAMESPACE_W L"0.0.ECSchema.xml");

    SimulateDeveloper();

    OpenClientDb(Db::OpenMode::ReadWrite);
    AutoCloseClientDb closeClientDbAtEnd(*this);

    //  Create the target model in the client. (Do this first, so that the first imported CM's will get a model id other than 1. Hopefully, that will help us catch more bugs.)
    PhysicalModelPtr targetModel;
    ASSERT_EQ( DgnDbStatus::Success , createPhysicalModel(targetModel, *m_clientDb, DgnModel::CreateModelCode("Instances")) );

    Client_ImportCM(TEST_GADGET_COMPONENT_NAME);
    Client_ImportCM(TEST_THING_COMPONENT_NAME);

    ComponentModelPtr nestingComponentModel = getModelByName<ComponentModel>(*m_clientDb, TEST_THING_COMPONENT_NAME);  // Open the client's imported copy
    ASSERT_TRUE( nestingComponentModel.IsValid() );

    DgnDbStatus status;
    ModelSolverDef::ParameterSet params = nestingComponentModel->GetSolver().GetParameters(); // get a copy of the component's parameters
    params.GetParameterP("A")->SetValue(ECN::ECValue(1));   // set some new values ...
    params.GetParameterP("B")->SetValue(ECN::ECValue(2));
    params.GetParameterP("C")->SetValue(ECN::ECValue(3));

    DgnElementCPtr instanceElement = nestingComponentModel->MakeInstance(&status, *targetModel, "", params); // create a unique/singleton instance with these parameters
    ASSERT_TRUE(instanceElement.IsValid()) << Utf8PrintfString("CreateInstanceItem failed with error code %x", status);
    Client_CheckComponentInstance(instanceElement->GetElementId(), 1, params.GetParameterP("A")->GetValue().GetDouble(), params.GetParameterP("B")->GetValue().GetDouble(), params.GetParameterP("C")->GetValue().GetDouble());

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
    m_componentDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_Component.idgndb");
    m_clientDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_ClientWithNesting.idgndb");
    BeTest::GetHost().GetOutputRoot(m_componentSchemaFileName);
    m_componentSchemaFileName.AppendToPath(TEST_JS_NAMESPACE_W L"0.0.ECSchema.xml");

    SimulateDeveloper();

    OpenClientDb(Db::OpenMode::ReadWrite);
    AutoCloseClientDb closeClientDbAtEnd(*this);

    //  Create the target model in the client. (Do this first, so that the first imported CM's will get a model id other than 1. Hopefully, that will help us catch more bugs.)
    PhysicalModelPtr targetModel;
    ASSERT_EQ( DgnDbStatus::Success , createPhysicalModel(targetModel, *m_clientDb, DgnModel::CreateModelCode("Instances")) );

    Client_ImportCM(TEST_GADGET_COMPONENT_NAME);
    Client_ImportCM(TEST_THING_COMPONENT_NAME);

    ComponentModelPtr nestingComponentModel = getModelByName<ComponentModel>(*m_clientDb, TEST_THING_COMPONENT_NAME);  // Open the client's imported copy
    ASSERT_TRUE( nestingComponentModel.IsValid() );

    PhysicalModelPtr catalogModel = getModelByName<PhysicalModel>(*m_clientDb, "Catalog");

    DgnElementId n1;
    Client_PlaceInstance(n1, "Instances", *catalogModel, TEST_THING_COMPONENT_NAME, "nsln1", true);
    ASSERT_TRUE(n1.IsValid());

    Client_CheckComponentInstance(n1, 1, m_nsln1["A"].asDouble(), m_nsln1["B"].asDouble(), m_nsln1["C"].asDouble());

    Client_CheckNestedInstance(*m_clientDb->Elements().GetElement(n1), TEST_GADGET_COMPONENT_NAME, 1);
    }


#endif //ndef BENTLEYCONFIG_NO_JAVASCRIPT
