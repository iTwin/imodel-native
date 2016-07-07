/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/ComponentModel_PerfTest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    ASSERT_TRUE( db.IsValid() ) << WPrintfString(L"Failed to open %ls in mode %d => result=%x", name.c_str(), (int)mode, (int)result).c_str();
    ASSERT_EQ( BE_SQLITE_OK , result );
    TestDataManager::MustBeBriefcase(db, mode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus createSpatialModel(SpatialModelPtr& catalogModel, DgnDbR db, DgnCode const& code)
    {
    DgnClassId mclassId = DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel));
    catalogModel = new SpatialModel(SpatialModel::CreateParams(db, mclassId, code));
    catalogModel->SetInGuiList(false);
    return catalogModel->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
RefCountedPtr<T> getModelByName(DgnDbR db, Utf8CP cmname)
    {
    return db.Models().Get<T>(db.Models().QueryModelId(DgnModel::CreateModelCode(cmname)));
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

    ECN::ECSchemaPtr testSchema = ComponentDefCreator::GenerateSchema(*m_componentDb, TEST_JS_NAMESPACE);
    ASSERT_TRUE(testSchema.IsValid());

    ECN::ECClassCP baseClass = m_componentDb->Schemas().GetECClass(BIS_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement);

    TsComponentParameterSet params;
    params["H"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));
    params["W"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));
    params["D"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue(1.0));
    params["box_count"] = TsComponentParameter(ComponentDef::ParameterVariesPer::Instance, ECN::ECValue("text"));

    ComponentDefCreator creator(*m_componentDb, *testSchema, TEST_BOXES_COMPONENT_NAME, *baseClass, TEST_JS_NAMESPACE "." TEST_BOXES_COMPONENT_NAME, "Boxes", "", params);
    ECN::ECClassCP ecClass = creator.GenerateECClass();
    ASSERT_TRUE(nullptr != ecClass);

    ASSERT_TRUE(ComponentDefCreator::ImportSchema(*m_componentDb, *testSchema, false) != nullptr);

    // Here is the model solver that should be used. 
    // Note that we must put it into the Script library under the same name that was used in the model definition above.
    // It must also register itself as a model solver under the same name as was used in the model definition above
    // Note that a script will generally create elements from scratch. That's why it starts by deleting all elements in the model. They would have been the outputs of the last run.
    AddToFakeScriptLibrary(TEST_JS_NAMESPACE, 
"(function () { \
    function makeBoxes(model, params, options) { \
        var angles = new Bentley.Dgn.YawPitchRollAngles(0,0,0);\
        for (var i = 0; i < params.box_count; i++)\
            {\
            var boxSize = new be.DPoint3d(params.H, params.W, params.D); \
            var solid = be.DgnBox.CreateCenteredBox(new be.DPoint3d(0,0,0), boxSize, true); \
            var element = model.CreateElement('dgn.PhysicalElement', options.Category);\
            var origin = new Bentley.Dgn.DPoint3d(i,i,i);\
            var builder = new Bentley.Dgn.GeometryBuilder(element, origin, angles); \
            builder.Append(solid); \
            builder.SetGeometryStreamAndPlacement(element); \
            element.Insert(); \
            }\
        return 0;\
    } \
    Bentley.Dgn.RegisterModelSolver('" TEST_JS_NAMESPACE ".Boxes" "', makeBoxes); \
})();\
");

    m_componentDb->SaveChanges(); // should trigger validation

    CloseComponentDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
