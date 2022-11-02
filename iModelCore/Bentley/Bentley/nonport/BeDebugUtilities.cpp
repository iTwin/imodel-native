/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeDebugUtilities.h>

#include <Bentley/BeDebugLog.h>

USING_NAMESPACE_BENTLEY

#if defined(DEBUG) && defined(__APPLE__)

#include <execinfo.h>
#include <unistd.h>
#include <mach/mach.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeDebugUtilities::StackFrameInfo BeDebugUtilities::GetStackFrameInfoAt(size_t frameIndex)
    {
    return StackFrameInfo();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<BeDebugUtilities::StackFrameInfo> BeDebugUtilities::GetStackFrameInfosAt(size_t frameIndex, size_t frameCount)
    {
    return {};
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
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
* @bsimethod
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
    _Analysis_assume_(nullptr != symbol);
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = 255;

    IMAGEHLP_MODULE64 * module = (IMAGEHLP_MODULE64 *) calloc(sizeof(IMAGEHLP_MODULE64), 1);
    _Analysis_assume_(nullptr != module);
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
    return std::move(GetStackFrameInfosAt(frameIndex + 1, 1)[0]);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<BeDebugUtilities::StackFrameInfo> BeDebugUtilities::GetStackFrameInfosAt(size_t frameIndex, size_t frameCount)
    {
    frameIndex += 1; // Skip this function

    auto stack = new void*[frameCount];

    SymSetOptions(SYMOPT_LOAD_LINES);

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    USHORT frames = CaptureStackBackTrace((DWORD)frameIndex, (DWORD)frameCount, stack, NULL);

    SYMBOL_INFO* symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    _Analysis_assume_(nullptr != symbol);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    std::vector<StackFrameInfo> infos;

    for (USHORT i = 0; i < frames && i < frameCount; i++)
        {
        StackFrameInfo info;

        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);

        DWORD dwDisplacement;
        IMAGEHLP_LINE line;
        if (SymGetLineFromAddr(process, (DWORD64)(stack[i]), &dwDisplacement, &line))
            {
            info.functionName = symbol->Name;
            info.fileName = line.FileName;
            info.fileLine = line.LineNumber;
            }
        infos.push_back(std::move(info));
        }

    free(symbol);
    delete[] stack;

    return std::move(infos);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BeDebugUtilities::GetMemoryUsed()
    {
    return 0;
    }

#else

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeDebugUtilities::GetStackTraceDescription(size_t maxFrames)
    {
    return "";
    }
    
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeDebugUtilities::StackFrameInfo BeDebugUtilities::GetStackFrameInfoAt(size_t frameIndex)
    {
    return StackFrameInfo();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<BeDebugUtilities::StackFrameInfo> BeDebugUtilities::GetStackFrameInfosAt(size_t frameIndex, size_t frameCount)
    {
    return {};
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BeDebugUtilities::GetMemoryUsed()
    {
    return 0;
    }

#endif
