/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <windows.h>
#include    <objbase.h>
#include    <excpt.h>
#include    <float.h>
#include    <errno.h>
#include    <fcntl.h>
#include    <stdlib.h>     // For strtoul, _makepath
#include    <malloc.h>
#include    <time.h>
#include    <io.h>
#include    <shlwapi.h>
#include    <psapi.h>

// ...\DbgHelp.h(1544): error C2220: warning treated as error - no 'object' file generated
// ...\DbgHelp.h(1544): warning C4091: 'typedef ': ignored on left of '' when no variable is declared
// ...\DbgHelp.h(3190): warning C4091: 'typedef ': ignored on left of '' when no variable is declared
#if defined (_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4091)
#endif

#include <dbghelp.h>

#if defined (_MSC_VER)
    #pragma warning(pop)
#endif

#include    <tlhelp32.h>
#include    <Winternl.h>
#include    <DgnPlatform/DesktopTools/ConfigurationManager.h>
typedef struct _LDR_MODULE {

    LIST_ENTRY              InLoadOrderModuleList;
    LIST_ENTRY              InMemoryOrderModuleList;
    LIST_ENTRY              InInitializationOrderModuleList;
    PVOID                   BaseAddress;
    PVOID                   EntryPoint;
    ULONG                   SizeOfImage;
    UNICODE_STRING          FullDllName;
    UNICODE_STRING          BaseDllName;
    ULONG                   Flags;
    SHORT                   LoadCount;
    SHORT                   TlsIndex;
    LIST_ENTRY              HashTableEntry;
    ULONG                   TimeDateStamp;

    } LDR_MODULE, *PLDR_MODULE;

extern "C"
    {
    extern  void *  _ReturnAddress (void);
    extern  void *  _AddressOfReturnAddress (void);
    }

#pragma intrinsic ( _ReturnAddress          )
#pragma intrinsic ( _AddressOfReturnAddress )


#undef BENTLEY_PORTABLE_CODE
#undef DGN_PLATFORM_MT
#define     INCLUDE_win32tools_recordDelayLoadHookFailure   (1)
#include    <DgnPlatform/DesktopTools/w32tools.h>

USING_NAMESPACE_BENTLEY_DGN

#pragma comment(lib, "VERSION")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "psapi")

#define ___ (0)


typedef struct delayloadhookfailure
    {
    bool          isValid;
    uint32_t      dliNotify;
    DelayLoadInfo   dli;
    } DelayLoadHookFailure;


void * _ReturnAddress(void);
#pragma intrinsic(_ReturnAddress)

typedef struct threadnamehistory
    {
    SYSTEMTIME  windowsTime;
    void      * pStartedFrom;
    uint32_t     dwThreadId;
    char        szThreadName[10];
    } ThreadNameHistory;


static DelayLoadHookFailure    g_DelayLoadHookFailure;
static FILE                   *g_stream;
static char const g_divider[] = "\n=================================================================\n";

static BeThreadLocalStorage     s_fpuMask;

