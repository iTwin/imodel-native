/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/nonport/winnt/gdileak.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <windows.h>
#include <map>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <TlHelp32.h>
#include "gdileak.h"

#define FARPROC_T void*

enum PagallocSeverity
    {
    PAGALLOC_SEVERITY_TRACE         = 0x00000001,
    PAGALLOC_SEVERITY_DEBUG         = 0x00000002,
    PAGALLOC_SEVERITY_INFORMATION   = 0x00000003,
    PAGALLOC_SEVERITY_WARNING       = 0x00000004,
    PAGALLOC_SEVERITY_ERROR         = 0x00000005
    };
void     pagalloc_printf (PagallocSeverity level, char const * const  szFormat, ...);

//-----------------------------------------------------------------------------------//
//              Windows Graphics Programming: Win32 GDI and DirectDraw               //
//                             ISBN  0-13-086985-6                                   //
//                                                                                   //
//  Written            by  Yuan, Feng                             www.fengyuan.com   //
//  Copyright (c) 2000 by  Hewlett-Packard Company                www.hp.com         //
//  Published          by  Prentice Hall PTR, Prentice-Hall, Inc. www.phptr.com      //
//                                                                                   //
//  Description: API hooking through import/export table                             //
//-----------------------------------------------------------------------------------//
class KPEFile
    {
    PIMAGE_DOS_HEADER   pDOSHeader;
    PIMAGE_NT_HEADERS   pNTHeader;    // will be PIMAGE_NT_HEADERS32 or PIMAGE_NT_HEADERS64
    BOOL                m_bPE;

public:
    const char* pModule;

    const char* RVA2Ptr(uintptr_t rva)
        {
        if ( (pModule!=NULL) && rva )
                return pModule + rva;
        else
                return NULL;
        }

    KPEFile(HMODULE hModule);
    BOOL IsPeFile() const{return m_bPE;};
    const void * GetDirectory(int id);
    PIMAGE_IMPORT_DESCRIPTOR GetImportDescriptor(LPCSTR pDllName);
    const uintptr_t* GetFunctionPtr(PIMAGE_IMPORT_DESCRIPTOR pImport, LPCSTR pProcName);

    FARPROC_T SetImportAddress( LPCSTR pDllName, LPCSTR pProcName, FARPROC_T pNewProc);
    FARPROC_T SetImportAddress (PIMAGE_IMPORT_DESCRIPTOR pImport, LPCSTR pProcName, FARPROC_T pNewProc);

    FARPROC_T SetExportAddress(LPCSTR pProcName, FARPROC_T pNewProc);
    };

KPEFile::KPEFile (HMODULE hModule)
    {
    m_bPE = FALSE;
    pModule = (const char *) hModule;

    if ( IsBadReadPtr(pModule, sizeof(IMAGE_DOS_HEADER)) )
        {
        pDOSHeader = NULL;
        pNTHeader  = NULL;
        }
    else
        {
        pDOSHeader = (PIMAGE_DOS_HEADER) pModule;
        if(pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
                return;

        if ( IsBadReadPtr(RVA2Ptr(pDOSHeader->e_lfanew), sizeof(IMAGE_NT_HEADERS)) )
            {
            pNTHeader = NULL;
            }
        else
            {
            pNTHeader = (PIMAGE_NT_HEADERS) RVA2Ptr(pDOSHeader-> e_lfanew);
            m_bPE = (pNTHeader->Signature == IMAGE_NT_SIGNATURE);
            }
        }
    }

// returns address of a PE directory
const void * KPEFile::GetDirectory (int id)
    {
    return RVA2Ptr(pNTHeader->OptionalHeader.DataDirectory[id].VirtualAddress);
    }

// returns PIMAGE_IMPORT_DESCRIPTOR for an imported module
PIMAGE_IMPORT_DESCRIPTOR KPEFile::GetImportDescriptor (LPCSTR pDllName)
    {
    // first IMAGE_IMPORT_DESCRIPTOR
    PIMAGE_IMPORT_DESCRIPTOR pImport = (PIMAGE_IMPORT_DESCRIPTOR)GetDirectory (IMAGE_DIRECTORY_ENTRY_IMPORT);

    if ( pImport==NULL )
            return NULL;

    while ( pImport->FirstThunk )
        {
        if (stricmp(pDllName, RVA2Ptr(pImport->Name))==0 )
            return pImport;

        // move to next imported module
        pImport ++;
        }

    return NULL;
    }

// returns address of __imp__xxx variable for an import function
const uintptr_t* KPEFile::GetFunctionPtr (PIMAGE_IMPORT_DESCRIPTOR pImport, LPCSTR pProcName)
    {
    PIMAGE_THUNK_DATA pThunk;

    pThunk = (PIMAGE_THUNK_DATA) RVA2Ptr(pImport->OriginalFirstThunk);

    for (int i=0; pThunk->u1.Function; i++)
        {
        bool match;

#if defined(_X86_)
        if ( pThunk->u1.Ordinal & 0x80000000 )  // by ordinal
            match = (pThunk->u1.Ordinal & 0xFFFF) == ((uintptr_t) pProcName);
#else
        if ( pThunk->u1.Ordinal & 0x800000000000000 )  // by ordinal
            match = (pThunk->u1.Ordinal & 0xFFFF) == ((uintptr_t) pProcName);  // Not sure what the constant should be
#endif        
        else
            {
            char const* currentName = RVA2Ptr((unsigned)pThunk->u1.AddressOfData)+2;
            match = (stricmp(pProcName, currentName) == 0);
            //match = (stricmp(pProcName, RVA2Ptr((unsigned)pThunk->u1.AddressOfData)+2) == 0);
            }

        if (match)
            return (uintptr_t *) RVA2Ptr(pImport->FirstThunk)+i;

        pThunk ++;
        }

    return NULL;
    }

bool HackWriteProcessMemory (HANDLE /*hProcess*/, void* pDest, void* pSource, size_t nSize, size_t* pWritten)
    {
    __try
        {
        MEMORY_BASIC_INFORMATION mbi;

        VirtualQuery(pDest, &mbi, sizeof(mbi));
        DWORD originalProtection = mbi.AllocationProtect;
        VirtualProtect (mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect);

        memcpy(pDest, pSource, nSize);
        VirtualProtect (mbi.BaseAddress, mbi.RegionSize, originalProtection, &mbi.Protect);
        }
    __except ( EXCEPTION_EXECUTE_HANDLER )
        {
        return false;
        }

    *pWritten = nSize;
    return true;
    }

FARPROC_T KPEFile::SetImportAddress (LPCSTR pDllName, LPCSTR pProcName, FARPROC_T pNewProc)
    {
    PIMAGE_IMPORT_DESCRIPTOR pImport = GetImportDescriptor(pDllName);
    if ( pImport )
        return SetImportAddress(pImport, pProcName, pNewProc);
    else
        return NULL;
    }

FARPROC_T KPEFile::SetImportAddress(PIMAGE_IMPORT_DESCRIPTOR pImport, LPCSTR pProcName, FARPROC_T pNewProc)
    {
    const uintptr_t * pfn = GetFunctionPtr(pImport, pProcName);

    if (IsBadReadPtr(pfn, sizeof(uintptr_t)))
        return NULL;
    
    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "GDILEAK::   patching function %s\n", pProcName);
/*
    char buffer[512];
    sprintf (buffer, "procName: %s\n", pProcName);
    OutputDebugString (buffer);
    sprintf (buffer, "  Ptr in DLL: %I64x\n", pfn);
    OutputDebugString (buffer);
    sprintf (buffer, "    oldFunction: %I64x\n", (void*) *pfn);
    OutputDebugString (buffer);
    sprintf (buffer, "    newFunction: %I64x\n", pNewProc);
    OutputDebugString (buffer);
*/
    // read the original function address
    void* oldproc = (void*) *pfn;

    size_t dwWritten=0;

    // overwrite with new function address
    HackWriteProcessMemory(GetCurrentProcess(), (void *) pfn, &pNewProc, sizeof(uintptr_t), &dwWritten);

/*
    const uintptr_t * newPfn = GetFunctionPtr(pImport, pProcName);
    sprintf (buffer, "After Write\n");
    OutputDebugString (buffer);
    sprintf (buffer, "  Ptr in DLL: %I64x\n", newPfn);
    OutputDebugString (buffer);
    sprintf (buffer, "    Current function: %I64x\n", (void*) *newPfn);
    OutputDebugString (buffer);
    sprintf (buffer, "    Return function: %I64x\n", oldproc);
    OutputDebugString (buffer);
*/
    return (FARPROC_T)oldproc;
    }


