/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/DgnJsApi/ts/DgnJsApiBootstrap.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Bentley/WString.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      7/15
//---------------------------------------------------------------------------------------
Utf8CP dgnScriptContext_GetBootstrappingSource()
    {
    static Utf8Char source[] = __DGNSCRIPTJSSOURCE__;
    return source;
    }
