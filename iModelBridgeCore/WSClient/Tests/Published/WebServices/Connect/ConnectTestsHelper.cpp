/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Connect/ConnectTestsHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConnectTestsHelper.h"

Utf8String StubSamlTokenXML (uint32_t validMinutes, Utf8StringCR stubCertificate)
    {
    BentleyStatus status;
    DateTime notBeforeDate = DateTime::GetCurrentTimeUtc ();

    int64_t unixMs = 0;
    status = notBeforeDate.ToUnixMilliseconds (unixMs);
    EXPECT_EQ (SUCCESS, status);

    DateTime notOnOrAfterDate;
    status = DateTime::FromUnixMilliseconds (notOnOrAfterDate, unixMs + validMinutes * 60 * 1000);
    EXPECT_EQ (SUCCESS, status);

    Utf8PrintfString token
        (
        R"(<saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion">
            <saml:Conditions 
                NotBefore   ="%s"
                NotOnOrAfter="%s">
            </saml:Conditions>
            <ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#">
                <KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#">
                    <X509Data>
                        <X509Certificate>%s</X509Certificate>
                    </X509Data>
                </KeyInfo>
            </ds:Signature>
        </saml:Assertion>)",
        Utf8String (notBeforeDate.ToString ()).c_str (),
        Utf8String (notOnOrAfterDate.ToString ()).c_str (),
        stubCertificate.c_str ()
        );

    return token;
    }

SamlTokenPtr StubSamlToken (uint32_t validMinutes)
    {
    auto token = std::make_shared<SamlToken> (StubSamlTokenXML (validMinutes));
    EXPECT_FALSE (token->AsString ().empty ());
    return token;
    }

HttpResponse StubImsTokenHttpResponse (uint32_t validMinutes)
    {
    Json::Value authBody;
    authBody["RequestedSecurityToken"] = StubSamlToken (validMinutes)->AsString ();
    return StubHttpResponse (HttpStatus::OK, authBody.toStyledString ());
    }

HttpResponse StubImsTokenHttpResponse (SamlTokenCR token)
    {
    Json::Value authBody;
    authBody["RequestedSecurityToken"] = token.AsString ();
    return StubHttpResponse (HttpStatus::OK, authBody.toStyledString ());
    }