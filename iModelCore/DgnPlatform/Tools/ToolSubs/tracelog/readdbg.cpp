/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/tracelog/readdbg.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// These are probably all wrong, but we really want to get rid of this stuff anyway.
#pragma  warning(disable:4302)
#define CHECKHND_OUTPUT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <objbase.h>
#include <dbghelp.h>
#undef DGN_PLATFORM_MT
#include <Bentley/Bentley.h>
#include <DgnPlatform/ExportMacros.h>
#include <DgnPlatform/DesktopTools/filemap.h>
#include <RmgrTools/Tools/ToolSubs.h>
#include <DgnPlatform/DesktopTools/checkhnd.h>
#include <Bentley/Bentley.h>
#include <Bentley/Wstring.h>
#include "cvexefmt.h"

#include <DgnPlatform/DesktopTools/readdbg.h> /* need cvexefmt.h before this */
#include <RmgrTools/Tools/UglyStrings.h>

#include "readdbgh.h"
#define snprintf _snprintf
#define READDBG_ALLOC(size)         VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE)
#define READDBG_FREE(ptr)           VirtualFree(ptr, 0, MEM_RELEASE)
#define READDBG_ACCESS(name)        (0xffffffff == GetFileAttributesA(name))
#define NATIVE_DIRSEP "\\"
#define READDBG_GETENV(val, name)   GetEnvironmentVariableA(name, val, sizeof(val))
/*----------------------------------------------------------------------+
|                                                                       |
|       Prototypes                                                      |
|                                                                       |
+----------------------------------------------------------------------*/
static void freeModuleBaseArray(ReadDbgHandle *readDbgHandleP);
/*----------------------------------------------------------------------+
|                                                                       |
|   Globals                                                             |
|                                                                       |
+----------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    01/97
+---------------+---------------+---------------+---------------+---------------+------*/
static int32_t  readDbgI_open
(
ReadDbgHandle *readDbgHandleP,
int32_t *sysStatusP
)
    {
    int32_t appStatus = 0;
    int32_t *appStatusP = &appStatus;
    DWORD section;
    IMAGE_DOS_HEADER *imageDosHeaderP = NULL;
    /*-------------------------------------------------------------------
    locate image headers
    -------------------------------------------------------------------*/
    imageDosHeaderP = (IMAGE_DOS_HEADER *)readDbgHandleP->fileBaseP;

    if (IMAGE_SEPARATE_DEBUG_SIGNATURE == imageDosHeaderP->e_magic)
        {
        IMAGE_SEPARATE_DEBUG_HEADER *imageSeparateDebugHeaderP =
            (IMAGE_SEPARATE_DEBUG_HEADER *)readDbgHandleP->fileBaseP;
        readDbgHandleP->imageSectionHeaderP = (IMAGE_SECTION_HEADER *)&imageSeparateDebugHeaderP->Reserved[3];
        readDbgHandleP->timeDateStamp = imageSeparateDebugHeaderP->TimeDateStamp;
        readDbgHandleP->checkSum = imageSeparateDebugHeaderP->CheckSum;
        readDbgHandleP->imageBase = imageSeparateDebugHeaderP->ImageBase;
        readDbgHandleP->sizeOfImage = imageSeparateDebugHeaderP->SizeOfImage;
        readDbgHandleP->numberOfSections = imageSeparateDebugHeaderP->NumberOfSections;
        readDbgHandleP->debugDirectoryCount = imageSeparateDebugHeaderP->DebugDirectorySize / sizeof(IMAGE_DEBUG_DIRECTORY);
        readDbgHandleP->imageDebugDirectoryP = (IMAGE_DEBUG_DIRECTORY *)&readDbgHandleP->imageSectionHeaderP[imageSeparateDebugHeaderP->NumberOfSections];
        if (imageSeparateDebugHeaderP->ExportedNamesSize)
            {
            char *tmp = (char *)readDbgHandleP->imageDebugDirectoryP;
            tmp += imageSeparateDebugHeaderP->ExportedNamesSize;
            readDbgHandleP->imageDebugDirectoryP = (IMAGE_DEBUG_DIRECTORY *)tmp;
            }
        }
    else if(IMAGE_DOS_SIGNATURE == imageDosHeaderP->e_magic)
        {
        DWORD debugDirectoryVirtualAddress = 0;
        DWORD exportDirectoryVirtualAddress = 0;
        DWORD importDirectoryVirtualAddress = 0;
        DWORD delayImportDirectoryVirtualAddress = 0;
        DWORD resourceDirectoryVirtualAddress = 0;
        IMAGE_SECTION_HEADER *imageSectionHeaderP = NULL;

        readDbgHandleP->imageNTHeadersP =
            (IMAGE_NT_HEADERS *)(readDbgHandleP->fileBaseP + imageDosHeaderP->e_lfanew);
        CHECK_BOOL_LEAVE((!IsBadReadPtr(readDbgHandleP->imageNTHeadersP, sizeof(IMAGE_NT_HEADERS))), L"IMAGE_NT_SIGNATURE", READDBG_ERROR_NT_SIGNATURE);
        CHECK_BOOL_LEAVE((IMAGE_NT_SIGNATURE == readDbgHandleP->imageNTHeadersP->Signature), L"IMAGE_NT_SIGNATURE", READDBG_ERROR_NT_SIGNATURE);
        imageSectionHeaderP = (IMAGE_SECTION_HEADER *)
            &readDbgHandleP->imageNTHeadersP->OptionalHeader.DataDirectory[readDbgHandleP->imageNTHeadersP->OptionalHeader.NumberOfRvaAndSizes];
        readDbgHandleP->imageSectionHeaderP = imageSectionHeaderP;
        readDbgHandleP->timeDateStamp = readDbgHandleP->imageNTHeadersP->FileHeader.TimeDateStamp;
        readDbgHandleP->checkSum = readDbgHandleP->imageNTHeadersP->OptionalHeader.CheckSum;
        readDbgHandleP->imageBase = readDbgHandleP->imageNTHeadersP->OptionalHeader.ImageBase;
        readDbgHandleP->sizeOfImage = readDbgHandleP->imageNTHeadersP->OptionalHeader.SizeOfImage;
        readDbgHandleP->numberOfSections = readDbgHandleP->imageNTHeadersP->FileHeader.NumberOfSections;
        readDbgHandleP->debugDirectoryCount = readDbgHandleP->imageNTHeadersP->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size / sizeof(IMAGE_DEBUG_DIRECTORY);
        /*---------------------------------------------------------------
        find IMAGE_DEBUG_DIRECTORY and IMAGE_EXPORT_DIRECTORY
        ---------------------------------------------------------------*/
        debugDirectoryVirtualAddress  = readDbgHandleP->imageNTHeadersP->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
        exportDirectoryVirtualAddress = readDbgHandleP->imageNTHeadersP->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        importDirectoryVirtualAddress = readDbgHandleP->imageNTHeadersP->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        delayImportDirectoryVirtualAddress = readDbgHandleP->imageNTHeadersP->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress;
        resourceDirectoryVirtualAddress = readDbgHandleP->imageNTHeadersP->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
        for (section = 0; section < readDbgHandleP->numberOfSections; section++)
            {
            if (( imageSectionHeaderP[section].VirtualAddress <= debugDirectoryVirtualAddress) &&
                ((imageSectionHeaderP[section].VirtualAddress +
                imageSectionHeaderP[section].SizeOfRawData)  > debugDirectoryVirtualAddress))
                {
                readDbgHandleP->imageDebugDirectoryP = (IMAGE_DEBUG_DIRECTORY *)
                    (
                    readDbgHandleP->fileBaseP + debugDirectoryVirtualAddress -
                    imageSectionHeaderP[section].VirtualAddress +
                    imageSectionHeaderP[section].PointerToRawData
                    );
                }
            if (( imageSectionHeaderP[section].VirtualAddress <= exportDirectoryVirtualAddress) &&
                ((imageSectionHeaderP[section].VirtualAddress +
                imageSectionHeaderP[section].SizeOfRawData)  > exportDirectoryVirtualAddress))
                {
                readDbgHandleP->imageExportDirectoryP = (IMAGE_EXPORT_DIRECTORY *)
                    (
                    readDbgHandleP->fileBaseP + exportDirectoryVirtualAddress -
                    imageSectionHeaderP[section].VirtualAddress +
                    imageSectionHeaderP[section].PointerToRawData
                    );
                readDbgHandleP->exportBaseP =
                    readDbgHandleP->fileBaseP -
                    imageSectionHeaderP[section].VirtualAddress +
                    imageSectionHeaderP[section].PointerToRawData;
                }
            if (( imageSectionHeaderP[section].VirtualAddress <= importDirectoryVirtualAddress) &&
                ((imageSectionHeaderP[section].VirtualAddress +
                imageSectionHeaderP[section].SizeOfRawData)  > importDirectoryVirtualAddress))
                {
                readDbgHandleP->imageImportDescriptorP = (IMAGE_IMPORT_DESCRIPTOR *)
                    (
                    readDbgHandleP->fileBaseP + importDirectoryVirtualAddress -
                    imageSectionHeaderP[section].VirtualAddress +
                    imageSectionHeaderP[section].PointerToRawData
                    );
                readDbgHandleP->importBaseP =
                    readDbgHandleP->fileBaseP -
                    imageSectionHeaderP[section].VirtualAddress +
                    imageSectionHeaderP[section].PointerToRawData;
                }
            if (( imageSectionHeaderP[section].VirtualAddress <= delayImportDirectoryVirtualAddress) &&
                ((imageSectionHeaderP[section].VirtualAddress +
                imageSectionHeaderP[section].SizeOfRawData)  > delayImportDirectoryVirtualAddress))
                {
                readDbgHandleP->imageDelayImportDescriptorP = (IMAGE_DELAY_IMPORT_DESCRIPTOR *)
                    (
                    readDbgHandleP->fileBaseP + delayImportDirectoryVirtualAddress -
                    imageSectionHeaderP[section].VirtualAddress +
                    imageSectionHeaderP[section].PointerToRawData
                    );
                readDbgHandleP->delayImportBaseP =
                    readDbgHandleP->fileBaseP -
                    imageSectionHeaderP[section].VirtualAddress +
                    imageSectionHeaderP[section].PointerToRawData;
                }
            if (( imageSectionHeaderP[section].VirtualAddress <= resourceDirectoryVirtualAddress) &&
                ((imageSectionHeaderP[section].VirtualAddress +
                imageSectionHeaderP[section].SizeOfRawData)  > resourceDirectoryVirtualAddress))
                {
                readDbgHandleP->imageResourceDirectoryP = (IMAGE_RESOURCE_DIRECTORY *)
                    (
                    readDbgHandleP->fileBaseP + resourceDirectoryVirtualAddress -
                    imageSectionHeaderP[section].VirtualAddress +
                    imageSectionHeaderP[section].PointerToRawData
                    );
                readDbgHandleP->resourceBaseP =
                    readDbgHandleP->fileBaseP -
                    imageSectionHeaderP[section].VirtualAddress +
                    imageSectionHeaderP[section].PointerToRawData;
                }
            }
        }
    else
        {
        CHECK_HANDLE_LEAVE(NULL, L"IMAGE_DOS_SIGNATURE", READDBG_ERROR_FILE_FORMAT);
        }
    /*-------------------------------------------------------------------
    traverse debug directories
    -------------------------------------------------------------------*/
    for (section = 0; section < readDbgHandleP->debugDirectoryCount; section++)
        {
        switch(readDbgHandleP->imageDebugDirectoryP[section].Type)
            {
        case IMAGE_DEBUG_TYPE_COFF:
            readDbgHandleP->imageCoffSymbolsHeaderP = (IMAGE_COFF_SYMBOLS_HEADER *)(readDbgHandleP->fileBaseP + readDbgHandleP->imageDebugDirectoryP[section].PointerToRawData);
            break;
        case IMAGE_DEBUG_TYPE_CODEVIEW:
            readDbgHandleP->codeViewBaseP = readDbgHandleP->fileBaseP + readDbgHandleP->imageDebugDirectoryP[section].PointerToRawData;
            break;
        case IMAGE_DEBUG_TYPE_FPO:
            readDbgHandleP->fpoDataP = (FPO_DATA *)(readDbgHandleP->fileBaseP + readDbgHandleP->imageDebugDirectoryP[section].PointerToRawData);
            readDbgHandleP->fpoCount = readDbgHandleP->imageDebugDirectoryP[section].SizeOfData / SIZEOF_RFPO_DATA;
            break;
        case IMAGE_DEBUG_TYPE_MISC:
            readDbgHandleP->imageDebugMiscP = (IMAGE_DEBUG_MISC *)(readDbgHandleP->fileBaseP + readDbgHandleP->imageDebugDirectoryP[section].PointerToRawData);
            break;
            }
        }
    return 0;

mycheck_leave:
    return appStatus;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    01/97
+---------------+---------------+---------------+---------------+---------------+------*/
void *          readDbg_open
(
char *filename,
int32_t *appStatusP,
int32_t *sysStatusP
)
    {
    ReadDbgHandle *readDbgHandleP = NULL;
    int32_t appStatus = 0;
    /*-------------------------------------------------------------------
    initialize status to success
    -------------------------------------------------------------------*/
    if (appStatusP)
        *appStatusP = 0;
    if (sysStatusP)
        *sysStatusP = 0;
    /*-------------------------------------------------------------------
    allocate memory for handle
    -------------------------------------------------------------------*/
    readDbgHandleP = (ReadDbgHandle*)READDBG_ALLOC(sizeof(ReadDbgHandle));
    CHECK_HANDLE_RETURN_NULL(readDbgHandleP, L"READDBG_ALLOC", READDBG_ERROR_NOT_ENOUGH_MEMORY);
    memset(readDbgHandleP, 0, sizeof(ReadDbgHandle));
    /*-------------------------------------------------------------------
    create read-only file mapping
    -------------------------------------------------------------------*/
    readDbgHandleP->hFileMapping = fileMap_open(UglyUnicodeString (filename).GetWCharCP (), appStatusP, sysStatusP);
    CHECK_HANDLE_LEAVE((HANDLE)(readDbgHandleP->hFileMapping), L"fileMap_open", READDBG_ERROR_FILE_OPEN);
    /*-------------------------------------------------------------------
    save filename into handle structure
    -------------------------------------------------------------------*/
    strncpy(readDbgHandleP->filename, filename, sizeof(readDbgHandleP)->filename);
    /*-------------------------------------------------------------------
    map file into read-only memory
    -------------------------------------------------------------------*/
    readDbgHandleP->fileBaseP = (Byte*)fileMap_getPtr(readDbgHandleP->hFileMapping, appStatusP, sysStatusP);
    CHECK_HANDLE_LEAVE(readDbgHandleP->fileBaseP, L"fileMap_getPtr", READDBG_ERROR_VIEW_MAP);
    /*-------------------------------------------------------------------
    finish open by reading data into handle structure
    -------------------------------------------------------------------*/
    appStatus = readDbgI_open(readDbgHandleP, sysStatusP);
    if (!appStatus)
        return  (void *)readDbgHandleP;
    else if (appStatusP)
        *appStatusP = appStatus;

mycheck_leave:
    readDbg_close((void *)readDbgHandleP, NULL);
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    01/97
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t         readDbg_close
(
void *hDbg,
int32_t *sysStatusP
)
    {
    bool ok;
    int32_t appStatus = 0;
    int32_t *appStatusP = &appStatus;
    ReadDbgHandle *readDbgHandleP = (ReadDbgHandle *)hDbg;
    CHECK_HANDLE_LEAVE(readDbgHandleP, L"readDbgHandleP", READDBG_ERROR_HANDLE_DBG);

    freeModuleBaseArray(readDbgHandleP);
    if (readDbgHandleP->hFileMapping)
        {
        ok = !fileMap_close(readDbgHandleP->hFileMapping, appStatusP, sysStatusP);
        CHECK_HANDLE_LEAVE((void *)ok, L"fileMap_close", READDBG_ERROR_HANDLE_MAP);
        readDbgHandleP->hFileMapping = 0;
        }
    READDBG_FREE(readDbgHandleP);
mycheck_leave:
    return  appStatus;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    03/97
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t         readDbg_getImageRange
(
void *hDbg,
uintptr_t *pImageBase,
uintptr_t *pImageHigh
)
    {
    int32_t *appStatusP = NULL;
    int32_t *sysStatusP = NULL;
    ReadDbgHandle *readDbgHandleP = (ReadDbgHandle *)hDbg;

    if (pImageBase)
        *pImageBase = 0;
    if (pImageHigh)
        *pImageHigh = 0;
    CHECK_HANDLE_RETURN(readDbgHandleP, L"readDbgHandleP", READDBG_ERROR_HANDLE_DBG);
    if (pImageBase)
        *pImageBase = readDbgHandleP->imageBase;
    if (pImageHigh)
        *pImageHigh = readDbgHandleP->imageBase + readDbgHandleP->sizeOfImage;
    return (readDbgHandleP->imageBase && readDbgHandleP->sizeOfImage) ? 0 : READDBG_ERROR_NOT_FOUND;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    01/04
+---------------+---------------+---------------+---------------+---------------+------*/
static int32_t  readDbg_checkOMFSignature
(
void *hDbg
)
    {
    ReadDbgHandle *readDbgHandleP = (ReadDbgHandle *)hDbg;
    OMFSignature *omfSignatureP = (OMFSignature *)readDbgHandleP->codeViewBaseP;
#if 0
    printf("cv %c%c%c%c\n", omfSignatureP->Signature[0], omfSignatureP->Signature[1],
        omfSignatureP->Signature[2], omfSignatureP->Signature[3]);
#endif
    if (!readDbgHandleP->codeViewBaseP)
        return READDBG_ERROR_NBXX_SIGNATURE;

    if ((strncmp( omfSignatureP->Signature, "NB05", 4 ) != 0) &&
        (strncmp( omfSignatureP->Signature, "NB08", 4 ) != 0) &&
        (strncmp( omfSignatureP->Signature, "NB09", 4 ) != 0) &&
        (strncmp( omfSignatureP->Signature, "NB11", 4 ) != 0))
        return READDBG_ERROR_NBXX_SIGNATURE;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    01/97
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t         readDbg_getSrc
(
void *hDbg,
uintptr_t ipAddress,
char *szSrc,
size_t cbSrc
)
    {
    int32_t appStatus = 0;
    int32_t *appStatusP = NULL;
    int32_t *sysStatusP = NULL;
    char *nameFoundP = NULL;
    int32_t lengthFound = 0;
    uint32_t imodFound = 0;
    uint32_t lineFound = 0;
    ptrdiff_t deltaFound = 0xffffffff;
    char *nameP = NULL;
    uint32_t length;
    uint32_t i;
    uint32_t j;
    uint32_t k;
    uint32_t n;
    uint32_t seg = 0;
    uintptr_t addr = 0;
    ptrdiff_t delta = 0;
    ReadDbgHandle *readDbgHandleP = (ReadDbgHandle *)hDbg;

    CHECK_HANDLE_RETURN(readDbgHandleP, L"readDbgHandleP", READDBG_ERROR_HANDLE_DBG);
    /*-------------------------------------------------------------------
    check that ipAddress is within range for this .dbg image
    -------------------------------------------------------------------*/
    if ((ipAddress < readDbgHandleP->imageBase) ||
        (ipAddress > readDbgHandleP->imageBase +
        readDbgHandleP->sizeOfImage))
        {
#if 0
        printf("address 0x%x is not within range of %s (0x%x-0x%x)\n",
            ipAddress, readDbgHandleP->filename, readDbgHandleP->imageBase,
            readDbgHandleP->imageBase + readDbgHandleP->sizeOfImage);
#endif
        return READDBG_ERROR_NOT_IN_ADDRESS_RANGE;
        }

    if (SUCCESS == readDbg_checkOMFSignature(readDbgHandleP))
        {
        OMFSignature *omfSignatureP = (OMFSignature *)readDbgHandleP->codeViewBaseP;
        OMFDirHeader *omfDirHeaderP = (OMFDirHeader *)((DWORD)readDbgHandleP->codeViewBaseP + omfSignatureP->filepos);
        OMFDirEntry  *omfDirEntryP  = (OMFDirEntry *) ((DWORD)omfDirHeaderP + omfDirHeaderP->cbDirHeader);
        OMFSourceModule  *omfSourceModuleP = NULL;
        OMFSourceFile    *omfSourceFileP   = NULL;
        OMFSourceLine    *omfSourceLineP   = NULL;

        /*---------------------------------------------------------------
        go through dir entries to search for nearest line number
        ---------------------------------------------------------------*/
        for (i = 0; i < omfDirHeaderP->cDir; i++)
            {
            /*-----------------------------------------------------------
            these are the source file and line number entries
            -----------------------------------------------------------*/
            if (sstSrcModule == omfDirEntryP[i].SubSection)
                {
                omfSourceModuleP = (OMFSourceModule *)((DWORD)readDbgHandleP->codeViewBaseP + omfDirEntryP[i].lfo);
#if 0
                printf("sstSrcModule 0x%x @0x%x cb=0x%x: cFile=%d cSeg=%d\n",
                    omfDirEntryP[i].iMod, (DWORD)omfSourceModuleP, omfDirEntryP[i].cb,
                    omfSourceModuleP->cFile, omfSourceModuleP->cSeg);
#endif
                for (j = 0; j < omfSourceModuleP->cFile; j++)
                    {
                    omfSourceFileP = (OMFSourceFile *)((DWORD)omfSourceModuleP +
                        omfSourceModuleP->baseSrcFile[j]);
                    nameP = (char *)((DWORD)omfSourceFileP + 4 + 12*omfSourceFileP->cSeg);
                    length = (uint32_t)(*nameP);
                    nameP++;
#if 0
                    memset(name, 0, sizeof(name));
                    strncpy(name, nameP, min(sizeof(name),length));
                    printf("%s start=0x%x end=0x%x\n", name,
                        omfSourceFileP->baseSrcLn[omfSourceFileP->cSeg],
                        omfSourceFileP->baseSrcLn[omfSourceFileP->cSeg + 1]);
#endif
                    for (k = 0;k < omfSourceFileP->cSeg; k++)
                        {
                        uint16_t *lineNbr;
                        omfSourceLineP = (OMFSourceLine *)((DWORD)omfSourceModuleP +
                            omfSourceFileP->baseSrcLn[k]);
                        seg = omfSourceLineP->Seg;
                        if (seg > 0)
                            {
                            lineNbr = (uint16_t *)(&omfSourceLineP->offset[omfSourceLineP->cLnOff]);
                            for (n = 0; n < omfSourceLineP->cLnOff; n++)
                                {
                                addr = readDbgHandleP->imageBase +
                                    readDbgHandleP->imageSectionHeaderP[seg - 1].VirtualAddress +
                                    omfSourceLineP->offset[n];
#if 0
                                printf("(%d): 0x%08lx\n",
                                    lineNbr[n], omfSourceLineP->offset[n]);
#endif
                                if (addr && (addr <= ipAddress))
                                    {
                                    delta = ipAddress - addr;
                                    if (delta < deltaFound)
                                        {
                                        deltaFound = delta;
                                        lengthFound = length;
                                        nameFoundP = nameP;
                                        lineFound = lineNbr[n];
                                        imodFound = 0; /* don't know this */
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    /*---------------------------------------------------------------
    print out the closest one we found
    ---------------------------------------------------------------*/
    if (nameFoundP)
        {
        char name[1024];

        memset(name, 0, sizeof(name));
        strncpy(name, nameFoundP, min(sizeof(name), lengthFound));
        memset(szSrc, 0, cbSrc);
        snprintf(szSrc, cbSrc, "%s(%d)+0x%x", name, lineFound, deltaFound);
        return      0;
        }
    else
        {
#if 0
        printf("%s could not find source location for address 0x%x\n",
            programNameP, ipAddress);
#endif
        return appStatus ? appStatus : READDBG_WARNING_SRC_NOT_FOUND;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t *getModuleBaseArray(ReadDbgHandle *readDbgHandleP)
    {
    OMFSignature *omfSignatureP = (OMFSignature *)readDbgHandleP->codeViewBaseP;
    OMFDirHeader *omfDirHeaderP = NULL;
    OMFDirEntry  *omfDirEntryP  = NULL;
    PROCSYM32    *procSym32P    = NULL;
    uint32_t imodMax      = 0;
    uint32_t *imodBaseA     = NULL;
    uint32_t i;
    int32_t              size;

    /*---------------------------------------------------------------
    get cached pointer from handle if available
    ---------------------------------------------------------------*/
    if (readDbgHandleP->imodBaseA)
        return readDbgHandleP->imodBaseA;

    /*---------------------------------------------------------------
    avoid crashing if no CodeView data available
    ---------------------------------------------------------------*/
    if (SUCCESS != readDbg_checkOMFSignature(readDbgHandleP))
        return NULL;

    /*---------------------------------------------------------------
    go through all dir entries once to find maximum module number
    ---------------------------------------------------------------*/
    omfDirHeaderP = (OMFDirHeader *)((DWORD)readDbgHandleP->codeViewBaseP + omfSignatureP->filepos);
    omfDirEntryP  = (OMFDirEntry *) ((DWORD)omfDirHeaderP + omfDirHeaderP->cbDirHeader);
    for (i = 0; i < omfDirHeaderP->cDir; i++)
        {
        if ((omfDirEntryP[i].iMod > imodMax) && (0xffff != omfDirEntryP[i].iMod))
            imodMax = omfDirEntryP[i].iMod;
        }
    readDbgHandleP->imodCount = 1 + imodMax;
    /*---------------------------------------------------------------
    allocate memory for module number array
    ---------------------------------------------------------------*/
    size = sizeof(uint32_t *) * (1 + imodMax);
    imodBaseA = (uint32_t *)READDBG_ALLOC(size);
    if (!imodBaseA)
        return NULL;
    memset(imodBaseA, 0, size);
    /*---------------------------------------------------------------
    go back through dir entries to collect module base addresses
    ---------------------------------------------------------------*/
    for (i = 0; i < omfDirHeaderP->cDir; i++)
        {
        /*-----------------------------------------------------------
        just need to save the start of these for references into them
        -----------------------------------------------------------*/
        if (sstAlignSym == omfDirEntryP[i].SubSection)
            {
            procSym32P  = (PROCSYM32 *) ((DWORD)readDbgHandleP->codeViewBaseP + omfDirEntryP[i].lfo);
            imodBaseA[omfDirEntryP[i].iMod] = (DWORD)procSym32P;
            }
        }
    /*---------------------------------------------------------------
    cache pointer into handle
    ---------------------------------------------------------------*/
    if (!readDbgHandleP->imodBaseA)
        readDbgHandleP->imodBaseA = imodBaseA;
    return imodBaseA;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void freeModuleBaseArray(ReadDbgHandle *readDbgHandleP)
    {
    /*---------------------------------------------------------------
    free cached pointer from handle if there
    ---------------------------------------------------------------*/
    if (readDbgHandleP->imodBaseA)
        READDBG_FREE(readDbgHandleP->imodBaseA);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    01/97
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t         readDbg_getSym
(
void *hDbg,
uintptr_t ipAddress,
char *szSym,
int32_t cbSym
)
    {
    int32_t appStatus = 0;
    int32_t *appStatusP = NULL;
    int32_t *sysStatusP = NULL;
    char *nameFoundP = NULL;
    size_t lengthFound = 0;
    uint32_t imodFound = 0;
    ptrdiff_t deltaFound = 0xffffffff;
    uint32_t rectypFound = 0;
    uint32_t iFound = 0xffffffff;
    uint32_t jFound = 0xffffffff;
    int32_t lineFound = 0;
    char name[1024];
    char *nameP = NULL;
    uint32_t length;
    uint32_t i;
    uint32_t j;
    uint32_t seg = 0;
    uintptr_t addr = 0;
    ptrdiff_t delta = 0;
    ReadDbgHandle *readDbgHandleP = (ReadDbgHandle *)hDbg;

    CHECK_HANDLE_RETURN(readDbgHandleP, L"readDbgHandleP", READDBG_ERROR_HANDLE_DBG);
    /*-------------------------------------------------------------------
    check that ipAddress is within range for this .dbg image
    -------------------------------------------------------------------*/
    if ((ipAddress < readDbgHandleP->imageBase) ||
        (ipAddress > readDbgHandleP->imageBase +
        readDbgHandleP->sizeOfImage))
        {
#if 0
        printf("address 0x%x is not within range of %s (0x%x-0x%x)\n",
            ipAddress, readDbgHandleP->filename, readDbgHandleP->imageBase,
            readDbgHandleP->imageBase + readDbgHandleP->sizeOfImage);
#endif
        return      READDBG_ERROR_NOT_IN_ADDRESS_RANGE;
        }

    if (SUCCESS == readDbg_checkOMFSignature(readDbgHandleP))
        {
        OMFSignature *omfSignatureP = (OMFSignature *)readDbgHandleP->codeViewBaseP;
        OMFDirHeader *omfDirHeaderP = NULL;
        OMFDirEntry  *omfDirEntryP  = NULL;
        OMFSymHash   *omfSymHashP   = NULL;
        DATASYM32    *dataSym32P    = NULL;
        PROCSYM32    *procSym32P    = NULL;
        uint32_t *imodBaseA     = NULL;
        bool         isVC5 = !strncmp(omfSignatureP->Signature, "NB11", 4);
        uint32_t rectyp           = 0;
        int32_t          __line__       = 0;
        /*---------------------------------------------------------------
        get or create cached module base array
        ---------------------------------------------------------------*/
        imodBaseA = getModuleBaseArray(readDbgHandleP);
        if (!imodBaseA)
            {
#if 0
            printf( "%s could not allocate memory\n", programNameP);
#endif
            appStatus = READDBG_ERROR_NOT_ENOUGH_MEMORY;
            goto checkCoffSym;
            }
        /*---------------------------------------------------------------
        go through dir entries to search for nearest symbol
        ---------------------------------------------------------------*/
        omfDirHeaderP = (OMFDirHeader *)((DWORD)readDbgHandleP->codeViewBaseP + omfSignatureP->filepos);
        omfDirEntryP  = (OMFDirEntry *) ((DWORD)omfDirHeaderP + omfDirHeaderP->cbDirHeader);
        for (i = 0; i < omfDirHeaderP->cDir; i++)
            {
            /*-----------------------------------------------------------
            these are the globals packed by CVPACK
            -----------------------------------------------------------*/
            if (sstGlobalPub == omfDirEntryP[i].SubSection)
                {
                omfSymHashP = (OMFSymHash *)((DWORD)readDbgHandleP->codeViewBaseP + omfDirEntryP[i].lfo);
                dataSym32P  = (DATASYM32 *) ((DWORD)readDbgHandleP->codeViewBaseP + omfDirEntryP[i].lfo + sizeof(OMFSymHash));
#if 0
                printf("sstGlobalPub 0x%x @0x%x cb=0x%x\n",
                    omfDirEntryP[i].iMod, (DWORD)omfSymHashP, omfDirEntryP[i].cb);
#endif
                for (j = sizeof(OMFSymHash); j <= omfSymHashP->cbSymbol; )
                    {
                    addr = 0;
#ifdef DEBUG_PRINTF
                    if ((S_PUB32_16t != dataSym32P->_16t.rectyp) &&
                        (S_PUB32_32t != dataSym32P->_16t.rectyp) &&
                        (S_ALIGN     != dataSym32P->_16t.rectyp))
                        {
                        printf("unrecognized rectyp 0x%x\n", dataSym32P->_16t.rectyp);
                        /* __asm int 3; */
                        }
#endif
                    if (isVC5)
                        seg = dataSym32P->_32t.seg;
                    else
                        seg = dataSym32P->_16t.seg;

                    if (seg > 0)
                        {
                        addr = readDbgHandleP->imageBase +
                            readDbgHandleP->imageSectionHeaderP[seg - 1].VirtualAddress;
                        if (isVC5)
                            {
                            addr += dataSym32P->_32t.off;
                            nameP = (char *)dataSym32P->_32t.name;
                            }
                        else
                            {
                            addr += dataSym32P->_16t.off;
                            nameP = (char *)dataSym32P->_16t.name;
                            }
                        length = (uint32_t)*nameP;
                        nameP++;
                        rectyp = dataSym32P->_16t.rectyp;
                        __line__ = __LINE__;
                        }
                    if (addr && (addr <= ipAddress))
                        {
                        delta = ipAddress - addr;
                        if (delta < deltaFound)
                            {
                            deltaFound = delta;
                            lengthFound = length;
                            nameFoundP = nameP;
                            imodFound = 0; /* don't know this */
                            rectypFound = rectyp;
                            iFound = i;
                            jFound = j;
                            lineFound = __line__;
#ifdef DEBUG_PRINTF
                            memset(name, 0, sizeof(name));
                            strncpy(name, nameP, min(sizeof(name),length));
                            printf("0x%08lx: %s (0x%x) [%d,%d] @ %s(%d)\n", addr, name, rectypFound, iFound, jFound, __FILE__, __line__);
#endif
                            }
                        }
                    j += dataSym32P->_16t.reclen + 2;
                    dataSym32P = (DATASYM32*) ((DWORD)dataSym32P + dataSym32P->_16t.reclen + 2);
                    }
                }
            /*-----------------------------------------------------------
            these are the references back into the unpacked symbols
            -----------------------------------------------------------*/
            else if ((sstGlobalSym == omfDirEntryP[i].SubSection) ||
                (sstStaticSym == omfDirEntryP[i].SubSection))
                {
                REFSYM     *refSymP     = NULL;

                omfSymHashP = (OMFSymHash *)((DWORD)readDbgHandleP->codeViewBaseP + omfDirEntryP[i].lfo);
                refSymP     = (REFSYM *)    ((DWORD)readDbgHandleP->codeViewBaseP + omfDirEntryP[i].lfo + sizeof(OMFSymHash));
#ifdef DEBUG_PRINTF
                printf("sizeof (REFSYM) == 0x%x, sizeof (OMFSymHash) == 0x%x\n",
                    sizeof (REFSYM), sizeof (OMFSymHash));
                printf("sizeof (PROCSYM32) == 0x%x, sizeof (DATASYM32) == 0x%x\n",
                    sizeof (PROCSYM32), sizeof (DATASYM32));

                procSym32P = NULL;
                printf("0x%02x: reclen\n", &(procSym32P->_32t.reclen));
                printf("0x%02x: rectyp\n", &(procSym32P->_32t.rectyp));
                printf("0x%02x: pParent\n", &(procSym32P->_32t.pParent));
                printf("0x%02x: pEnd\n", &(procSym32P->_32t.pEnd));
                printf("0x%02x: pNext\n", &(procSym32P->_32t.pNext));
                printf("0x%02x: len\n", &(procSym32P->_32t.len));
                printf("0x%02x: DbgStart\n", &(procSym32P->_32t.DbgStart));
                printf("0x%02x: DbgEnd\n", &(procSym32P->_32t.DbgEnd));
                printf("0x%02x: typind\n", &(procSym32P->_32t.typind));
                printf("0x%02x: off\n", &(procSym32P->_32t.off));
                printf("0x%02x: seg\n", &(procSym32P->_32t.seg));
                printf("0x%02x: flags\n", &(procSym32P->_32t.flags));
                printf("0x%02x: name\n", (procSym32P->_32t.name));

                if (sstStaticSym == omfDirEntryP[i].SubSection)
                    printf("sstStaticSym 0x%x, @0x%x cb=0x%x\n",
                    omfDirEntryP[i].iMod, (DWORD)omfSymHashP, omfDirEntryP[i].cb);
                else
                    printf("sstGlobalSym 0x%x, @0x%x cb=0x%x\n",
                    omfDirEntryP[i].iMod, (DWORD)omfSymHashP, omfDirEntryP[i].cb);
#endif
                for (j = sizeof(OMFSymHash); j <= omfSymHashP->cbSymbol; )
                    {
                    addr = 0;
#ifdef DEBUG_PRINTF
                    if ((S_PROCREF      != refSymP->rectyp) &&
                        (S_DATAREF      != refSymP->rectyp) &&
                        (S_ALIGN    != refSymP->rectyp) &&
                        (S_UDT_16t      != refSymP->rectyp) &&
                        (S_GDATA32_16t  != refSymP->rectyp) &&
                        (S_UDT_32t  != refSymP->rectyp) &&
                        (S_GDATA32_32t  != refSymP->rectyp))
                        {
                        printf("unrecognized rectyp 0x%x\n", dataSym32P->_16t.rectyp);
                        /* __asm int 3; */
                        }
#endif
                    if ((S_PROCREF == refSymP->rectyp) ||
                        (S_DATAREF == refSymP->rectyp))
                        {
#if 0
                        printf("rectyp=0x%04x imod=0x%04x ibSym=0x%lx\n",
                            (DWORD)refSymP->rectyp,
                            (DWORD)refSymP->imod,
                            (DWORD)refSymP->ibSym);
#endif
                        if (imodBaseA[refSymP->imod])
                            {
                            procSym32P = (PROCSYM32*)(imodBaseA[refSymP->imod] + refSymP->ibSym);
                            switch (procSym32P->_16t.rectyp)
                                {
                            case S_LDATA32_16t:
                            case S_GDATA32_16t:
                            case S_PUB32_16t:
                                dataSym32P = (DATASYM32*)procSym32P;
                                seg = dataSym32P->_16t.seg;
                                if (seg > 0)
                                    {
                                    addr = readDbgHandleP->imageBase + readDbgHandleP->imageSectionHeaderP[seg - 1].VirtualAddress + dataSym32P->_16t.off;
                                    nameP = (char *)dataSym32P->_16t.name;
                                    length = (uint32_t)*nameP;
                                    nameP++;
                                    rectyp = procSym32P->_16t.rectyp;
                                    __line__ = __LINE__;
#if 0
                                    memset(name, 0, sizeof(name));
                                    strncpy(name, nameP, min(sizeof(name),length));
                                    printf("0x%08lx: %s (%s)\n", addr, name, recTyp2S[0xf & rectyp]);
#endif
                                    }
                                break;

                            case S_LPROC32_16t:
                            case S_GPROC32_16t:
                                seg = procSym32P->_16t.seg;
                                if (seg > 0)
                                    {
                                    addr = readDbgHandleP->imageBase + readDbgHandleP->imageSectionHeaderP[seg - 1].VirtualAddress + procSym32P->_16t.off;
                                    nameP = (char *)procSym32P->_16t.name;
                                    length = (uint32_t)*nameP;
                                    nameP++;
                                    rectyp = procSym32P->_16t.rectyp;
                                    __line__ = __LINE__;
#if 0
                                    memset(name, 0, sizeof(name));
                                    strncpy(name, nameP, min(sizeof(name),length));
                                    printf("0x%08lx: %s (%s)\n", addr, name, recTyp2S[0xf & dataSym32P->_16t.rectyp]);
#endif
                                    }
                                break;

                            case S_LDATA32_32t:
                            case S_GDATA32_32t:
                            case S_PUB32_32t:
                                dataSym32P = (DATASYM32*)procSym32P;
                                seg = dataSym32P->_32t.seg;
                                if (seg > 0)
                                    {
                                    addr = readDbgHandleP->imageBase + readDbgHandleP->imageSectionHeaderP[seg - 1].VirtualAddress + dataSym32P->_32t.off;
                                    nameP = (char *)dataSym32P->_32t.name;
                                    length = (uint32_t)*nameP;
                                    nameP++;
                                    rectyp = procSym32P->_16t.rectyp;
                                    __line__ = __LINE__;
#if 0
                                    memset(name, 0, sizeof(name));
                                    strncpy(name, nameP, min(sizeof(name),length));
                                    printf("0x%08lx: %s (%s)\n", addr, name, recTyp2S[0xf & dataSym32P->_32t.rectyp]);
#endif
                                    }
                                break;

                            case S_LPROC32_32t:
                            case S_GPROC32_32t:
                                seg = procSym32P->_32t.seg;
                                if (seg > 0)
                                    {
                                    addr = readDbgHandleP->imageBase + readDbgHandleP->imageSectionHeaderP[seg - 1].VirtualAddress + procSym32P->_32t.off;
                                    nameP = (char *)procSym32P->_32t.name;
                                    length = (uint32_t)*nameP;
                                    nameP++;
                                    rectyp = procSym32P->_16t.rectyp;
                                    __line__ = __LINE__;
#if 0
                                    memset(name, 0, sizeof(name));
                                    strncpy(name, nameP, min(sizeof(name),length));
                                    printf("0x%08lx: %s (%s)\n", addr, name, recTyp2S[0xf & rectyp]);
#endif
                                    }
                                break;

                            default:
                                break;
                                }
                            }
                        }
                    if (addr && (addr <= ipAddress))
                        {
                        delta = ipAddress - addr;
                        if (delta < deltaFound)
                            {
                            deltaFound = delta;
                            lengthFound = length;
                            nameFoundP = nameP;
                            imodFound = refSymP->imod;
                            iFound = i;
                            jFound = j;
                            rectypFound = rectyp;
                            lineFound = __line__;
#ifdef DEBUG_PRINTF
                            memset(name, 0, sizeof(name));
                            strncpy(name, nameP, min(sizeof(name),length));
                            printf("0x%08lx: %s (0x%x) [%d,%d] @ %s(%d)\n", addr, name, rectypFound, iFound, jFound, __FILE__, __line__);
#endif
                            }
                        }
                    j += refSymP->reclen + 2;
                    refSymP = (REFSYM*) ((DWORD)refSymP + refSymP->reclen + 2);
                    }
                }
            }
        }
checkCoffSym:
    /*-------------------------------------------------------------------
    here is where we try to use exports table if available (non-.dbg file)
    -------------------------------------------------------------------*/
    if (readDbgHandleP->exportBaseP && readDbgHandleP->imageExportDirectoryP)
        {
        uint32_t iFunc;
        uint32_t iName;
        uintptr_t addr = 0;
        ptrdiff_t delta = 0;
        IMAGE_EXPORT_DIRECTORY *imageExportDirectoryP = readDbgHandleP->imageExportDirectoryP;
        DWORD *functionsP = (DWORD *)(readDbgHandleP->exportBaseP + imageExportDirectoryP->AddressOfFunctions);
        DWORD *namesP =             (DWORD *)(readDbgHandleP->exportBaseP + imageExportDirectoryP->AddressOfNames);
        WORD *nameOrdsP =   (WORD  *)(readDbgHandleP->exportBaseP + imageExportDirectoryP->AddressOfNameOrdinals);

        for (iFunc = 0; iFunc < imageExportDirectoryP->NumberOfFunctions; iFunc++)
            {
            addr = readDbgHandleP->imageBase + functionsP[iFunc];
            if (addr && (addr <= ipAddress))
                {
                delta = ipAddress - addr;
                if (delta < deltaFound)
                    {
                    deltaFound = delta;
                    for (iName = 0; iName < imageExportDirectoryP->NumberOfNames; iName++)
                        {
                        if (iFunc == nameOrdsP[iName])
                            {
                            nameFoundP = (char*)readDbgHandleP->exportBaseP + namesP[iName];
                            lengthFound = strlen(nameFoundP);
                            break;
                            }
                        }
                    }
                }
            }
        }
    /*---------------------------------------------------------------
    print out the closest one we found
    ---------------------------------------------------------------*/
    if (nameFoundP)
        {
        memset(name, 0, sizeof(name));
        strncpy(name, nameFoundP, min(sizeof(name),lengthFound));
#ifdef DEBUG_PRINTF
        printf("0x%08lx: %s+0x%x (0x%x) [%d,%d] @ %s(%d)\n", ipAddress, name, deltaFound, rectypFound, iFound, jFound, __FILE__, lineFound);
#endif
        memset(szSym, 0, cbSym);
        snprintf(szSym, cbSym, "%s+0x%x", name, deltaFound);
        return 0;
        }
    else
        {
#if 0
        printf("%s could not find symbol for address 0x%x\n",
            programNameP, ipAddress);
#endif
        return appStatus ? appStatus : READDBG_WARNING_SYM_NOT_FOUND;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    01/97
+---------------+---------------+---------------+---------------+---------------+------*/
static void     readDbgI_testProcRange
(
uintptr_t  const procAddr,
uintptr_t  const addr,
ptrdiff_t    * const deltaStartP,
ptrdiff_t    * const deltaEndP,
uintptr_t    * const procStartP,
uintptr_t    * const procEndP,
int32_t *appStatusP
)
    {

    if (procAddr >= addr)
        {
        ptrdiff_t const deltaAddr = procAddr - addr;
        if (deltaAddr < *deltaStartP)
            {
            *deltaStartP = deltaAddr;
            *procStartP = addr;
            if (*procEndP)
                *appStatusP = 0;
            }
        }
    else
        {
        ptrdiff_t deltaAddr = addr - procAddr;
        if (deltaAddr < *deltaEndP)
            {
            *deltaEndP = deltaAddr;
            *procEndP = addr;
            if (*procStartP)
                *appStatusP = 0;
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  PhilipMcGraw    03/2002
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t         readDbg_getProcRange
(
void *hDbg,
BitMaskP moduleBitMaskP,
uintptr_t procAddr,
uintptr_t *procStartP,
uintptr_t *procEndP
)
    {
    int32_t appStatus = READDBG_ERROR_NOT_FOUND;
    int32_t *appStatusP = NULL;
    int32_t *sysStatusP = NULL;
    uint32_t i;
    uint32_t j;
    int32_t seg = 0;
    uintptr_t addr = 0;
    uintptr_t procStart = 0;
    uintptr_t procEnd = 0;
    ptrdiff_t deltaStart = 0x7fffffff;
    ptrdiff_t deltaEnd   = 0x7fffffff;
    ReadDbgHandle *readDbgHandleP = (ReadDbgHandle *)hDbg;

    CHECK_HANDLE_RETURN(readDbgHandleP, L"readDbgHandleP", READDBG_ERROR_HANDLE_DBG);
    if (SUCCESS == readDbg_checkOMFSignature(readDbgHandleP))
        {
        DWORD            dwCodeViewBase = (DWORD)readDbgHandleP->codeViewBaseP;
        OMFSignature *omfSignatureP = (OMFSignature *)readDbgHandleP->codeViewBaseP;
        OMFDirHeader *omfDirHeaderP = NULL;
        OMFDirEntry  *omfDirEntryP  = NULL;
        PROCSYM32    *procSym32P    = NULL;
#ifdef DEBUG_OBJNAME
        OBJNAMESYM   *objNameSymP   = NULL;
        char  objName[512];
        char *objNameP = NULL;
        int32_t objNameLen = 0;
#endif /* DEBUG_OBJNAME */
#ifdef DEBUG_FUNCNAME
        char  funcName[512];
        char *funcNameP = NULL;
        int32_t funcNameLen = 0;
#endif /* DEBUG_FUNCNAME */
        uintptr_t imageBase     = readDbgHandleP->imageBase;
        IMAGE_SECTION_HEADER *imageSectionHeaderP = readDbgHandleP->imageSectionHeaderP;

        omfDirHeaderP = (OMFDirHeader *)(dwCodeViewBase       + omfSignatureP->filepos);
        omfDirEntryP  = (OMFDirEntry *) ((DWORD)omfDirHeaderP + omfDirHeaderP->cbDirHeader);
        for (i = 0; i < omfDirHeaderP->cDir; i++)
            {
            if (sstAlignSym == omfDirEntryP[i].SubSection)
                {
                procSym32P      = (PROCSYM32 *) (dwCodeViewBase + omfDirEntryP[i].lfo);
#ifdef DEBUG_OBJNAME
                objNameP = NULL;
#endif /* DEBUG_OBJNAME */
#ifdef DEBUG_PRINTF
                printf("sstAlignSym 0x%x @0x%x cb=0x%x\n",
                    omfDirEntryP[i].iMod, (DWORD)procSym32P, omfDirEntryP[i].cb);
#endif
                if (moduleBitMaskP)
                    {
                    bool bitFlagOut = false;

                    bitFlagOut = moduleBitMaskP->Test(omfDirEntryP[i].iMod);
                    /* skip over this subsection if iMod bit not set in mask */
                    if (!bitFlagOut)
                        continue;
                    }

                for (j = 0; (j < omfDirEntryP[i].cb) && procSym32P->_16t.reclen; )
                    {
                    int32_t delta = 0;
                    addr = 0;
                    seg = 0;
#ifdef DEBUG_FUNCNAME
                    funcNameP = NULL;
#endif /* DEBUG_FUNCNAME */

                    switch (procSym32P->_16t.rectyp)
                        {
                    case S_LPROC32_16t:
                    case S_GPROC32_16t:
                        seg = procSym32P->_16t.seg;
                        addr = imageBase + imageSectionHeaderP[seg - 1].VirtualAddress + procSym32P->_16t.off;
#ifdef DEBUG_FUNCNAME
                        funcNameP = (char *)procSym32P->_16t.name;
#endif /* DEBUG_FUNCNAME */
                        break;

                    case S_LPROC32_32t:
                    case S_GPROC32_32t:
                        seg = procSym32P->_32t.seg;
                        addr = imageBase + imageSectionHeaderP[seg - 1].VirtualAddress + procSym32P->_32t.off;
#ifdef DEBUG_FUNCNAME
                        funcNameP = (char *)procSym32P->_32t.name;
#endif /* DEBUG_FUNCNAME */
                        readDbgI_testProcRange(procAddr, addr, &deltaStart, &deltaEnd, &procStart, &procEnd, &appStatus);
                        break;
#ifdef DEBUG_OBJNAME
                    case S_OBJNAME:
                        objNameSymP = (OBJNAMESYM*)procSym32P;
                        objNameP = (char *)objNameSymP->name;
                        objNameLen = (uint32_t)*objNameP;
                        objNameP++;
                        if (srcModuleListP)
                            {
                            objNameLen = min((sizeof(objName)) - 1, objNameLen);
                            memcpy(objName, objNameP, objNameLen);
                            objName[objNameLen] = (char)0;
                            }
                        break;
#endif /* DEBUG_OBJNAME */
                        }

#ifdef DEBUG_FUNCNAME
                    if (funcNameP)
                        {
                        funcNameLen = (uint32_t)*funcNameP;
                        funcNameP++;
                        if (srcModuleListP)
                            {
                            funcNameLen = min((sizeof(funcName)) - 1, funcNameLen);
                            memcpy(funcName, funcNameP, funcNameLen);
                            funcName[funcNameLen] = (char)0;
                            }
#if 0
                        typind = (uint32_t)procSym32P->_16t.typind;
                        printf("\t0x%08lx: %s, seg=0x%x rectyp=%s typind=0x%x %s\n",
                            addr, funcName, seg, get_s_rectyp_funcName(procSym32P->_16t.rectyp),
                            typind, primative_type_funcName ? primative_type_funcName : "");
#endif
                        }
#endif /* DEBUG_FUNCNAME */

                    /* advance to next symbol pointer */
                    delta = (3 + procSym32P->_16t.reclen) & 0xfffffffe; /* word align */
                    j += delta;
                    procSym32P = (PROCSYM32 *)((DWORD)procSym32P + delta);
                    }
                }
            }
        }
    /* search export table */
    if (appStatus && readDbgHandleP->exportBaseP && readDbgHandleP->imageExportDirectoryP)
        {
        uint32_t iFunc;
        uint32_t iName;
        uintptr_t addr = 0;
//        ptrdiff_t delta = 0;
        IMAGE_EXPORT_DIRECTORY *imageExportDirectoryP = readDbgHandleP->imageExportDirectoryP;
        DWORD *functionsP = (DWORD *)(readDbgHandleP->exportBaseP + imageExportDirectoryP->AddressOfFunctions);
//        DWORD *namesP =             (DWORD *)(readDbgHandleP->exportBaseP + imageExportDirectoryP->AddressOfNames);
        WORD *nameOrdsP =   (WORD  *)(readDbgHandleP->exportBaseP + imageExportDirectoryP->AddressOfNameOrdinals);

        for (iName = 0; iName < imageExportDirectoryP->NumberOfNames; iName++)
            {
#if 0
            nameFoundP = nameP = readDbgHandleP->exportBaseP + namesP[iName];
#endif
            iFunc = nameOrdsP[iName];
            addr = readDbgHandleP->imageBase + functionsP[iFunc];
            if (addr)
                {
                readDbgI_testProcRange(procAddr, addr, &deltaStart, &deltaEnd, &procStart, &procEnd, &appStatus);
                }
            }
        }
    if (procStartP)
        *procStartP = procStart;
    if (procEndP)
        *procEndP = procEnd;

    return appStatus;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  PhilipMcGraw    03/2002
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t         readDbg_getProcAddr
(
void          const * const hDbg,
uintptr_t  *  const procAddrP,
char          const * const procName
)
    {
    int32_t *appStatusP = NULL;
    int32_t *sysStatusP = NULL;
    uint32_t i;
    uint32_t j;
    int32_t seg = 0;
    uintptr_t addr = 0;
    ReadDbgHandle *readDbgHandleP = (ReadDbgHandle *)hDbg;

    CHECK_HANDLE_RETURN(readDbgHandleP, L"readDbgHandleP", READDBG_ERROR_HANDLE_DBG);
    if (SUCCESS == readDbg_checkOMFSignature(readDbgHandleP))
        {
        DWORD            dwCodeViewBase = (DWORD)readDbgHandleP->codeViewBaseP;
        OMFSignature *omfSignatureP = (OMFSignature *)readDbgHandleP->codeViewBaseP;
        OMFDirHeader *omfDirHeaderP = NULL;
        OMFDirEntry  *omfDirEntryP  = NULL;
        PROCSYM32    *procSym32P    = NULL;
#ifdef DEBUG_OBJNAME
        OBJNAMESYM   *objNameSymP   = NULL;
        char  objName[512];
        char *objNameP = NULL;
        int32_t objNameLen = 0;
#endif /* DEBUG_OBJNAME */
        char  funcName[512];
        char *funcNameP = NULL;
        int32_t funcNameLen = 0;
        uintptr_t imageBase     = readDbgHandleP->imageBase;
        IMAGE_SECTION_HEADER *imageSectionHeaderP = readDbgHandleP->imageSectionHeaderP;

        omfDirHeaderP = (OMFDirHeader *)(dwCodeViewBase       + omfSignatureP->filepos);
        omfDirEntryP  = (OMFDirEntry *) ((DWORD)omfDirHeaderP + omfDirHeaderP->cbDirHeader);
        for (i = 0; i < omfDirHeaderP->cDir; i++)
            {
            if (sstAlignSym == omfDirEntryP[i].SubSection)
                {
                procSym32P      = (PROCSYM32 *) (dwCodeViewBase + omfDirEntryP[i].lfo);
#ifdef DEBUG_OBJNAME
                objNameP = NULL;
#endif /* DEBUG_OBJNAME */
#ifdef DEBUG_PRINTF
                printf("sstAlignSym 0x%x @0x%x cb=0x%x\n",
                    omfDirEntryP[i].iMod, (DWORD)procSym32P, omfDirEntryP[i].cb);
#endif

                for (j = 0; (j < omfDirEntryP[i].cb) && procSym32P->_16t.reclen; )
                    {
                    int32_t delta = 0;
                    addr = 0;
                    seg = 0;
                    funcNameP = NULL;

                    switch (procSym32P->_16t.rectyp)
                        {
                    case S_LPROC32_16t:
                    case S_GPROC32_16t:
                        seg = procSym32P->_16t.seg;
                        addr = imageBase + imageSectionHeaderP[seg - 1].VirtualAddress + procSym32P->_16t.off;
                        funcNameP = (char *)procSym32P->_16t.name;
                        break;

                    case S_LPROC32_32t:
                    case S_GPROC32_32t:
                        seg = procSym32P->_32t.seg;
                        addr = imageBase + imageSectionHeaderP[seg - 1].VirtualAddress + procSym32P->_32t.off;
                        funcNameP = (char *)procSym32P->_32t.name;
                        break;
#ifdef DEBUG_OBJNAME
                    case S_OBJNAME:
                        objNameSymP = (OBJNAMESYM*)procSym32P;
                        objNameP = (char *)objNameSymP->name;
                        objNameLen = (uint32_t)*objNameP;
                        objNameP++;
                        if (srcModuleListP)
                            {
                            objNameLen = min((sizeof(objName)) - 1, objNameLen);
                            memcpy(objName, objNameP, objNameLen);
                            objName[objNameLen] = (char)0;
                            }
                        break;
#endif /* DEBUG_OBJNAME */
                        }

                    if (funcNameP)
                        {
                        funcNameLen = (uint32_t)*funcNameP;
                        funcNameP++;
                        funcNameLen = min((sizeof(funcName)) - 1, funcNameLen);
                        memcpy(funcName, funcNameP, funcNameLen);
                        funcName[funcNameLen] = (char)0;
                        if (!strcmp(funcName, procName))
                            {
                            if (procAddrP)
                                *procAddrP = addr;
                            return 0;
                            }
#if 0
                        typind = (uint32_t)procSym32P->_16t.typind;
                        printf("\t0x%08lx: %s, seg=0x%x rectyp=%s typind=0x%x %s\n",
                            addr, funcName, seg, get_s_rectyp_funcName(procSym32P->_16t.rectyp),
                            typind, primative_type_funcName ? primative_type_funcName : "");
#endif
                        }

                    /* advance to next symbol pointer */
                    delta = (3 + procSym32P->_16t.reclen) & 0xfffffffe; /* word align */
                    j += delta;
                    procSym32P = (PROCSYM32 *)((DWORD)procSym32P + delta);
                    }
                }
            }
        }
    /* search export table */
    if (readDbgHandleP->exportBaseP && readDbgHandleP->imageExportDirectoryP)
        {
        uint32_t iFunc;
        uint32_t iName;
        uintptr_t addr = 0;
        IMAGE_EXPORT_DIRECTORY *imageExportDirectoryP = readDbgHandleP->imageExportDirectoryP;
        DWORD *functionsP = (DWORD *)(readDbgHandleP->exportBaseP + imageExportDirectoryP->AddressOfFunctions);
        DWORD *namesP =             (DWORD *)(readDbgHandleP->exportBaseP + imageExportDirectoryP->AddressOfNames);
        WORD *nameOrdsP =   (WORD  *)(readDbgHandleP->exportBaseP + imageExportDirectoryP->AddressOfNameOrdinals);

        for (iName = 0; iName < imageExportDirectoryP->NumberOfNames; iName++)
            {
            char *nameP = (char*)readDbgHandleP->exportBaseP + namesP[iName];
            iFunc = nameOrdsP[iName];
            addr = readDbgHandleP->imageBase + functionsP[iFunc];
            if (addr)
                {
                if (!strcmp(nameP, procName))
                    {
                    if (procAddrP)
                        *procAddrP = addr;
                    return 0;
                    }
                }
            }
        }
    return READDBG_ERROR_NOT_FOUND;
    }
