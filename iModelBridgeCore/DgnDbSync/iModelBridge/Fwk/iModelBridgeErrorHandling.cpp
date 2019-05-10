/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <windows.h>
#include "iModelBridgeErrorHandling.h"
#include <DgnPlatform/DesktopTools/w32tools.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING

#undef min
#undef max

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridge"))

static bool s_initialized;
static iModelBridgeErrorHandling::Config s_config;
static BeFileName s_logFileName;
static BeFileName s_dmpFileName;

static void writeMiniDump(EXCEPTION_POINTERS const* exceptionInfoP)
    {
    /* TODO:
    if (s_config.m_maxDumpsInDir > 0)
        {
        BeSQLite::BeGuid guid;
        guid.Create();
        WString wguid(guid.ToString().c_str(), true);
        s_dmpFileName.SetName(s_config.m_crashDir);
        s_dmpFileName.AppendToPath(L"iModelBridge-");
        s_dmpFileName.append(wguid);
        s_dmpFileName.append(L".dmp");
        }
    */

    /* TODO: 
        s_config.m_params => comment stream in .dmp file? Or part of JSON uploaded to service? 
    */

    win32Tools_generateMiniDump(nullptr, exceptionInfoP, s_dmpFileName.c_str(), &s_config.m_wantFullMemory);

    // TOOD:    send via HTTP to => s_config.m_uploadUrl
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
LONG WINAPI reportUnhandledException(struct _EXCEPTION_POINTERS *exceptionInfoP)
    {
    // this function is invoked only in the case of code executing outside the scope of our IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS/IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS macros
    writeMiniDump(exceptionInfoP);
    return EXCEPTION_CONTINUE_SEARCH;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeErrorHandling::Config::Config()
    {
    SetDefaults();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeErrorHandling::Config::SetDefaults()
    {
    m_maxDumpsInDir = 0;        // Always upload immediately and then delete.

    m_crashDir.SetNameA(getenv("TEMP"));
    m_crashDir.AppendToPath(L"Crash");

    m_wantFullMemory = false;
    m_writeDumpsToCrashDir = true;

    auto trap = getenv("IMODEL_BRIDGE_TRAP");
    if (trap != nullptr)
        {
        if (0 == stricmp(trap, "none"))
            m_writeDumpsToCrashDir = false;
        else if (0 == stricmp(trap, "full"))
            m_wantFullMemory = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeErrorHandling::Initialize(Config const& cfg)
    {
    if (s_initialized)
        return;
    s_initialized = true;

    s_config = cfg;

    // Set up as much as possible now, to avoid allocating memory when a crash occurs.
    s_logFileName.SetName(cfg.m_crashDir);
    s_logFileName.AppendToPath(L"exception.log");

    s_dmpFileName.SetName(cfg.m_crashDir);
    s_dmpFileName.AppendToPath(L"iModelBridge.dmp");

    /* TODO:
    if (s_config.m_maxDumpsInDir > 0)
        {
        ... remove files, so that we have room for at least one more ...
        }
    */

    SetUnhandledExceptionFilter(reportUnhandledException);  // just in case there is any code executing outside the scope of our IMODEL_BRIDGE_TRY_ALL_EXCEPTIONS/IMODEL_BRIDGE_CATCH_ALL_EXCEPTIONS macros
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeErrorHandling::FilterException(EXCEPTION_POINTERS const* ptrs)
    {
    if (!s_initialized || !s_config.m_writeDumpsToCrashDir)
        return EXCEPTION_CONTINUE_SEARCH;   // This means that the bridge/fwk WILL crash, and Windows Error Reporting WILL be invoked. The process will then be terminated.

    // NEEDS WORK: Crash processing should be done in another thread or another process. => breakpad

    win32Tools_resetFloatingPointExceptions(0);
    writeMiniDump(ptrs);
    return EXCEPTION_EXECUTE_HANDLER;   // This means that the bridge/fwk will NOT crash, and Windows Error Reporting will NOT be invoked. Instead, the code with the TRY/CATCH macros will continue executing.
    }

