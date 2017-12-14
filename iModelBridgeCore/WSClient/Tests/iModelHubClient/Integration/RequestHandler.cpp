#include "RequestHandler.h"



USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

RequestHandler::RequestHandler()
    {
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
Response CreateiModel(Request req)
    {
    printf("here in Post\n");
    Response resp;
    return resp;
    }
Response RequestHandler::PerformOtherRequest(Request req)
    {
    CreateiModel(req);
    Response resp;
    return resp;
    }