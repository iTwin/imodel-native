/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Connect/DelegationTokenProviderTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServicesTestsHelper.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ImsClient.h>
#include <BeHttp/ProxyHttpHandler.h>

#include "../../../Connect/DelegationTokenProvider.h"
#include "../../UnitTests/Published/WebServices/Connect/StubTokenStore.h"
#include "../../UnitTests/Published/WebServices/Connect/MockConnectTokenProvider.h"

struct DelegationTokenProviderTests : public WSClientBaseTest {};

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DelegationTokenProviderTests, UpdateToken_ValidParentToken_GetsNewToken)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &localState, nullptr, proxy);

    auto imsClient = ImsClient::Create(StubValidClientInfo(), proxy);
    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");
    auto result = imsClient->RequestToken(credentials, ImsClient::GetLegacyRelyingPartyUri(), 1)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    SamlTokenPtr initialToken = result.GetValue();

    auto parentProvider = std::make_shared<MockConnectTokenProvider>();
    EXPECT_CALL(*parentProvider, GetToken()).WillOnce(Return(initialToken));

    auto provider = std::make_shared<DelegationTokenProvider>(imsClient, ImsClient::GetLegacyRelyingPartyUri(), parentProvider);

    auto token1 = provider->GetToken();
    ASSERT_TRUE(token1 == nullptr);

    auto token2 = provider->UpdateToken()->GetResult();
    EXPECT_NE(token2, token1);
    ASSERT_TRUE(token2 != nullptr);

    auto token3 = provider->GetToken();
    EXPECT_EQ(token3, token2);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DelegationTokenProviderTests, UpdateToken_ExpiredParentTokenAndParentTokenIsUpdatable_SuccessAndReturnsNewToken)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &localState, nullptr, proxy);
    
    auto imsClient = ImsClient::Create(StubValidClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");
    auto result = imsClient->RequestToken(credentials, ImsClient::GetLegacyRelyingPartyUri(), 1)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    SamlTokenPtr validToken = result.GetValue();

    auto tokenStr = R"xml(
<saml:Assertion MajorVersion="1" MinorVersion="1" AssertionID="_110c9538-9aff-4ddc-9e9a-5e90992f3ef9" Issuer="https://ims.bentley.com/" IssueInstant="2018-11-22T08:36:23.224Z" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion"><saml:Conditions NotBefore="2018-11-22T08:36:23.224Z" NotOnOrAfter="2018-11-22T08:37:23.224Z"><saml:AudienceRestrictionCondition><saml:Audience>https://connect-wsg20.bentley.com</saml:Audience></saml:AudienceRestrictionCondition></saml:Conditions><saml:AttributeStatement><saml:Subject><saml:NameIdentifier>33ad56b2-f424-49f5-b69b-70d2fdd41b00</saml:NameIdentifier><saml:SubjectConfirmation><saml:ConfirmationMethod>urn:oasis:names:tc:SAML:1.0:cm:holder-of-key</saml:ConfirmationMethod><KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#"><trust:BinarySecret xmlns:trust="http://docs.oasis-open.org/ws-sx/ws-trust/200512">vI1bU/c91UrjcXaWXar0soWse0sC9VBICP2xt+Ilnx4=</trust:BinarySecret></KeyInfo></saml:SubjectConfirmation></saml:Subject><saml:Attribute AttributeName="name" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>8cc45bd041514b58947ea6c09c@gmail.com</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="givenname" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>John</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="surname" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>Stevenson</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="emailaddress" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>8cc45bd041514b58947ea6c09c@gmail.com</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="sapbupa" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>0003796640</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="countryiso" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>US</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="languageiso" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>EN</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="ismarketingprospect" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>true</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="isbentleyemployee" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>false</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="becommunitiesusername" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>33AD56B2-F424-49F5-B69B-70D2FDD41B00</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="becommunitiesemailaddress" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>8cc45bd041514b58947ea6c09c@gmail.com</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="userid" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>33ad56b2-f424-49f5-b69b-70d2fdd41b00</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="has_select" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>false</saml:AttributeValue></saml:Attribute></saml:AttributeStatement><ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#"><ds:SignedInfo><ds:CanonicalizationMethod Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#" /><ds:SignatureMethod Algorithm="http://www.w3.org/2001/04/xmldsig-more#rsa-sha256" /><ds:Reference URI="#_110c9538-9aff-4ddc-9e9a-5e90992f3ef9"><ds:Transforms><ds:Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature" /><ds:Transform Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#" /></ds:Transforms><ds:DigestMethod Algorithm="http://www.w3.org/2001/04/xmlenc#sha256" /><ds:DigestValue>uaxptgBgCnwPYXFO+lQfKACigm45Af4xVH8bhfLp5w0=</ds:DigestValue></ds:Reference></ds:SignedInfo><ds:SignatureValue>KGIGmQcIguXMdsg+B05xQUbxPCeVJWgPm547uioWAilt/9MoZAHtP1jQ0thSDwM0T0pOavdiKYIE0VnW0Z5sb225Teg3ezB5VDU8rbsoIaT4g3ZzKvgJlR4jUQiOwb6tmpgtdDWH025OJkqKQHG8PWxgJwG5c4AQaykPaLQ0cu+YE4BqMQxYlWY0jeGlX4Hwx8GJI66AbJ/NHm2Zy0FdAJfaMELUU7xhTCzjks0yyz1x/JQuSYUt/31BKqxAyQeu/sR3mxZY2qlBSyfanJZN5AEZ7dC7VkSV4rLeftesl6rEfvktOmlUzwuHLp0er0rZrIljEHZ14FK9EMncYJ/x/Q==</ds:SignatureValue><KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#"><X509Data><X509Certificate>MIIFTzCCBDegAwIBAgIQBu32LObIZLQ0C8lW0d+QzjANBgkqhkiG9w0BAQsFADBNMQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMScwJQYDVQQDEx5EaWdpQ2VydCBTSEEyIFNlY3VyZSBTZXJ2ZXIgQ0EwHhcNMTcwOTAxMDAwMDAwWhcNMTkxMTI3MTIwMDAwWjCBkTELMAkGA1UEBhMCVVMxFTATBgNVBAgTDFBlbm5zeWx2YW5pYTEOMAwGA1UEBxMFRXh0b24xJjAkBgNVBAoTHUJlbnRsZXkgU3lzdGVtcywgSW5jb3Jwb3JhdGVkMQswCQYDVQQLEwJJVDEmMCQGA1UEAxMdaW1zLXRva2VuLXNpZ25pbmcuYmVudGxleS5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDgYLZczB8Xi1LnkzKquDH5tlR0m1BoifgKBs2HfentwrKFfmCiVxNDZgDpAovLUhBsrniYrRleJIrGRIn5Hd9VkiZxLMWCc8zST6bLGpCc4LFP7R6Wnqf6j1eQCIFAqV9FKoBOTgAKbQUQJnvwVbLD/BO3YOWm8XCy0wvuTJMn/rYo4BZEEyzukoppWiPeKRfYr3if+Koj/f0C8piBNLr3+RoNh50yfaC+v3+opctIJzp20iNn+jUt8CMEEKihRbQOYxRww8aGD4A9dbrCVz8zSNqMTl1cUoWAJkLJeDtCc8U2N4kV7S+FwBR8r9uVW2UnUJXQwv2ZLKJdnl3hbltDAgMBAAGjggHkMIIB4DAfBgNVHSMEGDAWgBQPgGEcgjFh1S8o541GOLQs4cbZ4jAdBgNVHQ4EFgQUGWYxPyDohu1zEgCg6VXLdoLd3kQwKAYDVR0RBCEwH4IdaW1zLXRva2VuLXNpZ25pbmcuYmVudGxleS5jb20wDgYDVR0PAQH/BAQDAgWgMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjBrBgNVHR8EZDBiMC+gLaArhilodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vc3NjYS1zaGEyLWcxLmNybDAvoC2gK4YpaHR0cDovL2NybDQuZGlnaWNlcnQuY29tL3NzY2Etc2hhMi1nMS5jcmwwTAYDVR0gBEUwQzA3BglghkgBhv1sAQEwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29tL0NQUzAIBgZngQwBAgIwfAYIKwYBBQUHAQEEcDBuMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5jb20wRgYIKwYBBQUHMAKGOmh0dHA6Ly9jYWNlcnRzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydFNIQTJTZWN1cmVTZXJ2ZXJDQS5jcnQwDAYDVR0TAQH/BAIwADANBgkqhkiG9w0BAQsFAAOCAQEASGipTbF15V5C7MCo7udTP81Hc2jswF6vLoA3NLVmM1BiPWGxVdAo0TfNlxoBoZFQ1nYQCy5tNfVzBell8xxrguhUU+Vnv59YLWrIkeAWALMCHOJ34Eec8ki655TlNlMttO36VuUtTGQPfxjDfFVhIqWCgvoh29JEtfMndZYmeu0IqNuFGWODtaeSfViBjfoGRDbnCoMOjt1coZLuvSV5+ECAXJ4UJxP3ch7y4y5RG5qbDB58oD4CIpb9yw0r+U2cip+ywemPGfKUYJ1MGVKNbu5/+eelkUzGPqJd+GMbA4xrGyEz0VRELAYgPP6B1vrmgCmpEyrjC4XNohVFjowQ9g==</X509Certificate></X509Data></KeyInfo></ds:Signature></saml:Assertion>
)xml";
    SamlTokenPtr initialToken = std::make_shared<SamlToken>(tokenStr);

    auto parentProvider = std::make_shared<MockConnectTokenProvider>();

    InSequence seq;
    EXPECT_CALL(*parentProvider, GetToken()).WillOnce(Return(initialToken));
    EXPECT_CALL(*parentProvider, UpdateToken()).WillOnce(Return(CreateCompletedAsyncTask<ISecurityTokenPtr>(validToken)));
    EXPECT_CALL(*parentProvider, GetToken()).WillOnce(Return(validToken));

    auto provider = std::make_shared<DelegationTokenProvider>(imsClient, ImsClient::GetLegacyRelyingPartyUri(), parentProvider);

    auto token1 = provider->GetToken();
    ASSERT_TRUE(token1 == nullptr);

    auto token2 = provider->UpdateToken()->GetResult();
    EXPECT_NE(token2, token1);
    ASSERT_TRUE(token2 != nullptr);

    auto token3 = provider->GetToken();
    EXPECT_EQ(token3, token2);
    }


