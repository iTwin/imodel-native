/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/DgnScriptStubbedOut.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnPlatformLib.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScript::ExecuteEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms)
    {
    return DgnDbStatus::NotEnabled;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::ScriptAdmin()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::~ScriptAdmin()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BeJsEnvironmentR DgnPlatformLib::Host::ScriptAdmin::GetBeJsEnvironment()
    {
    BeAssert(false);
        return *(BeJsEnvironmentP)(nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/16
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::_ThrowException(Utf8CP exname, Utf8CP details)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::_OnHostTermination(bool px)
    {
    delete this;
    }
