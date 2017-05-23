/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/Render_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/RenderPrimitives.h>

USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectPointsEqual(QPoint3dCR lhs, QPoint3dCR rhs)
    {
    EXPECT_EQ(lhs.x, rhs.x);
    EXPECT_EQ(lhs.y, rhs.y);
    EXPECT_EQ(lhs.z, rhs.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectPointsEqual(DPoint3dCR lhs, DPoint3dCR rhs, double tolerance)
    {
    EXPECT_TRUE(lhs.IsEqual(rhs, tolerance));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectQPoint3d(DPoint3dCR dpt, QPoint3d::Params params, QPoint3dCR exp, double tolerance)
    {
    QPoint3d qpt(dpt, params);
    ExpectPointsEqual(qpt, exp);
    ExpectPointsEqual(qpt.Unquantize(params), dpt, tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Render_Tests, QPoint)
    {
    DRange3d range = DRange3d::From(DPoint3d::FromXYZ(0.0, -100.0, 200.0), DPoint3d::FromXYZ(50.0, 100.0, 10000.0));
    QPoint3d::Params params(range);
    ExpectPointsEqual(params.origin, range.low, 0.0);

    ExpectQPoint3d(range.low, params, QPoint3d(0,0,0), 0.0);
    ExpectQPoint3d(range.high, params, QPoint3d(0xffff,0xffff,0xffff), 0.0);

    DPoint3d center = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
    ExpectQPoint3d(center, params, QPoint3d(0x8000,0x8000,0x8000), 0.08);

    range.low.z = range.high.z = 500.0;
    params = QPoint3d::Params(range);

    ExpectQPoint3d(range.low, params, QPoint3d(0,0,0), 0.0);
    ExpectQPoint3d(range.high, params, QPoint3d(0xffff,0xffff,0), 0.0);

    center.z = 500.0;
    ExpectQPoint3d(center, params, QPoint3d(0x8000,0x8000,0), 0.002);
    }

enum Comparison { kLess, kGreater, kEqual };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectComparison(Comparison exp, VertexKeyCR lhs, VertexKeyCR rhs)
    {
    VertexKey::Comparator comp;
    bool less = comp(lhs, rhs);
    bool grtr = comp(rhs, lhs);
    EXPECT_FALSE(less && grtr);

    switch (exp)
        {
        case kLess:
            EXPECT_TRUE(less);
            EXPECT_FALSE(grtr);
            break;
        case kGreater:
            EXPECT_TRUE(grtr);
            EXPECT_FALSE(less);
            break;
        case kEqual:
            EXPECT_FALSE(grtr);
            EXPECT_FALSE(less);
            break;
        }
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct FakeVertexKey : VertexKey
{
    explicit FakeVertexKey(QVertex3dCR vert)
        {
        m_normal = QPoint3d(0,1,0);
        m_feature = Feature();
        m_position = vert;
        m_normalValid = true;
        m_paramValid = false;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectComparison(Comparison exp, QVertex3dCR lhs, QVertex3dCR rhs)
    {
    ExpectComparison(exp, FakeVertexKey(lhs), FakeVertexKey(rhs));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectLessThan(QVertex3dCR lhs, QVertex3dCR rhs) { ExpectComparison(kLess, lhs, rhs); }
void ExpectGreaterThan(QVertex3dCR lhs, QVertex3dCR rhs) { ExpectComparison(kGreater, lhs, rhs); }
void ExpectEqual(QVertex3dCR lhs, QVertex3dCR rhs) { ExpectComparison(kEqual, lhs, rhs); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Render_Tests, QVert)
    {
    DRange3d range = DRange3d::From(DPoint3d::FromXYZ(0.0, 0.0, 0.0), DPoint3d::FromXYZ(100.0, 100.0, 100.0));
    QVertex3d::Params params(range);

    QVertex3d qlow(range.low, params);
    EXPECT_TRUE(qlow.IsQuantized());
    ExpectPointsEqual(qlow.GetQPoint3d(), QPoint3d(0,0,0));

    QVertex3d qhigh(range.high, params);
    EXPECT_TRUE(qhigh.IsQuantized());
    ExpectPointsEqual(qhigh.GetQPoint3d(), QPoint3d(0xffff, 0xffff, 0xffff));

    DPoint3d tooHigh = DPoint3d::FromXYZ(101.0, 150.0, 200.0);
    QVertex3d qtooHigh(tooHigh, params);
    EXPECT_FALSE(qtooHigh.IsQuantized());
    ExpectPointsEqual(DPoint3d::From(qtooHigh.GetFPoint3d()), tooHigh, 0.00001);

    DPoint3d tooLow = tooHigh;
    tooLow.Negate();
    QVertex3d qtooLow(tooLow, params);
    EXPECT_FALSE(qtooLow.IsQuantized());
    ExpectPointsEqual(DPoint3d::From(qtooLow.GetFPoint3d()), tooLow, 0.00001);

    // Note: an unquantized vertex is always considered 'less than' a quantized vertex.
    // If both vertices are (or both are not) quantized, coordinates are compared.
    ExpectLessThan(qtooLow, qlow);
    ExpectLessThan(qtooLow, qhigh);
    ExpectLessThan(qtooLow, qtooHigh);
    ExpectLessThan(qlow, qhigh);
    ExpectGreaterThan(qlow, qtooHigh);
    ExpectGreaterThan(qhigh, qtooHigh);

    ExpectLessThan(qtooHigh, qhigh);
    ExpectLessThan(qtooHigh, qlow);
    ExpectGreaterThan(qtooHigh, qtooLow);
    ExpectGreaterThan(qhigh, qlow);
    ExpectGreaterThan(qhigh, qtooLow);
    ExpectGreaterThan(qlow, qtooLow);

    ExpectEqual(qtooLow, qtooLow);
    ExpectEqual(qlow, qlow);
    ExpectEqual(qhigh, qhigh);
    ExpectEqual(qtooHigh, qtooHigh);

    QVertex3dList verts(range);
    verts.Add(qtooLow);
    verts.Add(qlow);
    verts.Add(qhigh);
    verts.Add(qtooHigh);

    EXPECT_FALSE(verts.IsFullyQuantized());
    EXPECT_EQ(4, verts.size());
    verts.Requantize();
    EXPECT_TRUE(verts.IsFullyQuantized());
    EXPECT_EQ(4, verts.size());

    ExpectPointsEqual(verts.GetRange().low, tooLow, 0.00001);
    ExpectPointsEqual(verts.GetRange().high, tooHigh, 0.00001);

    QPoint3dListCR qpts = verts.GetQuantizedPoints();
    ExpectPointsEqual(qpts.Unquantize(0), tooLow, 0.0);
    ExpectPointsEqual(qpts.Unquantize(1), range.low, 0.0031);
    ExpectPointsEqual(qpts.Unquantize(2), range.high, 0.0023);
    ExpectPointsEqual(qpts.Unquantize(3), tooHigh, 0.0);
    }

