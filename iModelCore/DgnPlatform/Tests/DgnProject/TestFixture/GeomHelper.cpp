/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/GeomHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "GeomHelper.h"


const double GeomHelper::PLANE_LEN = 100;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
CurveVectorPtr GeomHelper::computeShape(double len)
{
    DPoint3d pts[6];
    pts[0] = DPoint3d::From(-len, -len);
    pts[1] = DPoint3d::From(+len, -len);
    pts[2] = DPoint3d::From(+len, +len);
    pts[3] = DPoint3d::From(-len, +len);
    pts[4] = pts[0];
    pts[5] = pts[0];
    pts[5].z = 1;

    return CurveVector::CreateLinear(pts, _countof(pts), CurveVector::BOUNDARY_TYPE_Open);
}
//---------------------------------------------------------------------------------------
//*@bsimethod                                    Umar.Hayat      07 / 15
//---------------------------------------------------------------------------------------
CurveVectorPtr GeomHelper::computeShape2d(double len)
{
    DPoint2d GTs[6];
    GTs[0] = DPoint2d::From(-len, -len);
    GTs[1] = DPoint2d::From(+len, -len);
    GTs[2] = DPoint2d::From(+len, +len);
    GTs[3] = DPoint2d::From(-len, +len);
    GTs[4] = GTs[0];
    GTs[5] = GTs[0];

    return CurveVector::CreateLinear(GTs, _countof(GTs), CurveVector::BOUNDARY_TYPE_Open);
}
