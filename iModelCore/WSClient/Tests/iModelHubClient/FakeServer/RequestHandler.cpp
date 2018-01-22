#include "RequestHandler.h"
#include <Bentley/BeTest.h>
#include "../../../iModelHubClient/Utils.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

RequestHandler::RequestHandler()
    {
    BeFileName outPath;
    BeTest::GetHost().GetOutputRoot(outPath);
    outPath.AppendToPath(L"Server");
    serverPath = outPath.GetWCharCP();
    }

RequestHandler::~RequestHandler()
    {
    }

bvector<Utf8String> ParseUrl(Request req) 
    {
    Utf8String requestUrl = req.GetUrl();
    bvector<Utf8String> tokens;
    Utf8CP url = requestUrl.c_str();
    BeStringUtilities::Split(url, "/", nullptr, tokens);
    return tokens;
    }

Response ProjectSchemaClassiModelWithQuery()
    {
    /*Utf8CP url = "asd";
    HttpResponseContentPtr respContent;
    HttpBodyPtr body;
    respContent->Create(body);
    Response resp(respContent, url, ConnectionStatus::OK, HttpStatus::OK);*/
    Response resp;
    return resp;
    }

Response ProjectSchemaClassiModel()
    {
    Utf8CP url = "asd";
    HttpResponseContentPtr respContent;
    HttpBodyPtr body;
    respContent->Create(body);
    Response resp(respContent, url, ConnectionStatus::OK, HttpStatus::OK);
    //Response resp;
    return resp;
    }

Response GetProjectSchema(bvector<Utf8String> args) 
    {
    if (args.size() > 7)
        return ProjectSchemaClassiModelWithQuery();
    return ProjectSchemaClassiModel();
    }

Response RequestHandler::PerformGetRequest(Request req)
    {
    //identify the type of request
    // do the specific operation by passing to the concerned function
    // build up the response according to the action taken
    // return response
    bvector<Utf8String> args = ParseUrl(req);
    //get request with Project Schema
    Response resp;
    if (args[5] == ServerSchema::Schema::Project)
        resp = GetProjectSchema(args);

    //get request with iModel--Nr
    return resp;
    }

Response RequestHandler::CreateiModel(Request req)
    {
    HttpBodyPtr reqBody = req.GetRequestBody();
    char readBuff[1000] ;
    size_t buffSize = 100000;
    reqBody->Read(readBuff, buffSize);
    Utf8String reqBodyRead(readBuff);
    Json::Reader reader;
    Json::Value settings;

    Response resp;
    if (!reader.Parse(reqBodyRead, settings))
        return resp;

    Utf8String fileName = settings["instance"]["properties"]["Name"].asString();
    BeFileName fileToCreate(fileName);
    FakeServer::CreateiModel(serverPath, fileToCreate.GetWCharCP());
    return resp;
    }

Response RequestHandler::PerformOtherRequest(Request req)
    {
    RequestHandler::CreateiModel(req);
    Response resp;
    return resp;
    }
