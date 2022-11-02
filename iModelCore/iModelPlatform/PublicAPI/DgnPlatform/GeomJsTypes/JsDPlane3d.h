/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDPLANE3d_H_
#define _JSDPLANE3d_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//=======================================================================================
// @bsiclass
//=======================================================================================
struct JsDPlane3d : JsGeomWrapperBase<DPlane3d>
{
public:
    JsDPlane3d() {}
    JsDPlane3d(DPlane3dCR data) {m_data = data;}
    // Note -- constructors and CreateXXX methods overlap.  The .d.ts file can decide which to expose.
    JsDPlane3d (DPoint3dCR point, DVec3dCR normal) {m_data = DPlane3d::FromOriginAndNormal (point, normal);}
    JsDPlane3d (JsDPoint3dP point, JsDVector3dP normal) {m_data = DPlane3d::FromOriginAndNormal (point->Get (), normal->Get ());}

    JsDPlane3d (double x0, double y0, double z0, double ux, double uy, double uz)
        {m_data = DPlane3d::FromOriginAndNormal (DPoint3d::From (x0, y0, z0), DVec3d::From (ux, uy, uz));}

    JsDPlane3dP Clone (){return new JsDPlane3d (m_data);}

    JsDPlane3dP CreateOriginAndNormal (JsDPoint3dP origin, JsDVector3dP normal)
        {
        return new JsDPlane3d (origin->Get (), normal->Get ());
        }

    JsDPlane3dP CreateOriginAndNormalXYZ (double ax, double ay, double az, double ux, double uy, double uz) 
        {
        return new JsDPlane3d (DPlane3d::FromOriginAndNormal (ax, ay, az, uz, uy,uz));
        }

    // READ/WRITE PROPERTIES Origin, Normal
    JsDPoint3dP GetOrigin() {return new JsDPoint3d (m_data.origin);}
    JsDVector3dP     GetNormal() {return new JsDVector3d (m_data.normal);}
    void SetOrigin (JsDPoint3dP point) {m_data.origin = point->Get ();}
    void SetNormal (JsDVector3dP normal) {m_data.normal = normal->Get ();}

    double Evaluate (JsDPoint3dP xyz)
        {
        return m_data.Evaluate (xyz->Get ());
        }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDPLANE3d_H_

