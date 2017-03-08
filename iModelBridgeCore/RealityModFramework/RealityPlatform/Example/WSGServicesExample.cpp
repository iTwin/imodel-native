/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/Example/WSGServicesExample.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/WSGServices.h>

#include <stdio.h>
#include <conio.h>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

int main(int argc, char *argv[])
    {
    // the sandbox server does not have a certificate, so we must set verify peer to false;
    // normally it is recommended to always verify the peer
    WSGServer server = WSGServer("dev-realitydataservices-eus.cloudapp.net", false); 

    //-----------------WSGServer methods-----------------------//
    Utf8String version = server.GetVersion(); 

    std::cout << "Version:" << std::endl;
    std::cout << version << std::endl << std::endl;
    
    bvector<Utf8String> plugins = server.GetPlugins();

    std::cout << "Plugins:" << std::endl;
    for( Utf8String plugin : plugins )
        std::cout<< plugin << std::endl;
    std::wcout << std::endl;

    bvector<Utf8String> repos = server.GetRepositories();

    std::cout << "Repos:" << std::endl;
    for (Utf8String repo : repos)
        std::cout << repo << std::endl;
    std::cout << std::endl;

    bvector<Utf8String> schemas = server.GetSchemaNames(repos[0]);

    std::cout << "Schemas in " << repos[0] << ":" << std::endl;
    for (Utf8String schema : schemas)
        std::cout << schema << std::endl;
    std::cout << std::endl;
    
    bvector<Utf8String> classes = server.GetClassNames(repos[0], schemas[0]);

    std::cout << "Classes in " << schemas[0] << ":" << std::endl;
    for (Utf8String classname : classes)
        std::cout << classname << std::endl;
    std::cout << std::endl;

    Utf8String classJSON = server.GetJSONClassDefinition(repos[0], schemas[0], classes[0]);

    std::cout << classes[0] << " as JSON:" << std::endl;
    std::cout << classJSON << std::endl << std::endl;

    getch();

    //-----------------WSGRequest methods-----------------------//
    //getting all Nav Roots
    bvector<NavNode> nodes = NodeNavigator::GetInstance().GetRootNodes(server, repos[0]);

    std::cout << "NavRoots:" << std::endl;
    for (NavNode root : nodes)
        std::cout << root.GetNavString() << std::endl;
    std::cout << std::endl;
    
    //using a NavNode request
    bvector<NavNode> subNodes = bvector<NavNode>();
    int nodeIndex;

    for(nodeIndex = 0; (subNodes.size() < 1) && nodeIndex < nodes.size(); ++nodeIndex)
        {
        subNodes = NodeNavigator::GetInstance().GetChildNodes(server, repos[0], nodes[nodeIndex]);
        }

    if(nodeIndex > nodes.size())
        {
        std::cout<< "No root nodes have any sub nodes" << std::endl;
        getch();
        return 0;
        }

    std::cout << "NavNodes under "<< nodes[nodeIndex].GetNavString() << " :" << std::endl;
    for (NavNode subNode : subNodes)
        std::cout << subNode.GetNavString() << std::endl;
    std::cout << std::endl;


    //finding an object
    bool objectFound = false;
    int objectIndex = 0;
    Utf8String navString;
    while(!objectFound && subNodes.size() > 0)
        {
        for( int i = 0; i < subNodes.size(); i ++ )
            {
            if(subNodes[i].GetClassName().Equals("Document"))
                {
                objectFound = true;
                objectIndex = i;
                navString = subNodes[i].GetRootNode();
                navString.append("~2F");
                navString.append(subNodes[i].GetInstanceId());
                navString.ReplaceAll("/", "~2F");
                break;
                }
            }
        if (!objectFound)
            subNodes = NodeNavigator::GetInstance().GetChildNodes(server, repos[0], subNodes[0]);
        }
        
    if(!objectFound)
        {
        std::cout << "no document found, with basic exploration" << std::endl;
        getch();
        return 1;
        }

    std::cout<<"Object location :" << std::endl;
    std::cout<< navString << std::endl << std::endl;

    Utf8String objectId = subNodes[objectIndex].GetRootId();
    objectId.append("~2F");
    objectId.append(subNodes[objectIndex].GetInstanceId());
    objectId.ReplaceAll("/","~2F");

    WSGObjectRequest* objRequest = new WSGObjectRequest(server.GetServerName(), server.GetVersion(), repos[0], subNodes[0].GetSchemaName(), subNodes[0].GetClassName(), objectId);
    int status = 0;
    Utf8String returnJsonString = WSGRequest::GetInstance().PerformRequest(*objRequest, status, 0);

    std::cout << "Object JSON :" << std::endl;
    std::cout << returnJsonString << std::endl << std::endl;

    // we won't execute the actual download, because a randomly chosen file could represent several gigabytes
    // but this code shows how it could be done
    /*WSGObjectContentRequest* objRequest = new WSGObjectContentRequest(server.GetServerName(), server.GetVersion(), repos[0], subNodes[0].GetSchemaName(), subNodes[0].GetClassName(), objectId);
    
    //open file
    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);  
    BeFileName fileName = BeFileName(exeDir);
    fileName.AppendToPath(BeFileName(subNodes[objectIndex].GetInstanceId()));
    char outfile[1024] = "";
    strcpy(outfile, fileName.GetNameUtf8().c_str());
    FILE* file = fopen(outfile, "wb");

    //performRequest with file pointer
    WSGRequest::GetInstance().PerformRequest(*objRequest, status, 0, file);*/

    getch();

    return 0;
    }