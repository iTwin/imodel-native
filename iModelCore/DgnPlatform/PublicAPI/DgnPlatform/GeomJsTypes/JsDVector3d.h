/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDVector3d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDVECTOR3D_H_
#define _JSDVECTOR3D_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE


//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDVector3d : JsGeomWrapperBase<DVec3d>
{
public:
    JsDVector3d() {m_data.Init(0,0,0);}
    JsDVector3d(DVec3dCR data) {m_data = data;}
    JsDVector3d(double x, double y, double z) {m_data.x=x; m_data.y=y; m_data.z=z;}
    double GetX() {return m_data.x;}
    double GetY() {return m_data.y;}
    double GetZ() {return m_data.z;}
    void SetX(double v) {m_data.x = v;}
    void SetY(double v) {m_data.y = v;}
    void SetZ(double v) {m_data.z = v;}
};




END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDVECTOR3D_H_

