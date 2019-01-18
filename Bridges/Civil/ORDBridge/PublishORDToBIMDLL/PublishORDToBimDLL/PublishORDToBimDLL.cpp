/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/PublishORDToBIMDLL/PublishORDToBimDLL/PublishORDToBimDLL.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ORDBridge/ORDBridgeApi.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>
#include <ORDBridge/PublishORDToBimDLL.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

#define RESULT_AFFINITY_CHECK_NONE      0x0001
#define RESULT_AFFINITY_CHECK_LOW       0x0002
#define RESULT_AFFINITY_CHECK_MEDIUM    0x0003
#define RESULT_AFFINITY_CHECK_HIGH      0x0004
#define RESULT_ERROR_FAILED_TO_PARSE_COMMANDLINE    0x1001
#define RESULT_ERROR_FAILED_TO_INITIALIZE_BRIDGE    0x1002
#define RESULT_ERROR_GENERAL            BentleyStatus::ERROR

int PublishORDToBimDLL::RunBridge(int argc, WCharCP argv[])
    {
    //Initialize
    iModelBridgeSacAdapter::InitCrt(false);

    auto* iModelBridgeP = iModelBridge_getInstance(iModelBridge_getRegistrySubKey());

    iModelBridgeSacAdapter::Params saparams;
    if (BentleyStatus::SUCCESS != iModelBridgeSacAdapter::ParseCommandLine(*iModelBridgeP, saparams, argc, argv))
        return RESULT_ERROR_FAILED_TO_PARSE_COMMANDLINE;

    bool shouldTryUpdate = saparams.ShouldTryUpdate();
    saparams.SetShouldTryUpdate(true);

    if(&DgnViewLib::GetHost() == nullptr)
        iModelBridgeSacAdapter::InitializeHost(*iModelBridgeP);

    // Testing affinity interface
    WChar buffer[_MAX_PATH];
    iModelBridgeAffinityLevel affinityLevel;
    iModelBridge_getAffinity(buffer, _MAX_PATH, affinityLevel, BeFileName(argv[0]).GetDirectoryName(), iModelBridgeP->_GetParams().GetInputFileName());

    if (BSISUCCESS != iModelBridgeP->_Initialize(argc, argv))
    {
        fprintf(stderr, "_Initialize failed\n");
        return RESULT_ERROR_FAILED_TO_INITIALIZE_BRIDGE;
    }

    saparams.Initialize();

    if (affinityLevel != iModelBridgeAffinityLevel::ExactMatch)
    {
        fprintf(stderr, "No Civil data found.\n");
        if (iModelBridgeAffinityLevel::None == affinityLevel)
            return RESULT_AFFINITY_CHECK_NONE;
    }

    //Execute
    auto retVal = iModelBridgeSacAdapter::Execute(*iModelBridgeP, saparams);

    //Terminate
    iModelBridge_releaseInstance(iModelBridgeP);
    /*
    DgnViewLib::GetHost().Terminate(true);
    ImageppLib::GetHost().Terminate();
    iModelBridge::L10N::Terminate();
    */

    if (BentleyStatus::SUCCESS == retVal)
        return retVal;
    else
        return RESULT_ERROR_GENERAL;

    }
