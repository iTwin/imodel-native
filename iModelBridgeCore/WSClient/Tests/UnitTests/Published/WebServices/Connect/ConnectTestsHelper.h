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

Utf8String StubSamlTokenXML (uint32_t validMinutes = 0, Utf8StringCR stubCertificate = "TestCert");
SamlTokenPtr StubSamlToken (uint32_t validMinutes = 0);
HttpResponse StubImsTokenHttpResponse (uint32_t validMinutes = 0);
HttpResponse StubImsTokenHttpResponse (SamlTokenCR token);

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
bool operator==(const SamlToken& a, const SamlToken& b);
END_BENTLEY_WEBSERVICES_NAMESPACE