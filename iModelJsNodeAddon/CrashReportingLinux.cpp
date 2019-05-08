/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/BentleyConfig.h>
#ifndef BENTLEYCONFIG_OS_LINUX
    #error This file is for Linux only
#endif
#include <breakpad/client/linux/handler/exception_handler.h>
#include <breakpad/client/linux/handler/minidump_descriptor.h>
#undef MAP_TYPE     // linux.h #define's this, while ECSchema.h uses it as a template parameter name
#include "IModelJsNative.h"

#ifdef COMMENT_OFF
static google_breakpad::ExceptionHandler* s_exceptionHandler = nullptr;
#endif
static bmap<Utf8String,Utf8String> s_customProperties;
static int s_nextNativeCrashTxtFileNo;

using namespace IModelJsNative;

#ifdef COMMENT_OFF

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void writeCustomPropertiesToFile(bmap<Utf8String,Utf8String> const& props, Utf8CP dmpFileName)
    {
    // preallocate space for the longest file name.
    static std::string paramsFile(PATH_MAX, ' ');
    paramsFile.assign(dmpFileName);
    paramsFile.append(".txt");

    char tbuf[128];
    JsInterop::FormatCurrentTime(tbuf, sizeof(tbuf));
    sys_write(log_fd, "CrashTime,", 10);
    sys_write(log_fd, tbuf, strlen(tbuf));
    sys_write(log_fd, "\n"), 1);

    const int kLogOpenFlags = O_CREAT | O_WRONLY | O_APPEND | O_CLOEXEC;
    int log_fd = sys_open(paramsFile.c_str(), kLogOpenFlags, 0600);
    if (log_fd > 0)
        {
        for (auto const& prop : props)
            {
            sys_write(log_fd, prop.first.c_str(), strlen(prop.first.c_str()));
            sys_write(log_fd, ",", 1);
            sys_write(log_fd, prop.second.c_str(), strlen(prop.second.c_str()));
            sys_write(log_fd, "\n"), 1);
            }
        sys_close(log_fd);
        }
    return;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static bool crashDone(const google_breakpad::MinidumpDescriptor& minidump, void* context, const bool succeeded)
    {
    writeCustomPropertiesToFile(s_customProperties, minidump.path());

    sys_write(2, minidump.path(), strlen(minidump.path()));
    return true;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::InitializeCrashReporting(CrashReportingConfig const& cfg)
    {
    MaintainCrashDumpDir(s_nextNativeCrashTxtFileNo, cfg);

    s_customProperties = GetCrashReportCustomProperties(cfg); 

#ifdef COMMENT_OFF
    google_breakpad::MinidumpDescriptor minidump_descriptor(Utf8String(cfg.m_crashDir).c_str());
    // minidump_descriptor.set_size_limit(kMaxMinidumpFileSize); // set a limit if servers impose one

    s_exceptionHandler = new google_breakpad::ExceptionHandler(minidump_descriptor, NULL, crashDone, nullptr, true, -1);
#endif
    }