FARPROC_T KPEFile::SetExportAddress(LPCSTR pProcName, FARPROC_T pNewProc)
    {
    PIMAGE_EXPORT_DIRECTORY pExport = (PIMAGE_EXPORT_DIRECTORY)GetDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT);

    if ( pExport==NULL )
        return NULL;

    uintptr_t ord = 0;

    if ( (uintptr_t) pProcName < 0xFFFF ) // ordinal ?  // Not sure what the constant should be for x64
        {
        ord = (uintptr_t) pProcName;
        }
    else
        {
        const DWORD * pNames = (const DWORD *) RVA2Ptr(pExport->AddressOfNames);
        const WORD  * pOrds  = (const WORD  *) RVA2Ptr(pExport->AddressOfNameOrdinals);

        // find the entry with the function name
        for (unsigned i=0; i<pExport->AddressOfNames; i++)
            if ( stricmp(pProcName, RVA2Ptr(pNames[i]))==0 )
                {
                // get the corresponding ordinal
                ord = pExport->Base + pOrds[i];
                break;
                }
        }

    if ( (ord<pExport->Base) || (ord>(pExport->Base+pExport->NumberOfFunctions)) )
        return NULL;

    // use ordinal to get the address where export RVA is stored
    uintptr_t* pRVA = (uintptr_t *) RVA2Ptr(pExport->AddressOfFunctions) + ord - pExport->Base;

    // read original function address
    uintptr_t rslt = *pRVA;

    assert (((intptr_t)pNewProc - (intptr_t)pModule) > 0); // Make sure our DLL is above the current one so that the RVA will be a positive value.
    size_t dwWritten = 0;
    uintptr_t newRVA = (uintptr_t) pNewProc - (uintptr_t) pModule;
    HackWriteProcessMemory(GetCurrentProcess(), pRVA, &newRVA, sizeof(uintptr_t), &dwWritten);

    return (FARPROC_T) RVA2Ptr(rslt);
    }


