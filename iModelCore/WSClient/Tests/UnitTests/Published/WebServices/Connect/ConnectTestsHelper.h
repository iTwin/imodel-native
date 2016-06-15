/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/ConnectTestsHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../../Utils/WebServicesTestsHelper.h"
#include <BeHttp/HttpClient.h>
#include <WebServices/Connect/SamlToken.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

Utf8String StubSamlTokenXML(uint32_t validMinutes = 10000, Utf8StringCR stubCertificate = "TestCert", const std::map<Utf8String, Utf8String>& attributes = {});
SamlTokenPtr StubSamlToken(uint32_t validMinutes = 10000);
SamlTokenPtr StubSamlToken(const std::map<Utf8String, Utf8String>& attributes);
HttpResponse StubImsTokenHttpResponse (uint32_t validMinutes = 10000);
HttpResponse StubImsTokenHttpResponse (SamlTokenCR token);
