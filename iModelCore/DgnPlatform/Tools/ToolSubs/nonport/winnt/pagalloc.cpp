/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/nonport/winnt/pagalloc.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------
Here are the environment variables that affect PAGALLOC memory debugging:

    MS_PAGALLOC                 when set (to any value), enables PAGALLOC memory
                    debugging if EXE uses papatch.c in the way
                    that w32start.c does for USTATION.EXE,
                    and if the MSVCRT.DLL has recognizable
                    signatures for malloc, free, etc.

    PAGE_DEBUG_HEADERS_ALWAYS_VISIBLE   when non-zero, makes header pages visible
                    instead of PAGE_NOACCESS-protected at the
                    expense of one page per allocation.

    PAGE_DEBUG_LOW              when non-zero, top-justifies allocation for
                    more effectively finding accesses
                    at addresses lower than malloced ptrs.

    PAGE_DEBUG_NO_RELEASE_SBH
    PAGE_DEBUG_NO_RELEASE       when non-zero, address space is never re-used.
                    This will cause address space exhaustion
                    for all but the smallest programs.
    PAGE_DEBUG_FREE_CHECK       when non-zero, checks fill pattern during frees.
    PAGE_DEBUG_ALLOC_BREAK      when non-zero, breakpoints when can't malloc.
    PAGE_DEBUG_CALL_STACK       when non-zero, captures call stack on mallocs.
    PAGE_DEBUG_MAX_FRAMES       when non-zero, limits number of frames to record.
    PAGE_DEBUG_ALLOC_DUMP       when non-zero, dumps "heap" on malloc failures.
                    Value can be an integral number of mallocs
                    to skip for "heap" dump.
    PAGE_DEBUG_REALLOC_BREAK    when non-zero, breakpoints when realloc frees fail.
    PAGE_DEBUG_FREE_BREAK       when non-zero, breakpoints when free fails.
    PAGE_DEBUG_MSIZE_BREAK      when non-zero, breakpoints when msize fails.
    PAGE_DEBUG_EXPAND_BREAK     when non-zero, breakpoints when expand fails.
    PAGE_DEBUG_ADDRESS_BREAK    when non-zero, breakpoints when address freed
                    doesn't exactly match address returned by malloc.
    PAGE_DEBUG_FILL_DATA        if set to hex value, this is used to fill
                    malloced memory.  Will accept 1, 2, or 4
                    byte fill patterns.
    PAGE_DEBUG_ALIGN_BITS       if set to integral value, will align all
                    malloced pointers by masking off this many
                    bits.  For example, 1 bit will align on word
                    bounds, and 2 bits will align on 32-bit bounds.
    PAGE_DEBUG_SBH_MB           if set to integral value, uses this to size the
                    small block heap (units are megabytes).
    PAGE_DEBUG_SBH_DECR_MB      if set to integral value, uses this to decrement
                    the size of the small block heap when the
                    initial reservation fails (units are megabytes).
    PAGE_DEBUG_SBH_BASE         if set to integral value, uses this to set the
                    base address of the small block heap.
    PAGE_DEBUG_SBH_TOP_DOWN     when non-zero, allocates the small block heap using
                    highest possible virtual address range.
    PAGE_DEBUG_LRU_DISABLE      when non-zero, disables least recently used
                    allocation algorithm for small block heap.
    PAGE_DEBUG_TRACELOG         if set to integral non-zero value, calls toolsubs
                    traceLog functions to log significant events,
                    potentially even logging each malloc and free
                    if the pagalloc_malloc and pagalloc_free
                    traceLog layers are toggled on with blogtog.
    PAGE_DEBUG_DUMP_MODULES     if set to integral value, dumps all module names
                    and base addresses after this many mallocs.
                    Dump goes to debugger output and, if enabled,
                    traceLog severity debug.
    PAGE_DEBUG_OUTFILE          if set to filename (full path), output normally
                    sent to debugger and/or traceLog will get
                    appended to this file.
    PAGE_DEBUG_CHECK_DOUBLE_FREE
                if non-zero, print out the stack for a double freed block.
                (asserts PAGE_DEBUG_CALL_STACK)

    PAGE_DEBUG_SORT_BY_COUNT    Print TopLeakers by occurrence count instead of memory use.
                Normally sort top leakers by memory use.

    PAGE_DEBUG_SHOW_BLOCK_CONTENTS   Show formatted memory dump in leak detail

    PAGE_DEBUG_TOP_MATCHES_TO_PRINT  Number of leakers to print in the leak detect summary

    PAGE_DEBUG_TOP_MATCHES_TO_DETAIL Number of leakers to print in the leak detect details

    PAGE_DEBUG_SHOW_STACK_USED       Show stack space used per call.

    PAGE_LEAK_DETECT_TRIGGER        Invoke a "leak detect" when the SMH is about to run out.
                                    The value can be 0 to disable (default) or the number megabytes to reserve from PAGE_DEBUG_SBH_MB.

    PAGE_DEBUG_LITE             0 for traditional PAGALLOC using a full page per allocation;
                                1 for soft PAGALLOC functionality
                                2 for very minimal support


-----------------------------------------------------------------------*/
//WIP #pragma optimize( "y", on ) // Ensure that we get standard stack frames
#if !defined (_WIN32_WINNT)
#  define _WIN32_WINNT (0x0500)
#endif
#define STRICT
#include <windows.h>
#include <winnt.h>
#include <dbghelp.h>
#include <objbase.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#undef DGN_PLATFORM_MT
#undef BENTLEY_PORTABLE_CODE
#include <Bentley/Bentley.h>
#include <DgnPlatform/ExportMacros.h>
#include <RmgrTools/Tools/memutil.h>
#include <DgnPlatform/DesktopTools/readdbg.h>
//#include <boost/crc.hpp>

/* these functions need to be intrinsic to avoid C runtime recursion */
#pragma intrinsic(memset, memcpy, memcmp, strlen, pow, log)
#define PAGALLOC_API __declspec(dllexport)
#include <RmgrTools/Tools/pagstruc.h>
#include <DgnPlatform/Tools/pagalloc.h>
#include <DgnPlatform/Tools/w32tools.h>
#include <Bentley/Bentley.h>

extern "C" void* _ReturnAddress (void);
#pragma intrinsic(_ReturnAddress)
#define PAGALLOC_CONTEXT _ReturnAddress()

#define CRTCallback  static

#include <DgnPlatform/Tools/pagalloc.fdf>

#if ! defined (___)
#   define ___      (0)
#endif

#if ! defined (BSISUCCESS)
#   define BSISUCCESS      (0)
#endif

#define PatchedCall static                              // This is a routine that is replacing another routine

/* NT Server allows 3GB of address space under some conditions */
#define THREE_GB (s_memoryStatus.dwTotalVirtual > (UInt32)0x7fffffff)

#define STACK_BYTES_NEEDED_FOR_PRINTF           (0x9000)
#define STACK_BYTES_NEEDED_FOR_LOG_PRINTF       (0x8000 + STACK_BYTES_NEEDED_FOR_PRINTF)

/* structure for small block heap */
#define INITIAL_NSBH 2
typedef struct _sbh_struct
    {
    MEMORY_BASIC_INFORMATION mbi;
    char           *blockP;             // Base of small block heap (SBH)
    UInt32          mbytes;            // SBH sized scaled by 1 Meg (i.e.  << 20 )
    UInt32          blocks;            // Number of 4K, 8K, or whatever blocks
    UInt32          blockBytes;                // Number of bytes in a block (4K, 8K, or whatever)
    UInt32          bitmapBytes;       //
    UInt32         *bitmapP;            //
    UInt32         *lruBitmapP;         //
    UInt32          bitmapDwords;       //
    Int32           pagesPerBlock;      //
    UInt32          blocksInUseCount;   // Number of allocated SBH blocks in use
    } Sbh;


enum PagallocSeverity
    {
    PAGALLOC_SEVERITY_TRACE         = 0x00000001,
    PAGALLOC_SEVERITY_DEBUG         = 0x00000002,
    PAGALLOC_SEVERITY_INFORMATION   = 0x00000003,
    PAGALLOC_SEVERITY_WARNING       = 0x00000004,
    PAGALLOC_SEVERITY_ERROR         = 0x00000005
    };

/* a newer critical section function we will dynamically lookup */
typedef BOOL (WINAPI *tTryEnterCriticalSection)(LPCRITICAL_SECTION lpCriticalSection);

// Win2K function to return the number of object in use
#if ! defined (GR_GDIOBJECTS)
#   define GR_GDIOBJECTS     0          // Count of GDI objects
#   define GR_USEROBJECTS    1          // Count of USER objects
#endif
typedef UInt32 (WINAPI *tGetGuiResources) (HANDLE hProcess, UInt32 uiFlags);


/*----------------------------------------------------------------------+
|                                                                                                       |
|   Local prototypes                                                                            |
|                                                                                                       |
+----------------------------------------------------------------------*/
static void pagallocI_freeHook(void);
void pagalloc_printf (PagallocSeverity level, char const * const  szFormat, ...);


typedef void * TaskContext;

typedef struct mdlStackTrace
    {
    void        *pPC;       //  codep, ProgramCounter
    void        *pFrame;
    void        *pMdlDesc;
    } MdlStackTrace;

typedef struct functionexclusionlist
    {
    uintptr_t start;                // Start address of function to exclude.
    uintptr_t end;                          // End   address of function to exclude.
    UInt32 hitCount;                // Number of times that this function was found in the last leak detect sweep
    } FunctionExclusionList;

typedef void    MdlDesc, *MdlFunctionP;
typedef void *  (*MdlSystem_getUstnMdlDesc)  (void);
typedef void *  (*MdlSystem_getCurrMdlDesc)  (void);
typedef Int32   (*DlmSystem_callAnyFunction) (char *pArgs,  MdlDesc *descP, MdlFunctionP offset, ...);

typedef enum
    {
    PAGALLOC_TRADITIONAL    = 0,
    PAGALLOC_LITE           = 1,
    PAGALLOC_MINIMAL        = 2
    } PagallocMethod;

static FunctionExclusionList    g_fel[50];
static uintptr_t                g_felUsed;

static SYSTEM_INFO          s_systemInfo;
static MEMORYSTATUS         s_memoryStatus;

//  If true, pagalloc is using the lightweight version of the memory
//  diagnostic tools. If false, pagalloc is using the traditional approach
static Int32                s_usingLite;
static HeapLiteHeader*      s_heapList;
static HeapLiteHeader*      s_heapListEnd;
static UInt32               s_heapListLength;
static PageMallocEntry*     s_headers [6];   //  We generate headers on the fly for PageAllocLite
static Int32                s_nextHeader;
static HANDLE               s_hPagAllocHeap;  //  For PagAllocLite, alloc memory from here

/* global flags set via system environment variables */
static Int32        s_pageDebugHeadersAlwaysVisible;
static Int32        s_pageDebugLow;
static Int32        s_pageDebugNoRelease;
static Int32        s_pageDebugNoReleaseSBH;
static Int32        s_pageDebugFreeCheck;
static Int32        s_pageDebugAllocBreak;
static Int32        s_pageDebugAllocDump;
static Int32        s_pageDebugCallStack;
static Int32        s_pageDebugMaxFrames;
static Int32        s_pageDebugCheckDoubleFree;
static Int32        s_pageDebugReallocBreak;
static Int32        s_pageDebugFreeBreak;
static Int32        s_pageDebugMsizeBreak;
static Int32        s_pageDebugExpandBreak;
static Int32        s_pageDebugAddressBreak;
static Int32        s_pageDebugAlignBits;
static Int32        s_pageDebugAlignMask;
static Int32        s_pageDebugFillSize;
static UInt32       s_pageDebugFillData;
static UInt32       s_pageDebugSbhMB;
static UInt32       s_pageDebugSbhDecrMB;
static UInt32       s_pageDebugSbhBase;
static UInt32       s_pageDebugSbhTopDown;
static Int32        s_pageDebugLruDisable;
static Int32        s_pageDebugTraceLog;
static Int32        s_pageDebugDumpModules;
static char         s_pageDebugOutfileName[512];
static HANDLE       s_pageDebugOutfile;
static Int32        s_pageDebugShowStackUsed;

/* global counters */
static UInt32       s_mallocSerialNumber;         // Ever increasing number of allocations
static UInt32       s_mallocCount;                // Specific number of "mallocs" (excluding "new", "realloc", etc)
static UInt32       s_callocCount;
static UInt32       s_strdupCount;
static UInt32       s_wcsdupCount;
static UInt32       s_reallocCount;
static UInt32       s_freeCount;
static size_t       s_byteCount;
static UInt32       s_waitCount;
static UInt32       s_operatorNewCount;
static UInt32       s_operatorDeleteCount;
static UInt32       s_largeBlockCount;            // Number of large blocks in use

static UInt32       s_countInitializeCriticalSection;
static UInt32       s_countDeleteCriticalSection;

/* globals for fill data and dumping */
static char                *s_fillDataP;
static CRITICAL_SECTION     s_csDump;
static CRITICAL_SECTION     s_csSymbolize;
/* global for memory protection serialization */
static Int32                s_csVisibleCount;
static CRITICAL_SECTION     s_csVisible;
/* globals for small block heap(s) */
static CRITICAL_SECTION     s_csHeap;
static Int32                s_nHeap;
static Sbh*                 s_sbh = NULL;
static UInt32               s_sbhTotalMbytes;
/* globals for handler functions */
static CRuntimeFreeFunc     pagallocFreeHandler;
static CRuntimeMsizeFunc    pagallocMsizeHandler;
/* globals for traceLog APIs */
static Int32                s_hComponent;
static Int32                s_hLayerPagalloc;
static Int32                s_hLayerMalloc;
static Int32                s_hLayerFree;
/* global for WriteFile results */
static Int32                s_bytesWritten;

static tTryEnterCriticalSection    pTryEnterCriticalSection;                   /* global for newer critical section function */

static UInt32               s_workingMsgCount;
static UInt32               s_leakDetectSinceBase;
static bool                 s_sortLeakersBySize = true;

static PagallocOutputFunction  fpAlternateOutputFunction;

static UInt32       g_userAuxiliaryValue;     /* User specified ID is recorded in thewhen the block is allocated */

static MdlSystem_getCurrMdlDesc     mdlSystem_getCurrMdlDesc;
static DlmSystem_callAnyFunction    dlmSystem_callAnyFunction;

static void *fpAlternateOutputMdlDesc;

/*----------------------------------------------------------------------+
|   Leak Detector functions.                                            |
+----------------------------------------------------------------------*/
static enum
    {
    DISABLE_TRIGGER                     =     0,
    MAX_TOPMATCHES                      =   100,
    MAX_TOPMATCHES_DETAIL               =    10,
    MAX_TOPMATCHES_USERAUXILIARYVALUES  =    10,
    MAX_LEAK_DETECT_CANDIDATES          = 50000,
    PAGALLOC_CRITICAL_SECTION_SIGNATURE = 'PaCS',
    MAX_FAILED_MODULES                  = 256
    };

static UInt32       g_pageLeakDetectTrigger;

typedef enum PDE_CONTROL
    {
    PDE_CONTINUE    = 0,
    PDE_STOP        = 1
    } PDE_CONTROL;

typedef union
    {
    PageMallocEntry*headerP;
    void*       liteData;
    } LeakCandidateDetail;

typedef struct
    {
    LeakCandidateDetail detail;
    uintptr_t    callStackSum;
    size_t       userSize;
    size_t       smallestSize;
    size_t       largestSize;
    size_t       totalSize;
    union
        {
        Int32    groupMemberCount;
        void    *groupRoot;
        };
    } LeakDetectCandidate;

typedef struct
    {
    LeakDetectCandidate     *candidates;
    LeakDetectCandidate     *nextCandidate;
    LeakDetectCandidate     *endCandidate;
    Int32                    maxTopMatches;
    LeakDetectCandidate     *topMatches[MAX_TOPMATCHES];
    } LeakDetectContext;

/*-----------------------------------------------------------------------
 GroupUserAuxiliaryValues is used to
-----------------------------------------------------------------------*/
typedef struct
    {
    UInt32      nEntries;
    uintptr_t   callStackSum[MAX_TOPMATCHES];
    UInt32      userAuxiliaryValues[MAX_TOPMATCHES][MAX_TOPMATCHES_USERAUXILIARYVALUES];
    } GroupUserAuxiliaryValues;


static UInt32       g_topMatchesToPrint     = MAX_TOPMATCHES;
static UInt32       g_topMatchesToDetail    = MAX_TOPMATCHES_DETAIL;
static bool         g_showBlockContents     = false;
static bool         g_leakDetectAllMemory   = true;
static UInt32       s_symbolizeFailedModules[MAX_FAILED_MODULES];
static UInt32       s_symFailedCount        = 0;


/*----------------------------------------------------------------------+
|                                                                       |
|   Prototypes                                                          |
|                                                                       |
+----------------------------------------------------------------------*/
Public PAGALLOC_API void __cdecl   pagalloc_dumpHeader     (PageMallocEntry const * const headerP);
Public PAGALLOC_API void __cdecl   pagalloc_dumpEngine    (PDE_CONTROL (*fpCallBack) (PageMallocEntry *headerP, void *context),void  *context,char  *pStart,char  *pEnd);

