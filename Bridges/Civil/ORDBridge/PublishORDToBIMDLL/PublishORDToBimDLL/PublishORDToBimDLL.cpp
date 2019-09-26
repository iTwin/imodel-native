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

static bool s_hostInitialized = false;

int PublishORDToBimDLL::RunBridge(int argc, WCharCP argv[])
    {
    //Initialize
    iModelBridgeSacAdapter::InitCrt(false);

    auto* iModelBridgeP = iModelBridge_getInstance(iModelBridge_getRegistrySubKey());


    iModelBridgeSacAdapter::Params saparams;
    if (BentleyStatus::SUCCESS != iModelBridgeSacAdapter::ParseCommandLine(*iModelBridgeP, saparams, argc, argv))
        return RESULT_ERROR_FAILED_TO_PARSE_COMMANDLINE;

    // Need to add the '--logging-config-file' parameter since 
    // is not done by caller and we are unit testing and the default is not good enough
    // Not able to pass as args in ParseCommandLine ... strings get killed
    char* logFile = getenv("BEGTEST_LOGGING_CONFIG");
    BeFileName logFilePath(logFile);
    if (logFilePath.DoesPathExist())
        {
        argv = AddLoggingConfigParameter(argc, argv);
        saparams.SetLoggingConfigFile(logFilePath);
        }

    // <Jdec> Should the below line be used to test for update?
    // bool shouldTryUpdate = saparams.ShouldTryUpdate();
    saparams.SetShouldTryUpdate(true);

    if (!s_hostInitialized)
        {
        iModelBridgeSacAdapter::InitializeHost(*iModelBridgeP);
        s_hostInitialized = true;
        }

    // Testing affinity interface
    WChar buffer[_MAX_PATH];
    iModelBridgeAffinityLevel affinityLevel;
    iModelBridge_getAffinity(buffer, _MAX_PATH, affinityLevel, BeFileName(argv[0]).GetDirectoryName(), iModelBridgeP->_GetParams().GetInputFileName());

    //Need to add the '--unit-testing' parameter to argv so our ORDBridge instance knows we're unit testing
    argv = AddUnitTestingParameter(argc, argv);

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

    delete[] argv;

    if (BentleyStatus::SUCCESS == retVal)
        return retVal;
    else
        return RESULT_ERROR_GENERAL;

    }

WCharCP* PublishORDToBimDLL::AddUnitTestingParameter(int& argc, WCharCP argv[])
    {
    WCharCP* argvWithArgument = new WCharCP[argc+1];
    for (int i = 0; i < argc; i++)
        {
        argvWithArgument[i] = argv[i];
        }
    WCharCP* unitTestArg = new WCharCP(L"--unit-testing");
    argvWithArgument[argc] = *unitTestArg;
    argc++;
    return argvWithArgument;
    }

WCharCP* PublishORDToBimDLL::AddLoggingConfigParameter(int& argc, WCharCP argv[])
    {
    char* logFile = getenv("BEGTEST_LOGGING_CONFIG");

    BeFileName filePath(logFile);
    if (filePath.DoesPathExist())
        {
        WCharCP inputArgument(WString(WCharCP(L"--logging-config-file=\"")).append(filePath.c_str()).append(WCharCP(L"\"")).c_str());

        WCharCP* argvWithArgument = new WCharCP[argc+1];
        for (int i = 0; i < argc; i++)
            argvWithArgument[i] = argv[i];

        argvWithArgument[argc] = inputArgument;
        argc++;

        return argvWithArgument;
        }
    return argv;
    }

