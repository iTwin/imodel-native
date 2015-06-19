/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/WebServicesTestsHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../MobileUtilsTests.h"
#include <Bentley/BeTest.h>
#include <WebServices/Client/WSError.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/Client/ObjectId.h>
#include "Connect/StubLocalState.h"
#include "Configuration/StubBuddiClient.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

HttpResponse StubHttpResponse (ConnectionStatus status = ConnectionStatus::CouldNotConnect);
HttpResponse StubHttpResponse (HttpStatus httpStatus, Utf8StringCR body = "", const std::map<Utf8String, Utf8String>& headers = std::map<Utf8String, Utf8String> ());
HttpResponse StubHttpResponse (HttpStatus httpStatus, HttpBodyPtr body, const std::map<Utf8String, Utf8String>& headers = std::map<Utf8String, Utf8String> ());
HttpResponse StubJsonHttpResponse (HttpStatus httpStatus, Utf8StringCR body = "", const std::map<Utf8String, Utf8String>& headers = std::map<Utf8String, Utf8String> ());
HttpResponse StubHttpResponseWithUrl (HttpStatus httpStatus, Utf8StringCR url);

WSInfo StubWSInfoWebApi (BeVersion webApiVersion = BeVersion (1, 3), WSInfo::Type type = WSInfo::Type::BentleyWSG);
//! Stub WebApi 1.1 and BentleyConnect server
HttpResponse StubWSInfoHttpResponseBentleyConnectV1 ();
//! Stub WebApi 1.1 and BWSG server
HttpResponse StubWSInfoHttpResponseWebApi11 ();
//! Stub WebApi 1.2 and BWSG server
HttpResponse StubWSInfoHttpResponseWebApi12 ();
//! Stub WebApi 1.3 and BWSG server. Default for testing WSG 1.x client code
HttpResponse StubWSInfoHttpResponseWebApi13 ();
//! Stub WebApi 2.0 and BWSG server
HttpResponse StubWSInfoHttpResponseWebApi20 ();
//! Stub WebApi 2.1 and BWSG server
HttpResponse StubWSInfoHttpResponseWebApi21 ();
//! Stub WebApi 2.2 and BWSG server
HttpResponse StubWSInfoHttpResponseWebApi22 ();
//! Stub WebApi version and BWSG server
HttpResponse StubWSInfoHttpResponseWebApi (BeVersion webApiVersion);

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

ClientInfoPtr StubClientInfo ();