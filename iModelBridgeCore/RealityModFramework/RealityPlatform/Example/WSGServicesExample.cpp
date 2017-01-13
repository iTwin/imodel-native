/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/Example/WSGServicesExample.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <RealityPlatform/WSGServices.h>

#include <stdio.h>
#include <conio.h>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

int main(int argc, char *argv[])
    {
    WSGServer server = WSGServer("dev-realitydataservices-eus.cloudapp.net");

    Utf8String version = server.GetVersion(); 

    std::cout << version << std::endl;

    getch();

    bvector<Utf8String> plugins = server.GetPlugins();

    for( Utf8String plugin : plugins )
        std::cout<< plugin << std::endl;

    getch();

    bvector<Utf8String> repos = server.GetRepositories();

    for (Utf8String repo : repos)
        std::cout << repo << std::endl;

    getch();

    bvector<Utf8String> schemas = server.GetSchemaNames(repos[0]);

    for (Utf8String schema : schemas)
        std::cout << schema << std::endl;

    getch();

    bvector<Utf8String> classes = server.GetClassNames(repos[0], schemas[0]);

    for (Utf8String classname : classes)
        std::cout << classname << std::endl;

    getch();

    Utf8String classJSON = server.GetJSONClassDefinition(repos[0], schemas[0], classes[0]);

    std::cout << classJSON << std::endl;

    getch();

    return 0;
    }


