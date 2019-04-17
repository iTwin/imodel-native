/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

#define TEST_URL_UNSAFE_CHARS           R"(http://httpbin.bentley.com:80/ip?<>"##{}|\^[]`)"
#define TEST_URL_UNSAFE_CHARS_ESCAPED   R"(http://httpbin.bentley.com:80/ip?%3C%3E%22#%23%7B%7D%7C%5C%5E%5B%5D%60)"

#define TEST_URL_UNSAFE_CHARS_NO_FRAGMENT           R"(http://httpbin.bentley.com:80/ip?<>"{}|\^[]`)"
#define TEST_URL_UNSAFE_CHARS_NO_FRAGMENT_ESCAPED   R"(http://httpbin.bentley.com:80/ip?%3C%3E%22%7B%7D%7C%5C%5E%5B%5D%60)"

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