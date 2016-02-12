/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnScriptContext_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef BENTLEYCONFIG_NO_JAVASCRIPT
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnScript.h>
#include <DgnPlatform/GenericDomain.h>
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static DgnCategoryId getFirstCategory(DgnDbR db)
    {
    return *DgnCategory::QueryCategories(db).begin();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static RefCountedCPtr<DgnElement> insertElement(DgnModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId mid = model.GetModelId();

    DgnCategoryId cat = getFirstCategory(db);

    DgnElementPtr gelem;
    if (model.Is3d())
        gelem = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASSNAME_PhysicalObject)), cat, Placement3d()));
    else
        gelem = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationElement2d)), cat, Placement2d()));

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*gelem->ToGeometrySource());
    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1,0,0))));

    if (BSISUCCESS != builder->SetGeomStreamAndPlacement(*gelem->ToGeometrySourceP()))  // We actually catch 2d3d mismatch in SetGeomStreamAndPlacement
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
//=======================================================================================
// @bsiclass                                                    Umar.Hayat     11/2015
//=======================================================================================
struct DgnScriptTest : public ::testing::Test
{
    ScopedDgnHost autoDgnHost;
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnScriptTest, TestEga)
    {
    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite, /*needBriefcase*/false);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    DgnModelPtr model = project->Models().GetModel(project->Models().QueryFirstModelId());
    model->FillModel();

    RefCountedPtr<DgnElement> el;
        {
        RefCountedCPtr<DgnElement> newel = insertElement(*model);
        ASSERT_TRUE( newel.IsValid() );

        checkGeomStream(*newel->ToGeometrySource(), ElementGeometry::GeometryType::CurvePrimitive, 1);
        ASSERT_TRUE( (*(ElementGeometryCollection (*newel->ToGeometrySource()).begin()))->GetAsICurvePrimitive()->GetLineStringCP() != nullptr ) << "Initial geometry should be a line";

        el = newel->CopyForEdit();
        }

    DPoint3d org = DPoint3d::FromZero();
    YawPitchRollAngles angles;
    Json::Value parms (Json::objectValue);

    for (int i=0; i<2; ++i)
        {
        int sres = -1;
        DgnDbStatus xstatus = DgnScript::ExecuteEga(sres, *el, "DgnScriptTest.TestEga", org, angles, parms);
        ASSERT_NE( DgnDbStatus::Success , xstatus ) << "Haven't registered the EGA yet";
        ASSERT_NE( 0 , sres );
        }

    JsProg jsProg;
    jsProg.m_jsProgramName = "DgnScriptTest";
    jsProg.m_jsProgramText =
"(function () { \
    function testEga(element, origin, angles, params) { \
        var boxSize = new Bentley.Dgn.DPoint3d(params.X, params.Y, params.Z); \
        var box = Bentley.Dgn.DgnBox.CreateCenteredBox (new Bentley.Dgn.DPoint3d(0,0,0), boxSize, true); \
        var builder = new Bentley.Dgn.ElementGeometryBuilder(element, origin, angles); \
        builder.Append(box); \
        builder.SetGeomStreamAndPlacement(element); \
        return 0;\
    } \
    function testEgaBadReturn(element, origin, angles, params) { return 'abc'; }\
    Bentley.Dgn.RegisterEGA('DgnScriptTest.TestEga', testEga); \
    Bentley.Dgn.RegisterEGA('DgnScriptTest.TestEgaBadReturn', testEgaBadReturn); \
})();\
";

    autoDgnHost.SetFetchScriptCallback(&jsProg);

    parms["X"] = 1.0;
    parms["Y"] = 2.0;
    parms["Z"] = 3.0;

    StopWatch timeIt(L"", true);
    int niters = 1000;
    for (int i=0; i<niters; ++i)
        {
        int sres;
        DgnDbStatus xstatus = DgnScript::ExecuteEga(sres, *el, "DgnScriptTest.TestEga", org, angles, parms);
        ASSERT_EQ( DgnDbStatus::Success , xstatus );
        ASSERT_EQ( 0 , sres );

        checkGeomStream(*el->ToGeometrySource(), ElementGeometry::GeometryType::SolidPrimitive, 1);
        DgnBoxDetail box;
        ASSERT_TRUE( (*(ElementGeometryCollection (*el->ToGeometrySource()).begin()))->GetAsISolidPrimitive()->TryGetDgnBoxDetail(box) ) << "Geometry should be a slab";
        ASSERT_EQ( box.m_baseX , parms["X"].asDouble() );
        ASSERT_EQ( box.m_baseY , parms["Y"].asDouble() );
        ASSERT_EQ( box.m_topOrigin.Distance(box.m_baseOrigin) , parms["Z"].asDouble() );

        parms["Z"] = parms["Z"].asDouble() + 1;
        }
    timeIt.Stop();
    BeTest::Log("DgnScriptTest", BeTest::LogPriority::PRIORITY_ERROR, Utf8PrintfString("%d / %lf seconds = %lf/second\n", niters, timeIt.GetElapsedSeconds(), niters/timeIt.GetElapsedSeconds()));

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
TEST_F(DgnScriptTest, RunScripts)
    {
    T_HOST.GetScriptAdmin().RegisterScriptNotificationHandler(*new DetectJsErrors);

    BeFileName jsFileName;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(jsFileName);
    jsFileName.AppendToPath(L"Script/DgnScriptTest.js");
    Utf8String jsProgram;
    DgnScriptLibrary::ReadText(jsProgram, jsFileName);
    T_HOST.GetScriptAdmin().EvaluateScript(jsProgram.c_str());


    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite, /*needBriefcase*/false);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);
    DgnModelPtr model = project->Models().GetModel(project->Models().QueryFirstModelId());
    model->FillModel();
    Json::Value parms = Json::objectValue;
    parms["modelName"] = model->GetCode().GetValueCP();
    parms["categoryName"] = DgnCategory::QueryCategory(getFirstCategory(*project), *project)->GetCategoryName();
    int retstatus = 0;
    DgnScript::ExecuteDgnDbScript(retstatus, *project, "DgnScriptTests.TestDgnDbScript", parms);
    ASSERT_EQ(0, retstatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef COMMENT_OFF_NOT_USED
static bool areDateTimesEqual(DateTime const& d1, DateTime const& d2)
    {
    // TRICKY: avoid problems with rounding.
    double jd1, jd2;
    d1.ToJulianDay(jd1);
    d2.ToJulianDay(jd2);
    return jd1 == jd2;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnScriptTest, CRUD)
    {

    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite, /*needBriefcase*/false);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    T_HOST.GetScriptAdmin().RegisterScriptNotificationHandler(*new DetectJsErrors);

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
