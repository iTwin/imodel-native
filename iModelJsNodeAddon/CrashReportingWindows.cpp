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
#include <signal.h>
#include <breakpad/client/windows/handler/exception_handler.h>
#include <breakpad/client/windows/crash_generation/crash_generation_client.h>
#include <breakpad/client/windows/crash_generation/crash_generation_server.h>
#include <breakpad/client/windows/crash_generation/client_info.h>
#include <breakpad/client/windows/sender/crash_report_sender.h>
#include "iModelJsNative.h"
#include <Bentley/BeDirectoryIterator.h>

const DWORD MS_VC_EXCEPTION = 0x406D1388;
const DWORD MS_CPP_EXCEPTION = 0xE06D7363;

using namespace IModelJsNative;

static google_breakpad::ExceptionHandler* s_exceptionHandler = nullptr;
static google_breakpad::CrashGenerationServer* s_crashServer = nullptr;
static google_breakpad::CrashReportSender* s_crashSender = nullptr;
static std::map<std::wstring, std::wstring>* s_uploadAdditionalParameters = nullptr;
static std::map<std::wstring, std::wstring>* s_dumpFiles = nullptr;
static std::wstring s_crashFilesDir;
static JsInterop::CrashReportingConfig* s_config;
static bool s_dumpFinished = false;
static std::wstring s_uploadUrl;

static const wchar_t kPipeName[] = L"\\\\.\\pipe\\imodeljs\\crashReporting\\server";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void writeCustomPropertiesToFile(std::map<std::wstring, std::wstring> const& props, const std::wstring& dumpFileName)
    {
    // preallocate space for the longest file name.
    static std::wstring paramsFile(MAX_PATH, L' ');
    paramsFile.assign(dumpFileName.c_str());
    paramsFile.append(L".txt");
    FILE* fp = _wfopen(paramsFile.c_str(), L"w+");
    for (auto const& prop : props)
        {
        fwprintf(fp, L"%ls, %ls\n", prop.first.c_str(), prop.second.c_str());
        }
    fclose(fp);
    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void onDumpWritten(void* context, const google_breakpad::ClientInfo* client_info, const std::wstring* dmpFilePath)
    {
    s_dumpFinished = true;

    if (s_uploadUrl.empty())
        {
        writeCustomPropertiesToFile(*s_uploadAdditionalParameters, *dmpFilePath);
        return;
        }

    (*s_dumpFiles)[L"upload_file_minidump"].assign(*dmpFilePath);   // try not to allocate any memory!

    int nTries = 0;
    while(true)
        {
        google_breakpad::ReportResult res = s_crashSender->SendCrashReport(s_uploadUrl, *s_uploadAdditionalParameters, *s_dumpFiles, nullptr);

        if (google_breakpad::ReportResult::RESULT_THROTTLED == res)
            {
            break;
            }

        if (google_breakpad::ReportResult::RESULT_SUCCEEDED == res)
            {
            BeFileName::BeDeleteFile(dmpFilePath->c_str());
            break;
            }

        if (++nTries > s_config->m_maxUploadRetries)
            {
            break;
            }

        ::Sleep((DWORD) s_config->m_uploadRetryWaitInterval);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static LONG CALLBACK vectoredExceptionHandler(PEXCEPTION_POINTERS exceptionInfo)
    {
    static bool s_processingException;
    if (s_processingException)
        {
        return EXCEPTION_CONTINUE_SEARCH;
        }
    s_processingException = true;

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

    if (code == EXCEPTION_INVALID_HANDLE && s_exceptionHandler->get_consume_invalid_handle_exceptions())
        return EXCEPTION_CONTINUE_EXECUTION;

    if (!is_debug_exception || s_exceptionHandler->get_handle_debug_exceptions())
        {
        s_exceptionHandler->WriteMinidumpForException(exceptionInfo);

        for (int i = 0; !s_dumpFinished && i < 100; ++i)
            ::Sleep(100);
        }

    s_processingException = false;
    return EXCEPTION_CONTINUE_SEARCH;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void abortHandler(int signal)
  {
  RaiseException(0, 0, 0, NULL);
  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::InitializeCrashReporting(CrashReportingConfig const& cfg)
    {
    s_config = new CrashReportingConfig(cfg);

    s_uploadUrl = WString(cfg.m_uploadUrl.c_str(), true).c_str();

    // Start the "server" -- this is a thread that writes out the minidumps
    BeFileName::CreateNewDirectory(cfg.m_crashDumpDir);

    s_crashFilesDir = cfg.m_crashDumpDir.c_str();
    
    MaintainCrashDumpDir(cfg);

    s_crashServer = new google_breakpad::CrashGenerationServer(kPipeName, nullptr, nullptr, nullptr, onDumpWritten, nullptr, nullptr, nullptr, nullptr, nullptr, true, &s_crashFilesDir);
    if (!s_crashServer->Start())
        {
        fwprintf(stderr, L"Unable to start server\n");
        delete s_crashServer;
        s_crashServer = nullptr;
        return;
        }

    ::Sleep(1); // let server thread run and connect to the pipe

    // Create a helper class for the server to use to upload crash reports
    BeFileName checkpointFileName(cfg.m_crashDumpDir);
    checkpointFileName.AppendToPath(L"crashesSent.txt");
    
    s_crashSender = new google_breakpad::CrashReportSender(checkpointFileName.c_str());
    if (cfg.m_maxReportsPerDay > 0)
        s_crashSender->set_max_reports_per_day((int)cfg.m_maxReportsPerDay);

    s_dumpFiles = new std::map<std::wstring, std::wstring>();
    (*s_dumpFiles)[L"upload_file_minidump"] = std::wstring(MAX_PATH, ' ');  // preallocate space for the longest file name.
    
    s_uploadAdditionalParameters = new std::map<std::wstring, std::wstring>();
    
    for (auto& prop : GetCrashReportCustomProperties(cfg))
        (*s_uploadAdditionalParameters)[WString(prop.first.c_str(), true).c_str()] = WString(prop.second.c_str(), true).c_str();

    MINIDUMP_TYPE dumptype;

    if (cfg.m_wantFullMemory)
        dumptype = (MINIDUMP_TYPE)(MiniDumpWithFullMemory | MiniDumpWithFullMemoryInfo | MiniDumpWithHandleData | MiniDumpWithUnloadedModules | MiniDumpWithThreadInfo);
    else
        dumptype = (MINIDUMP_TYPE)(MiniDumpNormal | MiniDumpWithThreadInfo);

    s_exceptionHandler = new google_breakpad::ExceptionHandler(cfg.m_crashDumpDir.c_str(), nullptr, nullptr, nullptr,
                                   google_breakpad::ExceptionHandler::HANDLER_ALL, dumptype, kPipeName, nullptr);

    if (cfg.m_needsVectorExceptionHandler)
        {
        // breakpad's call to SetUnhandledExceptionFilter seems to be ignored when running in node.
        // Perhaps all or most exceptions are indeed "handled" in node/v8?
        // In any case, to work around this, we use a lower-level API to get a look at all(!) exceptions
        // and use that to trigger the breakpad crash-reporting logic.
        AddVectoredExceptionHandler(1, vectoredExceptionHandler);
        }

    signal(SIGABRT, abortHandler);
    }
