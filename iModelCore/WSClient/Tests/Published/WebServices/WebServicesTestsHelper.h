/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/WebServicesTestsHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <WebServices/WSError.h>
#include <WebServices/WSRepositoryClient.h>
#include <WebServices/ObjectId.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

HttpResponse StubHttpResponse (ConnectionStatus status = ConnectionStatus::CouldNotConnect);
HttpResponse StubHttpResponse (HttpStatus httpStatus, Utf8StringCR body = "", const std::map<Utf8String, Utf8String>& headers = {});
HttpResponse StubHttpResponse (HttpStatus httpStatus, HttpBodyPtr body, const std::map<Utf8String, Utf8String>& headers = {});
HttpResponse StubJsonHttpResponse (HttpStatus httpStatus, Utf8StringCR body = "", const std::map<Utf8String, Utf8String>& headers = {});
HttpResponse StubHttpResponseWithUrl (HttpStatus httpStatus, Utf8StringCR url);

void WriteStringToHttpBody (Utf8StringCR string, HttpBodyPtr body);
Utf8String ReadHttpBody (HttpBodyPtr body);

WSInfo StubWSInfo (BeVersion version = BeVersion (1, 2), WSInfo::Type type = WSInfo::Type::BentleyWSG);

HttpResponse StubWSErrorHttpResponse (HttpStatus status, Utf8StringCR errorId, Utf8StringCR message = "", Utf8StringCR description = ""); 
HttpResponse StubWSInfoHttpResponseV1 ();
HttpResponse StubWSInfoHttpResponseV1BentleyConnect ();
HttpResponse StubWSInfoHttpResponseV2 ();
HttpResponse StubWSInfoHttpResponse (BeVersion serverVersion);

WSError StubWSConnectionError ();
WSError StubWSCanceledError ();