#define DEFINE_GDI_API(Name)    CGdiMonitor::Name##Proc CGdiMonitor::__##Name = NULL;
// device context
DEFINE_GDI_API(CreateDCA)
DEFINE_GDI_API(CreateDCW)
DEFINE_GDI_API(CreateCompatibleDC)
DEFINE_GDI_API(CreateICA)
DEFINE_GDI_API(CreateICW)
DEFINE_GDI_API(GetDC)
DEFINE_GDI_API(GetDCEx)
DEFINE_GDI_API(GetWindowDC)
// pen
DEFINE_GDI_API(CreatePen)
DEFINE_GDI_API(CreatePenIndirect)
DEFINE_GDI_API(ExtCreatePen)
// brush API
DEFINE_GDI_API(CreateSolidBrush)
DEFINE_GDI_API(CreateHatchBrush)
DEFINE_GDI_API(CreateBrushIndirect)
DEFINE_GDI_API(CreatePatternBrush)
DEFINE_GDI_API(CreateDIBPatternBrush)
DEFINE_GDI_API(CreateDIBPatternBrushPt)
// bitmap API
DEFINE_GDI_API(LoadBitmapA)
DEFINE_GDI_API(LoadBitmapW)
DEFINE_GDI_API(CreateBitmap)
DEFINE_GDI_API(CreateBitmapIndirect)
DEFINE_GDI_API(CreateCompatibleBitmap)
DEFINE_GDI_API(CreateDIBitmap)
DEFINE_GDI_API(CreateDIBSection)
DEFINE_GDI_API(CreateDiscardableBitmap)
// font
DEFINE_GDI_API(CreateFontA)
DEFINE_GDI_API(CreateFontW)
DEFINE_GDI_API(CreateFontIndirectA)
DEFINE_GDI_API(CreateFontIndirectW)
DEFINE_GDI_API(CreateFontIndirectExA)
DEFINE_GDI_API(CreateFontIndirectExW)
// region
DEFINE_GDI_API(CreateRectRgn)
DEFINE_GDI_API(CreateRectRgnIndirect)
DEFINE_GDI_API(CreateEllipticRgn)
DEFINE_GDI_API(CreateEllipticRgnIndirect)
DEFINE_GDI_API(CreatePolygonRgn)
DEFINE_GDI_API(CreatePolyPolygonRgn)
DEFINE_GDI_API(CreateRoundRectRgn)
DEFINE_GDI_API(PathToRegion)
DEFINE_GDI_API(ExtCreateRegion)
DEFINE_GDI_API(CombineRgn)
// metafile dc(released by CloseMetaFile/CloseEnhMetaFile)
DEFINE_GDI_API(CreateMetaFileA)
DEFINE_GDI_API(CreateMetaFileW)
DEFINE_GDI_API(CreateEnhMetaFileA)
DEFINE_GDI_API(CreateEnhMetaFileW)
// metafile
DEFINE_GDI_API(GetEnhMetaFileA)
DEFINE_GDI_API(GetEnhMetaFileW)
DEFINE_GDI_API(GetMetaFileA)
DEFINE_GDI_API(GetMetaFileW)
// palette
DEFINE_GDI_API(CreateHalftonePalette)
DEFINE_GDI_API(CreatePalette)
// Icons
DEFINE_GDI_API(CopyIcon)
DEFINE_GDI_API(CreateIcon)
DEFINE_GDI_API(CreateIconFromResource)
DEFINE_GDI_API(CreateIconFromResourceEx)
DEFINE_GDI_API(CreateIconIndirect)
DEFINE_GDI_API(DestroyIcon)
DEFINE_GDI_API(DuplicateIcon)
DEFINE_GDI_API(ExtractAssociatedIconA)
DEFINE_GDI_API(ExtractAssociatedIconW)
DEFINE_GDI_API(ExtractAssociatedIconExA)
DEFINE_GDI_API(ExtractAssociatedIconExW)
DEFINE_GDI_API(ExtractIconA)
DEFINE_GDI_API(ExtractIconW)
//DEFINE_GDI_API(ExtractIconExA)
//DEFINE_GDI_API(ExtractIconExW)
DEFINE_GDI_API(LoadIconA)
DEFINE_GDI_API(LoadIconW)
//DEFINE_GDI_API(PrivateExtractIconsA)
//DEFINE_GDI_API(PrivateExtractIconsW)
// object deletion
DEFINE_GDI_API(DeleteObject)
DEFINE_GDI_API(DeleteDC)
DEFINE_GDI_API(DeleteMetaFile)
DEFINE_GDI_API(DeleteEnhMetaFile)
// object release
DEFINE_GDI_API(ReleaseDC)
//delete metafile dc and generate metafile
DEFINE_GDI_API(CloseMetaFile)
DEFINE_GDI_API(CloseEnhMetaFile)

//All the dlls have been sorted by alphabeta and capitalized.
const CHAR* g_SystemDlls[] = 
{
        "ACTIVEDS.DLL",
        "ADSLDPC.DLL",
        "ADVAPI32.DLL",
        "ADVPACK.DLL",
        "APPHELP.DLL",
        "ATL.DLL",
        "AUTHZ.DLL",
        "BROWSEUI.DLL",
        "CABINET.DLL",
        "CDFVIEW.DLL",
        "CERTCLI.DLL",
        "CFGMGR32.DLL",
        "CLUSAPI.DLL",
        "COMDLG32.DLL",
        "CREDUI.DLL",
        "CRYPT32.DLL",
        "CRYPTUI.DLL",
        "CSCDLL.DLL",
        "DBGHELP.DLL",
        "DEVMGR.DLL",
        "DHCPCSVC.DLL",
        "DNSAPI.DLL",
        "DUSER.DLL",
        "DWMAPI.DLL",
        "EFSADU.DLL",
        "ESENT.DLL",
        "GDI32.DLL",
        "HLINK.DLL",
        "HNETCFG.DLL",
        "IEFRAME.DLL",
        "IERTUTIL.DLL",
        "IEUI.DLL",
        "IMAGEHLP.DLL",
        "IMGUTIL.DLL",
        "IMM32.DLL",
        "INETCOMM.DLL",
        "IPHLPAPI.DLL",
        "KERNEL32.DLL",
        "LINKINFO.DLL",
        "LZ32.DLL",
        "MLANG.DLL",
        "MOBSYNC.DLL",
        "MPR.DLL",
        "MPRAPI.DLL",
        "MPRUI.DLL",
        "MSASN1.DLL",
        //Microsoft Input Method
        "MSCTFIME.IME",
        "MSGINA.DLL",
        "MSHTML.DLL",
        "MSI.DLL",
        "MSIMG32.DLL",
        "MSLS31.DLL",
        "MSOERT2.DLL",
        "MSRATING.DLL",
        "MSSIGN32.DLL",
        "MSVCP60.DLL",
        "MSVCRT.DLL",
        "MSWSOCK.DLL",
        "NETAPI32.DLL",
        "NETCFGX.DLL",
        "NETMAN.DLL",
        "NETPLWIZ.DLL",
        "NETRAP.DLL",
        "NETSHELL.DLL",
        "NETUI0.DLL",
        "NETUI1.DLL",
        "NETUI2.DLL",
        "NORMALIZ.DLL",
        "NTDLL.DLL",
        "NTDSAPI.DLL",
        "NTLANMAN.DLL",
        "ODBC32.DLL",
        "OLE32.DLL",
        "OLEACC.DLL",
        "OLEAUT32.DLL",
        "OLEDLG.DLL",
        "OLEPRO32.DLL",
        "POWRPROF.DLL",
        "PRINTUI.DLL",
        "PSAPI.DLL",
        "QUERY.DLL",
        "RASAPI32.DLL",
        "RASDLG.DLL",
        "RASMAN.DLL",
        "REGAPI.DLL",
        "RPCRT4.DLL",
        "RTUTILS.DLL",
        "SAMLIB.DLL",
        "SCECLI.DLL",
        "SECUR32.DLL",
        "SETUPAPI.DLL",
        "SHDOCVW.DLL",
        "SHELL32.DLL",
        "SHLWAPI.DLL",
        "SHSVCS.DLL",
        "TAPI32.DLL",
        "URLMON.DLL",
        "USER32.DLL",
        "USERENV.DLL",
        "USP10.DLL",
        "UTILDLL.DLL",
        "UXTHEME.DLL",
        "VERSION.DLL",
        "W32TOPL.DLL",
        "WINHTTP.DLL",
        "WININET.DLL",
        "WINMM.DLL",
        "WINSCARD.DLL",
        "WINSPOOL.DRV",
        "WINSTA.DLL",
        "WINTRUST.DLL",
        "WLDAP32.DLL",
        "WMI.DLL",
        "WS2_32.DLL",
        "WS2HELP.DLL",
        "WSOCK32.DLL",
        "WTSAPI32.DLL",
        "WZCDLG.DLL",
        "WZCSAPI.DLL",
        "WZCSVC.DLL"
};

