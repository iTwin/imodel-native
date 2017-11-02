/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebTestsHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"

#include <Bentley/BeVersion.h>
#include <BeHttp/HttpClient.h>

#include "MockHttpHandler.h"
#include "BaseMockHttpHandlerTest.h"

USING_NAMESPACE_BENTLEY_HTTP

BEGIN_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE

Request StubHttpRequest(Utf8StringCR url, Utf8StringCR method, Utf8StringCR body, const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String> ());
Request StubHttpGetRequest(Utf8StringCR url, const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String> ());

Response StubHttpResponse(ConnectionStatus status = ConnectionStatus::CouldNotConnect);
Response StubHttpResponse(HttpStatus httpStatus, Utf8StringCR body = "", const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String> ());
Response StubHttpResponse(HttpStatus httpStatus, HttpBodyPtr body, const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String> ());
Response StubJsonHttpResponse(HttpStatus httpStatus, Utf8StringCR body = "", const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String> ());
Response StubHttpResponseWithUrl(HttpStatus httpStatus, Utf8StringCR url);

void WriteStringToHttpBody(Utf8StringCR string, HttpBodyPtr body);
Utf8String ReadHttpBody(HttpBodyPtr body);

END_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE