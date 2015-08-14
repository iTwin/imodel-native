/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDSegment3d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDSEGMENT3D_H_
#define _JSDSEGMENT3D_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE


//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDSegment3d : RefCountedBase
{
private:
    DSegment3d m_segment;
public:
    JsDSegment3d() {m_segment.InitZero ();}
    JsDSegment3d (DPoint3dCR point0, DPoint3dCR point1) {m_segment.Init (point0, point1);}
    JsDSegment3d (JsDPoint3dP point0, JsDPoint3dP point1) {m_segment.Init (point0->Get (), point1->Get ());}

    JsDSegment3d (double x0, double y0, double z0, double x1, double y1, double z1)
        {m_segment.Init (x0, y0, z0, x1, y1, z1);}

    JsDPoint3dP GetStartPoint() {return new JsDPoint3d (m_segment.point[0]);}
    JsDPoint3dP GetEndPoint() {return new JsDPoint3d (m_segment.point[1]);}
    void SetStartPoint (JsDPoint3dP point) {m_segment.point[0] = point->Get ();}
    void SetEndPoint (JsDPoint3dP point) {m_segment.point[1] = point->Get ();}
    DSegment3d Get (){return m_segment;}
    JsDPoint3dP PointAtFraction (double f)
        {
        DPoint3d xyz;
        m_segment.FractionParameterToPoint (xyz, f);
        return new JsDPoint3d (xyz);
        }
    JsDRay3dP PointAndDerivativeAtFraction (double f)
        {
        DPoint3d xyz;
        DVec3d tangent;
        m_segment.FractionParameterToTangent (xyz, tangent, f);
        return new JsDRay3d (xyz, tangent);
        }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDSEGMENT3D_H_

