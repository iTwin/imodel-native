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
};