/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementProperties.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECSqlBuilder.h>
#include <DgnPlatform/DgnCore/WebMercator.h>

#define TMTEST_SCHEMA_NAME                               "DgnPlatformTest"
#define TMTEST_SCHEMA_NAMEW                             L"DgnPlatformTest"
#define TMTEST_TEST_ELEMENT_CLASS_NAME                   "TestElement"
#define TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME    "TestElementDrivesElement"

USING_NAMESPACE_BENTLEY_SQLITE
struct TestElementHandler;

//=======================================================================================
//! A test Element
//@bsiclass                                                     Sam.Wilson      04/15
//=======================================================================================
struct TestElement : Dgn::PhysicalElement
    {
    DEFINE_T_SUPER (Dgn::PhysicalElement)

    private:
        friend struct TestElementHandler;

        TestElement (CreateParams const& params) : T_Super (params)
            {}
    };
static CurveVectorPtr computeShape ()
    {
    static const double PLANE_LEN = 100;

    DPoint3d pts[6];
    pts[0] = DPoint3d::From (-PLANE_LEN, -PLANE_LEN);
    pts[1] = DPoint3d::From (+PLANE_LEN, -PLANE_LEN);
    pts[2] = DPoint3d::From (+PLANE_LEN, +PLANE_LEN);
    pts[3] = DPoint3d::From (-PLANE_LEN, +PLANE_LEN);
    pts[4] = pts[0];
    pts[5] = pts[0];
    pts[5].z = 1;

    return CurveVector::CreateLinear (pts, _countof (pts), CurveVector::BOUNDARY_TYPE_Open);
    }
//=======================================================================================
//! A test ElementHandler
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct TestElementHandler : Dgn::ElementHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS ("TestElement", TestElement, TestElementHandler, Dgn::ElementHandler, )

    ECN::ECClassCP GetTestElementECClass (DgnDbR db)
        {
        return db.Schemas ().GetECClass (TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_CLASS_NAME);
        }

    DgnElementKey InsertElement (DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode, ElemDisplayParamsCR ep)
        {
        DgnModelP model = db.Models ().GetModel (mid);
        DgnElementPtr testElement = TestElementHandler::Create (TestElement::CreateParams (*model, DgnClassId (GetTestElementECClass (db)->GetId ()), categoryId, Placement3d (), elementCode));
        GeometricElementP geomElem = const_cast<GeometricElementP>(testElement->ToGeometricElement ());
        ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld (*geomElem);

        EXPECT_TRUE (builder->Append (ep));
        EXPECT_TRUE (builder->Append (*computeShape ()));

        if (SUCCESS != builder->SetGeomStreamAndPlacement (*geomElem))
            return DgnElementKey ();

        return db.Elements ().Insert (*testElement)->GetElementKey ();
        }

    };

HANDLER_DEFINE_MEMBERS (TestElementHandler)

//=======================================================================================
//! A test Domain
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct ElementPropertiesDomain : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS (ElementPropertiesDomain, )
    public:
        ElementPropertiesDomain ();
    };

