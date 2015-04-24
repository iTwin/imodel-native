/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnGraphics_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_SQLITE

#if defined (NEEDS_WORK_DGNITEM)
//=======================================================================================
// @bsiclass                                                    MattGooding     08/13
//=======================================================================================
struct DgnGraphicsTest3d : GenericDgnModelTestFixture
    {
    DgnGraphicsTest3d() : GenericDgnModelTestFixture (__FILE__, true) {}
    PhysicalModelP GetPhysicalModel() {return dynamic_cast<PhysicalModelP>(GetDgnModelP());}
    };

//=======================================================================================
// @bsiclass                                                    MattGooding     08/13
//=======================================================================================
struct DgnGraphicsTest2d : GenericDgnModelTestFixture
    {
    DgnGraphicsTest2d() : GenericDgnModelTestFixture (__FILE__, false) {}
    DrawingModelP GetDrawingModel() {return dynamic_cast<DrawingModelP>(GetDgnModelP());}
    };

static bool verifyRangeLow (double actual, double expected)
        {
        return fabs (expected - actual) < 1.0;
        }

static bool verifyRangeHigh (double actual, double expected)
{
        return fabs (expected - actual) < 1.0;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
static bool verifyRange (DgnElementP element, DRange3dCR expected)
    {
    ElementHandle eh (element);
    DRange3d elementRange = eh.GetElementCP()->GetRange();
    return  verifyRangeLow (elementRange.low.x, expected.low.x)
                && verifyRangeLow(elementRange.low.y, expected.low.y)
                && verifyRangeLow(elementRange.low.z, expected.low.z)
                && verifyRangeHigh(elementRange.high.x, expected.high.x)
                && verifyRangeHigh(elementRange.high.y, expected.high.y)
                && verifyRangeHigh(elementRange.high.z, expected.high.z)
                ;
//              0 == memcmp(&expected, &elementRange, sizeof (expected));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/13
//---------------------------------------------------------------------------------------
static bool conesEqual (DgnConeDetail const& cone0, DgnConeDetail const& cone1)
    {
    if (!cone0.m_centerA.AlmostEqual (cone1.m_centerA))
        return false;
    if (!cone0.m_centerB.AlmostEqual (cone1.m_centerB))
        return false;
    if (!cone0.m_vector0.AlmostEqual (cone1.m_vector0))
        return false;
    if (!cone0.m_vector90.AlmostEqual (cone1.m_vector90))
        return false;
    if (abs (cone0.m_radiusA - cone1.m_radiusA) > 1.0E-5)
        return false;
    if (abs (cone0.m_radiusB - cone1.m_radiusB) > 1.0E-5)
        return false;
    if (cone0.m_capped != cone1.m_capped)
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/13
//---------------------------------------------------------------------------------------
static bool boxesEqual (DgnBoxDetail const& box0, DgnBoxDetail const& box1)
    {
    if (!box0.m_baseOrigin.AlmostEqual (box1.m_baseOrigin))
        return false;
    if (!box0.m_topOrigin.AlmostEqual (box1.m_topOrigin))
        return false;
    if (!box0.m_vectorX.AlmostEqual (box1.m_vectorX))
        return false;
    if (!box0.m_vectorY.AlmostEqual (box1.m_vectorY))
        return false;
    if (abs (box0.m_baseX - box1.m_baseX) > 1.0E-5)
        return false;
    if (abs (box0.m_baseY - box1.m_baseY) > 1.0E-5)
        return false;
    if (abs (box0.m_topX - box1.m_topX) > 1.0E-5)
        return false;
    if (abs (box0.m_topY - box1.m_topY) > 1.0E-5)
        return false;
    if (box0.m_capped != box1.m_capped)
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, ChangeTypeAndSave)
    {
    DSegment3d segment;
    segment.point[0] = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    segment.point[1] = DPoint3d::FromXYZ (100.0, 100.0, 100.0);

    EditElementHandle line;
    ASSERT_TRUE (SUCCESS == LineHandler::CreateLineElement (line, NULL, segment, true, *GetDgnModelP()));
    ASSERT_TRUE (SUCCESS == line.AddToModel());
    DgnElementId originalId = line.GetElementId();

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging

    PhysicalGraphicsPtr modGraphics = GetPhysicalModel()->ReadPhysicalGraphics (originalId);
    ASSERT_TRUE (modGraphics.IsValid());

    DgnBoxDetail boxDetail = DgnBoxDetail::InitFromCenterAndSize (DPoint3d::FromZero(),
                                                                  DPoint3d::From (10000.0, 10000.0, 10000.0),
                                                                  true);
    ASSERT_TRUE (SUCCESS == modGraphics->AddSolidPrimitive (*ISolidPrimitive::CreateDgnBox (boxDetail)));

    DgnElementId updatedId = modGraphics->Save();
    ASSERT_TRUE (updatedId.IsValid());
    ASSERT_TRUE (originalId == updatedId);

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging

    PhysicalGraphicsPtr readGraphics = GetPhysicalModel()->ReadPhysicalGraphics (updatedId);
    ASSERT_TRUE (readGraphics.IsValid());

    ASSERT_TRUE (2 == readGraphics->GetSize());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, SaveAndLoadPhysicalGraphics)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());
    ASSERT_TRUE (graphics->IsEmpty());

    PolyfaceHeaderPtr polyface = PolyfaceHeader::CreateVariableSizeIndexed();

    DPoint3d polygon[3];
    polygon[0].Init (0.0, 0.0, 0.0);
    polygon[1].Init (10000.0, 0.0, 0.0);
    polygon[2].Init (0.0, 10000.0, 0.0);
    polyface->AddPolygon (polygon, 3);

    ASSERT_TRUE (SUCCESS == graphics->AddPolyface (*polyface.get()));

    DPoint3d linePoints[4];
    linePoints[0].Init (0.0, 0.0, 0.0);
    linePoints[1].Init (10000.0, 0.0, 0.0);
    linePoints[3].Init (10000.0, 10000.0, 0.0);
    linePoints[2].Init (0.0, 10000.0, 0.0);

    ASSERT_TRUE (SUCCESS == graphics->AddCurvePrimitive (*ICurvePrimitive::CreateLineString (linePoints, 4)));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    PhysicalGraphicsPtr loadedGraphics = GetPhysicalModel()->ReadPhysicalGraphics (graphicsId);
    ASSERT_TRUE (loadedGraphics.IsValid());
    ASSERT_TRUE (loadedGraphics->GetSize() == graphics->GetSize());

    bool foundPolyface = false, foundCurve = false;
    for (DgnGraphics::const_iterator iter = loadedGraphics->begin(); loadedGraphics->end() != iter; ++iter)
        {
        switch ((*iter)->GetType())
            {
            case DgnGraphics::Entry::Type::Polyface:
                {
                PolyfaceHeaderCP polyfaceInGraphics = (*iter)->GetAsPolyfaceHeaderCP();
                ASSERT_TRUE (NULL != polyfaceInGraphics);
                ASSERT_TRUE (polyfaceInGraphics->GetPointCount() == polyface->GetPointCount());
                ASSERT_TRUE (polyfaceInGraphics->GetPointIndexCount() == polyface->GetPointIndexCount());
                foundPolyface = true;
                break;
                }
            case DgnGraphics::Entry::Type::CurveVector:
                {
                CurveVectorCP curveInGraphics = (*iter)->GetAsCurveVectorCP();
                ASSERT_TRUE (NULL != curveInGraphics);
                ASSERT_TRUE (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == curveInGraphics->HasSingleCurvePrimitive());
                foundCurve = true;
                break;
                }
            default:
                ASSERT_TRUE (false);
                break;
            }
        }

    ASSERT_TRUE (foundPolyface);
    ASSERT_TRUE (foundCurve);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest2d, SaveAndLoadDrawingGraphics)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    DrawingGraphicsPtr graphics = GetDrawingModel()->Create2dGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());
    ASSERT_TRUE (graphics->IsEmpty());

    DPoint3d linePoints[4];
    linePoints[0].Init (0.0, 0.0, 0.0);
    linePoints[1].Init (10000.0, 0.0, 0.0);
    linePoints[3].Init (10000.0, 10000.0, 0.0);
    linePoints[2].Init (0.0, 10000.0, 0.0);

    ASSERT_TRUE (SUCCESS == graphics->AddCurvePrimitive (*ICurvePrimitive::CreateLineString (linePoints, 4)));

    DPoint3d curvePts[8];
    curvePts[0].Init (0.0, 0.0, 0.0);
    curvePts[1].Init (10000.0, 0.0, 0.0);
    curvePts[2].Init (15000.0, 2000.0, 0.0);
    curvePts[3].Init (13000.0, 4000.0, 0.0);
    curvePts[4].Init (10000.0, 6000.0, 0.0);
    curvePts[5].Init (8000.0, 4000.0, 0.0);
    curvePts[6].Init (6000.0, 2000.0, 0.0);
    curvePts[7] = curvePts[0];

    ASSERT_TRUE (SUCCESS == graphics->AddCurveVector (*CurveVector::CreateLinear (curvePts, 8, CurveVector::BOUNDARY_TYPE_Outer).get()));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DrawingGraphicsPtr loadedGraphics = GetDrawingModel()->Read2dGraphics (graphicsId);
    ASSERT_TRUE (loadedGraphics.IsValid());
    ASSERT_TRUE (loadedGraphics->GetSize() == graphics->GetSize());

    bool foundCurve = false, foundLineString = false;
    for (DgnGraphics::const_iterator iter = loadedGraphics->begin(); loadedGraphics->end() != iter; ++iter)
        {
        switch ((*iter)->GetType())
            {
            case DgnGraphics::Entry::Type::CurveVector:
                {
                CurveVectorCP curveInGraphics = (*iter)->GetAsCurveVectorCP();
                ASSERT_TRUE (NULL != curveInGraphics);

                bvector <DPoint3d> const* pointsInGraphics = (*curveInGraphics->begin())->GetLineStringCP();
                if (0 == memcmp (&(*pointsInGraphics)[0], linePoints, sizeof (DPoint3d) * 4))
                    foundLineString = true;
                else if (0 == memcmp (&(*pointsInGraphics)[0], curvePts, sizeof (DPoint3d) * 8))
                    foundCurve = true;
                else
                    ASSERT_TRUE (false);
                break;
                }
            default:
                ASSERT_TRUE (false);
                break;
            }
        }

    ASSERT_TRUE (foundLineString);
    ASSERT_TRUE (foundCurve);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, IterateOverGeometry)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());
    ASSERT_TRUE (graphics->IsEmpty());

    DPoint3d linePoints[4];
    linePoints[0].Init (0.0, 0.0, 0.0);
    linePoints[1].Init (10000.0, 0.0, 0.0);
    linePoints[3].Init (10000.0, 10000.0, 0.0);
    linePoints[2].Init (0.0, 10000.0, 0.0);

    ASSERT_TRUE (SUCCESS == graphics->AddCurvePrimitive (*ICurvePrimitive::CreateLineString (linePoints, 4)));

    DgnBoxDetail boxDetail = DgnBoxDetail::InitFromCenterAndSize (DPoint3d::From (0.0, 0.0, 0.0),
                                                                  DPoint3d::From (10000.0, 10000.0, 10000.0),
                                                                  true);
    ASSERT_TRUE (SUCCESS == graphics->AddSolidPrimitive (*ISolidPrimitive::CreateDgnBox (boxDetail)));

    DgnConeDetail coneDetail (DPoint3d::From (0.0, 0.0, 0.0),
                              DPoint3d::From (0.0, 0.0, 10000.0),
                              5000.0,
                              5000.0,
                              true);
    ASSERT_TRUE (SUCCESS == graphics->AddSolidPrimitive (*ISolidPrimitive::CreateDgnCone (coneDetail)));

    ASSERT_FALSE (graphics->IsEmpty());
    ASSERT_TRUE (3 == graphics->GetSize());

    bool foundCurve = false, foundBox = false, foundCone = false;
    for (auto iter : *graphics)
        {
        switch (iter->GetType())
            {
            case DgnGraphics::Entry::Type::CurvePrimitive:
                {
                foundCurve = true;
                ICurvePrimitiveCP curveInGraphics = iter->GetAsICurvePrimitiveCP();
                bvector<DPoint3d> const* lineString = curveInGraphics->GetLineStringCP();
                ASSERT_TRUE (lineString != NULL);
                ASSERT_TRUE (0 == memcmp (linePoints, &lineString->at (0), sizeof (*linePoints) * 4));
                break;
                }
            case DgnGraphics::Entry::Type::SolidPrimitive:
                {
                ISolidPrimitiveCP solid = iter->GetAsISolidPrimitiveCP();
                switch (solid->GetSolidPrimitiveType())
                    {
                    case SolidPrimitiveType_DgnBox:
                        {
                        foundBox = true;
                        DgnBoxDetail boxDetailInGraphics;
                        ASSERT_TRUE (solid->TryGetDgnBoxDetail (boxDetailInGraphics));
                        ASSERT_TRUE (boxesEqual (boxDetail, boxDetailInGraphics));
                        break;
                        }
                    case SolidPrimitiveType_DgnCone:
                        {
                        foundCone = true;
                        DgnConeDetail coneDetailInGraphics;
                        ASSERT_TRUE (solid->TryGetDgnConeDetail (coneDetailInGraphics));
                        ASSERT_TRUE (conesEqual (coneDetail, coneDetailInGraphics));
                        break;
                        }
                    }
                break;
                }
            }
        }
    ASSERT_TRUE (foundCurve);
    ASSERT_TRUE (foundBox);
    ASSERT_TRUE (foundCone);

    ASSERT_TRUE (graphics->Save().IsValid());

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, CreatePhysicalGraphics)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    // Cannot save empty graphics.
    DgnElementId graphicsId = graphics->Save();
    ASSERT_FALSE (graphicsId.IsValid());

    DPoint3d linePoints[4];
    linePoints[0].Init (0.0, 0.0, 0.0);
    linePoints[1].Init (10000.0, 0.0, 0.0);
    linePoints[3].Init (10000.0, 10000.0, 0.0);
    linePoints[2].Init (0.0, 10000.0, 0.0);

    ASSERT_TRUE (SUCCESS == graphics->AddCurvePrimitive (*ICurvePrimitive::CreateLineString (linePoints, 4)));

    graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DgnElementPtr element = GetDgnModelP()->GetDgnDb().Elements().GetElementById (graphicsId);
    ASSERT_TRUE (element.IsValid());

    DRange3d expectedRange;
    expectedRange.low.x   = 0.;
    expectedRange.low.y   = 0.;
    expectedRange.low.z   = 0.;
    expectedRange.high.x  = 10000.;
    expectedRange.high.y  = 10000.;
    expectedRange.high.z  = 0.;
    ASSERT_TRUE (verifyRange (element.get(), expectedRange));

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, AddTextString)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    DPoint2d textScale;
    textScale.Init (5000.0, 5000.0);
    TextStringStylePtr style = TextStringStyle::Create();
    style->SetFont(DgnFontManager::GetDefaultTrueTypeFont());
    style->SetSize(textScale);
    
    TextStringPtr text = TextString::Create();
    text->SetText("DgnGraphics Test");
    text->SetStyle(*style);
    ASSERT_TRUE (text.IsValid());

    text->SetOriginFromJustificationOrigin(DPoint3d::FromZero(), TextString::HorizontalJustification::Center, TextString::VerticalJustification::Middle);

    ASSERT_TRUE (SUCCESS == graphics->AddTextString (*text));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DgnElementPtr element = GetDgnModelP()->GetDgnDb().Elements().GetElementById (graphicsId);
    ASSERT_TRUE (element.IsValid());

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging

    PhysicalGraphicsPtr loadedGraphics = GetPhysicalModel()->ReadPhysicalGraphics (graphicsId);
    ASSERT_TRUE (loadedGraphics.IsValid());
    ASSERT_TRUE (loadedGraphics->GetSize() == graphics->GetSize());

    TextStringCP loadedText = (*loadedGraphics->begin())->GetAsTextStringCP();
    ASSERT_TRUE (NULL != loadedText);
    ASSERT_TRUE (loadedText->GetText() == text->GetText());
    ASSERT_TRUE(loadedText->GetRange().IsEqual(text->GetRange(), 1.0E-8));
    ASSERT_TRUE (loadedText->GetOrigin().IsEqual (text->GetOrigin(), 1.0E-8));
    ASSERT_TRUE (loadedText->GetOrientation().IsEqual (text->GetOrientation(), 1.0E-8));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, AddSolidPrimitive_Box)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    DgnBoxDetail detail = DgnBoxDetail::InitFromCenterAndSize (DPoint3d::From (0.0, 0.0, 0.0),
                                                               DPoint3d::From (10000.0, 10000.0, 10000.0),
                                                               true);
    ASSERT_TRUE (SUCCESS == graphics->AddSolidPrimitive (*ISolidPrimitive::CreateDgnBox (detail)));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DgnElementPtr element = GetDgnModelP()->GetDgnDb().Elements().GetElementById (graphicsId);
    ASSERT_TRUE (element.IsValid());

    DRange3d expectedRange;
    expectedRange.low.x    = -5000.;
    expectedRange.low.y    = -5000.;
    expectedRange.low.z    = -5000.;
    expectedRange.high.x   = 5000.;
    expectedRange.high.y   = 5000.;
    expectedRange.high.z   = 5000.;
    ASSERT_TRUE (verifyRange (element.get(), expectedRange));

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, AddSolidPrimitive_Cone)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    DgnConeDetail detail (DPoint3d::From (0.0, 0.0, 0.0),
                          DPoint3d::From (0.0, 0.0, 10000.0),
                          5000.0,
                          5000.0,
                          true);
    ASSERT_TRUE (SUCCESS == graphics->AddSolidPrimitive (*ISolidPrimitive::CreateDgnCone (detail)));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DgnElementPtr element = GetDgnModelP()->GetDgnDb().Elements().GetElementById (graphicsId);
    ASSERT_TRUE (element.IsValid());

    DRange3d expectedRange;
    expectedRange.low.x    = -5000.;
    expectedRange.low.y    = -5000.;
    expectedRange.low.z    = 0.;
    expectedRange.high.x   = 5000.;
    expectedRange.high.y   = 5000.;
    expectedRange.high.z   = 10000.;
    ASSERT_TRUE (verifyRange (element.get(), expectedRange));

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, AddSolidPrimitive_Sphere)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    DgnSphereDetail detail (DPoint3d::FromZero (), 5000.0);
    ASSERT_TRUE (SUCCESS == graphics->AddSolidPrimitive (*ISolidPrimitive::CreateDgnSphere (detail)));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DgnElementPtr element = GetDgnModelP()->GetDgnDb().Elements().GetElementById (graphicsId);
    ASSERT_TRUE (element.IsValid());

    DRange3d expectedRange;
    expectedRange.low.x    = -5000.;
    expectedRange.low.y    = -5000.;
    expectedRange.low.z    = -5000.;
    expectedRange.high.x   = 5000.;
    expectedRange.high.y   = 5000.;
    expectedRange.high.z   = 5000.;
    ASSERT_TRUE (verifyRange (element.get(), expectedRange));

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, AddSolidPrimitive_Extrusion)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    CurveVectorPtr shape = CurveVector::CreateRectangle (-5000.0, -5000.0, 5000.0, 5000.0, 0.0, CurveVector::BOUNDARY_TYPE_Outer);
    DgnExtrusionDetail detail (shape, DVec3d::From (0.0, 0.0, 10000.0), true);
    ASSERT_TRUE (SUCCESS == graphics->AddSolidPrimitive (*ISolidPrimitive::CreateDgnExtrusion (detail)));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DgnElementPtr element = GetDgnModelP()->GetDgnDb().Elements().GetElementById (graphicsId);
    ASSERT_TRUE (element.IsValid());

    DRange3d expectedRange;
    expectedRange.low.x    = -5000;
    expectedRange.low.y    = -5000;
    expectedRange.low.z    = 0;
    expectedRange.high.x   = 5000;
    expectedRange.high.y   = 5000;
    expectedRange.high.z   = 10000;
    ASSERT_TRUE (verifyRange (element.get(), expectedRange));

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, AddSolidPrimitive_Rotation)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    CurveVectorPtr shape = CurveVector::CreateRectangle (0.0, 0.0, 10000.0, 10000.0, 0.0, CurveVector::BOUNDARY_TYPE_Outer);
    DgnRotationalSweepDetail detail (shape, DPoint3d::FromZero(), DVec3d::From (0.0, 1.0, 0.0), Angle::Pi(), true);
    ASSERT_TRUE (SUCCESS == graphics->AddSolidPrimitive (*ISolidPrimitive::CreateDgnRotationalSweep (detail)));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DgnElementPtr element = GetDgnModelP()->GetDgnDb().Elements().GetElementById (graphicsId);
    ASSERT_TRUE (element.IsValid());

    DRange3d expectedRange;
    expectedRange.low.x   = -10490;
    expectedRange.low.y   = -490;
    expectedRange.low.z   = -10490;
    expectedRange.high.x  = 10490;
    expectedRange.high.y  = 10490;
    expectedRange.high.z  = 490;
    ASSERT_TRUE (verifyRange (element.get(), expectedRange));

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, AddSolidPrimitive_Ruled)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    CurveVectorPtr  shapeA = CurveVector::CreateRectangle (0.0, 0.0, 10000.0, 10000.0, 0.0),
                    shapeB = CurveVector::CreateRectangle (0.0, 0.0, 10000.0, 10000.0, 10000.0);

    DgnRuledSweepDetail detail (shapeA, shapeB, false);
    ASSERT_TRUE (SUCCESS == graphics->AddSolidPrimitive (*ISolidPrimitive::CreateDgnRuledSweep (detail)));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DgnElementPtr element = GetDgnModelP()->GetDgnDb().Elements().GetElementById (graphicsId);
    ASSERT_TRUE (element.IsValid());

    DRange3d expectedRange;
    expectedRange.low.x   = 0;
    expectedRange.low.y   = 0;
    expectedRange.low.z   = 0;
    expectedRange.high.x  = 10000;
    expectedRange.high.y  = 10000;
    expectedRange.high.z  = 10000;
    ASSERT_TRUE (verifyRange (element.get(), expectedRange));

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, AddMSBsplineCurve)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    DPoint3d poles[3];
    poles[0].Init (0.0, 0.0, 0.0);
    poles[1].Init (10000.0, 0.0, 0.0);
    poles[2].Init (0.0, 10000.0, 0.0);
    MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (poles, 3, 3, false);
    ASSERT_TRUE (SUCCESS == graphics->AddCurvePrimitive (*ICurvePrimitive::CreateBsplineCurve (*curve.get())));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DgnElementPtr element = GetDgnModelP()->GetDgnDb().Elements().GetElementById (graphicsId);
    ASSERT_TRUE (element.IsValid());

    DRange3d expectedRange;
    expectedRange.low.x   = 0;
    expectedRange.low.y   = 0;
    expectedRange.low.z   = 0;
    expectedRange.high.x  = 10000;
    expectedRange.high.y  = 10000;
    expectedRange.high.z  = 0;
    ASSERT_TRUE (verifyRange (element.get(), expectedRange));

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, AddMSBsplineSurface)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    DPoint3d poles[3];
    poles[0].Init (0.0, 0.0, 0.0);
    poles[1].Init (10000.0, 0.0, 0.0);
    poles[2].Init (0.0, 10000.0, 0.0);
    MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (poles, 3, 3, false);

    MSBsplineSurfacePtr surface = MSBsplineSurface::CreateLinearSweep (*curve.get(), DVec3d::From (5000.0, 0.0, 10000.0));
    ASSERT_TRUE (SUCCESS == graphics->AddMSBsplineSurface (*surface.get()));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DgnElementPtr element = GetDgnModelP()->GetDgnDb().Elements().GetElementById (graphicsId);
    ASSERT_TRUE (element.IsValid());

    DRange3d expectedRange;
    expectedRange.low.x   = 0;
    expectedRange.low.y   = 0;
    expectedRange.low.z   = 0;
    expectedRange.high.x  = 15000;
    expectedRange.high.y  = 10000;
    expectedRange.high.z  = 10000;
    ASSERT_TRUE (verifyRange (element.get(), expectedRange));

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, AddPolyface)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    PolyfaceHeaderPtr polyface = PolyfaceHeader::CreateVariableSizeIndexed();

    DPoint3d polygon[3];
    polygon[0].Init (0.0, 0.0, 0.0);
    polygon[1].Init (10000.0, 0.0, 0.0);
    polygon[2].Init (0.0, 10000.0, 0.0);
    polyface->AddPolygon (polygon, 3);

    ASSERT_TRUE (SUCCESS == graphics->AddPolyface (*polyface.get()));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DgnElementPtr element = GetDgnModelP()->GetDgnDb().Elements().GetElementById (graphicsId);
    ASSERT_TRUE (element.IsValid());

    DRange3d expectedRange;
    expectedRange.low.x   = 0;
    expectedRange.low.y   = 0;
    expectedRange.low.z  = 0;
    expectedRange.high.x  = 10000;
    expectedRange.high.y  = 10000;
    expectedRange.high.z  = 0;
    ASSERT_TRUE (verifyRange (element.get(), expectedRange));

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest2d, AddCurveVector)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    DrawingGraphicsPtr graphics = GetDrawingModel()->Create2dGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    DPoint3d curvePts[8];
    curvePts[0].Init (0.0, 0.0, 0.0);
    curvePts[1].Init (10000.0, 0.0, 0.0);
    curvePts[2].Init (15000.0, 2000.0, 0.0);
    curvePts[3].Init (13000.0, 4000.0, 0.0);
    curvePts[4].Init (10000.0, 6000.0, 0.0);
    curvePts[5].Init (8000.0, 4000.0, 0.0);
    curvePts[6].Init (6000.0, 2000.0, 0.0);
    curvePts[7] = curvePts[0];
    CurveVectorPtr curve = CurveVector::CreateLinear (curvePts, 8, CurveVector::BOUNDARY_TYPE_Outer);

    ASSERT_TRUE (SUCCESS == graphics->AddCurveVector (*curve.get()));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DgnElementPtr element = GetDgnModelP()->GetDgnDb().Elements().GetElementById (graphicsId);
    ASSERT_TRUE (element.IsValid());

    DRange3d expectedRange;
    expectedRange.low.x   = 0;
    expectedRange.low.y   = 0;
    expectedRange.low.z   = 0;
    expectedRange.high.x  = 15000;
    expectedRange.high.y  = 6000;
    expectedRange.high.z  = 0;
    ASSERT_TRUE (verifyRange (element.get(), expectedRange));

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest2d, CreateDrawingGraphics)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    DrawingGraphicsPtr graphics = GetDrawingModel()->Create2dGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    // Cannot save empty graphics.
    DgnElementId graphicsId = graphics->Save();
    ASSERT_FALSE (graphicsId.IsValid());

    DPoint3d linePoints[4];
    linePoints[0].Init (0.0, 0.0, 0.0);
    linePoints[1].Init (10000.0, 0.0, 0.0);
    linePoints[3].Init (10000.0, 10000.0, 0.0);
    linePoints[2].Init (0.0, 10000.0, 0.0);

    ASSERT_TRUE (SUCCESS == graphics->AddCurvePrimitive (*ICurvePrimitive::CreateLineString (linePoints, 4)));

    graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    DgnElementPtr element = GetDgnModelP()->GetDgnDb().Elements().GetElementById (graphicsId);
    ASSERT_TRUE (element.IsValid());

    DRange3d expectedRange;
    expectedRange.low.x   = 0;
    expectedRange.low.y   = 0;
    expectedRange.low.z   = 0;
    expectedRange.high.x  = 10000;
    expectedRange.high.y  = 10000;
    expectedRange.high.z  = 0;
    ASSERT_TRUE (verifyRange (element.get(), expectedRange));

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, LoadModifyAndSave)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    DPoint3d linePoints[4];
    linePoints[0].Init (0.0, 0.0, 0.0);
    linePoints[1].Init (10000.0, 0.0, 0.0);
    linePoints[2].Init (0.0, 10000.0, 0.0);
    linePoints[3].Init (10000.0, 10000.0, 0.0);
    ASSERT_TRUE (SUCCESS == graphics->AddCurvePrimitive (*ICurvePrimitive::CreateLineString (linePoints, 4)));

    PolyfaceHeaderPtr polyface = PolyfaceHeader::CreateVariableSizeIndexed();
    DPoint3d polygon[3];
    polygon[0].Init (0.0, 0.0, 0.0);
    polygon[1].Init (10000.0, 0.0, 0.0);
    polygon[2].Init (0.0, 10000.0, 0.0);
    polyface->AddPolygon (polygon, 3);
    ASSERT_TRUE (SUCCESS == graphics->AddPolyface (*polyface.get()));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    PhysicalGraphicsPtr loadedGraphics = GetPhysicalModel()->ReadPhysicalGraphics (graphicsId);
    ASSERT_TRUE (loadedGraphics.IsValid());
    ASSERT_TRUE (loadedGraphics->GetSize() == 2);

    DgnGraphics::iterator loadedGraphicsIter = loadedGraphics->begin();
    loadedGraphicsIter = loadedGraphics->Erase (loadedGraphicsIter);
    ASSERT_TRUE (loadedGraphics->end() != loadedGraphicsIter);

    ASSERT_TRUE (loadedGraphics->Save() == graphicsId);

    PhysicalGraphicsPtr modifiedGraphics =  GetPhysicalModel()->ReadPhysicalGraphics (graphicsId);
    ASSERT_TRUE (modifiedGraphics.IsValid());
    ASSERT_TRUE (modifiedGraphics->GetSize() == 1);
    ASSERT_TRUE ((*modifiedGraphics->begin())->GetType() == DgnGraphics::Entry::Type::Polyface);

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, CreateWithLevel)
    {
    auto& levelTable = GetDgnModelP()->GetDgnDb().Levels();

    DgnLevels::SubLevel::Appearance appear;
    DgnLevels::Level level ("Test Level 1", DgnLevels::Scope::Physical);
    ASSERT_TRUE (BE_SQLITE_OK == levelTable.InsertLevel (level, appear));

    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics (level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());
    ASSERT_TRUE (graphics->GetElementHandleR().GetWriteableElement()->Element().GetLevel() == level.GetLevelId());

    DPoint3d linePoints[4];
    linePoints[0].Init (0.0, 0.0, 0.0);
    linePoints[1].Init (10000.0, 0.0, 0.0);
    linePoints[2].Init (0.0, 10000.0, 0.0);
    linePoints[3].Init (10000.0, 10000.0, 0.0);
    ASSERT_TRUE (SUCCESS == graphics->AddCurvePrimitive (*ICurvePrimitive::CreateLineString (linePoints, 4)));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    PhysicalGraphicsPtr loadedGraphics = GetPhysicalModel()->ReadPhysicalGraphics (graphicsId);
    ASSERT_TRUE (loadedGraphics.IsValid());
    ASSERT_TRUE (loadedGraphics->GetElementHandleR().GetWriteableElement()->Element().GetLevel() == level.GetLevelId());

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnGraphicsTest3d, AddWithSymbology)
    {
    DgnLevels::Level level = GetDgnProjectP()->Levels().QueryLevelByName ("Default");
    PhysicalGraphicsPtr graphics = GetPhysicalModel()->CreatePhysicalGraphics(level.GetLevelId());
    ASSERT_TRUE (graphics.IsValid());

    DPoint3d linePoints[4];
    linePoints[0].Init (0.0, 0.0, 0.0);
    linePoints[1].Init (10000.0, 0.0, 0.0);
    linePoints[3].Init (10000.0, 10000.0, 0.0);
    linePoints[2].Init (0.0, 10000.0, 0.0);

    DgnGraphics::Symbology modifiedSymb = DgnGraphics::GetDefaultSymbology();
    modifiedSymb.m_color = ColorDef(255, 0, 0);
    modifiedSymb.m_weight = 11;
    modifiedSymb.m_style = 4;

    ASSERT_TRUE (SUCCESS == graphics->AddCurvePrimitive (*ICurvePrimitive::CreateLineString (linePoints, 4), modifiedSymb));

    PolyfaceHeaderPtr polyface = PolyfaceHeader::CreateVariableSizeIndexed();

    DPoint3d polygon[3];
    polygon[0].Init (0.0, 0.0, 0.0);
    polygon[1].Init (10000.0, 0.0, 0.0);
    polygon[2].Init (0.0, 10000.0, 0.0);
    polyface->AddPolygon (polygon, 3);
    ASSERT_TRUE (SUCCESS == graphics->AddPolyface (*polyface.get(), DgnGraphics::GetDefaultSymbology()));

    DgnElementId graphicsId = graphics->Save();
    ASSERT_TRUE (graphicsId.IsValid());

    PhysicalGraphicsPtr loadedGraphics = GetPhysicalModel()->ReadPhysicalGraphics (graphicsId);
    ASSERT_TRUE (loadedGraphics.IsValid());
    ASSERT_TRUE (loadedGraphics->GetSize() == 2);

    for (auto& loadedEntry : *loadedGraphics)
        {
        switch (loadedEntry->GetType())
            {
            case DgnGraphics::Entry::Type::CurveVector:
                ASSERT_TRUE (0 == memcmp (&loadedEntry->GetSymbology(), &modifiedSymb, sizeof (modifiedSymb)));
                break;
            case DgnGraphics::Entry::Type::Polyface:
                ASSERT_TRUE (0 == memcmp (&loadedEntry->GetSymbology(), &DgnGraphics::GetDefaultSymbology(), sizeof (modifiedSymb)));
                break;
            default:
                ASSERT_TRUE (false);
                break;
            }
        }

    // just so we can verify what's happened in another process for debugging - can verify visually
    // that color-by-level works properly.
    auto& levelTable  = GetDgnModelP()->GetDgnDb().Levels();
    DgnLevels::SubLevel sublevel = levelTable.QuerySubLevel(DgnLevels::DefaultSubLevelId(level.GetLevelId()));

    ColorDef rgbColor (0,0,255);
    sublevel.GetAppearanceR().SetColor(rgbColor);
    ASSERT_TRUE (BE_SQLITE_OK == levelTable.UpdateSubLevel(sublevel));

    GetDgnProjectP()->SaveChanges(); // just so we can verify what's happened in another process for debugging
    }
#endif