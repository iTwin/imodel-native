/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformTools/Example/RealityDataServiceErrorReportingExample.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatformTools/RealityDataService.h>
#include <curl/curl.h>

#include <stdio.h>
#include <conio.h>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

static void errorCallback(Utf8String basicMessage, const RawServerResponse& rawResponse)
    {
    //std::cout << basicMessage << std::endl;
    std::cout << "Request failed with the following response:\n" << std::endl;
    std::cout << Utf8PrintfString("response header:\n %s\n", rawResponse.header.c_str()) << std::endl;
    std::cout << Utf8PrintfString("response body:\n %s\n", rawResponse.body.c_str()) << std::endl;
    std::cout << Utf8PrintfString("response code: %lu\n", rawResponse.responseCode) << std::endl;
    std::cout << Utf8PrintfString("response toolCode: %d - %s\n", rawResponse.toolCode, curl_easy_strerror((CURLcode)(rawResponse.toolCode))) << std::endl;
    }

/*-----------------------------------------------------------------//
* Bentley RealityDataServiceExample
* This application uses hard coded values and is not guaranteed to 
* function properly.
* The purpose of its existence is only to show how to structure
* and execute RealityDataService operations
//----------------------------------------------------------------*/
int main(int argc, char *argv[])
    {
    RealityDataService::SetServerComponents("dev-realitydataservices-eus.cloudapp.net", "2.4", "S3MXECPlugin--Server", "S3MX");
    RealityDataService::SetProjectId("72524420-7d48-4f4e-8b0f-144e5fa0aa22");
    RealityDataService::SetErrorCallback(errorCallback);
    
    
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    properties.Insert(RealityDataField::Name, "ERROR TEST");

    properties.Insert(RealityDataField::Classification, "BAD SETTING");

    properties.Insert(RealityDataField::Type, "BAD SETTING");

    properties.Insert(RealityDataField::Visibility, "BAD SETTING");

    RealityDataCreateRequest createRequest = RealityDataCreateRequest("", RealityDataServiceUpload::PackageProperties(properties));

    RawServerResponse createResponse = RawServerResponse();
    Utf8String response = RealityDataService::Request(createRequest, createResponse);
    
    getch();
    return 0;
    }