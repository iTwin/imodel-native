#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <errno.h>

#define NO_IODEFS
#include <DgnPlatform/Tools/pagalloc.h>
#include <DgnPlatform/Tools/pagalloc.fdf>
#include <RmgrTools/Tools/memutil.h>

#include <DgnPlatform/Tools/papatch.h>

#pragma comment (lib, "user32")

#include <Bentley/Bentley.r.h>


/* Dummy class to test that new/delete on custom classes works */
class customConstructor
    {
    public:
        byte* dynamicMem;
        Int32 constMem[10000];
        customConstructor(Int32 memSize)
            {
            dynamicMem = new byte[memSize];
            }
        ~customConstructor()
            {
            delete dynamicMem;
            }

    };

void test_customConstructor()
    {
    customConstructor* c1 = new customConstructor(4096);
    //leak
    customConstructor* c2 = new customConstructor(4096);
    delete c1;
    c2 = NULL; // Quiet the compiler.
    }

void  test_strdup()
    {
    Int32 i;
    char *cp, fmt[]=" strdup %2d @ %#x\n";

    printf ("  Begin test_strdup\n");
    // we can't detect this because the call stack is different
    cp = strdup("from test_strdup");
    printf (fmt, 1, cp);
    cp = strdup("from test_strdup");
    printf (fmt, 2, cp);
    cp = strdup("from test_strdup");
    printf (fmt, 3, cp);
    // +3 leaks
    // we can detect this because the call stack is the same
    for (i=0; i < 10; i++)
        {
        cp = strdup("from test_strdup");
        printf (fmt, i+10, cp);
        if ( i & 1)
            free (cp);
        }                                                                       // +5 leaks
    printf ("  done  test_strdup\n");
    }

byte *a[10];
void  test_cppNew_new
(
void
)
    {
    for (Int32 i=0; i < 10; i++)
        {
        a[i] = new byte[11+i];                                                  // + 10 leaks
        memset (a[i], 'A'+i, 11+i);
        }
    }

void  test_cppNew_delete
(
)
{
    for (Int32 i=0; i < 10; i+=3)   // 6 leaked allocations here                        // + 6 leaks
        //    for (Int32 i=0; i < 10; i+=1)     // No leak here
        delete [] a[i];

    delete [] a[0];     // force a double free
    }


