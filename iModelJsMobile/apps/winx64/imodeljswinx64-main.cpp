/*--------------------------------------------------------------------------------------+
|
|     $Source: apps/winx64/imodeljswinx64-main.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <iModelJs/iModelJs.h>
#include <iModelJs/iModelJsServicesTier.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
int wmain(int argc, WCharCP argv[])
    {
    BentleyApi::iModelJs::ServicesTier::UvHost host;
    while (!host.IsReady()) { ; }

    return 0;
    }
