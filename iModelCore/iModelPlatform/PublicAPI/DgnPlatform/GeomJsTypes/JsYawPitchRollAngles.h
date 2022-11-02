/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSROLLPITCHYAWANGLES_H_
#define _JSROLLPITCHYAWANGLES_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct JsYawPitchRollAngles : RefCountedBase
{
private:
    YawPitchRollAngles m_angles;
public:    
    JsYawPitchRollAngles() {m_angles.FromDegrees(0,0,0);}
    JsYawPitchRollAngles(YawPitchRollAnglesCR angles) : m_angles(angles) {;}
    JsYawPitchRollAngles(double yaw, double pitch, double roll) : m_angles(YawPitchRollAngles::FromDegrees(yaw,pitch,roll)) {;}

    JsYawPitchRollAnglesP Clone () {return new JsYawPitchRollAngles (m_angles);}
    
    YawPitchRollAngles GetYawPitchRollAngles () { return m_angles;}
    double GetYawDegrees  () {return m_angles.GetYaw().Degrees();}
    double GetPitchDegrees() {return m_angles.GetPitch().Degrees();}
    double GetRollDegrees () {return m_angles.GetRoll().Degrees();}
    void SetYawDegrees  (double v) {m_angles.SetYaw   (AngleInDegrees::FromDegrees (v));}
    void SetPitchDegrees(double v) {m_angles.SetPitch (AngleInDegrees::FromDegrees (v));}
    void SetRollDegrees (double v) {m_angles.SetRoll  (AngleInDegrees::FromDegrees (v));}
};



END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSROLLPITCHYAWANGLES_H_

