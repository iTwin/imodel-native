/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDRay3d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDRAY3D_H_
#define _JSDRAY3D_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDRay3d : JsGeomWrapperBase<DRay3d>
{
public:
    JsDRay3d() {}
    JsDRay3d (DPoint3dCR point, DVec3dCR vector) {m_data.InitFromOriginAndVector (point, vector);}
    JsDRay3d (JsDPoint3dP point, JsDVector3dP vector) {m_data.InitFromOriginAndVector (point->Get (), vector->Get ());}

    JsDRay3d (double x0, double y0, double z0, double ux, double uy, double uz)
        {m_data.InitFromOriginAndVector (DPoint3d::From (x0, y0, z0), DVec3d::From (ux, uy, uz));}

    JsDPoint3dP GetOrigin() {return new JsDPoint3d (m_data.origin);}
    JsDVector3dP     GetDirection() {return new JsDVector3d (m_data.direction);}
    void SetOrigin (JsDPoint3dP point) {m_data.origin = point->Get ();}
    void SetDirection (JsDVector3dP vector) {m_data.direction = vector->Get ();}

    JsDPoint3dP PointAtFraction (double f)
        {
        return new JsDPoint3d (m_data.FractionParameterToPoint (f));
        }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDRAY3D_H_

