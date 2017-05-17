/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/Render_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/Render.h>

USING_NAMESPACE_BENTLEY_RENDER

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
void ExpectQuantizedPoint(DPoint3dCR dpt, DRange3dCR range, uint16_t qx, uint16_t qy, uint16_t qz, double tolerance)
    {
    QuantizedPoint qpt(range, dpt);
    EXPECT_EQ(qpt.m_x, qx);
    EXPECT_EQ(qpt.m_y, qy);
    EXPECT_EQ(qpt.m_z, qz);
    ExpectPointsEqual(qpt.Unquantized(range), dpt, tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Render_Tests, QuantizedPoint)
    {
    DRange3d range = DRange3d::From(DPoint3d::FromXYZ(0.0, -100.0, 200.0), DPoint3d::FromXYZ(50.0, 100.0, 10000.0));

    ExpectQuantizedPoint(range.low, range, 0, 0, 0, 0.0);
    ExpectQuantizedPoint(range.high, range, 0xffff, 0xffff, 0xffff, 0.0);

    DPoint3d center = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
    ExpectQuantizedPoint(center, range, 0x8000, 0x8000, 0x8000, 0.08);
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

