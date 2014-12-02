/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/filemap.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#define __fileMapH__
/*__BENTLEY_INTERNAL_ONLY__*/

#include <BaseTsd.h>    // from Windows SDK
#include <Bentley/Bentley.h>

/*----------------------------------------------------------------------+
|                                                                       |
|       Definitions                                                     |
|                                                                       |
+----------------------------------------------------------------------*/
/* Error codes */
#define FILEMAP_ERROR_FILE_OPEN             (0x4000)
#define FILEMAP_ERROR_FILE_MAP              (0x4001)
#define FILEMAP_ERROR_VIEW_MAP              (0x4002)
#define FILEMAP_ERROR_HANDLE_DBG            (0x4005)
#define FILEMAP_ERROR_HANDLE_VIEW           (0x4006)
#define FILEMAP_ERROR_HANDLE_MAP            (0x4007)
#define FILEMAP_ERROR_HANDLE_FILE           (0x4008)
#define FILEMAP_ERROR_FLUSH                 (0x4009)
#define FILEMAP_ERROR_NOT_ENOUGH_MEMORY     (0x400c)

/* Flags */
#define FILEMAP_FLAG_READONLY               (1)
#define FILEMAP_FLAG_READWRITE              (0)
#define FILEMAP_FLAG_SHARED                 (1)
#define FILEMAP_FLAG_PRIVATE                (0)
#define FILEMAP_FLAG_ASYNC                  (1)
#define FILEMAP_FLAG_SYNC                   (0)

BEGIN_EXTERN_C
/*----------------------------------------------------------------------+
|                                                                       |
|       Prototypes                                                      |
|                                                                       |
+----------------------------------------------------------------------*/
/* original APIs: */
uintptr_t fileMap_open(WCharCP filename, Int32 *appStatusP, Int32 *sysStatusP);
StatusInt fileMap_close(uintptr_t handle,   Int32 *appStatusP, Int32 *sysStatusP);
void   *fileMap_getPtr(uintptr_t handle,  Int32 *appStatusP, Int32 *sysStatusP);
size_t fileMap_getSize(uintptr_t handle, Int32 *appStatusP, Int32 *sysStatusP);

/* new read/write APIs: */
uintptr_t fileMap_openRW
    (
    WCharCP filename,
    WCharCP mapname,
    Int32 readonly,
    Int32 shared,
    Int32 offset,
    Int32 sizeIn,
    Int32 *sizeOutP,
    Int32 *appStatusP,
    Int32 *sysStatusP
    );
StatusInt fileMap_flush
    (
    uintptr_t handle,
    Byte *start,
    Int32 size,
    Int32 async,
    Int32 *appStatusP,
    Int32 *sysStatusP
    );

/* new file handle based APIs: */
uintptr_t fileMap_fhopen
    (
    uintptr_t hFile,
    Int32 *appStatusP,
    Int32 *sysStatusP
    );
uintptr_t fileMap_fhopenRW
    (
    uintptr_t hFile,
    WCharCP mapname,
    Int32 readonly,
    Int32 shared,
    Int32 offset,
    Int32 sizeIn,
    Int32 *sizeOutP,
    Int32 *appStatusP,
    Int32 *sysStatusP
    );

END_EXTERN_C
