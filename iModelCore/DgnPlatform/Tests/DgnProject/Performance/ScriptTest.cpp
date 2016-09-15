/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/ScriptTest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// This test does the same thing as ScriptTest.ts. It is used to compare native and script performance.
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../TestFixture/GenericDgnModelTestFixture.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnElementHelpers.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnDbUtilities.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/ElementGeometry.h>
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnElementDependency.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_DGNDB_UNIT_TESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

BEGIN_UNNAMED_NAMESPACE
/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct ScriptTest : ::testing::Test
{
    public: static void SetUpTestCase();
    public: static void TearDownTestCase();

    ScopedDgnHost m_host;
    DgnDbPtr      m_db;
    DgnModelId    m_defaultModelId;
    DgnCategoryId m_defaultCategoryId;
    static DgnDbTestUtils::SeedDbInfo s_seedFileInfo;

    ScriptTest();
    ~ScriptTest();
    void CloseDb() {m_db->CloseDb();}
    DgnModelR GetDefaultModel() {return *m_db->Models().GetModel(m_defaultModelId);}

    void SetupProject(WCharCP testFile, Db::OpenMode mode, bool needBriefcase = true);
    WString GetTestFileName(WCharCP testname) {return WPrintfString(L"%ls.bim",testname);}

    void PopulateProperties(GeometricElement3d& el);
};

DgnDbTestUtils::SeedDbInfo ScriptTest::s_seedFileInfo;

END_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
// Do one-time setup for all tests in this group
// In this case, I just request the (root) seed file that my tests will use and make a note of it.
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void ScriptTest::SetUpTestCase()
    {
    ScopedDgnHost tempHost;
    ScriptTest::s_seedFileInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(true, true));
    }

