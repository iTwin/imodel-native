/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/nonport/filemap.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifdef winNT
#define NOMINMAX
#include <windows.h>
#define FILEMAP_ALLOC(size)         VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE)
#define FILEMAP_FREE(ptr)           VirtualFree(ptr, 0, MEM_RELEASE)
#else
#include <unistd.h>
#include <sys/mman.h>
#define FILEMAP_ALLOC(size)         malloc(size)
#define FILEMAP_FREE(ptr)           free(ptr)
typedef void *HANDLE;
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#undef BENTLEY_PORTABLE_CODE

#include <DgnPlatform/DesktopTools/checkhnd.h>
#include <DgnPlatform/DesktopTools/filemap.h>
#include <Bentley/BeFileName.h>
/*----------------------------------------------------------------------+
|                                                                       |
|       Typedefs                                                        |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct fileMapHandle
    {
    BeFileName filename;
    int32_t readonly;
    int32_t shared;
    int32_t offset;
    size_t size;
#ifdef winNT
    HANDLE hFile;
    HANDLE hFileMapping;
    PSECURITY_ATTRIBUTES pSA;
#else
    uintptr_t hFile;
    struct stat fileStat;
#endif
    Byte* fileBaseP; /* pointer where mapped file begins in memory */
    } FileMapHandle;
/*----------------------------------------------------------------------+
|                                                                       |
|   Globals                                                             |
|                                                                       |
+----------------------------------------------------------------------*/

