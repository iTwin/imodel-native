/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/nonport/winnt/DelayLoadHook.cpp $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

// Don't care about delay load on static libs.
#if defined (_DLL) && !defined (STANDALONE_BSI)

// #pragma optimize ("", off)

// The remaining purpose of having a delay load hook is largely diagnostic.
//  SHOW_DELAY_LOAD_CALLERS: Prints a message to the debugger output window the first time a loaded module causes another to load due to a delay load.
//      It will print the caller symbol name and module, as well as the symbol and module being delay loaded.
//      Normal output is in the form "####:#### @ ######## - DelayLoadHook - INFO001: <CallingSymbolName> in <CallingSymbolModule> caused module <DelayLoadedModule> to load for symbol <DelayLoadedSymbol>", where the preamble is process ID (0-padded hex), thread ID (0-padded hex), and tickcount (0-padded decimal), and angle brackets are kept.
//      SHOW_DELAY_LOAD_CALLERS_FULL_STACK can optionally be defined to also print the full stack trace for each call (as INFO002).
// #define SHOW_DELAY_LOAD_CALLERS
// #define SHOW_DELAY_LOAD_CALLERS_FULL_STACK

#ifdef SHOW_DELAY_LOAD_CALLERS
#define NOMINMAX
#include <windows.h>
#endif // SHOW_DELAY_LOAD_CALLERS

#include <delayimp.h>

#ifdef SHOW_DELAY_LOAD_CALLERS
#include <dbghelp.h>
#include <malloc.h>
#include <stdio.h>
#endif // SHOW_DELAY_LOAD_CALLERS

