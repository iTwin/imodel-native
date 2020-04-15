/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeFwkRegistry.h>
#include <Logging/bentleylogging.h>

#define LOG (*LoggingManager::GetLogger(L"iModelBridgeRegistry"))

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int iModelBridgeRegistryUtils::ComputeAffinityMain(int argc, WCharCP argv[])
    {
    InitCrt(false);

    if (argc == 6)
        {
        WCharCP outputFileName = argv[1];
        WCharCP affinityLibraryPathStr = argv[2];
        WCharCP assetsPathStr = argv[3];
        WCharCP sourceFileNameStr = argv[4];
        WCharCP bridgeRegSubkey = argv[5];
        
        BeFileName affinityLibraryPath(affinityLibraryPathStr);

        auto DiscloseFilesAndAffinities = (T_iModelBridge_discloseFilesAndAffinities*)iModelBridgeRegistryUtils::GetBridgeFunction(affinityLibraryPath, "iModelBridge_discloseFilesAndAffinities");
        if (nullptr == DiscloseFilesAndAffinities)
            {
            LOG.warningv(L"%ls - does not export the iModelBridge_discloseFilesAndAffinities function. This bridge should be upgraded to implement that function.", affinityLibraryPath.c_str());
            return -1;
            }
        return DiscloseFilesAndAffinities(outputFileName, affinityLibraryPathStr, assetsPathStr, sourceFileNameStr, bridgeRegSubkey);
        }


    if (argc != 2)
        {
        fprintf(stderr, "syntax: iModelBridgeGetAffinityHost affinityLibraryPath\n");
        return -1;
        }

    BeFileName affinityLibraryPath(argv[1]);

    auto getAffinity = (T_iModelBridge_getAffinity*)iModelBridgeRegistryUtils::GetBridgeFunction(affinityLibraryPath, "iModelBridge_getAffinity");
    if (nullptr == getAffinity)
        {
        LOG.errorv(L"%ls - does not export the iModelBridge_getAffinity function. That is probably an error.", affinityLibraryPath.c_str());
        return -1;
        }

    char filePathUtf8[MAX_PATH*2];
    while (fgets(filePathUtf8, sizeof(filePathUtf8), stdin))
        {
        BeFileName filePath(filePathUtf8, true);
        filePath.RemoveQuotes();
        
        WChar registryName[MAX_PATH] = {0};
        iModelBridgeAffinityLevel affinity = iModelBridgeAffinityLevel::None;
        getAffinity(registryName, MAX_PATH, affinity, affinityLibraryPath.c_str(), filePath.c_str());
        fprintf(stdout, "%d\n%s\n", (int) affinity, Utf8String(registryName).c_str());
        fflush(stdout);
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, WCharCP argv[])
    {
    return iModelBridgeRegistryUtils::ComputeAffinityMain(argc, argv);
    }

#ifdef __unix__
UNIX_MAIN_CALLS_WMAIN(WCharCP*)
#endif
