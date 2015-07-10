/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScriptContext/DgnScriptContextStubbedOut.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnScriptContext.h>
#include <DgnPlatform/DgnPlatformLib.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContext::ExecuteJavaScriptEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms)
    {
    return DgnDbStatus::NotEnabled;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptingAdmin::ScriptingAdmin()
    {
    m_jsenv = nullptr;
    m_dgnContext = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptingAdmin::~ScriptingAdmin()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BeJsEnvironmentR DgnPlatformLib::Host::ScriptingAdmin::GetBeJsEnvironment()
    {
    BeAssert(false);
    return *m_jsenv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnScriptContextR DgnPlatformLib::Host::ScriptingAdmin::GetDgnScriptContext()
    {
    BeAssert(false);
    return *m_dgnContext;
    }
