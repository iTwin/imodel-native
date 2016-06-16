/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebTestsHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeVersion.h>
#include <BeHttp/HttpClient.h>

#include "MockHttpHandler.h"
#include "BaseMockHttpHandlerTest.h"
#include "TestsHelper.h"
#include "ValuePrinter.h"
USING_NAMESPACE_BENTLEY_HTTP

BEGIN_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE

HttpRequest StubHttpRequest (Utf8StringCR url, Utf8StringCR method, Utf8StringCR body, const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String> ());
HttpRequest StubHttpGetRequest (Utf8StringCR url, const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String> ());

HttpResponse StubHttpResponse (ConnectionStatus status = ConnectionStatus::CouldNotConnect);
HttpResponse StubHttpResponse (HttpStatus httpStatus, Utf8StringCR body = "", const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String> ());
HttpResponse StubHttpResponse (HttpStatus httpStatus, HttpBodyPtr body, const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String> ());
HttpResponse StubJsonHttpResponse (HttpStatus httpStatus, Utf8StringCR body = "", const bmap<Utf8String, Utf8String>& headers = bmap<Utf8String, Utf8String> ());
HttpResponse StubHttpResponseWithUrl (HttpStatus httpStatus, Utf8StringCR url);

void WriteStringToHttpBody (Utf8StringCR string, HttpBodyPtr body);
Utf8String ReadHttpBody (HttpBodyPtr body);

END_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE