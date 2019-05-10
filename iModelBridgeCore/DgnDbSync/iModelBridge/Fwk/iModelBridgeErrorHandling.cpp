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

struct CrashReportingConfig
    {
    BeFileName m_crashDir;
    bmap<Utf8String,Utf8String> m_params;
    size_t m_maxDumpsInDir;
    bool m_writeDumpsToCrashDir;
    bool m_wantFullMemory;
    bool m_needsVectorExceptionHandler;
    };

static bool s_initialized;
static CrashReportingConfig s_config;
static BeFileName s_logFileName;
static BeFileName s_dmpFileName;

static void writeMiniDump(EXCEPTION_POINTERS const* exceptionInfoP)
    {
    /*
    BeSQLite::BeGuid guid;
    guid.Create();
    WString wguid(guid.ToString().c_str(), true);
    s_dmpFileName.AppendToPath(WPrintfString(L"%ls.dmp", wguid.c_str()).c_str());
    */



    win32Tools_generateMiniDump(nullptr, exceptionInfoP, s_dmpFileName.c_str(), &s_config.m_wantFullMemory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
LONG WINAPI reportUnhandledException(struct _EXCEPTION_POINTERS *exceptionInfoP)
    {
    writeMiniDump(exceptionInfoP);
    return EXCEPTION_CONTINUE_SEARCH;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeErrorHandling::Initialize()
    {
    if (s_initialized)
        return;
    s_initialized = true;

    s_config.m_crashDir.SetNameA(getenv("TEMP"));
    s_config.m_crashDir.AppendToPath(L"Crash");

    s_logFileName.SetName(s_config.m_crashDir);
    s_logFileName.AppendToPath(L"exception.log");

    s_dmpFileName.SetName(s_config.m_crashDir);
    s_dmpFileName.AppendToPath(L"iModelBridge.dmp");

    s_config.m_wantFullMemory = true;
    s_config.m_writeDumpsToCrashDir = true;

    auto trap = getenv("IMODEL_BRIDGE_TRAP");
    if (trap != nullptr)
        {
        if (0 == stricmp(trap, "none"))
            s_config.m_writeDumpsToCrashDir = false;
        else if (0 == stricmp(trap, "smalldump"))
            s_config.m_wantFullMemory = false;
        }

    SetUnhandledExceptionFilter(reportUnhandledException);  // just in case anyone executes crashing code outside of our TRY/CATCH macros
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeErrorHandling::FilterException(EXCEPTION_POINTERS const* ptrs)
    {
    if (!s_initialized || !s_config.m_writeDumpsToCrashDir)
        return EXCEPTION_CONTINUE_SEARCH;

    win32Tools_resetFloatingPointExceptions(0);
    writeMiniDump(ptrs);
    return EXCEPTION_EXECUTE_HANDLER;   // This means that the bridge/fwk will NOT crash and Windows Error Reporting will NOT be invoked. Instead, the code with the TRY/CATCH macros will continue executing.
    }

