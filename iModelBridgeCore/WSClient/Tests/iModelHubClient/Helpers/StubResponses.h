/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/StubResponses.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "MockHttpHandler.h"
#include <BeHttp\HttpClient.h>

Http::Response StubHttpResponse(Http::HttpStatus httpStatus, Http::HttpBodyPtr body, const std::map<Utf8String, Utf8String>& headers = std::map<Utf8String, Utf8String>());
Http::Response StubHttpResponse(Http::HttpStatus httpStatus, Utf8StringCR body = "", const std::map<Utf8String, Utf8String>& headers = std::map<Utf8String, Utf8String>());
Http::Response SuccessResponse(Http::RequestCR request);
Http::Response CreatedResponse(Http::RequestCR request);
Http::Response EmptyInstancesResponse(Http::RequestCR request);
Http::Response TimeoutResponse(Http::RequestCR request);
Http::Response ServerErrorResponse(Http::RequestCR request);
Http::Response BadRequestResponse(Http::RequestCR request);