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
    WSGServer server = WSGServer("s3mxcloudservice.cloudapp.net", false); 

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

    std::cout << "Schemas:" << std::endl;
    for (Utf8String schema : schemas)
        std::cout << schema << std::endl;
    std::cout << std::endl;
    
    bvector<Utf8String> classes = server.GetClassNames(repos[0], schemas[0]);

    std::cout << "Classes:" << std::endl;
    for (Utf8String classname : classes)
        std::cout << classname << std::endl;
    std::cout << std::endl;

    Utf8String classJSON = server.GetJSONClassDefinition(repos[0], schemas[0], classes[0]);

    std::cout << "Class as JSON:" << std::endl;
    std::cout << classJSON << std::endl << std::endl;

    getch();

    WSGNavRootRequest* navRoot = new WSGNavRootRequest(server.GetServerName(), server.GetVersion(), repos[0]);
    
    int status = 0;
    Utf8String returnJsonString = WSGRequest::GetInstance().PerformRequest(*navRoot, status, 0); //again, normally you should call PerformRequest without the 0

    Json::Value instances(Json::objectValue);
    BeAssert ((status == CURLE_OK) && Json::Reader::Parse(returnJsonString, instances) && (!instances.isMember("errorMessage") && instances.isMember("instances")));
        
    bvector<Utf8String> nodes = bvector<Utf8String>();
    for (auto instance : instances["instances"])
        {
        if (instance.isMember("instanceId") && instance.isMember("properties") && instance["properties"].isMember("Label"))
            nodes.push_back(instance["instanceId"].asCString());
        }

    std::cout << "NavRoots:" << std::endl;
    for (Utf8String root : nodes)
        std::cout << root << std::endl;
    std::cout << std::endl;

    WSGNavNodeRequest* navNode = new WSGNavNodeRequest(server.GetServerName(), server.GetVersion(), repos[0], nodes[0]);
    status = 0;
    returnJsonString = WSGRequest::GetInstance().PerformRequest(*navNode, status, 0);

    Json::Value instances2(Json::objectValue);
    BeAssert((status == CURLE_OK) && Json::Reader::Parse(returnJsonString, instances2) && (!instances2.isMember("errorMessage") && instances2.isMember("instances")));

    bvector<Utf8String> subNodes = bvector<Utf8String>();
    for (auto instance : instances2["instances"])
        {
        if (instance.isMember("instanceId"))
            subNodes.push_back(instance["instanceId"].asCString());
        }

    std::cout << "NavNodes under "<< nodes[0] << " :" << std::endl;
    for (Utf8String subNode : subNodes)
        std::cout << subNode << std::endl;
    std::cout << std::endl;

    getch();

    return 0;
    }


