/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/PublishORDToBIM/PublishORDToBim.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublishORDToBim.h"
#include <ORDBridge/ORDBridgeApi.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

#define RESULT_AFFINITY_CHECK_NONE      0x0001
#define RESULT_AFFINITY_CHECK_LOW       0x0002
#define RESULT_AFFINITY_CHECK_MEDIUM    0x0003
#define RESULT_AFFINITY_CHECK_HIGH      0x0004
#define RESULT_ERROR_FAILED_TO_PARSE_COMMANDLINE    0x1001
#define RESULT_ERROR_FAILED_TO_INITIALIZE_BRIDGE    0x1002
#define RESULT_ERROR_GENERAL            BentleyStatus::ERROR

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
static int runBridge(int argc, WCharCP argv[])
    {
    iModelBridgeSacAdapter::InitCrt(false);

    auto* iModelBridgeP = iModelBridge_getInstance(iModelBridge_getRegistrySubKey());

    iModelBridgeSacAdapter::Params saparams;
    if (BentleyStatus::SUCCESS != iModelBridgeSacAdapter::ParseCommandLine(*iModelBridgeP, saparams, argc, argv))
        return RESULT_ERROR_FAILED_TO_PARSE_COMMANDLINE;

    iModelBridgeSacAdapter::InitializeHost(*iModelBridgeP);

    // Testing affinity interface
    // In reality, this getAfinity API is expected to be called only by the iModelBridge Framework, on a separate process than
    // the actual publishing. Thus, not testing it here anymore.
	/*WChar buffer[_MAX_PATH];
	iModelBridgeAffinityLevel affinityLevel;
	iModelBridge_getAffinity(buffer, _MAX_PATH, affinityLevel, BeFileName(argv[0]).GetDirectoryName(), iModelBridgeP->_GetParams().GetInputFileName());*/

	if (BSISUCCESS != iModelBridgeP->_Initialize(argc, argv))
		{
		fprintf(stderr, "_Initialize failed\n");
		return RESULT_ERROR_FAILED_TO_INITIALIZE_BRIDGE;
		}

	saparams.Initialize();

    /*if (affinityLevel != iModelBridgeAffinityLevel::ExactMatch)
        {
        fprintf(stderr, "No Civil data found.\n");
        if (iModelBridgeAffinityLevel::None == affinityLevel)
            return RESULT_AFFINITY_CHECK_NONE;
        }*/

    auto retVal = iModelBridgeSacAdapter::Execute(*iModelBridgeP, saparams);

    iModelBridge_releaseInstance(iModelBridgeP);

    if (BentleyStatus::SUCCESS == retVal)
        return retVal;
    else
        return RESULT_ERROR_GENERAL;
    }

#if defined(__unix__)
//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
int main(int argc, char** argv)
    {
    WCharP* argv_w = new WCharP[argc];
    for (int i = 0; i<argc; i++)
        {
        argv_w[i] = new WChar[strlen(argv[i]) + 1];
        BeStringUtilities::Utf8ToWChar(argv_w[i], argv[i], strlen(argv[i]) + 1);
        }

    return runBridge(argc, argv_w);
    }
#else

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
int wmain(int argc, WCharCP argv[])
    {
    return runBridge(argc, argv);
    }
#endif
