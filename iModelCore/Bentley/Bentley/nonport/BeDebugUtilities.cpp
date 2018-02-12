/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeDebugUtilities.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeDebugUtilities.h>

#include <Bentley/BeDebugLog.h>

USING_NAMESPACE_BENTLEY

#if defined(DEBUG) && defined(__APPLE__)

#include <execinfo.h>
#include <unistd.h>
#include <mach/mach.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeDebugUtilities::GetStackTraceDescription(size_t maxFrames)
    {
    Utf8String stackTrace;

    if (maxFrames < std::numeric_limits<size_t>::max())
        maxFrames += 1;

    auto stack = std::unique_ptr<void*>(new void*[maxFrames]);
    size_t framesCount = backtrace(stack.get(), maxFrames);
    char** stackTraceC = backtrace_symbols(stack.get(), framesCount);
    if (nullptr == stackTraceC)
        return stackTrace;

    for (size_t i = 1; i < framesCount; i++)
        {
        stackTrace += stackTraceC[i];
        stackTrace += "\n";
        }

    free(stackTraceC);

    return stackTrace;
    }
    
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeDebugUtilities::StackFrameInfo BeDebugUtilities::GetStackFrameInfoAt(size_t frameIndex)
    {
    return StackFrameInfo();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BeDebugUtilities::GetMemoryUsed()
    {
    struct task_basic_info info;
    mach_msg_type_number_t size = sizeof(info);
    kern_return_t status = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &size);
    if (status != KERN_SUCCESS)
        {
        BeDebugLog(Utf8PrintfString("BeDebugUtilities::GetMemoryUsed() error: %s", mach_error_string(status)).c_str());
        return 0;
        }
    return info.resident_size;
    }

#elif defined(DEBUG) && defined(BENTLEY_WIN32)

#define NOMINMAX
#define NOMSG
#include <Windows.h>

// 'typedef enum' with no variable declared
#pragma warning( push )
#pragma warning( disable : 4091)
#include <Dbghelp.h>
#pragma warning( pop )

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeDebugUtilities::GetStackTraceDescription(size_t maxFrames)
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

    for (USHORT i = 0; i < frames; i++)
        {
        auto symbolAddress = (DWORD64) (stack.get()[i]);

        module->ModuleName[0] = 0;
        symbol->Address = 0;
        symbol->Name[0] = 0;

        DWORD64 moduleAddress = SymGetModuleBase64(process, symbolAddress);
        SymGetModuleInfo64(process, moduleAddress, module);
        SymFromAddr(process, symbolAddress, 0, symbol);

        stackTrace += Utf8PrintfString("%-4d %-36s 0x%0X %s\n", i + 1, module->ModuleName, symbol->Address, symbol->Name);
        }

    free(symbol);
    free(module);

    return stackTrace;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                  
+---------------+---------------+---------------+---------------+---------------+------*/
BeDebugUtilities::StackFrameInfo BeDebugUtilities::GetStackFrameInfoAt(size_t frameIndex)
    {
    frameIndex += 1; // Skip this function

    void*           stack[1];
    unsigned short  frames;
    SYMBOL_INFO*    symbol;
    HANDLE          process;

    SymSetOptions(SYMOPT_LOAD_LINES);

    process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    frames = CaptureStackBackTrace((DWORD)frameIndex, 1, stack, NULL);
    symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO)+256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    IMAGEHLP_LINE line;

    SymFromAddr(process, (DWORD64)(stack[0]), 0, symbol);
    DWORD dwDisplacement;

    StackFrameInfo info;
    if (SymGetLineFromAddr(process, (DWORD64) (stack[0]), &dwDisplacement, &line))
        {
        info.functionName = symbol->Name;
        info.fileName = line.FileName;
        info.fileLine = line.LineNumber;
        }

    free(symbol);

    return info;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BeDebugUtilities::GetMemoryUsed()
    {
    return 0;
    }

#else

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeDebugUtilities::GetStackTraceDescription(size_t maxFrames)
    {
    return "";
    }
    
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeDebugUtilities::StackFrameInfo BeDebugUtilities::GetStackFrameInfoAt(size_t frameIndex)
    {
    return StackFrameInfo();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BeDebugUtilities::GetMemoryUsed()
    {
    return 0;
    }

#endif
