/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "StackExaminerGtestHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                           GeorgeDulchinos      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
DWORD_PTR getStackBase
(
)
    {
    DWORD_PTR stackBase;

#if defined (_WIN64)
    PNT_TIB64 pTib = reinterpret_cast<PNT_TIB64>(NtCurrentTeb ());
    stackBase = (DWORD_PTR) pTib->StackBase;
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
    DWORD_PTR stackBase = getStackBase ();

    MEMORY_BASIC_INFORMATION mbi;
    memset (&mbi, 0, sizeof mbi);

    VirtualQuery (&mbi, &mbi, sizeof mbi);
    // DWORD_PTR availableStack = (DWORD_PTR)mbi.BaseAddress - (DWORD_PTR)mbi.AllocationBase;

    DWORD_PTR maxStack = stackBase - (DWORD_PTR) mbi.AllocationBase;

    // get the reserved area and find what our peak usage was
    VirtualQuery (mbi.AllocationBase, &mbi, sizeof mbi);
    DWORD_PTR peakStackUsage = maxStack - mbi.RegionSize;

    *pBase = stackBase;
    *pSize = maxStack;
    *pPeak = peakStackUsage;
    }

void reclaimUnusedStackPages
(
)
    {
    SYSTEM_INFO si;
    GetSystemInfo (&si);

    MEMORY_BASIC_INFORMATION mbi;
    memset (&mbi, 0, sizeof mbi);

    VirtualQuery (&mbi, &mbi, sizeof mbi);
    DWORD_PTR unusedStack = (DWORD_PTR) &mbi - (DWORD_PTR) mbi.AllocationBase;

    if (unusedStack > 32768)
        {
        // 32K safety margin so we don't decommit any stack pages near the current SP and we leave room for guard page
        VirtualFree (mbi.AllocationBase, unusedStack - 32768, MEM_DECOMMIT);

        // Make the 2 pages beyond the current stack guard pages (we only need one, but since we are using the address of a
        // local in this proc to get the approximate SP, we make 2 guard pages in case the locals straddle a page boundary)
        DWORD dwOldProtect;
        VirtualProtect ((LPBYTE) ((DWORD_PTR) &mbi - si.dwPageSize), 1, PAGE_GUARD | PAGE_READWRITE, &dwOldProtect);
        VirtualProtect ((LPBYTE) ((DWORD_PTR) &mbi - 2 * si.dwPageSize), 1, PAGE_GUARD | PAGE_READWRITE, &dwOldProtect);
        }
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
StackInfo::StackInfo (DWORD64 sz, DWORD64 peak, std::string const& name)
    {
    m_size = sz;
    m_peak = peak;
    m_testName = name;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
std::string StackInfo::ToString ()
    {
    std::stringstream s;
    s << "peak: " << m_peak << ", " << m_testName.c_str ();
    return std::string (s.str ().c_str ());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
StackExaminer::StackExaminer ()
    {
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void StackExaminer::OnTestStart (const ::testing::TestInfo& /* test_info */)
    {
    reclaimUnusedStackPages ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void StackExaminer::OnTestEnd (const ::testing::TestInfo& test_info)
    {
    DWORD_PTR   pBase = NULL;
    DWORD64       size;
    DWORD64       peak;

    getCurrentThreadStackInfo (&pBase, &size, &peak);

    std::string name = std::string (test_info.test_case_name ()) + "." + std::string (test_info.name ());


    StackInfo info (size, peak, name);
    m_stacktable.insert (std::make_pair (peak, info));

    reclaimUnusedStackPages ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void StackExaminer::DumpStackInfo ()
    {
    StackMap::iterator iter;
    for (iter = m_stacktable.begin (); iter != m_stacktable.end (); ++iter)
        {
        if (iter->second.m_peak > 35000)
            std::cout << "[ STACK RESULT ] " << iter->second.ToString ().c_str () << "\n";
        }
    }

