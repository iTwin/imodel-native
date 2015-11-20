/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/ComponentModel_PerfTest.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef BENTLEYCONFIG_NO_JAVASCRIPT
#include "PerformanceTestFixture.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/ECUtils.h>
#include <DgnPlatform/DgnScript.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeNumerical.h>
#include <Logging/bentleylogging.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE

#define TEST_JS_NAMESPACE    "ComponentModelPerfTest"
#define TEST_JS_NAMESPACE_W L"ComponentModelPerfTest"
#define TEST_BOXES_COMPONENT_NAME "Boxes"

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
struct ComponentModelPerfTest : public testing::Test
{
protected:
BeFileName         m_componentDbName;
BeFileName         m_clientDbName;
DgnDbPtr           m_componentDb;
DgnDbPtr           m_clientDb;
Dgn::ScopedDgnHost m_host;
FakeScriptLibrary  m_scriptLibrary;

ComponentModelPerfTest();
void AddToFakeScriptLibrary(Utf8CP jns, Utf8CP jtext);
DgnCategoryId Developer_CreateCategory(Utf8CP code, ColorDef const&);
void Developer_CreateCMs();
void OpenComponentDb(DgnDb::OpenMode mode) {openDb(m_componentDb, m_componentDbName, mode);}
void CloseComponentDb() {m_componentDb->CloseDb(); m_componentDb=nullptr;}
void OpenClientDb(DgnDb::OpenMode mode) {openDb(m_clientDb, m_clientDbName, mode);}
void CloseClientDb() {m_clientDb->CloseDb(); m_clientDb=nullptr;}
void Client_ImportCM(Utf8CP componentName);
void Client_SolveAndCapture(PhysicalElementCPtr&, PhysicalModelR catalogModel, Utf8CP componentName, Json::Value const& parms, Utf8StringCR);

void PlaceInstances(int ninstances, int boxCount, DPoint3d boxSize);
void PlaceElements(int ninstances, int boxCount, DPoint3d boxSize);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentModelPerfTest::ComponentModelPerfTest()
    {
    m_host.SetFetchScriptCallback(&m_scriptLibrary);// In this test, we redirect all requests for JS programs to our fake library
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelPerfTest::AddToFakeScriptLibrary(Utf8CP jns, Utf8CP jtext)
    {
    // In this test, there is only one JS program in the fake library at a time.
    m_scriptLibrary.m_jsProgramName = jns;
    m_scriptLibrary.m_jsProgramText = jtext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId ComponentModelPerfTest::Developer_CreateCategory(Utf8CP code, ColorDef const& color)
    {
    DgnCategory cat(DgnCategory::CreateParams(*m_componentDb, code, DgnCategory::Scope::Any));
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);
    cat.Insert(appearance);
    return cat.GetCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* This function defines 2 ComponenModels: a Boxes and a TwentyBoxes.
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelPerfTest::Developer_CreateCMs()
    {
    OpenComponentDb(Db::OpenMode::ReadWrite);

    ASSERT_TRUE(m_componentDb.IsValid());

    // Define the CM's Element Category (in the CM's DgnDb). Use the same name as the component model. 
    ASSERT_TRUE( Developer_CreateCategory("Boxes", ColorDef(0xff,0x00,0x00)).IsValid() );

    // Define the Solver wparameters for use by this model.
    //ModelSolverDef::Parameter::Scope ip = ModelSolverDef::Parameter::Scope::Instance;
    ModelSolverDef::Parameter::Scope tp = ModelSolverDef::Parameter::Scope::Type;
    bvector<ModelSolverDef::Parameter> wparameters;
    wparameters.push_back(ModelSolverDef::Parameter("H", tp, ECN::ECValue(1.0))); 
    wparameters.push_back(ModelSolverDef::Parameter("W", tp, ECN::ECValue(1.0))); 
    wparameters.push_back(ModelSolverDef::Parameter("D", tp, ECN::ECValue(1.0))); 
    wparameters.push_back(ModelSolverDef::Parameter("box_count", tp, ECN::ECValue(1.0)));
    ModelSolverDef wsolver(ModelSolverDef::Type::Script, TEST_JS_NAMESPACE ".Boxes", wparameters); // Identify the JS solver that should be used. Note: this JS program must be in the script library

    // Create the models
    ComponentModel::CreateParams wparms(*m_componentDb, TEST_BOXES_COMPONENT_NAME, "dgn.PhysicalElement", "Boxes", "", wsolver);
    ComponentModelPtr wcm = new ComponentModel(wparms);
    ASSERT_TRUE( wcm->IsValid() );
    ASSERT_EQ( DgnDbStatus::Success , wcm->Insert() );       /* Insert the new model into the DgnDb */

    // Here is the model solver that should be used. 
    // Note that we must put it into the Script library under the same name that was used in the model definition above.
    // It must also register itself as a model solver under the same name as was used in the model definition above
    // Note that a script will generally create elements from scratch. That's why it starts by deleting all elements in the model. They would have been the outputs of the last run.
    AddToFakeScriptLibrary(TEST_JS_NAMESPACE, 
"(function () { \
    function makeBoxes(model, params, options) { \
        model.DeleteAllElements();\
        var angles = new BentleyApi.Dgn.JsYawPitchRollAngles(0,0,0);\
        for (var i = 0; i < params.box_count; i++)\
            {\
            var element = model.CreateElement('dgn.PhysicalElement', options.Category);\
            var origin = new BentleyApi.Dgn.JsDPoint3d(i,i,i);\
            var builder = new BentleyApi.Dgn.JsElementGeometryBuilder(element, origin, angles); \
            builder.AppendBox(params.H, params.W, params.D); \
            builder.SetGeomStreamAndPlacement(element); \
            element.Insert(); \
            }\
        return 0;\
    } \
    BentleyApi.Dgn.RegisterModelSolver('" TEST_JS_NAMESPACE ".Boxes" "', makeBoxes); \
})();\
");
    ASSERT_TRUE( wcm.IsValid() );

    m_componentDb->SaveChanges(); // should trigger validation

    CloseComponentDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelPerfTest::Client_ImportCM(Utf8CP componentName)
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
void ComponentModelPerfTest::Client_SolveAndCapture(PhysicalElementCPtr& catalogItem, PhysicalModelR catalogModel, Utf8CP componentName, Json::Value const& parmsToChange, Utf8StringCR ciname)
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
    catalogItem = componentModel->CaptureSolution(&status, catalogModel, newParameterValues, ciname);
    ASSERT_TRUE(catalogItem.IsValid()) << Utf8PrintfString("ComponentModel::CaptureSolution failed with error %x", status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
static void insertBoxesElement(DgnElementId& eid, PhysicalModelR physicalTestModel, DgnCategoryId testCategoryId, DPoint3dCR placementOrigin, DPoint3dCR sizeOfBlock, bvector<DPoint3d> const& originsOfBlocks)
    {
    PhysicalElementPtr testElement = PhysicalElement::Create(physicalTestModel, testCategoryId);

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(physicalTestModel, testCategoryId, placementOrigin, YawPitchRollAngles());
    for (auto const& originOfBlock : originsOfBlocks)
        {
        DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(originOfBlock, sizeOfBlock, true);
        ISolidPrimitivePtr testGeomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
        builder->Append(*testGeomPtr);
        }

    builder->SetGeomStreamAndPlacement(*testElement);

    eid = physicalTestModel.GetDgnDb().Elements().Insert(*testElement)->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelPerfTest::PlaceInstances(int ninstances, int boxCount, DPoint3d boxSize)
    {
    ASSERT_TRUE(m_componentDbName.DoesPathExist());
    ASSERT_TRUE(m_clientDbName.DoesPathExist());

    // Create component models (in component db)
    Developer_CreateCMs();

    OpenClientDb(Db::OpenMode::ReadWrite);

    //  Create the catalog model in the client.
    PhysicalModelPtr catalogModel;
    ASSERT_EQ( DgnDbStatus::Success , createPhysicalModel(catalogModel, *m_clientDb, DgnModel::CreateModelCode("Catalog")) );

    //  Create the target model in the client.
    PhysicalModelPtr targetModel;
    ASSERT_EQ( DgnDbStatus::Success , createPhysicalModel(targetModel, *m_clientDb, DgnModel::CreateModelCode("Instances")) );

    StopWatch timer("place components");
    timer.Start();

    //  Import the component model
    Client_ImportCM(TEST_BOXES_COMPONENT_NAME);

    //  Cache a solution
    Json::Value parameters(Json::objectValue);
    parameters["H"] = boxSize.x;
    parameters["W"] = boxSize.y;
    parameters["D"] = boxSize.z;
    parameters["box_count"] = boxCount;
    DgnElementId w1;
    PhysicalElementCPtr catalogItem;
    Client_SolveAndCapture(catalogItem, *catalogModel, TEST_BOXES_COMPONENT_NAME, parameters, "catalog_item_name");

    DgnDbStatus status;
    YawPitchRollAngles placementAngles;

    //  Place instances of this solution
    for (int i=0; i<ninstances; ++i)
        {
        DPoint3d placementOrigin = DPoint3d::From(-i,-i,-i);
        ComponentModel::MakeInstanceOfSolution(&status, *targetModel, *catalogItem, placementOrigin, placementAngles, DgnElement::Code());
        }
    timer.Stop();
    NativeLogging::LoggingManager::GetLogger("Performance")->infov("place instances of %d solutions: %lf seconds (%lf instances / second)", ninstances, timer.GetElapsedSeconds(), ninstances/timer.GetElapsedSeconds());

    m_clientDb->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelPerfTest::PlaceElements(int ninstances, int boxCount, DPoint3d boxSize)
    {
    ASSERT_TRUE(m_clientDbName.DoesPathExist());

    OpenClientDb(Db::OpenMode::ReadWrite);
    
    PhysicalModelPtr targetModel;
    createPhysicalModel(targetModel, *m_clientDb, DgnModel::CreateModelCode("Instances"));
    DgnCategoryId someCat = DgnCategory::QueryFirstCategoryId(*m_clientDb);

    bvector<DPoint3d> originsOfBoxes;
    for (int i=0; i<boxCount; ++i)
        originsOfBoxes.push_back(DPoint3d::From(i,i,i));

    StopWatch timer("place components");
    timer.Start();
    for (int i=0; i<ninstances; ++i)
        {
        DPoint3d placementOrigin = DPoint3d::From(-i,-i,-i);
        DgnElementId eid;
        insertBoxesElement(eid, *targetModel, someCat, placementOrigin, boxSize, originsOfBoxes);
        }
    timer.Stop();
    NativeLogging::LoggingManager::GetLogger("Performance")->infov("place %d plain physical elements: %lf seconds (%lf instances / second)", ninstances, timer.GetElapsedSeconds(), ninstances/timer.GetElapsedSeconds());
    
    m_clientDb->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelPerfTest, PlaceInstances_TwoBoxes)
    {
    m_componentDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_Component2.idgndb");
    m_clientDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_Performance_PlaceInstances2.idgndb");
    PlaceInstances(100000, 2, DPoint3d::From(10,12,13));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelPerfTest, PlaceElements_TwoBoxes)
    {
    m_clientDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_Performance_PlaceElements2.idgndb");
    PlaceElements(100000, 2, DPoint3d::From(10,12,13));
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelPerfTest, PlaceInstances_TwentyBoxes)
    {
    m_componentDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_Component20.idgndb");
    m_clientDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_Performance_PlaceInstances20.idgndb");
    PlaceInstances(100000, 20, DPoint3d::From(10,12,13));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelPerfTest, PlaceElements_TwentyBoxes)
    {
    m_clientDbName = copyDb(L"DgnDb/3dMetricGeneral.idgndb", L"ComponentModelTest_Performance_PlaceElements20.idgndb");
    PlaceElements(100000, 20, DPoint3d::From(10,12,13));
    };

#endif //ndef BENTLEYCONFIG_NO_JAVASCRIPT
