/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#include "ORDBridgeTestApp.h"
#include <ORDBridge/ORDBridgeApi.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
static int runBridge(int argc, WCharCP argv[])
    {
    iModelBridgeSacAdapter::InitCrt(false);

    auto* iModelBridgeP = iModelBridge_getInstance(iModelBridge_getRegistrySubKey());

    iModelBridgeSacAdapter::Params saparams;
    if (BentleyStatus::SUCCESS != iModelBridgeSacAdapter::ParseCommandLine(*iModelBridgeP, saparams, argc, argv))
        return BentleyStatus::ERROR;

    iModelBridgeSacAdapter::InitializeHost(*iModelBridgeP);

    if (BSISUCCESS != iModelBridgeP->_Initialize(argc, argv))
        {
        fprintf(stderr, "_Initialize failed\n");
        return BentleyStatus::ERROR;
        }

    saparams.Initialize();

    // Testing affinity interface
    WChar buffer[_MAX_PATH];
    iModelBridgeAffinityLevel affinityLevel;
    iModelBridge_getAffinity(buffer, _MAX_PATH, affinityLevel, BeFileName(argv[0]).GetDirectoryName(), iModelBridgeP->_GetParams().GetInputFileName());

    if (affinityLevel == iModelBridgeAffinityLevel::None)
        {
        fprintf(stderr, "Affinity-check failed\n");
        return BentleyStatus::ERROR;
        }

    auto retVal = iModelBridgeSacAdapter::Execute(*iModelBridgeP, saparams);

    iModelBridge_releaseInstance(iModelBridgeP);

    return retVal;
    }

#if defined(__unix__)
//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
int main(int argc, char** argv)
    {
    WCharP* argv_w = new WCharP[argc];
    for(int i=0;i<argc;i++)
        {
        argv_w[i] = new WChar[strlen(argv[i]) + 1];
        BeStringUtilities::Utf8ToWChar(argv_w[i], argv[i],strlen(argv[i])+1);
        }
    
    return runBridge(argc,argv_w);
    }
#else

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
int wmain(int argc, WCharCP argv[])
    {
    return runBridge(argc,argv);
    }
#endif
