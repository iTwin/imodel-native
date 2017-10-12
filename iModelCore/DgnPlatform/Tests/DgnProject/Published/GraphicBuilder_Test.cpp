/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/GraphicBuilder_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../TestFixture/DgnDbTestFixtures.h"
#include "FakeRenderSystem.h"

using namespace FakeRender;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/17
//=======================================================================================
struct DisjointCurvesTest : DgnDbTestFixture, GraphicProcessor
{
protected:
    GraphicProcessorSystem m_system;
    uint32_t m_numDisjoint = 0;
    uint32_t m_numContinuous = 0;

    DisjointCurvesTest() : m_system(*this) { }

    void ExpectDisjoint(uint32_t num) const { EXPECT_EQ(num, m_numDisjoint); }
    void ExpectContinuous(uint32_t num) const { EXPECT_EQ(num, m_numContinuous); }

    void Process(IndexedPolylineArgsCR args) override
        {
        if (args.m_disjoint)
            ++m_numDisjoint;
        else
            ++m_numContinuous;
        }

    template<typename T> void Test(uint32_t numDisjoint, uint32_t numContinuous, T createGraphic)
        {
        m_numDisjoint = m_numContinuous = 0;

        // Using view coords because have no viewport from which to determine appropriate facet tolerance
        GraphicBuilderPtr gf = m_system._CreateGraphic(GraphicBuilder::CreateParams::View(GetDgnDb()));
        ActivateGraphicParams(*gf);
        createGraphic(*gf);
        gf->Finish();

        EXPECT_EQ(numDisjoint, m_numDisjoint);
        EXPECT_EQ(numContinuous, m_numContinuous);
        }

    template<typename T> void ExpectDisjoint(T createGraphic) { Test(1, 0, createGraphic); }
    template<typename T> void ExpectContinuous(T createGraphic) { Test(0, 1, createGraphic); }

    static void ActivateGraphicParams(GraphicBuilderR gf)
        {
        gf.SetSymbology(ColorDef::Red(), ColorDef::Blue(), 5);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DisjointCurvesTest, SinglePrimitives)
    {
    SetupSeedProject();

    // Ellipse
    ExpectContinuous([](GraphicBuilderR gf)
        {
        gf.AddArc(DEllipse3d::FromCenterRadiusXY(DPoint3d::FromXYZ(50, 50, 0), 10), false, false);
        });

    // Line string
    ExpectContinuous([](GraphicBuilderR gf)
        {
        DPoint2d pts[3] =
            {
            DPoint2d::From(0,0),
            DPoint2d::From(10, 0),
            DPoint2d::From(10, 10)
            };

        gf.AddLineString2d(3, pts, 0);
        });

    DPoint2d pts[2] = { DPoint2d::From(0, 0), DPoint2d::From(10, 0) };

    // Point String
    ExpectDisjoint([&](GraphicBuilderR gf)
        {
        gf.AddPointString2d(2, pts, 0);
        });

    // zero-length line string
    ExpectDisjoint([](GraphicBuilderR gf)
        {
        DPoint2d pts[2] = { DPoint2d::From(0,0), DPoint2d::From(0,0) };
        gf.AddLineString2d(2, pts, 0);
        });

    // single-point line string
    ExpectDisjoint([&](GraphicBuilderR gf)
        {
        gf.AddLineString2d(1, pts, 0);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DisjointCurvesTest, CurveVector)
    {
    SetupSeedProject();

    DPoint3d pts[2] = { DPoint3d::FromXYZ(200, 50, 0), DPoint3d::FromXYZ(250, 50, 0) };
    auto adjustY = [&]() { pts[0].y += 35; pts[1].y = pts[0].y; };

    auto curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

    // point string
    curve->push_back(ICurvePrimitive::CreatePointString(pts, 2));

    // line
    adjustY();
    curve->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(pts[0], pts[1])));

    // zero-length line
    adjustY();
    curve->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(pts[0], pts[0])));

    // single-point line string
    adjustY();
    curve->push_back(ICurvePrimitive::CreateLineString(pts, 1));

    // line string
    adjustY();
    curve->push_back(ICurvePrimitive::CreateLineString(pts, 2));

    // zero-length line string with 2 points
    adjustY();
    pts[1] = pts[0];
    curve->push_back(ICurvePrimitive::CreateLineString(pts, 2));

    // zero-length line string with 3 points.
    // As in MicroStation, NOT treated as a point if it contains more than 2 vertices
    adjustY();
    DPoint3d same3Pts[3] = { *pts, *pts, *pts };
    curve->push_back(ICurvePrimitive::CreateLineString(same3Pts, 3));

    bool wantFilled = false;
    auto addCurveVector = [&](GraphicBuilderR gf) { gf.AddCurveVector(*curve, wantFilled); };

    // The disjoint curves get batched into one primitive, the continuous curves into another.
    Test(1, 1, addCurveVector);

    // Filled curve vectors cannot be rendered disjoint...
    wantFilled = true;
    ExpectContinuous(addCurveVector);

    // BOUNDARY_TYPE_None required for disjoint curve primitives...
    wantFilled = false;
    curve->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
    ExpectContinuous(addCurveVector);
    }