/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DelegationTokenProviderTests, UpdateToken_ExpiredParentTokenAndParentTokenUpdateReturnsInvalid_ReturnsNull)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &localState, nullptr, proxy);

    auto imsClient = ImsClient::Create(StubValidClientInfo(), proxy);

    //Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");
    auto tokenStr = R"xml(
<saml:Assertion MajorVersion="1" MinorVersion="1" AssertionID="_110c9538-9aff-4ddc-9e9a-5e90992f3ef9" Issuer="https://ims.bentley.com/" IssueInstant="2018-11-22T08:36:23.224Z" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion"><saml:Conditions NotBefore="2018-11-22T08:36:23.224Z" NotOnOrAfter="2018-11-22T08:37:23.224Z"><saml:AudienceRestrictionCondition><saml:Audience>https://connect-wsg20.bentley.com</saml:Audience></saml:AudienceRestrictionCondition></saml:Conditions><saml:AttributeStatement><saml:Subject><saml:NameIdentifier>33ad56b2-f424-49f5-b69b-70d2fdd41b00</saml:NameIdentifier><saml:SubjectConfirmation><saml:ConfirmationMethod>urn:oasis:names:tc:SAML:1.0:cm:holder-of-key</saml:ConfirmationMethod><KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#"><trust:BinarySecret xmlns:trust="http://docs.oasis-open.org/ws-sx/ws-trust/200512">vI1bU/c91UrjcXaWXar0soWse0sC9VBICP2xt+Ilnx4=</trust:BinarySecret></KeyInfo></saml:SubjectConfirmation></saml:Subject><saml:Attribute AttributeName="name" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>8cc45bd041514b58947ea6c09c@gmail.com</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="givenname" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>John</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="surname" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>Stevenson</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="emailaddress" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>8cc45bd041514b58947ea6c09c@gmail.com</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="sapbupa" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>0003796640</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="countryiso" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>US</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="languageiso" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>EN</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="ismarketingprospect" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>true</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="isbentleyemployee" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>false</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="becommunitiesusername" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>33AD56B2-F424-49F5-B69B-70D2FDD41B00</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="becommunitiesemailaddress" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>8cc45bd041514b58947ea6c09c@gmail.com</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="userid" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>33ad56b2-f424-49f5-b69b-70d2fdd41b00</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="has_select" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>false</saml:AttributeValue></saml:Attribute></saml:AttributeStatement><ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#"><ds:SignedInfo><ds:CanonicalizationMethod Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#" /><ds:SignatureMethod Algorithm="http://www.w3.org/2001/04/xmldsig-more#rsa-sha256" /><ds:Reference URI="#_110c9538-9aff-4ddc-9e9a-5e90992f3ef9"><ds:Transforms><ds:Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature" /><ds:Transform Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#" /></ds:Transforms><ds:DigestMethod Algorithm="http://www.w3.org/2001/04/xmlenc#sha256" /><ds:DigestValue>uaxptgBgCnwPYXFO+lQfKACigm45Af4xVH8bhfLp5w0=</ds:DigestValue></ds:Reference></ds:SignedInfo><ds:SignatureValue>KGIGmQcIguXMdsg+B05xQUbxPCeVJWgPm547uioWAilt/9MoZAHtP1jQ0thSDwM0T0pOavdiKYIE0VnW0Z5sb225Teg3ezB5VDU8rbsoIaT4g3ZzKvgJlR4jUQiOwb6tmpgtdDWH025OJkqKQHG8PWxgJwG5c4AQaykPaLQ0cu+YE4BqMQxYlWY0jeGlX4Hwx8GJI66AbJ/NHm2Zy0FdAJfaMELUU7xhTCzjks0yyz1x/JQuSYUt/31BKqxAyQeu/sR3mxZY2qlBSyfanJZN5AEZ7dC7VkSV4rLeftesl6rEfvktOmlUzwuHLp0er0rZrIljEHZ14FK9EMncYJ/x/Q==</ds:SignatureValue><KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#"><X509Data><X509Certificate>MIIFTzCCBDegAwIBAgIQBu32LObIZLQ0C8lW0d+QzjANBgkqhkiG9w0BAQsFADBNMQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMScwJQYDVQQDEx5EaWdpQ2VydCBTSEEyIFNlY3VyZSBTZXJ2ZXIgQ0EwHhcNMTcwOTAxMDAwMDAwWhcNMTkxMTI3MTIwMDAwWjCBkTELMAkGA1UEBhMCVVMxFTATBgNVBAgTDFBlbm5zeWx2YW5pYTEOMAwGA1UEBxMFRXh0b24xJjAkBgNVBAoTHUJlbnRsZXkgU3lzdGVtcywgSW5jb3Jwb3JhdGVkMQswCQYDVQQLEwJJVDEmMCQGA1UEAxMdaW1zLXRva2VuLXNpZ25pbmcuYmVudGxleS5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDgYLZczB8Xi1LnkzKquDH5tlR0m1BoifgKBs2HfentwrKFfmCiVxNDZgDpAovLUhBsrniYrRleJIrGRIn5Hd9VkiZxLMWCc8zST6bLGpCc4LFP7R6Wnqf6j1eQCIFAqV9FKoBOTgAKbQUQJnvwVbLD/BO3YOWm8XCy0wvuTJMn/rYo4BZEEyzukoppWiPeKRfYr3if+Koj/f0C8piBNLr3+RoNh50yfaC+v3+opctIJzp20iNn+jUt8CMEEKihRbQOYxRww8aGD4A9dbrCVz8zSNqMTl1cUoWAJkLJeDtCc8U2N4kV7S+FwBR8r9uVW2UnUJXQwv2ZLKJdnl3hbltDAgMBAAGjggHkMIIB4DAfBgNVHSMEGDAWgBQPgGEcgjFh1S8o541GOLQs4cbZ4jAdBgNVHQ4EFgQUGWYxPyDohu1zEgCg6VXLdoLd3kQwKAYDVR0RBCEwH4IdaW1zLXRva2VuLXNpZ25pbmcuYmVudGxleS5jb20wDgYDVR0PAQH/BAQDAgWgMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjBrBgNVHR8EZDBiMC+gLaArhilodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vc3NjYS1zaGEyLWcxLmNybDAvoC2gK4YpaHR0cDovL2NybDQuZGlnaWNlcnQuY29tL3NzY2Etc2hhMi1nMS5jcmwwTAYDVR0gBEUwQzA3BglghkgBhv1sAQEwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29tL0NQUzAIBgZngQwBAgIwfAYIKwYBBQUHAQEEcDBuMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5jb20wRgYIKwYBBQUHMAKGOmh0dHA6Ly9jYWNlcnRzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydFNIQTJTZWN1cmVTZXJ2ZXJDQS5jcnQwDAYDVR0TAQH/BAIwADANBgkqhkiG9w0BAQsFAAOCAQEASGipTbF15V5C7MCo7udTP81Hc2jswF6vLoA3NLVmM1BiPWGxVdAo0TfNlxoBoZFQ1nYQCy5tNfVzBell8xxrguhUU+Vnv59YLWrIkeAWALMCHOJ34Eec8ki655TlNlMttO36VuUtTGQPfxjDfFVhIqWCgvoh29JEtfMndZYmeu0IqNuFGWODtaeSfViBjfoGRDbnCoMOjt1coZLuvSV5+ECAXJ4UJxP3ch7y4y5RG5qbDB58oD4CIpb9yw0r+U2cip+ywemPGfKUYJ1MGVKNbu5/+eelkUzGPqJd+GMbA4xrGyEz0VRELAYgPP6B1vrmgCmpEyrjC4XNohVFjowQ9g==</X509Certificate></X509Data></KeyInfo></ds:Signature></saml:Assertion>
)xml";
    SamlTokenPtr initialToken = std::make_shared<SamlToken>(tokenStr);

    auto parentProvider = std::make_shared<MockConnectTokenProvider>();

    InSequence seq;
    EXPECT_CALL(*parentProvider, GetToken()).WillOnce(Return(initialToken));
    EXPECT_CALL(*parentProvider, UpdateToken()).WillOnce(Return(CreateCompletedAsyncTask<ISecurityTokenPtr>(initialToken)));
    EXPECT_CALL(*parentProvider, GetToken()).WillOnce(Return(initialToken));

    auto provider = std::make_shared<DelegationTokenProvider>(imsClient, ImsClient::GetLegacyRelyingPartyUri(), parentProvider);

    auto token1 = provider->GetToken();
    ASSERT_TRUE(token1 == nullptr);

    auto token2 = provider->UpdateToken()->GetResult();
    EXPECT_TRUE(token2 == nullptr);

    auto token3 = provider->GetToken();
    ASSERT_TRUE(token3 == nullptr);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DelegationTokenProviderTests, UpdateToken_ExpiredParentTokenAndParentTokenIsDoesNotUpdate_ReturnsNull)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &localState, nullptr, proxy);

    auto imsClient = ImsClient::Create(StubValidClientInfo(), proxy);

    //Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");
    auto tokenStr = R"xml(
