/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDPoint3d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDPOINT3D_H_
#define _JSDPOINT3D_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDPoint3d : RefCountedBase
{
friend struct JsDVector3d;
friend struct JsDEllipse3d;
friend struct JsDSegment3d;
friend struct JsCurvePrimitive;
friend struct JsCurveVector;
friend struct JsDVector3d;
friend struct JsDRay3d;
friend struct JsDPoint3dDVector3dDVector3d;

private:
    DPoint3d m_point;

public:   
    JsDPoint3d() {m_point.Init(0,0,0);}
    JsDPoint3d(DPoint3dCR pt) : m_point(pt) {;}
    JsDPoint3d(double x, double y, double z) {m_point.x=x; m_point.y=y; m_point.z=z;}
    DPoint3d GetDPoint3d (){return m_point;}

    double GetX() {return m_point.x;}
    double GetY() {return m_point.y;}
    double GetZ() {return m_point.z;}
    void SetX(double v) {m_point.x = v;}
    void SetY(double v) {m_point.y = v;}
    void SetZ(double v) {m_point.z = v;}
};
END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDPOINT3D_H_