#ifdef SHOW_DELAY_LOAD_CALLERS

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     12/2010
//=======================================================================================
struct AutoSymCleanup
    {
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     12/2010
    //---------------------------------------------------------------------------------------
    public: ~AutoSymCleanup ()
        {
        typedef BOOL (WINAPI *SymCleanupFuncType) (__in HANDLE);
        SymCleanupFuncType SymCleanupFunc = (SymCleanupFuncType)::GetProcAddress (::GetModuleHandleW (L"dbghelp.dll"), "SymCleanup"); 
        SymCleanupFunc (::GetCurrentProcess ());
        }
    
    }; // AutoSymCleanup

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2010
//---------------------------------------------------------------------------------------
static void OutputPreableDebugString ()
    {
    DWORD   procId      = ::GetCurrentProcessId ();
    DWORD   threadId    = ::GetCurrentThreadId ();
    DWORD   tick        = ::GetTickCount ();
    
    wchar_t preamble[64];
    _snwprintf_s (preamble, 64, _TRUNCATE, L"%04x:%04x @ %08d - ", procId, threadId, tick);

    ::OutputDebugStringW (preamble);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2010
//---------------------------------------------------------------------------------------
static void recordCallerOfDelayLoad (char const* moduleName, char const* procName)
    {
    typedef DWORD   (WINAPI *SymSetOptionsFuncType)         (__in DWORD);
    typedef DWORD   (WINAPI *SymGetOptionsFuncType)         (void);
    typedef BOOL    (WINAPI *SymInitializeFuncType)         (__in HANDLE, __in PCTSTR, __in BOOL);
    typedef USHORT  (WINAPI *CaptureStackBackTraceFuncType) (__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);
    typedef BOOL    (WINAPI *SymGetSymFromAddr64FuncType)   (__in HANDLE, __in DWORD64, __out PDWORD64, __out PIMAGEHLP_SYMBOL64);
    typedef BOOL    (WINAPI *SymGetModuleInfo64FuncType)    (__in HANDLE, __in DWORD64, __out PIMAGEHLP_MODULE64);

    // We should not dictate what each DLL that uses us links with, so only rely on some basics like kernel32 (for LoadLibrary and GetProcAddress), and manually load anything else.
    
    ::LoadLibraryW (L"ntdll.dll");
    ::LoadLibraryW (L"dbghelp.dll");
    
    SymSetOptionsFuncType   SymSetOptionsFunc   = (SymSetOptionsFuncType)::GetProcAddress   (::GetModuleHandleW (L"dbghelp.dll"),   "SymSetOptions"); 
    SymGetOptionsFuncType   SymGetOptionsFunc   = (SymGetOptionsFuncType)::GetProcAddress   (::GetModuleHandleW (L"dbghelp.dll"),   "SymGetOptions"); 
    SymInitializeFuncType   SymInitializeFunc   = (SymInitializeFuncType)::GetProcAddress   (::GetModuleHandleW (L"dbghelp.dll"),   "SymInitialize"); 
    
    SymSetOptionsFunc (SymGetOptionsFunc () | SYMOPT_LOAD_LINES | SYMOPT_OMAP_FIND_NEAREST);
        
    if (TRUE != SymInitializeFunc (::GetCurrentProcess (), NULL, TRUE))
        {
        OutputPreableDebugString ();
        ::OutputDebugStringW (L"DelayLoadHook - ERROR001: Failed to initialize DbgHelp (SymInitialize failed).\n");
        return;
        }
    
    // Ensure we cleanup regardless of early return.
    AutoSymCleanup autoSymCleanup;

    CaptureStackBackTraceFuncType   CaptureStackBackTraceFunc   = (CaptureStackBackTraceFuncType)::GetProcAddress   (::GetModuleHandleW (L"ntdll.dll"),     "RtlCaptureStackBackTrace"); 
    SymGetSymFromAddr64FuncType     SymGetSymFromAddr64Func     = (SymGetSymFromAddr64FuncType)::GetProcAddress     (::GetModuleHandleW (L"dbghelp.dll"),   "SymGetSymFromAddr64"); 
    SymGetModuleInfo64FuncType      SymGetModuleInfo64Func      = (SymGetModuleInfo64FuncType)::GetProcAddress      (::GetModuleHandleW (L"dbghelp.dll"),   "SymGetModuleInfo64"); 

    // The typical delay load call stack goes like this:
    //  recordCallerOfDelayLoad (us)
    //  delayLoadNotifyHook     (our hook)
    //  __delayLoadHelper2      (boilerplate delay loader)
    //  __tailMerge_*           (more boilerplate, where * is the DLL being delay loaded)
    //  <caller>                (the callsite in the loaded DLL that caused the delay load to load)
    //  ...
    //  
    //  Therefore, we are only really interested in the 4th frame down.

    void*   addrs[64];
    USHORT  numAddrs    = CaptureStackBackTraceFunc (4, 1, addrs, NULL);
    
    if (0 == numAddrs)
        {
        OutputPreableDebugString ();
        ::OutputDebugStringW (L"DelayLoadHook - WARNING001: Failed to get the caller stack frame.\n");
        return;
        }
    
    PIMAGEHLP_SYMBOL64 symInfo = (PIMAGEHLP_SYMBOL64)_alloca (sizeof (IMAGEHLP_SYMBOL64) + (1023 * sizeof (TCHAR)));
    memset (symInfo, 0, sizeof (IMAGEHLP_SYMBOL64));
    symInfo->SizeOfStruct = sizeof (IMAGEHLP_SYMBOL64);
    symInfo->MaxNameLength = 1024;

    BOOL getSymResult = SymGetSymFromAddr64Func (::GetCurrentProcess (), (DWORD64)addrs[0], NULL, symInfo);
    if (TRUE != getSymResult)
        {
        OutputPreableDebugString ();
        ::OutputDebugStringW (L"DelayLoadHook - WARNING002: Failed to get symbol information from the caller stack frame.\n");
        return;
        }
    
    IMAGEHLP_MODULE64 modInfo;
    memset (&modInfo, 0, sizeof (modInfo));
    modInfo.SizeOfStruct = sizeof (modInfo);
    
    BOOL getModInfoResult = SymGetModuleInfo64Func (::GetCurrentProcess (), (DWORD64)addrs[0], &modInfo);
    if (TRUE != getModInfoResult)
        {
        OutputPreableDebugString ();
        ::OutputDebugStringW (L"DelayLoadHook - WARNING003: Failed to get module information from the caller stack frame.\n");
        return;
        }

    wchar_t delayModuleNameW[1024];     ::MultiByteToWideChar (CP_ACP, 0, moduleName, -1, delayModuleNameW, 1024);
    wchar_t delayProcNameW[1024];       ::MultiByteToWideChar (CP_ACP, 0, procName, -1, delayProcNameW, 1024);
    wchar_t callerModuleNameW[1024];    ::MultiByteToWideChar (CP_ACP, 0, modInfo.ModuleName, -1, callerModuleNameW, 1024);
    wchar_t callerProcNameW[1024];      ::MultiByteToWideChar (CP_ACP, 0, symInfo->Name, -1, callerProcNameW, 1024);
    
    OutputPreableDebugString ();
    ::OutputDebugStringW (L"DelayLoadHook - INFO001: <");
    ::OutputDebugStringW (callerProcNameW);
    ::OutputDebugStringW (L"> in <");
    ::OutputDebugStringW (callerModuleNameW);
    ::OutputDebugStringW (L"> caused module <");
    ::OutputDebugStringW (delayModuleNameW);
    ::OutputDebugStringW (L"> to load for symbol <");
    ::OutputDebugStringW (delayProcNameW);
    ::OutputDebugStringW (L">\n");

#ifdef SHOW_DELAY_LOAD_CALLERS_FULL_STACK

    numAddrs = CaptureStackBackTraceFunc (0, 64, addrs, NULL);

    for (USHORT iAddr = 0; iAddr < numAddrs; ++iAddr)
        {
        memset (symInfo, 0, sizeof (IMAGEHLP_SYMBOL64));
        symInfo->SizeOfStruct = sizeof (IMAGEHLP_SYMBOL64);
        symInfo->MaxNameLength = 1024;

        if (TRUE != SymGetSymFromAddr64Func (::GetCurrentProcess (), (DWORD64)addrs[iAddr], NULL, symInfo))
            {
            OutputPreableDebugString ();
            ::OutputDebugStringW (L"DelayLoadHook - WARNING002: Failed to get symbol information from the caller stack frame.\n");
            continue;
            }
    
        memset (&modInfo, 0, sizeof (modInfo));
        modInfo.SizeOfStruct = sizeof (modInfo);
    
        if (TRUE != SymGetModuleInfo64Func (::GetCurrentProcess (), (DWORD64)addrs[iAddr], &modInfo))
            {
            OutputPreableDebugString ();
            ::OutputDebugStringW (L"DelayLoadHook - WARNING003: Failed to get module information from the caller stack frame.\n");
            continue;
            }
        
        wchar_t callerModuleNameW[1024];    ::MultiByteToWideChar (CP_ACP, 0, modInfo.ModuleName, -1, callerModuleNameW, 1024);
        wchar_t callerProcNameW[1024];      ::MultiByteToWideChar (CP_ACP, 0, symInfo->Name, -1, callerProcNameW, 1024);
        
        OutputPreableDebugString ();
        ::OutputDebugStringW (L"DelayLoadHook - INFO002: ");
        ::OutputDebugStringW (callerModuleNameW);
        ::OutputDebugStringW (L"!");
        ::OutputDebugStringW (callerProcNameW);
        ::OutputDebugStringW (L"\n");
        }

#endif // SHOW_DELAY_LOAD_CALLERS_FULL_STACK
    }

#endif // SHOW_DELAY_LOAD_CALLERS

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Chuck.Kirschman 06/99
//---------------------------------------------------------------------------------------
extern "C" FARPROC WINAPI delayLoadNotifyHook (unsigned dliNotify, PDelayLoadInfo pdli)
    {
#ifdef SHOW_DELAY_LOAD_CALLERS
    if (dliNotePreLoadLibrary == dliNotify)
        {
        char* procName = (char*)_alloca (64 * sizeof (char));
            
        if (pdli->dlp.fImportByName)
            procName = const_cast<char*>(pdli->dlp.szProcName);
        else
            _snprintf_s (procName, 1024, _TRUNCATE, "Ordinal-0x%16x", pdli->dlp.dwOrdinal);
            
        recordCallerOfDelayLoad (pdli->szDll, procName);
        }
#endif // SHOW_DELAY_LOAD_CALLERS
        
    return NULL;
    }

// Assigning this global allows the boilerplate delay loader (see .../VC/include/delayhlp.cpp) to call our notification function.
PfnDliHook __pfnDliNotifyHook2 = delayLoadNotifyHook;

// #pragma optimize ("", on)

#endif // defined (_DLL) && !defined (STANDALONE_BSI) 
