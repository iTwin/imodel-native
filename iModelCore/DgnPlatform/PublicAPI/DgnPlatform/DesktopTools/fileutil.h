/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/fileutil.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include <DgnPlatform/DgnPlatform.h>
#include <Bentley/BeFile.h>

BEGIN_EXTERN_C

DESKTOP_TOOLS_EXPORT int     util_findFile
(
BeFile          *handle,   /* <= File object                           */
wchar_t         *outname,  /* <= Output file name                      */
const wchar_t   *inname,   /* => Input file name                       */
const wchar_t   *envvar,   /* => List of related names/logicals        */
const wchar_t   *iext,     /* => Default name and/or extension         */
int             option     /* => Open mode / search options            */
);

DESKTOP_TOOLS_EXPORT int      util_findFileInPath
(
WCharP  expandedNameP,
WCharCP filenameP,
WCharCP extensionP
);

END_EXTERN_C