//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void ScriptTest::TearDownTestCase()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
ScriptTest::ScriptTest()
    {
    // Must register my domain whenever I initialize a host
    DgnPlatformTestDomain::Register();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
ScriptTest::~ScriptTest()
    {
    if (m_db.IsValid())
        m_db->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .bim project file after copying it to out
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ScriptTest::SetupProject(WCharCP testFile, Db::OpenMode mode, bool needBriefcase)
    {
    // Note: We know that our group's TC_SETUP function has already created the group seed file. We can just ask for it.
    if (Db::OpenMode::ReadWrite == mode)
        m_db = DgnDbTestUtils::OpenSeedDbCopy(s_seedFileInfo.fileName, testFile);
    else
        m_db = DgnDbTestUtils::OpenSeedDb(s_seedFileInfo.fileName);
    ASSERT_TRUE(m_db.IsValid());

    if (needBriefcase)
        {
        ASSERT_TRUE(m_db->IsBriefcase());
        ASSERT_TRUE(m_db->Txns().IsTracking());
        }

    m_defaultModelId = m_db->Models().QueryModelId(s_seedFileInfo.modelCode);
    DgnModelPtr defaultModel = m_db->Models().GetModel(m_defaultModelId);
    ASSERT_TRUE(defaultModel.IsValid());
    GetDefaultModel().FillModel();

    m_defaultCategoryId = DgnCategory::QueryCategoryId(s_seedFileInfo.categoryName, *m_db);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/16
//---------------------------------------------------------------------------------------
void ScriptTest::PopulateProperties(GeometricElement3d& el)
    {
    el.SetPropertyValue("IntegerProperty1", ECN::ECValue(1));
    el.SetPropertyValue("IntegerProperty2", ECN::ECValue(2));
    el.SetPropertyValue("IntegerProperty3", ECN::ECValue(3));
    el.SetPropertyValue("IntegerProperty4", ECN::ECValue(4));
    el.SetPropertyValue("DoubleProperty1",  ECN::ECValue(1));
    el.SetPropertyValue("DoubleProperty2",  ECN::ECValue(2));
    el.SetPropertyValue("DoubleProperty3",  ECN::ECValue(3));
    el.SetPropertyValue("DoubleProperty4",  ECN::ECValue(4));
    el.SetPropertyValue("PointProperty1",   ECN::ECValue(DPoint3d::From(1,2,3))); 
    el.SetPropertyValue("PointProperty2",   ECN::ECValue(DPoint3d::From(2,2,3))); 
    el.SetPropertyValue("PointProperty3",   ECN::ECValue(DPoint3d::From(3,2,3))); 
    el.SetPropertyValue("PointProperty4",   ECN::ECValue(DPoint3d::From(4,2,3))); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      05/16
//---------------------------------------------------------------------------------------
static ISolidPrimitivePtr createSphere (DPoint3dCR center, double radius)
    {
    DgnSphereDetail data (center, radius);
    return ISolidPrimitive::CreateDgnSphere (data);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      05/16
//---------------------------------------------------------------------------------------
static ISolidPrimitivePtr createCenteredBox (DPoint3dCR center, DVec3dCR diagonalSize, bool capped)
    {
    auto data = DgnBoxDetail::InitFromCenterAndSize(center, diagonalSize, capped);
    return ISolidPrimitive::CreateDgnBox (data);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      05/16
//---------------------------------------------------------------------------------------
static ISolidPrimitivePtr createCircularCone (DPoint3dCR pointA, DPoint3dCR pointB, double radiusA, double radiusB, bool capped)
    {
    DgnConeDetail coneData (pointA, pointB, radiusA, radiusB, capped);
    return ISolidPrimitive::CreateDgnCone (coneData);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/16
//---------------------------------------------------------------------------------------
TEST_F(ScriptTest, Test)
    {
    SetupProject(GetTestFileName(L"Test").c_str(), Db::OpenMode::ReadWrite);
    ASSERT_TRUE(m_db.IsValid());

    auto& db = *m_db;

    auto model = db.Models().GetModel(m_defaultModelId);//db.Models.QueryModelId(DgnModel.CreateModelCode(params.newModelName)));
    auto catid = m_defaultCategoryId; // DgnCategory::QueryCategoryId(params.categoryName, db);

    //auto starti = Date.now();
    double lowcommit = 9999999;
    double highcommit = 0;
    int ncommits = 0;
    double totalcommit = 0;

    StopWatch timer(true);

    for (auto i = 0; i < 10; ++i) {

        //auto startj = Date.now();
        Utf8String kvalues = "";
        Utf8CP comma = "";

        for (auto j = 0; j < 10; ++j) {

            auto startk = timer.GetCurrentSeconds();

            for (auto k = 0; k < 10000; ++k) {
                //auto ele = GeometricElement3d::CreateGeometricElement3d(model, catid, 'DgnPlatformTest.TestElementWithNoHandler');
                auto ele = TestElement::Create(db, model->GetModelId(), catid);
                auto builder = GeometryBuilder::Create(*model, catid, DPoint3d::From(i, j, k), YawPitchRollAngles::FromDegrees(0, 0, 0));
                ISolidPrimitivePtr geom;
                if (k % 3 == 0)
                    geom = createSphere(DPoint3d::From(0, 0, 0), 1.0);
                else if (k % 3 == 1)
                    geom = createCenteredBox(DPoint3d::From(0, 0, 0), DVec3d::From(1, 1, 1), true);
                else
                    geom = createCircularCone(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, 1), 1, 1, true);
                builder->Append(*geom);
                if (0 != builder->Finish(*ele)) {
                    FAIL() << "Finish failed";
                    return;
                }
                PopulateProperties(*ele);
                if (!ele->Insert().IsValid()) {
                    FAIL() << "Insert failed";
                    return;
                }
                // Release the objects that I know that I don't need any more.
                /*builder->Dispose();*/ builder = nullptr;
                /*geom.Dispose();*/ geom = nullptr;
                /*ele.Dispose();*/ ele = nullptr;
            }

            auto endk = timer.GetCurrentSeconds();
            auto elapsedk = endk - startk;
            kvalues = kvalues + comma + Utf8PrintfString("%lf", elapsedk);
            comma = ",";

            db.SaveChanges();

            auto endcommitk = timer.GetCurrentSeconds();
            auto elapsedcommit = endcommitk - endk;
            //Logging.Message('DgnScriptTest', LoggingSeverity.Info, "k commit:" + elapsedk);
            if (elapsedcommit < lowcommit)
                lowcommit = elapsedcommit;
            if (elapsedcommit > highcommit)
                highcommit = elapsedcommit;
            totalcommit += elapsedcommit;
            ++ncommits;
            
        }

        printf("DgnScriptTest - %s\n", kvalues.c_str());
        //auto endj = Date.now();
        //auto elapsedj = endj - startj;
        //Logging.Message('DgnScriptTest', LoggingSeverity.Info, "0:" + elapsedj);
    }

    //auto endi = Date.now();
    //auto elapsedi = endi - starti;
    //Logging.Message('DgnScriptTest', LoggingSeverity.Info, "i:" + elapsedi);

    printf("DgnScriptTest - commit time: low=%lf high=%lf avg=%lf\n", lowcommit, highcommit, (totalcommit / ncommits));
    }
