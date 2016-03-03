/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/ConnectTestsHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../../Utils/WebServicesTestsHelper.h"
#include <MobileDgn/Utils/Http/HttpClient.h>
#include <WebServices/Connect/SamlToken.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

Utf8String StubSamlTokenXML(uint32_t validMinutes = 0, Utf8StringCR stubCertificate = "TestCert", const std::map<Utf8String, Utf8String>& attributes = {});
SamlTokenPtr StubSamlToken(uint32_t validMinutes = 0);
SamlTokenPtr StubSamlToken(const std::map<Utf8String, Utf8String>& attributes);
HttpResponse StubImsTokenHttpResponse (uint32_t validMinutes = 0);
HttpResponse StubImsTokenHttpResponse (SamlTokenCR token);
