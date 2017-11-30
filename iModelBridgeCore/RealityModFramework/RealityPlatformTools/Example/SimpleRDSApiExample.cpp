/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformTools/Example/SimpleRDSApiExample.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatformTools/SimpleRDSApi.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

int main(int argc, char *argv[])
    {
    RDSRequestManager::GetInstance();

    bvector<ConnectedNavNode> rootNodes = bvector<ConnectedNavNode>();

    ConnectedResponse response = ConnectedNavNode::GetRootNodes(rootNodes);

    ConnectedRealityData rd = ConnectedRealityData(rootNodes[0].GetInstanceId());

    return 0;
    }