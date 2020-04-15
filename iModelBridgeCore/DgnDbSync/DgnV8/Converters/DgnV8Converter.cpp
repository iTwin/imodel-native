/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnDbSync/DgnV8/DgnV8Bridge.h>
#if defined (BENTLEYCONFIG_PARASOLID)
#include <BRepCore/PSolidUtil.h>
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, wchar_t const* argv[])
    {
    DgnV8Bridge app;
    return app.Run(argc, argv);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge* iModelBridge_getInstance(wchar_t const* bridgeRegSubKey)
    {
    // Note that bridgeRegSubKey may not be "DgnV8Bridge". The ABD bridge, for example, reuses the v8 bridge code and customizes it at run time. The ABD bridge uses its own
    //  registry subkey. We have no way currently to check if brigeRegSubKey is reasonable. We'll just have to trust that the fwk knows what it's doing.
    return new DgnV8Bridge;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BentleyStatus iModelBridge_releaseInstance(iModelBridge* bridge)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (DgnDbApi::PSolidKernelManager::IsSessionStarted())
        DgnDbApi::PSolidKernelManager::StopSession();
#endif

    delete bridge;
    return BentleyApi::BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridge_getAffinity(WCharP buffer,
                              const size_t bufferSize,
                              iModelBridgeAffinityLevel& affinityLevel,
                              WCharCP affinityLibraryPathStr,
                              WCharCP sourceFileNameStr)
    {
    Converter::GetAffinity(buffer, bufferSize, affinityLevel, affinityLibraryPathStr, sourceFileNameStr);


#ifdef COMMENT_OUT_EXAMPLE_CODE
    // *** The following is just an example of how iModelBridge_getAffinity could support more than one bridge.
    if (sourceFileName.EndsWith(L".csv"))
        {
        bridgeAffinity.m_affinity = BentleyApi::Dgn::iModelBridge::Affinity::Low;
        bridgeAffinity.m_bridgeRegSubKey = L"MythicalCsvBridge";
        return;
        }
#endif
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" BentleyApi::BentleyStatus iModelBridge_discloseFilesAndAffinities(WCharCP outputFileName, WCharCP affinityLibraryPathStr, WCharCP assetsPathStr, WCharCP sourceFileNameStr, WCharCP bridgeId)
    {
    DgnV8Bridge bridge;
    return iModelBridge::DiscloseFilesAndAffinities(bridge, outputFileName, affinityLibraryPathStr, assetsPathStr, sourceFileNameStr, bridgeId);
    }