#if defined (NEEDSWORK_DesktopPlaform_MoveToRmgrToolsIfNeeded)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    GeorgeDulchinos 1/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public int32_t win32Util_getCRuntimeMemFuncs
(
CRuntimeMemFuncs *memFuncsP
)
    {
#if defined (PAGALLOC_HAS_BEEN_REMOVED)
    char buf[256];

    if (GetEnvironmentVariable("MS_PAGALLOC", buf, sizeof(buf)))
        {
        memFuncsP->mallocFunc   = (CRuntimeMallocFunc)pagalloc_malloc;
        memFuncsP->callocFunc   = (CRuntimeCallocFunc)pagalloc_calloc;
        memFuncsP->reallocFunc  = (CRuntimeReallocFunc)pagalloc_realloc;
        memFuncsP->freeFunc     = (CRuntimeFreeFunc)pagalloc_free;
        }
    else
#endif
        {
        memFuncsP->mallocFunc   = (CRuntimeMallocFunc)malloc;
        memFuncsP->callocFunc   = (CRuntimeCallocFunc)calloc;
        memFuncsP->reallocFunc  = (CRuntimeReallocFunc)realloc;
        memFuncsP->freeFunc     = (CRuntimeFreeFunc)free;
        }

    return SUCCESS;
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    04/2015
//---------------------------------------------------------------------------------------
static void setThreadName
(
char const * const  sz9CharacterThreadName      // => Thread name, 9 characters + null max
)
    {
#define MS_VC_EXCEPTION 0x406D1388

#pragma pack(push,8)
    typedef struct tagTHREADNAME_INFO
        {
        uint32_t dwType; // Must be 0x1000.
        LPCSTR szName; // Pointer to name (in user addr space).
        uint32_t dwThreadID; // Thread ID (-1=caller thread).
        uint32_t dwFlags; // Reserved for future use, must be zero.
        } THREADNAME_INFO;
#pragma pack(pop)
    THREADNAME_INFO stn;
    char    sz9NameBuf[10];

    strncpy (sz9NameBuf, sz9CharacterThreadName, 9)[sizeof(sz9NameBuf) - 1] = 0;

    stn.dwType      = 0x1000;
    stn.szName      = sz9NameBuf;
    stn.dwThreadID  = -1;
    stn.dwFlags     = 0;

    __try
        {
        RaiseException (MS_VC_EXCEPTION, 0, sizeof(stn) / sizeof (ULONG_PTR), (ULONG_PTR*)&stn);
        }
    __except (EXCEPTION_CONTINUE_EXECUTION)
        {
        ;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MikeStratoti    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     toolSubsystem_setThreadName     // Set thread name for Visual Studio debugger.
(
char const * const  sz9CharacterThreadName      // => Thread name, 9 characters + null max
)
    {
    //  This is to get around a debugger bug.  When running a managed debugging session
    //  RaiseException does not save RBX. However, in an optimized build the caller 
    //  relies on RBX being saved and crashes when RBX is not saved. Therefore we
    //  use this code to explicitly save and restore the registers.
    CONTEXT ctxt = { 0 };
    ctxt.ContextFlags = CONTEXT_INTEGER;
    GetThreadContext(GetCurrentThread(), &ctxt);

    setThreadName(sz9CharacterThreadName);

    SetThreadContext(GetCurrentThread(), &ctxt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MichaelStratoti 12/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public WCharCP    win32Tools_exceptionToString (uint32_t exceptionCode)
    {
    static struct
        {
        int32_t   cErrno;
        uint32_t  excptNumber;
        WCharCP   excptString;
        }
    exceptionTable[]=
        {
            {EACCES, EXCEPTION_NONCONTINUABLE,         L"Can't continue."                },
            {EACCES, EXCEPTION_ACCESS_VIOLATION,       L"Access Violation"               },
            {EACCES, EXCEPTION_DATATYPE_MISALIGNMENT,  L"Datatype Misalignment"          },
            {EACCES, EXCEPTION_BREAKPOINT,             L"Breakpoint"                     },
            {EACCES, EXCEPTION_SINGLE_STEP,            L"Single Step"                    },
            {EACCES, EXCEPTION_ARRAY_BOUNDS_EXCEEDED,  L"Array Bounds Exceeded"          },
            {EACCES, EXCEPTION_FLT_DENORMAL_OPERAND,   L"Float Denormal Operand"         },
            {EDOM,   EXCEPTION_FLT_DIVIDE_BY_ZERO,     L"Float Divide By Zero"           },
            {EACCES, EXCEPTION_FLT_INEXACT_RESULT,     L"Float Inexact Result"           },
            {EACCES, EXCEPTION_FLT_INVALID_OPERATION,  L"Float Invalid Operation"        },
            {ERANGE, EXCEPTION_FLT_OVERFLOW,           L"Float Overflow"                 },
            {EACCES, EXCEPTION_FLT_STACK_CHECK,        L"Float Stack Check"              },
            {ERANGE, EXCEPTION_FLT_UNDERFLOW,          L"Float Underflow"                },
            {EDOM,   EXCEPTION_INT_DIVIDE_BY_ZERO,     L"Int Divide By Zero"             },
            {ERANGE, EXCEPTION_INT_OVERFLOW,           L"Int Overflow"                   },
            {EACCES, EXCEPTION_PRIV_INSTRUCTION,       L"Priv Instruction"               },
            {EINTR,  CONTROL_C_EXIT,                   L"Control C Exit"                 },
            {EACCES, STATUS_WAIT_0,                    L"Wait 0"                         },
            {EACCES, STATUS_ABANDONED_WAIT_0,          L"Abandoned Wait 0"               },
            {EACCES, STATUS_USER_APC,                  L"User APC"                       },
            {EACCES, STATUS_TIMEOUT,                   L"Timeout"                        },
            {EACCES, STATUS_PENDING,                   L"Pending"                        },
            {EACCES, STATUS_DATATYPE_MISALIGNMENT,     L"Datatype Misalignment"          },
            {EACCES, STATUS_BREAKPOINT,                L"Breakpoint"                     },
            {EACCES, STATUS_SINGLE_STEP,               L"Single Step"                    },
            {EACCES, STATUS_ACCESS_VIOLATION,          L"Access Violation"               },
            {EACCES, STATUS_IN_PAGE_ERROR,             L"In Page Error"                  },
            {EACCES, STATUS_ILLEGAL_INSTRUCTION,       L"Illegal Instruction"            },
            {EACCES, STATUS_NONCONTINUABLE_EXCEPTION,  L"Noncontinuable Exception"       },
            {ERANGE, STATUS_INVALID_DISPOSITION,       L"Invalid Disposition"            },
            {ERANGE, STATUS_ARRAY_BOUNDS_EXCEEDED,     L"Array Bounds Exceeded"          },
            {ERANGE, STATUS_FLOAT_DENORMAL_OPERAND,    L"Float Denormal Operand"         },
            {ERANGE, STATUS_FLOAT_DIVIDE_BY_ZERO,      L"Float Divide By Zero"           },
            {ERANGE, STATUS_FLOAT_INEXACT_RESULT,      L"Float Inexact Result"           },
            {ERANGE, STATUS_FLOAT_INVALID_OPERATION,   L"Float Invalid Operation"        },
            {ERANGE, STATUS_FLOAT_OVERFLOW,            L"Float Overflow"                 },
            {ERANGE, STATUS_FLOAT_STACK_CHECK,         L"Float Stack Check"              },
            {ERANGE, STATUS_FLOAT_UNDERFLOW,           L"Float Underflow"                },
            {ERANGE, STATUS_INTEGER_DIVIDE_BY_ZERO,    L"Integer Divide by Zero"         },
            {ERANGE, STATUS_INTEGER_OVERFLOW,          L"Integer Overflow"               },
            {EACCES, STATUS_PRIVILEGED_INSTRUCTION,    L"Privileged Instruction"         },
            {EACCES, STATUS_STACK_OVERFLOW,            L"Stack Overflow"                 },
            {EINTR,  STATUS_CONTROL_C_EXIT,            L"Control C Exit"                 },
            {EACCES, 0xc0000013,                       L"I/O error"                      },
            {EACCES, VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND),  L"Delay load module not found"  },
            {EACCES, VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND), L"Delay load function not found"},
            {EACCES, 0xe0434f4d,                       L"Exception from Managed Code"    }
        };

    for (int32_t i=0; i < (sizeof(exceptionTable)/sizeof(*exceptionTable)); i++)
        {
        if (exceptionCode == exceptionTable[i].excptNumber)
            {
            errno = exceptionTable[i].cErrno;
            return exceptionTable[i].excptString;
            }
        }

    return L"*Unknown*";
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    MikeStratoti                    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     win32tools_recordDelayLoadHookFailure
(
uint32_t        dliNotify,
PDelayLoadInfo  pdli
)
    {
    g_DelayLoadHookFailure.isValid   = true;
    g_DelayLoadHookFailure.dliNotify = dliNotify;
    g_DelayLoadHookFailure.dli       = *pdli;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    MikeStratoti                    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    win32Tools_printDelayLoadHookFailure
(
BeTextFile*                    stream
)
    {
    WChar           const *msg;
    uint32_t        const  notify = g_DelayLoadHookFailure.dliNotify;
    DelayLoadInfo   const  dli    = g_DelayLoadHookFailure.dli;

    if      (dliStartProcessing       == notify) msg = L"StartProcessing";
    else if (dliNotePreGetProcAddress == notify) msg = L"NotePreGetProcAddress";
    else if (dliFailLoadLib           == notify) msg = L"FailLoadLib";
    else if (dliFailGetProc           == notify) msg = L"FailGetProc";
    else if (dliNoteEndProcessing     == notify) msg = L"NoteEndProcessing";
    else                                         msg = L"Unknown";

    stream->PrintfTo (FALSE,  L"     notify: %d '%ls'\n", notify, msg);
    stream->PrintfTo (FALSE,  L"     dli: cb %d,  last error %d, hmodCur %#x\n", dli.cb, dli.dwLastError, dli.hmodCur, dli.szDll);
    stream->PrintfTo (FALSE,  L"          lib '%hs', ", dli.szDll);
    if (dli.dlp.fImportByName)
        stream->PrintfTo (FALSE,  L" function '%hs'", dli.dlp.szProcName);
    else
        stream->PrintfTo (FALSE,  L" oridinal %d/%#x", dli.dlp.dwOrdinal, dli.dlp.dwOrdinal);
    stream->PrintfTo (FALSE,  L"\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mike.Stratoti                   10/03
+---------------+---------------+---------------+---------------+---------------+------*/
void    win32Tools_generateMiniDump
(
BeTextFile*                      stream,
EXCEPTION_POINTERS const * const exceptionInfoP,
WCharCP                        dmpFileName
)
    {
    /*-----------------------------------------------------------------------------------
    The default MiniDumpNormal, writes out the file version information and the stack for each thread so you can walk the stack.

    MiniDumpWithDataSegs   will include all the data segments from all loaded modules so you can see module global variables.
    MiniDumpWithHandleData includes the operating system handle information for the process.
    MiniDumpWithFullMemory creates gigantic dump files because it includes all accessible memory in the process space.

    Of course, you can OR all of these flags together to get everything possible.
    After looking at quite a few dumps and analyzing what I felt was the best information,

    I like to default to MiniDumpNormal and MiniDumpWithHandleData together as that's the
    core information you generally need and produces small files.

    Set the config variable <MS_MiniDumpType> as follows:
    "none" turns off the dump
    4    is typical  (~64K bytes)
    5    is more     (~ 3 megabytes)
    7    is gigantic (10's of megabytes of binary dump)
    -----------------------------------------------------------------------------------*/

    typedef enum _MINIDUMP_TYPE
        {
        MiniDumpNormal                          = 0x0000,       // Include just the information necessary to capture stack traces for all existing threads in a process.
        MiniDumpWithDataSegs                    = 0x0001,       // Include the data sections from all loaded modules. This results in the inclusion of global variables, which can make the minidump file significantly larger.
        MiniDumpWithFullMemory                  = 0x0002,       // Include all accessible memory in the process. The raw memory data is included at the end, so that the initial structures can be mapped directly without the raw memory information. This option can result in a very large file.
        MiniDumpWithHandleData                  = 0x0004,       // Include high-level information about the operating system handles that are active when the minidump is made.  Windows Me/98/95:  This value is not supported.

        MiniDumpFilterMemory                    = 0x0008,       // Not useful -- Stack and backing store memory written to the minidump file should be filtered to remove all but the pointer values necessary to reconstruct a stack trace. Typically, this removes any private information.
        MiniDumpScanMemory                      = 0x0010,       // Not useful -- Stack and backing store memory should be scanned for pointer references to modules in the module list. If a module is referenced by stack or backing store memory, the ModuleWriteFlags member of the MINIDUMP_CALLBACK_OUTPUT structure is set to ModuleReferencedByMemory.

        MiniDumpWithUnloadedModules             = 0x0020,       // DbgHelp 5.1 and earlier:  This value is not supported.
        MiniDumpWithIndirectlyReferencedMemory  = 0x0040,       // DbgHelp 5.1 and earlier:  This value is not supported.
        MiniDumpFilterModulePaths               = 0x0080,       // DbgHelp 5.1 and earlier:  This value is not supported.
        MiniDumpWithProcessThreadData           = 0x0100,       // DbgHelp 5.1 and earlier:  This value is not supported.
        MiniDumpWithPrivateReadWriteMemory      = 0x0200        // DbgHelp 5.1 and earlier:  This value is not supported.
        } MINIDUMP_TYPE;


    /*-----------------------------------------------------------------------------------
    Attempt to load library in application directory first because WIN2K DBGHELP.DLL
    doesn't contain necessary entry point.
    -----------------------------------------------------------------------------------*/
    WChar                 szDbgHelpName[] = L"DBGHELP";                          // dbghelp.dll version 5.1 or greater is required
    HMODULE               hDbgHelp = NULL;
    WChar                 modulePath [MAX_PATH]=L"", exePath[MAX_PATH]=L"";
    WCharCP               szWitchDll;

    {
    WCharP szFileName = NULL;
    GetModuleFileNameW (___, exePath, MAX_PATH);
    GetFullPathNameW (exePath, MAX_PATH, exePath, &szFileName);
    if (szFileName)
        *szFileName = '\0';
    _wmakepath (modulePath, ___, exePath, szDbgHelpName,___);
    }

    hDbgHelp = LoadLibraryW (szWitchDll = modulePath);

    if ( ! hDbgHelp)                                                            // Try again if DLL can't be found in application directory.
        hDbgHelp = LoadLibraryW (szWitchDll = szDbgHelpName);

    if (! hDbgHelp)
        {
        if (NULL != stream)
            stream->PrintfTo (FALSE,  L"\n\nMissing \"%ls\" or from system. No dump generated.\n", modulePath);
        FreeLibrary (hDbgHelp);
        return;
        }
    
    typedef BOOL (__stdcall *FPMiniDumpWriteDump) (HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType, PMINIDUMP_EXCEPTION_INFORMATION, PVOID, PVOID);

    FPMiniDumpWriteDump const fpMiniDumpWriteDump = (FPMiniDumpWriteDump) GetProcAddress (hDbgHelp, "MiniDumpWriteDump");
    if (!fpMiniDumpWriteDump)
        {
        if (NULL != stream)
            stream->PrintfTo (FALSE,  L"\n\nMissing \"MiniDumpWriteDump\" API from \"%ls\". No dump generated.\n", szWitchDll);
        FreeLibrary (hDbgHelp);
        return;
        }
    
    uint32_t     dumpOpts = MiniDumpNormal | MiniDumpWithHandleData;
        {
        WString     miniDumpType;
        uint32_t    val = 0;

        if (SUCCESS == ConfigurationManager::GetVariable (miniDumpType, L"MS_MiniDumpType")
            &&       1 == swscanf (miniDumpType.c_str(), L"%x", &val))
            dumpOpts = val;

        if (miniDumpType.EqualsI (L"none"))
            {
            if (NULL != stream)
                stream->PrintfTo (FALSE,  L"\n\nSuppressed mini-dump generation.\n");
            FreeLibrary (hDbgHelp);
            return;
            }
        }
    
    HANDLE          hDumpFile = INVALID_HANDLE_VALUE;
    WCharP          szDumpFile = modulePath;                            // Reuse the buffer from above to minimize stack consumption
    if (NULL == dmpFileName)
        {
        WCharP szMS_TMP = exePath;                               // Reuse the buffer from above to minimize stack consumption
        wcscpy(szMS_TMP, T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName().GetName ());
        
        BeStringUtilities::Snwprintf(szDumpFile, MAX_PATH, L"%ls%ls", szMS_TMP, L"MiniDump.dmp");

        hDumpFile = ::CreateFileW (szDumpFile, GENERIC_WRITE, FILE_SHARE_READ, ___, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, ___);
        }
    else
        {
        hDumpFile = ::CreateFileW (dmpFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }

    if (INVALID_HANDLE_VALUE != hDumpFile)
        {
        MINIDUMP_EXCEPTION_INFORMATION xinfo = {GetCurrentThreadId(), (PEXCEPTION_POINTERS)exceptionInfoP, false};

        BOOL stat = fpMiniDumpWriteDump (GetCurrentProcess(), (uint32_t)GetCurrentProcessId(), hDumpFile, (MINIDUMP_TYPE)dumpOpts, &xinfo, ___, ___);
        if (NULL != stream)
            {
            if (stat)
                stream->PrintfTo (FALSE,  L"\n\nWrote a mini-dump type %#x to \"%ls\".\n", dumpOpts, szDumpFile);
            else
                stream->PrintfTo (FALSE,  L"\n\nFailed to write a mini-dump. GLE %#x.\n", GetLastError());
            }
        CloseHandle (hDumpFile);
        }
    
    FreeLibrary (hDbgHelp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void setFpuMask (uint32_t v) {s_fpuMask.SetValueAsInteger(v);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t getFpuMask () {return (uint32_t)s_fpuMask.GetValueAsInteger();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    GeorgeDulchinos                 7/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public uint32_t win32Tools_resetFloatingPointExceptions
(
uint32_t newFpuMask     /* => new excp. mask or 0 to use default or previously saved mask */
)
    {
    if (newFpuMask)
        {
        setFpuMask (newFpuMask);  /* if user specified, override */
        }
    else if (!getFpuMask())
        {
        /* one time initialization */
        WString temp;
        uint32_t defaultMask;

        if (SUCCESS == ConfigurationManager::GetVariable (temp, L"MS_FPUMASK") &&
            swscanf (temp.c_str(), L"%x", &defaultMask) && defaultMask)
            {
            printf ("Warning: FPU mask set to 0x%x.\n", defaultMask);
            }
        else
            {
            defaultMask = MCW_EM;

#if !defined (NDEBUG)
            /* --------------------------------------------------------------------
            For development and beta builds, we want to catch FPU exceptions, but .NET interferes.
            The only ones it allows are EM_OVERFLOW and EM_ZERODIVIDE
            NOTE: We tried it with EM_UNDERFLOW also allowed, but we get a lot of those for some reason.

            NOTE: When managed code that deals with Double.MaxValue (or equivalent) is JIT compiled,
            it will generate a floating-point overflow exception (EM_OVERFLOW) -- the runtime
            masks all floating point interrupt exceptions when it initializes, and does not
            expect the masking to change. Although masking this prevents potentially meaningful
            exceptions for native floating-point problems, not masking it causes MicroStation
            to crash. If you do not use affected managed code, you can set MS_FPUMASK=0x00080013,
            which is equivalent to the previous default of (MCW_EM & ~(EM_OVERFLOW | EM_ZERODIVIDE)).
            -------------------------------------------------------------------*/
            /* -------------------------------------------------------------------
            Setting the Ribbon as the default user interface for MicroStation, which uses 
            WPF & Telerik controls. These cause DivideByZero exceptions a lot internally, 
            so we can't include EM_ZERODIVIDE.
            //defaultMask = MCW_EM & ~(EM_ZERODIVIDE);
            -------------------------------------------------------------------*/
#endif
            }

        setFpuMask (defaultMask);
        }

    _clearfp();
    _fpreset();

    return _controlfp (getFpuMask(), MCW_EM);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    MikeStratoti                    02/100
+---------------+---------------+---------------+---------------+---------------+------*/
#if ! defined (VER_SUITENAME)
#   define VER_SUITE_TERMINAL   (0x00000010)
#   define VER_SUITENAME        (0x00000040)
#   define VER_AND              (         6)
#endif

    typedef struct _OSVERSIONINFOEXA_bsi {
        uint32_t dwOSVersionInfoSize;
        uint32_t dwMajorVersion;
        uint32_t dwMinorVersion;
        uint32_t dwBuildNumber;
        uint32_t dwPlatformId;
        CHAR  szCSDVersion[ 128 ];      // Maintenance string for PSS usage
        WORD  wServicePackMajor;
        WORD  wServicePackMinor;
        WORD  wSuiteMask;
        BYTE  wProductType;
        BYTE  wReserved;
    } OSVERSIONINFOEXA_bsi, *POSVERSIONINFOEXA_bsi, *LPOSVERSIONINFOEXA_bsi;

Public bool win32Tools_isRunningWindowsTerminalServer
(
void
)
    {
    uint32_t fVersionFlags = GetVersion();
    uint32_t wOSMajorVersion = LOBYTE(LOWORD(fVersionFlags));
    bool isRunningWinNT  = !(fVersionFlags & 0x80000000);
    bool isRunningWinNTTerminalServer = false;

    if ( ! isRunningWinNT)
        return false;

    /*-------------------------------------------------------------------
    Determine if we are running under Terminal Server on NT 4
    -------------------------------------------------------------------*/
    if (wOSMajorVersion == 4)
        {
        HKEY    hKey = NULL;
        int32_t  rtn;

        if (ERROR_SUCCESS == RegOpenKey (HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Control\\ProductOptions", &hKey))
            {
            DWORD type = 0;
            DWORD size = 0;
            if (ERROR_SUCCESS == RegQueryValueEx (hKey, "ProductSuite", ___, &type, ___, &size)    &&  size)
                {
                char*  productSuite;
                productSuite = (char*) alloca (size);
                if (productSuite)
                    {
                    rtn = RegQueryValueEx (hKey, "ProductSuite", ___, &type, (LPBYTE)productSuite, &size);
                    if (ERROR_SUCCESS == rtn  &&  REG_MULTI_SZ == type)
                        {
                        for (; *productSuite;  productSuite += strlen (productSuite)+1)
                            {
                            if ( ! strcmp (productSuite, "Terminal Server"))
                                {
                                isRunningWinNTTerminalServer = true;
                                break;
                                }
                            }
                        }
                    }
                }
            if (hKey)
                RegCloseKey (hKey);
            }
        }
    /*-------------------------------------------------------------------
    Determine if we are running under Terminal Server on NT 2000
    -------------------------------------------------------------------*/
    else
        if (wOSMajorVersion >= 5)            // Is it Windows 2000 (NT 5.0) or greater ?
            {
            // In Windows 2000 we need to use the Product Suite APIs
            // Don't static link because it won't load on non-Win2000 systems
            HMODULE const hmodNtDll = GetModuleHandle("ntdll.dll");
            if (hmodNtDll)
                {
                uint64_t             dwlConditionMask = 0;
                typedef LONGLONG     (WINAPI *PFnVerSetConditionMask)(ULONGLONG, DWORD, BYTE);
                typedef BOOL         (WINAPI *PFnVerifyVersionInfoA) (POSVERSIONINFOEXA_bsi, DWORD, DWORDLONG);
                PFnVerSetConditionMask  pfnVerSetConditionMask;
                PFnVerifyVersionInfoA   pfnVerifyVersionInfoA;

                pfnVerSetConditionMask = (PFnVerSetConditionMask) GetProcAddress (hmodNtDll, "VerSetConditionMask");
                if (pfnVerSetConditionMask)
                    {
                    HMODULE hmodK32 = NULL;
                    dwlConditionMask = (*pfnVerSetConditionMask) (dwlConditionMask, VER_SUITENAME, VER_AND);
                    hmodK32 = GetModuleHandle ("KERNEL32.DLL");
                    if (hmodK32)
                        {
                        pfnVerifyVersionInfoA = (PFnVerifyVersionInfoA)GetProcAddress (hmodK32, "VerifyVersionInfoA");
                        if (pfnVerifyVersionInfoA)
                            {
                            OSVERSIONINFOEXA_bsi    osVersionInfo;
                            memset (&osVersionInfo, 0, sizeof(osVersionInfo));
                            osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
                            osVersionInfo.wSuiteMask          = VER_SUITE_TERMINAL;
                            isRunningWinNTTerminalServer = ((*pfnVerifyVersionInfoA) (&osVersionInfo, VER_SUITENAME, dwlConditionMask) != 0); //conversion from WinAPI BOOL to C++ bool
                            }
                        }
                    }
                }
            }
        return isRunningWinNTTerminalServer;
    }



#pragma warning( disable : 4189)
/*----------------------------------------------------------------------+
| name          win32Tools_executeExternalProbe                         |
| author        MikeStratoti                            09/01           |
+----------------------------------------------------------------------*/
Public int      win32Tools_executeExternalProbe
(
BeTextFile*       stream,                           // => Append to this stream
WChar             * const szProbeExeName,           // => can't make it const * const because of CreateProcess
STARTUPINFOW const * const psiOption                 // => Optional STARTUPINFO for probe window visibility
)
    {
    BOOL                    ok = FALSE;
    HANDLE                  hStdoutChild =  INVALID_HANDLE_VALUE;               // OS handle of duped hStream
    STARTUPINFOW            si;


    /*-------------------------------------------------------------------
     Honor the optional STARTUPINFO.
    -------------------------------------------------------------------*/
    if (psiOption)
        {
        si = *psiOption;
        }
    else
        {
        memset (&si, 0, sizeof si);
        si.cb          = sizeof si;
        si.dwFlags     = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        }

    /*-------------------------------------------------------------------
     Get the OS level handle to the output stream
    -------------------------------------------------------------------*/
    if (stream)
        {
#if defined NOT_IMPLEMENTED
        HANDLE      hStream  =  INVALID_HANDLE_VALUE;                           // OS handdle for the fopen'ed stream

        fflush (fpStream);
        hStream = (HANDLE) _get_osfhandle (_fileno (fpStream));

        /*-------------------------------------------------------------------
         Create a duplicate of the output write handle for the std error write handle.
         This is necessary in case the child application closes one of its std output handles
         we don't want ours to close, too.
        -------------------------------------------------------------------*/
        HANDLE  const  currProc = GetCurrentProcess ();
        ok = DuplicateHandle (currProc, hStream, currProc,  &hStdoutChild, ___, TRUE, DUPLICATE_SAME_ACCESS);
        if ( ! ok )
            return  -1;

        /*-------------------------------------------------------------------
         Make child process use this app's standard files.
        -------------------------------------------------------------------*/
        si.dwFlags   |= STARTF_USESTDHANDLES;
        si.hStdInput  = hStdoutChild;
        si.hStdOutput = hStdoutChild;
        si.hStdError  = hStdoutChild;
#endif
        }

    {
    SECURITY_ATTRIBUTES     sa;
    PROCESS_INFORMATION     pi;
    memset (&pi, 0, sizeof pi);

    /*-------------------------------------------------------------------
     Set up the security attributes struct so that the child can inherit our file handles.
    -------------------------------------------------------------------*/
    memset (&sa, 0, sizeof sa);
    sa.nLength              = sizeof sa;
    sa.bInheritHandle       = TRUE;

    ok = CreateProcessW (___, szProbeExeName, ___, ___, TRUE, DETACHED_PROCESS , ___, ___, &si, &pi);
    if (ok)
        {
        WaitForSingleObject (pi.hProcess, 3 * (60 * 1000) );                    // 3 min timeout
        CloseHandle (pi.hProcess);
        CloseHandle (pi.hThread);
        }
    }
    if (INVALID_HANDLE_VALUE != hStdoutChild)
        CloseHandle (hStdoutChild);
    return  ok ? 0 : -3;
    }



/*----------------------------------------------------------------------+
| name          win32tools_processBSIExceptionLog                       |
| author        Mike.Stratoti                           10/03           |
+----------------------------------------------------------------------*/
DGNPLATFORM_EXPORT int32_t  win32tools_processBSIExceptionLog        // WIP - Must implement
(
BeTextFile*             stream,                                 // => Optional
WChar   const * const   szDumpFile                              // => Optional
)
    {
    WChar   cmdBuf[2048] = L"ProcessBSIExceptionLogs";
    WChar  *cmd = cmdBuf;
    int32_t rtn = -1;
    WString cfgVariable;

//    __try
        {
        STARTUPINFOW     si, *psi = NULL;
        /*-------------------------------------------------------------------------------
         Get an optional external override.  THe config var <MS_ProcessExceptionLog> can
         assume 2 forms:
                        <program_name> <arguments>  full-path-to-dumpfile
            [<swValue>] <program_name> <arguments>  full-path-to-dumpfile

         Where <swValue> is passed as the STARTUPINFO::wShowWindow value
        -------------------------------------------------------------------------------*/
        ConfigurationManager::GetVariable (cfgVariable, L"MS_PROCESSEXCEPTIONLOG");
        if (!cfgVariable.empty())
            {
            DWORD     swFlags=0;
            WChar    * pDigitsEnd = NULL;

            BeStringUtilities::Wcsncpy(cmdBuf, (sizeof cmdBuf/sizeof WChar), cfgVariable.c_str(), _TRUNCATE);

            // If there is a leading hex number then assume that it is one of the SW_xxx constants.
            swFlags = wcstoul (cmd, &pDigitsEnd, 16);
            if (cmd != pDigitsEnd)
                {
                memset (&si, 0, sizeof si);
                si.cb          = sizeof si;
                si.dwFlags     = STARTF_USESHOWWINDOW;
                si.wShowWindow = (WORD) swFlags;
                psi = &si;
    
                cmd = pDigitsEnd;               // skip white space after window show value
                while (*cmd && isspace (*cmd))
                    cmd++;

                }
            }

        // Append a space and the dump file.  Enquote the dump filename
        if (szDumpFile)
            {
            wcscat (cmd, L" ");
            if ('"' == *szDumpFile)
                wcscat (cmd, szDumpFile);
            else
                {
                wcscat (cmd, L" \"");
                wcscat (cmd, szDumpFile);
                wcscat (cmd, L"\"");
                }
            }

        rtn = win32Tools_executeExternalProbe (stream, cmd, psi);
        }
//    __except (EXCEPTION_EXECUTE_HANDLER) { ; }

    return rtn;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                    Peter.Segal                     03/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void    win32Tools_displayMemoryStats (BeTextFile* stream)
    {
    MEMORYSTATUSEX              stat = { sizeof stat };
    PROCESS_MEMORY_COUNTERS     pmc;

    HANDLE const    hProc    = GetCurrentProcess ();

    stream->PrintfTo (FALSE,  L"\nMemory Statistics for process ID %d:\n\n", GetCurrentProcessId());

    if (GlobalMemoryStatusEx (&stat))
        {
        stream->PrintfTo (FALSE,  L"    %10I64u MB Total Phys\n",                 stat.ullTotalPhys >> 20);
        stream->PrintfTo (FALSE,  L"    %10I64u MB Avail Phys\n",                 stat.ullAvailPhys >> 20); 
        stream->PrintfTo (FALSE,  L"    %10I64u MB Total Page File\n",            stat.ullTotalPageFile >> 20);
        stream->PrintfTo (FALSE,  L"    %10I64u MB Avail Page File\n",            stat.ullAvailPageFile >> 20);
        stream->PrintfTo (FALSE,  L"    %10I64u MB Total Virtual\n",              stat.ullTotalVirtual >> 20);
        stream->PrintfTo (FALSE,  L"    %10I64u MB Avail Virtual\n",              stat.ullAvailVirtual >> 20);
        stream->PrintfTo (FALSE,  L"    %10I64u MB Avail Extended Virtual\n\n",   stat.ullAvailExtendedVirtual >> 20);
        }

    if (GetProcessMemoryInfo (hProc, &pmc, sizeof pmc))
        {
        stream->PrintfTo (FALSE,  L"    %10u MB PeakWorkingSetSize\n",            pmc.PeakWorkingSetSize >> 20);
        stream->PrintfTo (FALSE,  L"    %10u MB WorkingSetSize\n",                pmc.WorkingSetSize >> 20);
        stream->PrintfTo (FALSE,  L"    %10u MB PagefileUsage\n",                 pmc.PagefileUsage >> 20); 
        stream->PrintfTo (FALSE,  L"    %10u MB PeakPagefileUsage\n\n",           pmc.PeakPagefileUsage >> 20);

        stream->PrintfTo (FALSE,  L"       %10u PageFaultCount\n",                pmc.PageFaultCount);
        stream->PrintfTo (FALSE,  L"       %10u QuotaPeakPagedPoolUsage\n",       pmc.QuotaPeakPagedPoolUsage);
        stream->PrintfTo (FALSE,  L"       %10u QuotaPagedPoolUsage\n",           pmc.QuotaPagedPoolUsage);
        stream->PrintfTo (FALSE,  L"       %10u QuotaPeakNonPagedPoolUsage\n",    pmc.QuotaPeakNonPagedPoolUsage);
        stream->PrintfTo (FALSE,  L"       %10u QuotaNonPagedPoolUsage\n\n",      pmc.QuotaNonPagedPoolUsage);
        }

    stream->PrintfTo (FALSE,  L"       %10u GDI Objects\n",                       GetGuiResources (hProc, GR_GDIOBJECTS));
    stream->PrintfTo (FALSE,  L"       %10u USER Objects\n",                      GetGuiResources (hProc, GR_USEROBJECTS));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    PhilipMcGraw                    03/2002
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     win32Tools_getStackInfo
(
PBYTE  *outBaseP,       // <=
PBYTE  *outEndP,        // <=
SIZE_T *outSizeP,       // <=
PBYTE   espRegister     // =>
)
    {
    MEMORY_BASIC_INFORMATION mbi;
    PBYTE stackBase = 0;
    PBYTE stackEnd  = 0;

    /* find stack base and end */
    memset(&mbi, 0, sizeof mbi);
    VirtualQuery ((void *)espRegister, &mbi, sizeof mbi);
    stackBase = (PBYTE)mbi.AllocationBase;
    stackEnd  = (PBYTE)mbi.BaseAddress + mbi.RegionSize;

    if (outBaseP)
        *outBaseP = stackBase;
    if (outEndP)
        *outEndP  = stackEnd;
    if (outSizeP)
        *outSizeP = (SIZE_T)(stackEnd - stackBase);
    }
   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    GeorgeDulchinos 10/12
+---------------+---------------+---------------+---------------+---------------+------*/
#if !defined (_WIN64)
void    dumpRegistersX86
(
BeTextFile*     stream,
CONTEXT&        regs
)
    {
        WChar       const invalid[] = L" (may not be valid):\n";
        WChar       const colon[]   = L":\n";

        stream->PrintfTo (FALSE,  L"\nMachine Registers:\n");
        stream->PrintfTo (FALSE,  L"\nData Registers%ls",                             (CONTEXT_INTEGER & regs.ContextFlags) ? colon : invalid);
        stream->PrintfTo (FALSE,  L" EAX: %08lx    EBX: %08lx    ECX: %08lx\n",       regs.Eax, regs.Ebx, regs.Ecx);
        stream->PrintfTo (FALSE,  L" EDX: %08lx    EDI: %08lx    ESI: %08lx\n",       regs.Edx, regs.Edi, regs.Esi);

        stream->PrintfTo (FALSE,  L"\nControl Registers%ls",                          (CONTEXT_CONTROL & regs.ContextFlags) ? colon : invalid);
        stream->PrintfTo (FALSE,  L" EBP: %08lx    Flags: %08lx\n",                   regs.Ebp, regs.EFlags);
        stream->PrintfTo (FALSE,  L" CS:EIP  %02lx:%08lx    SS:ESP  %02lx:%08lx\n",   regs.SegCs, regs.Eip,       regs.SegSs, regs.Esp);

        PBYTE       stackBase=0, stackEnd=0;
        SIZE_T      stackSize=0, stackUsed=0;
        win32Tools_getStackInfo (&stackBase, &stackEnd, &stackSize, (PBYTE)regs.Esp);
        if (stackSize)
            stackUsed = (100 * (stackEnd - (PBYTE)regs.Esp)) / stackSize;
        stream->PrintfTo (FALSE,  L" Stack range:(%08lx-%08lx) Stack usage: %lu%%\n", stackBase, stackEnd, stackUsed);

        stream->PrintfTo (FALSE,  L"\nDebug Registers%ls",                            (CONTEXT_DEBUG_REGISTERS & regs.ContextFlags) ? colon : invalid);

        stream->PrintfTo (FALSE,  L" Dr0: %08lx   Dr1: %08lx   Dr2: %08lx\n",         regs.Dr0, regs.Dr1, regs.Dr2);
        stream->PrintfTo (FALSE,  L" Dr3: %08lx   Dr6: %08lx   Dr7: %08lx\n",         regs.Dr3, regs.Dr6, regs.Dr7);

        stream->PrintfTo (FALSE,  L"\nFloating Point Registers%ls",                    (CONTEXT_FLOATING_POINT & regs.ContextFlags) ? colon : invalid);

        stream->PrintfTo (FALSE,  L" Control: %04x   Status: %04x   Tag: %04x\n",  regs.FloatSave.ControlWord & 0x0000ffff,
                                                                         regs.FloatSave.StatusWord  & 0x0000ffff,
                                                                         regs.FloatSave.TagWord     & 0x0000ffff);

        stream->PrintfTo (FALSE,  L" Last FPU operation address: %02lx:%08lx    Data: %02lx:%08lx\n",
            regs.FloatSave.ErrorSelector & 0x0000ffff, regs.FloatSave.ErrorOffset,
            regs.FloatSave.DataSelector  & 0x0000ffff, regs.FloatSave.DataOffset);

        stream->PrintfTo (FALSE,  L"  Register Area:");
        int i;
        for (i=0; i < SIZE_OF_80387_REGISTERS; i++)
            {
            if (0 == (i%20) )
                stream->PrintfTo (FALSE,   L"\n     ");
            stream->PrintfTo (FALSE,  L" %02x", regs.FloatSave.RegisterArea[i]);
            }
        stream->PrintfTo (FALSE,   L"\n");

        stream->PrintfTo (FALSE,  L"\nSegment Registers%ls", (CONTEXT_SEGMENTS & regs.ContextFlags) ? colon : invalid);
        stream->PrintfTo (FALSE,  L" DS: %02lx    ES: %02lx    FS: %02lx    GS: %02lx\n", regs.SegDs, regs.SegEs, regs.SegFs, regs.SegGs);
    }
#endif  // !defined (_WIN64)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static void    invokeManagedStackDump (BeTextFile* stream)
    {
    char            const managedWalkerDLL [] = "Assemblies\\Bentley.MicroStation.General.dll";
    char                  moduleName [MAX_PATH]="", exePath[MAX_PATH]="";
    HMODULE         generalLib;
    void    (*dumper)(BeTextFilePtr stream) = NULL;

    char *szFileName = NULL;
    GetModuleFileName (___, exePath, MAX_PATH);
    GetFullPathName (exePath, MAX_PATH, exePath, &szFileName);
    if (szFileName)
        *szFileName = '\0';
    _makepath (moduleName, ___, exePath, managedWalkerDLL,___);

    generalLib = LoadLibrary (moduleName);
        
    if (NULL != generalLib)
        dumper = (void (*)(BeTextFilePtr stream))GetProcAddress (generalLib, "ManagedStackWalker_dumpToFile");
        
    if (NULL != dumper)
        {
        BeTextFilePtr streamPtr (stream);
        dumper (streamPtr);
        }
    }



#if defined (NEEDSWORK_Exception_Logging)
static BeTextFile* g_logStream;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void __stdcall logCallback (LPCTSTR pMsg)
    {
    if (g_logStream)
        g_logStream->PrintfTo (FALSE, L"%s", pMsg );
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    MichaelStratoti                 12/92  
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void    win32Tools_dumpExceptionCallStack           // WIP - Must implement
(
BeTextFile*                      stream,
EXCEPTION_POINTERS const * const exceptionInfoP,
int32_t                    const debugLevel
)
    {
    CONTEXT      regs;
    static bool dejavu;

    if (dejavu)
        return;

    dejavu = true;

    memset (&regs, 0, sizeof regs);
    PBYTE stackPointer = 0; 

    if (exceptionInfoP  &&  exceptionInfoP->ExceptionRecord)
        {
        /*-------------------------------------------------------------------
         Display Exception Text
        -------------------------------------------------------------------*/
        __try 
            {  
            stream->PrintfTo (FALSE,  L"\nException String:  '%ls'\n", win32Tools_exceptionToString (exceptionInfoP->ExceptionRecord->ExceptionCode) );
            }    
        __except (EXCEPTION_EXECUTE_HANDLER) { ; }
        }

    if (exceptionInfoP  &&  exceptionInfoP->ContextRecord)
        {
        /*-------------------------------------------------------------------
         Display all registers.
        -------------------------------------------------------------------*/
        regs = *exceptionInfoP->ContextRecord;

#if defined (_WIN64)
        stackPointer = (PBYTE)regs.Rsp; 
#else
        stackPointer = (PBYTE)regs.Esp;
        dumpRegistersX86 (stream, regs);
#endif

        __try { win32Tools_displayMemoryStats (stream); }    __except (EXCEPTION_EXECUTE_HANDLER) { ; }
        __try { win32Tools_generateMiniDump (stream, exceptionInfoP, NULL); }    __except (EXCEPTION_EXECUTE_HANDLER) { ; }



#if defined (NEEDSWORK_Exception_Logging)
    g_logStream = stream;
    exception_registerExceptionLogCallback(logCallback);
    exception_logCallStackInfo((LPEXCEPTION_POINTERS)exceptionInfoP);
    g_logStream = NULL;
#endif

    __try 
        { 
        invokeManagedStackDump (stream); }  __except (EXCEPTION_EXECUTE_HANDLER) { ; }
        }

    /*-------------------------------------------------------------------
     Display delay load problems.
    -------------------------------------------------------------------*/
    if (g_DelayLoadHookFailure.isValid)
        {
        stream->PrintfTo (FALSE,  L"\nDelay load hook failure detail:\n");
        __try { win32Tools_printDelayLoadHookFailure (stream); }    __except (EXCEPTION_EXECUTE_HANDLER) { ; }
        g_DelayLoadHookFailure.isValid = FALSE;
        }

    /*-------------------------------------------------------------------
     Display raw stack dump.
    -------------------------------------------------------------------*/
    if (exceptionInfoP)
        {
        PBYTE byteP = (PBYTE)  stackPointer;
        PBYTE endP  = byteP;

        win32Tools_getStackInfo (___, &endP, ___, stackPointer);

        stream->PrintfTo (FALSE,  L"\nRaw stack dump:\n");

        __try
            {
            uint32_t  const *  ulP = (uint32_t *) stackPointer;
            int i, lines;
            for (lines=0; (lines < 1024) && (byteP <= endP); lines++)
                {
                stream->PrintfTo (FALSE,  L"%08x : ", byteP);
                for (i=0; i< 4; i++, ulP++)
                    stream->PrintfTo (FALSE,  L" %08x ", *ulP);

                stream->PrintfTo (FALSE,  L" : ");

                for (i=0; i < 8; i++, byteP++)
                    stream->PrintfTo (FALSE,  L"%hc", isprint(*byteP) ? *byteP : '.');

                stream->PrintfTo (FALSE,  L" ");

                for (i=0; i < 8; i++, byteP++)
                    stream->PrintfTo (FALSE,  L"%hc", isprint(*byteP) ? *byteP : '.');

                stream->PrintfTo (FALSE,  L" :\n");
                }
            }
        __except (EXCEPTION_EXECUTE_HANDLER)
            {
            ;
            }
        }

    dejavu = false;
    }