<saml:Assertion MajorVersion="1" MinorVersion="1" AssertionID="_110c9538-9aff-4ddc-9e9a-5e90992f3ef9" Issuer="https://ims.bentley.com/" IssueInstant="2018-11-22T08:36:23.224Z" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion"><saml:Conditions NotBefore="2018-11-22T08:36:23.224Z" NotOnOrAfter="2018-11-22T08:37:23.224Z"><saml:AudienceRestrictionCondition><saml:Audience>https://connect-wsg20.bentley.com</saml:Audience></saml:AudienceRestrictionCondition></saml:Conditions><saml:AttributeStatement><saml:Subject><saml:NameIdentifier>33ad56b2-f424-49f5-b69b-70d2fdd41b00</saml:NameIdentifier><saml:SubjectConfirmation><saml:ConfirmationMethod>urn:oasis:names:tc:SAML:1.0:cm:holder-of-key</saml:ConfirmationMethod><KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#"><trust:BinarySecret xmlns:trust="http://docs.oasis-open.org/ws-sx/ws-trust/200512">vI1bU/c91UrjcXaWXar0soWse0sC9VBICP2xt+Ilnx4=</trust:BinarySecret></KeyInfo></saml:SubjectConfirmation></saml:Subject><saml:Attribute AttributeName="name" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>8cc45bd041514b58947ea6c09c@gmail.com</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="givenname" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>John</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="surname" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>Stevenson</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="emailaddress" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>8cc45bd041514b58947ea6c09c@gmail.com</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="sapbupa" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>0003796640</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="countryiso" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>US</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="languageiso" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>EN</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="ismarketingprospect" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>true</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="isbentleyemployee" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>false</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="becommunitiesusername" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>33AD56B2-F424-49F5-B69B-70D2FDD41B00</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="becommunitiesemailaddress" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>8cc45bd041514b58947ea6c09c@gmail.com</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="userid" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>33ad56b2-f424-49f5-b69b-70d2fdd41b00</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="has_select" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>false</saml:AttributeValue></saml:Attribute></saml:AttributeStatement><ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#"><ds:SignedInfo><ds:CanonicalizationMethod Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#" /><ds:SignatureMethod Algorithm="http://www.w3.org/2001/04/xmldsig-more#rsa-sha256" /><ds:Reference URI="#_110c9538-9aff-4ddc-9e9a-5e90992f3ef9"><ds:Transforms><ds:Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature" /><ds:Transform Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#" /></ds:Transforms><ds:DigestMethod Algorithm="http://www.w3.org/2001/04/xmlenc#sha256" /><ds:DigestValue>uaxptgBgCnwPYXFO+lQfKACigm45Af4xVH8bhfLp5w0=</ds:DigestValue></ds:Reference></ds:SignedInfo><ds:SignatureValue>KGIGmQcIguXMdsg+B05xQUbxPCeVJWgPm547uioWAilt/9MoZAHtP1jQ0thSDwM0T0pOavdiKYIE0VnW0Z5sb225Teg3ezB5VDU8rbsoIaT4g3ZzKvgJlR4jUQiOwb6tmpgtdDWH025OJkqKQHG8PWxgJwG5c4AQaykPaLQ0cu+YE4BqMQxYlWY0jeGlX4Hwx8GJI66AbJ/NHm2Zy0FdAJfaMELUU7xhTCzjks0yyz1x/JQuSYUt/31BKqxAyQeu/sR3mxZY2qlBSyfanJZN5AEZ7dC7VkSV4rLeftesl6rEfvktOmlUzwuHLp0er0rZrIljEHZ14FK9EMncYJ/x/Q==</ds:SignatureValue><KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#"><X509Data><X509Certificate>MIIFTzCCBDegAwIBAgIQBu32LObIZLQ0C8lW0d+QzjANBgkqhkiG9w0BAQsFADBNMQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMScwJQYDVQQDEx5EaWdpQ2VydCBTSEEyIFNlY3VyZSBTZXJ2ZXIgQ0EwHhcNMTcwOTAxMDAwMDAwWhcNMTkxMTI3MTIwMDAwWjCBkTELMAkGA1UEBhMCVVMxFTATBgNVBAgTDFBlbm5zeWx2YW5pYTEOMAwGA1UEBxMFRXh0b24xJjAkBgNVBAoTHUJlbnRsZXkgU3lzdGVtcywgSW5jb3Jwb3JhdGVkMQswCQYDVQQLEwJJVDEmMCQGA1UEAxMdaW1zLXRva2VuLXNpZ25pbmcuYmVudGxleS5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDgYLZczB8Xi1LnkzKquDH5tlR0m1BoifgKBs2HfentwrKFfmCiVxNDZgDpAovLUhBsrniYrRleJIrGRIn5Hd9VkiZxLMWCc8zST6bLGpCc4LFP7R6Wnqf6j1eQCIFAqV9FKoBOTgAKbQUQJnvwVbLD/BO3YOWm8XCy0wvuTJMn/rYo4BZEEyzukoppWiPeKRfYr3if+Koj/f0C8piBNLr3+RoNh50yfaC+v3+opctIJzp20iNn+jUt8CMEEKihRbQOYxRww8aGD4A9dbrCVz8zSNqMTl1cUoWAJkLJeDtCc8U2N4kV7S+FwBR8r9uVW2UnUJXQwv2ZLKJdnl3hbltDAgMBAAGjggHkMIIB4DAfBgNVHSMEGDAWgBQPgGEcgjFh1S8o541GOLQs4cbZ4jAdBgNVHQ4EFgQUGWYxPyDohu1zEgCg6VXLdoLd3kQwKAYDVR0RBCEwH4IdaW1zLXRva2VuLXNpZ25pbmcuYmVudGxleS5jb20wDgYDVR0PAQH/BAQDAgWgMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjBrBgNVHR8EZDBiMC+gLaArhilodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vc3NjYS1zaGEyLWcxLmNybDAvoC2gK4YpaHR0cDovL2NybDQuZGlnaWNlcnQuY29tL3NzY2Etc2hhMi1nMS5jcmwwTAYDVR0gBEUwQzA3BglghkgBhv1sAQEwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29tL0NQUzAIBgZngQwBAgIwfAYIKwYBBQUHAQEEcDBuMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5jb20wRgYIKwYBBQUHMAKGOmh0dHA6Ly9jYWNlcnRzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydFNIQTJTZWN1cmVTZXJ2ZXJDQS5jcnQwDAYDVR0TAQH/BAIwADANBgkqhkiG9w0BAQsFAAOCAQEASGipTbF15V5C7MCo7udTP81Hc2jswF6vLoA3NLVmM1BiPWGxVdAo0TfNlxoBoZFQ1nYQCy5tNfVzBell8xxrguhUU+Vnv59YLWrIkeAWALMCHOJ34Eec8ki655TlNlMttO36VuUtTGQPfxjDfFVhIqWCgvoh29JEtfMndZYmeu0IqNuFGWODtaeSfViBjfoGRDbnCoMOjt1coZLuvSV5+ECAXJ4UJxP3ch7y4y5RG5qbDB58oD4CIpb9yw0r+U2cip+ywemPGfKUYJ1MGVKNbu5/+eelkUzGPqJd+GMbA4xrGyEz0VRELAYgPP6B1vrmgCmpEyrjC4XNohVFjowQ9g==</X509Certificate></X509Data></KeyInfo></ds:Signature></saml:Assertion>
)xml";
    SamlTokenPtr initialToken = std::make_shared<SamlToken>(tokenStr);

    auto parentProvider = std::make_shared<MockConnectTokenProvider>();

    InSequence seq;
    EXPECT_CALL(*parentProvider, GetToken()).WillOnce(Return(initialToken));
    EXPECT_CALL(*parentProvider, UpdateToken()).WillOnce(Return(CreateCompletedAsyncTask<ISecurityTokenPtr>(SamlTokenPtr())));

    auto provider = std::make_shared<DelegationTokenProvider>(imsClient, ImsClient::GetLegacyRelyingPartyUri(), parentProvider);

    auto token1 = provider->GetToken();
    ASSERT_TRUE(token1 == nullptr);

    auto token2 = provider->UpdateToken()->GetResult();
    EXPECT_TRUE(token2 == nullptr);

    auto token3 = provider->GetToken();
    ASSERT_TRUE(token3 == nullptr);
    }