/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/nonport/winnt/test_pagalloc.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   10/2001
+---------------+---------------+---------------+---------------+---------------+------*/
void            test_doubleFree
(
)
{
    Int32     i=0;
    void *vp0, *vp0p, *vp1, *vp2;
    printf("PagallocOutputFunction\n");
    printf ("  Begin test_doubleFree\n");
    vp0p = vp0 = malloc(100);
    free (vp0);

    vp1 = malloc(100);
    i++;
    vp2 = malloc(100);
    i++;

    vp0 = vp2;
    i++;
    vp0 = vp2;
    i++;
    vp0 = vp2;
    i++;

    free (vp2);
    i++;
    free (vp1);
    i++;

    free (vp0);             // force error here
    i++;

    free (vp0p);            // force error here
    i++;

    for (i=0; i < 5; i++)
        {
        vp1 = malloc(0x65);     // Leak here
        }

    printf ("  done  test_doubleFree\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   10/2001
+---------------+---------------+---------------+---------------+---------------+------*/
void test_heapWalk
(
void
)
{
    _HEAPINFO    hinfo;
    Int32        heapstatus;

    printf ("test_heapWalk: \n");
    memset (&hinfo, 0, sizeof(hinfo));
    //    while ( (heapstatus = pagalloc_heapWalk (&hinfo) ) == _HEAPOK )
    while ( (heapstatus = _heapwalk (&hinfo) ) == _HEAPOK )
        {
        printf ("%-6s block at %Fp of size %4.4X\n", ( hinfo._useflag == _USEDENTRY ? "USED" : "free" ),  hinfo._pentry, hinfo._size );
        }

    switch( heapstatus )
        {
    case _HEAPEMPTY:
        printf( "OK - empty heap\n" );
        break;

    case _HEAPEND:
        printf( "OK - end of heap\n" );
        break;

    case _HEAPBADPTR:
        printf( "ERROR - bad pointer to heap\n" );
        break;

    case _HEAPBADBEGIN:
        printf( "ERROR - bad start of heap\n" );
        break;

    case _HEAPBADNODE:
        printf( "ERROR - bad node in heap\n" );
        break;
        }
    }

#define EVAL(a) #a
#define EVALL(a) L#a
void test_checkered
    (
    void
    )
    {
    // 2 "New" blocks
    byte *a = new  byte [12];
    byte *b = new  byte [13];

    a = NULL; b = NULL;
    delete new  byte [14];

    // 3 block outside of the SBH
    for (Int32 i=1; i < 4; i++)
        {
        byte* x = (byte*)calloc(1, 4096 + i);
        //Check for proper initialization
        for(Int32 j=0; j < 4096 + i; j++)
            if(x[j])
                DebugBreak();
        }
    wchar_t *wc=_wcsdup (L"This is a _wcsdup leak from line " EVALL(__LINE__) );
    wc = NULL;

    // 2 alternate used/free
    free(strdup("This is a strdup leak from line " EVAL(__LINE__) ));
    strdup("This is a strdup leak from line " EVAL(__LINE__) );
    free(strdup("This is a strdup leak from line " EVAL(__LINE__) ));
    strdup("This is a strdup leak from line " EVAL(__LINE__) );
    }


void  test_cppNew_3GB()
    {
    const Int32 chunkSize = 500;
    const UInt32 lim =  840000;
    for (UInt32  i = 0; i < lim; i+= chunkSize)      // testing 2 small block heaps
        {
        void *p= NULL;
        if ( ! (i % 1000))
            printf ("%8d\r", i);
        p = malloc(chunkSize);
        if (!p)
            DebugBreak();
        free (p);
        }
    }

//Test that realloc properly handles non-pagalloced memory,
//and works on pagalloced memory
void test_realloc(void* oldMem)
    {
    //ought to complain: allocated with CRT
    realloc(oldMem, 20);
    //Ought to work
    void* p = malloc(10);
    //Successfully realloced
    if(realloc(p, 20))
        return;
    OutputDebugString("Realloc failed!");
    DebugBreak();
    }

//Test that free properly handles non-pagalloced memory
void test_freeCRT(void* oldMem)
    {
    //ought to complain: allocated with CRT
    free(oldMem);
    }

//Test that _msize properly handles non-pagalloced memory
//and works on pagalloced memory
void test_msize(void* oldMem)
    {
    //Ought to complain: allocated with CRT
    _msize(oldMem);
    //Ought to work
    if(_msize(malloc(10)) != 10)
        OutputDebugString("Msize returned wrong size!\r\n");
    return;
    }

void test_expand(void* oldMem)
    {
    //Ought to complain: allocated with CRT
    _expand(oldMem, 2);
    //Ought to work
    _expand(malloc(1),2);
    //Should not have enough memory
    void* x = malloc(1);
    void* y = _expand(malloc(1), 0xFFFFFFFF);
    if(!y)
        printf("_expand error code is %d. This is a test\r\n", errno);
    free(x);
    }

/** Test that pagalloc correctly saved the original functions and can
still make calls to them. This feature isn't heavily used right now
but adding tests can save future pain.

Testing note: It's a good idea to test this in a debugger with a human eye.
Bugs in the patching code can lead to a wild branch here, which the automated
test may not catch. */
void test_CRTOriginals()
    {
    /* First test that all the functions can be called without exploding */
    byte* CRTMem = (byte*)patchData->callCrtMalloc(10);
    for(Int32 i=0; i < 10; i++)
        {
        CRTMem[i]=1;
        }
    CRTMem = (byte*)patchData->callCrtRealloc(CRTMem, 20);
    for(Int32 i=10; i<20; i++)
        {
        CRTMem[i]=1;
        }
    Int32 memSize = patchData->callCrtMsize(CRTMem);
    if(memSize == 10)
        OutputDebugString("Realloc failed to change memory size!\r\n");
    else if(memSize != 20)
        OutputDebugString("Realloc left memory at wrong size!\r\n");

    byte* CallocMem = (byte*) patchData->callCrtCalloc(10, 2);
    //Check that the memory is all 0s
    for(Int32 i=0; i < 20; i++)
        if(CallocMem[i])
            OutputDebugString("Calloc did not initialize its data! \r\n");

    //Make sure new returns writeable memory.
    byte* NewMem = (byte*) patchData->callCrtOperatorNew(10);
    for(Int32 i=0; i<10; i++)
        NewMem[i] = 0;
    patchData->callCrtFree(CallocMem);
    patchData->callCrtFree(CRTMem);
    patchData->callCrtOperatorDelete(NewMem);

    char* dup = patchData->callCrtStrdup("This string has been duplicated");
    if(strcmp(dup, "This string has been duplicated"))
        {
        OutputDebugString("Strings not equal : \"This string has been duplicated\" and ");
        OutputDebugString(dup);
        OutputDebugString("\r\n");
        }
    OutputDebugString("Testing that Crt Mallocs are treated as non-pagalloced\r\n");
    void* CRTMem2 = patchData->callCrtMalloc(1);
    free(CRTMem2);
    free(dup);
    wchar_t* wide = L"This string is wide";
    wchar_t* wideDup = patchData->callCrtWcsdup(wide);
    if(wcscmp(wide, wideDup))
        OutputDebugString("Wcscmp did not produce identical strings\r\n");
    if(!patchData->callCrtExpand(patchData->callCrtMalloc(1), 100))
        {
        if(errno == 12)
            OutputDebugString("Expand didn't have enough space. Error, but not our fault\r\n");
        else if(errno)
            {
            OutputDebugString("Crt expand failed! Check command line for error number\r\n");
            printf("Error number: %d\r\n", errno);
            DebugBreak();
            }
        }
    free(wideDup);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mike.Stratoti   10/101
+---------------+---------------+---------------+---------------+---------------+------*/
Int32 main()
    {
    // hard-coded breakpoint here for debugging
    DebugBreak();
    Int32 success = 0;
    memutil_forceHighAddress(false);

    //Allocate some memory with CRT, see if pagalloc successfully detects this later.
    void* crtReallocMem = malloc(1);
    void* crtMsizeMem = malloc(1);
    void* crtFreeMem = malloc(1);
    void* crtExpandMem = malloc(1);
    _msize(crtFreeMem);
    success = paPatch_patchCRuntimeMemFuncs();
    printf (" %s\n", success ? "MSVCRT patched" : "***Error: MSVCRT not patched");

    printf (" Addresses:\n");
    printf ("   %-20s  %p\n", "main", main);
    printf ("   %-20s  %p\n", "test_doubleFree", test_doubleFree);
    printf ("   %-20s  %p\n", "test_strdup", test_strdup);
    printf (" CRT Addresses:\n");
    printf ("   %-20s  %p\n", "malloc", malloc);
    printf ("   %-20s  %p\n", "free", free);

    printf ("\nTests:\n");

    test_cppNew_3GB();

    test_cppNew_new ();
    test_cppNew_delete ();
    test_doubleFree();
    test_strdup ();

    printf (" All test complete.\r\n");
    pagalloc_dumpSince (0);


    OutputDebugString("---------------------\r\n");
    DebugBreak();
    pagalloc_setSinceBase (1);
    pagalloc_displayStatistics();


    test_checkered();
    pagalloc_setLeakDetectDisplayLimit(10,10);
    test_customConstructor();
    test_realloc(crtReallocMem);
    test_freeCRT(crtFreeMem);
    test_msize(crtMsizeMem);
    test_expand(crtExpandMem);
    OutputDebugString("Testing showFullCallStack\r\n");
    pagalloc_showFullCallStack();
    pagalloc_displayStatistics();

    test_heapWalk();
//Maintaining these on x64 is a lot of work for little payoff. Only those that are really needed (msize and free) are currently supported.
#ifdef _X86_
    test_CRTOriginals();
#endif
    pagalloc_detectLeaks ();
    DebugBreak();
    paPatch_unpatchCRuntimeMemFuncs();
    return 0;
    }