bool Str1LessStr2(const CHAR* str1, const CHAR* str2)
    {
    return strcmp(str1, str2) < 0;
    }

bool IsSystemDll(CHAR* dllName)
    {
    strupr (dllName);
    const CHAR** pIndex = std::lower_bound(&g_SystemDlls[0], &g_SystemDlls[0]+_countof(g_SystemDlls), dllName, Str1LessStr2);
    if(pIndex != &g_SystemDlls[0]+_countof(g_SystemDlls))
        return strcmp(*pIndex, dllName) == 0;
    return FALSE;
    }


CGdiMonitor::CGdiMonitor(void)
    {
    HMODULE hGDI32 = GetModuleHandleW(L"GDI32.DLL");
    HMODULE hUSER32 = GetModuleHandleW(L"USER32.DLL");
    // device context
    __CreateDCA = (CreateDCAProc)GetProcAddress(hGDI32, "CreateDCA");
    __CreateDCW = (CreateDCWProc)GetProcAddress(hGDI32, "CreateDCW");
    __CreateCompatibleDC = (CreateCompatibleDCProc)GetProcAddress(hGDI32, "CreateCompatibleDC");
    __CreateICA = (CreateICAProc)GetProcAddress(hGDI32, "CreateICA");
    __CreateICW = (CreateICWProc)GetProcAddress(hGDI32, "CreateICW");
    __GetDC = (GetDCProc)GetProcAddress(hUSER32, "GetDC");
    __GetDCEx = (GetDCExProc)GetProcAddress(hUSER32, "GetDCEx");
    __GetWindowDC = (GetWindowDCProc)GetProcAddress(hUSER32, "GetWindowDC");
    // pen
    __CreatePen = (CreatePenProc)GetProcAddress(hGDI32, "CreatePen");
    __CreatePenIndirect = (CreatePenIndirectProc)GetProcAddress(hGDI32, "CreatePenIndirect");
    __ExtCreatePen = (ExtCreatePenProc)GetProcAddress(hGDI32, "ExtCreatePen");
    // brush API
    __CreateSolidBrush = (CreateSolidBrushProc)GetProcAddress(hGDI32, "CreateSolidBrush");
    __CreateHatchBrush = (CreateHatchBrushProc)GetProcAddress(hGDI32, "CreateHatchBrush");
    __CreateBrushIndirect = (CreateBrushIndirectProc)GetProcAddress(hGDI32, "CreateBrushIndirect");
    __CreatePatternBrush = (CreatePatternBrushProc)GetProcAddress(hGDI32, "CreatePatternBrush");
    __CreateDIBPatternBrush = (CreateDIBPatternBrushProc)GetProcAddress(hGDI32, "CreateDIBPatternBrush");
    __CreateDIBPatternBrushPt = (CreateDIBPatternBrushPtProc)GetProcAddress(hGDI32, "CreateDIBPatternBrushPt");
    // bitmap API
    __LoadBitmapA = (LoadBitmapAProc)GetProcAddress(hUSER32, "LoadBitmapA");
    __LoadBitmapW = (LoadBitmapWProc)GetProcAddress(hUSER32, "LoadBitmapW");
    __CreateBitmap = (CreateBitmapProc)GetProcAddress(hGDI32, "CreateBitmap");
    __CreateBitmapIndirect = (CreateBitmapIndirectProc)GetProcAddress(hGDI32, "CreateBitmapIndirect");
    __CreateCompatibleBitmap = (CreateCompatibleBitmapProc)GetProcAddress(hGDI32, "CreateCompatibleBitmap");
    __CreateDIBitmap = (CreateDIBitmapProc)GetProcAddress(hGDI32, "CreateDIBitmap");
    __CreateDIBSection = (CreateDIBSectionProc)GetProcAddress(hGDI32, "CreateDIBSection");
    __CreateDiscardableBitmap = (CreateDiscardableBitmapProc)GetProcAddress(hGDI32, "CreateDiscardableBitmap");
    // font
    __CreateFontA = (CreateFontAProc)GetProcAddress(hGDI32, "CreateFontA");
    __CreateFontW = (CreateFontWProc)GetProcAddress(hGDI32, "CreateFontW");
    __CreateFontIndirectA = (CreateFontIndirectAProc)GetProcAddress(hGDI32, "CreateFontIndirectA");
    __CreateFontIndirectW = (CreateFontIndirectWProc)GetProcAddress(hGDI32, "CreateFontIndirectW");
    __CreateFontIndirectExA = (CreateFontIndirectExAProc)GetProcAddress(hGDI32, "CreateFontIndirectExA");
    __CreateFontIndirectExW = (CreateFontIndirectExWProc)GetProcAddress(hGDI32, "CreateFontIndirectExW");
    // region
    __CreateRectRgn = (CreateRectRgnProc)GetProcAddress(hGDI32, "CreateRectRgn");
    __CreateRectRgnIndirect = (CreateRectRgnIndirectProc)GetProcAddress(hGDI32, "CreateRectRgnIndirect");
    __CreateEllipticRgn = (CreateEllipticRgnProc)GetProcAddress(hGDI32, "CreateEllipticRgn");
    __CreateEllipticRgnIndirect = (CreateEllipticRgnIndirectProc)GetProcAddress(hGDI32, "CreateEllipticRgnIndirect");
    __CreatePolygonRgn = (CreatePolygonRgnProc)GetProcAddress(hGDI32, "CreatePolygonRgn");
    __CreatePolyPolygonRgn = (CreatePolyPolygonRgnProc)GetProcAddress(hGDI32, "CreatePolyPolygonRgn");
    __CreateRoundRectRgn = (CreateRoundRectRgnProc)GetProcAddress(hGDI32, "CreateRoundRectRgn");
    __PathToRegion = (PathToRegionProc)GetProcAddress(hGDI32, "PathToRegion");
    __ExtCreateRegion = (ExtCreateRegionProc)GetProcAddress(hGDI32, "ExtCreateRegion");
    __CombineRgn = (CombineRgnProc)GetProcAddress(hGDI32, "CombineRgn");

    // metafile dc(released by CloseMetaFile/CloseEnhMetaFile)
    __CreateMetaFileA = (CreateMetaFileAProc)GetProcAddress(hGDI32, "CreateMetaFileA");
    __CreateMetaFileW = (CreateMetaFileWProc)GetProcAddress(hGDI32, "CreateMetaFileW");
    __CreateEnhMetaFileA = (CreateEnhMetaFileAProc)GetProcAddress(hGDI32, "CreateEnhMetaFileA");
    __CreateEnhMetaFileW = (CreateEnhMetaFileWProc)GetProcAddress(hGDI32, "CreateEnhMetaFileW");
    __GetEnhMetaFileA = (GetEnhMetaFileAProc)GetProcAddress(hGDI32, "GetEnhMetaFileA");
    __GetEnhMetaFileW = (GetEnhMetaFileWProc)GetProcAddress(hGDI32, "GetEnhMetaFileW");
    __GetMetaFileA = (GetMetaFileAProc)GetProcAddress(hGDI32, "GetMetaFileA");
    __GetMetaFileW = (GetMetaFileWProc)GetProcAddress(hGDI32, "GetMetaFileW");
    // palette
    __CreateHalftonePalette = (CreateHalftonePaletteProc)GetProcAddress(hGDI32, "CreateHalftonePalette");
    __CreatePalette = (CreatePaletteProc)GetProcAddress(hGDI32, "CreatePalette");
    // icons
    __CopyIcon = (CopyIconProc)GetProcAddress(hUSER32, "CopyIcon");
    __CreateIcon = (CreateIconProc)GetProcAddress(hUSER32, "CreateIcon");
    __CreateIconFromResource = (CreateIconFromResourceProc)GetProcAddress(hUSER32, "CreateIconFromResource");
    __CreateIconFromResourceEx = (CreateIconFromResourceExProc)GetProcAddress(hUSER32, "CreateIconFromResourceEx");
    __CreateIconIndirect = (CreateIconIndirectProc)GetProcAddress(hUSER32, "CreateIconIndirect");
    __DestroyIcon = (DestroyIconProc)GetProcAddress(hUSER32, "DestroyIcon");
    __DuplicateIcon = (DuplicateIconProc)GetProcAddress(hUSER32, "DuplicateIcon");
    __ExtractAssociatedIconA = (ExtractAssociatedIconAProc)GetProcAddress(hUSER32, "ExtractAssociatedIconA");
    __ExtractAssociatedIconW = (ExtractAssociatedIconWProc)GetProcAddress(hUSER32, "ExtractAssociatedIconW");
    __ExtractAssociatedIconExA = (ExtractAssociatedIconExAProc)GetProcAddress(hUSER32, "ExtractAssociatedIconExA");
    __ExtractAssociatedIconExW = (ExtractAssociatedIconExWProc)GetProcAddress(hUSER32, "ExtractAssociatedIconExW");
    __ExtractIconA = (ExtractIconAProc)GetProcAddress(hUSER32, "ExtractIconA");
    __ExtractIconW = (ExtractIconWProc)GetProcAddress(hUSER32, "ExtractIconW");
//    __ExtractIconExA = (ExtractIconExAProc)GetProcAddress(hUSER32, "ExtractIconExA");
//    __ExtractIconExW = (ExtractIconExWProc)GetProcAddress(hUSER32, "ExtractIconExW");
    __LoadIconA = (LoadIconAProc)GetProcAddress(hUSER32, "LoadIconA");
    __LoadIconW = (LoadIconWProc)GetProcAddress(hUSER32, "LoadIconW");
//    __PrivateExtractIconsA = (PrivateExtractIconsAProc)GetProcAddress(hUSER32, "PrivateExtractIconsA");
//    __PrivateExtractIconsW = (PrivateExtractIconsWProc)GetProcAddress(hUSER32, "PrivateExtractIconsW");
    // object deletion
    __DeleteObject = (DeleteObjectProc)GetProcAddress(hGDI32, "DeleteObject");
    __DeleteDC = (DeleteDCProc)GetProcAddress(hGDI32, "DeleteDC");
    __DeleteMetaFile = (DeleteMetaFileProc)GetProcAddress(hGDI32, "DeleteMetaFile");
    __DeleteEnhMetaFile = (DeleteEnhMetaFileProc)GetProcAddress(hGDI32, "DeleteEnhMetaFile");
    // object release
    __ReleaseDC = (ReleaseDCProc)GetProcAddress(hUSER32, "ReleaseDC");
    //delete metafile dc and generate metafile
    __CloseMetaFile = (CloseMetaFileProc)GetProcAddress(hGDI32, "CloseMetaFile");
    __CloseEnhMetaFile = (CloseEnhMetaFileProc)GetProcAddress(hGDI32, "CloseEnhMetaFile");

    HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32;

    DWORD ProcessID = GetCurrentProcessId();
    // Take a snapshot of all modules in the specified process.
    hModuleSnap = CreateToolhelp32Snapshot (TH32CS_SNAPMODULE, ProcessID);
    if (INVALID_HANDLE_VALUE == hModuleSnap)
        {
        return;
        }

    // Set the size of the structure before using it.
    me32.dwSize = sizeof (MODULEENTRY32);

    // Retrieve information about the first module,
    // and exit if unsuccessful
    if( !Module32First( hModuleSnap, &me32 ) )
        {
        CloseHandle (hModuleSnap);     // Must clean up the snapshot object!
        return;
        }

    // Now walk the module list of the process,
    do
        {
        if(IsSystemDll(me32.szModule))
            {
            pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "GDILEAK:: skipping system module %s\n", me32.szModule);
            continue;
            }
        pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "GDILEAK:: patching module %s\n", me32.szModule);

        //KPEFile can be found in the book "Windows Graphics Programming" (author: Feng Yuan). The
        //class can modify the addresses of imported functions and exported functions.
        //The parameter of constructor is the base address of a module.
        KPEFile module((HMODULE)me32.modBaseAddr);
        if(!module.IsPeFile())
            continue;
        __try
            {
            PIMAGE_IMPORT_DESCRIPTOR pImport1 = module.GetImportDescriptor("GDI32.dll");
            if(pImport1)
                {
                // device context
                module.SetImportAddress(pImport1, "CreateDCA", (FARPROC_T)_CreateDCA);
                module.SetImportAddress(pImport1, "CreateDCW", (FARPROC_T)_CreateDCW);
                module.SetImportAddress(pImport1, "CreateCompatibleDC", (FARPROC_T)_CreateCompatibleDC);
                module.SetImportAddress(pImport1, "CreateICA", (FARPROC_T)_CreateICA);
                module.SetImportAddress(pImport1, "CreateICW", (FARPROC_T)_CreateICW);
                // pen
                module.SetImportAddress(pImport1, "CreatePen", (FARPROC_T)_CreatePen);
                module.SetImportAddress(pImport1, "CreatePenIndirect", (FARPROC_T)_CreatePenIndirect);
                module.SetImportAddress(pImport1, "ExtCreatePen", (FARPROC_T)_ExtCreatePen);
                // brush API
                module.SetImportAddress(pImport1, "CreateSolidBrush", (FARPROC_T)_CreateSolidBrush);
                module.SetImportAddress(pImport1, "CreateHatchBrush", (FARPROC_T)_CreateHatchBrush);
                module.SetImportAddress(pImport1, "CreateBrushIndirect", (FARPROC_T)_CreateBrushIndirect);
                module.SetImportAddress(pImport1, "CreatePatternBrush", (FARPROC_T)_CreatePatternBrush);
                module.SetImportAddress(pImport1, "CreateDIBPatternBrush", (FARPROC_T)_CreateDIBPatternBrush);
                module.SetImportAddress(pImport1, "CreateDIBPatternBrushPt", (FARPROC_T)_CreateDIBPatternBrushPt);
                // bitmap API
                module.SetImportAddress(pImport1, "CreateBitmap", (FARPROC_T)_CreateBitmap);
                module.SetImportAddress(pImport1, "CreateBitmapIndirect", (FARPROC_T)_CreateBitmapIndirect);
                module.SetImportAddress(pImport1, "CreateCompatibleBitmap", (FARPROC_T)_CreateCompatibleBitmap);
                module.SetImportAddress(pImport1, "CreateDIBitmap", (FARPROC_T)_CreateDIBitmap);
                module.SetImportAddress(pImport1, "CreateDIBSection", (FARPROC_T)_CreateDIBSection);
                module.SetImportAddress(pImport1, "CreateDiscardableBitmap", (FARPROC_T)_CreateDiscardableBitmap);
                // font
                module.SetImportAddress(pImport1, "CreateFontA", (FARPROC_T)_CreateFontA);
                module.SetImportAddress(pImport1, "CreateFontW", (FARPROC_T)_CreateFontW);
                module.SetImportAddress(pImport1, "CreateFontIndirectA", (FARPROC_T)_CreateFontIndirectA);
                module.SetImportAddress(pImport1, "CreateFontIndirectW", (FARPROC_T)_CreateFontIndirectW);
                module.SetImportAddress(pImport1, "CreateFontIndirectExA", (FARPROC_T)_CreateFontIndirectExA);
                module.SetImportAddress(pImport1, "CreateFontIndirectExW", (FARPROC_T)_CreateFontIndirectExW);
                // region
                module.SetImportAddress(pImport1, "CreateRectRgn", (FARPROC_T)_CreateRectRgn);
                module.SetImportAddress(pImport1, "CreateRectRgnIndirect", (FARPROC_T)_CreateRectRgnIndirect);
                module.SetImportAddress(pImport1, "CreateEllipticRgn", (FARPROC_T)_CreateEllipticRgn);
                module.SetImportAddress(pImport1, "CreateEllipticRgnIndirect", (FARPROC_T)_CreateEllipticRgnIndirect);
                module.SetImportAddress(pImport1, "CreatePolygonRgn", (FARPROC_T)_CreatePolygonRgn);
                module.SetImportAddress(pImport1, "CreatePolyPolygonRgn", (FARPROC_T)_CreatePolyPolygonRgn);
                module.SetImportAddress(pImport1, "CreateRoundRectRgn", (FARPROC_T)_CreateRoundRectRgn);
                module.SetImportAddress(pImport1, "PathToRegion", (FARPROC_T)_PathToRegion);
                module.SetImportAddress(pImport1, "ExtCreateRegion", (FARPROC_T)_ExtCreateRegion);
                module.SetImportAddress(pImport1, "CombineRgn", (FARPROC_T)_CombineRgn);
                // metafile dc(released by CloseMetaFile/CloseEnhMetaFile)
                module.SetImportAddress(pImport1, "CreateMetaFileA", (FARPROC_T)_CreateMetaFileA);
                module.SetImportAddress(pImport1, "CreateMetaFileW", (FARPROC_T)_CreateMetaFileW);
                module.SetImportAddress(pImport1, "CreateEnhMetaFileA", (FARPROC_T)_CreateEnhMetaFileA);
                module.SetImportAddress(pImport1, "CreateEnhMetaFileW", (FARPROC_T)_CreateEnhMetaFileW);
                // metafile
                module.SetImportAddress(pImport1, "GetEnhMetaFileA", (FARPROC_T)_GetEnhMetaFileA);
                module.SetImportAddress(pImport1, "GetEnhMetaFileW", (FARPROC_T)_GetEnhMetaFileW);
                module.SetImportAddress(pImport1, "GetMetaFileA", (FARPROC_T)_GetMetaFileA);
                module.SetImportAddress(pImport1, "GetMetaFileW", (FARPROC_T)_GetMetaFileW);
                // palette
                module.SetImportAddress(pImport1, "CreateHalftonePalette", (FARPROC_T)_CreateHalftonePalette);
                module.SetImportAddress(pImport1, "CreatePalette", (FARPROC_T)_CreatePalette);
                // object deletion
                module.SetImportAddress(pImport1, "DeleteObject", (FARPROC_T)_DeleteObject);
                module.SetImportAddress(pImport1, "DeleteDC", (FARPROC_T)_DeleteDC);
                module.SetImportAddress(pImport1, "DeleteMetaFile", (FARPROC_T)_DeleteMetaFile);
                module.SetImportAddress(pImport1, "DeleteEnhMetaFile", (FARPROC_T)_DeleteEnhMetaFile);
                //delete metafile dc and generate metafile
                module.SetImportAddress(pImport1, "CloseMetaFile", (FARPROC_T)_CloseMetaFile);
                module.SetImportAddress(pImport1, "CloseEnhMetaFile", (FARPROC_T)_CloseEnhMetaFile);

/* From what this fellow found (http://www.codeproject.com/Articles/175591/LeakMon-Part-2-Under-the-hood.aspx) 
   these seem to be missing functions.  Note that there are also a few implemented here that are not listed there.
   
LoadImageA
LoadImageW
CopyImage

CreateCursor    (cursor funcs in user32)
DestroyCursor
LoadCursorA
LoadCursorW
LoadCursorFromFileA
LoadCursorFromFileW


CopyEnhMetaFileA
CopyEnhMetaFileW   
*/

                }

            PIMAGE_IMPORT_DESCRIPTOR pImport2 = module.GetImportDescriptor("USER32.dll");
            if(pImport2)
                {
                module.SetImportAddress(pImport2, "GetDC", (FARPROC_T)_GetDC);
                module.SetImportAddress(pImport2, "GetDCEx", (FARPROC_T)_GetDCEx);
                module.SetImportAddress(pImport2, "GetWindowDC", (FARPROC_T)_GetWindowDC);
                module.SetImportAddress(pImport2, "LoadBitmapA", (FARPROC_T)_LoadBitmapA);
                module.SetImportAddress(pImport2, "LoadBitmapW", (FARPROC_T)_LoadBitmapW);
                module.SetImportAddress(pImport2, "ReleaseDC", (FARPROC_T)_ReleaseDC);
                // icons
                module.SetImportAddress(pImport2, "CopyIcon", (FARPROC_T)_CopyIcon);
                module.SetImportAddress(pImport2, "CreateIcon", (FARPROC_T)_CreateIcon);
                module.SetImportAddress(pImport2, "CreateIconFromResource", (FARPROC_T)_CreateIconFromResource);
                module.SetImportAddress(pImport2, "CreateIconFromResourceEx", (FARPROC_T)_CreateIconFromResourceEx);
                module.SetImportAddress(pImport2, "CreateIconIndirect", (FARPROC_T)_CreateIconIndirect);
                module.SetImportAddress(pImport2, "DestroyIcon", (FARPROC_T)_DestroyIcon);
                module.SetImportAddress(pImport2, "DuplicateIcon", (FARPROC_T)_DuplicateIcon);
                module.SetImportAddress(pImport2, "ExtractAssociatedIconA", (FARPROC_T)_ExtractAssociatedIconA);
                module.SetImportAddress(pImport2, "ExtractAssociatedIconW", (FARPROC_T)_ExtractAssociatedIconW);
                module.SetImportAddress(pImport2, "ExtractAssociatedIconExA", (FARPROC_T)_ExtractAssociatedIconExA);
                module.SetImportAddress(pImport2, "ExtractAssociatedIconExW", (FARPROC_T)_ExtractAssociatedIconExW);
                module.SetImportAddress(pImport2, "ExtractIconA", (FARPROC_T)_ExtractIconA);
                module.SetImportAddress(pImport2, "ExtractIconW", (FARPROC_T)_ExtractIconW);
//                module.SetImportAddress(pImport2, "ExtractIconExA", (FARPROC_T)_ExtractIconExA);
//                module.SetImportAddress(pImport2, "ExtractIconExW", (FARPROC_T)_ExtractIconExW);
                module.SetImportAddress(pImport2, "LoadIconA", (FARPROC_T)_LoadIconA);
                module.SetImportAddress(pImport2, "LoadIconW", (FARPROC_T)_LoadIconW);
//                module.SetImportAddress(pImport2, "PrivateExtractIconsA", (FARPROC_T)_PrivateExtractIconsA);
//                module.SetImportAddress(pImport2, "PrivateExtractIconsW", (FARPROC_T)_PrivateExtractIconsW);
                }
            }
        __except(EXCEPTION_EXECUTE_HANDLER)
            {
            }
    } while (Module32Next( hModuleSnap, &me32 ));

    // Don't forget to clean up the snapshot object.
    CloseHandle( hModuleSnap );

    //for those DLLs which are loaded after GdiSpy.dll was loaded, we must modify addresses of
    //the exported functions of GDI32.dll to hook the functions in the DLLs.
