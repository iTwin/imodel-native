/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformTools/Example/SimpleRDSApiExample.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatformTools/SimpleRDSApi.h>

#include <stdio.h>
#include <conio.h>
#include <iomanip>
#include <sstream>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

int main(int argc, char *argv[])
    {
    RDSRequestManager::GetInstance();

    bvector<ConnectedNavNode> rootNodes = bvector<ConnectedNavNode>();

    ConnectedResponse response = ConnectedNavNode::GetRootNodes(rootNodes);

    if(!response.simpleSuccess)
        {
        std::cout << response.simpleMessage << std::endl;
        return 1;
        }

    ConnectedRealityData rd = ConnectedRealityData(rootNodes[0].GetInstanceId());

    std::cout << "RealityData name: " << rd.GetName() << std::endl;

    bvector<ConnectedRealityDataRelationshipPtr> relationships = bvector<ConnectedRealityDataRelationshipPtr>();
    response = ConnectedRealityDataRelationship::RetrieveAllForRDId(relationships, rd.GetIdentifier());

    std::cout << "has " << relationships.size() << " relationships" << std::endl;

    bvector<ConnectedNavNode> childNodes = bvector<ConnectedNavNode>();
    response = rootNodes[0].GetChildNodes(childNodes);

    if(!childNodes.empty())
        {
        if(childNodes[0].GetECClassName() == "Folder")
            {
            ConnectedRealityDataFolder folder = ConnectedRealityDataFolder(childNodes[0].GetInstanceId());
            std::cout << "folder name: " << folder.GetName() << std::endl;
            }
        else if (childNodes[0].GetECClassName() == "Document")
            {
            ConnectedRealityDataDocument doc = ConnectedRealityDataDocument(childNodes[0].GetInstanceId());
            std::cout << "document content type: " << doc.GetContentType() << std::endl;
            }
        }

    ConnectedRealityDataEnterpriseStat stats = ConnectedRealityDataEnterpriseStat();

    response = stats.GetStats();

    std::cout << "total size on server: " << stats.GetTotalSizeKB() << "KB" << std::endl;

    return 0;
    }