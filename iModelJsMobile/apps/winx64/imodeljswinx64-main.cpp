/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
