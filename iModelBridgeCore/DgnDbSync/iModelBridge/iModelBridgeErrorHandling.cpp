/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#if defined(_WIN32)
#include <windows.h>
// 'typedef enum' with no variable declared
#pragma warning( push )
#pragma warning( disable : 4091)
#include <Dbghelp.h>
#pragma warning( pop )
#elif defined(__linux)
#include <unistd.h>
#endif

#include <iModelBridge/iModelBridgeErrorHandling.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING

#undef min
#undef max

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridge"))

#ifdef _WIN32
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String iModelBridgeErrorHandling::GetStackTraceDescription(size_t maxFrames, size_t nIgnoreFrames)
    {
    // Solution adapted from https://stackoverflow.com/questions/5693192/win32-backtrace-from-c-code
    Utf8String stackTrace;

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, true);

    auto stack = std::unique_ptr<void*>(new void*[maxFrames]);
    USHORT frames = CaptureStackBackTrace(1, (DWORD)maxFrames, stack.get(), NULL);

    SYMBOL_INFO* symbol = (SYMBOL_INFO*) calloc(sizeof(PIMAGEHLP_MODULE64) + 256 * sizeof(char), 1);
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = 255;

    IMAGEHLP_MODULE64 * module = (IMAGEHLP_MODULE64 *) calloc(sizeof(IMAGEHLP_MODULE64), 1);
    module->SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

    for (size_t i = nIgnoreFrames; i < frames; i++)
        {
        auto symbolAddress = (DWORD64) (stack.get()[i]);

        module->ModuleName[0] = 0;
        symbol->Address = 0;
        symbol->Name[0] = 0;

        DWORD64 moduleAddress = SymGetModuleBase64(process, symbolAddress);
        SymGetModuleInfo64(process, moduleAddress, module);
        DWORD64 displacement;
        SymFromAddr(process, symbolAddress, &displacement, symbol);

        IMAGEHLP_LINE lineInfo{};
        DWORD dwDisplacement = (DWORD)displacement;
        SymGetLineFromAddr( GetCurrentProcess(), symbolAddress, &dwDisplacement, &lineInfo);

        stackTrace += Utf8PrintfString("%s %s %d\n", module->ModuleName, symbol->Name, (int)lineInfo.LineNumber);
        }

    free(symbol);
    free(module);

    return stackTrace;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
LONG WINAPI reportUnhandledException(struct _EXCEPTION_POINTERS *ExceptionInfo)
    {
    if (!ExceptionInfo || !ExceptionInfo->ExceptionRecord || !ExceptionInfo->ExceptionRecord->ExceptionCode)
        return EXCEPTION_CONTINUE_SEARCH;

    LONG code = ExceptionInfo->ExceptionRecord->ExceptionCode;
    if (STATUS_STACK_OVERFLOW == code)
        {
        // TODO: Can we at least get the name of the crashing function?
        fprintf(stderr, "Stack overflow\n");
        return EXCEPTION_CONTINUE_SEARCH;
        }

    if (EXCEPTION_BREAKPOINT == code)
        {	// this actually works, when you debug break explicitly
        DebugBreak();
        return EXCEPTION_EXECUTE_HANDLER;
        }

    LOG.errorv("Exception %lx", code);
    LOG.error(iModelBridgeErrorHandling::GetStackTraceDescription(20, 0).c_str());
    return EXCEPTION_CONTINUE_SEARCH;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeErrorHandling::Initialize()
    {
    static bool s_initialized;
    if (s_initialized)
        return;
    s_initialized = true;
    SetUnhandledExceptionFilter(reportUnhandledException);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeErrorHandling::ShouldProcessSEH() {
  return EXCEPTION_EXECUTE_HANDLER;
}

#else
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String iModelBridgeSacAdapter::GetStackTraceDescription(size_t maxFrames)
    {
    BeAssert(false && "TBD");
    return "";
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeErrorHandling::GetStackTraceDescriptionFixed(char* buf, size_t bufSize)
    {
    auto fullBacktrace = GetStackTraceDescription(20, 1);
    strncpy(buf, fullBacktrace.c_str(), bufSize-1);
    if ((bufSize-1) > fullBacktrace.size())
        buf[bufSize-1] = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeErrorHandling::LogStackTrace()
    {
    // LOG.error(GetStackTraceDescription(20, 1).c_str());
    }
