/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <functional>
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Logging/bentleylogging.h>

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  02/18
//=======================================================================================
struct C3dBridgeTestsLogProvider : BentleyApi::NativeLogging::Provider::ILogProvider
{
    typedef bool T_IsSeverityEnabled(BentleyApi::Utf8StringCR, BentleyApi::NativeLogging::SEVERITY);
    typedef void T_LogMessage(BentleyApi::Utf8StringCR, BentleyApi::NativeLogging::SEVERITY, BentleyApi::Utf8StringCR msg);

    std::function<T_IsSeverityEnabled> m_sev;
    std::function<T_LogMessage> m_proc;

    C3dBridgeTestsLogProvider()
        {
        m_sev = [](BentleyApi::Utf8StringCR, BentleyApi::NativeLogging::SEVERITY) {return false/*true*/;};
        m_proc = [](BentleyApi::Utf8StringCR ns, BentleyApi::NativeLogging::SEVERITY sev, BentleyApi::Utf8StringCR msg) {/*printf("[%d] %s - %s\n", (int)sev, ns.c_str(), msg.c_str());*/};
        }

    int STDCALL_ATTRIBUTE Initialize() override {return SUCCESS;}

    int STDCALL_ATTRIBUTE Uninitialize() override {return SUCCESS;}

    int STDCALL_ATTRIBUTE CreateLogger(BentleyApi::WCharCP nameSpace, BentleyApi::NativeLogging::Provider::ILogProviderContext** ppContext) override
        {
        *ppContext = reinterpret_cast<BentleyApi::NativeLogging::Provider::ILogProviderContext*>(new BentleyApi::WString(nameSpace));
        return SUCCESS;
        }

    int STDCALL_ATTRIBUTE DestroyLogger(BentleyApi::NativeLogging::Provider::ILogProviderContext* pContext) override
        {
        BentleyApi::WString* ns = reinterpret_cast<BentleyApi::WString*>(pContext);
        if(nullptr != ns)
            delete ns;
        return SUCCESS;
        }

    int STDCALL_ATTRIBUTE SetOption(BentleyApi::WCharCP attribName, BentleyApi::WCharCP attribValue) override {BeAssert(false); return SUCCESS;}

    int STDCALL_ATTRIBUTE GetOption(BentleyApi::WCharCP attribName, WCharP attribValue, uint32_t valueSize) override {return ERROR;}

    void STDCALL_ATTRIBUTE LogMessage(BentleyApi::NativeLogging::Provider::ILogProviderContext* context, BentleyApi::NativeLogging::SEVERITY sev, BentleyApi::WCharCP msg) override
        {
        LogMessage(context, sev, BentleyApi::Utf8String(msg).c_str());
        }

    int  STDCALL_ATTRIBUTE SetSeverity(BentleyApi::WCharCP nameSpace, BentleyApi::NativeLogging::SEVERITY severity) override
        {
        BeAssert(false && "only the app (in TypeScript) sets severities");
        return ERROR;
        }

    void STDCALL_ATTRIBUTE LogMessage(BentleyApi::NativeLogging::Provider::ILogProviderContext* context, BentleyApi::NativeLogging::SEVERITY sev, Utf8CP msg) override
        {
        BentleyApi::WString* ns = reinterpret_cast<BentleyApi::WString*>(context);
        m_proc(BentleyApi::Utf8String(*ns), sev, msg);
        }

    bool STDCALL_ATTRIBUTE IsSeverityEnabled(BentleyApi::NativeLogging::Provider::ILogProviderContext* context, BentleyApi::NativeLogging::SEVERITY sev) override
        {
        BentleyApi::WString* ns = reinterpret_cast<BentleyApi::WString*>(context);
        return m_sev(BentleyApi::Utf8String(*ns), sev);
        }

};  // C3dBridgeTestsLogProvider
