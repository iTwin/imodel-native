/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsApi.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#ifndef _GEOM_JS_API_H_
#define _GEOM_JS_API_H_

#include <BeJavaScript/BeJavaScript.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Geom/DPoint3d.h>
#include <Geom/Angle.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDPoint3d : RefCountedBase
{
    DPoint3d m_pt;

    JsDPoint3d() {m_pt.Init(0,0,0);}
    JsDPoint3d(DPoint3dCR pt) : m_pt(pt) {;}
    JsDPoint3d(double x, double y, double z) {m_pt.x=x; m_pt.y=y; m_pt.z=z;}

    double GetX() {return m_pt.x;}
    double GetY() {return m_pt.y;}
    double GetZ() {return m_pt.z;}
    void SetX(double v) {m_pt.x = v;}
    void SetY(double v) {m_pt.y = v;}
    void SetZ(double v) {m_pt.z = v;}
};

typedef JsDPoint3d* JsDPoint3dP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsYawPitchRollAngles : RefCountedBase
{
    YawPitchRollAngles m_angles;

    JsYawPitchRollAngles() {m_angles.FromDegrees(0,0,0);}
    JsYawPitchRollAngles(YawPitchRollAnglesCR angles) : m_angles(angles) {;}
    JsYawPitchRollAngles(double yaw, double pitch, double roll) : m_angles(YawPitchRollAngles::FromDegrees(yaw,pitch,roll)) {;}

    double GetYaw  () {return m_angles.GetYaw().Degrees();}
    double GetPitch() {return m_angles.GetPitch().Degrees();}
    double GetRoll () {return m_angles.GetRoll().Degrees();}
    void SetYaw  (double v) {m_angles.FromDegrees(v, GetPitch(), GetRoll());}
    void SetPitch(double v) {m_angles.FromDegrees(GetYaw(), v, GetRoll());}
    void SetRoll (double v) {m_angles.FromDegrees(GetYaw(), GetPitch(), v);}
};

typedef JsYawPitchRollAngles* JsYawPitchRollAnglesP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct GeomJsApi : DgnPlatformLib::Host::ScriptAdmin::ScriptLibraryImporter
{
    BeJsContextR m_context;

    GeomJsApi(BeJsContextR);
    ~GeomJsApi();

    void _ImportScriptLibrary(BeJsContextR, Utf8CP) override;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _GEOM_JS_API_H_

