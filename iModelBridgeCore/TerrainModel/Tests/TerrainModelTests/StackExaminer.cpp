/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "CommonApi.h"



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                           GeorgeDulchinos      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
DWORD_PTR getStackBase
(
)
    {
    DWORD_PTR stackBase;

#if defined (_WIN64)
    PNT_TIB64 pTib = reinterpret_cast<PNT_TIB64>(NtCurrentTeb());
    stackBase = (DWORD_PTR)pTib->StackBase;
#else
    _asm    {
            mov EAX,FS:[4]
            mov stackBase,EAX
            }
#endif

    return stackBase;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                           GeorgeDulchinos      04/04
+---------------+---------------+---------------+---------------+---------------+------*/
void getCurrentThreadStackInfo
(
DWORD_PTR * pBase,   // <= base (highest) address value
DWORD64 *   pSize,   // <= max size of stack
DWORD64 *   pPeak    // <= peak usage of stack
)
    {
    DWORD_PTR stackBase = getStackBase();

    MEMORY_BASIC_INFORMATION mbi;
    memset(&mbi, 0, sizeof mbi);

    VirtualQuery(&mbi, &mbi, sizeof mbi);
    // DWORD_PTR availableStack = (DWORD_PTR)mbi.BaseAddress - (DWORD_PTR)mbi.AllocationBase;

    DWORD_PTR maxStack       = stackBase - (DWORD_PTR)mbi.AllocationBase;

    // get the reserved area and find what our peak usage was
    VirtualQuery(mbi.AllocationBase, &mbi, sizeof mbi);
    DWORD_PTR peakStackUsage = maxStack - mbi.RegionSize;

    *pBase = stackBase;
    *pSize = maxStack;
    *pPeak = peakStackUsage;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                           GeorgeDulchinos      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void reclaimUnusedStackPages
(
)
    {
    SYSTEM_INFO si;
    GetSystemInfo (&si);

    MEMORY_BASIC_INFORMATION mbi;
    memset(&mbi, 0, sizeof mbi);

    VirtualQuery(&mbi, &mbi, sizeof mbi);
    DWORD_PTR unusedStack = (DWORD_PTR)&mbi - (DWORD_PTR)mbi.AllocationBase;

    if (unusedStack > 32768)
        {
        // 32K safety margin so we don't decommit any stack pages near the current SP and we leave room for guard page
        VirtualFree (mbi.AllocationBase, unusedStack - 32768,  MEM_DECOMMIT);

        // Make the 2 pages beyond the current stack guard pages (we only need one, but since we are using the address of a
        // local in this proc to get the approximate SP, we make 2 guard pages in case the locals straddle a page boundary)
        DWORD dwOldProtect;
        VirtualProtect ( (LPBYTE)((DWORD_PTR)&mbi - si.dwPageSize),   1, PAGE_GUARD | PAGE_READWRITE, &dwOldProtect);
        VirtualProtect ( (LPBYTE)((DWORD_PTR)&mbi - 2*si.dwPageSize), 1, PAGE_GUARD | PAGE_READWRITE, &dwOldProtect);
        }
    }

