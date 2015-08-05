/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnScriptContext_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef BENTLEYCONFIG_NO_JAVASCRIPT
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnCore/DgnScriptContext.h>
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static RefCountedCPtr<GeometricElement> insertElement(DgnModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnModelId mid = model.GetModelId();

    DgnCategoryId cat = db.Categories().QueryHighestId();

    GeometricElementPtr gelem;
    if (model.Is3d())
        gelem = PhysicalElement::Create(PhysicalElement::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "PhysicalElement")), cat, Placement3d()));
    else
        gelem = DrawingElement::Create(DrawingElement::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "DrawingElement")), cat, Placement2d()));

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*gelem);
    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1,0,0))));

    if (BSISUCCESS != builder->SetGeomStreamAndPlacement(*gelem))  // We actually catch 2d3d mismatch in SetGeomStreamAndPlacement
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
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnScriptContextTest, Test1)
    {
    ScopedDgnHost  autoDgnHost;

    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    DgnModelPtr model = project->Models().GetModel(project->Models().QueryFirstModelId());
    model->FillModel();

    RefCountedPtr<GeometricElement> el;
        {
        RefCountedCPtr<GeometricElement> newel = insertElement(*model);
        ASSERT_TRUE( newel.IsValid() );

        checkGeomStream(*newel->ToGeometricElement(), ElementGeometry::GeometryType::CurvePrimitive, 1);
        ASSERT_TRUE( (*(ElementGeometryCollection (*newel).begin()))->GetAsICurvePrimitive()->GetLineStringCP() != nullptr ) << "Initial geometry should be a line";

        el = newel->CopyForEdit()->ToGeometricElementP();
        }

    DPoint3d org = DPoint3d::FromZero();
    YawPitchRollAngles angles;
    Json::Value parms (Json::objectValue);

    DgnPlatformLib::Host& host = T_HOST;
    DgnPlatformLib::Host::ScriptingAdmin& admin = host.GetScriptingAdmin();
    DgnScriptContextR context = admin.GetDgnScriptContext();

    for (int i=0; i<2; ++i)
        {
        int sres = -1;
        DgnDbStatus xstatus = context.ExecuteEga(sres, *el, "DgnScriptContextTest.TestEga", org, angles, parms);
        ASSERT_NE( DgnDbStatus::Success , xstatus ) << "Haven't registered the EGA yet";
        ASSERT_NE( 0 , sres );
        }

    JsProg jsProg;
    jsProg.m_jsProgramName = "DgnScriptContextTest";
    jsProg.m_jsProgramText =
"(function () { \
    function testEga(element, origin, angles, params) { \
        var builder = BentleyApi.Dgn.JsElementGeometryBuilder.Create(element, origin, angles); \
        builder.AppendBox(params[\"X\"], params[\"Y\"], params[\"Z\"]); \
        builder.SetGeomStreamAndPlacement(element); \
        return 0;\
    } \
    function testEgaBadReturn(element, origin, angles, params) { return 'abc'; }\
    BentleyApi.Dgn.RegisterEGA('DgnScriptContextTest.TestEga', testEga); \
    BentleyApi.Dgn.RegisterEGA('DgnScriptContextTest.TestEgaBadReturn', testEgaBadReturn); \
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
        DgnDbStatus xstatus = context.ExecuteEga(sres, *el, "DgnScriptContextTest.TestEga", org, angles, parms);
        ASSERT_EQ( DgnDbStatus::Success , xstatus );
        ASSERT_EQ( 0 , sres );

        checkGeomStream(*el->ToGeometricElement(), ElementGeometry::GeometryType::SolidPrimitive, 1);
        DgnBoxDetail box;
        ASSERT_TRUE( (*(ElementGeometryCollection (*el).begin()))->GetAsISolidPrimitive()->TryGetDgnBoxDetail(box) ) << "Geometry should be a slab";
        ASSERT_EQ( box.m_baseX , parms["X"].asDouble() );
        ASSERT_EQ( box.m_baseY , parms["Y"].asDouble() );
        ASSERT_EQ( box.m_topOrigin.Distance(box.m_baseOrigin) , parms["Z"].asDouble() );

        parms["Z"] = parms["Z"].asDouble() + 1;
        }
    timeIt.Stop();
    BeTest::Log("DgnScriptContextTest", BeTest::LogPriority::PRIORITY_ERROR, Utf8PrintfString("%d / %lf seconds = %lf/second\n", niters, timeIt.GetElapsedSeconds(), niters/timeIt.GetElapsedSeconds()));

    // Check that attempting to call a non-registered function fails with a non-zero xstatus
    if (true)
        {
        int sres = -1;
        BeTest::SetFailOnAssert(false);
        DgnDbStatus xstatus = context.ExecuteEga(sres, *el, "DgnScriptContextTest.TestEgaNotRegistered", org, angles, parms);
        BeTest::SetFailOnAssert(true);
        ASSERT_EQ( DgnDbStatus::NotEnabled , xstatus ) << "this function is not registered so the attempt should fail";
        ASSERT_EQ( -1 , sres );
        }

    // Check that attempting to call a registered function that returns anything other than a integer fails with a non-zero xstatus
    if (true)
        {
        int sres = -1;
        BeTest::SetFailOnAssert(false);
        DgnDbStatus xstatus = context.ExecuteEga(sres, *el, "DgnScriptContextTest.TestEgaBadReturn", org, angles, parms);
        BeTest::SetFailOnAssert(true);
        ASSERT_EQ( DgnDbStatus::NotEnabled , xstatus ) << "this function should not be callable so the attempt should fail";
        ASSERT_EQ( -1 , sres );
        }
    }
#endif
