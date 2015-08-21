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
struct JsDPoint3d : JsGeomWrapperBase<DPoint3d>
{

public:   
    JsDPoint3d() {m_data.Init(0,0,0);}
    JsDPoint3d(DPoint3dCR data) {m_data = data;}
    JsDPoint3d(double x, double y, double z) {m_data.x=x; m_data.y=y; m_data.z=z;}

    double GetX() {return m_data.x;}
    double GetY() {return m_data.y;}
    double GetZ() {return m_data.z;}
    void SetX(double v) {m_data.x = v;}
    void SetY(double v) {m_data.y = v;}
    void SetZ(double v) {m_data.z = v;}
    JsDPoint3dP Interpolate(JsDPoint3dP left, double fraction, JsDPoint3dP right); 
    JsDPoint3dP AddPointAndVector(JsDPoint3dP point, JsDVector3dP vector);
    JsDPoint3dP SubtractPointAndVector(JsDPoint3dP point, JsDVector3dP vector);
    JsDPoint3dP AddPointAndScaledVector(JsDPoint3dP point, JsDVector3dP vector,double scalar);
    JsDPoint3dP AddPointAndTwoScaledVectors(JsDPoint3dP point, JsDVector3dP vectorA, double scalarA,  JsDVector3dP vectorB, double scalarB);
    JsDPoint3dP AddPointAndThreeScaledVectors(JsDPoint3dP point, JsDVector3dP vectorA, double scalarA,  JsDVector3dP vectorB, double scalarB, JsDVector3dP vectorC, double scalarC);
};
END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDPOINT3D_H_

