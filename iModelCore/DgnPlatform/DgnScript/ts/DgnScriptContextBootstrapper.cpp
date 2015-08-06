/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/ts/DgnScriptContextBootstrapper.cpp $
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
    static Utf8Char source[] = __OBJECTMODELJSSOURCE__;
    return source;
    }