#if defined (CANNOT_SET_EXPORT)
    // This doesn't work in the world of DLL random base addresses.  Because it's an RVA, the
    //   patching DLL has to be in memory above GDI32.dll and USER32.dll.  This seeems to be hard to do.
    //   When I set the base address and set it to fixed the DLL would just refuse to load.
    //   So my current plan is just to patch the imports later, such that almost everything
    //   is patched and hope for the best.  Odds are in my favor.
    KPEFile module(hGDI32);
    // device context
    module.SetExportAddress("CreateDCA", (FARPROC)_CreateDCA);
    module.SetExportAddress("CreateDCW", (FARPROC)_CreateDCW);
    module.SetExportAddress("CreateCompatibleDC", (FARPROC)_CreateCompatibleDC);
    module.SetExportAddress("CreateICA", (FARPROC)_CreateICA);
    module.SetExportAddress("CreateICW", (FARPROC)_CreateICW);
    // pen
    module.SetExportAddress("CreatePen", (FARPROC)_CreatePen);
    module.SetExportAddress("CreatePenIndirect", (FARPROC)_CreatePenIndirect);
    module.SetExportAddress("ExtCreatePen", (FARPROC)_ExtCreatePen);
    // brush API
    module.SetExportAddress("CreateSolidBrush", (FARPROC)_CreateSolidBrush);
    module.SetExportAddress("CreateHatchBrush", (FARPROC)_CreateHatchBrush);
    module.SetExportAddress("CreateBrushIndirect", (FARPROC)_CreateBrushIndirect);
    module.SetExportAddress("CreatePatternBrush", (FARPROC)_CreatePatternBrush);
    module.SetExportAddress("CreateDIBPatternBrush", (FARPROC)_CreateDIBPatternBrush);
    module.SetExportAddress("CreateDIBPatternBrushPt", (FARPROC)_CreateDIBPatternBrushPt);
    // bitmap API
    module.SetExportAddress("CreateBitmap", (FARPROC)_CreateBitmap);
    module.SetExportAddress("CreateBitmapIndirect", (FARPROC)_CreateBitmapIndirect);
    module.SetExportAddress("CreateCompatibleBitmap", (FARPROC)_CreateCompatibleBitmap);
    module.SetExportAddress("CreateDIBitmap", (FARPROC)_CreateDIBitmap);
    module.SetExportAddress("CreateDIBSection", (FARPROC)_CreateDIBSection);
    module.SetExportAddress("CreateDiscardableBitmap", (FARPROC)_CreateDiscardableBitmap);
    // font
    module.SetExportAddress("CreateFontA", (FARPROC)_CreateFontA);
    module.SetExportAddress("CreateFontW", (FARPROC)_CreateFontW);
    module.SetExportAddress("CreateFontIndirectA", (FARPROC)_CreateFontIndirectA);
    module.SetExportAddress("CreateFontIndirectW", (FARPROC)_CreateFontIndirectW);
    module.SetExportAddress("CreateFontIndirectExA", (FARPROC)_CreateFontIndirectExA);
    module.SetExportAddress("CreateFontIndirectExW", (FARPROC)_CreateFontIndirectExW);
    // region
    module.SetExportAddress("CreateRectRgn", (FARPROC)_CreateRectRgn);
    module.SetExportAddress("CreateRectRgnIndirect", (FARPROC)_CreateRectRgnIndirect);
    module.SetExportAddress("CreateEllipticRgn", (FARPROC)_CreateEllipticRgn);
    module.SetExportAddress("CreateEllipticRgnIndirect", (FARPROC)_CreateEllipticRgnIndirect);
    module.SetExportAddress("CreatePolygonRgn", (FARPROC)_CreatePolygonRgn);
    module.SetExportAddress("CreatePolyPolygonRgn", (FARPROC)_CreatePolyPolygonRgn);
    module.SetExportAddress("CreateRoundRectRgn", (FARPROC)_CreateRoundRectRgn);
    module.SetExportAddress("PathToRegion", (FARPROC)_PathToRegion);
    module.SetExportAddress("ExtCreateRegion", (FARPROC)_ExtCreateRegion);
    module.SetExportAddress("CombineRgn", (FARPROC)_CombineRgn);
    // metafile dc(released by CloseMetaFile/CloseEnhMetaFile)
    module.SetExportAddress("CreateMetaFileA", (FARPROC)_CreateMetaFileA);
    module.SetExportAddress("CreateMetaFileW", (FARPROC)_CreateMetaFileW);
    module.SetExportAddress("CreateEnhMetaFileA", (FARPROC)_CreateEnhMetaFileA);
    module.SetExportAddress("CreateEnhMetaFileW", (FARPROC)_CreateEnhMetaFileW);
    // metafile
    module.SetExportAddress("GetEnhMetaFileA", (FARPROC)_GetEnhMetaFileA);
    module.SetExportAddress("GetEnhMetaFileW", (FARPROC)_GetEnhMetaFileW);
    module.SetExportAddress("GetMetaFileA", (FARPROC)_GetMetaFileA);
    module.SetExportAddress("GetMetaFileW", (FARPROC)_GetMetaFileW);
    // palette
    module.SetExportAddress("CreateHalftonePalette", (FARPROC)_CreateHalftonePalette);
    module.SetExportAddress("CreatePalette", (FARPROC)_CreatePalette);
    // object deletion
    module.SetExportAddress("DeleteObject", (FARPROC)_DeleteObject);
    module.SetExportAddress("DeleteDC", (FARPROC)_DeleteDC);
    module.SetExportAddress("DeleteMetaFile", (FARPROC)_DeleteMetaFile);
    module.SetExportAddress("DeleteEnhMetaFile", (FARPROC)_DeleteEnhMetaFile);
    //delete metafile dc and generate metafile
    module.SetExportAddress("CloseMetaFile", (FARPROC)_CloseMetaFile);
    module.SetExportAddress("CloseEnhMetaFile", (FARPROC)_CloseEnhMetaFile);

    KPEFile user32module(hUSER32);
    user32module.SetExportAddress("GetDC", (FARPROC)_GetDC);
    user32module.SetExportAddress("GetDCEx", (FARPROC)_GetDCEx);
    user32module.SetExportAddress("GetWindowDC", (FARPROC)_GetWindowDC);
    user32module.SetExportAddress("LoadBitmapA", (FARPROC)_LoadBitmapA);
    user32module.SetExportAddress("LoadBitmapW", (FARPROC)_LoadBitmapW);
    user32module.SetExportAddress("ReleaseDC", (FARPROC)_ReleaseDC);
