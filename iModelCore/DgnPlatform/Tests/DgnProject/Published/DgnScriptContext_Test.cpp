/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnScriptContext_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef BENTLEYCONFIG_NO_JAVASCRIPT
#include "DgnHandlersTests.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"
#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnScript.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/ScriptDomain.h>
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

//#define RUN_SCRIPT_PERFORMANCE_TESTS

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
        gelem = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AnnotationElement2d)), cat, Placement2d()));

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
    SetupSeedProject();
    DgnDbP project = m_db.get();// tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    DgnModelPtr model = project->Models().GetModel(project->Models().QueryFirstModelId());
    model->FillModel();

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
        var builder = Bentley.Dgn.GeometryBuilder.CreateFor3dModel(element.Model, element.CategoryId, origin, angles); \
        builder.AppendGeometry(box); \
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
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus createSpatialModel(SpatialModelPtr& catalogModel, DgnDbR db, DgnCode const& code)
    {
    DgnClassId mclassId = DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SpatialModel));
    catalogModel = new SpatialModel(SpatialModel::CreateParams(db, mclassId, code));
    catalogModel->SetInGuiList(false);
    return catalogModel->Insert();
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

    DgnModelPtr model = project->Models().GetModel(project->Models().QueryFirstModelId());
    model->FillModel();
    SpatialModelPtr newmodel;
    ASSERT_EQ(DgnDbStatus::Success, createSpatialModel(newmodel, *project, DgnModel::CreateModelCode("DgnScriptTest.RunScripts")));
    Json::Value parms = Json::objectValue;
    parms["modelName"] = model->GetCode().GetValueCP();
    parms["newModelName"] = newmodel->GetCode().GetValueCP();
    parms["categoryName"] = DgnCategory::QueryCategory(getFirstCategory(*project), *project)->GetCategoryName();
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnScriptTest, ScriptElementCRUD)
    {
    SetupSeedProject();
    DgnDbStatus dstatus = ScriptDomain::ImportSchema(*m_db);
    ASSERT_EQ(DgnDbStatus::Success, dstatus);
    m_db->SaveChanges();

    DgnDbStatus status;

    //  Create a ScriptLibraryModel to hold the scripts
    auto scriptLib = ScriptLibraryModel::Create(*m_db, DgnModel::CreateModelCode("scripts"));
    ASSERT_TRUE(scriptLib.IsValid());
    status = scriptLib->Insert();
    ASSERT_EQ(DgnDbStatus::Success, status);

    if (true)
        {
        //  Create a valid script definition
        ScriptDefinitionElementPtr scriptEl = ScriptDefinitionElement::Create(&status, *scriptLib, SCRIPT_DOMAIN_CLASSNAME_FilterElement,
            "function foo(element) {return true;}", "foo", "a script");
        ASSERT_TRUE(scriptEl.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, status);
        ASSERT_STREQ("a script", scriptEl->GetDescription().c_str());
        ASSERT_STREQ("function foo(element) {return true;}", scriptEl->GetText().c_str());
        ASSERT_STREQ("foo", scriptEl->GetEntryPoint().c_str());
        ASSERT_STREQ("ES5", scriptEl->GetEcmaScriptVersionRequired().c_str());
        ASSERT_STREQ("", scriptEl->GetSourceUrl().c_str());
        Utf8String rt, args;
        scriptEl->GetSignature(rt, args);
        ASSERT_STREQ("boolean", rt.c_str());
        ASSERT_STREQ("DgnElement", args.c_str());

        //  Create a invalid script definition (incorrect entry point)
        ScriptDefinitionElementPtr scriptElBad = ScriptDefinitionElement::Create(&status, *scriptLib, SCRIPT_DOMAIN_CLASSNAME_FilterElement,
            "function foo(element) {return true;}", "missing_entry_point", "a script");
        ASSERT_FALSE(scriptElBad.IsValid());
        ASSERT_NE(DgnDbStatus::Success, status);

        //  Persist the valid script definition
        auto persistentScriptEl = m_db->Elements().Insert(*scriptEl, &status);
        ASSERT_TRUE(persistentScriptEl.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, status);
        
        //  Execute the valid script definition
        Utf8String retVal;
        status = persistentScriptEl->Execute(retVal, { persistentScriptEl.get() });
        ASSERT_EQ(DgnDbStatus::Success, status);
        ASSERT_STREQ("true", retVal.c_str());
        }

    if (true)
        {
        // Try executing various kinds of scripts and checking their return values
        ScriptDefinitionElementCPtr scriptEl1;
        
        scriptEl1 = m_db->Elements().Insert(*ScriptDefinitionElement::Create(nullptr, *scriptLib, SCRIPT_DOMAIN_CLASSNAME_FilterElement,
            "function ifElementIsNotNull(element) {return element != null;}"));
        Utf8String retVal;
        ASSERT_EQ(DgnDbStatus::Success, scriptEl1->Execute(retVal, {scriptEl1.get()}));
        ASSERT_STREQ("true", retVal.c_str());
        ASSERT_EQ(DgnDbStatus::Success, scriptEl1->Execute(retVal, {nullptr}));
        ASSERT_STREQ("false", retVal.c_str());

#ifdef WIP_EXCEPTIONS
        scriptEl1 = m_db->Elements().Insert(*ScriptDefinitionElement::Create(nullptr, *scriptLib, SCRIPT_DOMAIN_CLASSNAME_FilterElement,
            "function throwsException(element) {return element.Id != null;}", "throwsException", ""));
        ASSERT_STREQ("false", scriptEl1->Execute(&status, nullptr).c_str());
        ASSERT_NE(DgnDbStatus::Success, status);
#endif

        scriptEl1 = m_db->Elements().Insert(*ScriptDefinitionElement::Create(nullptr, *scriptLib, SCRIPT_DOMAIN_CLASSNAME_PopulateElementList,
"function fillEleList(dgnObjectIdSet,db,viewport) {\
    var be = Bentley.Dgn;\
    var categories = be.DgnCategory.QueryCategories(db);\
    for (var catiter = categories.Begin(); categories.IsValid(catiter); categories.ToNext(catiter))\
        {\
        dgnObjectIdSet.Insert(categories.GetId(catiter));\
        }\
    }"
        ));
        DgnElementIdSet ids;
        ASSERT_EQ(DgnDbStatus::Success, scriptEl1->Execute(retVal, {&ids, m_db.get(), nullptr}));
        //ASSERT_STREQ("", retVal.c_str());
        ASSERT_NE(0, ids.size());
        auto categories = DgnCategory::QueryCategories(*m_db);
        ASSERT_EQ(categories.size(), ids.size());
        ASSERT_TRUE(categories.begin()->GetValue() == ids.begin()->GetValue());
        }
    }

#endif
