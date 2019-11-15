/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#ifdef __PUBLISHORDTOBIMDLL_BUILD__
#define PUBLISHORDTOBIMDLL_EXPORT EXPORT_ATTRIBUTE
#else
#define PUBLISHORDTOBIMDLL_EXPORT IMPORT_ATTRIBUTE
#endif

struct PublishORDToBimDLL
{
public:
    PUBLISHORDTOBIMDLL_EXPORT static int RunBridge(int argc, WCharCP argv[]);
    static WCharCP* AddUnitTestingParameter(int& argc, WCharCP argv[]);
    static WCharCP* AddLoggingConfigParameter(int& argc, WCharCP argv[]);
};
