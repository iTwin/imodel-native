/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/ConnectTestsHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConnectTestsHelper.h"

Utf8String StubSamlTokenXML(uint32_t validMinutes, Utf8StringCR stubCertificate, const std::map<Utf8String, Utf8String>& attributes)
    {
    BentleyStatus status;
    DateTime notBeforeDate = DateTime::GetCurrentTimeUtc();

    int64_t unixMs = 0;
    status = notBeforeDate.ToUnixMilliseconds(unixMs);
    EXPECT_EQ(SUCCESS, status);

    DateTime notOnOrAfterDate;
    status = DateTime::FromUnixMilliseconds(notOnOrAfterDate, unixMs + validMinutes * 60 * 1000);
    EXPECT_EQ(SUCCESS, status);

    Utf8String attributesStr;
    for (auto pair : attributes)
        {
        attributesStr += Utf8PrintfString(
            R"(
                <saml:Attribute AttributeName="%s">
                    <saml:AttributeValue>%s</saml:AttributeValue>
                </saml:Attribute>)",
        pair.first.c_str(),
        pair.second.c_str());
        }

    Utf8PrintfString tokenStr
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
            <saml:AttributeStatement>
                <saml:Attribute AttributeName="unique-test-attribute">
                    <saml:AttributeValue>%s</saml:AttributeValue>
                </saml:Attribute>%s
            </saml:AttributeStatement>
        </saml:Assertion>)",
        Utf8String(notBeforeDate.ToString()).c_str(),
        Utf8String(notOnOrAfterDate.ToString()).c_str(),
        stubCertificate.c_str(),
        BeGuid(true).ToString().c_str(),
        attributesStr.c_str()
        );

    return tokenStr;
    }

SamlTokenPtr StubSamlToken(uint32_t validMinutes)
    {
    auto token = std::make_shared<SamlToken>(StubSamlTokenXML(validMinutes, "TestCert", {{"name", "TestUser"}}));
    EXPECT_FALSE(token->AsString().empty());
    return token;
    }

SamlTokenPtr StubSamlToken(const std::map<Utf8String, Utf8String>& attributes)
    {
    auto token = std::make_shared<SamlToken>(StubSamlTokenXML(10000, "TestCert", attributes));
    EXPECT_FALSE(token->AsString().empty());
    return token;
    }

Response StubImsTokenHttpResponse(uint32_t validMinutes)
    {
    Json::Value authBody;
    authBody["RequestedSecurityToken"] = StubSamlToken(validMinutes)->AsString();
    return StubHttpResponse(HttpStatus::OK, authBody.toStyledString());
    }

Response StubImsTokenHttpResponse(SamlTokenCR token)
    {
    Json::Value authBody;
    authBody["RequestedSecurityToken"] = token.AsString();
    return StubHttpResponse(HttpStatus::OK, authBody.toStyledString());
    }

SamlTokenPtr StubSamlTokenWithUser(Utf8StringCR username)
    {
    auto token = std::make_shared<SamlToken>(StubSamlTokenXML(100000, "TestCert", {{"name", username}}));
    EXPECT_FALSE(token->AsString().empty());
    return token;
    }
