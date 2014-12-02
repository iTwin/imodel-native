/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/envvutil.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnPlatform.h>

BEGIN_EXTERN_C

DESKTOP_TOOLS_EXPORT  BentleyStatus   util_getSysEnv (WStringP envVar, WCharCP name);
DESKTOP_TOOLS_EXPORT  BentleyStatus   util_putenv (WCharCP varName, WCharCP value);

int     util_getCSIDL (WCharCP name);

END_EXTERN_C

DESKTOP_TOOLS_EXPORT  StatusInt       util_readRegistry (WStringR value, WCharCP data);
DESKTOP_TOOLS_EXPORT  bool            util_getFontFaceNameFromFileName (WStringR fontFaceName, WCharCP fontFileName);