static void insertBoxesElement(DgnElementId& eid, SpatialModelR physicalTestModel, DgnCategoryId testCategoryId, DPoint3dCR placementOrigin, DPoint3dCR sizeOfBlock, bvector<DPoint3d> const& originsOfBlocks)
    {
    GenericPhysicalObjectPtr testElement = GenericPhysicalObject::Create(physicalTestModel, testCategoryId);

    GeometryBuilderPtr builder = GeometryBuilder::Create(physicalTestModel, testCategoryId, placementOrigin, YawPitchRollAngles());
    for (auto const& originOfBlock : originsOfBlocks)
        {
        DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(originOfBlock, sizeOfBlock, true);
        ISolidPrimitivePtr testGeomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
        builder->Append(*testGeomPtr);
        }

    builder->SetGeometryStreamAndPlacement(*testElement);

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

    if (true)
        {
        OpenClientDb(Db::OpenMode::ReadWrite);

        //  Import the component model
        OpenComponentDb(Db::OpenMode::Readonly);
        m_componentDb->Schemas().GetECSchema(TEST_JS_NAMESPACE, true);
        ComponentDefPtr sourceCdef = ComponentDef::FromECSqlName(nullptr, *m_componentDb, Utf8PrintfString("%s.%s", TEST_JS_NAMESPACE, TEST_BOXES_COMPONENT_NAME));
        DgnImportContext ctx(*m_componentDb, *m_clientDb);
        ASSERT_EQ( DgnDbStatus::Success , sourceCdef->Export(ctx));
        CloseComponentDb();

        m_clientDb->SaveChanges();
        CloseClientDb();
        }

    OpenClientDb(Db::OpenMode::ReadWrite);

    //  Create the catalog model in the client.
    SpatialModelPtr catalogModel;
    ASSERT_EQ( DgnDbStatus::Success , createSpatialModel(catalogModel, *m_clientDb, DgnModel::CreateModelCode("Catalog")) );

    //  Create the target model in the client.
    SpatialModelPtr targetModel;
    ASSERT_EQ( DgnDbStatus::Success , createSpatialModel(targetModel, *m_clientDb, DgnModel::CreateModelCode("Instances")) );

    StopWatch timer("place components");
    timer.Start();
        {
        ComponentDefPtr cdef = ComponentDef::FromECSqlName(nullptr, *m_clientDb, TEST_JS_NAMESPACE "." TEST_BOXES_COMPONENT_NAME);
        ASSERT_TRUE(cdef.IsValid());

        //  Cache a solution
        ECN::IECInstancePtr params = cdef->MakeVariationSpec();
        params->SetValue("H", ECN::ECValue(boxSize.x));
        params->SetValue("W", ECN::ECValue(boxSize.y));
        params->SetValue("D", ECN::ECValue(boxSize.z));
        params->SetValue("box_count", ECN::ECValue(boxCount));
        DgnElementCPtr variation = cdef->MakeVariation(nullptr, *catalogModel, *params, "stuff");
        ASSERT_TRUE(variation.IsValid());

        DgnDbStatus status;
        YawPitchRollAngles placementAngles;

        //  Place instances of this solution
        for (int i=0; i<ninstances; ++i)
            {
            DgnElementCPtr instance = cdef->MakeInstanceOfVariation(&status, *targetModel, *variation, nullptr);

            auto pinst = instance->MakeCopy<GeometricElement3d>();
            Placement3d placement;
            placement.GetOriginR() = DPoint3d::From(-i,-i,-i);
            pinst->SetPlacement(placement);
            pinst->Update();
            }
        }
    timer.Stop();
    NativeLogging::LoggingManager::GetLogger("Performance")->infov("place instances of %d solutions: %lf seconds (%lf instances / second)", ninstances, timer.GetElapsedSeconds(), ninstances/timer.GetElapsedSeconds());

    m_clientDb->SaveChanges();

    CloseClientDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModelPerfTest::PlaceElements(int ninstances, int boxCount, DPoint3d boxSize)
    {
    ASSERT_TRUE(m_clientDbName.DoesPathExist());

    OpenClientDb(Db::OpenMode::ReadWrite);
    
    SpatialModelPtr targetModel;
    createSpatialModel(targetModel, *m_clientDb, DgnModel::CreateModelCode("Instances"));
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
    m_componentDbName = copyDb(L"DgnDb/3dMetricGeneral.ibim", L"ComponentModelTest_Component2.ibim");
    m_clientDbName = copyDb(L"DgnDb/3dMetricGeneral.ibim", L"ComponentModelTest_Performance_PlaceInstances2.ibim");
    PlaceInstances(100000, 2, DPoint3d::From(10,12,13));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelPerfTest, PlaceElements_TwoBoxes)
    {
    m_clientDbName = copyDb(L"DgnDb/3dMetricGeneral.ibim", L"ComponentModelTest_Performance_PlaceElements2.ibim");
    PlaceElements(100000, 2, DPoint3d::From(10,12,13));
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelPerfTest, PlaceInstances_TwentyBoxes)
    {
    m_componentDbName = copyDb(L"DgnDb/3dMetricGeneral.ibim", L"ComponentModelTest_Component20.ibim");
    m_clientDbName = copyDb(L"DgnDb/3dMetricGeneral.ibim", L"ComponentModelTest_Performance_PlaceInstances20.ibim");
    PlaceInstances(100000, 20, DPoint3d::From(10,12,13));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ComponentModelPerfTest, PlaceElements_TwentyBoxes)
    {
    m_clientDbName = copyDb(L"DgnDb/3dMetricGeneral.ibim", L"ComponentModelTest_Performance_PlaceElements20.ibim");
    PlaceElements(100000, 20, DPoint3d::From(10,12,13));
    };

#endif //ndef BENTLEYCONFIG_NO_JAVASCRIPT

