/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/readdbg.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#define __readDbgH__
#include <Bentley/Bentley.h>
#include <DgnPlatform/DgnPlatformBaseType.r.h>
#include <DgnPlatform/Tools/BitMask.h>

extern "C" {

/*----------------------------------------------------------------------+
|                                                                       |
|       Definitions                                                     |
|                                                                       |
+----------------------------------------------------------------------*/
#define READDBG_ERROR_FILE_OPEN             0x1000
#define READDBG_ERROR_FILE_MAP              0x1001
#define READDBG_ERROR_VIEW_MAP              0x1002
#define READDBG_ERROR_NT_SIGNATURE          0x1003
#define READDBG_ERROR_FILE_FORMAT           0x1004
#define READDBG_ERROR_HANDLE_DBG            0x1005
#define READDBG_ERROR_HANDLE_VIEW           0x1006
#define READDBG_ERROR_HANDLE_MAP            0x1007
#define READDBG_ERROR_HANDLE_FILE           0x1008
#define READDBG_ERROR_NOT_IN_ADDRESS_RANGE  0x1009
#define READDBG_ERROR_FPO_NOT_FOUND         0x100a
#define READDBG_ERROR_MISC_NOT_FOUND        0x100b
#define READDBG_ERROR_NOT_ENOUGH_MEMORY     0x100c
#define READDBG_ERROR_NBXX_SIGNATURE        0x100d
#define READDBG_ERROR_CHECKSUM              0x100e
#define READDBG_ERROR_FILEINDEX_NOT_FOUND   0x100f
#define READDBG_ERROR_NOT_IN_IMOD_RANGE     0x1010
#define READBG_ERROR_NOT_IN_IFILE_RANGE     0x1011
#define READDBG_ERROR_IMPORT_SIGNATURE      0x1012

#define READDBG_ERROR_NOT_FOUND             0x1fff

#define READDBG_WARNING_SRC_NOT_FOUND       0x2000
#define READDBG_WARNING_SYM_NOT_FOUND       0x2001
#define READDBG_WARNING_ADDR_NOT_FOUND      0x2002

/* typedef some things if wintypes.h hasn't yet: */
#ifndef IMAGE_NT_SIGNATURE
typedef void *                              HANDLE;
typedef UInt32                              DWORD;
typedef struct _fpo_data                    FPO_DATA;
typedef struct _image_nt_headers            IMAGE_NT_HEADERS;
typedef struct _image_section_header        IMAGE_SECTION_HEADER;
typedef struct _image_debug_directory       IMAGE_DEBUG_DIRECTORY;
typedef struct _image_export_directory      IMAGE_EXPORT_DIRECTORY;
typedef struct _image_resource_directory    IMAGE_RESOURCE_DIRECTORY;
typedef struct _image_coff_symbols_header   IMAGE_COFF_SYMBOLS_HEADER;
typedef struct _image_debug_misc            IMAGE_DEBUG_MISC;
typedef struct _import_object_header        IMPORT_OBJECT_HEADER;
#endif
#ifndef __cvexefmtH__
typedef struct _omf_global_types            OMFGlobalTypes;
#endif
#ifndef _CV_INFO_INCLUDED
typedef struct _TYPTYPE                     TYPTYPE;
#endif
typedef struct _TypeData
    {
    struct _offset
    {
    UInt32 bytes;
    UInt32 bits;
    } offset;
    struct _size
    {
    UInt32 bytes;
    UInt32 bits;
    } size;
    UInt32 value;
    UInt32 count;
    UInt32 leaf;
    UInt32 typeIndexRef;
    char prefix[1024];                      // WIP_CHAR_OK
    char name[1024];                        // WIP_CHAR_OK
    char suffix[1024];                      // WIP_CHAR_OK
    } TypeData;

/*----------------------------------------------------------------------+
|
|       Prototypes
|
+----------------------------------------------------------------------*/

// open and close functions:
void *          readDbg_open(char *filename, Int32 *appStatus, Int32 *sysStatus);       // WIP_CHAR_OK - not worth updating arcane debugging code.
Int32           readDbg_close(void *hDbg, Int32 *sysStatus);

// low-level data functions:
Int32           readDbg_getImageRange(void *hDbg, uintptr_t *pImageBase, uintptr_t *pImageHigh);

// high-level functions:
Int32           readDbg_getSrc(void *hDbg, uintptr_t ipAddress, char *szSrc, size_t cbSrc);                                                   // WIP_CHAR_OK - files store narrow chars.
Int32           readDbg_getSym(void *hDbg, uintptr_t ipAddress, char *szSym, Int32 cbSym);                                                    // WIP_CHAR_OK - files store narrow chars.
Int32           readDbg_getProcRange(void *hDbg, BitMaskP moduleBitMaskP, uintptr_t procAddr, uintptr_t *procStartP, uintptr_t *procEndP);    // WIP_CHAR_OK - files store narrow chars.
Int32           readDbg_getProcAddr(void const * const hDbg, uintptr_t * const procAddrP, char const * const procName);                       // WIP_CHAR_OK - files store narrow chars.

}       // extern "C"