#endif
    }

CGdiMonitor::~CGdiMonitor(void)
    {
    }

void     pagalloc_symbolizeHeaderAddresses (const PageMallocEntry * const headerP);

static CGdiMonitor* s_gdiMonitor = NULL;

static uint32_t s_recordNumber = 0;
static uint32_t s_initialNumberOfHandles = 0;
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Chuck.Kirschman   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void AddEntry (uintptr_t handleValue, void* returnAddress)
    {
    DWORD totalGdiCount = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS); 
    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "GDILEAK::Adding handle %lx count %d total GDI count %d (total %d)\n", 
                            handleValue, g_HandleInfo.size(), totalGdiCount-s_initialNumberOfHandles, totalGdiCount);
    if(handleValue != NULL)
        {
        PageMallocEntry *entry = new PageMallocEntry;
        memset (entry, 0, sizeof *entry);
        entry->mallocSerialNumber = s_recordNumber++;
        pagallocI_recordCallingStack (entry, returnAddress);
        g_HandleInfo[handleValue] = entry;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Chuck.Kirschman   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RemoveEntry (uintptr_t handleValue, bool bReturn)
    {
    pagalloc_printf (PAGALLOC_SEVERITY_DEBUG, "GDILEAK::Removing handle %lx (return %d)\n", handleValue, bReturn);
    if (bReturn)
        {
        delete g_HandleInfo[handleValue];
        g_HandleInfo.erase(handleValue);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Chuck.Kirschman   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void GdiMonitor_Init ()
    {
    assert (s_gdiMonitor == NULL);
    s_gdiMonitor = new CGdiMonitor();
    s_initialNumberOfHandles = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Chuck.Kirschman   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t GdiMonitor_GetCurrentEntryID ()
    {
    return s_recordNumber;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Chuck.Kirschman   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void GdiMonitor_Dump (uint32_t start, void (*callbackFunc) (PageMallocEntry const * const entry, uintptr_t handleId, uint32_t leakNumber, uint32_t count) )
    {
    uint32_t leakNumber = 0;
    std::map<uintptr_t, PageMallocEntry const*>::iterator it;

    // Get counts
    std::map<uintptr_t, uint32_t> callStackCounts;
    for(it = g_HandleInfo.begin(); it != g_HandleInfo.end(); ++it)
        {
        if (it->second->mallocSerialNumber < start)
            continue;
        
        uintptr_t callStackTail = it->second->callStackFrame[0].returnAddress;
        if (callStackCounts.find (callStackTail) == callStackCounts.end())
            callStackCounts [callStackTail] = 1;
        else
            callStackCounts [callStackTail] = callStackCounts [callStackTail] + 1;
        }


    for(it = g_HandleInfo.begin(); it != g_HandleInfo.end(); ++it)
        {
        if (it->second->mallocSerialNumber < start)
            continue;
        callbackFunc (it->second, it->first, leakNumber++, callStackCounts[(uintptr_t)it->second->callStackFrame[0].returnAddress]);
        }
    }

    