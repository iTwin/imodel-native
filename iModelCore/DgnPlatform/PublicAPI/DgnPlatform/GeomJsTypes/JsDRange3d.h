/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDRange3d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDRANGE3D_H_
#define _JSDRANGE3D_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDRange3d : JsGeomWrapperBase<DRange3d>
{
public:
    JsDRange3d() {m_data.Init ();}

    JsDPoint3dP GetLow() {return new JsDPoint3d (m_data.low);}
    JsDPoint3dP GetHigh() {return new JsDPoint3d (m_data.high);}
    void SetLow (JsDPoint3dP point) {m_data.low = point->Get ();}
    void SetHigh (JsDPoint3dP point) {m_data.high = point->Get ();}

    bool IsNull (){return m_data.IsNull ();}
    void Extend (double x, double y, double z){m_data.Extend (x,y,z);}
    void Extend (JsDPoint3dP point){m_data.Extend (point->Get ());}
    void Init (){m_data.Init ();}


};
END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDRANGE3D_H_

