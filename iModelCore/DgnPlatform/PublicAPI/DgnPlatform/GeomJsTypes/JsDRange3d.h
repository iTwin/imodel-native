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
struct JsDRange3d : RefCountedBase
{
    DRange3d m_range;

    JsDRange3d() {m_range.Init ();}

    JsDPoint3dP GetLow() {return new JsDPoint3d (m_range.low);}
    JsDPoint3dP GetHigh() {return new JsDPoint3d (m_range.high);}
    void SetLow (JsDPoint3dP point) {m_range.low = point->m_point;}
    void SetHigh (JsDPoint3dP point) {m_range.high = point->m_point;}

    bool IsNull (){return m_range.IsNull ();}
    void Extend (double x, double y, double z){m_range.Extend (x,y,z);}
    void Extend (JsDPoint3dP point){m_range.Extend (point->m_point);}
    void Init (){m_range.Init ();}


};
END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDRANGE3D_H_

