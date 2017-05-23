/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnScriptContext_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef BENTLEYCONFIG_NO_JAVASCRIPT
#include "DgnHandlersTests.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"
#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnScript.h>
#include <DgnPlatform/GenericDomain.h>
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

//#define RUN_SCRIPT_PERFORMANCE_TESTS

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static RefCountedCPtr<DgnElement> insertElement(DgnModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId mid = model.GetModelId();

    DgnElementPtr gelem;
    if (model.Is3d())
        {
        DgnCategoryId categoryId = DgnDbTestUtils::GetFirstSpatialCategoryId(db);
        gelem = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(db, mid, DgnClassId(db.Schemas().GetClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject)), categoryId, Placement3d()));
        }
    else
        {
        DgnCategoryId categoryId = DgnDbTestUtils::GetFirstDrawingCategoryId(db);
        gelem = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, mid, DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AnnotationElement2d)), categoryId, Placement2d()));
        }

    GeometryBuilderPtr builder = GeometryBuilder::Create(*gelem->ToGeometrySource());
    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1,0,0))));

    if (BSISUCCESS != builder->Finish(*gelem->ToGeometrySourceP()))  // We actually catch 2d3d mismatch in Finish
        return nullptr;

    return db.Elements().Insert(*gelem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct JsProg : ScopedDgnHost::FetchScriptCallback
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static void checkGeometryStream(GeometrySourceCR gel, GeometricPrimitive::GeometryType exptectedType, size_t expectedCount)
    {
    //  Verify that item generated a line
    size_t count=0;
    for (auto iter : GeometryCollection (gel))
        {
        GeometricPrimitivePtr geom = iter.GetGeometryPtr();

        if (!geom.IsValid())
            continue;

        ASSERT_EQ( exptectedType , geom->GetGeometryType() );
        ++count;
        }
    ASSERT_EQ( expectedCount , count );
    }
//=======================================================================================
// @bsiclass                                                    Umar.Hayat     11/2015
//=======================================================================================
struct DgnScriptTest : public DgnDbTestFixture
{
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnScriptTest, TestEga)
    {
    static bool s_hasRunOnce;

    SetupSeedProject();
    DgnDbP project = m_db.get();// tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    PhysicalModelPtr model = GetDefaultPhysicalModel();

    RefCountedPtr<DgnElement> el;
        {
        RefCountedCPtr<DgnElement> newel = insertElement(*model);
        ASSERT_TRUE( newel.IsValid() );

        checkGeometryStream(*newel->ToGeometrySource(), GeometricPrimitive::GeometryType::CurvePrimitive, 1);
        ASSERT_TRUE( (*(GeometryCollection (*newel->ToGeometrySource()).begin())).GetGeometryPtr()->GetAsICurvePrimitive()->GetLineStringCP() != nullptr ) << "Initial geometry should be a line";

        el = newel->CopyForEdit();
        }

    DPoint3d org = DPoint3d::FromZero();
    YawPitchRollAngles angles;
    Json::Value parms (Json::objectValue);

    if (!s_hasRunOnce)
        {
        // This test is only valid if the EGA below is not yet registered. If somebody runs gtest with a repeat count,
        //  then we'll get here a second time in the same session (using the same BeJsContext), and we would find
        //  that the EGA is registered.
        for (int i=0; i<2; ++i)
            {
            int sres = -1;
            DgnDbStatus xstatus = DgnScript::ExecuteEga(sres, *el, "DgnScriptTest.TestEga", org, angles, parms);
            ASSERT_NE( DgnDbStatus::Success , xstatus ) << "Haven't registered the EGA yet";
            ASSERT_NE( 0 , sres );
            }
        }
    s_hasRunOnce = true;

    JsProg jsProg;
    jsProg.m_jsProgramName = "DgnScriptTest";
    jsProg.m_jsProgramText =
"(function () { \
    function testEga(element, origin, angles, params) { \
        var gsource = element.ToGeometrySource(); \
        if (!gsource) \
            { \
            Bentley.Dgn.Script.ReportError('You must pass a geometry source of some kind to an EGA. You passed in an element that is not a geometry source.');\
            return; \
            } \
        var g3d = element.ToGeometrySource3d(); \
        if (!g3d) \
            { \
            Bentley.Dgn.Script.ReportError('This particular EGA expects a geometry source 3D. You passed in a 2D geometry source.');\
            return; \
            } \
        var boxSize = new Bentley.Dgn.DPoint3d(params.X, params.Y, params.Z); \
        var box = Bentley.Dgn.DgnBox.CreateCenteredBox (new Bentley.Dgn.DPoint3d(0,0,0), boxSize, true); \
        var builder = Bentley.Dgn.GeometryBuilder.CreateFor3dModel(element.Model, g3d.CategoryId, origin, angles); \
        builder.AppendGeometry(box, 0); \
        builder.Finish(element); \
        return 0;\
    } \
    function testEgaBadReturn(element, origin, angles, params) { return 'abc'; }\
    Bentley.Dgn.RegisterEGA('DgnScriptTest.TestEga', testEga); \
    Bentley.Dgn.RegisterEGA('DgnScriptTest.TestEgaBadReturn', testEgaBadReturn); \
})();\
";

    m_host.SetFetchScriptCallback(&jsProg);

    parms["X"] = 1.0;
    parms["Y"] = 2.0;
    parms["Z"] = 3.0;

    StopWatch timeIt(true);
    int niters = 1000;
    for (int i=0; i<niters; ++i)
        {
        int sres;
        DgnDbStatus xstatus = DgnScript::ExecuteEga(sres, *el, "DgnScriptTest.TestEga", org, angles, parms);
        ASSERT_EQ( DgnDbStatus::Success , xstatus );
        ASSERT_EQ( 0 , sres );

        checkGeometryStream(*el->ToGeometrySource(), GeometricPrimitive::GeometryType::SolidPrimitive, 1);
        DgnBoxDetail box;
        ASSERT_TRUE( (*(GeometryCollection (*el->ToGeometrySource()).begin())).GetGeometryPtr()->GetAsISolidPrimitive()->TryGetDgnBoxDetail(box) ) << "Geometry should be a slab";
        ASSERT_EQ( box.m_baseX , parms["X"].asDouble() );
        ASSERT_EQ( box.m_baseY , parms["Y"].asDouble() );
        ASSERT_EQ( box.m_topOrigin.Distance(box.m_baseOrigin) , parms["Z"].asDouble() );

        parms["Z"] = parms["Z"].asDouble() + 1;
        }
    timeIt.Stop();
    BeTest::Log("DgnScriptTest", BeTest::LogPriority::PRIORITY_ERROR, Utf8PrintfString("%d / %lf seconds = %lf/second\n", niters, timeIt.GetElapsedSeconds(), niters/timeIt.GetElapsedSeconds()).c_str());

    // Check that attempting to call a non-registered function fails with a non-zero xstatus
    if (true)
        {
        int sres = -1;
        BeTest::SetFailOnAssert(false);
        DgnDbStatus xstatus = DgnScript::ExecuteEga(sres, *el, "DgnScriptTest.TestEgaNotRegistered", org, angles, parms);
        BeTest::SetFailOnAssert(true);
        ASSERT_EQ( DgnDbStatus::NotEnabled , xstatus ) << "this function is not registered so the attempt should fail";
        ASSERT_EQ( -1 , sres );
        }

    // Check that attempting to call a registered function that returns anything other than a integer fails with a non-zero xstatus
    if (true)
        {
        int sres = -1;
        BeTest::SetFailOnAssert(false);
        DgnDbStatus xstatus = DgnScript::ExecuteEga(sres, *el, "DgnScriptTest.TestEgaBadReturn", org, angles, parms);
        BeTest::SetFailOnAssert(true);
        ASSERT_EQ( DgnDbStatus::NotEnabled , xstatus ) << "this function should not be callable so the attempt should fail";
        ASSERT_EQ( -1 , sres );
        }
    }

BeConditionVariable s_mtCv;
struct MtStats {int failed; int accum; int finished;};
static MtStats s_mtStats;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void runScript(DgnScriptTest* thisTest)
    {
    int sres;
    DgnDbStatus xstatus;
        {
        DgnScriptThreadEnabler canEvaluateScriptsInThisScope;

        Json::Value parms (Json::objectValue);

        xstatus = DgnScript::ExecuteDgnDbScript(sres, thisTest->GetDgnDb(), "DgnScriptTestMT.ScriptsInMultipleThreads", parms);
        }

    BeMutexHolder _v_(s_mtCv.GetMutex());

    if (DgnDbStatus::Success != xstatus)
        s_mtStats.failed++;

    s_mtStats.accum += sres;

    s_mtStats.finished++;

    s_mtCv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnScriptTest, ScriptsInMultipleThreads)
    {
    SetupSeedProject();
    DgnDbP project = m_db.get();// tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );
    if (true)
        {
        PhysicalModelPtr model = GetDefaultPhysicalModel();
        RefCountedCPtr<DgnElement> newel = insertElement(*model);
        ASSERT_TRUE( newel.IsValid() );
        }
    
    //  Set up the script that all threads will run.
    //  Remember that the "host" is global, not thread-specific.
    JsProg jsProg;
    jsProg.m_jsProgramName = "DgnScriptTestMT";
    jsProg.m_jsProgramText =
"(function () { \
    function testMT(db, params) { \
        junk = db.Elements;\
        return 1;\
    } \
    Bentley.Dgn.RegisterDgnDbScript('DgnScriptTestMT.ScriptsInMultipleThreads', testMT); \
})();\
";
    m_host.SetFetchScriptCallback(&jsProg);

    if (true)
        {
        //  Execute this script in this thread, using auto-initialization of the jscontext
        Json::Value parms (Json::objectValue);
        int sres;
        DgnScript::ExecuteDgnDbScript(sres, GetDgnDb(), "DgnScriptTestMT.ScriptsInMultipleThreads", parms);
        }

    //  Execute this script in this thread using explicit initialization of the jscontext
    runScript(this);

    //  Execute this script in multiple threads
    std::thread threads[3];
    for (auto& thread : threads)
        thread = std::thread(runScript, this);

    BeMutexHolder lock(s_mtCv.GetMutex());
    while (s_mtStats.finished != (1 + _countof(threads)))
        s_mtCv.InfiniteWait(lock);
    
    for (auto& thread : threads)
        thread.detach();

    //  Execute this script in this thread again
    runScript(this);

    ASSERT_EQ(2 + _countof(threads), s_mtStats.accum);
    ASSERT_EQ(0, s_mtStats.failed);

    memset(&s_mtStats, 0, sizeof(s_mtStats)); // in case this test is executed again in the same session due to a repeat count

    T_HOST.GetScriptAdmin().CheckCleanup();
    }

/*=================================================================================**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+===============+===============+===============+===============+===============+======*/
namespace 
{
struct DgnScriptTest_DetectJsErrors : DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler
    {
    void _HandleScriptError(BeJsContextR, Category category, Utf8CP description, Utf8CP details) override
        {
        //enum class Category {ReportedByScript, ParseError, Exception, Other};
        static char const* s_errTypes[] = {"ReportedByScript", "ParseError", "Exception", "Other"};
        FAIL() << Utf8PrintfString("JavaScript error %s: %s, %s", s_errTypes[(int)category], description, details).c_str();
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
TEST_F(DgnScriptTest, RunScripts)
    {
    T_HOST.GetScriptAdmin().RegisterScriptNotificationHandler(*new DgnScriptTest_DetectJsErrors);

    BeFileName jsFileName;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(jsFileName);
    jsFileName.AppendToPath(L"Script/DgnScriptTest.js");// NOTE: All scripts in the DgnPlatform\Tests\DgnProject\Published\DgnScriptTest directory are combined into one file: Script/DgnScriptTest.js 
    Utf8String jsProgram;
    DgnScriptLibrary::ReadText(jsProgram, jsFileName);
    T_HOST.GetScriptAdmin().EvaluateScript(jsProgram.c_str());

    SetupSeedProject();
    DgnDbP project = m_db.get();
    ASSERT_TRUE(project != NULL);

    PhysicalModelPtr model = GetDefaultPhysicalModel();

    PhysicalModelPtr newModel = DgnDbTestUtils::InsertPhysicalModel(*project, "NewModel");
    ASSERT_TRUE(newModel.IsValid());

    Json::Value parms = Json::objectValue;
    parms["modeledElementIdStr"] = model->GetModeledElementId().ToString().c_str();
    parms["newModeledElementIdStr"] = newModel->GetModeledElementId().ToString().c_str();
    parms["categoryName"] = SpatialCategory::Get(*project, DgnDbTestUtils::GetFirstSpatialCategoryId(*project))->GetCategoryName();
    project->SaveChanges(); // digest other schema imports ??!!
    int retstatus = 0;
    DgnScript::ExecuteDgnDbScript(retstatus, *project, "DgnScriptTests.TestDgnDbScript", parms);
    ASSERT_EQ(0, retstatus);
#ifdef RUN_SCRIPT_PERFORMANCE_TESTS
    DgnScript::ExecuteDgnDbScript(retstatus, *project, "DgnScriptTests.TestDgnDbScriptPerformance", parms);
    ASSERT_EQ(0, retstatus);
#endif
    // undefined function
    DgnScript::ExecuteDgnDbScript(retstatus, *project, "DgnScriptTests.SomeUndefinedFunction", parms);
    ASSERT_NE(0, retstatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnScriptTest, CRUD)
    {
    SetupSeedProject();
    DgnDbP project = m_db.get(); //tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    T_HOST.GetScriptAdmin().RegisterScriptNotificationHandler(*new DgnScriptTest_DetectJsErrors);

    BeFileName jsFileName;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(jsFileName);
    jsFileName.AppendToPath(L"Script/DgnScriptTest.js");
    BeFileName tsFileName;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(tsFileName);
    tsFileName.AppendToPath(L"Script/DgnScriptTest.d.ts");

    Utf8String jsProgram;
    DgnScriptLibrary::ReadText(jsProgram, jsFileName);

    Utf8String tsProgram;
    DgnScriptLibrary::ReadText(tsProgram, tsFileName);

    DgnScriptLibrary scriptLib(*project);
    DateTime scriptLastModifiedTime = DateTime::GetCurrentTimeUtc();
    // Insert JS
    EXPECT_TRUE(DgnDbStatus::Success == scriptLib.RegisterScript("TestJsScript", jsProgram.c_str(), DgnScriptType::JavaScript, scriptLastModifiedTime, false));
    // Insert JS ( Updated existing )
    EXPECT_TRUE(DgnDbStatus::Success == scriptLib.RegisterScript("TestJsScript", jsProgram.c_str(), DgnScriptType::JavaScript, scriptLastModifiedTime, true));
    // Insert TS
    EXPECT_TRUE(DgnDbStatus::Success == scriptLib.RegisterScript("TestTsScript", tsProgram.c_str(), DgnScriptType::TypeScript, scriptLastModifiedTime, false));
    // Insert anonymous
    EXPECT_TRUE(DgnDbStatus::Success == scriptLib.RegisterScript("", tsProgram.c_str(), DgnScriptType::TypeScript, scriptLastModifiedTime, false));

    // Query JS
    Utf8String outText;
    DgnScriptType outType;
    DateTime queryLastModifiedTime;
    EXPECT_TRUE(DgnDbStatus::Success == scriptLib.QueryScript(outText, outType, queryLastModifiedTime, "TestJsScript", DgnScriptType::JavaScript));
    EXPECT_TRUE(jsProgram.Equals(outText));
    EXPECT_TRUE(DgnScriptType::JavaScript == outType);
    // EXPECT_TRUE(areDateTimesEqual(queryLastModifiedTime, scriptLastModifiedTime)); // *** NEEDS WORK - fails in DgnDb06, VC12, Optimized, WinX86. 

    // Query TS with wrong type
    EXPECT_TRUE(DgnDbStatus::Success == scriptLib.QueryScript(outText, outType, queryLastModifiedTime, "TestTsScript", DgnScriptType::JavaScript));
    EXPECT_TRUE(tsProgram.Equals(outText));
    EXPECT_TRUE(DgnScriptType::TypeScript == outType);
    // EXPECT_TRUE(areDateTimesEqual(queryLastModifiedTime, scriptLastModifiedTime));

    // Query Annonyous
    EXPECT_TRUE(DgnDbStatus::Success == scriptLib.QueryScript(outText, outType, queryLastModifiedTime, "", DgnScriptType::TypeScript));
    EXPECT_TRUE(tsProgram.Equals(outText));
    EXPECT_TRUE(DgnScriptType::TypeScript == outType);
    // EXPECT_TRUE(areDateTimesEqual(queryLastModifiedTime, scriptLastModifiedTime)); // *** NEEDS WORK - fails in DgnDb06, VC12, Optimized, WinX86. 

    // Update
    Utf8String updatedScript("<script>Updated One </script>");
    scriptLastModifiedTime = DateTime::GetCurrentTimeUtc();
    EXPECT_TRUE(DgnDbStatus::Success != scriptLib.RegisterScript("TestTsScript", updatedScript.c_str(), DgnScriptType::TypeScript, scriptLastModifiedTime, false));
    EXPECT_TRUE(DgnDbStatus::Success == scriptLib.QueryScript(outText, outType, queryLastModifiedTime, "TestTsScript", DgnScriptType::TypeScript));
    EXPECT_TRUE(tsProgram.Equals(outText));
    // EXPECT_TRUE(!areDateTimesEqual(queryLastModifiedTime, scriptLastModifiedTime)); // *** NEEDS WORK - fails in DgnDb06, VC12, Optimized, WinX86. 

    EXPECT_TRUE(DgnDbStatus::Success == scriptLib.RegisterScript("TestTsScript", updatedScript.c_str(), DgnScriptType::TypeScript, scriptLastModifiedTime, true));
    EXPECT_TRUE(DgnDbStatus::Success == scriptLib.QueryScript(outText, outType, queryLastModifiedTime, "TestTsScript", DgnScriptType::TypeScript));
    EXPECT_TRUE(updatedScript.Equals(outText));
    // EXPECT_TRUE(areDateTimesEqual(queryLastModifiedTime, scriptLastModifiedTime)); // *** NEEDS WORK - fails in DgnDb06, VC12, Optimized, WinX86. 
    }

#endif
