/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/fileutil.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include <DgnPlatform/DgnPlatform.h>
#include <Bentley/BeFile.h>

BEGIN_EXTERN_C

enum FindFileConstants              // WIP_DGNPLATFORM_TOOLS
    {
    UF_WTR_SUCCESS         = 42,
    UF_OPEN_READ           = 0,
    UF_OPEN_WRITE          = 1,
    UF_OPEN_CREATE         = 2,
    UF_TRY_WRITE_THEN_READ = 4,
    UF_CUR_DIR_SWAP        = 8,
    UF_NO_CUR_DIR          = 0x10,
    UF_JUST_BUILD          = 0x20,
    UF_FIND_FOLDER         = 0x100,
    };

enum FindFileStatus
    {
    FINDFILESTATUS_NameTooLong,
    };

DGNPLATFORM_EXPORT int     util_findFile
(
BeFile          *handle,   /* <= File object                           */
BeFileNameP     outname,  /* <= Output file name                      */
const wchar_t   *inname,   /* => Input file name                       */
const wchar_t   *envvar,   /* => List of related names/logicals        */
const wchar_t   *iext,     /* => Default name and/or extension         */
int             option     /* => Open mode / search options            */
);

DGNPLATFORM_EXPORT int      util_findFileInPath
(
BeFileNameP  expandedNameP,
WCharCP filenameP,
WCharCP extensionP
);

END_EXTERN_C

