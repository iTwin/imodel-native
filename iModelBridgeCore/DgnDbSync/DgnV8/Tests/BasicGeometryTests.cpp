/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/BasicGeometryTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include <VersionedDgnV8Api/DgnPlatform/LevelTypes.h>
#include "GeomTestHelper.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      02/16
//----------------------------------------------------------------------------------------
struct BasicGeometryTests : public GeomTestFixture
{
    DEFINE_T_SUPER(GeomTestFixture);
    DgnV8Api::MultilineStylePtr CreateSimpleStyle(WCharCP styleName, DgnFileR dgnFile) const;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicGeometryTests, Line)
    {
    LineUpFiles(L"Line.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::ElementId eid1;
    v8editor.AddLine(&eid1);
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // 
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    VerifyElement(*db, eid1, GeometricPrimitive::GeometryType::CurvePrimitive);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicGeometryTests, LineUpdateWithoutAnyChange)
    {
    LineUpFiles(L"LineUpdateWithoutAnyChange.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::ElementId eid1;
    v8editor.AddLine(&eid1);
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // 
    if (true)
    {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
        VerifyElement(*db, eid1, GeometricPrimitive::GeometryType::CurvePrimitive);
    }

    DoUpdate(m_dgnDbFileName, m_v8FileName, false, false);
    // *** TBD: Check that m_count == 0
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicGeometryTests, EnlargeProjectExtents)
    {
    LineUpFiles(L"LineUpdateWithoutAnyChange.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    //  Do the initial conversion on a file containing a single line that starts at 0,0,0 and extends 1 meter to the right.
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::ElementId eid1;
    v8editor.AddLine(&eid1, v8editor.m_defaultModel, DPoint3d::FromZero());
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    BentleyApi::DRange3d projectExtents;
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
        projectExtents = db->GeoLocation().GetProjectExtents();
        }

    // Add another line that is 1 meter above and update
    DgnV8Api::ElementId eid2;
    v8editor.AddLine(&eid2, v8editor.m_defaultModel, DPoint3d::From(0, 1000, 0));
    v8editor.Save();
    DoUpdate(m_dgnDbFileName, m_v8FileName);
    BentleyApi::DRange3d updatedProjectExtents;
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
        updatedProjectExtents = db->GeoLocation().GetProjectExtents();
        }
    // Check that the project's extents were expanded
    EXPECT_TRUE(projectExtents.IsContained(updatedProjectExtents)) << " original project extents should be contained IN the updated extents (updated is bigger)";
    EXPECT_FALSE(updatedProjectExtents.IsContained(projectExtents)) << " updated project extents should NOT be contained in the original extents (original is smaller)";

    // Delete the second line 
    DgnV8Api::EditElementHandle l2eh(eid2, v8editor.m_defaultModel);
    ASSERT_TRUE(l2eh.IsValid());
    ASSERT_EQ(0, l2eh.DeleteFromModel());
    v8editor.Save();
    DoUpdate(m_dgnDbFileName, m_v8FileName);
    BentleyApi::DRange3d updatedProjectExtents2;
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
        updatedProjectExtents2 = db->GeoLocation().GetProjectExtents();
        }
    // Check that the project's extents did NOT shrink
    EXPECT_TRUE(updatedProjectExtents.IsEqual(updatedProjectExtents2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicGeometryTests, Cell)
    {
    LineUpFiles(L"Cell.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    
    // -----------------------------------------------------------------------------------------------------------
    // Create Named Cell with following hirarchy
    //  Cell 
    //      --> Arc
    //      --> Arc

    V8FileEditor v8editor;
    BentleyStatus status = ERROR;
    v8editor.Open(m_v8FileName);
    DgnV8Api::EditElementHandle arcEEH1, arcEEH2;
    v8editor.CreateArc(arcEEH1, false);
    v8editor.CreateArc(arcEEH2, false);

    DgnV8Api::EditElementHandle cellEEH;
    v8editor.CreateCell(cellEEH, L"UserCell",false);

    status = DgnV8Api::NormalCellHeaderHandler::AddChildElement(cellEEH, arcEEH1);
    EXPECT_TRUE(SUCCESS == status);
    status = DgnV8Api::NormalCellHeaderHandler::AddChildElement(cellEEH, arcEEH2);
    EXPECT_TRUE(SUCCESS == status);
    status = DgnV8Api::NormalCellHeaderHandler::AddChildComplete(cellEEH);
    EXPECT_TRUE(SUCCESS == status);

    EXPECT_TRUE( SUCCESS == cellEEH.AddToModel());
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // Verify
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    VerifyCellElement(*db, cellEEH.GetElementId(), 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicGeometryTests, NestedCell)
    {
    LineUpFiles(L"NestedCell.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    
    // -----------------------------------------------------------------------------------------------------------
    // Create Named Shared Cell with following hirarchy 

    //  Cell 
    //      --> Arc
    //      --> Cell
    //              --> Arc
    //              --> Arc
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::EditElementHandle arceeh0;
    v8editor.CreateArc(arceeh0, false);

    DgnV8Api::EditElementHandle cellEEH;
    v8editor.CreateCell(cellEEH, L"UserCell", false);

    BentleyStatus status = DgnV8Api::NormalCellHeaderHandler::AddChildElement(cellEEH, arceeh0);
    EXPECT_TRUE(SUCCESS == status);

    // Child Cell 
    DgnV8Api::EditElementHandle childCellEEH;
    v8editor.CreateCell(childCellEEH, L"ChildUserCell" , false);

    DgnV8Api::EditElementHandle arceeh1, arceeh2;
    v8editor.CreateArc(arceeh1, false);
    v8editor.CreateArc(arceeh2, false);

    status = DgnV8Api::NormalCellHeaderHandler::AddChildElement(childCellEEH, arceeh1);
    EXPECT_TRUE(SUCCESS == status);
    status = DgnV8Api::NormalCellHeaderHandler::AddChildElement(childCellEEH, arceeh2);
    EXPECT_TRUE(SUCCESS == status);
    status = DgnV8Api::NormalCellHeaderHandler::AddChildComplete(childCellEEH);
    EXPECT_TRUE(SUCCESS == status);

    status = DgnV8Api::NormalCellHeaderHandler::AddChildElement(cellEEH, childCellEEH);
    EXPECT_TRUE(SUCCESS == status);
    status = DgnV8Api::NormalCellHeaderHandler::AddChildComplete(cellEEH);
    EXPECT_TRUE(SUCCESS == status);

    EXPECT_TRUE(SUCCESS == cellEEH.AddToModel());
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // Verify
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    VerifyCellElement(*db, cellEEH.GetElementId(), 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicGeometryTests, NamedView)
    {
    LineUpFiles(L"NamedView.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    DgnV8Api::NamedViewPtr    namedView;
    EXPECT_TRUE(DgnV8Api::NamedViewStatus::Success == DgnV8Api::NamedView::Create(namedView, *v8editor.m_file, L"Test View"));
    EXPECT_EQ(true, namedView.IsValid());
    EXPECT_TRUE(DgnV8Api::NamedViewStatus::Success == namedView->WriteToFile());
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicGeometryTests, Elements)
    {
    LineUpFiles(L"Elements.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    DgnV8Api::EditElementHandle eeh1;
    v8editor.CreateEllipse(eeh1);
    DgnV8Api::EditElementHandle eeh2;
    v8editor.CreateBSplineCurve(eeh2);
    DgnV8Api::EditElementHandle eeh3;
    v8editor.CreatePointString(eeh3);
    DgnV8Api::EditElementHandle eeh4;
    v8editor.CreateCone(eeh4);
    DgnV8Api::EditElementHandle eeh5;
    v8editor.CreateComplex(eeh5);
    v8editor.Save();
    DoConvert(m_dgnDbFileName, m_v8FileName);
    // 
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    VerifyElement(*db, eeh1.GetElementId(), GeometricPrimitive::GeometryType::CurveVector);
    VerifyElement(*db, eeh2.GetElementId(), GeometricPrimitive::GeometryType::CurveVector);
    VerifyElement(*db, eeh3.GetElementId(), GeometricPrimitive::GeometryType::CurvePrimitive);
    VerifyElement(*db, eeh4.GetElementId(), GeometricPrimitive::GeometryType::SolidPrimitive);
    VerifyElement(*db, eeh5.GetElementId(), GeometricPrimitive::GeometryType::CurveVector);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicGeometryTests, GroupHole)
    {
    LineUpFiles(L"GroupHole.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    DgnV8Api::EditElementHandle eeh1;
    v8editor.CreateGroupHole(eeh1,true);
    v8editor.Save();
    DoConvert(m_dgnDbFileName, m_v8FileName);
    // 
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    VerifyElement(*db, eeh1.GetElementId(), GeometricPrimitive::GeometryType::CurveVector);
    }

static const double CENTER_OFFSET = 0.0;
static const double MAX_OFFSET = 10.0;
static const double MIN_OFFSET = -20.0;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::MultilineStylePtr BasicGeometryTests::CreateSimpleStyle(WCharCP styleName, DgnFileR dgnFile) const
    {
    DgnV8Api::MultilineStylePtr tmpStyle = DgnV8Api::MultilineStyle::Create(styleName, dgnFile);

    //DgnV8Api::linestl tmpStyle = DgnV8Api::MultilineStyle::Create(styleName, dgnFile);

    DgnV8Api::MultilineProfilePtr styleprof = DgnV8Api::MultilineProfile::Create();

    styleprof->SetDistance (CENTER_OFFSET);
    if (SUCCESS != tmpStyle->InsertProfile(*styleprof, -1))
        return NULL;

    styleprof->SetDistance (MAX_OFFSET);
    if (SUCCESS != tmpStyle->InsertProfile(*styleprof, -1))
        return NULL;

    styleprof->SetDistance (MIN_OFFSET);
    if (SUCCESS != tmpStyle->InsertProfile(*styleprof, -1))
        return NULL;

    return tmpStyle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicGeometryTests, MultiLine)
    {
    LineUpFiles(L"MultiLine.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName


    DgnV8Api::EditElementHandle eeh1;
    DgnV8Api::ElementId eid;
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);

        DgnV8Api::MultilineStylePtr m_mlineStyle = CreateSimpleStyle(L"TestStyle", *v8editor.m_file);
        ASSERT_TRUE(m_mlineStyle.IsValid());

        ASSERT_EQ(SUCCESS, m_mlineStyle->Add());

        // ----------------------------------------
        // Create an element
        // ----------------------------------------
        DVec3d normal = DVec3d::From(0.0, 0.0, 1.0);
        DPoint3d points[3] = { { 0.0, 0.0, 0.0 }, { 0.0, 100.0, 0.0 }, { 100.0, 100.0, 0.0 } };
        ASSERT_EQ(SUCCESS, DgnV8Api::MultilineHandler::CreateMultilineElement(eeh1, NULL, *m_mlineStyle, 1.0, normal, points, 3, v8editor.m_defaultModel->Is3D(), *v8editor.m_defaultModel));
        eeh1.AddToModel();
        eid = eeh1.GetElementId();
        v8editor.Save();
        }
    DoConvert(m_dgnDbFileName, m_v8FileName);
    // 
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    VerifyElement(*db, eid, GeometricPrimitive::GeometryType::CurveVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetUpGradient(DgnV8Api::GradientSymbPtr& gradient, DgnV8Api::GradientMode mode)
    {
    gradient = DgnV8Api::GradientSymb::Create();

    const int TEST_NUMKEYS = 6;
    RgbColorDef colors[TEST_NUMKEYS];
    double pvalues[TEST_NUMKEYS];
    
    for (char i = 0;i < TEST_NUMKEYS;i++)
        {
        colors[i].red = 40*i;
        colors[i].green = 40 * (TEST_NUMKEYS - i);
        colors[i].blue = 255;
        pvalues[i] = 255;
        }

    gradient->SetMode (mode);
    gradient->SetFlags (0);
    gradient->SetAngle (45.0);
    gradient->SetTint (2.0);
    gradient->SetShift (3.0);
    gradient->SetKeys (TEST_NUMKEYS, colors, pvalues);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicGeometryTests, GradientFills)
    {
    LineUpFiles(L"GradientFills.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    DgnV8Api::EditElementHandle eeh1;
    v8editor.CreateEllipse(eeh1);
    ElementRefP oldRef = eeh1.GetElementRef();

    DgnV8Api::IAreaFillPropertiesEdit *fpEdit = dynamic_cast<DgnV8Api::IAreaFillPropertiesEdit *>(&eeh1.GetHandler());
    //Add gradient fill.
    DgnV8Api::GradientSymbPtr gradientR;
    SetUpGradient(gradientR, DgnV8Api::GradientMode::Linear);
    EXPECT_EQ(true, fpEdit->AddGradientFill(eeh1, *gradientR));
    //Replace in model.
    eeh1.ReplaceInModel(oldRef);
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // 
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    VerifyElement(*db, eeh1.GetElementId(), GeometricPrimitive::GeometryType::CurveVector);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetUpPattern(Bentley::PatternParamsPtr& params)
    {
    params = DgnV8Api::PatternParams::Create ();
    DgnV8Api::DwgHatchDef hatchDef;
    memset (&hatchDef, 0, sizeof (hatchDef));
    hatchDef.nDefLines = 1;
    hatchDef.pixelSize = 1.0;
    hatchDef.islandStyle = 0;

    params->SetDwgHatchDef (hatchDef);
    params->SetColor(100);
    params->SetPrimarySpacing(0.1524);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetUpHatchLine(Bentley::DwgHatchDefLineR hatchline)
    {
    hatchline.angle = 45.0;
    hatchline.nDashes = 1;
    hatchline.dashes[0] = 1.0;
    hatchline.offset.x = 0.0;
    hatchline.offset.y = 0.0;
    hatchline.through = hatchline.offset;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicGeometryTests, PatternFill)
    {
    LineUpFiles(L"PatternFill.ibim", L"Test3d.dgn", false); // creates TestAddRef.ibim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    DgnV8Api::EditElementHandle eeh1;
    v8editor.CreateEllipse(eeh1);
    ElementRefP oldRef = eeh1.GetElementRef();
    DgnV8Api::IAreaFillPropertiesEdit *fpEdit = dynamic_cast<DgnV8Api::IAreaFillPropertiesEdit *>(&eeh1.GetHandler());
    Bentley::PatternParamsPtr pattern;
    DgnV8Api::DwgHatchDefLine v8HatchDef;
    SetUpPattern(pattern);
    SetUpHatchLine(v8HatchDef);
    EXPECT_EQ(true, fpEdit->AddPattern(eeh1, *pattern, &v8HatchDef));

    //Replace in model.
    eeh1.ReplaceInModel(oldRef);
    v8editor.Save(); 

    DoConvert(m_dgnDbFileName, m_v8FileName);

    // 
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    VerifyElement(*db, eeh1.GetElementId(), GeometricPrimitive::GeometryType::CurveVector);
    }

