/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
#ifdef WIP_DUMP_UPLOAD
#include <breakpad/client/windows/sender/crash_report_sender.h>
#endif
#include "iModelJsNative.h"
#include <Bentley/BeDirectoryIterator.h>

const DWORD MS_VC_EXCEPTION = 0x406D1388;
const DWORD MS_CPP_EXCEPTION = 0xE06D7363;

using namespace IModelJsNative;

static google_breakpad::ExceptionHandler* s_exceptionHandler = nullptr;
static google_breakpad::CrashGenerationServer* s_crashServer = nullptr;
static std::map<std::wstring, std::wstring>* s_customCrashProperties = nullptr;
static std::wstring s_crashFilesDir;
static JsInterop::CrashReportingConfig* s_config;
static bool s_dumpFinished = false;
static int s_nextNativeCrashTxtFileNo;
static int s_currentExceptionCode;
#ifdef WIP_DUMP_UPLOAD
static google_breakpad::CrashReportSender* s_crashSender = nullptr;
static std::map<std::wstring, std::wstring>* s_dumpFiles = nullptr;
static std::wstring s_uploadUrl;
#endif

static const wchar_t s_pipeName[] = L"\\\\.\\pipe\\imodeljs\\crashReporting\\server";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t findStartOfBasename(const std::wstring& dumpFileName)
    {
    auto iStartBasename = dumpFileName.rfind('/');
    if (iStartBasename != std::wstring::npos)
        return 1 + iStartBasename;

    iStartBasename = dumpFileName.rfind('\\');
    if (iStartBasename != std::wstring::npos)
        return 1 + iStartBasename;

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void writeCustomPropertiesFile(std::map<std::wstring, std::wstring> const& props, const std::wstring& dumpFileName)
    {
    // preallocate space for the longest file name.
    static std::wstring paramsFile(MAX_PATH, L' ');
    paramsFile.assign(dumpFileName.c_str());
    paramsFile.append(L".properties.txt");

    FILE* fp = _wfopen(paramsFile.c_str(), L"w+");

    char tbuf[128];
    JsInterop::FormatCurrentTime(tbuf, sizeof(tbuf));
    fprintf(fp, "CrashTime, %s\n", tbuf);

    fwprintf(fp, L"DumpFile, %ls\n", dumpFileName.c_str() + findStartOfBasename(dumpFileName));

    fprintf(fp, "ExceptionCode, %x\n", s_currentExceptionCode);

    for (auto const& prop : props)
        {
        fwprintf(fp, L"%ls, %ls\n", prop.first.c_str(), prop.second.c_str());
        }
    for (auto it : JsInterop::s_openDgnDbFileNames)
        {
        fwprintf(fp, L"DgnDb, \"%ls\"\n", it.second.c_str());
        }
    for (auto it : JsInterop::s_crashReportProperties)
        {
        fprintf(fp, "%s, %s\n", it.first.c_str(), it.second.c_str());
        }
    fclose(fp);
    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
static int safeStrCat(wchar_t* dest, wchar_t const* source, size_t& remainingDestCapacity)
    {
    auto slen = wcslen(source);
    if (slen >= remainingDestCapacity)
        return -1;

    wcscat(dest, source);
    remainingDestCapacity -= slen;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void runScript(wchar_t const* dumpFilename)
    {
    if (s_config->m_dumpProcessorScriptFileName.empty())
        return;
    
    static wchar_t cmd[2*MAX_PATH + 7]; // NB: avoid allocating memory
    size_t canCopy = _countof(cmd);
    cmd[0] = '\0';
    if (0 != safeStrCat(cmd, L"node ", canCopy))
        return;
    if (0 != safeStrCat(cmd, s_config->m_dumpProcessorScriptFileName.c_str(), canCopy))
        return;
    if (0 != safeStrCat(cmd, L" ", canCopy))
        return;
    if (0 != safeStrCat(cmd, dumpFilename, canCopy))
        return;

    _wsystem(cmd);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void writeCustomPropertiesFileAlone()
    {
    BeFileName dmpFileName(s_crashFilesDir.c_str());
    dmpFileName.AppendToPath(L"iModelJsNativeCrash-");
    wchar_t buf[32];
    dmpFileName.append(_itow(++s_nextNativeCrashTxtFileNo, buf, 10));
    writeCustomPropertiesFile(*s_customCrashProperties, dmpFileName.c_str());
    runScript(dmpFileName.c_str());
    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void onDumpWritten(void* context, const google_breakpad::ClientInfo* client_info, const std::wstring* dmpFilePath)
    {
    s_dumpFinished = true;

#ifdef WIP_DUMP_UPLOAD
    if (s_uploadUrl.empty())
        {
        writeCustomPropertiesFile(*s_customCrashProperties, *dmpFilePath);
        return;
        }

// Uploads have proved to be unreliable. This should be handled by another process.
    (*s_dumpFiles)[L"upload_file_minidump"].assign(*dmpFilePath);   // try not to allocate any memory!

    int nTries = 0;
    while(true)
        {
        google_breakpad::ReportResult res = s_crashSender->SendCrashReport(s_uploadUrl, *s_customCrashProperties, *s_dumpFiles, nullptr);

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
#else
    writeCustomPropertiesFile(*s_customCrashProperties, *dmpFilePath);
    runScript(dmpFilePath->c_str());
#endif
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

    s_currentExceptionCode = code;

    if (nullptr == s_exceptionHandler)
        {
        writeCustomPropertiesFileAlone();
        return EXCEPTION_CONTINUE_SEARCH;
        }

    if (code == EXCEPTION_INVALID_HANDLE && s_exceptionHandler->get_consume_invalid_handle_exceptions())
        return EXCEPTION_CONTINUE_EXECUTION;

    if (!is_debug_exception || s_exceptionHandler->get_handle_debug_exceptions())
        {
        s_exceptionHandler->WriteMinidumpForException(exceptionInfo);

        for (int i = 0; !s_dumpFinished && i < 100; ++i)
            ::Sleep(100);
        }

    return EXCEPTION_CONTINUE_SEARCH;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void abortHandler(int signal)
    {
    if (nullptr == s_exceptionHandler)
        {
        s_currentExceptionCode = 0;
        writeCustomPropertiesFileAlone();
        return;
        }
        
    RaiseException(0, 0, 0, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::InitializeCrashReporting(CrashReportingConfig const& cfg)
    {
    s_config = new CrashReportingConfig(cfg);

    // Start the "server" -- this is a thread that writes out the minidumps
    BeFileName::CreateNewDirectory(cfg.m_crashDir);

    s_crashFilesDir = cfg.m_crashDir.c_str();
    
    MaintainCrashDumpDir(s_nextNativeCrashTxtFileNo, cfg);

    if (cfg.m_enableCrashDumps)
        {
        s_crashServer = new google_breakpad::CrashGenerationServer(s_pipeName, nullptr, nullptr, nullptr, onDumpWritten, nullptr, nullptr, nullptr, nullptr, nullptr, true, &s_crashFilesDir);
        if (!s_crashServer->Start())
            {
            fwprintf(stderr, L"Unable to start server\n");
            delete s_crashServer;
            s_crashServer = nullptr;
            return;
            }

        ::Sleep(1); // let server thread run and connect to the pipe

        // Create a helper class for the server to use to upload crash reports
        BeFileName checkpointFileName(cfg.m_crashDir);
        checkpointFileName.AppendToPath(L"crashesSent.txt");
        
#ifdef WIP_DUMP_UPLOAD
        s_uploadUrl = WString(cfg.m_uploadUrl.c_str(), true).c_str();

        s_crashSender = new google_breakpad::CrashReportSender(checkpointFileName.c_str());
        if (cfg.m_maxReportsPerDay > 0)
            s_crashSender->set_max_reports_per_day((int)cfg.m_maxReportsPerDay);

        s_dumpFiles = new std::map<std::wstring, std::wstring>();
        (*s_dumpFiles)[L"upload_file_minidump"] = std::wstring(MAX_PATH, ' ');  // preallocate space for the longest file name.
#endif        
        MINIDUMP_TYPE dumptype;

        if (cfg.m_wantFullMemory)
            dumptype = (MINIDUMP_TYPE)(MiniDumpWithFullMemory | MiniDumpWithFullMemoryInfo | MiniDumpWithHandleData | MiniDumpWithUnloadedModules | MiniDumpWithThreadInfo);
        else
            dumptype = (MINIDUMP_TYPE)(MiniDumpNormal | MiniDumpWithThreadInfo);

        s_exceptionHandler = new google_breakpad::ExceptionHandler(cfg.m_crashDir.c_str(), nullptr, nullptr, nullptr,
                                    google_breakpad::ExceptionHandler::HANDLER_ALL, dumptype, s_pipeName, nullptr);
        }

    s_customCrashProperties = new std::map<std::wstring, std::wstring>();
    
    for (auto& prop : GetCrashReportCustomProperties(cfg))
        (*s_customCrashProperties)[WString(prop.first.c_str(), true).c_str()] = WString(prop.second.c_str(), true).c_str();

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
