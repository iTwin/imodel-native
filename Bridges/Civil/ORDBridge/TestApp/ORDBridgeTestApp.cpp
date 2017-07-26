/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/TestApp/ORDBridgeTestApp.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#include "ORDBridgeTestApp.h"
#include <ORDBridge/ORDBridgeApi.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_ORDBRIDGE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
static int runBridge(int argc, WCharCP argv[])
    {
    iModelBridgeSacAdapter::InitCrt(false);

    ORDBridge* bridge = dynamic_cast<ORDBridge*>(iModelBridge_getInstance());

    iModelBridgeSacAdapter::Params saparams;
    if (BentleyStatus::SUCCESS != iModelBridgeSacAdapter::ParseCommandLine(*bridge, saparams, argc, argv))
        return BentleyStatus::ERROR;

    iModelBridgeSacAdapter::InitializeHost(*bridge);

    if (BSISUCCESS != bridge->_Initialize(argc, argv))
        {
        fprintf(stderr, "_Initialize failed\n");
        return BentleyStatus::ERROR;
        }

    saparams.Initialize();

    return iModelBridgeSacAdapter::Execute(*bridge, saparams);
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