DOMAIN_DEFINE_MEMBERS (ElementPropertiesDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementPropertiesDomain::ElementPropertiesDomain () : DgnDomain (TMTEST_SCHEMA_NAME, "DgnProject Test Schema", 1)
    {
    RegisterHandler (TestElementHandler::GetHandler ());
    }

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Element Properties
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementDisplayProperties : public ::testing::Test
    {
    public:
        ScopedDgnHost m_host;
        DgnDbPtr      m_db;
        DgnModelId    m_defaultModelId;
        DgnCategoryId m_defaultCategoryId;

        ElementDisplayProperties ()
            {
            // Must register my domain whenever I initialize a host
            DgnDomains::RegisterDomain (ElementPropertiesDomain::GetDomain ());
            }

        ~ElementDisplayProperties ()
            {}

        void CloseDb ()
            {
            FinalizeStatements ();
            m_db->CloseDb ();
            }

        DgnModelR GetDefaultModel ()
            {
            return *m_db->Models ().GetModel (m_defaultModelId);
            }

        virtual void FinalizeStatements ()
            {}

        /*---------------------------------------------------------------------------------**//**
        * set up method that opens an existing .dgndb project file after copying it to out
        * @bsimethod                                                    Sam.Wilson      01/15
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetupProject (WCharCP projFile, WCharCP testFile, BeSQLite::Db::OpenMode mode)
            {
            BeFileName outFileName;
            ASSERT_EQ (SUCCESS, DgnDbTestDgnManager::GetTestDataOut (outFileName, projFile, testFile, __FILE__));
            DbResult result;
            m_db = DgnDb::OpenDgnDb (&result, outFileName, DgnDb::OpenParams (mode));
            ASSERT_TRUE (m_db.IsValid ());
            ASSERT_TRUE (result == BE_SQLITE_OK);

            BeFileName schemaFile (T_HOST.GetIKnownLocationsAdmin ().GetDgnPlatformAssetsDirectory ());
            schemaFile.AppendToPath (L"ECSchemas/" TMTEST_SCHEMA_NAMEW L".01.00.ecschema.xml");

            BentleyStatus status = ElementPropertiesDomain::GetDomain ().ImportSchema (*m_db, schemaFile);
            ASSERT_TRUE (BentleyStatus::SUCCESS == status);

            auto schema = m_db->Schemas ().GetECSchema (TMTEST_SCHEMA_NAME, true);
            ASSERT_NE (nullptr, schema);
            ASSERT_NE (nullptr, TestElementHandler::GetHandler ().GetTestElementECClass (*m_db));
            ASSERT_NE (nullptr, m_db->Schemas ().GetECClass (TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME));

            m_defaultModelId = m_db->Models ().QueryFirstModelId ();
            DgnModelP defaultModel = m_db->Models ().GetModel (m_defaultModelId);
            ASSERT_NE (nullptr, defaultModel);
            GetDefaultModel ().FillModel ();

            m_defaultCategoryId = m_db->Categories ().MakeIterator ().begin ().GetCategoryId ();
            }

        ///*---------------------------------------------------------------------------------**//**
        // * @bsimethod                                    Sam.Wilson      01/15
        // +---------------+---------------+---------------+---------------+---------------+------*/
        DgnElementKey InsertElement (Utf8CP elementCode, ElemDisplayParamsCR ep, DgnModelId mid = DgnModelId (), DgnCategoryId categoryId = DgnCategoryId ())
            {
            if (!mid.IsValid ())
                mid = m_defaultModelId;

            if (!categoryId.IsValid ())
                categoryId = m_defaultCategoryId;

            return TestElementHandler::GetHandler ().InsertElement (*m_db, mid, categoryId, elementCode, ep);
            }
    };

/*---------------------------------------------------------------------------------**//**
* Test for Setting and Getting Gradient properties.
* @bsimethod                                    Maha.Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementDisplayProperties, SetGradient)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"ElementDisplayProperties.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    auto seedModelId3 = m_defaultModelId;
    DgnModelP model3 = m_db->Models ().CreateNewModelFromSeed (NULL, "model3", seedModelId3);
    ASSERT_TRUE (model3 != nullptr);
    DgnModelId m3id = m_db->Models ().QueryModelId ("model3");

    ElemDisplayParams ep;
    ep.SetCategoryId (m_defaultCategoryId);
    ep.SetFillDisplay (FillDisplay::Always);

    GradientSymbPtr   gradient = GradientSymb::Create ();
    double   keyValues[2];
    ColorDef    keyColors[2];

    keyValues[0] = 0.0;
    keyValues[1] = 0.5;
    keyColors[0] = ColorDef::Yellow ();
    keyColors[1] = ColorDef::Red ();

    gradient->SetMode (GradientMode::Spherical);
    gradient->SetFlags (0);
    gradient->SetAngle (8.0);
    gradient->SetTint (1.0);
    gradient->SetShift (1.0);
    gradient->SetKeys (2, keyColors, keyValues);
    ep.SetGradient (gradient.get ());

    auto keyE1 = InsertElement ("E3", ep, m3id);
    DgnElementId E1id = keyE1.GetElementId ();
    DgnElementCP pE1 = m_db->Elements ().FindElement (E1id);

    GeometricElementP geomElem = const_cast<GeometricElementP>(pE1->ToGeometricElement ());
    ElementGeometryCollection collection (*geomElem);

    for (ElementGeometryPtr geom : collection)
        {
        ElemDisplayParamsCR params = collection.GetElemDisplayParams ();
        GradientSymbCP gradient = params.GetGradient ();
        EXPECT_NE (nullptr, params.GetGradient ());
        EXPECT_EQ (GradientMode::Spherical, gradient->GetMode ());
        EXPECT_EQ (0, gradient->GetFlags ());
        EXPECT_EQ (8.0, gradient->GetAngle ());
        EXPECT_EQ (1.0, gradient->GetTint ());
        EXPECT_EQ (1.0, gradient->GetShift ());
        EXPECT_EQ (2, gradient->GetNKeys ());
        }
    }

/*---------------------------------------------------------------------------------**//**
* Test for Setting and Getting Pattern parameters.
* @bsimethod                                    Maha.Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
//Will uncomment this test once SetPatternParams method is hooked up with elemDisplayParams.
//TEST_F (ElementDisplayProperties, SetDisplayPattern)
//    {
//    SetupProject (L"3dMetricGeneral.idgndb", L"ElementDisplayProperties.idgndb", BeSQLite::Db::OPEN_ReadWrite);
//
//    auto seedModelId3 = m_defaultModelId;
//    DgnModelP model3 = m_db->Models ().CreateNewModelFromSeed (NULL, "model3", seedModelId3);
//    ASSERT_TRUE (model3 != nullptr);
//    DgnModelId m3id = m_db->Models ().QueryModelId ("model3");
//
//    ElemDisplayParams ep;
//    ep.SetCategoryId (m_defaultCategoryId);
//
//    PatternParamsPtr pattern = PatternParams::Create ();
//    pattern->SetColor (ColorDef::Maroon ());
//    pattern->SetWeight (6);
//    pattern->SetStyle (1);
//    //ep.SetPatternParams (pattern.get());
//
//    auto keyE1 = InsertElement ("E2", ep, m3id);
//    DgnElementId E1id = keyE1.GetElementId ();
//    DgnElementCP pE1 = m_db->Elements ().FindElement (E1id);
//
//    GeometricElementP geomElem = const_cast<GeometricElementP>(pE1->ToGeometricElement ());
//    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld (*geomElem);
//    ElementGeometryCollection collection (*geomElem);
//
//    for (ElementGeometryPtr geom : collection)
//        {
//        ElemDisplayParamsCR params = collection.GetElemDisplayParams ();
//        PatternParamsCP pattern = params.GetPatternParams ();
//        EXPECT_NE (nullptr, params.GetPatternParams ());
//        EXPECT_EQ (ColorDef::Maroon (), pattern->GetColor ());
//        EXPECT_EQ (6, pattern->GetWeight ());
//        EXPECT_EQ (1, pattern->GetStyle ());
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* Test for Setting and Getting Transparency.
* @bsimethod                                    Maha.Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementDisplayProperties, SetTransparency)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"ElementDisplayProperties.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    auto seedModelId3 = m_defaultModelId;
    DgnModelP model3 = m_db->Models ().CreateNewModelFromSeed (NULL, "model3", seedModelId3);
    ASSERT_TRUE (model3 != nullptr);
    DgnModelId m3id = m_db->Models ().QueryModelId ("model3");

    ElemDisplayParams ep;
    ep.SetCategoryId (m_defaultCategoryId);
    ep.SetTransparency (0.5);

    auto keyE1 = InsertElement ("E2", ep, m3id);
    DgnElementId E1id = keyE1.GetElementId ();
    DgnElementCP pE1 = m_db->Elements ().FindElement (E1id);

    GeometricElementP geomElem = const_cast<GeometricElementP>(pE1->ToGeometricElement ());
    ElementGeometryCollection collection (*geomElem);

    for (ElementGeometryPtr geom : collection)
        {
        ElemDisplayParamsCR params = collection.GetElemDisplayParams ();
        EXPECT_EQ (0.5, params.GetTransparency ());
        EXPECT_EQ (0.5, params.GetNetTransparency ());
        }
    }

/*---------------------------------------------------------------------------------**//**
* Test for Setting and Getting Cateogory and SubCategory Id.
* @bsimethod                                    Maha.Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementDisplayProperties, SetCategory)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"ElementDisplayProperties.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    auto seedModelId3 = m_defaultModelId;
    DgnModelP model3 = m_db->Models ().CreateNewModelFromSeed (NULL, "model3", seedModelId3);
    ASSERT_TRUE (model3 != nullptr);
    DgnModelId m3id = m_db->Models ().QueryModelId ("model3");

    ElemDisplayParams ep;
    ep.SetCategoryId (m_defaultCategoryId);

    auto keyE1 = InsertElement ("E2", ep, m3id);
    DgnElementId E1id = keyE1.GetElementId ();
    DgnElementCP pE1 = m_db->Elements ().FindElement (E1id);

    GeometricElementP geomElem = const_cast<GeometricElementP>(pE1->ToGeometricElement ());
    ElementGeometryCollection collection (*geomElem);

    for (ElementGeometryPtr geom : collection)
        {
        ElemDisplayParamsCR params = collection.GetElemDisplayParams ();
        DgnCategoryId CId = params.GetCategoryId ();
        ASSERT_TRUE (CId.IsValid ());
        //Setting the Category Id also sets the SubCategory to the default.
        DgnSubCategoryId SId = params.GetSubCategoryId ();
        ASSERT_TRUE (SId.IsValid ());
        }
    }

/*---------------------------------------------------------------------------------**//**
* Test for Setting and Getting Display Parameters.
* @bsimethod                                    Maha Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementDisplayProperties, SetDisplayParams)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"ElementDisplayProperties.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    auto seedModelId3 = m_defaultModelId;
    DgnModelP model3 = m_db->Models ().CreateNewModelFromSeed (NULL, "model3", seedModelId3);
    ASSERT_TRUE (model3 != nullptr);
    DgnModelId m3id = m_db->Models ().QueryModelId ("model3");

    ElemDisplayParams ep;
    ep.SetCategoryId (m_defaultCategoryId);
    ep.SetWeight (21);
    ep.SetMaterial (NULL);
    ep.SetDisplayPriority (2);

    auto keyE1 = InsertElement ("E2", ep, m3id);
    DgnElementId E1id = keyE1.GetElementId ();
    DgnElementCP pE1 = m_db->Elements ().FindElement (E1id);

    GeometricElementP geomElem = const_cast<GeometricElementP>(pE1->ToGeometricElement ());
    ElementGeometryCollection collection (*geomElem);

    for (ElementGeometryPtr geom : collection)
        {
        ElemDisplayParamsCR params = collection.GetElemDisplayParams ();
        EXPECT_EQ (21, params.GetWeight ());
        bool weight = params.IsWeightFromSubCategoryAppearance ();
        EXPECT_FALSE (weight);

#if defined (NEEDS_WORK_MATERIAL)
        EXPECT_EQ (NULL, params.GetMaterial ());
#endif

        EXPECT_EQ (2, params.GetDisplayPriority ());
        }
    }

/*---------------------------------------------------------------------------------**//**
* Test for Setting Fill properties.
* @bsimethod                                    Maha.Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementDisplayProperties, FillProperties)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"ElementDisplayProperties.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    auto seedModelId3 = m_defaultModelId;
    DgnModelP model3 = m_db->Models ().CreateNewModelFromSeed (NULL, "model3", seedModelId3);
    ASSERT_TRUE (model3 != nullptr);
    DgnModelId m3id = m_db->Models ().QueryModelId ("model3");

    ElemDisplayParams ep;
    ep.SetCategoryId (m_defaultCategoryId);
    ep.SetFillDisplay (FillDisplay::Always);
    ep.SetFillColor (ColorDef::Red ());
    ep.SetFillTransparency (0.8);

    auto keyE1 = InsertElement ("E2", ep, m3id);
    DgnElementId E1id = keyE1.GetElementId ();
    DgnElementCP pE1 = m_db->Elements ().FindElement (E1id);

    GeometricElementP geomElem = const_cast<GeometricElementP>(pE1->ToGeometricElement ());
    ElementGeometryCollection collection (*geomElem);

    for (ElementGeometryPtr geom : collection)
        {
        ElemDisplayParamsCR params = collection.GetElemDisplayParams ();
        EXPECT_EQ (FillDisplay::Always, params.GetFillDisplay ());
        EXPECT_EQ (ColorDef::Red (), params.GetFillColor ());
        bool FillColor = params.IsFillColorFromSubCategoryAppearance ();
        EXPECT_FALSE (FillColor);
        EXPECT_EQ (0.8, params.GetFillTransparency ());
        EXPECT_EQ (0.8, params.GetNetFillTransparency ());
        }
    }