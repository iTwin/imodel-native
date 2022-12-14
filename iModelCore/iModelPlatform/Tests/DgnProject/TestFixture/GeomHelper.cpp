/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "GeomHelper.h"


const double GeomHelper::PLANE_LEN = 100;

//---------------------------------------------------------------------------------------
// @bsimethod
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
//*@bsimethod
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
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineSurfacePtr GeomHelper::CreateGridSurface(DPoint3dCR origin, double dx, double dy, size_t order, size_t numX, size_t numY)
    {
    bvector<DPoint3d> poles;
    for (size_t j = 0; j < numY; j++)
        {
        for (size_t i = 0; i < numX; i++)
            poles.push_back (DPoint3d::From (origin.x + i * dx, origin.y + j * dy, origin.z));
        }

    return MSBsplineSurface::CreateFromPolesAndOrder (poles,  NULL,
                NULL, (int)order, (int)numX, false,
                NULL, (int)order, (int)numY, false, true
                );
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TextStringPtr GeomHelper::CreateTextString(DgnDbR db, TextStringStylePtr style)
    {
    if (!style.IsValid())
        {
        style = TextStringStyle::Create();
        style->SetFont(FontId((uint64_t)1));
        style->SetSize(DPoint2d::From(1000.0, 1000.0));
        }

    TextStringPtr text = TextString::Create(db);
    text->SetText("lorem ipsum");
    text->SetStyle(*style);

    return text;
    }
