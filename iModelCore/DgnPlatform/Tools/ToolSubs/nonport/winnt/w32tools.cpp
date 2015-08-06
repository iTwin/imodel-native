/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/nonport/winnt/w32tools.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#define NOMINMAX
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
#include    <dbghelp.h>
#include    <tlhelp32.h>
#include    <Winternl.h>
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
#include    <Bentley/Bentley.h>
#include    <Bentley/BeThreadLocalStorage.h>
#include    <DgnPlatform/ExportMacros.h>
#include    <DgnPlatform/DgnPlatformlib.h>
#include    <DgnPlatform/DesktopTools/envvutil.h>
#include    <DgnPlatform/DesktopTools/ConfigurationManager.h>
#include    <RmgrTools/Tools/memutil.h>
#include    <DgnPlatform/Tools/ToolsAPI.h>
#include    <DgnPlatform/DesktopTools/pagalloc.h>   /* page protection based memory allocation */
#include    <RmgrTools/Tools/pagstruc.h>    /* PAGALLOC function prototypes */
#include    <DgnPlatform/DesktopTools/pagalloc.fdf>

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

static enum
    {
    MAX_THREAD_NAME_HISTORY    =   20
    };

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MikeStratoti    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     toolSubsystem_setThreadName     // Set thread name for VC6 debugger.
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
uint32_t      dliNotify,
PDelayLoadInfo  pdli
)
    {
    g_DelayLoadHookFailure.isValid   = true;
    g_DelayLoadHookFailure.dliNotify = dliNotify;
    g_DelayLoadHookFailure.dli       = *pdli;
    }

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    MikeStratoti                    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    win32Tools_printDelayLoadHookFailure
(
FILE            * const stream
)
    {
    char           const *msg;
    uint32_t const  notify = g_DelayLoadHookFailure.dliNotify;
    DelayLoadInfo  const  dli    = g_DelayLoadHookFailure.dli;

    if      (dliStartProcessing       == notify) msg = "StartProcessing";
    else if (dliNotePreGetProcAddress == notify) msg = "NotePreGetProcAddress";
    else if (dliFailLoadLib           == notify) msg = "FailLoadLib";
    else if (dliFailGetProc           == notify) msg = "FailGetProc";
    else if (dliNoteEndProcessing     == notify) msg = "NoteEndProcessing";
    else                                         msg = "Unknown";

    fprintf (stream, "     notify: %d '%s'\n", notify, msg);
    fprintf (stream, "     dli: cb %d,  last error %d, hmodCur %#x\n", dli.cb, dli.dwLastError, dli.hmodCur, dli.szDll);
    fprintf (stream, "          lib '%s', ", dli.szDll);
    if (dli.dlp.fImportByName)
        fprintf (stream, " function '%s'", dli.dlp.szProcName);
    else
        fprintf (stream, " oridinal %d/%#x", dli.dlp.dwOrdinal, dli.dlp.dwOrdinal);
    fprintf (stream, "\n");
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    Peter.Segal                     03/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void    win32Tools_displayMemoryStats
(
FILE * const    stream
)
    {
    MEMORYSTATUSEX              stat = { sizeof(stat) };
    PROCESS_MEMORY_COUNTERS     pmc;

    HANDLE const    hProc    = GetCurrentProcess ();

    fprintf (stream, "\nMemory Statistics for process ID %d:\n\n", GetCurrentProcessId());

    if (GlobalMemoryStatusEx (&stat))
        {
        fprintf (stream, "    %10I64u MB Total Phys\n",                 stat.ullTotalPhys >> 20);
        fprintf (stream, "    %10I64u MB Avail Phys\n",                 stat.ullAvailPhys >> 20);
        fprintf (stream, "    %10I64u MB Total Page File\n",            stat.ullTotalPageFile >> 20);
        fprintf (stream, "    %10I64u MB Avail Page File\n",            stat.ullAvailPageFile >> 20);
        fprintf (stream, "    %10I64u MB Total Virtual\n",              stat.ullTotalVirtual >> 20);
        fprintf (stream, "    %10I64u MB Avail Virtual\n",              stat.ullAvailVirtual >> 20);
        fprintf (stream, "    %10I64u MB Avail Extended Virtual\n\n",   stat.ullAvailExtendedVirtual >> 20);
        }

    if (GetProcessMemoryInfo (hProc, &pmc, sizeof(pmc)))
        {
        fprintf (stream, "    %10u MB PeakWorkingSetSize\n",            pmc.PeakWorkingSetSize >> 20);
        fprintf (stream, "    %10u MB WorkingSetSize\n",                pmc.WorkingSetSize >> 20);
        fprintf (stream, "    %10u MB PagefileUsage\n",                 pmc.PagefileUsage >> 20);
        fprintf (stream, "    %10u MB PeakPagefileUsage\n\n",           pmc.PeakPagefileUsage >> 20);

        fprintf (stream, "       %10u PageFaultCount\n",                pmc.PageFaultCount);
        fprintf (stream, "       %10u QuotaPeakPagedPoolUsage\n",       pmc.QuotaPeakPagedPoolUsage);
        fprintf (stream, "       %10u QuotaPagedPoolUsage\n",           pmc.QuotaPagedPoolUsage);
        fprintf (stream, "       %10u QuotaPeakNonPagedPoolUsage\n",    pmc.QuotaPeakNonPagedPoolUsage);
        fprintf (stream, "       %10u QuotaNonPagedPoolUsage\n\n",      pmc.QuotaNonPagedPoolUsage);
        }

    fprintf (stream, "       %10u GDI Objects\n",                       GetGuiResources (hProc, GR_GDIOBJECTS));
    fprintf (stream, "       %10u USER Objects\n",                      GetGuiResources (hProc, GR_USEROBJECTS));
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mike.Stratoti                   10/03
+---------------+---------------+---------------+---------------+---------------+------*/
void    win32Tools_generateMiniDump
(
FILE                     * const stream,
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
            fprintf (stream, "\n\nMissing \"%s\" or from system. No dump generated.\n", modulePath);
        FreeLibrary (hDbgHelp);
        return;
        }
    
    typedef struct _MINIDUMP_EXCEPTION_INFORMATION
        {
        uint32_t             ThreadId;
        PEXCEPTION_POINTERS ExceptionPointers;
        BOOL                ClientPointers;
        } MINIDUMP_EXCEPTION_INFORMATION, *PMINIDUMP_EXCEPTION_INFORMATION;
    typedef BOOL (__stdcall *FPMiniDumpWriteDump) (HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType, PMINIDUMP_EXCEPTION_INFORMATION, PVOID, PVOID);

    FPMiniDumpWriteDump const fpMiniDumpWriteDump = (FPMiniDumpWriteDump) GetProcAddress (hDbgHelp, "MiniDumpWriteDump");
    if (!fpMiniDumpWriteDump)
        {
        if (NULL != stream)
            fprintf (stream, "\n\nMissing \"MiniDumpWriteDump\" API from \"%s\". No dump generated.\n", szWitchDll);
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
                fprintf (stream, "\n\nSuppressed mini-dump generation.\n");
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
        
        _snwprintf (szDumpFile, MAX_PATH, L"%ls%ls", szMS_TMP, L"MiniDump.dmp");

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
                fprintf (stream, "\n\nWrote a mini-dump type %#x to \"%s\".\n", dumpOpts, szDumpFile);
            else
                fprintf (stream, "\n\nFailed to write a mini-dump. GLE %#x.\n", GetLastError());
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

#if !defined (PRG_CERTIFIEDBUILD) && !defined (PRG_BETABUILD)
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
            defaultMask = MCW_EM & ~(EM_ZERODIVIDE);
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
//    UInt32   fOsVersion = fVersionFlags & 0x0000FFFF;
    uint32_t wOSMajorVersion = LOBYTE(LOWORD(fVersionFlags));
//    UInt32   wOSMinorVersion = HIBYTE(LOWORD(fVersionFlags));
//    UInt32   wOSBuild        = HIWORD(fVersionFlags) & ~0x8000;
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
            uint32_t type = 0, size = 0;
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