#ifdef winNT
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    01/97
+---------------+---------------+---------------+---------------+---------------+------*/
PSECURITY_ATTRIBUTES fileMapNT_allocSA
(
void
)
    {
    PSECURITY_ATTRIBUTES    pSA = NULL;
#if 0
    PSECURITY_DESCRIPTOR    pSD = NULL;
    DWORD                   fVersion = GetVersion();
    /* return only NULL for Win9x to avoid error code 87 later... */
    if (0x80000000 & fVersion)
        return NULL;
    /*-------------------------------------------------------------------
    create a NULL discretionary access list so other users can access
    objects we create here
    -------------------------------------------------------------------*/
    pSD = (PSECURITY_DESCRIPTOR) FILEMAP_ALLOC(SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (pSD)
        {
        memset(pSD, 0, SECURITY_DESCRIPTOR_MIN_LENGTH);
        if (InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
            {
            if (SetSecurityDescriptorDacl(pSD, true, (PACL) NULL, false))
                {
                pSA = FILEMAP_ALLOC(sizeof(SECURITY_DESCRIPTOR));
                if (pSA)
                    {
                    memset(pSA, 0, sizeof(SECURITY_DESCRIPTOR));
                    pSA->nLength = sizeof(SECURITY_DESCRIPTOR);
                    pSA->lpSecurityDescriptor = pSD;
                    pSA->bInheritHandle = false;
                    }
                }
            }
        }
#endif
    return  pSA;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    01/97
+---------------+---------------+---------------+---------------+---------------+------*/
void            fileMapNT_freeSA
(
PSECURITY_ATTRIBUTES    pSA
)
    {
    if (pSA)
        {
        if (pSA->lpSecurityDescriptor)
            FILEMAP_FREE(pSA->lpSecurityDescriptor);
        FILEMAP_FREE(pSA);
        }
    return;
    }
#endif /* winNT */

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
static uintptr_t fileMapI_openRW
    (
    WCharCP filename,
    uintptr_t hFile,
    WCharCP mapname,
    int32_t readonly,
    int32_t shared,
    int32_t offset,
    int32_t sizeIn,
    int32_t *sizeOutP,
    int32_t *appStatusP,
    int32_t *sysStatusP
    )
    {
    FileMapHandle *fileMapHandleP = NULL;
    int32_t size = sizeIn;
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
    fileMapHandleP = (FileMapHandle*) FILEMAP_ALLOC(sizeof(FileMapHandle));
    CHECK_HANDLE_RETURN_ZERO(fileMapHandleP, L"FILEMAP_ALLOC", FILEMAP_ERROR_NOT_ENOUGH_MEMORY);
    memset(fileMapHandleP, 0, sizeof(FileMapHandle));
    fileMapHandleP->readonly = readonly;
    fileMapHandleP->shared = shared;
    fileMapHandleP->offset = offset;
#ifdef winNT
    /*-------------------------------------------------------------------
    open file
    -------------------------------------------------------------------*/
    if (filename)
    {
    fileMapHandleP->pSA = fileMapNT_allocSA();
    fileMapHandleP->hFile = ::CreateFileW
        (
        filename,
        readonly ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        fileMapHandleP->pSA,
        readonly ? OPEN_EXISTING : OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
        0
        );
    }
    else
    fileMapHandleP->hFile = (HANDLE)hFile;
#else
    if (filename)
    fileMapHandleP->hFile = open(filename, readonly ? O_RDONLY : O_CREAT | O_RDWR, 0666);
    else
    fileMapHandleP->hFile = hFile;
#endif
    CHECK_HANDLE_LEAVE((HANDLE)(fileMapHandleP->hFile), L"open", FILEMAP_ERROR_FILE_OPEN);
    if (filename)
    fileMapHandleP->filename.SetName (filename);
    /*-------------------------------------------------------------------
    determine size of file
    -------------------------------------------------------------------*/
#ifdef winNT
    size = GetFileSize(fileMapHandleP->hFile, NULL);
#else
    memset(&(fileMapHandleP->fileStat), 0, sizeof(fileMapHandleP)->fileStat);
    fstat(fileMapHandleP->hFile, &fileMapHandleP->fileStat);
    size = fileMapHandleP->fileStat.st_size;
#endif
    /*-------------------------------------------------------------------
    set new size of file
    -------------------------------------------------------------------*/
    if (!size && !readonly)
    {
    size = sizeIn;
#ifndef winNT
    /* read last int in file */
    lseek(fileMapHandleP->hFile, size - sizeof(data), SEEK_SET);
    read(fileMapHandleP->hFile, &data, sizeof(data));
    /* rewrite last int in file (to set size if not set) */
    if (-1 != lseek(fileMapHandleP->hFile, size - sizeof(data), SEEK_SET))
        write(fileMapHandleP->hFile, &data, sizeof(data));
    /* rewind file position to beginning */
    lseek(fileMapHandleP->hFile, 0, SEEK_SET);
#endif
    }
    /*-------------------------------------------------------------------
    map file into memory
    -------------------------------------------------------------------*/
#ifdef winNT
    fileMapHandleP->hFileMapping = CreateFileMappingW
    (
     fileMapHandleP->hFile,
     fileMapHandleP->pSA,
     readonly ? PAGE_READONLY : PAGE_READWRITE, 0, size,
     shared ? mapname : NULL
    );
    CHECK_HANDLE_LEAVE(fileMapHandleP->hFileMapping, L"CreateFileMapping", FILEMAP_ERROR_FILE_MAP);
    fileMapHandleP->fileBaseP = (Byte*)MapViewOfFileEx
    (
    fileMapHandleP->hFileMapping,
    readonly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, offset, size, NULL
    );
    CHECK_HANDLE_LEAVE(fileMapHandleP->fileBaseP, L"MapViewOfFile", FILEMAP_ERROR_VIEW_MAP);
#else /* POSIX */
    fileMapHandleP->fileBaseP = mmap
    (
    NULL,
    size,
    readonly ? PROT_READ : PROT_READ | PROT_WRITE,
    shared ? MAP_SHARED : MAP_PRIVATE,
    fileMapHandleP->hFile,
    offset
    );
    CHECK_HANDLE_LEAVE(fileMapHandleP->fileBaseP, L"mmap", FILEMAP_ERROR_VIEW_MAP);
#endif /* POSIX */
    fileMapHandleP->size = size;
    if (sizeOutP)
    *sizeOutP = size;
    return  (uintptr_t)fileMapHandleP;
mycheck_leave:
    fileMap_close((uintptr_t)fileMapHandleP, NULL, NULL);
    return  0;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t fileMap_openRW
    (
    WCharCP filename,
    WCharCP mapname,
    int32_t readonly,
    int32_t shared,
    int32_t offset,
    int32_t sizeIn,
    int32_t *sizeOutP,
    int32_t *appStatusP,
    int32_t *sysStatusP
    )
    {
    return fileMapI_openRW(filename, (-1), mapname, readonly, shared,
        offset, sizeIn, sizeOutP, appStatusP, sysStatusP);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t fileMap_fhopenRW
    (
    uintptr_t hFile,
    WCharCP mapname,
    int32_t readonly,
    int32_t shared,
    int32_t offset,
    int32_t sizeIn,
    int32_t *sizeOutP,
    int32_t *appStatusP,
    int32_t *sysStatusP
    )
    {
    return fileMapI_openRW(NULL, hFile, mapname, readonly, shared,
        offset, sizeIn, sizeOutP, appStatusP, sysStatusP);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    02/99
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t fileMap_open
    (
    WCharCP filename,
    int32_t *appStatusP,
    int32_t *sysStatusP
    )
    {
    return fileMap_openRW(filename, NULL,
        FILEMAP_FLAG_READONLY, FILEMAP_FLAG_PRIVATE,
        0, 0, NULL, appStatusP, sysStatusP);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    02/99
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t fileMap_fhopen
    (
    uintptr_t hFile,
    int32_t *appStatusP,
    int32_t *sysStatusP
    )
    {
    return fileMap_fhopenRW(hFile, NULL,
        FILEMAP_FLAG_READONLY, FILEMAP_FLAG_PRIVATE,
        0, 0, NULL, appStatusP, sysStatusP);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt fileMap_flush
    (
    uintptr_t handle,
    char *start,
    int32_t size,
    int32_t async,
    int32_t *appStatusP,
    int32_t *sysStatusP
    )
    {
    int32_t ok;
    int32_t appStatus = 0;
    FileMapHandle *fileMapHandleP = NULL;
#ifndef winNT
    uint32_t pagemask = (getpagesize() - 1);
#endif
    /*-------------------------------------------------------------------
    initialize status to success
    -------------------------------------------------------------------*/
    if (appStatusP)
    *appStatusP = 0;
    if (sysStatusP)
    *sysStatusP = 0;
    /*-------------------------------------------------------------------
    cast handle to internal structure
    -------------------------------------------------------------------*/
    fileMapHandleP = (FileMapHandle *)handle;
    CHECK_HANDLE_LEAVE(fileMapHandleP, L"fileMapHandleP", FILEMAP_ERROR_HANDLE_DBG);
    /*-------------------------------------------------------------------
    flush memory mapped file to disk
    -------------------------------------------------------------------*/
#ifdef winNT
    ok = FlushViewOfFile(start, size);
//  Commented out by BernMcCarty.  ok is NOT a handle...
//  CHECK_HANDLE_LEAVE((HANDLE)ok, L"FlushViewOfFile", FILEMAP_ERROR_FLUSH);
#else
    size += (uint32_t)start & pagemask;
    start = (char *)((uint32_t)start & ~pagemask);
    ok = !msync(start, size, (async ? MS_ASYNC : MS_SYNC) | MS_INVALIDATE);
    CHECK_HANDLE_LEAVE((HANDLE)ok, L"msync", FILEMAP_ERROR_FLUSH);
#endif
mycheck_leave:
    return  appStatus;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    02/99
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     fileMap_close
    (
    uintptr_t handle,
    int32_t *appStatusP,
    int32_t *sysStatusP
    )
    {
    int32_t ok;
    int32_t appStatus = 0;
    FileMapHandle *fileMapHandleP = NULL;
    /*-------------------------------------------------------------------
    initialize status to success
    -------------------------------------------------------------------*/
    if (appStatusP)
    *appStatusP = 0;
    if (sysStatusP)
    *sysStatusP = 0;
    /*-------------------------------------------------------------------
    cast handle to internal structure
    -------------------------------------------------------------------*/
    fileMapHandleP = (FileMapHandle *)handle;
    CHECK_HANDLE_LEAVE(fileMapHandleP, L"fileMapHandleP", FILEMAP_ERROR_HANDLE_DBG);
#ifdef winNT
    if (fileMapHandleP->fileBaseP)
    {
    ok = UnmapViewOfFile
        (
        fileMapHandleP->fileBaseP
        );
//      Commented out by BernMcCarty.  ok is NOT a handle...
//      CHECK_HANDLE_LEAVE((HANDLE)ok,L"UnmapViewOfFile", FILEMAP_ERROR_HANDLE_VIEW);
    fileMapHandleP->fileBaseP = NULL;
    }
    if (fileMapHandleP->hFileMapping)
    {
    ok = CloseHandle
        (
        fileMapHandleP->hFileMapping
        );
//      Commented out by BernMcCarty.  ok is NOT a handle...
//      CHECK_HANDLE_LEAVE((HANDLE)ok,L"CloseHandle", FILEMAP_ERROR_HANDLE_MAP);
    fileMapHandleP->hFileMapping = NULL;
    }
    if (INVALID_HANDLE_VALUE != fileMapHandleP->hFile)
    {
    ok = CloseHandle
        (
        fileMapHandleP->hFile
        );
//      Commented out by BernMcCarty.  ok is NOT a handle...
//      CHECK_HANDLE_LEAVE((HANDLE)ok,L"CloseHandle", FILEMAP_ERROR_HANDLE_FILE);
    fileMapHandleP->hFile = NULL;
    }
    fileMapNT_freeSA(fileMapHandleP->pSA);
#else /* POSIX */
    if (fileMapHandleP->fileBaseP)
    {
    ok = !munmap(fileMapHandleP->fileBaseP, fileMapHandleP->fileStat.st_size);
    CHECK_HANDLE_LEAVE((HANDLE)ok, L"munmap", FILEMAP_ERROR_HANDLE_VIEW);
    }
    if (-1 != fileMapHandleP->hFile)
    {
    ok = !close(fileMapHandleP->hFile);
    CHECK_HANDLE_LEAVE((HANDLE)ok, L"close", FILEMAP_ERROR_HANDLE_FILE);
    }
#endif /* POSIX */
    FILEMAP_FREE(fileMapHandleP);
mycheck_leave:
    return  appStatus;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    02/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            *fileMap_getPtr
    (
    uintptr_t handle,
    int32_t *appStatusP,
    int32_t *sysStatusP
    )
    {
    FileMapHandle *fileMapHandleP = NULL;
    /*-------------------------------------------------------------------
    initialize status to success
    -------------------------------------------------------------------*/
    if (appStatusP)
    *appStatusP = 0;
    if (sysStatusP)
    *sysStatusP = 0;
    /*-------------------------------------------------------------------
    cast handle to internal structure
    -------------------------------------------------------------------*/
    fileMapHandleP = (FileMapHandle *)handle;
    CHECK_HANDLE_RETURN_NULL(fileMapHandleP, L"fileMapHandleP", FILEMAP_ERROR_HANDLE_DBG);
    return  fileMapHandleP->fileBaseP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  PhilipMcGraw    05/2001
+---------------+---------------+---------------+---------------+---------------+------*/
size_t fileMap_getSize
    (
    uintptr_t handle,
    int32_t *appStatusP,
    int32_t *sysStatusP
    )
    {
    FileMapHandle *fileMapHandleP = NULL;
    /*-------------------------------------------------------------------
    initialize status to success
    -------------------------------------------------------------------*/
    if (appStatusP)
    *appStatusP = 0;
    if (sysStatusP)
    *sysStatusP = 0;
    /*-------------------------------------------------------------------
    cast handle to internal structure
    -------------------------------------------------------------------*/
    fileMapHandleP = (FileMapHandle *)handle;
    CHECK_HANDLE_RETURN_ZERO(fileMapHandleP, L"fileMapHandleP", FILEMAP_ERROR_HANDLE_DBG);
    return  fileMapHandleP->size;
    }

