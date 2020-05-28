/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeFwkRegistry.h>
#include <Logging/bentleylogging.h>
#include <Bentley/BeDirectoryIterator.h>

#define LOG (*LoggingManager::GetLogger(L"iModelBridgeRegistry"))

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/20
+---------------+---------------+---------------+---------------+---------------+------*/
static void addToPath(BeFileNameCR dir)
    {
    WPrintfString newPath(L"PATH=%ls;", dir.c_str());

    wchar_t *oldPath;
    size_t len;
    errno_t err = _wdupenv_s(&oldPath, &len, L"PATH");
    if (err)
        return;
    newPath.append(oldPath);
    free(oldPath);
    _wputenv(newPath.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/20
+---------------+---------------+---------------+---------------+---------------+------*/
static void addMdlSysAsNeededToPath(BeFileNameCR dir)
    {
    BeFileName msan(dir);
    msan.AppendToPath(L"MdlSys");
    msan.AppendToPath(L"AsNeeded");
    if (msan.DoesPathExist())
        addToPath(msan);
    }

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
    
        addMdlSysAsNeededToPath(affinityLibraryPath.GetDirectoryName()); // Needed only when loading and calling into OBD bridge's affinity dll

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
