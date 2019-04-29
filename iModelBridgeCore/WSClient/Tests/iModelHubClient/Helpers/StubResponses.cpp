/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "StubResponses.h"

USING_NAMESPACE_BENTLEY_HTTP

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
Response StubHttpResponse(HttpStatus httpStatus, HttpBodyPtr body, const std::map<Utf8String, Utf8String>& headers)
    {
    ConnectionStatus status = ConnectionStatus::OK;
    if (httpStatus == HttpStatus::None)
        {
        status = ConnectionStatus::CouldNotConnect;
        }
    auto content = HttpResponseContent::Create(body);
    for (const auto& header : headers)
        {
        content->GetHeaders().SetValue(header.first, header.second);
        }
    return Response(content, "", status, httpStatus);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
Response StubHttpResponse(HttpStatus httpStatus, Utf8StringCR body, const std::map<Utf8String, Utf8String>& headers)
    {
    return StubHttpResponse(httpStatus, HttpStringBody::Create(body), headers);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Response SuccessResponse(RequestCR request)
    {
    return StubHttpResponse(HttpStatus::OK, "", { { "Server" , "Bentley-WebAPI/2.4, Bentley-WSG/9.99.00.00" } });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Response CreatedResponse(RequestCR request)
    {
    return StubHttpResponse(HttpStatus::Created, "{\"changedInstance\": {\"instanceAfterChange\": {\"instanceId\": \"instanceId\", \"relationshipInstances\": [{\"relatedInstance\": {\"schemaName\": \"iModelScope\",\"className\": \"AccessKey\",\"properties\": {\"UploadUrl\": \"http://upload.url\",\"DownloadUrl\": \"http://download.url\"}}}]}}}");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Response EmptyInstancesResponse(RequestCR request)
    {
    return StubHttpResponse(HttpStatus::OK, "{instances: {}}");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Response TimeoutResponse(RequestCR request)
    {
    return StubHttpResponse(HttpStatus::ReqestTimeout,
        "{\"errorId\": \"\", "
        "\"errorMessage\" : \"Request timed out\", "
        "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Response ServerErrorResponse(RequestCR request)
    {
    return StubHttpResponse(HttpStatus::InternalServerError);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Response BadRequestResponse(RequestCR request)
    {
    return StubHttpResponse(HttpStatus::BadRequest);
    }