// Static table used for CRC hashing.
static bool     s_crcTableCreated = false;
static UInt32   s_crcTable[0x100];

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Chuck.Kirschman                 02/12
+---------------+---------------+---------------+---------------+---------------+------*/
class CRC32Hash
{
private:
    UInt32          m_crc;
    #define POLYNOMIAL 0x04C11DB7

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 11/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    UInt32 Reflect(UInt32 v,int bits)
        {
        UInt32 ret = 0;

        --bits;
        for(int iBit=0; iBit <= bits; iBit++)
            {
            if(v & (1<<iBit))
                ret |= 1 << (bits-iBit);
            }
        return ret;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 11/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    void BuildTable()
        {
        if (s_crcTableCreated)
            return;

        for(int iTableItem = 0; iTableItem < 0x100; iTableItem++)
            {
            s_crcTable[iTableItem] = Reflect(static_cast<UInt32>(iTableItem),8) << 24;

            for (int count = 0; count < 8; count++)
                s_crcTable[iTableItem] = (s_crcTable[iTableItem] << 1) ^ ( (s_crcTable[iTableItem] & (1<<31))  ? POLYNOMIAL : 0);

            s_crcTable[iTableItem] = Reflect(s_crcTable[iTableItem],32);
            }
        s_crcTableCreated = true;
        }

public:
    CRC32Hash (CharCP stringToHash) { Reset(); Hash (stringToHash); }

    UInt32 Get() const {return ~m_crc; }
    void        Reset() {m_crc = ~0;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 11/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    void Hash (CharCP stringToHash)
        {
        BuildTable();

        size_t stringLength = strlen(stringToHash);
        for(UInt32 iEntry = 0; iEntry < stringLength; ++iEntry)
            m_crc = (m_crc >> 8) ^ s_crcTable[ (m_crc & 0xFF) ^ stringToHash[iEntry] ];
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 11/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void memutil_initializeDbgHelp ()
    {
    static bool s_bSymInitialized = false;
    if (!s_bSymInitialized)
        {
        SymSetOptions (SymGetOptions () | SYMOPT_LOAD_LINES | SYMOPT_OMAP_FIND_NEAREST);
        s_bSymInitialized = SymInitialize (GetCurrentProcess(), 0, true) != 0;                // Let DbgHelp invade our process
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32 getHash (char const* moduleName)
    {
    // Tried using the boost hash, but it triggers a runtime error about returning a bigger value in a smaller sized variable in VC10.
    //   Perhaps switch to it in the next release of boost.
//    boost::crc_32_type result; 
//    result.process_bytes(moduleName, strlen (moduleName)); 
//    return result.checksum(); 
    return CRC32Hash (moduleName).Get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isKnownFailingMoudule (char const* moduleName)
    {
    // Just checks if the module name is in the list
    UInt32 hash = getHash (moduleName);
    
    for (UInt32 iSym=0; iSym<s_symFailedCount; iSym++)
        {
        if (hash == s_symbolizeFailedModules[iSym])
            {
            return true;
            }
        else if (s_symbolizeFailedModules[iSym] > hash) // Because they are sorted, if the hash value is larger than it isn't there.
            break;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void addKnownFailingMoudule (char const* moduleName)
    {
    if (s_symFailedCount == MAX_FAILED_MODULES)
        {
        pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "!!! Cannot add another failing module to the list due to insufficient space; performance will probably be degrated.\n");
        return;
        }
    
    // Add it to the list.  Optimization is to keep the list sorted.
    UInt32 hash = getHash (moduleName);
    if (0 == s_symFailedCount)
        {
        s_symbolizeFailedModules[s_symFailedCount] = hash;
        }
    else
        {
        bool inserted = false;
        for (UInt32 iSym=0; iSym<s_symFailedCount; iSym++)
            {
            if (s_symbolizeFailedModules[iSym] > hash)
                {
                memmove (s_symbolizeFailedModules+(iSym+1), s_symbolizeFailedModules+iSym, (s_symFailedCount-iSym)*sizeof(s_symbolizeFailedModules[0]));
                s_symbolizeFailedModules[iSym] = hash;
                inserted = true;
                break;
                }
            }
        if (!inserted)
            s_symbolizeFailedModules[s_symFailedCount] = hash;
        }
    s_symFailedCount++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mike.Stratoti                  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public Int32 memutil_symbolizeAddress 
(
void   *address,                    // => EIP address to query
char   *fileAndLine,                // <= Symbolized "file(line)"
size_t  sizeOfFileAndLine,          // =>
char   *funcName,                   // <= Symbolized "function name"
Int32  sizeofFuncName,              // =>
char   *moduleName,
Int32  sizeofModuleName
)
    {
    void*       hDbg = NULL;
    Int32       appStatus = 0, sysStatus = 0;
    uintptr_t   preferredBaseAddress,  preferredHighAddress;
    MEMORY_BASIC_INFORMATION mbi;
    memset (&mbi, 0, sizeof(mbi));

    VirtualQuery(address, &mbi, sizeof(mbi));
    if(!GetModuleFileName ((HMODULE)(mbi.AllocationBase), moduleName, sizeofModuleName))
        GetModuleFileName(0, moduleName, sizeofModuleName);

    memutil_initializeDbgHelp ();
        
    /*-----------------------------------------------------------------------------------
    Try to use Windows indigenous support for symbolization because it understands PDB files.
    -----------------------------------------------------------------------------------*/
    __try
        {
        Int32      nSymbolized = 0;                                        // When 0, try to locate the symbol using Phil & Mike's code because DbgHelp didn't work

        HANDLE    const hProc = GetCurrentProcess();

        struct
            {
            IMAGEHLP_SYMBOL ihSym;
            char            name[MAX_PATH];                                     // The symbol name follows the IMAGEHLP_SYMBOL structure.
            } ihSym;

        IMAGEHLP_LINE   ihLine;

        memset (&ihLine, 0, sizeof(ihLine));
        ihLine.SizeOfStruct = sizeof(ihLine);
        ihLine.Address = (uintptr_t)address;
        memset (&ihSym, 0, sizeof(ihSym));
        ihSym.ihSym.SizeOfStruct  = sizeof(ihSym).ihSym;
        ihSym.ihSym.MaxNameLength = sizeof(ihSym).name;
        DWORD unused;
        if (fileAndLine  &&  sizeOfFileAndLine)
            {
            BOOL getLineSucceeded = SymGetLineFromAddr (hProc, (uintptr_t)address, &unused,  &ihLine);
            // If this doesn't work, may need to reload the symbols because a new DLL was loaded.
            if (!getLineSucceeded)
                {
                if (!isKnownFailingMoudule (moduleName))
                    {
                    SymRefreshModuleList (hProc);
                    
                    getLineSucceeded = SymGetLineFromAddr (hProc, (uintptr_t)address, &unused,  &ihLine);
                    // Tried to reload the symbols and that didn't work.  Save it to ignore the future.
                    if (!getLineSucceeded)
                        addKnownFailingMoudule (moduleName);
                    }
                }
            
            if (getLineSucceeded)
                {
                _snprintf (fileAndLine, sizeOfFileAndLine,  "%s(%d)", ihLine.FileName, ihLine.LineNumber);
                nSymbolized++;
                }
            }

        if (funcName  &&  sizeofFuncName)
            {
    #if defined (_M_X64)
            DWORD64 displ = 0;
    #else
            DWORD   displ = 0;
    #endif
            nSymbolized += SymGetSymFromAddr (hProc, (uintptr_t)address, &displ,  &ihSym.ihSym);
            _snprintf (funcName, sizeofFuncName, "%s+%#x", ihSym.ihSym.Name, displ);
            }

        if (nSymbolized)                                                    // No need to call  Phil & Mike's symbolization code
            return 0;
        }
    __except (EXCEPTION_EXECUTE_HANDLER)
        {
        ;   // Null statement
        }

    hDbg = readDbg_open(moduleName, &appStatus, &sysStatus);
    if ( !hDbg)
        return  ERROR;

    /*-------------------------------------------------------------------
    Compensate for DLL relocation
    -------------------------------------------------------------------*/
    if ( ! readDbg_getImageRange (hDbg,  &preferredBaseAddress,  &preferredHighAddress) )
        address =  (void*) ( (uintptr_t)address - (uintptr_t)mbi.AllocationBase + preferredBaseAddress );

    /*-------------------------------------------------------------------
    Return requested information.
    -------------------------------------------------------------------*/
    if (fileAndLine)
        {
        appStatus = readDbg_getSrc (hDbg, (uintptr_t) address, fileAndLine, sizeOfFileAndLine);
        }

    if (funcName && sizeofFuncName)
        {
        char offset[16];
        char * plus;

        appStatus = readDbg_getSym (hDbg, (uintptr_t) address, funcName, sizeofFuncName);

        /* truncate offset until later */
        memset(offset, 0, sizeof(offset));
        plus = strrchr(funcName, '+');
        if (plus)
            {
            if (strcmp(plus + 1, "0x0"))
                {
                strncpy(offset, plus, sizeof(offset) - 1);
                offset[sizeof(offset) - 1] = '\0';
                }
            *plus = '\0';
            }

        /* unmangle C++ mangled names */
        if (!appStatus && ('?' == funcName[0]))
            {
            char mangled[512];

            memset(mangled, 0, sizeof(mangled));
            strncpy(mangled, funcName, sizeof(mangled) - 1);
            mangled[sizeof(mangled) - 1] = '\0';
            memset(funcName, 0, sizeofFuncName);
            sysStatus = UnDecorateSymbolName(mangled, funcName, sizeofFuncName, UNDNAME_COMPLETE);
            }

        /* add offset (if any) back on */
        strncat(funcName, offset, sizeofFuncName - 1);
        funcName[sizeofFuncName - 1] = '\0';
        }

    readDbg_close(hDbg, &sysStatus);
    return appStatus;
#if !(defined(_X86_) || defined(_M_X64))
    strncpy(fileAndLine, "memutil_symbolizeAddress only supported on x86 and x64", sizeOfFileAndLine);
    fileAndLine[sizeOfFileAndLine-1] = '\0';
    strncpy(funcName, "memutil_symbolizeAddress only supported on x86 and x64", sizeofFuncName);
    funcName[sizeofFuncName-1] = '\0';

    return ERROR;
#endif
    }

#ifdef UNUSED_FUNCTION
/*----------------------------------------------------------------------+
|                                                                       |
|   C runtime functions we have to internalize to avoid recursion       |
|                                                                       |
+----------------------------------------------------------------------*/
static char * pagallocI_strrchr(char *component, char target)
    {
    INT_PTR len = strlen(component);
    char *ptr;

    for (ptr = component + len; len; ptr--, len--)
        if (target == *ptr)
            return ptr;
    return NULL;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    09/00
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32 pagallocI_strtoul(char *szEnvData, char **pptr, Int32 base)
    {
    UInt32 iRet = 0;
    char maxchar = '9';
    char *ptr = szEnvData;

    if (!base)
        base = 10;
    iRet = 0;
    if (('0' == ptr[0]) &&
        (('x' == ptr[1]) ||
         ('X' == ptr[1])))
        {
        base = 16;
        maxchar = 'f';
        ptr +=2;
        }
    while ((*ptr >= '0') && (*ptr <= maxchar))
        {
        iRet *= base;
        iRet += (*ptr - '0');
        if ((base > 10) && (*ptr > '9'))
            {
            iRet -= ((0x60 & *ptr) - '9');
            }
        ptr++;
        }
    if (pptr)
        *pptr = ptr;
    return iRet;
    }

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    09/00
+---------------+---------------+---------------+---------------+---------------+------*/
static char *pagallocI_ultoa(UInt32 val, char *ptr, Int32 base)
    {
    char *pchRet = ptr;
    double dbase;
    Int32 place;
    UInt32 divisor;

    /* special case for zero (in any base) */
    if (!val)
        {
        *(ptr++) = '0';
        *(ptr++) = (char)0;
        return pchRet;
        }

    /* default to decimal base */
    if (!base)
        base = 10;

    /* calculate divisor */
    dbase = (double)base;
    place = (Int32)(log((double)val) / log(dbase));
    divisor = (UInt32)pow(dbase, place);

    /* divide and subtract for each place */
    for (; divisor; divisor /= base)
        {
        UInt32 digit = val / divisor;

        val -= (digit * divisor);
        if (digit > 9)
            digit += ((0x60 & 'a') - '9');
        *(ptr++) = '0' + digit;
        }
    *(ptr++) = (char)0;
    return pchRet;
    }
#endif

/*----------------------------------------------------------------------+
|                                                                       |
|   Internal functions                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
UInt32 pagalloc_lockVisibleCriticalSection
(
void
)
    {
    //pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: thread %d entered s_csVisible [%d] %s(%d)\r\n", GetCurrentThreadId(), s_csVisibleCount, __FILE__, __LINE__);
//    EnterCriticalSection(&s_csVisible);
    return ++s_csVisibleCount;
    }
typedef enum
    {
    eQuiet = 0,
    eShowDiagnostic = 1,
    } ShowCSDiagnostic;

UInt32 pagalloc_unlockVisibleCriticalSection
(
ShowCSDiagnostic  const bShowDiagnostic
)
    {
    --s_csVisibleCount;
//    if (eShowDiagnostic == bShowDiagnostic)
//        {
//        pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: thread %d left    s_csVisible [%d] %s(%d) [VirtualProtect gle=%d]\r\n", GetCurrentThreadId(), s_csVisibleCount, __FILE__, __LINE__, GetLastError());
//        // __asm int 3;
//        }

//    LeaveCriticalSection (&s_csVisible);
    return s_csVisibleCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PEM             05/01
+---------------+---------------+---------------+---------------+---------------+------*/
static Int32 pagalloc_sufficientStack
(
void
)
    {
    MEMORY_BASIC_INFORMATION mbi;

    memset(&mbi, 0, sizeof(mbi));
    VirtualQuery(&mbi, &mbi, sizeof(mbi));
    return (((uintptr_t)mbi.BaseAddress - (uintptr_t)mbi.AllocationBase) > STACK_BYTES_NEEDED_FOR_PRINTF);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static void pagalloc_logOutput (PagallocSeverity severity, char const *message)
    {
    // This should go through logging if it's set up.  But currently there isn't any real interest.
    // Plus it would make this low-level tools library require the logger.  I'm setting this up
    // as a replacement for the old tracelog stuff (removed) in case anyone wants to revive it, but it's
    // not being tested. 
    
    // Call logger here
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static void pagalloc_logOutputHdr (PagallocSeverity severity, PageMallocEntry const* headerP)
    {
    char debugString[4096];
    memset(debugString, 0, sizeof(debugString));

    _snprintf(debugString, sizeof(debugString),
	"PAGALLOC: pointer: 0x%08p size: 0x%08lx context: 0x%08lx ",
	headerP->dataStart, headerP->userSize, headerP->mallocFrom);
    pagalloc_logOutput (severity, debugString);

//    traceLog_dumpPagallocStack(user_dumpOut, headerP, (bytes - sizeof(PageMallocEntry)) / sizeof(CallStackFrame));
    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Michael.Stratoti08/97
+---------------+---------------+---------------+---------------+---------------+------*/
void     pagalloc_printf
(
PagallocSeverity    level,
char const * const  szFormat,
...
)
    {
    va_list     arg;
    char        debugString[1024 * 5];
    Int32               debugStringLen;

    if (!pagalloc_sufficientStack())
        return;

    va_start (arg, szFormat);
    debugStringLen = wvsprintf (debugString, szFormat, arg);
    va_end (arg);

    /*-------------------------------------------------------------------
     Call fpAlternateOutputFunction if set.  It it ever faults, reset the
     pointer so it won't be a recurring problem.
    -------------------------------------------------------------------*/
    if (fpAlternateOutputFunction)
        {
        __try
            {
            if (fpAlternateOutputMdlDesc  &&  dlmSystem_callAnyFunction)
                (*dlmSystem_callAnyFunction) ("P", fpAlternateOutputMdlDesc, fpAlternateOutputFunction, debugString);
            else
                (*fpAlternateOutputFunction) (debugString);
            }
        __except (EXCEPTION_EXECUTE_HANDLER)
            {
            fpAlternateOutputFunction = NULL;
            }
    return;
    }

    OutputDebugString (debugString);
    if (s_pageDebugTraceLog)
        pagalloc_logOutput (level, debugString);
    if (s_pageDebugOutfile)
        WriteFile (s_pageDebugOutfile, debugString, debugStringLen, (LPDWORD) &s_bytesWritten, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static bool rejectInMinimal ()
    {
    if (PAGALLOC_MINIMAL != s_usingLite)
        return false;   //  niot rejected

    pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION, "PAGALLOC:    operation not supported in minimal mode.\n");

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Gets a temporary header to be used so the lite mode can use functions from the
* traditional PAGALLOC support.
*
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static PageMallocEntry* getNextHeader ()
    {
    PageMallocEntry* retval;

    assert (PAGALLOC_LITE == s_usingLite);

    EnterCriticalSection (&s_csHeap);
    if (s_nextHeader >= _countof (s_headers))
        s_nextHeader = 0;

    retval = s_headers [s_nextHeader++];
    LeaveCriticalSection(&s_csHeap);

    return retval;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brandon.Bohrer  01/12
+---------------+---------------+---------------+---------------+---------------+------*/
static Int32 pagallocI_computeBlocksInUse ()
    {
    UInt32 total = 0;
    
    for (Int32 i=0; i < s_nHeap; i++)
        total += s_sbh[i].blocksInUseCount;

    return total;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brandon.Bohrer  01/12
*  Print SBH usage levels in muliple (aligned) columns to save screen space.
+---------------+---------------+---------------+---------------+---------------+------*/
static void pagallocI_printSbhUsage (char* msg)
    {
    Int32 const rowSize = 4;

    for (Int32 rowStart = 0; rowStart < s_nHeap; rowStart += rowSize)
        {
        pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: %-20s ", msg);

        for (Int32 i = rowStart; i < rowStart + rowSize && i < s_nHeap; i++)
            {
            char buf [128];
            // Print into a buffer first so we can set the width of the column as a whole.
            _snprintf (buf, _countof (buf), "SBH %d used %d / %d %d%%   ", i, s_sbh[i].blocksInUseCount,  s_sbh[i].blocks, 
                            (100 * s_sbh[i].blocksInUseCount) / (s_sbh[i].blocks ? s_sbh[i].blocks : 1));
            pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "%-32s", buf); // 32-character columns
            }

        pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "\r\n");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void pagalloc_displayUsageStatistics
(
char *msg
)
    {
    SYSTEMTIME               localTime;
    static UInt32            lastUse;
    Int32              const deltaUse = s_usingLite ?
                                        s_heapListLength - lastUse :
                                        (Int32) (pagallocI_computeBlocksInUse () - lastUse);
    UInt32                   usedGdi  = 0;
    UInt32                   usedUser = 0;
    static tGetGuiResources  pGetGuiResources = (tGetGuiResources) -1;

    if (!pagalloc_sufficientStack())
        return;

    GetLocalTime (&localTime);

    if ((tGetGuiResources) -1 == pGetGuiResources)
        {
        HMODULE const hUser32   = GetModuleHandle ("User32");
        pGetGuiResources = (tGetGuiResources) GetProcAddress (hUser32, "GetGuiResources");
        }

    if (pGetGuiResources)
        {
        HANDLE  const hProc    = GetCurrentProcess ();
        usedGdi  = pGetGuiResources (hProc, GR_GDIOBJECTS);
        usedUser = pGetGuiResources (hProc, GR_USEROBJECTS);
        }

    if (PAGALLOC_TRADITIONAL != s_usingLite)
        {
        pagalloc_printf (PAGALLOC_SEVERITY_DEBUG,
        "PAGALLOC: %-20s %4d-%02d-%02d %2d:%02d:%02d."
        "  %d allocated entries, delta %d, "
        "ma %u, ca %u, re %u, fr %u, bytes %u, new %u, del %u, wait %u, sd %u, wd %u, LBC %u, GDI %u, USER %u, InitCS %u, DelCS %u\r\n",
        msg,
        localTime.wYear, localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute, localTime.wSecond,
        s_heapListLength, deltaUse,
        s_mallocCount, s_callocCount, s_reallocCount, s_freeCount, s_byteCount, s_operatorNewCount, s_operatorDeleteCount, s_waitCount, s_strdupCount, s_wcsdupCount, s_largeBlockCount,
            usedGdi, usedUser,
            s_countInitializeCriticalSection, s_countDeleteCriticalSection);

        lastUse = s_heapListLength;
        }
    else
        {
        pagalloc_printf (PAGALLOC_SEVERITY_DEBUG,
        "PAGALLOC: %-20s %4d-%02d-%02d %2d:%02d:%02d. delta %d, "
        "ma %u, ca %u, re %u, fr %u, bytes %u, new %u, del %u, wait %u, sd %u, wd %u, LBC %u, GDI %u, USER %u, InitCS %u, DelCS %u\r\n",
        msg,
        localTime.wYear, localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute, localTime.wSecond, deltaUse, s_mallocCount, 
        s_callocCount, s_reallocCount, s_freeCount, s_byteCount, s_operatorNewCount, s_operatorDeleteCount, s_waitCount, s_strdupCount, 
        s_wcsdupCount, s_largeBlockCount, usedGdi, usedUser, s_countInitializeCriticalSection, s_countDeleteCriticalSection);
        
        pagallocI_printSbhUsage (msg);
        lastUse = pagallocI_computeBlocksInUse ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static __inline void    pagalloc_displayWorkingMessage
(
void
)
    {
    const  Int32  mod = 1000000;

    if (!pagalloc_sufficientStack())
        return;

    if ( ! (s_workingMsgCount++ % mod) )
        {
        SYSTEMTIME  localTime;
        GetLocalTime (&localTime);
        pagalloc_printf (PAGALLOC_SEVERITY_DEBUG,  "PAGALLOC:     working %4d     %4d-%02d-%02d %2d:%02d:%02d.\r\n", s_workingMsgCount / mod,
                localTime.wYear, localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute, localTime.wSecond);
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pagalloc_symbolizeHeaderAddressesForVisualC
(
PageMallocEntry const * const headerP,          // =>
Int32                           callsToSkip=0 // => Number of calls not to display, starting at the most recent. Internal calls are
                                            // removed at recording time, so this should be 0 most of the time.
)
    {
    Int32 iFrame;
    Int32 const cFrame = _countof(headerP->callStackFrame);

    __try
        {
        char            line[300];                  // File, line
        uintptr_t           stackUsed = 0;

        memset (line, 0, sizeof(line));

        for (iFrame = callsToSkip;
                (iFrame < cFrame)
            && (headerP->callStackFrame[iFrame].returnAddress)
            && (INVALID_FLOATING_POINT_PATTERN_DWORD != headerP->callStackFrame[iFrame].returnAddress);
            iFrame++)
            {
            // Compress duplicate call frames for recursive functions
            if (iFrame > 0 && headerP->callStackFrame[iFrame].returnAddress == headerP->callStackFrame[iFrame-1].returnAddress)
                {
                Int32 i;
                for (i=iFrame;  (i < (cFrame-1)) && headerP->callStackFrame[iFrame].returnAddress; i++)
                    {
                    if (headerP->callStackFrame[iFrame].returnAddress != headerP->callStackFrame[i].returnAddress)
                    break;
                    }
                stackUsed = headerP->callStackFrame[iFrame].basePointer - headerP->callStackFrame[i].basePointer;

                pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION,
                s_pageDebugShowStackUsed ? "PAGALLOC:    Suppressed duplicate call frames %d to %d (Stack used %d)\r\n" :
                                 "PAGALLOC:    Suppressed duplicate call frames %d to %d\r\n",
                iFrame, i, stackUsed);
                iFrame = i;
                }


            /*-------------------------------------------------------
             Format and display the call frame.  If the high bit is set,
             then we have an MDL frame.  Native code is always less than 0x7fffffff
            -------------------------------------------------------*/
    #define FIXED_FMT               "%-75s : %4d.  0x%p"

            stackUsed = headerP->callStackFrame[iFrame].basePointer - headerP->callStackFrame[iFrame-1].basePointer;

            char   *cp;
            char    func[400];          // Function name
                    line[sizeof(line) - 1] = '\0';
                    func[sizeof(func) - 1] = '\0';
            int const SZ_M = 512;
            char    module[SZ_M];
            module[SZ_M-1] = '\0';
            strcpy (line, " <Unavailable source file and line number>");
            strcpy (func, " <Unknown native code function>");

            EnterCriticalSection(&s_csSymbolize);   // SymGetSymFromAddr called in memutil_symbolizeAddress is not thread-safe
            memutil_symbolizeAddress ((void*)headerP->callStackFrame[iFrame].returnAddress, line, sizeof(line) - 1, func, sizeof(func) - 1, module, SZ_M-1);
            LeaveCriticalSection(&s_csSymbolize);

            char *lastSlash = NULL; 
            for (size_t i = 0; i < strlen(module); ++i)
                {
                if ('\\' == module[i])
                    lastSlash=&module[i];
                }
            if (NULL != lastSlash)
                strcpy (module, lastSlash+1);

            // The form for VC is        <fill_path_and_filename(line_number) : >
            cp = strchr(line, '+');
            if (cp)
                *cp = '\0';

            pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION,
            s_pageDebugShowStackUsed ? FIXED_FMT "  %-50s %-25s (Stack used%5d)\r\n" :
                            FIXED_FMT "  %-25s %-25s\r\n",
            line, iFrame - callsToSkip, headerP->callStackFrame[iFrame].returnAddress, func, module, stackUsed);

            /* stop after main, because it's just the same thing over and over. */
            if (NULL != strstr (func, "main_executeDgnMode"))
                break;
            }
        }
    __except(EXCEPTION_EXECUTE_HANDLER)
        {
        ;
        }
#if !(defined (_X86_) || defined(_M_X64))
    #pragma message ("**************                                                                        ****************")
    #pragma message ("************** pagalloc_symbolizeHeaderAddresses is not implemented for this platform. ****************")
    #pragma message ("**************                                                                        ****************")
#endif /* _X86_ */
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   02/01
+---------------+---------------+---------------+---------------+---------------+------*/
void     pagalloc_symbolizeHeaderAddresses
(
const PageMallocEntry * const headerP           // =>
)
    {
    pagalloc_symbolizeHeaderAddressesForVisualC (headerP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void pagallocI_sbHeapInit(Int32 iHeap)
    {
    Int32 sbhBlockPages;
    UInt32 bitmapAllocBytes = 0;
    UInt32 bitmapAllocDwords = 0;
    UInt32 bytesReserved = 0;

    assert (PAGALLOC_TRADITIONAL == s_usingLite);
    /* force top down for 2nd heap on 3GB systems */
    if ((iHeap > 0) && THREE_GB)
        {
        s_pageDebugSbhTopDown = true;
        }

    /* calculate pages in allocation bitmap */
    sbhBlockPages = (s_pageDebugSbhMB << 20) / s_systemInfo.dwPageSize;
    if (!s_sbh[iHeap].pagesPerBlock)
        s_sbh[iHeap].pagesPerBlock = (s_pageDebugHeadersAlwaysVisible ? 4 : 2);

    /* set s_sbh[iHeap].blockBytes parameters used later */
    s_sbh[iHeap].blockBytes = s_sbh[iHeap].pagesPerBlock * s_systemInfo.dwPageSize;

    /* calculate size of bitmap to allocate */
    bitmapAllocBytes = (sbhBlockPages / s_sbh[iHeap].pagesPerBlock) / BITS_PER_BYTE;
    bitmapAllocDwords = bitmapAllocBytes / sizeof (UInt32);

    if (!s_pageDebugLruDisable)
        {
        /* allocate "least recently used" s_sbh block bitmap */
        s_sbh[iHeap].lruBitmapP = (UInt32*)VirtualAlloc(NULL, bitmapAllocBytes + s_systemInfo.dwPageSize, MEM_RESERVE, PAGE_READWRITE);
        s_sbh[iHeap].lruBitmapP = (UInt32*)VirtualAlloc(s_sbh[iHeap].lruBitmapP, bitmapAllocBytes, MEM_COMMIT, PAGE_READWRITE);
        if (!s_sbh[iHeap].lruBitmapP)
            {
            pagalloc_printf(PAGALLOC_SEVERITY_ERROR, "PAGALLOC: could not allocate s_sbh lru bitmap!\r\n");
            DebugBreak();
            }
        }

    /* allocate s_sbh block bitmap */
    s_sbh[iHeap].bitmapP = (UInt32*)VirtualAlloc(NULL, bitmapAllocBytes + s_systemInfo.dwPageSize, MEM_RESERVE, PAGE_READWRITE);
    s_sbh[iHeap].bitmapP = (UInt32*)VirtualAlloc(s_sbh[iHeap].bitmapP, bitmapAllocBytes, MEM_COMMIT, PAGE_READWRITE);
    if (!s_sbh[iHeap].bitmapP)
        {
        pagalloc_printf(PAGALLOC_SEVERITY_ERROR, "PAGALLOC: could not allocate s_sbh bitmap!\r\n");
        DebugBreak();
        }

    /* reserve small block heap itself (initially invisible) */
    for (s_sbh[iHeap].mbytes = s_pageDebugSbhMB; (s_sbh[iHeap].mbytes > 0) && !s_sbh[iHeap].blockP; )
        {
        bytesReserved = s_sbh[iHeap].mbytes << 20;
        s_sbh[iHeap].blockP = (char*)VirtualAlloc((void *)s_pageDebugSbhBase, bytesReserved,
                       MEM_RESERVE | (s_pageDebugSbhTopDown?MEM_TOP_DOWN:0),
                       PAGE_NOACCESS);
        if (!s_sbh[iHeap].blockP)
            s_sbh[iHeap].blockP = (char*)VirtualAlloc(NULL, bytesReserved,
                       MEM_RESERVE | (s_pageDebugSbhTopDown?MEM_TOP_DOWN:0),
                       PAGE_NOACCESS);
        if (!s_sbh[iHeap].blockP)
            s_sbh[iHeap].mbytes -= s_pageDebugSbhDecrMB;
        }

    bytesReserved = s_sbh[iHeap].mbytes << 20;

    /* calculate and report actual number of pages in small block heap */
    VirtualQuery(s_sbh[iHeap].blockP, &s_sbh[iHeap].mbi, sizeof(s_sbh)[iHeap].mbi);
    sbhBlockPages = bytesReserved / s_systemInfo.dwPageSize;

    pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION, "PAGALLOC: reserved %uMB (%d block pairs, 0x%x pages) for small block heap %d starting at 0x%p to 0x%p\r\n",
                            s_sbh[iHeap].mbytes, sbhBlockPages/2, sbhBlockPages, iHeap, s_sbh[iHeap].blockP,   bytesReserved + s_sbh[iHeap].blockP);

    s_sbhTotalMbytes += s_sbh[iHeap].mbytes;

    /* calculate size of allocation bitmap that should actually be used */
    s_sbh[iHeap].bitmapBytes = (sbhBlockPages / s_sbh[iHeap].pagesPerBlock) / BITS_PER_BYTE;
    s_sbh[iHeap].bitmapDwords = s_sbh[iHeap].bitmapBytes / sizeof (UInt32);

    /* sanity test: bitmap size to be used should be greater or equal to size allocated */
    if (bitmapAllocBytes < s_sbh[iHeap].bitmapBytes)
        {
        pagalloc_printf(PAGALLOC_SEVERITY_ERROR, "PAGALLOC: bitmapBytes bug!\r\n");
        s_sbh[iHeap].bitmapBytes = bitmapAllocBytes;
        }
    if (bitmapAllocDwords < s_sbh[iHeap].bitmapDwords)
        {
        pagalloc_printf(PAGALLOC_SEVERITY_ERROR, "PAGALLOC: bitmapDwords bug!\r\n");
        s_sbh[iHeap].bitmapDwords = bitmapAllocDwords;
        }

    /* set s_sbh[iHeap].blocks parameters used later */
    s_sbh[iHeap].blocks = sbhBlockPages / s_sbh[iHeap].pagesPerBlock;

    /* set number of heaps global */
    if (s_sbh[iHeap].blockP)
        s_nHeap = 1 + iHeap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   01/2012
*  Ensure that nHeap SBH's exist and are initialized.
+---------------+---------------+---------------+---------------+---------------+------*/
static void pagallocI_sbHeapsEnsure (Int32 nHeap)
    {
    if (!pTryEnterCriticalSection(&s_csHeap))
        {
        InterlockedIncrement ((volatile Long32 *)&s_waitCount);
        EnterCriticalSection (&s_csHeap);
        }

    if (nHeap <= s_nHeap)
        return;

    Sbh* newHeapArray = (Sbh*)VirtualAlloc(NULL, nHeap * sizeof (s_sbh[0]), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (NULL == newHeapArray)
        {
        pagalloc_printf(PAGALLOC_SEVERITY_ERROR, "PAGALLOC: could not expand s_sbh!\r\n");
        DebugBreak (); // Out of virtual memory. Giving up.
        }

    // If we already have some heaps, copy them over and delete the old ones.
    if (NULL != s_sbh)
        {
        memcpy (newHeapArray, s_sbh, s_nHeap * sizeof (s_sbh[0]));
        VirtualFree (s_sbh, s_nHeap * sizeof (s_sbh[0]), MEM_RELEASE);
        }

    s_sbh = newHeapArray;

    for (Int32 iHeap = s_nHeap; iHeap < nHeap; iHeap++)
        pagallocI_sbHeapInit (iHeap);

    LeaveCriticalSection (&s_csHeap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32 pagallocI_getEnvULong
(
char *szEnvName,
UInt32 iDefault
)
    {
    char szEnvData[1024];
    UInt32 iRet = iDefault;
    UInt32 iStatus;
    char *ptr;

    memset(szEnvData, 0, sizeof(szEnvData));
    iStatus = GetEnvironmentVariable(szEnvName, szEnvData, sizeof(szEnvData));
    if (iStatus)
#if SAFE_TO_CALL_STRTOUL_HERE
        iRet = strtoul(szEnvData, &ptr, 0);
#else
        iRet = pagallocI_strtoul(szEnvData, &ptr, 0);
#endif
    return  iRet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static HeapLiteTrailer* getLiteTrailer
(
HeapLiteHeader*header
)
    {
    char*       pc = (char*)header + sizeof (*header);
    UInt32    userDataSize = (header->requestedSize + 3) & ~3;

    assert (PAGALLOC_LITE == s_usingLite);
    return (HeapLiteTrailer*)(pc + userDataSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getLiteInfo
(
HeapLiteHeader**liteHeader,
UInt32*       userDataSize,
HeapLiteTrailer**liteTrailer,
void const * const actualData
)
    {
    char*       pc = (char*)actualData;
    HeapLiteHeader*   header = (HeapLiteHeader*)(pc - sizeof (HeapLiteHeader));

    assert (PAGALLOC_LITE == s_usingLite);

    *liteHeader = header;

    if (PAGE_SIGNATURE != header->signature)
        //  This prevents a crash that can occur when using the liteTrailer computed in
        //  the next step.  We assume that the caller will detect the bad signature
        //  and will not use the header.
        *userDataSize = 0;
    else
        *userDataSize = (header->requestedSize + 3) & ~3;

    *liteTrailer = (HeapLiteTrailer*)(pc + *userDataSize);
    }

static Int32 s_pagallocInitialized;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/01
+---------------+---------------+---------------+---------------+---------------+------*/
Int32 __declspec(dllexport) __cdecl  pagalloc_isInitialized
(
void
)
    {
    return s_pagallocInitialized;
    }

static void pagallocI_pageModeCalc
(
size_t userSize,
size_t *allocSize,
size_t *commitSize,
uintptr_t *initialOffset
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void            initializePagAlloc ()
    {
    if (s_pagallocInitialized)
        return;

    {  //  Because it is not C++
    char szSemName[512];
    HANDLE hSem;
    Int32 status;

    memset(szSemName, 0, sizeof(szSemName));
    wsprintf(szSemName, "pagalloc_initialize_%04x_%p", GetCurrentProcessId(), pagallocI_pageModeCalc);
    hSem = CreateSemaphore (NULL,   /* address of security attributes */
                   0,       /* initial count */
                   1,       /* maximum count */
                   szSemName
                   );
    status = GetLastError();
    /*-------------------------------------------------------------------
    make sure we were unique creator of this named semaphore!
    -------------------------------------------------------------------*/
    if (status)
        {
        /*-------------------------------------------------------------------
        we were not unique creator; wait here for initialization to complete.
        -------------------------------------------------------------------*/
        if (WaitForSingleObject(hSem, INFINITE))
            {
            pagalloc_printf (PAGALLOC_SEVERITY_ERROR, "PAGALLOC: initialization failure (lastError=%d)\r\n", GetLastError() );
            DebugBreak();
            }
        }
    else
        {
        Int32 i;
        HMODULE const hKernel32 = GetModuleHandle ("KERNEL32");

        /*-------------------------------------------------------------------
        only if we were unique creator can we do the initialization.
        -------------------------------------------------------------------*/
        /* lookup a newer critical section function */
        if (!pTryEnterCriticalSection)
            pTryEnterCriticalSection = (tTryEnterCriticalSection) GetProcAddress(hKernel32, "TryEnterCriticalSection");
        if (!pTryEnterCriticalSection)
            pTryEnterCriticalSection = (tTryEnterCriticalSection) GetProcAddress(hKernel32, "EnterCriticalSection");
        if (!pTryEnterCriticalSection)
            pTryEnterCriticalSection = (tTryEnterCriticalSection) EnterCriticalSection;

        /* initialize critical sections */
        InitializeCriticalSection(&s_csVisible);
        InitializeCriticalSection(&s_csHeap);
        InitializeCriticalSection(&s_csDump);
        InitializeCriticalSection(&s_csSymbolize);

        /* other one-time initializations */
        memset(&s_memoryStatus, 0, sizeof(s_memoryStatus));
        s_memoryStatus.dwLength = sizeof(s_memoryStatus);
        GlobalMemoryStatus(&s_memoryStatus);
        GetSystemInfo(&s_systemInfo);
        if (!s_systemInfo.dwPageSize)
            s_systemInfo.dwPageSize = PAGE_SIZE_DEFAULT;

    #   if defined (PAGALLOC_PAGE_DEBUG_CHECK_DOUBLE_FREE)
    #   pragma message ("**Enabling default stack recording because of +ddebug.")
        bool canWalkStack = true;              // ON by default for Development
    #   else
        bool canWalkStack = false;             // OFF for released code because VC's Frame Pointer Omission (FPO) breaks the stack walking code.
    #   endif

        s_pageDebugCheckDoubleFree      =   pagallocI_getEnvULong("PAGE_DEBUG_CHECK_DOUBLE_FREE",       canWalkStack);
        s_pageDebugCallStack            =   pagallocI_getEnvULong ("PAGE_DEBUG_CALL_STACK",             canWalkStack);

        s_pageDebugHeadersAlwaysVisible =   pagallocI_getEnvULong ("PAGE_DEBUG_HEADERS_ALWAYS_VISIBLE", 0);
        s_pageDebugLow                  =   pagallocI_getEnvULong ("PAGE_DEBUG_LOW",                    0);
        s_usingLite                     =   pagallocI_getEnvULong ("PAGE_DEBUG_LITE",                   0);
        s_pageDebugNoRelease            =   pagallocI_getEnvULong ("PAGE_DEBUG_NO_RELEASE",             0);
        s_pageDebugNoReleaseSBH         =   pagallocI_getEnvULong ("PAGE_DEBUG_NO_RELEASE_SBH",         0);
        s_pageDebugAllocBreak           =   pagallocI_getEnvULong ("PAGE_DEBUG_ALLOC_BREAK",            0);
        s_pageDebugMaxFrames            =   pagallocI_getEnvULong ("PAGE_DEBUG_MAX_FRAMES",             0);             //  ignored if 0
        s_pageDebugAllocDump            =   pagallocI_getEnvULong ("PAGE_DEBUG_ALLOC_DUMP",             0);
        s_pageDebugReallocBreak         =   pagallocI_getEnvULong ("PAGE_DEBUG_REALLOC_BREAK",          0);
        s_pageDebugFreeBreak            =   pagallocI_getEnvULong ("PAGE_DEBUG_FREE_BREAK",             0);
        s_pageDebugMsizeBreak           =   pagallocI_getEnvULong ("PAGE_DEBUG_MSIZE_BREAK",            0);
        s_pageDebugExpandBreak          =   pagallocI_getEnvULong ("PAGE_DEBUG_EXPAND_BREAK",           0);
        s_pageDebugAddressBreak         =   pagallocI_getEnvULong ("PAGE_DEBUG_ADDRESS_BREAK",          0);
        s_pageDebugFillData             =   pagallocI_getEnvULong ("PAGE_DEBUG_FILL_DATA",              INVALID_FLOATING_POINT_PATTERN_WORD);
        s_pageDebugAlignBits            =   pagallocI_getEnvULong ("PAGE_DEBUG_ALIGN_BITS",             2);
        s_pageDebugFreeCheck            =   pagallocI_getEnvULong ("PAGE_DEBUG_FREE_CHECK",             s_pageDebugAlignBits);
        s_pageDebugSbhMB                =   pagallocI_getEnvULong ("PAGE_DEBUG_SBH_MB",                 1280);
        s_pageDebugSbhDecrMB            =   pagallocI_getEnvULong ("PAGE_DEBUG_SBH_DECR_MB",            16);
        s_pageDebugSbhBase              =   pagallocI_getEnvULong ("PAGE_DEBUG_SBH_BASE",               0);
        s_pageDebugSbhTopDown           =   pagallocI_getEnvULong ("PAGE_DEBUG_SBH_TOP_DOWN",           0);
        s_pageDebugLruDisable           =   pagallocI_getEnvULong ("PAGE_DEBUG_LRU_DISABLE",            0);
        s_pageDebugDumpModules          =   pagallocI_getEnvULong ("PAGE_DEBUG_DUMP_MODULES",           0);
        s_pageDebugTraceLog             =   pagallocI_getEnvULong ("PAGE_DEBUG_TRACELOG",               0);
        s_sortLeakersBySize             = ! pagallocI_getEnvULong ("PAGE_DEBUG_SORT_BY_COUNT",          false);
        g_showBlockContents             =   pagallocI_getEnvULong ("PAGE_DEBUG_SHOW_BLOCK_CONTENTS",    false) != 0;
        g_topMatchesToPrint             =   pagallocI_getEnvULong ("PAGE_DEBUG_TOP_MATCHES_TO_PRINT",   MAX_TOPMATCHES);
        g_topMatchesToDetail            =   pagallocI_getEnvULong ("PAGE_DEBUG_TOP_MATCHES_TO_DETAIL",  MAX_TOPMATCHES_DETAIL);
        s_pageDebugShowStackUsed        =   pagallocI_getEnvULong ("PAGE_DEBUG_SHOW_STACK_USED",        false);

        g_pageLeakDetectTrigger         =   pagallocI_getEnvULong ("PAGE_Leak_Detect_Trigger",          DISABLE_TRIGGER);
        g_pageLeakDetectTrigger         =   (g_pageLeakDetectTrigger << 20) / s_systemInfo.dwPageSize;        // Unit is "Number of small blocks"-- same as s_sbh[0].blocks and s_sbh[0].blocksInUseCount

        GetEnvironmentVariable ("PAGE_DEBUG_OUTFILE", s_pageDebugOutfileName,  sizeof(s_pageDebugOutfileName));

        /* open output file */
        if (s_pageDebugOutfileName[0])
            {
            s_pageDebugOutfile = ::CreateFileA
                (
                s_pageDebugOutfileName,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                0
                );
            if (INVALID_HANDLE_VALUE == s_pageDebugOutfile)
                s_pageDebugOutfile = NULL;
            else
                SetFilePointer(s_pageDebugOutfile , 0, NULL, FILE_END);
            }

        /* do some calculations based on env config vars */
        if (s_pageDebugFillData)
            {
            if (0xffff0000 & s_pageDebugFillData)
                s_pageDebugFillSize = 4;
            else if (0xffffff00 & s_pageDebugFillData)
                s_pageDebugFillSize = 2;
            else
                s_pageDebugFillSize = 1;
            }
        if (!s_pageDebugFillSize)
            s_pageDebugFillSize = 1;
        s_fillDataP = (char*)VirtualAlloc(NULL, s_systemInfo.dwPageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        switch (s_pageDebugFillSize)
            {
            case 1:
                memset(s_fillDataP, (Int32)s_pageDebugFillData, s_systemInfo.dwPageSize);
                break;
            case 2:
                {
                unsigned short *wordP = (unsigned short *)s_fillDataP;
                Int32 wordCount = s_systemInfo.dwPageSize / sizeof(short);
                for (i = 0; i < wordCount; i++)
                    {
                    wordP[i] = (unsigned short)s_pageDebugFillData;
                    }
                }
                break;
            case 4:
                {
                UInt32 *dwordP = (UInt32 *)s_fillDataP;
                Int32 dwordCount = s_systemInfo.dwPageSize / sizeof(Int32);
                for (i = 0; i < dwordCount; i++)
                    {
                    dwordP[i] = s_pageDebugFillData;
                    }
                }
                break;
            }

        if (s_pageDebugAlignBits)
            s_pageDebugAlignMask = 0xffffffff << s_pageDebugAlignBits;

        if (s_usingLite)
            {
            Int32 i;

            s_hPagAllocHeap = HeapCreate (0, 0, 0);
            for (i = 0; i < _countof (s_headers); i++)
                s_headers [i] = (PageMallocEntry*)HeapAlloc (s_hPagAllocHeap, 0, sizeof (PageMallocEntry));
            }
        else
            {
            /* small block heap initialization */
            if (s_pageDebugSbhMB >= 2048)
                s_pageDebugSbhMB = 2047;
            
            pagallocI_sbHeapsEnsure (INITIAL_NSBH);
            
            pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION, "PAGALLOC: Usage statistics meaning:\r\n"
                            "PAGALLOC:    number of: ma=malloc, ca=calloc, re=realloc, fr=free\r\n"
                            "PAGALLOC:               new=C++ new, del=C++ delete, wait=thread stalls by heap contention, sd=strdup, wd=_wcsdup\r\n"
                            "PAGALLOC:               SBH used n / t  p%% = n used blocks of t total blocks, %% blocks used\r\n"
                            "PAGALLOC:               delta=change in  number of used blocks used since last message, LBC=number of >4K in use allocations\r\n"
                            "PAGALLOC:    mem used:  bytes=in use\r\n");
            }
        }

    s_pagallocInitialized = true;
    ReleaseSemaphore(hSem, 1, NULL);

    }  //  Because it is not C++
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    12/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void pagallocI_pageModeCalc
(
size_t userSize,
size_t *allocSize,
size_t *commitSize,
uintptr_t *initialOffset
)
    {
    uintptr_t s_byteCount;
    uintptr_t pageCount;
    UInt32 onFirstPage;

    assert (PAGALLOC_TRADITIONAL == s_usingLite);

    s_byteCount = userSize;

    if (s_byteCount == s_systemInfo.dwPageSize)                                     // If this is exactly one page...
        {
        pageCount       = 1;
        onFirstPage     = s_systemInfo.dwPageSize;
        }
    else
        {
        pageCount       = 1 + (s_byteCount / s_systemInfo.dwPageSize);
        onFirstPage     = s_byteCount % s_systemInfo.dwPageSize;
        }
    *commitSize     = pageCount * s_systemInfo.dwPageSize;
    *allocSize      = (pageCount + (s_pageDebugHeadersAlwaysVisible ? 3 : 2)) * s_systemInfo.dwPageSize;
    *initialOffset  = s_systemInfo.dwPageSize - onFirstPage;
    if (s_pageDebugAlignMask)
        *initialOffset &= s_pageDebugAlignMask;
    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32 pagallocI_bitScanForwardClearAndSet
(
UInt32  *dwP,
Int32   iHeap
)
    {
    UInt32 dwNum, bitNum;

    if (!pTryEnterCriticalSection(&s_csHeap))
        {
        InterlockedIncrement ((volatile Long32 *)&s_waitCount);
        EnterCriticalSection (&s_csHeap);
        }

    for (bitNum = dwNum = 0; dwNum < s_sbh[iHeap].bitmapDwords; dwNum++)
        {
        UInt32 temp = ~dwP[dwNum];
        if (temp)
            {
            for (bitNum = 0; bitNum < BITS_PER_DWORD; bitNum++)
                {
                if (temp & (1 << bitNum))
                    {
                    temp        = ~temp;            /* complement back */
                    temp       |= (1 << bitNum);  /* set bit in bitmap */
                    dwP[dwNum]  = temp;     /* store back in bitmap array */
                    break;
                    }
                }
            break;
            }
        }

    LeaveCriticalSection(&s_csHeap);
    return (dwNum * BITS_PER_DWORD) + bitNum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    08/99
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32 pagallocI_bitSet
(
UInt32   *dwP,
UInt32    block,
Int32       iHeap
)
    {
    UInt32 dwNum;
    UInt32 bitNum;
    UInt32 retVal;
    UInt32 mask;

    if (block >= s_sbh[iHeap].blocks)
        return      0;

    if(!pTryEnterCriticalSection(&s_csHeap))
        {
        InterlockedIncrement ((volatile Long32 *)&s_waitCount);
        EnterCriticalSection (&s_csHeap);
        }
    dwNum       = block / BITS_PER_DWORD;
    bitNum      = block % BITS_PER_DWORD;
    mask        = 1 << bitNum;
    retVal      = dwP[dwNum] & mask;
    dwP[dwNum] |= mask;
    LeaveCriticalSection(&s_csHeap);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    08/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void pagallocI_bitCopyAll
(
UInt32   *dwDstP,
UInt32   *dwSrcP,
Int32       iHeap
)
    {
    if(!pTryEnterCriticalSection(&s_csHeap))
        {
        InterlockedIncrement ((volatile Long32 *)&s_waitCount);
        EnterCriticalSection (&s_csHeap);
        }
    memcpy(dwDstP, dwSrcP, s_sbh[iHeap].bitmapBytes);
    LeaveCriticalSection(&s_csHeap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32 pagallocI_bitTestAndClear /* <= zero means bit wasn't set (error) */
(
UInt32   *dwP,
UInt32    block,
Int32       iHeap
)
    {
    UInt32 dwNum;
    UInt32 bitNum;
    UInt32 retVal;
    UInt32 mask;

    if(!pTryEnterCriticalSection(&s_csHeap))
        {
        InterlockedIncrement ((volatile Long32 *)&s_waitCount);
        EnterCriticalSection (&s_csHeap);
        }
    dwNum   = block / BITS_PER_DWORD;
    bitNum  = block % BITS_PER_DWORD;
    mask    = 1 << bitNum;
    retVal  = dwP[dwNum] & mask;
        if (retVal)
        {
        dwP[dwNum] &= ~mask;
        }
    LeaveCriticalSection(&s_csHeap);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  PhilipMcGraw    02/2002
+---------------+---------------+---------------+---------------+---------------+------*/
static bool pagallocI_makeHeaderVisible
(
PageMallocEntry const * const headerP,
UInt32 newProtect,
UInt32 *oldProtectP
)
    {
    UInt32 oldProtect = 0;
    bool ok;

    if (s_pageDebugHeadersAlwaysVisible || s_usingLite)
        return true;

    if (!newProtect)
        newProtect = PAGE_READONLY;
    pagalloc_lockVisibleCriticalSection ();

    ok = VirtualProtect ((void *)headerP, s_systemInfo.dwPageSize, newProtect, (PDWORD)&oldProtect) !=0;
    if (!ok)
        pagalloc_unlockVisibleCriticalSection (eShowDiagnostic);

    if (oldProtectP)
        *oldProtectP = oldProtect;
    return ok;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  PhilipMcGraw    02/2002
+---------------+---------------+---------------+---------------+---------------+------*/
static bool pagallocI_makeHeaderInvisible
(
PageMallocEntry const * const headerP,
UInt32 newProtect,
UInt32 *oldProtectP
)
    {
    bool ok;
    UInt32 oldProtect = 0;

    if (s_pageDebugHeadersAlwaysVisible || s_usingLite)
        return true;

    if (!newProtect)
        newProtect = PAGE_NOACCESS;
    ok = VirtualProtect ((void *)headerP, s_systemInfo.dwPageSize, newProtect, (PDWORD)&oldProtect) !=0;

    /* make sure we really own critical section */
    if (0 >= s_csVisibleCount)
        {
        pagalloc_printf(PAGALLOC_SEVERITY_ERROR,
            "PAGALLOC: thread %d tried to leave s_csVisible without having entered critical section [%d] %s(%d) [VirtualProtect gle=%d]\r\n",
            GetCurrentThreadId(), s_csVisibleCount, __FILE__, __LINE__, GetLastError());
        return ok;
        }
    pagalloc_unlockVisibleCriticalSection (eShowDiagnostic);
    return ok;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool  pagallocI_sbFindBlock (Int32& iHeap, UInt32& block)
    {
    bool        flushed = false;

    assert (PAGALLOC_TRADITIONAL == s_usingLite);

    if(!pTryEnterCriticalSection(&s_csHeap))
        {
        InterlockedIncrement ((volatile Long32 *)&s_waitCount);
        EnterCriticalSection (&s_csHeap);
        }

    iHeap = s_nHeap; // Past the end of the array - indicates block not found.

    /* NB: to avoid deadlocks, please don't call anything (i.e. printf) *
    *     that might allocate memory while you own the s_csHeap lock!    */
    if (!s_pageDebugLruDisable)
        {
        /* look first in the LRU table to keep from re-using same blocks as often */
        for (Int32 i = 0; i < s_nHeap; i++)
            {
            block = pagallocI_bitScanForwardClearAndSet(s_sbh[i].lruBitmapP, i);
            if (block < s_sbh[i].blocks)
                {
                pagallocI_bitSet(s_sbh[i].bitmapP, block, i);
                iHeap = i;
                break;
                }
            }

        if (iHeap >= s_nHeap)
            {
            flushed = true;

            /* the LRU table is full, so we just get the next block directly... */
            for (Int32 i = 0; i < s_nHeap; i++)
                {
                block = pagallocI_bitScanForwardClearAndSet(s_sbh[i].bitmapP, i);
                if (block < s_sbh[i].blocks)
                    {
                    iHeap = i;
                    break;
                    }
                }

            /* ...and then copy all new LRU tables from current bitmaps */
            for (Int32 i = 0; i < s_nHeap; i++)
                pagallocI_bitCopyAll(s_sbh[i].lruBitmapP, s_sbh[i].bitmapP, i);
            }
        }
    else /* not using LRU bitmap */
        {
        for (Int32 i = 0; i < s_nHeap; i++)
            {
            block = pagallocI_bitScanForwardClearAndSet(s_sbh[i].bitmapP, i);
            if (block < s_sbh[i].blocks)
                {
                iHeap = i;
                break;
                }
            }
        }
    
    if (flushed)
        pagalloc_displayUsageStatistics ("SB-LRU cache flushed");
        
    // Put here to avoid accessing s_nHeap outside the critical section.
    bool foundBlock = iHeap < s_nHeap;

    LeaveCriticalSection(&s_csHeap);
    return foundBlock;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void *pagallocI_sbReserve
(
Int32 *iHeapP
)
    {
    UInt32      block = -1;
    Int32       iHeap = -1;
    char       *charP;
    
    assert (PAGALLOC_TRADITIONAL == s_usingLite);

    // If the current heaps are full, allocate a new one.
    if (!pagallocI_sbFindBlock (iHeap, block))
        {
        pagallocI_sbHeapsEnsure (s_nHeap + 1);

        bool foundNewBlock = pagallocI_sbFindBlock (iHeap, block);
        assert (foundNewBlock);
        }

    *iHeapP = iHeap;

    charP = s_sbh[iHeap].blockP + (block * s_sbh[iHeap].blockBytes);
    s_sbh[iHeap].blocksInUseCount++;

    /*---------------------------------------------------------------
    Initialize the header.  When <s_pageDebugCheckDoubleFree>  is set
    the control block may already exist so we just change the
    protection to access it.  If we can't change it then we must be
    allocating it for the first time.
    ---------------------------------------------------------------*/
    if (s_pageDebugCheckDoubleFree)
        {
        PageMallocEntry const * const headerP = (PageMallocEntry const * const)charP;
        bool ok = pagallocI_makeHeaderVisible (headerP, PAGE_READWRITE, ___);
        if (ok)
            {
            memset (charP, 0, s_systemInfo.dwPageSize);
            pagallocI_makeHeaderInvisible (headerP, PAGE_NOACCESS, ___);
            }
        }

    if ( s_pageDebugNoReleaseSBH )
        {
        bool ok;
        ok = VirtualFree (charP + s_systemInfo.dwPageSize, s_systemInfo.dwPageSize, MEM_DECOMMIT) != 0; //Conversion from WINAPI BOOL to c++ bool
        // It is alright for the above call to fail because it may be the first time that we are allocating this memory.
        //  if (ok)
        //      pagalloc_printf(PAGALLOC_SEVERITY_ERROR, "PAGALLOC: Unexpected VirtualFree failure. 0x%p, %d, %d.\r\n", charP, s_systemInfo.dwPageSize, GetLastError());
        }

// On X64 we make more SBHs as needed, so the check below doesn't work and is less useful since it's harder to run out of memory.
#ifdef _X86_
    UInt32 blocksFree = 0;

    for (Int32 i=0; i < s_nHeap; i++)
        blocksFree += s_sbh[i].blocks - s_sbh[i].blocksInUseCount;

    if (g_pageLeakDetectTrigger > blocksFree)
        {
        // Reset so that the leak detector code can run.  We are not going to try to recover because we are so close to exhausting the SBH
        g_pageLeakDetectTrigger = 0;
        pagalloc_detectLeaks ();
        DebugBreak ();
        }
#endif

    return  charP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool pagallocI_sbDecommit
(
PageMallocEntry *headerP
)
    {
    UInt32      block;
    UInt32      blocks;
    Int32       iHeap = headerP->iHeap;
    char       *charP = (char *)headerP;
    bool        ok;

    assert (PAGALLOC_TRADITIONAL == s_usingLite);

    if (iHeap <= s_nHeap)
        {
        block = (UInt32)(charP - s_sbh[iHeap].blockP)  / s_sbh[iHeap].blockBytes;
        blocks = s_sbh[iHeap].blocks;
        }

    if ((iHeap > s_nHeap) || (block >= blocks))
        {
        pagalloc_printf (PAGALLOC_SEVERITY_ERROR, "PAGALLOC: trying to free block not in small block heap\r\n");
        DebugBreak();
        return false;
        }

    if (!pagallocI_bitTestAndClear(s_sbh[iHeap].bitmapP, block, iHeap))
        {
        pagalloc_printf (PAGALLOC_SEVERITY_ERROR, "PAGALLOC: small block heap corrupted\r\n");
        DebugBreak();
        return false;
        }

    if (s_pageDebugCheckDoubleFree)
        {
        /*---------------------------------------------------------------
        Keep the control block but free the data block
        ---------------------------------------------------------------*/
        UInt32 fdwOldProtect = PAGE_NOACCESS;
        char *ctrlBlockP = s_sbh[iHeap].blockP + (block * s_sbh[iHeap].blockBytes);
        char *dataBlockP = ctrlBlockP + s_sbh[iHeap].blockBytes - s_systemInfo.dwPageSize;

        ok = pagallocI_makeHeaderVisible(headerP, PAGE_READWRITE, &fdwOldProtect);
        headerP->flags.isFree       = true;
        headerP->userAuxiliaryValue = g_userAuxiliaryValue;
        ok = pagallocI_makeHeaderInvisible(headerP, fdwOldProtect, ___);
        if ( s_pageDebugNoReleaseSBH )
            ok = VirtualProtect (dataBlockP, s_systemInfo.dwPageSize, PAGE_NOACCESS, (PDWORD)&fdwOldProtect) !=0;
        else
            ok = VirtualFree        (dataBlockP, s_systemInfo.dwPageSize, MEM_DECOMMIT) !=0;
        }
    else
        {
        /* decommit both control and data blocks */
        charP = s_sbh[iHeap].blockP + (block * s_sbh[iHeap].blockBytes);
        ok = VirtualFree(charP, s_sbh[iHeap].blockBytes, MEM_DECOMMIT) != 0;
        }
    s_sbh[iHeap].blocksInUseCount--;

    return ok;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static PageMallocEntry* getHeaderProxy
(
void const * const actualData
)
    {
    UInt32            i;
    PageMallocEntry*    entry = getNextHeader ();
    HeapLiteHeader*     liteHeader;
    HeapLiteTrailer*    liteTrailer;
    UInt32              userDataSize;
    intptr_t*           returnAddress;
    CallStackFrame*     callStackFrame;

    getLiteInfo (&liteHeader, &userDataSize, &liteTrailer, actualData);

    entry->signature    = liteHeader->signature;
    entry->commitSize   = entry->allocSize = entry->userSize = liteHeader->requestedSize;
    entry->initialOffset= 0;
    entry->mallocSerialNumber = liteTrailer->mallocSerialNumber;
    entry->mallocFrom   = liteTrailer->mallocFrom;
    entry->flags.isFree = liteTrailer->flags.isFree;
    entry->flags.lastOperation = liteTrailer->flags.lastOperation;
    entry->allocStart   = liteTrailer;
    entry->dataStart    = (void*)actualData;
    entry->callStackSum = liteTrailer->callStackSum;
    entry->userAuxiliaryValue = liteTrailer->userAuxiliaryValue;

    returnAddress       = liteTrailer->returnAddress;
    callStackFrame      = entry->callStackFrame;

    for (i = 0; i < liteTrailer->nFrames; i++)
        {
        callStackFrame [i].basePointer      = 0;
        callStackFrame [i].returnAddress    = returnAddress [i];
        callStackFrame [i].paramFirst       = 0;
        callStackFrame [i].paramSecond      = 0;
        }

    callStackFrame [i].returnAddress    = 0;
    return entry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    09/96
+---------------+---------------+---------------+---------------+---------------+------*/
static void *pagallocI_findHeader
(
void const * const voidP
)
    {
    MEMORY_BASIC_INFORMATION memoryBasicInformation;
    char   *byteP = (char *)voidP;
    Int32     iHeap;

    assert (PAGALLOC_MINIMAL != s_usingLite);
    if (s_usingLite)
        return getHeaderProxy (voidP);

    /*-------------------------------------------------------------------
    go below low guard page and query to find header at start of page
    -------------------------------------------------------------------*/
    if (s_pageDebugHeadersAlwaysVisible)
        byteP -= (s_systemInfo.dwPageSize<<1); /* two pages back: header + guard */
    else
        byteP -= (s_systemInfo.dwPageSize); /* one page back: header */

    memset(&memoryBasicInformation, 0, sizeof(memoryBasicInformation));
    VirtualQuery(byteP, &memoryBasicInformation, sizeof(memoryBasicInformation));
    if  (memoryBasicInformation.Protect & (PAGE_GUARD | PAGE_NOCACHE) )
        NULL;

    /*-------------------------------------------------------------------
    return base address if this memory is in our small block heap
    -------------------------------------------------------------------*/
    for (iHeap = 0; (iHeap < s_nHeap); iHeap++)
        {
        if (s_sbh[iHeap].mbi.AllocationBase == memoryBasicInformation.AllocationBase)
            {
            return  memoryBasicInformation.BaseAddress;
            }
        }
    return memoryBasicInformation.AllocationBase;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    08/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool pagallocI_openHeader
(
PageMallocEntry const * const headerP
)
    {
    UInt32 fdwOldProtect = PAGE_NOACCESS;
    bool ok;

    assert (PAGALLOC_MINIMAL != s_usingLite);
    if (!headerP)
        return      false;

    if (s_usingLite)
        {
        ok = !IsBadReadPtr(headerP, 1) && (PAGE_SIGNATURE == headerP->signature);
        return ok;
        }

    ok = pagallocI_makeHeaderVisible(headerP, PAGE_READONLY, &fdwOldProtect);
    if (!ok)
        return  false;

    ok = !IsBadReadPtr(headerP, 1) && (PAGE_SIGNATURE == headerP->signature);

    /* checkHeader leaves header visible unless returning false */
    if (!ok)
        pagallocI_makeHeaderInvisible(headerP, fdwOldProtect, ___);

    return  ok;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    08/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void pagallocI_closeHeader
(
PageMallocEntry const * const headerP
)
    {
    if (!headerP || s_usingLite)
        return;

    pagallocI_makeHeaderInvisible(headerP, PAGE_NOACCESS, ___);
    }

/* NB: this should only be called in special situations in which the
 * header has already been freed or for some other reason cannot be
 * closed using pagallocI_closeHeader or pagallocI_makeHeaderInvisible */
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    08/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pagallocI_leaveHeaderVisible(void)
    {
    /* make sure we really have a visible header */
    if (0 >= s_csVisibleCount)
        {
        pagalloc_printf(PAGALLOC_SEVERITY_ERROR,
            "PAGALLOC: thread %d tried to leave s_csVisible without having entered critical section [%d] %s(%d) [VirtualProtect gle=%d]\r\n",
            GetCurrentThreadId(), s_csVisibleCount, __FILE__, __LINE__, GetLastError());
        return;
        }
    pagalloc_unlockVisibleCriticalSection (eShowDiagnostic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   02/01
+---------------+---------------+---------------+---------------+---------------+------*/
Int32     pagallocI_recordCallingStack
(
PageMallocEntry *headerP,       // =>
void* returnAddress             // => Most recent call that will be recorded in the call stack
)
    {
    typedef USHORT (WINAPI *BackTraceFuncType)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);
    typedef BOOL (WINAPI *FP_SymGetSymFromAddr) (__in HANDLE hProcess, __in uintptr_t addr, __out PDWORD pdwDisplacement, __out PIMAGEHLP_SYMBOL symbol);

    //Only load the functions we need once.
    static BackTraceFuncType BackTraceFunc = (BackTraceFuncType) GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlCaptureStackBackTrace"); 
    static bool DbgHelpInitialized = false;
    if (!DbgHelpInitialized)
        {
        memutil_initializeDbgHelp();
        DbgHelpInitialized = true;
        }
    
    //Microsoft-Imposed limit: 62 total recorded/skipped calls on XP/Server 2003, from: http://msdn.microsoft.com/en-us/library/bb204633(VS.85).aspx
    const Int32 maxTotalAddrs = 62;
    const Int32 maxRecordedAddrs = maxTotalAddrs;
    void* addrs[maxTotalAddrs];
    UInt32 nAddrs = BackTraceFunc(0, maxRecordedAddrs, addrs, NULL);
    assert (nAddrs > 0);
    UInt32 i=0;
    for(; i < min(nAddrs, _countof(headerP->callStackFrame)); i++)
        {
        headerP->callStackFrame[i].returnAddress =(uintptr_t) addrs[i];
        headerP->callStackFrame[i].basePointer=0;
        headerP->callStackFrame[i].paramFirst=0;
        headerP->callStackFrame[i].paramSecond=0;
        headerP->callStackSum += (uintptr_t) addrs[i];
        }

    EnterCriticalSection(&s_csSymbolize);   // SymGetSymFromAddr is not thread-safe
    struct tagSymInfo
        {
        IMAGEHLP_SYMBOL sym;
        char symNameBuf [100];
        } SymInfo = { {sizeof (IMAGEHLP_SYMBOL)}};
    IMAGEHLP_SYMBOL * pSym = &SymInfo.sym;
    pSym->MaxNameLength = 100;
    Int32 callsToSkip=0;
    SymGetSymFromAddr(GetCurrentProcess(), (intptr_t)returnAddress, 0, pSym);
    LeaveCriticalSection(&s_csSymbolize);

    //Skip calls until we find the function that contains returnAddress
    for(; callsToSkip < _countof(headerP->callStackFrame); callsToSkip ++)
        if(headerP->callStackFrame[callsToSkip].returnAddress > pSym->Address
            && headerP->callStackFrame[callsToSkip].returnAddress < pSym->Address + pSym->Size)
            break;
    assert ((UInt32)callsToSkip <= nAddrs);
    //Once the good calls are copied, the other ones should be erased, so set the number of addresses to the number of addresses we care about.
    nAddrs -= callsToSkip;
    //Copy the good addresses forward to the beginning of the array
    memmove(headerP->callStackFrame, headerP->callStackFrame + callsToSkip, (_countof(headerP->callStackFrame)- callsToSkip) * sizeof(CallStackFrame));
    //Blank out the originals of the copied addresses, plus any garbage.
    memset(headerP->callStackFrame + nAddrs, 0, (_countof(headerP->callStackFrame) - nAddrs) * sizeof(CallStackFrame));
    return i;
#if !(defined(_X86_) || defined(_M_X64))
#pragma message ("**************                                                                    ****************")
#pragma message ("************** pagallocI_recordCallingStack is not implemented for this platform. ****************")
#pragma message ("**************                                                                    ****************")
    return 0;
#endif /* !(defined(_X86_) || defined(_M_X64)) */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void __cdecl pagallocI_fill
(
char    *dataP,
size_t     size
)
    {
    const uintptr_t initialOffset = (uintptr_t)dataP % s_systemInfo.dwPageSize;
    char const * const initialFillP  = s_fillDataP + initialOffset;
    const bool onFirstPage   = ((s_systemInfo.dwPageSize - initialOffset) % s_systemInfo.dwPageSize) != 0;
    const uintptr_t wholePages    = size / s_systemInfo.dwPageSize;
    const bool onLastPage    = (size - (wholePages * s_systemInfo.dwPageSize) - onFirstPage) != 0;
    uintptr_t page;

    if (onFirstPage)
        {
        memcpy(dataP, initialFillP, onFirstPage);
        dataP += onFirstPage;
        }

    for (page = 0; page < wholePages; page++)
        {
        memcpy(dataP, s_fillDataP, s_systemInfo.dwPageSize);
        dataP += s_systemInfo.dwPageSize;
        }

    if (onLastPage)
        {
        memcpy(dataP, s_fillDataP, onLastPage);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static void    addToMallocedList
(
HeapLiteHeader* header
)
    {
    //  Add to the end of the list. That way,
    if (!pTryEnterCriticalSection(&s_csHeap))
        {
        InterlockedIncrement ((volatile Long32 *)&s_waitCount);
        EnterCriticalSection (&s_csHeap);
        }

    if (NULL == s_heapList)
        {
        s_heapList = header;
        header->next = header->prev = NULL;
        s_heapListEnd = header;
        }
    else
        {
        s_heapList->prev = header;
        header->next = s_heapList;
        s_heapList = header;
        header->prev = NULL;
        }

    s_heapListLength++;

    LeaveCriticalSection(&s_csHeap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static void    removeFromMallocedList
(
HeapLiteHeader* header
)
    {
    if (!pTryEnterCriticalSection(&s_csHeap))
        {
        InterlockedIncrement ((volatile Long32 *)&s_waitCount);
        EnterCriticalSection (&s_csHeap);
        }

    if (NULL != header->next)
        header->next->prev = header->prev;
    else
        s_heapListEnd = header->prev;

    if (NULL != header->prev)
        header->prev->next = header->next;
    else
        s_heapList = header->next;

    s_heapListLength--;

    LeaveCriticalSection(&s_csHeap);
    }

/*---------------------------------------------------------------------------------**//**
* This handles every type of allocation that would use the CRT heap. It is used for
* malloc, calloc, realloc, operator new, strdup, and _wcsdup.
*
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static void*   pagAllocLiteI_malloc
(
size_t                const requestedSize,
void          const * const context,
unsigned char const         lastOperation,
PageMallocEntry*            pageMallocEntry,
Int32                    const recordedFrames
)
    {
    Int32         i;
    Int32         userDataSize = (requestedSize + 3) & ~3;
    Int32         allocSize;
    char*       retval;
    HeapLiteHeader* header;
    HeapLiteTrailer* trailer;
    intptr_t*   returnAddress;
    CallStackFrame* callStackFrame;

    allocSize = sizeof (HeapLiteHeader) + userDataSize + sizeof (HeapLiteTrailer) + (recordedFrames - 1) * sizeof (UInt32);
    header = (HeapLiteHeader*)HeapAlloc (s_hPagAllocHeap, 0, allocSize);

    header->requestedSize = requestedSize;
    header->signature = PAGE_SIGNATURE;
    retval = (char*)header + sizeof (*header);

    //  Some of these will be overwritten by the header, but there is no reason not to
    //  initialize the entries that will be overridden.
    *(retval + requestedSize) = PAGALLOC_LITE_FILLER_CHAR;
    *(retval + requestedSize + 1) = PAGALLOC_LITE_FILLER_CHAR;
    *(retval + requestedSize + 2) = PAGALLOC_LITE_FILLER_CHAR;

    trailer = (HeapLiteTrailer*)(retval + userDataSize);

    trailer->signature          = PAGE_SIGNATURE;
    trailer->mallocSerialNumber = ++s_mallocSerialNumber;
    trailer->mallocFrom         = (uintptr_t)context;
    trailer->flags.isFree       = false;
    trailer->flags.lastOperation= lastOperation;
    trailer->userAuxiliaryValue = g_userAuxiliaryValue;
    trailer->callStackSum       = pageMallocEntry->callStackSum;
    trailer->nFrames            = recordedFrames;

    s_byteCount += requestedSize;

    if (LOP_MALLOC == lastOperation)
        ++s_mallocCount;

    callStackFrame  = pageMallocEntry->callStackFrame;
    returnAddress   = trailer->returnAddress;

    for (i = 0; i < recordedFrames; i++)
        returnAddress [i] = callStackFrame [i].returnAddress;

    addToMallocedList (header);

    #ifdef NOTNOW
    //  If malloc failed
    if (!byteP)
        {
        pagalloc_printf (PAGALLOC_SEVERITY_ERROR, "PAGALLOC: VirtualAlloc commit failed -- try increasing pagefile size\r\n");

        if (s_pageDebugAllocBreak)
            DebugBreak();

        if (s_pageDebugAllocDump)
            pagalloc_dumpSince(s_pageDebugAllocDump);

        /* only release if not on small block heap */
        if (commitSize != s_systemInfo.dwPageSize)
            {
            VirtualFree(allocP, 0, MEM_RELEASE);
            pagallocI_leaveHeaderVisible();
            }
        else
            pagallocI_makeHeaderInvisible(headerP, PAGE_NOACCESS, ___);

        pagallocI_freeHook();
        return  NULL;
        }

    if (s_pageDebugTraceLog)
        pagalloc_logOutputHdr (PAGALLOC_SEVERITY_TRACE, headerP);

    if (s_pageDebugDumpModules && !(s_mallocSerialNumber % s_pageDebugDumpModules))
        pagalloc_dumpModules();
    #endif

    return retval;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    02/96
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void * __cdecl pagallocI_malloc
(
size_t                      size,
void          const * const context,
unsigned char const         lastOperation,
void *                      callingFunc //=> Function that called this one: new, malloc, etc.
)
    {
    size_t              allocSize=0;
    size_t              commitSize=0;
    size_t              initialOffset=0;
    char               *allocP=0;
    PageMallocEntry    *headerP;
    Int32               iHeap = 0x7fffffff; /* big positive number */

    initializePagAlloc ();

    /* MSVCRT makes zero byte allocations into one byte */
    if (!size)
        size = 1;

    if (PAGALLOC_LITE == s_usingLite)
        {
        Int32               recordedFrames = 0;
        PageMallocEntry*    tempEntry = getNextHeader ();
        void*               retval;

        if (s_pageDebugCallStack)
            //  Do this now to avoid getting pagallocI_malloc in the trace
            recordedFrames = pagallocI_recordCallingStack (tempEntry, callingFunc);

        retval = pagAllocLiteI_malloc (size, context, lastOperation, tempEntry, recordedFrames);
        return retval;
        }

    if (PAGALLOC_MINIMAL == s_usingLite)
        return HeapAlloc (s_hPagAllocHeap, 0, size);

    pagallocI_pageModeCalc (size, &allocSize, &commitSize, &initialOffset);
    if (commitSize == s_systemInfo.dwPageSize)
        {
        allocP = (char*)pagallocI_sbReserve(&iHeap);
        }
    else
        {
        s_largeBlockCount++;
        allocP = (char*)VirtualAlloc(NULL, allocSize, MEM_RESERVE, PAGE_NOACCESS);
        }

    if (allocP)
        {
        char *byteP;
        headerP = (PageMallocEntry *)VirtualAlloc(allocP, s_systemInfo.dwPageSize, MEM_COMMIT, PAGE_NOACCESS);

        if (!headerP)
            {
            pagalloc_printf (PAGALLOC_SEVERITY_ERROR, "PAGALLOC: VirtualAlloc commit failed -- try increasing pagefile size\r\n");
            if (s_pageDebugAllocBreak)
                DebugBreak();

            if (s_pageDebugAllocDump)
                pagalloc_dumpSince(s_pageDebugAllocDump);

            /* only release if not on small block heap */
            if (commitSize != s_systemInfo.dwPageSize)
                VirtualFree(allocP, 0, MEM_RELEASE);

            pagallocI_freeHook();
            return  NULL;
            }

        pagallocI_makeHeaderVisible(headerP, PAGE_READWRITE, ___);
        pagallocI_fill(allocP, s_systemInfo.dwPageSize);
        memset (headerP, 0, sizeof *headerP);
        headerP->signature          = PAGE_SIGNATURE;
        headerP->userSize           = size;
        headerP->allocSize          = allocSize;
        headerP->commitSize         = commitSize;
        headerP->initialOffset      = initialOffset;
        headerP->mallocSerialNumber = ++s_mallocSerialNumber;
        headerP->mallocFrom         = (uintptr_t)context;
        headerP->allocStart         = allocP;
        headerP->iHeap              = iHeap;
        headerP->flags.isFree       = false;
        headerP->flags.lastOperation= lastOperation;
        headerP->userAuxiliaryValue = g_userAuxiliaryValue;
        s_byteCount += size;

        if (LOP_MALLOC == lastOperation)
            ++s_mallocCount;

        /*---------------------------------------------------------------
        Optionally fill in call stack
        ---------------------------------------------------------------*/
        if (s_pageDebugCallStack)
            pagallocI_recordCallingStack (headerP, callingFunc);

        /*---------------------------------------------------------------
        skip over the header and/or low guard page
        ---------------------------------------------------------------*/
        if (s_pageDebugHeadersAlwaysVisible)
            byteP = allocP + (s_systemInfo.dwPageSize<<1); /* two pages: header + guard */
        else
            byteP = allocP + (s_systemInfo.dwPageSize); /* one page: header page */
        /*---------------------------------------------------------------
        commit and init the pages for user data
        ---------------------------------------------------------------*/
        byteP = (char*) VirtualAlloc(byteP, commitSize, MEM_COMMIT, PAGE_READWRITE);
        if (!byteP)
            {
            pagalloc_printf (PAGALLOC_SEVERITY_ERROR, "PAGALLOC: VirtualAlloc commit failed -- try increasing pagefile size\r\n");

            if (s_pageDebugAllocBreak)
                DebugBreak();

            if (s_pageDebugAllocDump)
                pagalloc_dumpSince(s_pageDebugAllocDump);

            /* only release if not on small block heap */
            if (commitSize != s_systemInfo.dwPageSize)
                {
                VirtualFree(allocP, 0, MEM_RELEASE);
                pagallocI_leaveHeaderVisible();
                }
            else
                pagallocI_makeHeaderInvisible(headerP, PAGE_NOACCESS, ___);
            pagallocI_freeHook();
            return  NULL;
            }
        pagallocI_fill(byteP, commitSize);
        /*---------------------------------------------------------------
        return pointer to start of user data
        ---------------------------------------------------------------*/
        if (s_pageDebugLow)
            {
            headerP->dataStart = byteP;
            }
        else
            {
            headerP->dataStart = byteP + initialOffset;
            }
        byteP = (char*)headerP->dataStart;

        if (s_pageDebugTraceLog)
            pagalloc_logOutputHdr (PAGALLOC_SEVERITY_TRACE, headerP);

        pagallocI_makeHeaderInvisible(headerP, PAGE_NOACCESS, ___);

        if (s_pageDebugDumpModules && !(s_mallocSerialNumber % s_pageDebugDumpModules))
            pagalloc_dumpModules();
        return (void *) byteP;
        }
    else
        {
        static Int32 warnOnce;

        if (!warnOnce)
            {
            pagalloc_printf (PAGALLOC_SEVERITY_ERROR, "PAGALLOC: VirtualAlloc reserve failed -- address space may be full\r\n");
            warnOnce = 1;
            }

        if (s_pageDebugAllocBreak)
            DebugBreak();

        if (s_pageDebugAllocDump)
            pagalloc_dumpSince(s_pageDebugAllocDump);

        return NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
Int32 __cdecl     pagallocI_fillCheck
(
char *dataP,
size_t size
)
    {
    const uintptr_t  initialOffset   = (uintptr_t)dataP % s_systemInfo.dwPageSize;
    char const * const  initialFillP    = s_fillDataP + initialOffset;
    const uintptr_t  onFirstPage     = ((s_systemInfo.dwPageSize - initialOffset) % s_systemInfo.dwPageSize);
    const uintptr_t wholePages      = size / s_systemInfo.dwPageSize;
    const uintptr_t  onLastPage      = (size - (wholePages * s_systemInfo.dwPageSize) - onFirstPage) ;
    uintptr_t   page;
    Int32                       iMemCmp         = 0;

    if (onFirstPage)
        {
        iMemCmp = memcmp(dataP, initialFillP, onFirstPage);
        if (iMemCmp)
            return  !iMemCmp;
        dataP += onFirstPage;
        }

    for (page = 0; page < wholePages; page++)
        {
        iMemCmp = memcmp(dataP, s_fillDataP, s_systemInfo.dwPageSize);
        if (iMemCmp)
            return  !iMemCmp;
        dataP += s_systemInfo.dwPageSize;
        }

    if (onLastPage)
        iMemCmp = memcmp(dataP, s_fillDataP, onLastPage);

    return  iMemCmp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void __cdecl    pagAllocLiteI_free
(
void * const    voidP,
void const * const context
)
    {
    char*       sizedData = (char*)voidP;
    HeapLiteHeader *header;
    HeapLiteTrailer*trailer;
    size_t         requestedSize;
    UInt32         userDataSize;

    if (!voidP)
        return; /*ANSI made me do it! */

    header = (HeapLiteHeader*)(sizedData - sizeof (HeapLiteHeader));

    if (PAGE_SIGNATURE != header->signature)
        {
        pagalloc_printf (PAGALLOC_SEVERITY_WARNING, "PAGALLOC: attempt to free non-pagalloced address 0x%p\r\n", voidP);

        if (pagallocFreeHandler)
            //  VCRT has internal calls that allocate data using the functions that malloc calls, but
            //  frees the data using free.  For those, we intercept the free but not the allocation.
            //  This gives the C runtime a chance to free it.
            pagallocFreeHandler (sizedData);

        if (s_pageDebugAddressBreak)
            DebugBreak();

        pagallocI_freeHook();
        return;
        }

    getLiteInfo (&header, (UInt32*)&userDataSize, &trailer, voidP);
    requestedSize = header->requestedSize;

    s_byteCount -= requestedSize;

    if (trailer->flags.isFree)
        {
        PageMallocEntry*    headerP = (PageMallocEntry*)pagallocI_findHeader (sizedData);
        pagalloc_printf (PAGALLOC_SEVERITY_WARNING, "PAGALLOC: attempt to free already freed memory 0x%p\r\n", voidP);

        pagalloc_dumpHeader (headerP);
        pagalloc_symbolizeHeaderAddressesForVisualC (headerP);

        DebugBreak();
        return;         // Don't process any more checks
        }

    for (; requestedSize < userDataSize; requestedSize++)
        {
        if (PAGALLOC_LITE_FILLER_CHAR != sizedData [requestedSize])
            {
            pagalloc_printf (PAGALLOC_SEVERITY_WARNING, "PAGALLOC: data changed outside block detected during free 0x%p\r\n", voidP);
            DebugBreak();
            break;
            }
        }

    if (PAGE_SIGNATURE != trailer->signature)
        {
        pagalloc_printf (PAGALLOC_SEVERITY_WARNING, "PAGALLOC: data changed outside block detected during free 0x%p\r\n", voidP);
        DebugBreak();
        }

    removeFromMallocedList (header);
    trailer->flags.isFree = true;

    //  May want to keep a list of these on a freed list.
    HeapFree (s_hPagAllocHeap, 0, header);
    s_freeCount++;

    pagallocI_freeHook();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    02/96
+---------------+---------------+---------------+---------------+---------------+------*/
void __cdecl pagallocI_free
(
void * const voidP,
void const * const context,
void * callingFunc
)
    {
    PageMallocEntry *headerP;

    if (!voidP)
        return; /*ANSI made me do it! */

    if (PAGALLOC_MINIMAL == s_usingLite)
        {
        Int32 retval = HeapFree (s_hPagAllocHeap, 0, voidP);

        if (0 == retval)
            {
            //  This is an allocation from the C runtime heap that we could not intercept.
            //  Free it via the original free.
            if (pagallocFreeHandler)
                pagallocFreeHandler (voidP);
            }

        return;
        }

    if (PAGALLOC_LITE == s_usingLite)
        {
        pagAllocLiteI_free (voidP, context);
        return;
        }

    headerP = (PageMallocEntry *)pagallocI_findHeader(voidP);
    /*-------------------------------------------------------------------
    check signature and fill characters
    -------------------------------------------------------------------*/
    if (pagallocI_openHeader(headerP)) /* leaves header visible */
        {
        bool ok;

        if (headerP->dataStart != voidP)
            {
            pagalloc_printf (PAGALLOC_SEVERITY_WARNING, "PAGALLOC: attempt to free non-pagalloced address 0x%p\r\n", voidP);

            if (s_pageDebugAddressBreak)
                DebugBreak();
            }

        if (headerP->flags.isFree)
            {
            pagalloc_printf (PAGALLOC_SEVERITY_WARNING, "PAGALLOC: attempt to free already freed memory 0x%p\r\n", voidP);

            pagalloc_dumpHeader (headerP);
            pagalloc_symbolizeHeaderAddressesForVisualC (headerP);

            DebugBreak();
            return;         // Don't process any more checks
            }

        if (s_pageDebugFreeCheck)
            {
            byte *startPtr = (byte *)headerP->dataStart;
            byte *endPtr = startPtr + headerP->userSize;
            size_t startSize;
            size_t endOffset = headerP->userSize;
            size_t endSize;

            if (s_pageDebugLow) /* top justified data */
                {
                /* nothing to check at beginning */
                startSize = 0;
                }
            else /* bottom justified data */
                {
                startPtr -= headerP->initialOffset; /* check at beginning  */
                startSize = headerP->initialOffset;
                endOffset += startSize;
                }
            if (startSize)
                if (pagallocI_fillCheck((char*)startPtr, startSize))
                    {
                    pagalloc_printf (PAGALLOC_SEVERITY_ERROR, "PAGALLOC: memory modified before block of size %d beginning at 0x%p\r\n", headerP->userSize, headerP->dataStart);
                    if (s_pageDebugTraceLog)
                        pagalloc_logOutputHdr (PAGALLOC_SEVERITY_ERROR, headerP);
                    DebugBreak();
                    }

                endSize = (s_systemInfo.dwPageSize - (endOffset % s_systemInfo.dwPageSize)) % s_systemInfo.dwPageSize;
                if (endSize)
                    {
                    if (pagallocI_fillCheck((char*)endPtr, endSize))
                        {
                        pagalloc_printf (PAGALLOC_SEVERITY_ERROR, "PAGALLOC: memory modified beyond block of size %d beginning at 0x%p\r\n", headerP->userSize, headerP->dataStart);
                        if (s_pageDebugTraceLog)
                            pagalloc_logOutputHdr (PAGALLOC_SEVERITY_ERROR, headerP);
                        DebugBreak();
                        }
                    }

                /*---------------------------------------------------------------
                Optionally fill in call stack
                ---------------------------------------------------------------*/
                if (s_pageDebugCallStack  ||  s_pageDebugCheckDoubleFree)
                    {
                    UInt32 fdwOldProtectReadOnly = PAGE_READONLY;
                    UInt32 fdwOldProtectReadWrite = PAGE_READWRITE;

                    /* already have header visible and have s_csVisible from checkHeader, so just need to temporarily make writable */
                    VirtualProtect (headerP, s_systemInfo.dwPageSize, PAGE_READWRITE, (PDWORD)&fdwOldProtectReadOnly);
                    pagallocI_recordCallingStack (headerP, callingFunc);
                    VirtualProtect (headerP, s_systemInfo.dwPageSize, fdwOldProtectReadOnly, (PDWORD)&fdwOldProtectReadWrite);
                    }
            }

        s_byteCount -= headerP->userSize;

        /*---------------------------------------------------------------
        free memory
        ---------------------------------------------------------------*/
        if (s_pageDebugTraceLog)
            pagalloc_logOutputHdr (PAGALLOC_SEVERITY_TRACE, headerP);

        if (headerP->commitSize == s_systemInfo.dwPageSize)
            {
            ok = pagallocI_sbDecommit(headerP);
            }
        else
            {
            s_largeBlockCount--;
            if (s_pageDebugNoRelease)
                ok = VirtualFree(headerP->allocStart, 0, MEM_DECOMMIT) !=0;
            else
                ok = VirtualFree(headerP->allocStart, 0, MEM_RELEASE)!=0;
            }
        /* see if there's anything specific to do after each free */
        pagallocI_freeHook();
        if (!ok)
            {
            pagalloc_printf (PAGALLOC_SEVERITY_ERROR, "PAGALLOC: VirtualFree or sbDecommit of header at 0x%p failed\r\n", headerP);
            DebugBreak();
            }
        else
            {
            s_freeCount++;
            }

        /* special case: need to leave the s_csVisible critical section here
        * rather than calling pagallocI_closeHeader because there is no
        * header to close or make invisible (its memory has been freed ) */
        pagallocI_leaveHeaderVisible();
        }
    else
        {
        pagalloc_printf (PAGALLOC_SEVERITY_WARNING,  "PAGALLOC: attempting to free non-pagalloced memory at 0x%p\r\n", voidP);

        if (s_pageDebugFreeBreak)
            DebugBreak();

        /* we will try using handler to free */
        if (pagallocFreeHandler)
            pagallocFreeHandler (voidP);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    02/96
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void * __cdecl pagallocI_realloc
(
void          * const oldDataP,
size_t          const newSize,
void    const * const context
)
    {
    PageMallocEntry *headerP;

    s_reallocCount++;
    if (!oldDataP)
        {
        if (newSize)
            return pagallocI_malloc(newSize, context, LOP_REALLOC, pagalloc_realloc);
        else
            return NULL;
        }

    if (!newSize)
        {
        pagallocI_free (oldDataP, context, pagallocI_realloc);
        return  NULL;
        }

    if (PAGALLOC_MINIMAL == s_usingLite)
        {
        void        *newP = pagallocI_malloc(newSize, context, LOP_REALLOC, pagalloc_realloc);
        size_t   oldSize = HeapSize (s_hPagAllocHeap, 0, oldDataP);

        memcpy (newP, oldDataP, (size_t)(oldSize) >= newSize ? newSize : oldSize);

        pagallocI_free(oldDataP, context, pagallocI_realloc);

        return newP;
        }

    headerP = (PageMallocEntry*)pagallocI_findHeader(oldDataP);
    if (pagallocI_openHeader(headerP)) /* leaves header visible */
        {
        void        *newP = pagallocI_malloc(newSize, context, LOP_REALLOC, pagalloc_realloc);
        if (newP)
            {
            memcpy (newP, headerP->dataStart,
                (size_t)(headerP->userSize) >= newSize ? newSize : headerP->userSize);
            pagallocI_closeHeader(headerP);
            pagallocI_free(oldDataP, context, pagallocI_realloc);
            return newP;
            }
        }
    else
        {
        pagalloc_printf (PAGALLOC_SEVERITY_WARNING, "PAGALLOC: attempting to realloc non-pagalloced memory at 0x%p\r\n", oldDataP);
        if (s_pageDebugReallocBreak)
            DebugBreak();

        /* we will try using handlers to realloc */
        if (pagallocMsizeHandler)
            {
            size_t oldSize = pagallocMsizeHandler(oldDataP);

            if (oldSize)
                {
                void *newP = pagallocI_malloc(newSize, context, LOP_REALLOC, pagalloc_realloc);
                if (newP)
                    {
                    memcpy (newP, oldDataP, (oldSize >= newSize) ? newSize : oldSize);
                    if (pagallocFreeHandler)
                        pagallocFreeHandler(oldDataP);
                    return newP;
                    }
                }
            /* couldn't realloc even using handlers */
            if (s_pageDebugReallocBreak)
                DebugBreak();
            }
        }
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    02/96
+---------------+---------------+---------------+---------------+---------------+------*/
void * __cdecl pagallocI_expand
(
void          * const oldDataP,
size_t          const newsize,
void    const * const context
)
    {
#ifdef PAGALLOC_IMPLEMENTS_EXPAND
    void            *newP = NULL;
    char            *pageP;
    PageMallocEntry *headerP;

    headerP = pagallocI_findHeader(oldDataP);
    if (pagallocI_openHeader(headerP)) /* leaves header visible */
        {
        UInt32 allocSize;
        UInt32 commitSize;
        UInt32 initialOffset;
        UInt32 oldProtect = 0;

        pagallocI_pageModeCalc(newsize, &allocSize, &commitSize, &initialOffset);

        /* expanding into or out of small block heap is not possible */
        if (((headerP->commitSize == s_systemInfo.dwPageSize) &&
            (         commitSize != s_systemInfo.dwPageSize)) ||
            ((headerP->commitSize != s_systemInfo.dwPageSize) &&
            (         commitSize  == s_systemInfo.dwPageSize)))
            {
            pagalloc_printf (PAGALLOC_SEVERITY_WARNING, "PAGALLOC: could not expand memory at 0x%p\r\n", oldDataP);
            if (s_pageDebugExpandBreak)
                DebugBreak();
            pagallocI_closeHeader(headerP);
            return  NULL;
            }

        /* check if expanded reservation is possible */
        if ((commitSize != s_systemInfo.dwPageSize) && (allocSize > headerP->allocSize))
            {
            newP = VirtualAlloc(headerP, allocSize, MEM_RESERVE, PAGE_READWRITE);
            if (!newP)
                {
                pagalloc_printf(PAGALLOC_SEVERITY_WARNING, "PAGALLOC: could not expand reservation for memory at 0x%p\r\n", oldDataP);
                if (s_pageDebugExpandBreak)
                    DebugBreak();
                pagallocI_closeHeader(headerP);
                return  NULL;
                }
            }

        /* prepare to update header */
        pageP = headerP->allocStart;
        if (s_pageDebugHeadersAlwaysVisible)
            pageP += s_systemInfo.dwPageSize<<1; /* advance two pages: header + guard */
        else
            pageP += s_systemInfo.dwPageSize; /* advance one page: header */

        /* attempt to expand commitment */
        if ((commitSize != s_systemInfo.dwPageSize) && (commitSize > headerP->commitSize))
            {
            newP = VirtualAlloc(pageP, commitSize, MEM_COMMIT, PAGE_READWRITE);
            if (!newP)
                {
                pagalloc_printf(PAGALLOC_SEVERITY_WARNING, "PAGALLOC: could not expand commitment for memory at 0x%p\r\n", oldDataP);
                if (s_pageDebugExpandBreak)
                    DebugBreak();
                pagallocI_closeHeader(headerP);
                return  NULL;
                }
            }

        /* shrink commitment if needed */
        if ((commitSize != s_systemInfo.dwPageSize) && (commitSize < headerP->commitSize))
            {
            bool ok;
            pageP += commitSize;
            ok = VirtualFree(pageP, headerP->commitSize - commitSize, MEM_DECOMMIT) != 0;
            if (!ok)
                pagalloc_printf (PAGALLOC_SEVERITY_WARNING, "PAGALLOC: warning: could not shrink commitment for memory at 0x%p\r\n", oldDataP);
            }

        /* update header and tally */
        pagallocI_makeHeaderVisible(headerP, PAGE_READWRITE, &oldProtect);
        headerP->userSize = newsize;
        headerP->allocSize = allocSize;
        headerP->commitSize = commitSize;
        headerP->mallocFrom = (uintptr_t)context; /* update context for expand */
        pagallocI_makeHeaderInvisible(headerP, oldProtect, ___);

        s_byteCount += (newsize - headerP->userSize); /* update tally */

        /* restore guard status of header */
        pagallocI_closeHeader(headerP);
        return oldDataP;
        }
    else
        {
        pagalloc_printf (PAGALLOC_SEVERITY_WARNING, "PAGALLOC: denied request to expand non-pagalloced memory at 0x%p\r\n", oldDataP);
        if (s_pageDebugExpandBreak)
            DebugBreak();
        }
#endif /* PAGALLOC_IMPLEMENTS_EXPAND */
    return  NULL;
    }
/*-----------------------------------------------------------------------
Exported functions
-----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    02/96
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API void * __cdecl pagalloc_malloc (size_t const size)
    {
    return  pagallocI_malloc(size, PAGALLOC_CONTEXT, LOP_MALLOC, pagalloc_malloc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void * __cdecl pagalloc_nh_malloc (size_t const size, Int32 const nhFlag)
    {
    return  pagallocI_malloc(size, PAGALLOC_CONTEXT, LOP_NH_MALLOC, pagalloc_nh_malloc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    12/95
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void * __cdecl pagalloc_calloc (size_t const num, size_t const size)
    {
    size_t    const totalSize = num * size;
    void    * const ptr       = pagallocI_malloc(totalSize, PAGALLOC_CONTEXT, LOP_CALLOC, pagalloc_calloc);
    if (ptr)
        memset(ptr, 0, totalSize);
    s_callocCount++;
    return ptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    02/96
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void __cdecl pagalloc_free(void *voidP)
    {
    pagallocI_free (voidP, PAGALLOC_CONTEXT, pagalloc_free);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    02/96
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void * __cdecl pagalloc_realloc
(
void  *oldDataP,
size_t newSize
)
    {
    return  pagallocI_realloc(oldDataP, newSize, PAGALLOC_CONTEXT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    02/96
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void * __cdecl pagalloc_expand
(
void *oldDataP,
size_t newsize
)
    {
    return  pagallocI_expand(oldDataP, newsize, PAGALLOC_CONTEXT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    12/95
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
size_t __cdecl pagalloc_msize
(
void *         pblck
)
    {
    PageMallocEntry *headerP;
    size_t           size = 0;

    if (PAGALLOC_MINIMAL == s_usingLite)
        return HeapSize (s_hPagAllocHeap, 0, pblck);

    headerP = (PageMallocEntry*)pagallocI_findHeader(pblck);
    if (pagallocI_openHeader(headerP)) /* leaves header visible */
        {
        size = headerP->userSize;
        /* restore guard status of header */
        pagallocI_closeHeader(headerP);
        }
    else
        {
        pagalloc_printf (PAGALLOC_SEVERITY_WARNING, "PAGALLOC: attempting to msize non-pagalloced memory at 0x%p\r\n", pblck);

        if (s_pageDebugMsizeBreak)
            DebugBreak();
        if (pagallocMsizeHandler)  //  Needed to handle C runtime allocations that use the same functions malloc calls.
            size = pagallocMsizeHandler(pblck);
        }
    return size;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API char * __cdecl pagalloc_strdup
(
char const * const string
)
    {
    if (string)
        {
        size_t      const size = 1 + strlen(string);
        char  * const copy = (char*)pagallocI_malloc(size, PAGALLOC_CONTEXT, LOP_STRDUP, pagalloc_strdup);
        if (copy)
            memcpy (copy, string, size);
        s_strdupCount++;
        return      copy;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   11/2001
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API wchar_t * __cdecl pagalloc_wcsdup
(
wchar_t const * const string
)
    {
    size_t     const size = (1 + wcslen(string)) * 2;
    wchar_t  * const copy = (wchar_t*)pagallocI_malloc(size, PAGALLOC_CONTEXT, LOP_WCSDUP, pagalloc_wcsdup);
    if (copy)
        memcpy(copy, string, size);
    s_wcsdupCount++;
    return  copy;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   07/2001
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API void * __cdecl pagalloc_operatorNew
(
size_t size
)
    {
    s_operatorNewCount++;
    return  pagallocI_malloc(size, PAGALLOC_CONTEXT, LOP_OPERNew, pagalloc_operatorNew);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   07/2001
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API void __cdecl pagalloc_operatorDelete
(
void * const voidP
)
    {
    s_operatorDeleteCount++;
    pagallocI_free(voidP, PAGALLOC_CONTEXT, pagalloc_operatorDelete);
    }



/*-----------------------------------------------------------------------
These functions set handler functions for free and msize failures
-----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void __cdecl pagalloc_set_free_handler(CRuntimeFreeFunc freeHandler)
    {
    pagallocFreeHandler = freeHandler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API void __cdecl pagalloc_set_msize_handler(CRuntimeMsizeFunc msizeHandler)
    {
    pagallocMsizeHandler = msizeHandler;
    }

/*-----------------------------------------------------------------------
Diagnostic function (e.g., call this from debugger after faulting)
-----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    12/95
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API void __cdecl pagalloc_dump
(
void *badPtr
)
    {
    PageMallocEntry *headerP;
    char *charPtr = (char *)badPtr;

    if (s_pageDebugLow)
        charPtr += s_systemInfo.dwPageSize;
    else
        charPtr -= s_systemInfo.dwPageSize;
    headerP = (PageMallocEntry*)pagallocI_findHeader(charPtr);
    if (pagallocI_openHeader(headerP)) /* leaves header visible */
        {
        pagalloc_printf(PAGALLOC_SEVERITY_DEBUG,
            "PAGALLOC: bad pointer: \t0x%p\r\nnearest good pointer: \t0x%p\r\nbytes allocated: \t0x%p\r\ncode address where allocated: \t0x%p\r\n",
            badPtr, headerP->dataStart, headerP->userSize, headerP->mallocFrom);
        pagallocI_closeHeader(headerP);
        }
    else
        {
        pagalloc_printf(PAGALLOC_SEVERITY_DEBUG,
            "PAGALLOC: bad pointer: \t0x%p\r\nCannot find nearest good pointer.  Pointer may be stale (i.e., attempt to reference freed memory).\r\n",
            badPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/98
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void __cdecl pagalloc_dumpModuleHeader
(
void *allocationBase,
size_t regionSize
)
    {
    char const * const moduleBaseP = (char *)allocationBase;
    IMAGE_DOS_HEADER const * const imageDosHeaderP = (IMAGE_DOS_HEADER *)moduleBaseP;

    if (sizeof(IMAGE_DOS_HEADER) > regionSize)
        return;

    if (IMAGE_DOS_SIGNATURE == imageDosHeaderP->e_magic)
        {
        IMAGE_NT_HEADERS *imageNTHeadersP =  (IMAGE_NT_HEADERS *)(moduleBaseP + imageDosHeaderP->e_lfanew);
        if (!IsBadReadPtr(imageNTHeadersP, sizeof(IMAGE_NT_HEADERS)) &&
            (IMAGE_NT_SIGNATURE == imageNTHeadersP->Signature))
            {
            uintptr_t imageBase =   imageNTHeadersP->OptionalHeader.ImageBase;
            size_t sizeOfImage = imageNTHeadersP->OptionalHeader.SizeOfImage;
            char moduleName[512] = "";

            GetModuleFileName ((HMODULE)allocationBase, moduleName, sizeof(moduleName));

            pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: hmodule: 0x%p base: 0x%p size: 0x%p name: %s\r\n", moduleBaseP, imageBase, sizeOfImage, moduleName);
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/98
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void __cdecl pagalloc_dumpModules
(
void
)
    {
    char *pMem;
    char       * const pMin = (char*)s_systemInfo.lpMinimumApplicationAddress;
    char const * const pMax = (char*)s_systemInfo.lpMaximumApplicationAddress;
    MEMORY_BASIC_INFORMATION mbi;

    for (pMem = pMin; pMem < pMax; pMem += mbi.RegionSize)
        {
        size_t cb;

        memset(&mbi, 0, sizeof(mbi));
        cb = VirtualQuery(pMem, &mbi, sizeof(mbi));

        /*-----------------------------------------------------------
        Only interested in committed, accessible, image pages.
        Make sure that we don't touch guard pages because that will
        break Windows' automatic stack growth.
        -----------------------------------------------------------*/
        if  (        mbi.Protect &  ( PAGE_NOACCESS | PAGE_GUARD | PAGE_NOCACHE) )
            continue;
        if  ( ! (mbi.Protect &        PAGE_READONLY ) )
            continue;

        if ( ! (mbi.State    &  MEM_COMMIT) )
            continue;

        if (mbi.AllocationBase !=  mbi.BaseAddress)
            continue;

        pagalloc_dumpModuleHeader(mbi.BaseAddress, mbi.RegionSize);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/98
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void __cdecl pagalloc_dumpHeader
(
PageMallocEntry const * const headerP
)
    {
    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG,
    "PAGALLOC: pointer: 0x%p size: %d id: %d stack: 0x%p 0x%p 0x%p 0x%p 0x%p 0x%p 0x%p 0x%p 0x%p 0x%p 0x%p 0x%p 0x%p 0x%p\r\n",
    headerP->dataStart, headerP->userSize, headerP->mallocSerialNumber,
    headerP->mallocFrom, headerP->callStackFrame[0].returnAddress,
    headerP->callStackFrame[ 1].returnAddress, headerP->callStackFrame[ 2].returnAddress,
    headerP->callStackFrame[ 3].returnAddress, headerP->callStackFrame[ 4].returnAddress,
    headerP->callStackFrame[ 5].returnAddress, headerP->callStackFrame[ 6].returnAddress,
    headerP->callStackFrame[ 7].returnAddress, headerP->callStackFrame[ 8].returnAddress,
    headerP->callStackFrame[ 9].returnAddress, headerP->callStackFrame[10].returnAddress,
    headerP->callStackFrame[11].returnAddress, headerP->callStackFrame[12].returnAddress);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void __cdecl    dumpSinceLite
(
UInt32 count
)
    {
    HeapLiteHeader*   header;

    if (!pTryEnterCriticalSection(&s_csHeap))
        {
        InterlockedIncrement ((volatile Long32 *)&s_waitCount);
        EnterCriticalSection (&s_csHeap);
        }

    for (header = s_heapList; NULL != header; header = header->next)
        {
        //  TODO_optimize We could optimize this by changing the logic to work with
        //  the lite data structures instead of generating the header on fly.
        PageMallocEntry *fullHeader = (PageMallocEntry*)pagallocI_findHeader ((char*)header + sizeof (*header));

        __try
            {
            if ((PAGE_SIGNATURE == fullHeader->signature) &&
                (count          <= fullHeader->mallocSerialNumber))
                {
                pagalloc_dumpHeader(fullHeader);
                }
            }
        __except(EXCEPTION_EXECUTE_HANDLER)
            {
            ;
            }
        }

    LeaveCriticalSection(&s_csHeap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void __cdecl    dumpSinceFull
(
UInt32 count
)
    {
    char              *pMem;
    char       * const pMin = (char*)s_systemInfo.lpMinimumApplicationAddress;
    char const * const pMax = (char*)s_systemInfo.lpMaximumApplicationAddress;
    MEMORY_BASIC_INFORMATION mbi;
    UInt32 fdwProtect = s_pageDebugHeadersAlwaysVisible ? PAGE_READWRITE : PAGE_NOACCESS;

    EnterCriticalSection(&s_csDump);
    for (pMem = pMin; pMem < pMax; pMem += mbi.RegionSize)
        {
        size_t cb;
        UInt32 fdwOldProtect = PAGE_NOACCESS;

        memset(&mbi, 0, sizeof(mbi));
        cb = VirtualQuery(pMem, &mbi, sizeof(mbi));
        if  (mbi.Protect & (PAGE_GUARD | PAGE_NOCACHE) )
            continue;

        if (MEM_COMMIT == mbi.State)
            {
            if (fdwProtect == mbi.Protect) /* might be PAGALLOC header */
                {
                PageMallocEntry *headerP = (PageMallocEntry*)mbi.BaseAddress;

                /* make invisible headers visible */
                pagallocI_makeHeaderVisible(headerP, PAGE_READONLY, &fdwOldProtect);
                __try
                    {
                    if ((PAGE_SIGNATURE == headerP->signature) &&
                        (count              <= headerP->mallocSerialNumber))
                        {
                        pagalloc_dumpHeader(headerP);
                        }
                    }
                __except(EXCEPTION_EXECUTE_HANDLER)
                    {
                    ;
                    }
                /* leave protection exactly how we found it */
                pagallocI_makeHeaderInvisible(headerP, fdwOldProtect, ___);
                }
            if (PAGE_READONLY == mbi.Protect) /* might be module header */
                {
                pagalloc_dumpModuleHeader(mbi.BaseAddress, mbi.RegionSize);
                }
            }
        }
    LeaveCriticalSection(&s_csDump);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/98
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void __cdecl pagalloc_dumpSince
(
UInt32 count
)
    {
    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: dumpSince: %u\r\n", count);

    if (s_usingLite)
        {
        dumpSinceLite (count);
        return;
        }

    dumpSinceFull (count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/98
+---------------+---------------+---------------+---------------+---------------+------*/
ULong32 WINAPI pagalloc_dumpSinceThreadProc
(
LPVOID lpParameter  // This is a UInt32 put into a pointer
)
    {
    pagalloc_dumpSince((uintptr_t)lpParameter & 0xffffffff);
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/98
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void __cdecl pagalloc_dumpSinceAsync
(
UInt32 count
)
    {
    ULong32 tid;
    CloseHandle(CreateThread(NULL, 0, pagalloc_dumpSinceThreadProc, (LPVOID)count, 0, &tid));
    }

/*-----------------------------------------------------------------------
specialized freeHook for MicroStation:
-----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
static Int32 pagallocI_initMdlFuncs(void)
    {
#if defined (LMSERVER) || defined (STANDALONE_BSI)
    /* in SELECTserver, calling GetModuleHandle during DLL initialization causes deadlocks. */
    /* in STANDALONE_BSI apps, it doesn't make any sense to get USTATION.DLL's module handle.   */
    /* TODO: see if deadlocks could happen in MicroStation too and try to work around this. */
    return 1;
#else
    static HANDLE   hUstation;
    static Int32      complete;

    if (complete)
        return complete;

    if ( ! hUstation)
        hUstation = GetModuleHandle("USTATION.DLL");

    if (! hUstation)
        return complete;

    if ( ! mdlSystem_getCurrMdlDesc)
        mdlSystem_getCurrMdlDesc = (MdlSystem_getCurrMdlDesc)GetProcAddress((HMODULE)hUstation, "mdlSystem_getCurrMdlDesc");
    
    if (!dlmSystem_callAnyFunction)
        dlmSystem_callAnyFunction = (DlmSystem_callAnyFunction)GetProcAddress((HMODULE)hUstation, "dlmSystem_callAnyFunction");

    return (complete = TRUE);
#endif
    }

/* this function should get called for all free calls */
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    07/98
*  This no longer gets called (it was used for checking the MDL heap). However it's
*  useful to keep so we know the right places to call any hooks in the future.
+---------------+---------------+---------------+---------------+---------------+------*/
static void pagallocI_freeHook(void)
    {
    }



/*----------------------------------------------------------------------+
|                                                                       |
|   Leak Detector functions.                                            |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static char const * const lastOperationToString
(
Int32             lastOperation
)
    {
    char const * const szLastOper = (LOP_CALLOC     == lastOperation) ? "calloc"     :
                    (LOP_NH_MALLOC  == lastOperation) ? "nh_malloc"  :
                    (LOP_MALLOC     == lastOperation) ? "malloc"     :
                    (LOP_OPERNew    == lastOperation) ? "C++ new"    :
                    (LOP_REALLOC    == lastOperation) ? "realloc"    :
                    (LOP_STRDUP     == lastOperation) ? "strdup"     :
                    (LOP_WCSDUP     == lastOperation) ? "_wcsdup"     :
                    (LOP_CriticalSection == lastOperation) ? "CriticalSection"
                        : "Unknown";
    return  szLastOper;
    }


#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static char const * const       pagalloc_toStringLastOperationLite
(
HeapLiteHeader * const header
)
    {
    HeapLiteTrailer*  trailer = getLiteTrailer (header);
    return lastOperationToString (trailer->flags.lastOperation);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mike.Stratoti   10/101
+---------------+---------------+---------------+---------------+---------------+------*/
static char const * const       pagalloc_toStringLastOperation
(
const PageMallocEntry        * const headerP            // =>
)
    {
    return lastOperationToString (headerP->flags.lastOperation);
    }


/*---------------------------------------------------------------------------------**//**
* This function outputs a classical hex dump of a memory block's the user area. This is typically used from the leak detector.
* @bsimethod                                                    Mike.Stratoti   02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pagalloc_dumpFormattedMemory
(
const PageMallocEntry        * const headerP,           // =>
const LeakDetectCandidate    * const leaker             // => [optional]
)
    {
    unsigned char const * const by  = (unsigned char*)headerP->dataStart;
    size_t len = (headerP->userSize > 256) ? 256 : headerP->userSize;
    char  line[200], *lp;
    UInt32 i,j, mlen;
    uintptr_t row;

    pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION,
        "PAGALLOC:\r\n"
        "PAGALLOC:  Root Control block (other non-root blocks may be different):\r\n");

    if (leaker)
        {
        if (leaker->smallestSize == leaker->largestSize )
            pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION, "PAGALLOC:   All blocks are %d bytes long.\r\n", leaker->smallestSize);
        else
            pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION, "PAGALLOC:   Block size ranges from %d to %d bytes long.\r\n", leaker->smallestSize, leaker->largestSize );
        }

    pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION,
        "PAGALLOC:   Pointer 0x%p, user size %d, aligned size %d, sequence %d, origination %s.\r\n"
        "PAGALLOC:\r\n",
        headerP->dataStart,  headerP->userSize, headerP->allocSize, headerP->mallocSerialNumber, pagalloc_toStringLastOperation(headerP));

    pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION,
        "PAGALLOC:  Root Data block contents start with (non-root blocks may be different):\r\n"
        "PAGALLOC:               \\  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F   + 0123456789ABCDEF\r\n");

    for (i=0; i < len; i += 16)
        {
        lp = line;
        memset (line, ' ', 100);
        mlen = wsprintf(lp,"PAGALLOC:      %p :", by + i);
        lp += mlen;

        row =   ((len - i - 1) < 16) ? (len - i) : 16;

        for (j=0; j < row; j++)
            {
            mlen = wsprintf(lp," %02x", by[i+j]);
            lp += mlen;
            }

        *lp = ' ';

        // The const 75 is added to insure that the ASCII dump is always aligned to column for short lines:
        //PAGALLOC:  Root Data block (non-root blocks may be different):        |Column 75
        //PAGALLOC:  008fffe8 : 10 7d fa 77 ff ff ff ff 00 00 00 00 00 00 00 00 : .}.w............
        //Otherwise you will get this:
        //PAGALLOC:  008ffff8 : 00 00 00 00 00 00 00 00 : ........
        lp = line + 75;

        strcpy (lp, " : ");
        lp += 3;

        for (j=0; j < row; j++)
            {
            unsigned char const ch =  by[i+j];
            *lp++ = isprint(ch) ? ch : '.';
            }

        strcpy (lp, "\r\n");
        pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION, line);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
CRTCallback Int32 pagalloc_sortfuncTopMatched
(
const void *p1,
const void *p2
)
    {
    const LeakDetectCandidate **t1 = (const LeakDetectCandidate **)p1;
    const LeakDetectCandidate **t2 = (const LeakDetectCandidate **)p2;

    if (NULL == *t1)
        return   1;
    if (NULL == *t2)
        return   -1;
    if(!s_sortLeakersBySize)
        return  (*t2)->groupMemberCount - (*t1)->groupMemberCount;
    ptrdiff_t sizeDiff = (*t2)->totalSize - (*t1)->totalSize;
    if(sizeDiff > 0)
        return 1;
    if(sizeDiff < 0)
        return -1;
    return 0;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   05/2002
+---------------+---------------+---------------+---------------+---------------+------*/
static PDE_CONTROL     pagalloc_collectUserAuxiliaryValues
(
PageMallocEntry const * const headerP,
void                  * const context
)
    {
    GroupUserAuxiliaryValues    * const guav =  (GroupUserAuxiliaryValues*)context;
    UInt32 i, j;

    if ( ! headerP->userAuxiliaryValue)
        return      PDE_CONTINUE;

    // Look for the matching root
    for (i=0; i < guav->nEntries; i++)
        {
        if ( ! guav->callStackSum[i])
            break;

        if (headerP->callStackSum == guav->callStackSum[i])
            {
            // Found the matching root.  Add userAuxiliaryValue to the first unused slot, uniquely.
            for (j=0; j < _countof(guav->userAuxiliaryValues[0]); j++)
                {
                UInt32 userVal = guav->userAuxiliaryValues[i][j];
                if (headerP->userAuxiliaryValue == userVal)
                    return  PDE_CONTINUE;

                if ( ! userVal)
                    {
                    guav->userAuxiliaryValues[i][j] =  headerP->userAuxiliaryValue;
                    return  PDE_CONTINUE;
                    }
                }

            break;
            }
        }

    return  PDE_CONTINUE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pagalloc_printTopLeakers
(
LeakDetectCandidate     ** const topMatches,
Int32                      const maxTopMatches
)
    {
    UInt32              i;
    GroupUserAuxiliaryValues    groupUserAuxiliaryValues;

    qsort (topMatches, maxTopMatches, sizeof (LeakDetectCandidate *), pagalloc_sortfuncTopMatched);

    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC:\r\nPAGALLOC: Leak summary ************************************\r\n");

    for (i=0; i < g_topMatchesToPrint; i++)
        {
        if (NULL == topMatches[i]  ||  topMatches[i]->groupMemberCount <= 0)
            continue;

        pagalloc_printf(PAGALLOC_SEVERITY_DEBUG,
            "PAGALLOC: %2d. Root 0x%p appeared%6d time%s totaling %6d bytes.\r\n",
            i+1, topMatches[i]->detail.headerP, topMatches[i]->groupMemberCount, (1<topMatches[i]->groupMemberCount)? "s" : " ", topMatches[i]->totalSize);
        }


    /*-------------------------------------------------------------------
    Find all  user auxiliary values for all group members of the top leakers.
    groupUserAuxiliaryValues is parallels the topMatches array.
    -------------------------------------------------------------------*/

    if (g_userAuxiliaryValue)
        {
        pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC:\r\nPAGALLOC: Collecting User Auxiliary Values.\r\nPAGALLOC:\r\n");

        memset (&groupUserAuxiliaryValues, 0, sizeof(groupUserAuxiliaryValues));
        groupUserAuxiliaryValues.nEntries = g_topMatchesToDetail;
        for (i=0; i < g_topMatchesToDetail; i++)
            if (topMatches[i]  &&  topMatches[i]->groupMemberCount > 0)
                groupUserAuxiliaryValues.callStackSum[i] = topMatches[i]->callStackSum;

        if (g_leakDetectAllMemory)
            pagalloc_dumpEngine ((PDE_CONTROL(__cdecl*)(PageMallocEntry *, void *))pagalloc_collectUserAuxiliaryValues, &groupUserAuxiliaryValues, ___, ___);
        else
            pagalloc_dumpEngine ((PDE_CONTROL(__cdecl*)(PageMallocEntry *, void *))pagalloc_collectUserAuxiliaryValues, &groupUserAuxiliaryValues, s_sbh[0].blockP, s_sbh[0].blockP + (s_sbh[0].mbytes << 20));
        }


    pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "PAGALLOC:\r\nPAGALLOC: Leak details ************************************\r\n");

    for (i=0; i < g_topMatchesToDetail; i++)
        {
        UInt32 fdwOldProtect = 0;

        if (topMatches[i]  &&  topMatches[i]->groupMemberCount > 0)
            {
            PageMallocEntry   const* headerP;

            if (s_usingLite)
                {
                headerP = (PageMallocEntry*)pagallocI_findHeader (topMatches[i]->detail.liteData);
                }
            else
                headerP = topMatches[i]->detail.headerP;

            pagallocI_makeHeaderVisible(headerP, PAGE_READONLY, &fdwOldProtect);
            pagalloc_printf(PAGALLOC_SEVERITY_DEBUG,
                "PAGALLOC: %2d. Root 0x%p appeared%6d time%s totaling %6d bytes from (sequence %d, origination %s)--.\r\n",
                i+1, headerP, topMatches[i]->groupMemberCount, (1<topMatches[i]->groupMemberCount)? "s" : " ", topMatches[i]->totalSize,
                headerP->mallocSerialNumber, pagalloc_toStringLastOperation(headerP));

            // Print any  User Auxiliary Values if any
            if (headerP->userAuxiliaryValue)
                {
                Int32   j;
                pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "PAGALLOC:      User Auxiliary Values:");
                for (j=0; j < MAX_TOPMATCHES_USERAUXILIARYVALUES; j++)
                    {
                    if ( ! groupUserAuxiliaryValues.userAuxiliaryValues[i][j])
                        break;
                    pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "  %d", groupUserAuxiliaryValues.userAuxiliaryValues[i][j]);
                    }

                pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "\r\n");
                }

            // Print the symbolized call stack
            pagalloc_symbolizeHeaderAddressesForVisualC (headerP);

            if (g_showBlockContents)
                pagalloc_dumpFormattedMemory (headerP, topMatches[i]);

            pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION,
                "PAGALLOC:\r\n"
                "PAGALLOC:      ======================================================================================\r\n"
                "PAGALLOC:\r\n");

            /* leave protection exactly as we found it */
            pagallocI_makeHeaderInvisible(headerP, fdwOldProtect, ___);
            }
        }

    pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION, "PAGALLOC:  Hit count of excluded functions:\r\n");
    int const SZ_M = 512;
    char    module[SZ_M];
    module[SZ_M-1] = '\0';
    for (i=0; i < g_felUsed; i++)
        {
        char                                    fname[MAX_PATH]= "<<Unknown>>";
        FunctionExclusionList const * const felP = g_fel+i;

        memutil_symbolizeAddress ((void*)felP->start, ___, ___, fname, sizeof(fname), module, SZ_M-1);
        pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION, "PAGALLOC:%10d       %s %s\r\n", felP->hitCount, fname, module);
        }
    pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION, "PAGALLOC:\r\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void     pagalloc_addToTopLeakers
(
LeakDetectCandidate * const     candidateP,
LeakDetectCandidate           **topMatches,
Int32                           maxTopMatches
)
    {
    Int32     iSlot;
    Int32     smallestSizeSlot = 0;

    for (iSlot=0; iSlot < maxTopMatches; iSlot++)
        {
        // Use the first zero slot
        if (NULL == topMatches[iSlot])
            {
            smallestSizeSlot = iSlot;
            break;
            }

        if ( (  s_sortLeakersBySize  &&  (topMatches[iSlot]->totalSize        < topMatches[smallestSizeSlot]->totalSize       ) )             // Sort top leakers by number of bytes
            || ( !s_sortLeakersBySize  &&  (topMatches[iSlot]->groupMemberCount < topMatches[smallestSizeSlot]->groupMemberCount) )       )     // Sort top leakers by number of occurrences
            {
            smallestSizeSlot = iSlot;
            }
        }

    if ( (NULL == topMatches[smallestSizeSlot])
        || (  s_sortLeakersBySize  &&  (topMatches[smallestSizeSlot]->totalSize        < candidateP->totalSize) )
        || ( !s_sortLeakersBySize  &&  (topMatches[smallestSizeSlot]->groupMemberCount < candidateP->groupMemberCount) ) )
        {
        topMatches[smallestSizeSlot] = candidateP;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   04/2002
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @Description Does a binary search of a sorted array for a key.
*
* @param    key     IN  key to search for
* @param    base    IN  base of sorted array to search
* @param    num     IN  number of elements in array
* @param    width   IN  number of bytes per element
* @param    compare IN  pointer to function that compares two array elements, returning neg when #1 < #2, pos when #1 > #2, and 0 when they are equal. Function is passed pointers to two array elements.
* @return   returns pointer to occurrence of key in array if key is found, NULL otherwise
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void const * const pagalloc_bsearch
(
void const * const key,
void const * const base,
size_t             num,
size_t       const width,
Int32 (__cdecl *compare)( void const * const, void const * const)
)
    {
    char const * lo = (char *)base;
    char const * hi = (char *)base + (num - 1) * width;
    char const * mid;
    size_t half;

    while (lo <= hi)
        {
        if (half = (num / 2) )
            {
            Int32     result;
            mid = lo + ((num & 1) ? half : (half - 1)) * width;
            result = (*compare)(key, mid);
            if ( ! result)
                return mid;
            else if (result < 0)
                {
                hi = mid - width;
                num = (num & 1) ? half : (half-1);
                }
            else
                {
                lo = mid + width;
                num = half;
                }
            }
        else if (num)
            return ((*compare)(key,lo)) ? NULL : lo;
        else
            break;
        }

    return NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   03/2002
+---------------+---------------+---------------+---------------+---------------+------*/
static Int32    bsearchfuncInFunctionExclusionList
(
void const * const keyP,
void const * const felVP
)
    {
    uintptr_t               const returnAddress = (uintptr_t)keyP;
    FunctionExclusionList   const * const felP  = (FunctionExclusionList *) felVP;

    if (returnAddress < felP->start)
        return -1;
    if (returnAddress > felP->end)
        return  1;
    return  0;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   03/2002
+---------------+---------------+---------------+---------------+---------------+------*/
static bool  inFunctionExclusionList
(
PageMallocEntry const * const headerP
)
    {
    Int32 i;
    for (i=0; i < _countof(headerP->callStackFrame)  &&  headerP->callStackFrame[i].returnAddress; i++)
        {
        FunctionExclusionList * const felP = (FunctionExclusionList *) pagalloc_bsearch ((void *) (headerP->callStackFrame[i].returnAddress),  g_fel, g_felUsed, sizeof(g_fel)[0], bsearchfuncInFunctionExclusionList);
        if ( ! felP)
            continue;

        felP->hitCount++;
        return      true;
        }

    return  false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dumpEngineLite
(
PDE_CONTROL     (*fpCallBack) (PageMallocEntry *headerP, void *context),
void *          context
)
    {
    HeapLiteHeader*   header;

    if (!pTryEnterCriticalSection(&s_csHeap))
        {
        InterlockedIncrement ((volatile Long32 *)&s_waitCount);
        EnterCriticalSection (&s_csHeap);
        }

    //  We go backwards through the list because by processing the older
    //  entries first we keep the Root values the same across leak detects.
    //  PAGALLOC: 84. Root 0x315c66f8 appeared     1 time  totaling   1072 bytes.
    for (header = s_heapListEnd; NULL != header; header = header->prev)
        {
        //  TODO_optimize We could optimize this by changing the logic to work with
        //  the lite data structures instead of generating the header on fly.
        PageMallocEntry *fullHeader = (PageMallocEntry*)pagallocI_findHeader ((char*)header + sizeof (*header));

        __try
            {
            if ( PAGE_SIGNATURE     != fullHeader->signature)           __leave;            // Skip non-PAGALLOC memory
            if (s_leakDetectSinceBase  > fullHeader->mallocSerialNumber)  __leave;            // Is PAGALLOC but may be too old
            if (g_felUsed  &&  inFunctionExclusionList(fullHeader) )    __leave;            // Skip if the return address is in the FunctionExclusionList

            //pagalloc_dumpHeader(headerP);
            if (PDE_STOP == (*fpCallBack) (fullHeader, context))
                break;
            }
        __except(EXCEPTION_EXECUTE_HANDLER)
            {
            pagalloc_printf (PAGALLOC_SEVERITY_ERROR, " pagalloc_dumpEngine exception\r\n");
            }
        }

    LeaveCriticalSection(&s_csHeap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dumpEngineFull
(
PDE_CONTROL (*fpCallBack) (PageMallocEntry *headerP, void *context),
void  *context,
char  *pStart,
char  *pEnd
)
    {
    char *pMem;
    char       * const pMin = pStart ? pStart : (char*)s_systemInfo.lpMinimumApplicationAddress;
    char const * const pMax = pEnd   ? pEnd   : (char*)s_systemInfo.lpMaximumApplicationAddress;
    UInt32 fdwProtect = s_pageDebugHeadersAlwaysVisible ? PAGE_READWRITE : PAGE_NOACCESS;
    MEMORY_BASIC_INFORMATION        mbi;

    for (pMem = pMin; pMem < pMax; pMem += mbi.RegionSize)
        {
        size_t cb;

        memset(&mbi, 0, sizeof(mbi));
        cb = VirtualQuery(pMem, &mbi, sizeof(mbi));

        if  (mbi.Protect & (PAGE_GUARD | PAGE_NOCACHE) )
            continue;

        if (MEM_COMMIT != mbi.State)
            continue;

        if (fdwProtect == mbi.Protect) /* might be PAGALLOC header */
            {
            UInt32 fdwOldProtect = PAGE_NOACCESS;
            PageMallocEntry *headerP = (PageMallocEntry*)mbi.BaseAddress;

            /* make invisible headers visible */
            pagallocI_makeHeaderVisible(headerP, PAGE_READONLY, &fdwOldProtect);
            __try
                {
                if ( PAGE_SIGNATURE     != headerP->signature)              __leave;        // Skip non-PAGALLOC memory
                if (s_leakDetectSinceBase  > headerP->mallocSerialNumber)     __leave;        // Is PAGALLOC but may be too old
                if (g_felUsed  &&  inFunctionExclusionList(headerP) )       __leave;        // Skip if the return address is in the FunctionExclusionList

                //pagalloc_dumpHeader(headerP);
                if (PDE_STOP == (*fpCallBack) (headerP, context))
                    break;
                }
            __except(EXCEPTION_EXECUTE_HANDLER)
                {
                pagalloc_printf (PAGALLOC_SEVERITY_ERROR, " pagalloc_dumpEngine exception\r\n");
                }
            /* leave protection exactly how we found it */
            pagallocI_makeHeaderInvisible(headerP, fdwOldProtect, ___);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   02/01
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API
void __cdecl    pagalloc_dumpEngine
(
PDE_CONTROL (*fpCallBack) (PageMallocEntry *headerP, void *context),
void  *context,
char  *pStart,
char  *pEnd
)
    {
    if (PAGALLOC_MINIMAL == s_usingLite)
        {
        pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION, "PAGALLOC:    Operation not supported in minimal mode\n");

        }

    if (s_usingLite)
        dumpEngineLite (fpCallBack, context);
    else
        dumpEngineFull (fpCallBack, context, pStart, pEnd);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/01
+---------------+---------------+---------------+---------------+---------------+------*/
static uintptr_t    pagalloc_calculateCallStackSum
(
PageMallocEntry *headerP
)
    {
    Int32 const       cFrame = _countof(headerP->callStackFrame);
    Int32                   iFrame;
    CallStackFrame  *frameP;
    uintptr_t   checkSum = 0xffffffff;
    for (iFrame=0, frameP = headerP->callStackFrame; iFrame < cFrame; iFrame++, frameP++)
        {
        if (0 == frameP->returnAddress)
            break;
        checkSum += frameP->returnAddress;
        }

    return checkSum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/01
+---------------+---------------+---------------+---------------+---------------+------*/
static Int32     pagalloc_callStacksReallyMatch
(
PageMallocEntry*header1P,
PageMallocEntry*header2P
)
    {
    const CallStackFrame   *cs1P, *csEndP, *cs2P;
    Int32 const             maxStackTest = _countof(header1P->callStackFrame);
    UInt32                  fdwOldProtect1 = PAGE_NOACCESS;
    UInt32                  fdwOldProtect2 = PAGE_NOACCESS;
    Int32                           matches = true;

    pagallocI_makeHeaderVisible (header1P, PAGE_READONLY, &fdwOldProtect1);
    pagallocI_makeHeaderVisible (header2P, PAGE_READONLY, &fdwOldProtect2);

    for (cs1P = header1P->callStackFrame, csEndP = cs1P + maxStackTest, cs2P = header2P->callStackFrame; cs1P < csEndP; cs1P++, cs2P++)
        {
        if (cs1P->returnAddress != cs2P->returnAddress)
            {
            matches = false;
            break;
            }
        // stop when we run out of call frames.
        if (0 == cs1P->returnAddress)
            break;
        }
    if (matches == false)
        {
        uintptr_t   cs1 = pagalloc_calculateCallStackSum (header1P);
        uintptr_t   cs2 = pagalloc_calculateCallStackSum (header2P);
        if (cs1 != cs2)
            pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: bad checksum\r\n");
        }

    pagallocI_makeHeaderInvisible (header2P, fdwOldProtect2, &fdwOldProtect2);
    pagallocI_makeHeaderInvisible (header1P, fdwOldProtect1, &fdwOldProtect1);

    return matches;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static Int32     consolidateLeakDetectCandidate
(
PageMallocEntry *headerP,
LeakDetectContext   *ldcP
)
    {
    LeakDetectCandidate     *candidate2P;

    // go through all those that follow and see if we find any that duplicate this one.
    for (candidate2P = ldcP->candidates; candidate2P < ldcP->nextCandidate; candidate2P++)
        {
        PageMallocEntry*    tempHeader;

        if (candidate2P->callStackSum != headerP->callStackSum)
            continue;

        if (s_usingLite)
            tempHeader = (PageMallocEntry*)pagallocI_findHeader (candidate2P->detail.liteData);
        else
            tempHeader = candidate2P->detail.headerP;

        if (pagalloc_callStacksReallyMatch (headerP, tempHeader))
            {
            candidate2P->groupMemberCount++;
            candidate2P->totalSize += headerP->userSize;

            if (candidate2P->smallestSize > headerP->userSize)
                candidate2P->smallestSize = headerP->userSize;

            if (candidate2P->largestSize < headerP->userSize)
                candidate2P->largestSize = headerP->userSize;

            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/01
+---------------+---------------+---------------+---------------+---------------+------*/
static PDE_CONTROL     pagalloc_collectLeakDetectCandidates
(
PageMallocEntry *headerP,
void            *context
)
    {
    LeakDetectContext   *ldcP = (LeakDetectContext *)context;

    pagalloc_displayWorkingMessage();

    // skip free ones.
    if (headerP->flags.isFree)                  // Ignore free blocks
        return      PDE_CONTINUE;

    //  Combine it with another if possible
    if (consolidateLeakDetectCandidate (headerP, ldcP))
        return PDE_CONTINUE;

    //  If the list is full, stop
    if (ldcP->nextCandidate >= ldcP->endCandidate)
        return PDE_STOP;

    if (s_usingLite)
        ldcP->nextCandidate->detail.liteData = headerP->dataStart;
    else
        ldcP->nextCandidate->detail.headerP = headerP;

    ldcP->nextCandidate->callStackSum   = headerP->callStackSum;
    ldcP->nextCandidate->userSize       = headerP->userSize;
    ldcP->nextCandidate->smallestSize   = headerP->userSize;
    ldcP->nextCandidate->largestSize    = headerP->userSize;
    ldcP->nextCandidate->totalSize= headerP->userSize;
    ldcP->nextCandidate->groupMemberCount = 1;  //  previous approach required 0 here.
    ldcP->nextCandidate++;

    return PDE_CONTINUE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    pagalloc_consolidateLeakCandidates
(
LeakDetectContext   *ldcP
)
    {
    LeakDetectCandidate     *candidate1P;

    // go through all the leak detect candidates
    for (candidate1P = ldcP->candidates; candidate1P < ldcP->nextCandidate; candidate1P++)
        {
#ifdef NOTNOW
        LeakDetectCandidate     *candidate2P;

        //  has already been processed if its groupMemberCount is not zero.
        //  Remember, groupRoot and groupMemberCount are members of the same union
        //  so  this was probably set by the statement "candidate2P->groupRoot = candidate1P->detail.headerP"
        if (0 != candidate1P->groupMemberCount)
            continue;

        // initialize groupMemberCount
        candidate1P->groupMemberCount = 1;

        // go through all those that follow and see if we find any that duplicate this one.
        for (candidate2P = candidate1P+1; candidate2P < ldcP->nextCandidate; candidate2P++)
            {
            if (0 != candidate2P->groupMemberCount)
                continue;

            if ((candidate2P->callStackSum == candidate1P->callStackSum) &&
                pagalloc_callStacksReallyMatch (&candidate2P->detail, &candidate1P->detail))
                {
                candidate1P->groupMemberCount++;
                candidate1P->totalSize += candidate2P->userSize;
                if (candidate2P->smallestSize < candidate1P->userSize)
                    candidate1P->smallestSize = candidate2P->userSize;
                if (candidate2P->largestSize > candidate1P->userSize)
                    candidate1P->largestSize = candidate2P->userSize;

                candidate2P->groupRoot = candidate1P->detail.headerP;  //  If s_usingLite is true, this is a liteHeader
                }
            }
#endif
        pagalloc_addToTopLeakers (candidate1P, ldcP->topMatches, ldcP->maxTopMatches);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   02/01
+---------------+---------------+---------------+---------------+---------------+------*/
void PAGALLOC_API __cdecl  pagalloc_detectLeaks
(
void
)
    {
    LeakDetectContext         leakDetectContext;
    UInt32              const sbhMaxAllocations = ((s_sbh[0].mbytes << 20 ) / s_systemInfo.dwPageSize / 2);     // Number of possible allocations in SBH
    UInt32                    nCandidates = min(MAX_LEAK_DETECT_CANDIDATES, s_usingLite ? s_heapListLength : sbhMaxAllocations);

    if (rejectInMinimal ())
        return;

    /*-------------------------------------------------------------------
    The stack walker may lose the frame of the caller to a CRT function
    -------------------------------------------------------------------*/
    EnterCriticalSection(&s_csDump);
    pagalloc_displayUsageStatistics ("\r\nPAGALLOC: Begin leak detection");

    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC:     starting at malloc serial number %d.\r\n", s_leakDetectSinceBase);
    s_workingMsgCount = 0;

    // Compute available stack
        {
        MEMORY_BASIC_INFORMATION    mbi;
        ptrdiff_t                   availableStack;
        Int32 const                stackNeeded = nCandidates * sizeof *leakDetectContext.candidates;

        memset(&mbi, 0, sizeof(mbi));
        VirtualQuery(&mbi, &mbi, sizeof(mbi));
        availableStack = (uintptr_t)mbi.BaseAddress - (uintptr_t)mbi.AllocationBase - STACK_BYTES_NEEDED_FOR_LOG_PRINTF;

        // Sufficient stack for our needs?
        if (stackNeeded > availableStack)
            {
            nCandidates =  (UInt32)(availableStack / sizeof (LeakDetectCandidate));
            pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION, "PAGALLOC:    *Insufficient stack for leak detection, %d bytes needed. ", stackNeeded);
            pagalloc_printf (PAGALLOC_SEVERITY_INFORMATION, "Reducing \"candidate\" array to %d from %d.\r\n", nCandidates, MAX_LEAK_DETECT_CANDIDATES);
            }
        }

        // Allocate the candidate array
        memset (&leakDetectContext, 0, sizeof(leakDetectContext));
        leakDetectContext.candidates    = (LeakDetectCandidate*)_alloca (nCandidates * sizeof *leakDetectContext.candidates);
        leakDetectContext.nextCandidate = leakDetectContext.candidates;
        leakDetectContext.endCandidate  = leakDetectContext.candidates + nCandidates;
        leakDetectContext.maxTopMatches = (g_topMatchesToPrint < MAX_TOPMATCHES) ? g_topMatchesToPrint : MAX_TOPMATCHES;

        if (g_leakDetectAllMemory)
            pagalloc_dumpEngine (pagalloc_collectLeakDetectCandidates, &leakDetectContext, ___, ___);
        else
            pagalloc_dumpEngine (pagalloc_collectLeakDetectCandidates, &leakDetectContext, s_sbh[0].blockP, s_sbh[0].blockP + (s_sbh[0].mbytes << 20));

        pagalloc_consolidateLeakCandidates (&leakDetectContext);

        pagalloc_displayUsageStatistics ("End leak detection");
        pagalloc_printTopLeakers(leakDetectContext.topMatches, leakDetectContext.maxTopMatches);
        s_workingMsgCount = 0;

        LeaveCriticalSection(&s_csDump);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static PDE_CONTROL pagalloc_resetLeakVariables
(
PageMallocEntry *headerP,
void            *context
)
    {
    pagalloc_displayWorkingMessage();

    if (s_usingLite)
        {
        HeapLiteHeader*   header;
        HeapLiteTrailer*  trailer;
        UInt32          userDataSize;

        getLiteInfo (&header, &userDataSize, &trailer, headerP->dataStart);
        }
    else
        {
        UInt32   fdwOldProtect = PAGE_NOACCESS;
        bool    ok;

        ok = pagallocI_makeHeaderVisible (headerP, PAGE_READWRITE, &fdwOldProtect);
        if (ok)
            {
            pagallocI_makeHeaderInvisible (headerP, fdwOldProtect, ___);
            }
        }

    return  PDE_CONTINUE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void __declspec(dllexport) __cdecl  pagalloc_setLeakDetectDisplayLimit
(
UInt32 const  maxLeakersToDisplay,
UInt32 const  maxLeakersToDetail
)
    {
    if (rejectInMinimal ())
        return;

    g_topMatchesToPrint =  ((0 < maxLeakersToDisplay) && (maxLeakersToDisplay <= MAX_TOPMATCHES)) ? maxLeakersToDisplay : MAX_TOPMATCHES;
    g_topMatchesToDetail = ((0 < maxLeakersToDetail) && (maxLeakersToDetail <= MAX_TOPMATCHES)) ? maxLeakersToDetail : MAX_TOPMATCHES;
    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: Setting leak detection display limit to %u, %u detail.\r\n", g_topMatchesToPrint, g_topMatchesToDetail);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    Chuck.Kirschman                 01/04
+---------------+---------------+---------------+---------------+---------------+------*/
void __declspec(dllexport) __cdecl  pagalloc_setSortType
(
PagallocSortType     sortType
)
    {
    if (rejectInMinimal ())
        return;

    s_sortLeakersBySize  = (PAGALLOC_SortBySize == sortType ? true : false);
    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: Setting leak detection sort type to %s\r\n", PAGALLOC_SortBySize == sortType ? "size" : "count");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void __declspec(dllexport) __cdecl  pagalloc_resetLeakDetect
(
void
)
    {
    EnterCriticalSection(&s_csDump);
    pagalloc_displayUsageStatistics ("****Begin leak detection reset");
    s_workingMsgCount = 0;

    if (g_leakDetectAllMemory)
        pagalloc_dumpEngine (pagalloc_resetLeakVariables, ___, ___, ___);                   // all of memory
    else
        pagalloc_dumpEngine (pagalloc_resetLeakVariables, ___, s_sbh[0].blockP,   s_sbh[0].blockP + (s_sbh[0].mbytes << 20) );    // only the small block heap

    pagalloc_displayUsageStatistics ("****Completed leak detection reset");
    s_workingMsgCount = 0;
    LeaveCriticalSection(&s_csDump);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   02/01
+---------------+---------------+---------------+---------------+---------------+------*/
 void __declspec(dllexport) __cdecl pagalloc_showCallStack
(
void const * const rawAddress       // =>
)
     {
     PageMallocEntry const * const headerP = (PageMallocEntry const * const ) pagallocI_findHeader (rawAddress);
     bool headerValid = false;

     if (rejectInMinimal ())
         return;

     if ( ! s_pagallocInitialized)
         {
         pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: not initialized\r\n.");
         return;
         }

     headerValid = pagallocI_openHeader(headerP); /* leaves header visible */
     if ( ! headerValid)
         {
         pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: not a valid address 0x%p (assumed header at 0x%p).\r\n", rawAddress, headerP );
         return;
         }

     pagalloc_printf (PAGALLOC_SEVERITY_DEBUG,
         "PAGALLOC: =Begin===========================================================\r\n"
         "PAGALLOC: Allocation call stack for 0x%p (header at %#x):\r\n", rawAddress, headerP);

     pagalloc_symbolizeHeaderAddressesForVisualC (headerP);
     pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: =End=============================================================\r\n");

     if (headerValid)
         pagallocI_closeHeader(headerP);
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   12/2001
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API  void __cdecl pagalloc_showCallStackSince
(
void const * const  rawAddress,         // =>
UInt32        const  thisSerialNumber   // => -1 == all blocks (same as pagalloc_showCallStack), 0 == s_leakDetectSinceBase, else use this number as low serial number limit
)
    {
    PageMallocEntry const * const headerP = (PageMallocEntry const * const ) pagallocI_findHeader (rawAddress);
    bool headerValid = false;

    if (rejectInMinimal ())
        return;

    if ( ! s_pagallocInitialized)
        {
        pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: not initialized\r\n.");
        return;
        }

    __try
        {
        headerValid = pagallocI_openHeader(headerP); /* leaves header visible */
        if ( ! headerValid)
            {
            pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: not a valid address 0x%p (assumed header at 0x%p).\r\n", rawAddress, headerP);
            return;
            }

        /*---------------------------------------------------------------
        Don't process this block if its mallocSerialNumber is too high.
        ---------------------------------------------------------------*/
        if (0xffffffff != thisSerialNumber)
            {
            if (thisSerialNumber)
                {
                if (thisSerialNumber    > headerP->mallocSerialNumber)
                    __leave;
                }
            else
                {
                if (s_leakDetectSinceBase > headerP->mallocSerialNumber)
                    __leave;
                }
            }

        pagalloc_printf (PAGALLOC_SEVERITY_DEBUG,
            "PAGALLOC: =Begin===========================================================\r\n"
            "PAGALLOC: Allocation call stack for 0x%p (header at %#x):\r\n", rawAddress, headerP);

        pagalloc_symbolizeHeaderAddressesForVisualC (headerP);
        pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: =End=============================================================\r\n");
        }
    __finally
        {
        if (headerValid)
            pagallocI_closeHeader(headerP);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API  void __cdecl pagalloc_setSinceBase
(
UInt32 const beginningMallocSerialNumber            // =>
)
    {
    if (rejectInMinimal ())
        return;

    s_leakDetectSinceBase = beginningMallocSerialNumber ? beginningMallocSerialNumber : s_mallocSerialNumber;
    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: Setting leak detection malloc base to %u.\r\n", s_leakDetectSinceBase);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API  void  __cdecl pagalloc_displayStatistics
(
void
)
    {
    if (rejectInMinimal ())
        return;

    pagalloc_displayUsageStatistics("Usage statistics");
    }

#if 0
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Stratoti   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
 void __declspec(dllexport) __cdecl pagalloc_queryStatistics
(
PagallocStatistics  *psp,
Int32                maxPsp
)
     {
     unsigned char    buf[8192];     // At least 1 machine page
     PageMallocEntry *headerP= (PageMallocEntry *)buf;

     s_mallocCount, s_reallocCount, s_byteCount, s_freeCount, s_waitCount, s_sbh[0].blocksInUseCount,  s_sbh[0].blocks, (100 * s_sbh[0].blocksInUseCount) / s_sbh[0].blocks, deltaUse);


     if (!enabled)
         return;

     if (!s_systemInfo.dwPageSize)
         {
         pagalloc_printf(PAGALLOC_SEVERITY_ERROR, "PAGALLOC: PAGALLOC is not initialized.  Set the environment variable MS_PAGALLOC=1 and restart.\r\n");
         return;
         }

     pagallocI_recordCallingStack (headerP, 1);
     pagalloc_symbolizeHeaderAddressesForVisualC (headerP);
     }
#endif

 PAGALLOC_API  void __cdecl pagalloc_showCallStackByReturnAddress
     (
     void* returnAddress // => Do not show frames more recent than this address
     )
     {
     unsigned char    buf[8192];     // At least 1 machine page
     PageMallocEntry *headerP = (PageMallocEntry *)buf;
     pagallocI_recordCallingStack (headerP, returnAddress);
     pagalloc_symbolizeHeaderAddressesForVisualC (headerP);
     }

/*---------------------------------------------------------------------------------**//**
* Display the most recent MDL call frame in the Debugger's output window. It is cool to find out what MDL function is calling this native
* code.
* Put {,,DgnPlatformTools.dll}pagalloc_showFullCallStack() in the "watch window"
* @bsimethod                                                    Mike.Stratoti   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API  void __cdecl pagalloc_showFullCallStack
(
)
    {
    if (!s_systemInfo.dwPageSize)
        {
        GetSystemInfo(&s_systemInfo);
        if (!s_systemInfo.dwPageSize)
            s_systemInfo.dwPageSize = PAGE_SIZE_DEFAULT;
        pagallocI_initMdlFuncs();
        }
    pagalloc_showCallStackByReturnAddress(PAGALLOC_CONTEXT);
    }


/*---------------------------------------------------------------------------------**//**
* Display the most recent MDL call frame in the Debugger's output window. It is cool to find out what MDL function is calling this native
* code.
* Put {,,toolsubs.dll}pagalloc_unprotect(1) in the "watch window"
* @bsimethod                                                    Mike.Stratoti   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void __declspec(dllexport) __cdecl pagalloc_unprotect
    (
    UInt32 const rawAddress     // =>
    )
    {
    PageMallocEntry const * const headerP = (PageMallocEntry const * const ) pagallocI_findHeader ((void*)rawAddress);
    bool headerValid = pagallocI_openHeader(headerP); /* leaves header visible */

    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: unprotect address 0x%p (base 0x%p) .\r\n", rawAddress, headerP);
    if ( ! headerValid)
        pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: Unexpected pagallocI_openHeader failure %d. Ignoring and proceeding.\r\n", GetLastError() );

    /* intentionally NOT using pagallocI_closeHeader here because we want
    * to leave header visible, but need to leave s_csVisible critical section */
    if (headerValid)
        pagallocI_leaveHeaderVisible();

    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: Address 0x%p (base 0x%p) unprotected.\r\n", rawAddress, headerP);
    }

/*---------------------------------------------------------------------------------**//**
* Reprotect pages from pagalloc_unprotect.
* Put {,,toolsubs.dll}pagalloc_unprotect (<some_address>) in the "watch window"
* @bsimethod                                                    Mike.Stratoti   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API  void __cdecl pagalloc_protect
(
UInt32 const rawAddress     // =>
)
    {
    PageMallocEntry const * const headerP = (PageMallocEntry const * const ) pagallocI_findHeader ((void*)rawAddress);
    bool headerValid = pagallocI_openHeader(headerP); /* leaves header visible */

    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: protect address 0x%p (base 0x%p) .\r\n", rawAddress, headerP);
    if ( ! headerValid)
        pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: Unexpected pagallocI_openHeader failure %d. Ignoring and proceeding.\r\n", GetLastError() );
    else
        pagallocI_closeHeader(headerP);
    }

/*---------------------------------------------------------------------------------**//**
* Sets an alternate output function for PAGALLOC printing. Not thread safe.
* @bsimethod                                                  Mike.Stratoti   09/2001
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API PagallocOutputFunction const __cdecl pagalloc_setOutputFunction
(
PagallocOutputFunction  const  pof          // =>
)
    {
    PagallocOutputFunction  const prev = fpAlternateOutputFunction;
    fpAlternateOutputFunction = pof;
    if (NULL == pof)
        fpAlternateOutputMdlDesc = NULL;
    else if (mdlSystem_getCurrMdlDesc)
        fpAlternateOutputMdlDesc = mdlSystem_getCurrMdlDesc();

    return  prev;
    }

/*---------------------------------------------------------------------------------**//**
* Sets an alternate output function for PAGALLOC printing. Not thread safe.
* @bsimethod                                                  Mike.Stratoti   10/2007
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API PagallocOutputFunction const __cdecl pagalloc_setNativeOutputFunction
(
PagallocOutputFunction  const  pof          // =>
)
    {
    PagallocOutputFunction  const prev = fpAlternateOutputFunction;
    fpAlternateOutputFunction = pof;
    return  prev;
    }


#include <malloc.h>
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static Int32     heapWalkLiteImpl
(
_HEAPINFO * const pHeapInfo
)
    {
    HeapLiteHeader*     next = s_heapList;
    HeapLiteTrailer*    trailer;

    /*-------------------------------------------------------------------
    If they specified an address, use it to find the next address.
    Otherwise, start at the beginning of memory.
    -------------------------------------------------------------------*/
    if (pHeapInfo->_pentry)
        {
        HeapLiteHeader* tempHeader;
        HeapLiteTrailer*tempTrailer;
        UInt32        size;

        getLiteInfo (&tempHeader, &size, &tempTrailer, pHeapInfo->_pentry);
        if (PAGE_SIGNATURE != tempHeader->signature)
            {
            return _HEAPBADPTR;
            }

        next = tempHeader->next;
        }

    if (NULL == next)
        return _HEAPEMPTY;

    trailer = getLiteTrailer (next);

    pHeapInfo->_pentry  = (Int32*)((char*)next + sizeof (*next));
    pHeapInfo->_size    = next->requestedSize;
    pHeapInfo->_useflag = trailer->flags.isFree ? _FREEENTRY : _USEDENTRY;

    return  _HEAPOK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static Int32     heapWalkLite
(
_HEAPINFO * const pHeapInfo
)
    {
    Int32                       retval = _HEAPBADPTR;

    EnterCriticalSection(&s_csHeap);

    __try
        {
        retval = heapWalkLiteImpl (pHeapInfo);
        }
    __except(EXCEPTION_EXECUTE_HANDLER)
        {
        retval =_HEAPBADNODE;
        }

    LeaveCriticalSection(&s_csHeap);
    return  retval;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   10/2001
+---------------+---------------+---------------+---------------+---------------+------*/
static Int32     heapWalkFull
(
_HEAPINFO * const pHeapInfo
)
    {
    Int32                      retval = _HEAPBADPTR;
    Int32                      found = false;
    char              *pMem;
    char       * const pMin = (char*)s_systemInfo.lpMinimumApplicationAddress;
    char const * const pMax = (char*)s_systemInfo.lpMaximumApplicationAddress;
    UInt32        const fdwProtect = s_pageDebugHeadersAlwaysVisible ? PAGE_READWRITE : PAGE_NOACCESS;
    MEMORY_BASIC_INFORMATION mbi;

    if (!s_systemInfo.dwPageSize)
        return _HEAPEMPTY;      // pagalloc is not active

    /*-------------------------------------------------------------------
    If they specified an address, use it to find the next address.
    Otherwise, start at the beginning of memory.
    -------------------------------------------------------------------*/
    if (pHeapInfo->_pentry)
        {
        memset(&mbi, 0, sizeof(mbi));
        VirtualQuery (pHeapInfo->_pentry, &mbi, sizeof(mbi));
        pMem =  (char*) ( (uintptr_t)pHeapInfo->_pentry + (size_t)mbi.RegionSize);
        }
    else
        {
        pMem = pMin;
        }

    EnterCriticalSection(&s_csDump);

    for (; pMem < pMax; pMem += mbi.RegionSize)
        {
        memset(&mbi, 0, sizeof(mbi));
        VirtualQuery(pMem, &mbi, sizeof(mbi));

        if  (mbi.Protect & (PAGE_GUARD | PAGE_NOCACHE) )
            continue;

        if (MEM_COMMIT == mbi.State)
            {
            if (fdwProtect == mbi.Protect) /* might be PAGALLOC header */
                {
                UInt32 fdwOldProtect = PAGE_NOACCESS;
                PageMallocEntry * const headerP = (PageMallocEntry*)mbi.BaseAddress;

                /* make invisible headers visible */
                pagallocI_makeHeaderVisible(headerP, PAGE_READONLY, &fdwOldProtect);
                __try
                    {
                    if (PAGE_SIGNATURE == headerP->signature)
                        {
                        pHeapInfo->_pentry  = (Int32*)headerP->dataStart;
                        pHeapInfo->_size    = headerP->userSize;
                        pHeapInfo->_useflag = headerP->flags.isFree ? _FREEENTRY : _USEDENTRY;
                        retval = _HEAPOK;
                        found = true;
                        }
                    }
                __except(EXCEPTION_EXECUTE_HANDLER)
                    {
                    retval =_HEAPBADNODE;
                    }
                /* leave protection how we found it */
                pagallocI_makeHeaderInvisible(headerP, fdwOldProtect, ___);
                }
            }

        if (found)
            break;
        }

    LeaveCriticalSection(&s_csDump);

    if ( ! found)
        retval = _HEAPEND;

    return  retval;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   10/2001
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API Int32 __cdecl pagalloc_heapWalk
(
_HEAPINFO * const pHeapInfo
)
    {
    if (PAGALLOC_MINIMAL == s_usingLite)
        {
        //  This is not supported
        assert (PAGALLOC_MINIMAL != s_usingLite);
        return _HEAPEND;
        }

    if (s_usingLite)
        return heapWalkLite (pHeapInfo);

    return heapWalkFull (pHeapInfo);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   03/2002
+---------------+---------------+---------------+---------------+---------------+------*/
CRTCallback Int32 pagalloc_sortfuncAssendingFunctionExclusionList
(
void const * const p1,
void const * const p2
)
    {
    FunctionExclusionList const * const t1 = (FunctionExclusionList *)p1;
    FunctionExclusionList const * const t2 = (FunctionExclusionList *)p2;

    if (NULL == t1)
        return       1;
    if (NULL == t2)
        return       -1;
    ptrdiff_t diff = t1->start - t2->start;
    if(diff>0)
        return 1;
    if(diff<0)
        return -1;
    return 0;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   03/2002
+---------------+---------------+---------------+---------------+---------------+------*/
static FunctionExclusionList *pagalloc_findEmptyFunctionExclusionListEntry
(
void
)
    {
    Int32 i;
    for (i=0; i < _countof(g_fel); i++)
        {
        if ( ! g_fel[i].start)
            return g_fel + i;
        }
    return  NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   03/2002
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt  addAddressToExclusionList
(
WCharCP     dllName,
WCharCP     funcName,
intptr_t    actualFuncAddress
)
    {

    StatusInt                       rtn = -1;
    FunctionExclusionList * const   felP = pagalloc_findEmptyFunctionExclusionListEntry();
    char                            moduleName[MAX_PATH] = "<<Unknown>>";
    void                           *hDbg;
    size_t                          ok;
    uintptr_t                       actualImageBaseAddress=0, deltaImageBaseAddress=0;
    uintptr_t                       preferredFunctionStartAddress=0, preferredFunctionEndAddress=0, preferredFunctionAnyAddress;
    MEMORY_BASIC_INFORMATION        mbi;
    HMODULE                         hDll;

    /*-------------------------------------------------------------------
    Remember that all of the readDbg_xxx function use "preferred" not
    "actual" addresses.
    -------------------------------------------------------------------*/

    /*-------------------------------------------------------------------
    Find a place to record the function's address range
    -------------------------------------------------------------------*/
    if ( ! felP)
        return  ERROR_NOT_ENOUGH_MEMORY;

    /*-----------------------------------------------------------------------
    Get the full module name so that we can open the debug records
    -----------------------------------------------------------------------*/
    if (actualFuncAddress)
        {
        ok = VirtualQuery ((void*)actualFuncAddress, &mbi, sizeof(mbi));
        }
    else
        {
        hDll = GetModuleHandleW (dllName);
        if (!hDll)
            return  ERROR_DLL_NOT_FOUND;
        ok = VirtualQuery (hDll, &mbi, sizeof(mbi));
        }

    if (! ok)
        return  ERROR_INVALID_ADDRESS;

    actualImageBaseAddress = (uintptr_t)mbi.AllocationBase;
    GetModuleFileName ((HMODULE)actualImageBaseAddress, moduleName, sizeof(moduleName));
    hDbg = readDbg_open (moduleName, ___, ___);
    if (!hDbg)
        return  ERROR_FILE_NOT_FOUND;

    /*-------------------------------------------------------------------
    Compensate for DLL relocation
    -------------------------------------------------------------------*/
        {
        uintptr_t   preferredImageBaseAddress,  preferredImageHighAddress;
        if ( ! readDbg_getImageRange (hDbg,  &preferredImageBaseAddress,  &preferredImageHighAddress) )
            deltaImageBaseAddress = actualImageBaseAddress - preferredImageBaseAddress;
        }

        if (actualFuncAddress)
            {
            preferredFunctionAnyAddress = actualFuncAddress + deltaImageBaseAddress;
            }
        else
            {
            char    mbFuncName[MAX_PATH];
            BeStringUtilities::WCharToCurrentLocaleChar (mbFuncName, funcName, _countof(mbFuncName));
            if (BSISUCCESS != readDbg_getProcAddr (hDbg, &preferredFunctionAnyAddress, mbFuncName))
                {
                rtn = TYPE_E_DLLFUNCTIONNOTFOUND;
                goto fini;
                }
            actualFuncAddress = preferredFunctionAnyAddress + deltaImageBaseAddress;
            }

        /*-----------------------------------------------------------------------
        Bail out if this is a duplicate address
        -----------------------------------------------------------------------*/
        if (pagalloc_bsearch ((void *) actualFuncAddress,  g_fel, g_felUsed, sizeof(g_fel)[0], bsearchfuncInFunctionExclusionList))
            {
            rtn = ERROR_DUPLICATE_TAG;
            goto fini;
            }

        /*-------------------------------------------------------------------
        Determine the function's actual address range in the current address space
        -------------------------------------------------------------------*/
        if (BSISUCCESS == readDbg_getProcRange (hDbg, ___, preferredFunctionAnyAddress, &preferredFunctionStartAddress, &preferredFunctionEndAddress) )
            {
            char    fname[MAX_PATH]= "<<Unknown>>";

            if ( !  preferredFunctionStartAddress  ||  ! preferredFunctionEndAddress)
                {
                rtn = TYPE_E_DLLFUNCTIONNOTFOUND;
                goto fini;
                }
            int const SZ_M = 512;
            char    module[SZ_M];
            module[SZ_M-1] = '\0';
            memutil_symbolizeAddress ((void*)actualFuncAddress, ___, ___, fname, sizeof(fname), module, SZ_M-1);
            felP->start = preferredFunctionStartAddress + deltaImageBaseAddress;
            felP->end   = preferredFunctionEndAddress   + deltaImageBaseAddress;

            if (actualFuncAddress != felP->start)
                pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: ****Internal error: specified function address (%#x) != to computed address (%#x).  Leak detection exclusion may not work properly.\r\n" , actualFuncAddress, felP->start);
            pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: Excluded from leak detection sweep: 0x%p to 0x%p   %s\r\n" , felP->start, felP->end, fname);
            g_felUsed = felP - g_fel + 1;
            qsort (g_fel, g_felUsed, sizeof(g_fel)[0], pagalloc_sortfuncAssendingFunctionExclusionList);
            rtn = BSISUCCESS;
            goto fini;
            }
        rtn = ERROR_PROC_NOT_FOUND;

fini:
        readDbg_close (hDbg,  ___);
        return  rtn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   03/2002
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API  Int32 __cdecl pagalloc_addFunctionByAddressToExclusionList
(
uintptr_t const funcAddress
)
    {
    if ( ! funcAddress)
        return      ERROR_INVALID_ADDRESS;

    return addAddressToExclusionList (___, ___, funcAddress);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   03/2002
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API  StatusInt __cdecl pagalloc_addFunctionByDllAndNameToExclusionList
(
WCharCP dllName,         // =>
WCharCP funcName         // =>  C++ names must be mangled
)
    {
    return  addAddressToExclusionList (dllName, funcName, ___);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   05/2002
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API  UInt32 __cdecl pagalloc_setUserAuxiliaryValue
(
UInt32          const newUserAuxiliaryValue,        // =>
void            const * const notYet                        // =>  Will eventually be an output function pointer
)
    {
    UInt32     const prevValue = g_userAuxiliaryValue;
    g_userAuxiliaryValue = newUserAuxiliaryValue;
    return  prevValue;
    }

void GdiMonitor_Init ();
UInt32 GdiMonitor_GetCurrentEntryID ();
void GdiMonitor_Dump (UInt32 start, void (*callbackFunc) (PageMallocEntry const * const, uintptr_t, UInt32, UInt32) );
static UInt32 s_gdiLeakStart = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Chuck.Kirschman   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API  void __cdecl pagalloc_initializeGDIMonitor (void)
    {
#if !defined (NDEBUG)
    GdiMonitor_Init ();
#endif
    }

#if !defined (NDEBUG)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Chuck.Kirschman   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static  void pagalloc_showGDILeaksCallback (PageMallocEntry const * const entry, uintptr_t handleId, UInt32 leakNumber, UInt32 count)
    {
    pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "PAGALLOC GDI:\r\n");
    pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "PAGALLOC GDI:      ======================================================================================\r\n");
    pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "PAGALLOC GDI:\r\n");
    pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "%d GDI Handle %lx (this is 1 of %d times you will see this allocator)\r\n", leakNumber, handleId, count); // Would be nice to dump the type here
    pagalloc_symbolizeHeaderAddresses (entry);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Chuck.Kirschman   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API  void __cdecl pagalloc_showGDILeaks (void)
    {
#if !defined (NDEBUG)
    pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: *******************************************************\r\n");
    pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: GDI Objects leaked ************************************\r\n");
    pagalloc_printf(PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: *******************************************************\r\n\r\n");
    GdiMonitor_Dump (s_gdiLeakStart, pagalloc_showGDILeaksCallback);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Chuck.Kirschman   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API void __cdecl pagalloc_setGDISinceBase (UInt32 beginningGDICount)
    {
#if !defined (NDEBUG)
    if (0 == beginningGDICount)
        s_gdiLeakStart = GdiMonitor_GetCurrentEntryID();
    else        
        s_gdiLeakStart = beginningGDICount;
#endif
    }

/*
        All this critical section stuff is unworkable because it uses bits that Microsoft had reserved and then started allocating
        and using for their own purposes as of Windows 2003 Server SP1.  That wasn't reflected in the Windows SDK that comes with
        Visual Studio until Visual Studio 2007. At that time the SDK showed that there are only 16 spare bits that remain.

        So, really, this feature of pagalloc is just plain broken on Win 2003 Server SP, Vista and one wonders if it works under
        wow64 on 64-bit XP. So this stuff won't compile with VC9.  For now I just blank it out for VC9.

        See http://blogs.msdn.com/oldnewthing/archive/2005/07/01/434648.aspx

        Addendum: It is probably possible to make this work on newer systems, because we control the routine that allocate the critsec
        structure, and if we allocate more space than it needs, we can store our bookkeeping data in the extra space. However, this may
        be more effort than it's worth.
*/
#if _MSC_VER <= 1400

typedef struct  criticalsectionimage
    {
    RTL_CRITICAL_SECTION        cs;
    RTL_CRITICAL_SECTION_DEBUG  csDebug;
    } CriticalSectionImage;

#define  PAPATCH_No_MSCVRT_Patch_Tables     (1)
#include <DgnPlatform/Tools/papatch.h>
//
//
//      dumpbin -exports %windir%\system32\kernel32.dll | findstr CriticalSec
//              123   7A          DeleteCriticalSection (forwarded to NTDLL.RtlDeleteCriticalSection)
//              144   8F          EnterCriticalSection (forwarded to NTDLL.RtlEnterCriticalSection)
//              527  20E 0001A745 InitializeCriticalSection
//              528  20F 0001DE5E InitializeCriticalSectionAndSpinCount
//              570  239          LeaveCriticalSection (forwarded to NTDLL.RtlLeaveCriticalSection)
//              752  2EF          SetCriticalSectionSpinCount (forwarded to NTDLL.RtlSetCriticalSectionSpinCount)
//              845  34C          TryEnterCriticalSection (forwarded to NTDLL.RtlTryEnterCriticalSection)
//
//
//
//        cs2.exe       00400000-00413000       O:\CriticalSection\cs2.exe      1                       [3808] cs2.exe: Native  1/8/2004 1:27 PM        Symbols loaded.
//        ntdll.dll     77F50000-77FF7000       C:\WINNT\system32\ntdll.dll     2       5.01.2600.1217  [3808] cs2.exe: Native  5/1/2003 6:56 PM        Symbols loaded.
//        kernel32.dll  77E60000-77F46000       C:\WINNT\system32\kernel32.dll  3       5.01.2600.1106  [3808] cs2.exe: Native  8/29/2002 5:40 AM       Symbols loaded.
//
//
//      Kernel32!_InitializeCriticalSection@4:
//        77E7A745 FF 74 24 04        push        dword ptr [esp+4]
//        77E7A749 FF 15 1C 12 E6 77  call        dword ptr [__imp__RtlInitializeCriticalSection@4 (77E6121Ch)]
//        77E7A74F 85 C0              test        eax,eax
//        77E7A751 0F 8C D2 5C 02 00  jl          _InitializeCriticalSection@4+0Eh (77EA0429h)
//        77E7A757 C2 04 00           ret         4
//
//        77EA0429 50                 push        eax
//        77EA042A FF 15 FC 13 E6 77  call        dword ptr [__imp__RtlRaiseStatus@4 (77E613FCh)]
//        77EA0430 E9 22 A3 FD FF     jmp         _InitializeCriticalSection@4+15h (77E7A757h)
//
//
//      Kernel32!_InitializeCriticalSectionAndSpinCount@8:
//        77E7DE5E 56               push        esi
//        77E7DE5F FF 74 24 0C      push        dword ptr [esp+0Ch]
//        77E7DE63 33 F6            xor         esi,esi
//        77E7DE65 FF 74 24 0C      push        dword ptr [esp+0Ch]
//        77E7DE69 46               inc         esi
//        77E7DE6A FF 15 8C 14 E6 77 call        dword ptr [__imp__RtlInitializeCriticalSectionAndSpinCount@8 (77E6148Ch)]
//        77E7DE70 85 C0            test        eax,eax
//        77E7DE72 0F 8C BD 25 02 00 jl          _InitializeCriticalSectionAndSpinCount@8+16h (77EA0435h)
//        77E7DE78 8B C6            mov         eax,esi
//        77E7DE7A 5E               pop         esi
//        77E7DE7B C2 08 00         ret         8
//
//        77EA0435 50               push        eax
//        77EA0436 E8 2B A0 FD FF   call        _BaseSetLastNTError@4 (77E7A466h)
//        77EA043B 33 F6            xor         esi,esi
//        77EA043D E9 36 DA FD FF   jmp         _InitializeCriticalSectionAndSpinCount@8+1Eh (77E7DE78h)
//
//
//
//
//
//
//  XP _RtlInitializeCriticalSection@4:
//        77F557D2 6A 00            push        0
//        77F557D4 FF 74 24 08      push        dword ptr [esp+8]
//        77F557D8 E8 83 43 01 00   call        _RtlInitializeCriticalSectionAndSpinCount@8 (77F69B60h)
//        77F557DD C2 04 00         ret         4
//
//  2K  RtlInitializeCriticalSection:
//        77F95749 6A 00                push        0
//        77F9574B FF 74 24 08          push        dword ptr [esp+8]
//        77F9574F E8 9F F0 FE FF       call        RtlInitializeCriticalSectionAndSpinCount (77f847f3)
//        77F95754 C2 04 00             ret         4
//
//===============
//
//  XP  _RtlDeleteCriticalSection@4:  WindowsXP
//        77F69D12 6A 1C            push        1Ch
//        77F69D14 68 30 16 F5 77   push        77F51630h
//        77F69D19 E8 36 B8 03 00   call        __SEH_prolog (77FA5554h)
//        77F69D1E 8B 5D 08         mov         ebx,dword ptr [ebp+8]
//
//  2K  RtlDeleteCriticalSection:
//        77F87BF9 55                   push        ebp
//        77F87BFA 8B EC                mov         ebp,esp
//        77F87BFC 6A FF                push        0FFh
//        77F87BFE 68 B0 7C F8 77       push        offset RtlDeleteCriticalSection+0B7h (77f87cb0)
//
//===============
//
//  XP  _RtlInitializeCriticalSectionAndSpinCount@8:
//        77F69B60 55               push        ebp
//        77F69B61 8B EC            mov         ebp,esp
//        77F69B63 83 EC 20         sub         esp,20h
//        77F69B66 53               push        ebx
//        77F69B67 33 DB            xor         ebx,ebx
//
//  2K   RtlInitializeCriticalSectionAndSpinCount:
//        77F847F3 55                   push        ebp
//        77F847F4 8B EC                mov         ebp,esp
//        77F847F6 53                   push        ebx
//        77F847F7 56                   push        esi
//        77F847F8 57                   push        edi
//
//

static BYTE *rtlInitializeCriticalSectionP;
static BYTE *rtlDeleteCriticalSectionP;
static BYTE *rtlInitializeCriticalSectionAndSpinCountP;

static const TestControl  s_Win2Ksp4RtlInitializeCriticalSectionCtrl[]                                               = {VEQU,VEQU,VEQU,VEQU,VEQU,VEQU};
static const BYTE         s_Win2Ksp4RtlInitializeCriticalSectionTest[_countof(s_Win2Ksp4RtlInitializeCriticalSectionCtrl)] = {0x6A,0x00,0xFF,0x74,0x24,0x08};
static       BYTE         s_Win2Ksp4RtlInitializeCriticalSectionSave[_countof(s_Win2Ksp4RtlInitializeCriticalSectionCtrl)];

static const TestControl  s_Win2Ksp4RtlDeleteCriticalSectionCtrl[]                                          = {VEQU,VEQU,VEQU,VEQU,VEQU};
static const BYTE         s_Win2Ksp4RtlDeleteCriticalSectionTest[_countof(s_Win2Ksp4RtlDeleteCriticalSectionCtrl)] = {0x55,0x8b,0xec,0x6a,0xff};
static       BYTE         s_Win2Ksp4RtlDeleteCriticalSectionSave[_countof(s_Win2Ksp4RtlDeleteCriticalSectionCtrl)];

static const TestControl  s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountCtrl[]                                                          = {VEQU,VEQU,VEQU,VEQU,VEQU,VEQU};
static const BYTE         s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountTest[_countof(s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountCtrl)] = {0x55,0x8B,0xEC,0x53,0x56,0x57};
static       BYTE         s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountSave[_countof(s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountCtrl)];

//---//

static const TestControl  s_WinXPsp1RtlInitializeCriticalSectionCtrl[]                                               = {VEQU,VEQU,VEQU,VEQU,VEQU,VEQU};
static const BYTE         s_WinXPsp1RtlInitializeCriticalSectionTest[_countof(s_WinXPsp1RtlInitializeCriticalSectionCtrl)] = {0x6A,0x00,0xFF,0x74,0x24,0x08};
static       BYTE         s_WinXPsp1RtlInitializeCriticalSectionSave[_countof(s_WinXPsp1RtlInitializeCriticalSectionCtrl)];

static const TestControl  s_WinXPsp1RtlDeleteCriticalSectionCtrl[]                                          = {VEQU,VEQU,VEQU,V___,V___,V___,V___};
static const BYTE         s_WinXPsp1RtlDeleteCriticalSectionTest[_countof(s_WinXPsp1RtlDeleteCriticalSectionCtrl)] = {0x6A,0x1c,0x68,0x30,0x16,0xf5,0x77};
static       BYTE         s_WinXPsp1RtlDeleteCriticalSectionSave[_countof(s_WinXPsp1RtlDeleteCriticalSectionCtrl)];

static const TestControl  s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountCtrl[]                                                          = {VEQU,VEQU,VEQU,VEQU,VEQU,VEQU};
static const BYTE         s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountTest[_countof(s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountCtrl)] = {0x55,0x8B,0xEC,0x83,0xEC,0x20};
static       BYTE         s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountSave[_countof(s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountCtrl)];

// Even though NTDLL.DLL has different checksum values between Win2K and WinXP, the CriticalSection function entry points are the same
typedef enum
    {
    Win2Ksp4 = 0x7A68C,
    WinXPsp1 = 0xA5841
    } NTDllBuild;

static NTDllBuild  ntDllBuild;

typedef void (__stdcall *PfnRtlInitializeCriticalSection) (LPCRITICAL_SECTION csP);
typedef void (__stdcall *PfnRtlDeleteCriticalSection)     (LPCRITICAL_SECTION csP);
typedef BOOL (__stdcall *PfnRtlInitializeCriticalSectionAndSpinCount)     (IN OUT LPCRITICAL_SECTION lpCriticalSection,  IN UInt32 dwSpinCount );


typedef struct ntdllPatchData
    {
    BYTE                        codeRtlInitializeCriticalSection[16];
    BYTE                        codeRtlDeleteCriticalSection[16];
    BYTE                        codeRtlInitializeCriticalSectionAndSpinCount[16];

    PfnRtlInitializeCriticalSection             callRtlInitializeCriticalSection;
    PfnRtlDeleteCriticalSection                 callRtlDeleteCriticalSection;
    PfnRtlInitializeCriticalSectionAndSpinCount callRtlInitializeCriticalSectionAndSpinCount;
    } NtdllPatchData;

static NtdllPatchData  *ntdllPatchData;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   01/2004
+---------------+---------------+---------------+---------------+---------------+------*/
PatchedCall BOOL WINAPI     pagalloc_rtlInitializeCriticalSectionAndSpinCount
(
LPCRITICAL_SECTION  lpCriticalSection,
UInt32              dwSpinCount
)
    {
    BOOL     rtn = 0;
    memset (lpCriticalSection, 0, sizeof *lpCriticalSection);
    rtn = (ntdllPatchData->callRtlInitializeCriticalSectionAndSpinCount) (lpCriticalSection, dwSpinCount);

    // if (rtn)            // The docs seem to be wrong, this always returns zero
    if (lpCriticalSection  &&  lpCriticalSection->DebugInfo)
        {
        CriticalSectionImage  * const csi = pagallocI_malloc (sizeof *csi, PAGALLOC_CONTEXT, LOP_CriticalSection, pagalloc_rtlInitializeCriticalSectionAndSpinCount);
        csi->cs      = *lpCriticalSection;
        csi->csDebug = *lpCriticalSection->DebugInfo;

        lpCriticalSection->DebugInfo->Spare[0] = PAGALLOC_CRITICAL_SECTION_SIGNATURE;
        lpCriticalSection->DebugInfo->Spare[1] = (UInt32) csi;

        s_countInitializeCriticalSection++;
        }
    else
        pagalloc_printf (PAGALLOC_SEVERITY_ERROR, "PAGALLOC: RtlInitializeCriticalSectionAndSpinCount failed unexpectedly, GLE %#x\r\n", GetLastError());

    return  rtn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   01/2004
+---------------+---------------+---------------+---------------+---------------+------*/
PatchedCall VOID WINAPI     pagalloc_rtlInitializeCriticalSection
(
LPCRITICAL_SECTION lpCriticalSection
)
    {
    pagalloc_rtlInitializeCriticalSectionAndSpinCount (lpCriticalSection, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   01/2004
+---------------+---------------+---------------+---------------+---------------+------*/
PatchedCall VOID WINAPI     pagalloc_rtlDeleteCriticalSection
(
LPCRITICAL_SECTION lpCriticalSection
)
    {
    if (PAGALLOC_CRITICAL_SECTION_SIGNATURE == lpCriticalSection->DebugInfo->Spare[0])
        {
        pagallocI_free ((void*)lpCriticalSection->DebugInfo->Spare[1], PAGALLOC_CONTEXT, pagalloc_rtlDeleteCriticalSection);
        s_countDeleteCriticalSection++;
        }

    (ntdllPatchData->callRtlDeleteCriticalSection) (lpCriticalSection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   01/2004
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API  StatusInt __cdecl pagalloc_trackCriticalSections
(
bool const tracking                                                          // true to enable tracking
)
    {
    if (tracking)
        {
        HINSTANCE   const hmodNTDLL = GetModuleHandle ("ntdll.dll");
        if (NULL == hmodNTDLL)
            {
            pagalloc_printf (PAGALLOC_SEVERITY_ERROR, "PAGALLOC: GetModuleHandle failed, GLE %#x\r\n", GetLastError());
            DebugBreak();
            return -1;
            }

        rtlInitializeCriticalSectionP             = (BYTE*) GetProcAddress (hmodNTDLL, "RtlInitializeCriticalSection");
        rtlDeleteCriticalSectionP                 = (BYTE*) GetProcAddress (hmodNTDLL, "RtlDeleteCriticalSection");
        rtlInitializeCriticalSectionAndSpinCountP = (BYTE*) GetProcAddress (hmodNTDLL, "RtlInitializeCriticalSectionAndSpinCount");

        if ( ! rtlInitializeCriticalSectionP  ||  ! rtlDeleteCriticalSectionP  ||  ! rtlInitializeCriticalSectionAndSpinCountP)
            {
            pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: Can't patch CriticalSection functions.\r\n");
            return  -4;
            }

        if (isFuncPatchable (rtlInitializeCriticalSectionP,             s_Win2Ksp4RtlInitializeCriticalSectionCtrl,             s_Win2Ksp4RtlInitializeCriticalSectionTest,              sizeof(s_Win2Ksp4RtlInitializeCriticalSectionTest))       &&
            isFuncPatchable (rtlDeleteCriticalSectionP,                 s_Win2Ksp4RtlDeleteCriticalSectionCtrl,                 s_Win2Ksp4RtlDeleteCriticalSectionTest,                  sizeof(s_Win2Ksp4RtlDeleteCriticalSectionTest))           &&
            isFuncPatchable (rtlInitializeCriticalSectionAndSpinCountP, s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountCtrl, s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountTest,  sizeof(s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountTest))
            )
            {
            UInt32   oldProt = 0;

            ntDllBuild = Win2Ksp4;

            pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: Patching Win2K sp4 NTDLL.DLL\r\n");

            /* save a copy of the originals for unpatching */
            memcpy (s_Win2Ksp4RtlInitializeCriticalSectionSave,             rtlInitializeCriticalSectionP,             sizeof(s_Win2Ksp4RtlInitializeCriticalSectionSave));
            memcpy (s_Win2Ksp4RtlDeleteCriticalSectionSave,                 rtlDeleteCriticalSectionP,                 sizeof(s_Win2Ksp4RtlDeleteCriticalSectionSave));
            memcpy (s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountSave, rtlInitializeCriticalSectionAndSpinCountP, sizeof(s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountSave));

            ntdllPatchData = (NtdllPatchData *)  VirtualAlloc (___, sizeof *ntdllPatchData, MEM_COMMIT, PAGE_READWRITE);

            createPatchDataEntry (ntdllPatchData->codeRtlInitializeCriticalSection,             rtlInitializeCriticalSectionP,              s_Win2Ksp4RtlInitializeCriticalSectionCtrl,             sizeof(s_Win2Ksp4RtlInitializeCriticalSectionTest));
            createPatchDataEntry (ntdllPatchData->codeRtlDeleteCriticalSection,                 rtlDeleteCriticalSectionP,                  s_Win2Ksp4RtlDeleteCriticalSectionCtrl,                 sizeof(s_Win2Ksp4RtlDeleteCriticalSectionTest));
            createPatchDataEntry (ntdllPatchData->codeRtlInitializeCriticalSectionAndSpinCount, rtlInitializeCriticalSectionAndSpinCountP,  s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountCtrl, sizeof(s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountTest));

            ntdllPatchData->callRtlInitializeCriticalSection                = (PfnRtlInitializeCriticalSection)            (void*)ntdllPatchData->codeRtlInitializeCriticalSection;
            ntdllPatchData->callRtlDeleteCriticalSection            = (PfnRtlDeleteCriticalSection)                (void*)ntdllPatchData->codeRtlDeleteCriticalSection;
            ntdllPatchData->callRtlInitializeCriticalSectionAndSpinCount= (PfnRtlInitializeCriticalSectionAndSpinCount)(void*)ntdllPatchData->codeRtlInitializeCriticalSectionAndSpinCount;

            // Write-protect this page
            VirtualProtect (ntdllPatchData, sizeof *ntdllPatchData, PAGE_EXECUTE_READ, &oldProt);

            // Now patch the runtime image - insert a branch to our replacement functions for each function
            patchRuntimeFunction (rtlInitializeCriticalSectionP,         (BYTE*)pagalloc_rtlInitializeCriticalSection);
            patchRuntimeFunction (rtlDeleteCriticalSectionP,                     (BYTE*)pagalloc_rtlDeleteCriticalSection);
            patchRuntimeFunction (rtlInitializeCriticalSectionAndSpinCountP, (BYTE*)pagalloc_rtlInitializeCriticalSectionAndSpinCount);
            }
        else
            if (isFuncPatchable (rtlInitializeCriticalSectionP,             s_WinXPsp1RtlInitializeCriticalSectionCtrl,             s_WinXPsp1RtlInitializeCriticalSectionTest,   sizeof(s_WinXPsp1RtlInitializeCriticalSectionTest))   &&
                isFuncPatchable (rtlDeleteCriticalSectionP,                 s_WinXPsp1RtlDeleteCriticalSectionCtrl,                 s_WinXPsp1RtlDeleteCriticalSectionTest,   sizeof(s_WinXPsp1RtlDeleteCriticalSectionTest))           &&
                isFuncPatchable (rtlInitializeCriticalSectionAndSpinCountP, s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountCtrl, s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountTest,  sizeof(s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountTest))
                )
                {
                UInt32   oldProt = 0;

                ntDllBuild = WinXPsp1;

                pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "PAGALLOC: Patching WinXP sp1 NTDLL.DLL\r\n");

                /* save a copy of the originals for unpatching */
                memcpy (s_WinXPsp1RtlInitializeCriticalSectionSave,             rtlInitializeCriticalSectionP,             sizeof(s_WinXPsp1RtlInitializeCriticalSectionSave));
                memcpy (s_WinXPsp1RtlDeleteCriticalSectionSave,                 rtlDeleteCriticalSectionP,                 sizeof(s_WinXPsp1RtlDeleteCriticalSectionSave));
                memcpy (s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountSave, rtlInitializeCriticalSectionAndSpinCountP, sizeof(s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountSave));

                ntdllPatchData = (NtdllPatchData *)  VirtualAlloc (___, sizeof *ntdllPatchData, MEM_COMMIT, PAGE_READWRITE);

                createPatchDataEntry (ntdllPatchData->codeRtlInitializeCriticalSection,             rtlInitializeCriticalSectionP,              s_WinXPsp1RtlInitializeCriticalSectionCtrl,             sizeof(s_WinXPsp1RtlInitializeCriticalSectionTest));
                createPatchDataEntry (ntdllPatchData->codeRtlDeleteCriticalSection,                 rtlDeleteCriticalSectionP,                  s_WinXPsp1RtlDeleteCriticalSectionCtrl,                 sizeof(s_WinXPsp1RtlDeleteCriticalSectionTest));
                createPatchDataEntry (ntdllPatchData->codeRtlInitializeCriticalSectionAndSpinCount, rtlInitializeCriticalSectionAndSpinCountP,  s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountCtrl, sizeof(s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountTest));

                ntdllPatchData->callRtlInitializeCriticalSection                = (PfnRtlInitializeCriticalSection)            (void*)ntdllPatchData->codeRtlInitializeCriticalSection;
                ntdllPatchData->callRtlDeleteCriticalSection            = (PfnRtlDeleteCriticalSection)                (void*)ntdllPatchData->codeRtlDeleteCriticalSection;
                ntdllPatchData->callRtlInitializeCriticalSectionAndSpinCount= (PfnRtlInitializeCriticalSectionAndSpinCount)(void*)ntdllPatchData->codeRtlInitializeCriticalSectionAndSpinCount;

                // Write-protect this page
                VirtualProtect (ntdllPatchData, sizeof *ntdllPatchData, PAGE_EXECUTE_READ, &oldProt);

                // Now patch the runtime image - insert a branch to our replacement functions for each function
                patchRuntimeFunction (rtlInitializeCriticalSectionP,         (BYTE*)pagalloc_rtlInitializeCriticalSection);
                patchRuntimeFunction (rtlDeleteCriticalSectionP,                     (BYTE*)pagalloc_rtlDeleteCriticalSection);
                patchRuntimeFunction (rtlInitializeCriticalSectionAndSpinCountP, (BYTE*)pagalloc_rtlInitializeCriticalSectionAndSpinCount);
                }
            else
                {
                OutputDebugString ("PAGALLOC: can't match NTDLL.DLL code for patching.\r\n");
                paPatch_complain("PAGALLOC: attempt to patch NTDLL.DLL failed.\r\nPAGALLOC: unsupported NTDLL.DLL found.\r\n");
                return -2;
                }

            return 0;
        }

    // Else detach our self from NTDLL.DLL
    switch  (ntDllBuild)
        {
    case Win2Ksp4:
        unpatchRuntimeFunction (rtlInitializeCriticalSectionP,          s_Win2Ksp4RtlInitializeCriticalSectionSave,             sizeof(s_Win2Ksp4RtlInitializeCriticalSectionSave));
        unpatchRuntimeFunction (rtlDeleteCriticalSectionP,                      s_Win2Ksp4RtlDeleteCriticalSectionSave,                 sizeof(s_Win2Ksp4RtlDeleteCriticalSectionSave));
        unpatchRuntimeFunction (rtlInitializeCriticalSectionAndSpinCountP,      s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountSave, sizeof(s_Win2Ksp4RtlInitializeCriticalSectionAndSpinCountSave));
        break;

    case WinXPsp1:
        unpatchRuntimeFunction (rtlInitializeCriticalSectionP,          s_WinXPsp1RtlInitializeCriticalSectionSave,             sizeof(s_WinXPsp1RtlInitializeCriticalSectionSave));
        unpatchRuntimeFunction (rtlDeleteCriticalSectionP,                      s_WinXPsp1RtlDeleteCriticalSectionSave,                 sizeof(s_WinXPsp1RtlDeleteCriticalSectionSave));
        unpatchRuntimeFunction (rtlInitializeCriticalSectionAndSpinCountP,      s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountSave, sizeof(s_WinXPsp1RtlInitializeCriticalSectionAndSpinCountSave));
        break;

    default:
        pagalloc_printf (PAGALLOC_SEVERITY_ERROR, "PAGALLOC: unexpected 'ntDllBuild' value %#x in detach.\r\n", ntDllBuild);
        DebugBreak();
        return -3;
        break;
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   01/2004
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API   Int32     pagalloc_walkCriticalSectionList
(
HANDLE const m_hProcess
)
    {
    RTL_CRITICAL_SECTION_DEBUG      csdLoaderLock;
    RTL_CRITICAL_SECTION_DEBUG      csdDeferedCriticalSectionDebug;
    RTL_CRITICAL_SECTION_DEBUG      csdCriticalSectionLockDebug;
    UInt32                     const PEBLoaderLock = 0x7ffdf0a0;
    PDWORD                          pLoaderLockAddress;
    RTL_CRITICAL_SECTION            csLoaderLock;
    RTL_CRITICAL_SECTION_DEBUG  *   pListEntry;
    UInt32                  cnt = 1;
    UInt32                   totalCS;

#if defined (Use_Local_Pointers) && 0
    __asm
        {
        mov eax, fs:[30h]       ;Teb.Peb                          0x7ffdf000
            mov eax, [eax+0a0h]     ;Peb.Ldr - PEB_LDR_DATA
            mov pLoaderLockAddress, eax                                              0x77fcf348
        }
#else
    // I have a hope of using this in an external program, too.  This is why I am using ReadProcessMemory instead of direct pointer manipulation.
    // Locate the address of the loader lock critical section in the process.
    if ( !ReadProcessMemory (m_hProcess, (PVOID) PEBLoaderLock, &pLoaderLockAddress, sizeof(pLoaderLockAddress), ___) )
        return -1;

    // Load the loader lock critical section into our own buffer.
    if ( !ReadProcessMemory (m_hProcess, pLoaderLockAddress, &csLoaderLock, sizeof(csLoaderLock), ___) )
        return -2;
#endif

    //1    if (pPebLdr < (PEB_LDR_DATA*)0x80000000)                                    // Must be NT series OS
    //1        {

    // Load the debug structure for the loader lock into our own buffer.
    // This is the 3rd debug area in the doubly-linked list.
    if ( !ReadProcessMemory (m_hProcess, csLoaderLock.DebugInfo, &csdLoaderLock, sizeof(csdLoaderLock), ___) )
        return -3;

    // We need to walk the list backwards to the 1st entry.
    pListEntry = (RTL_CRITICAL_SECTION_DEBUG*) ((UInt32) csdLoaderLock.ProcessLocksList.Blink  - (offsetof (RTL_CRITICAL_SECTION_DEBUG, ProcessLocksList)));

    // Load the 2nd debug area, i.e., the defered (sic) critical section debug structure.
    if ( !ReadProcessMemory (m_hProcess, pListEntry, &csdDeferedCriticalSectionDebug, sizeof(csdDeferedCriticalSectionDebug), ___))
        return -4;

    pListEntry = (RTL_CRITICAL_SECTION_DEBUG*)
        ((UInt32) csdDeferedCriticalSectionDebug.ProcessLocksList.Blink - (offsetof (RTL_CRITICAL_SECTION_DEBUG, ProcessLocksList)));

    // Now load the 1st debug area, i.e., the critical section lock debug structure.
    // Its previous or backward link - BLink - will mark the end of the list.
    if ( ! ReadProcessMemory (m_hProcess, pListEntry, &csdCriticalSectionLockDebug, sizeof(csdCriticalSectionLockDebug), ___))
        return -5;

    // Now, walk the chain from the beginning to the end.
    for (totalCS=0; ; totalCS++)
        {
        RTL_CRITICAL_SECTION_DEBUG      cs_debug;
        RTL_CRITICAL_SECTION            cs;

        // Read the critical section debug structure first...
        if ( ! ReadProcessMemory (m_hProcess, pListEntry, &cs_debug, sizeof(cs_debug), ___) )
            break;

        // since this will give us the address to the critical section.
        if ( !ReadProcessMemory (m_hProcess, cs_debug.CriticalSection, &cs, sizeof(cs), ___) )
            break;

        // Spare[0] contains PAGALLOC_CRITICAL_SECTION_SIGNATURE
        // Spare[1] contains a pointer to the pagalloc proxy data page
        if (PAGALLOC_CRITICAL_SECTION_SIGNATURE == cs_debug.Spare[0])
            {
            UInt32                    dwOldProtect = 0;
            PageMallocEntry const * const headerP = (PageMallocEntry const * const ) pagallocI_findHeader ((void*)cs_debug.Spare[1]);

            pagallocI_makeHeaderVisible (headerP, PAGE_READONLY, &dwOldProtect);

            pagalloc_printf (PAGALLOC_SEVERITY_DEBUG,
                "PAGALLOC:\r\n"
                "PAGALLOC:      ======================================================================================\r\n"
                "PAGALLOC:\r\n");

            pagalloc_printf (PAGALLOC_SEVERITY_DEBUG,
                "PAGALLOC: %2d. CriticalSection 0x%p from (sequence %d, origination %s)\r\n",
                cnt++, cs_debug.CriticalSection,
                headerP->mallocSerialNumber, pagalloc_toStringLastOperation(headerP));

            pagalloc_printf (PAGALLOC_SEVERITY_DEBUG,
                "PAGALLOC:    Lock %d, Recursion %u, TID %u, Semaphore %#x, Spin %u --\r\n",
                cs.LockCount, cs.RecursionCount, cs.OwningThread, cs.LockSemaphore, cs.SpinCount);

            pagalloc_symbolizeHeaderAddressesForVisualC (headerP);
            /* leave protection exactly as we found it */
            pagallocI_makeHeaderInvisible (headerP, dwOldProtect, ___);

            }
        else
            {
            //            pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "%5d.  %8x  %5d  %8x  %8x  %8x  %8x\n",
            //                cnt++,  cs.DebugInfo, cs.LockCount, cs.RecursionCount, cs.OwningThread,  cs.LockSemaphore,  cs.SpinCount);
            }

        // When the next or forward link is the same as the critical section lock's backward link we have reached the end of the list.
        if (cs_debug.ProcessLocksList.Flink == csdCriticalSectionLockDebug.ProcessLocksList.Blink)
            break;

        // The forward link (Flink) actually points into the middle of the
        // next critical section debug structure, so we take its address and
        // back up the correct number of DWORDs to obtain the next entry in
        // the chain.  Use the offsetof macro to let the structures and the
        // compiler do the actual work.
        pListEntry = (RTL_CRITICAL_SECTION_DEBUG*)  ((UInt32) cs_debug.ProcessLocksList.Flink - (offsetof (RTL_CRITICAL_SECTION_DEBUG, ProcessLocksList)));
        }

    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG,
        "PAGALLOC:\r\n"
        "PAGALLOC: Found %d Critical Sections, tracking %d.\r\n"
        "PAGALLOC:\r\n", totalCS, cnt-1);       // cnt starts at 1 for display purposes

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   01/2004
+---------------+---------------+---------------+---------------+---------------+------*/
PAGALLOC_API  void __cdecl pagalloc_symbolizeAllCriticalSections
(
void
)
    {
    pagalloc_walkCriticalSectionList (GetCurrentProcess());     // The pseudo handle need not be closed when it is no longer needed. Calling the CloseHandle function with a pseudo handle has no effect.
    }
#endif
