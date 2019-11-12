/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#include "CS06BridgeTestApp.h"
#include <CS06Bridge/CS06BridgeApi.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_CS06BRIDGE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
static int runBridge(int argc, WCharCP argv[])
{
	iModelBridgeSacAdapter::InitCrt(false);

	auto* iModelBridgeP = iModelBridge_getInstance(CS06Bridge::GetRegistrySubKey());

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
	iModelBridgeWithAffinity affinity;
	iModelBridge_getAffinity(affinity, BeFileName(), iModelBridgeP->_GetParams().GetInputFileName());

	if (affinity.m_affinity != iModelBridgeAffinityLevel::ExactMatch)
	{
		fprintf(stderr, "Affinity-check failed\n");
		return BentleyStatus::ERROR;
	}

	return iModelBridgeSacAdapter::Execute(*iModelBridgeP, saparams);
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
