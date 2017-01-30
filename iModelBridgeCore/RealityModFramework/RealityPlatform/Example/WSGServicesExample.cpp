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

Utf8String NavNodeFct(WSGServer server, Utf8String repo, Utf8String nodeString)
    {
    WSGNavNodeRequest* navNode = new WSGNavNodeRequest(server.GetServerName(), server.GetVersion(), repo, nodeString);
    int status = 0;
    Utf8String returnJsonString = WSGRequest::GetInstance().PerformRequest(*navNode, status, 0);

    Json::Value instances(Json::objectValue);
    BeAssert((status == CURLE_OK) && Json::Reader::Parse(returnJsonString, instances) && (!instances.isMember("errorMessage") && instances.isMember("instances")));

    return returnJsonString;
    }


int main(int argc, char *argv[])
    {
    // the sandbox server does not have a certificate, so we must set verify peer to false;
    // normally it is recommended to always verify the peer
    WSGServer server = WSGServer("s3mxcloudservice.cloudapp.net", false); 

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
    WSGNavRootRequest* navRoot = new WSGNavRootRequest(server.GetServerName(), server.GetVersion(), repos[0]);
    
    int status = 0;
    Utf8String returnJsonString = WSGRequest::GetInstance().PerformRequest(*navRoot, status, 0); //again, normally you should call PerformRequest without the 0

    Json::Value instances(Json::objectValue);
    BeAssert ((status == CURLE_OK) && Json::Reader::Parse(returnJsonString, instances) && (!instances.isMember("errorMessage") && instances.isMember("instances")));
        
    bvector<NavNode> nodes = bvector<NavNode>();
    for (auto instance : instances["instances"])
        nodes.push_back(NavNode(instance));

    std::cout << "NavRoots:" << std::endl;
    for (NavNode root : nodes)
        std::cout << root.GetNavString() << std::endl;
    std::cout << std::endl;


    //using a NavNode request
    WSGNavNodeRequest* navNode = new WSGNavNodeRequest(server.GetServerName(), server.GetVersion(), repos[0], nodes[0].GetNavString());
    status = 0;
    returnJsonString = WSGRequest::GetInstance().PerformRequest(*navNode, status, 0);

    Json::Value instances2(Json::objectValue);
    BeAssert((status == CURLE_OK) && Json::Reader::Parse(returnJsonString, instances2) && (!instances2.isMember("errorMessage") && instances2.isMember("instances")));

    bvector<NavNode> subNodes = bvector<NavNode>();
    for (auto instance : instances2["instances"])
        subNodes.push_back(NavNode(instance));

    std::cout << "NavNodes under "<< nodes[0].GetNavString() << " :" << std::endl;
    for (NavNode subNode : subNodes)
        std::cout << subNode.GetNavString() << std::endl;
    std::cout << std::endl;


    //finding an object
    bool objectFound = false;
    int objectIndex = 0;
    Utf8String navString;
    while(!objectFound)
        {
        for( int i = 0; i < subNodes.size(); i ++ )
            {
            if(subNodes[i].GetInstanceId().Contains(".")) // '.' used for file extension
                {
                objectFound = true;
                objectIndex = i;
                navString = nodes[0].GetNavString();
                navString.append("~2F");
                navString.append(subNodes[i].GetInstanceId());
                navString.ReplaceAll("/", "~2F");
                break;
                }
            }
        
        if(!objectFound)
            {
            navString = nodes[0].GetNavString();
            navString.append("~2F");
            navString.append(subNodes[0].GetInstanceId());
            navString.ReplaceAll("/", "~2F");
            returnJsonString = NavNodeFct(server, repos[0], navString);
            BeAssert(Json::Reader::Parse(returnJsonString, instances2) && (!instances2.isMember("errorMessage") && instances2.isMember("instances")));

            subNodes = bvector<NavNode>();
            for (auto instance : instances2["instances"])
                subNodes.push_back(NavNode(instance));
            }
        }

    std::cout<<"Object location :" << std::endl;
    std::cout<< navString << std::endl << std::endl;

    Utf8String objectId = nodes[0].GetInstanceId();
    objectId.append("~2F");
    objectId.append(subNodes[objectIndex].GetInstanceId());
    objectId.ReplaceAll("/","~2F");

    WSGObjectRequest* objRequest = new WSGObjectRequest(server.GetServerName(), server.GetVersion(), repos[0], subNodes[0].GetSchemaName(), subNodes[0].GetClassName(), objectId);
    returnJsonString = WSGRequest::GetInstance().PerformRequest(*objRequest, status, 0);

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
    strcpy(outfile, m_packageFileName.GetNameUtf8().c_str());
    FILE* file = fopen(outfile, "wb");

    //performRequest with file pointer
    WSGRequest::GetInstance().PerformRequest(*objRequest, status, 0, file);*/

    getch();

    return 0;
    }