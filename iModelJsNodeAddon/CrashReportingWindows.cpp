/*--------------------------------------------------------------------------------------+
|
|     $Source: ECSchemaXmlContextUtils.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef _WIN32
    #error This file is for Windows only
#endif
#include <breakpad/client/windows/handler/exception_handler.h>
#include <breakpad/client/windows/crash_generation/crash_generation_client.h>
#include <breakpad/client/windows/crash_generation/crash_generation_server.h>
#include <breakpad/client/windows/crash_generation/client_info.h>
#include "iModelJsNative.h"

const DWORD MS_VC_EXCEPTION = 0x406D1388;
const DWORD MS_CPP_EXCEPTION = 0xE06D7363;

using namespace IModelJsNative;

static google_breakpad::ExceptionHandler* handler = nullptr;
static google_breakpad::CrashGenerationServer* crash_server = nullptr;
static std::wstring dump_path;

static const wchar_t kPipeName[] = L"\\\\.\\pipe\\imodeljs\\crashReporting\\server";

static google_breakpad::CustomInfoEntry kCustomInfoEntries[] = {
    google_breakpad::CustomInfoEntry(L"prod", L"@bentley/imodeljs"),
    google_breakpad::CustomInfoEntry(L"ver", L"1.0"),                       // TODO: addon version
};

static int s_dumpFinished = false;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void onDumpWritten(void* context, const google_breakpad::ClientInfo* client_info, const std::wstring* dump_path)
    {
    s_dumpFinished = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static LONG CALLBACK vectoredExceptionHandler(PEXCEPTION_POINTERS exceptionInfo)
    {
    // *** I would like to just call google_breakpad::ExceptionHandler::HandleException, but that is private.
    // *** So, I try to do effectively the same thing here.

    // recognize internal debugger events that are not really exceptions
    DWORD code = exceptionInfo->ExceptionRecord->ExceptionCode;

    if ((code == MS_VC_EXCEPTION) || (code == MS_CPP_EXCEPTION))
        return EXCEPTION_CONTINUE_SEARCH;

    bool is_debug_exception = (code == EXCEPTION_BREAKPOINT) ||
                              (code == EXCEPTION_SINGLE_STEP) ||
                              (code == DBG_PRINTEXCEPTION_C) ||
                              (code == DBG_PRINTEXCEPTION_WIDE_C);

    if (code == EXCEPTION_INVALID_HANDLE && handler->get_consume_invalid_handle_exceptions())
        {
        return EXCEPTION_CONTINUE_EXECUTION;
        }

    if (!is_debug_exception || handler->get_handle_debug_exceptions())
        {
        handler->WriteMinidumpForException(exceptionInfo);

        for (int i = 0; !s_dumpFinished && i < 100; ++i)
            {
            ::Sleep(100);
            }
        }

    return EXCEPTION_CONTINUE_SEARCH;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::InitializeCrashReporting(CrashReportingConfig const& cfg)
    {
    // Start the "server" -- this is a thread that writes out the minidumps
    BeFileName::CreateNewDirectory(cfg.m_crashDumpDir);

    dump_path = cfg.m_crashDumpDir.c_str();

    crash_server = new google_breakpad::CrashGenerationServer(kPipeName, nullptr, nullptr, nullptr, onDumpWritten, nullptr, nullptr, nullptr, nullptr, nullptr, true, &dump_path);
    if (!crash_server->Start())
        {
        fwprintf(stderr, L"Unable to start server\n");
        delete crash_server;
        crash_server = nullptr;
        return;
        }

    ::Sleep(1); // let server thread run and connect to the pipe

    // Register the exception-handler
    google_breakpad::CustomClientInfo custom_info = {kCustomInfoEntries, _countof(kCustomInfoEntries)};

    MINIDUMP_TYPE dumptype;

    if (cfg.m_wantFullMemory)
        dumptype = (MINIDUMP_TYPE)(MiniDumpWithFullMemory | MiniDumpWithFullMemoryInfo | MiniDumpWithHandleData | MiniDumpWithUnloadedModules | MiniDumpWithThreadInfo);
    else
        dumptype = (MINIDUMP_TYPE)(MiniDumpNormal | MiniDumpWithThreadInfo);

    handler = new google_breakpad::ExceptionHandler(cfg.m_crashDumpDir.c_str(), nullptr, nullptr, nullptr,
                                   google_breakpad::ExceptionHandler::HANDLER_ALL, dumptype, kPipeName, &custom_info);

    if (cfg.m_needsVectorExceptionHandler)
        {
        // breakpad's call to SetUnhandledExceptionFilter seems to be ignored when running in node.
        // Perhaps all or most exceptions are indeed "handled" in node/v8?
        // In any case, to work around this, we use a lower-level API to get a look at all(!) exceptions
        // and use that to trigger the breakpad crash-reporting logic.
        AddVectoredExceptionHandler(1, vectoredExceptionHandler);
        }
    }
