/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_EXTERN_C

BentleyStatus   util_getSysEnv (WStringP envVar, WCharCP name);
BentleyStatus   util_putenv (WCharCP varName, WCharCP value);

int             util_getCSIDL (WCharCP name);

StatusInt       util_readRegistry (WStringR value, WCharCP data);
bool            util_getFontFaceNameFromFileName (WStringR fontFaceName, WCharCP fontFileName);

END_EXTERN_C
