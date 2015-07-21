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

/*---------------------------------------------------------------------------------**//**
* Make a copy of the specified input file, giving it a new name, and then open it 
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void openCopyOfDb (DgnDbPtr& db, WCharCP inputFileName, WCharCP outputFileName, BeSQLite::Db::OpenMode mode)
    {
    BeFileName fullInputFileName;
    BeTest::GetHost().GetDocumentsRoot (fullInputFileName);
    fullInputFileName.AppendToPath (inputFileName);

    BeFileName fullOutputFileName;
    BeTest::GetHost().GetOutputRoot(fullOutputFileName);
    fullOutputFileName.AppendToPath(outputFileName);

    ASSERT_EQ ( BeFileNameStatus::Success , BeFileName::BeCopyFile (fullInputFileName, fullOutputFileName) );
    
    DbResult result;
    db = DgnDb::OpenDgnDb(&result, fullOutputFileName, DgnDb::OpenParams(mode));
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

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson     02/2012
+===============+===============+===============+===============+===============+======*/
struct CMTestFixture : public testing::Test
{
protected:
DgnDbPtr           m_db;
Dgn::ScopedDgnHost m_host;
FakeScriptLibrary          m_jsLibrary;

CMTestFixture();
void AddToScriptLibrary(Utf8CP jns, Utf8CP jtext);
DgnCategoryId CreateCategory(Utf8CP code, ColorDef const&);
void CreateCm();
};

#define TEST_JS_NAMESPACE   "Acme"
#define TEST_JS_SOLVER_NAME "WidgetSolver"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
CMTestFixture::CMTestFixture()
    {
    m_host.SetFetchScriptCallback(&m_jsLibrary);// In this test, we redirect all requests for JS programs to our fake library
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void CMTestFixture::AddToScriptLibrary(Utf8CP jns, Utf8CP jtext)
    {
    // In this test, there is only one JS program in the fake library at a time.
    m_jsLibrary.m_jsProgramName = jns;
    m_jsLibrary.m_jsProgramText = jtext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId CMTestFixture::CreateCategory(Utf8CP code, ColorDef const& color)
    {
    DgnCategories::Category category(code, DgnCategories::Scope::Any);
    DgnCategories::SubCategory::Appearance appearance;
    appearance.SetColor(color);
    m_db->Categories().Insert(category, appearance);
    return category.GetCategoryId();
    }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CMTestFixture::CreateCm()
    {
    // For the purposes of this test, we'll put the CM in its own DgnDb
    openCopyOfDb (m_db, L"DgnDb/3dMetricGeneral.idgndb", L"DgnComponentModelTest_CM.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    m_db->Txns().EnableTracking(true);

    // Define the Solver parameters for use by this model.
    Json::Value parameters(Json::objectValue);
    parameters["X"] = 1;
    parameters["Y"] = 1;
    parameters["Z"] = 1;
    parameters["Other"] = "Something else";
    DgnModel::Solver solver(DgnModel::Solver::Type::Script, TEST_JS_NAMESPACE "." TEST_JS_SOLVER_NAME, parameters); // Identify the JS solver that should be used. Note: this JS program must be in the script library

    // Create the model
    DgnClassId mclassId = DgnClassId(m_db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ComponentModel));
    RefCountedPtr<ComponentModel> cm = new ComponentModel(PhysicalModel::CreateParams(*m_db, mclassId, "Widget", DgnModel::Properties(), solver));
    ASSERT_EQ( DgnDbStatus::Success , cm->Insert() );       /* Insert the new model into the DgnDb */

    // Here is the model solver that should be used. 
    // Note that we must put it into the Script library under the same name that was used in the model definition above.
    // It must also register itself as a model solver under the same name as was used in the model definition above
    // Note that a script will generate create elements from scratch. That's why it starts by deleting all elements in the model. They would have been the outputs of the last run.
    AddToScriptLibrary(TEST_JS_NAMESPACE, 
"(function () { \
    function testSolver(model, params) { \
        model.DeleteAllElements();\
        var element = model.CreateElement(\"dgn.PhysicalElement\", \"Widget\");\
        var origin = BentleyApi.Dgn.JsDPoint3d.Create(0,0,0);\
        var angles = BentleyApi.Dgn.JsYawPitchRollAngles.Create(0,0,0);\
        var builder = BentleyApi.Dgn.JsElementGeometryBuilder.Create(element, origin, angles); \
        builder.AppendBox(params[\"X\"], params[\"Y\"], params[\"Z\"]); \
        builder.SetGeomStreamAndPlacement(element); \
        model.InsertElement(element); \
        return 0;\
    } \
    BentleyApi.Dgn.RegisterModelSolver('" TEST_JS_NAMESPACE "." TEST_JS_SOLVER_NAME "', testSolver); \
})();\
");
    ASSERT_TRUE( cm.IsValid() );

    // The CM's Element Category
    DgnCategoryId widgetCategory = CreateCategory("Widget", ColorDef(0xff,0x00,0x00));
    cm->GetSolverParametersR()["Category"] = "Widget";  // Save the Element Category name as a "parameter" of cm. 
    cm->Update();

    m_db->SaveChanges(); // should trigger validation

    //  Check the initial solution
    if (true)
        {
        RefCountedCPtr<DgnElement> el = cm->begin()->second;
        checkGeomStream(*el->ToGeometricElement(), ElementGeometry::GeometryType::SolidPrimitive, 1);
        checkSlabDimensions(*el->ToGeometricElement(), 1, 1, 1);
        }

    // Test solving with a different set of parameters

    Json::Value& parms = cm->GetSolverParametersR();
    parms["X"] = 2.0;
    parms["Y"] = 3.0;
    parms["Z"] = 4.0;
    cm->Update(); // Don't forget to call Update and then ...
    m_db->SaveChanges(); // ... SaveChanges. That's how the txn manager finds out about the change to the model's parameters and then triggers its validation callback.
    if (true)
        {
        RefCountedCPtr<DgnElement> el = cm->begin()->second;
        checkGeomStream(*el->ToGeometricElement(), ElementGeometry::GeometryType::SolidPrimitive, 1);
        checkSlabDimensions(*el->ToGeometricElement(), 2, 3, 4);
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CMTestFixture, Test1)
    {
    CreateCm();
    }
