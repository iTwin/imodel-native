/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/GeomJsApi/ts/GeomJsApiBootstrap.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Bentley/WString.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      7/15
//---------------------------------------------------------------------------------------
Utf8CP geomJsApi_GetBootstrappingSource()
    {
    static Utf8Char source[] = __GEOMSCRIPTJSSOURCE__;
    return source;
    }
