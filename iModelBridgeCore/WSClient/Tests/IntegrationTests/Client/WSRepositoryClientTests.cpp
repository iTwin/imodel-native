/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Client/WSRepositoryClientTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WSRepositoryClientTests.h"

#include <Bentley/BeDebugLog.h>
#include <Bentley/BeTimeUtilities.h>
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectTokenProvider.h>
#include <WebServices/Connect/ImsClient.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <curl/curl.h>

void WSRepositoryClientTests::SetUp()
    {
    WSClientBaseTest::SetUp();

    m_localState = StubLocalState();

    ConnectAuthenticationPersistence::CustomInitialize(&m_localState);
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &m_localState);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_DGNCLIENTFX_UTILS_HTTP, NativeLogging::LOG_INFO);
    }

TEST_F(WSRepositoryClientTests, SendQueryRequest_ConnectGlobalProjectQueryWithConnectSignInManager_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://qa-wsg20-eus.cloudapp.net/";
    Utf8String repositoryId = "BentleyCONNECT.Global--CONNECT.GLOBAL";
    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = client->SendQueryRequest(WSQuery("GlobalSchema", "Project"))->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    auto resultStr = RapidJsonToString(result.GetValue().GetRapidJsonDocument());
    }

TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_NavigateConnectGlobalWithExpiredToken_RetrievesNewTokenAndSucceeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://dev-wsg20-eus.cloudapp.net/";
    Utf8String repositoryId = "BentleyCONNECT.Global--CONNECT.GLOBAL";

    auto imsClient = ImsClient::Create(StubValidClientInfo(), proxy);
    auto persistence = ConnectAuthenticationPersistence::GetShared();
    auto provider = std::make_shared<ConnectTokenProvider>(imsClient, persistence);
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(serverUrl, provider, proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");
    persistence->SetCredentials(credentials);

    Utf8String expiredTokenStr(R"xml(<saml:Assertion MajorVersion="1" MinorVersion="1" AssertionID="_0c2f6953-48a5-40ca-92f6-3c613bdc0579" Issuer="https://ims-testing.bentley.com/" IssueInstant="2015-04-03T08:38:30.287Z" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion"><saml:Conditions NotBefore="2015-04-03T08:38:30.209Z" NotOnOrAfter="2015-04-03T09:08:30.209Z"><saml:AudienceRestrictionCondition><saml:Audience>https://dev-wsg20-eus.cloudapp.net</saml:Audience></saml:AudienceRestrictionCondition></saml:Conditions><saml:AttributeStatement><saml:Subject><saml:SubjectConfirmation><saml:ConfirmationMethod>urn:oasis:names:tc:SAML:1.0:cm:holder-of-key</saml:ConfirmationMethod><KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#"><trust:BinarySecret xmlns:trust="http://docs.oasis-open.org/ws-sx/ws-trust/200512">PV03neG5CWyeYHKsDlGnavKapHke96FWEQabwf453NE=</trust:BinarySecret></KeyInfo></saml:SubjectConfirmation></saml:Subject><saml:Attribute AttributeName="name" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>8cc45bd041514b58947ea6c09c@gmail.com</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="givenname" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>John</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="surname" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>Stevenson</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="emailaddress" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>8cc45bd041514b58947ea6c09c@gmail.com</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="sapbupa" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>1004124332</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="site" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>4024103</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="ultimatesite" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>1001381840</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="sapentitlement" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>NEW_BENTLEY_LEARN</saml:AttributeValue><saml:AttributeValue>ELS_FOUR_HR</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="entitlement" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>NEW_BENTLEY_LEARN</saml:AttributeValue><saml:AttributeValue>ELS_Four_Hour</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="countryiso" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>US</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="languageiso" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>EN</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="ismarketingprospect" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>false</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="isbentleyemployee" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>false</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="userid" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>53e87de9-ed14-4ef1-bd78-35e8218bb5cc</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="organization" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>Pennsylvania DOT</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="has_select" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>true</saml:AttributeValue></saml:Attribute></saml:AttributeStatement><ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#"><ds:SignedInfo><ds:CanonicalizationMethod Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#" /><ds:SignatureMethod Algorithm="http://www.w3.org/2001/04/xmldsig-more#rsa-sha256" /><ds:Reference URI="#_0c2f6953-48a5-40ca-92f6-3c613bdc0579"><ds:Transforms><ds:Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature" /><ds:Transform Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#" /></ds:Transforms><ds:DigestMethod Algorithm="http://www.w3.org/2001/04/xmlenc#sha256" /><ds:DigestValue>zuD1kD7SvjbASTm6n61CgDZgUe/bwi8ncnYTssIVai0=</ds:DigestValue></ds:Reference></ds:SignedInfo><ds:SignatureValue>Jcm4Bn43ifm9lYuHfREXM/C49u/QuiSgZMQ/ptweUK3UKw6yZVK4E6oCU25NzmNXp26fuftil30VqsyQaeZIuIb/22HQgZmFLwsvmbzMbNRp3cxiJ5dYLmTb99jsFzs/SqxfQMFJTUKN2CJhLgt33Ib928vBgC7TzP7aV6iKsgfCpX0ukEloVtmwbwSpSiVYTkU917lVmjmtIyY5Jn9D7wLg6rFcKM+739Z+Ft0Hr5LZC14jhJJ8JB0pYq8lA773J+f7+XF8CWXJqWTedpCWHpvdoQvopX8PHIPf1qANtuKBemHy0uXcB+shDW5W2dEfwWpxDTEP+e03Rt2cI118qw==</ds:SignatureValue><KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#"><X509Data><X509Certificate>MIIFKzCCBBOgAwIBAgIQAe1xT+EFUrGFCgROz+3tMjANBgkqhkiG9w0BAQsFADBNMQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMScwJQYDVQQDEx5EaWdpQ2VydCBTSEEyIFNlY3VyZSBTZXJ2ZXIgQ0EwHhcNMTQxMDEzMDAwMDAwWhcNMTUxMTA0MTIwMDAwWjCBhDELMAkGA1UEBhMCVVMxFTATBgNVBAgTDFBlbm5zeWx2YW5pYTEOMAwGA1UEBxMFRXh0b24xJjAkBgNVBAoTHUJlbnRsZXkgU3lzdGVtcywgSW5jb3Jwb3JhdGVkMQswCQYDVQQLEwJJVDEZMBcGA1UEAxMQYXV0aC5iZW50bGV5LmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPF+DcSZwV3PiexVlIAmxrOZBw6lI6nHcA8iO7EFD7yP/Z0706F0wKmxVsVm1iCt6TVdoD2DzB3HXaCY1CczbYLcZzBpgfbYN4Lkdby+UsTY3aHoaNvmYqtMwntDKtyI9j+Z+T6fMm/HcQ8EO/d0Bb49afO2RiBSq/k4fTD58MCjIMOa9KCoaLeCTu7IIA7RgVZw1DlIF6lJZSaISHHFhlK6lE9jjM1uxoOH1VZzEnutTj4puPOs3IhtCZAWF8vnRWDtDmKeEAjonZ1ng8WviEsFSY11cPaggWxyWgMhWV80i7sJn0nLEihCoqsxtQRf95r61zPM385xvXdqxYu8MskCAwEAAaOCAc0wggHJMB8GA1UdIwQYMBaAFA+AYRyCMWHVLyjnjUY4tCzhxtniMB0GA1UdDgQWBBTo8RZMp1LccsSyAcziGgvSru8eAjAbBgNVHREEFDASghBhdXRoLmJlbnRsZXkuY29tMA4GA1UdDwEB/wQEAwIFoDAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwawYDVR0fBGQwYjAvoC2gK4YpaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL3NzY2Etc2hhMi1nMy5jcmwwL6AtoCuGKWh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNvbS9zc2NhLXNoYTItZzMuY3JsMEIGA1UdIAQ7MDkwNwYJYIZIAYb9bAEBMCowKAYIKwYBBQUHAgEWHGh0dHBzOi8vd3d3LmRpZ2ljZXJ0LmNvbS9DUFMwfAYIKwYBBQUHAQEEcDBuMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5jb20wRgYIKwYBBQUHMAKGOmh0dHA6Ly9jYWNlcnRzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydFNIQTJTZWN1cmVTZXJ2ZXJDQS5jcnQwDAYDVR0TAQH/BAIwADANBgkqhkiG9w0BAQsFAAOCAQEAsnH+6e49UpUUq+m2zWxioWEewUU42ak9Kr2TTyYiFMwPeEmH09czp3Z5b2dEFt6wDSrTlGugO0SHDz8gI79hdckl/vi1lF0zzW8grMPJw5QxYoDtHgkOAMTBMZKWGnrE/dhr02EkjVmIdxVvWCJ/mt4GNJroTcFMHCRy9+rplHKA1n9fMXAHspmzILq8wHd/HkDT4sDHkoM4RJhNlzt3mKvyyknhFf7K0JjG/+Cp1+RQ/k5m1QZL1LaoiRkWzF31hQRTwkqYAoyr6rHCiDmoMWQgzH9TfbfX2zFcnOO1kv6euB5doDoUsb4ppof8hM67O5sBtKVwzUkRsP069NUGlg==</X509Certificate></X509Data></KeyInfo></ds:Signature></saml:Assertion>)xml");
    persistence->SetToken(std::make_shared<SamlToken>(expiredTokenStr));

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = client->SendGetChildrenRequest(ObjectId())->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    auto resultStr = RapidJsonToString(result.GetValue().GetRapidJsonDocument());
    printf(resultStr.c_str());
    }

TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_NavigateConnectGlobalWithExpiredTokenAndWrongPassword_ReturnsCredentialError)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://dev-wsg20-eus.cloudapp.net/";
    Utf8String repositoryId = "BentleyCONNECT.Global--CONNECT.GLOBAL";

    auto imsClient = ImsClient::Create(StubValidClientInfo(), proxy);
    auto persistence = ConnectAuthenticationPersistence::GetShared();
    auto provider = std::make_shared<ConnectTokenProvider>(imsClient, persistence);
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(serverUrl, provider, proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "wrongPassword");
    persistence->SetCredentials(credentials);

    Utf8String expiredTokenStr(R"xml(<saml:Assertion MajorVersion="1" MinorVersion="1" AssertionID="_0c2f6953-48a5-40ca-92f6-3c613bdc0579" Issuer="https://ims-testing.bentley.com/" IssueInstant="2015-04-03T08:38:30.287Z" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion"><saml:Conditions NotBefore="2015-04-03T08:38:30.209Z" NotOnOrAfter="2015-04-03T09:08:30.209Z"><saml:AudienceRestrictionCondition><saml:Audience>https://dev-wsg20-eus.cloudapp.net</saml:Audience></saml:AudienceRestrictionCondition></saml:Conditions><saml:AttributeStatement><saml:Subject><saml:SubjectConfirmation><saml:ConfirmationMethod>urn:oasis:names:tc:SAML:1.0:cm:holder-of-key</saml:ConfirmationMethod><KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#"><trust:BinarySecret xmlns:trust="http://docs.oasis-open.org/ws-sx/ws-trust/200512">PV03neG5CWyeYHKsDlGnavKapHke96FWEQabwf453NE=</trust:BinarySecret></KeyInfo></saml:SubjectConfirmation></saml:Subject><saml:Attribute AttributeName="name" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>8cc45bd041514b58947ea6c09c@gmail.com</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="givenname" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>John</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="surname" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>Stevenson</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="emailaddress" AttributeNamespace="http://schemas.xmlsoap.org/ws/2005/05/identity/claims"><saml:AttributeValue>8cc45bd041514b58947ea6c09c@gmail.com</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="sapbupa" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>1004124332</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="site" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>4024103</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="ultimatesite" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>1001381840</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="sapentitlement" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>NEW_BENTLEY_LEARN</saml:AttributeValue><saml:AttributeValue>ELS_FOUR_HR</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="entitlement" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>NEW_BENTLEY_LEARN</saml:AttributeValue><saml:AttributeValue>ELS_Four_Hour</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="countryiso" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>US</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="languageiso" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>EN</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="ismarketingprospect" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>false</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="isbentleyemployee" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>false</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="userid" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>53e87de9-ed14-4ef1-bd78-35e8218bb5cc</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="organization" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>Pennsylvania DOT</saml:AttributeValue></saml:Attribute><saml:Attribute AttributeName="has_select" AttributeNamespace="http://schemas.bentley.com/ws/2011/03/identity/claims"><saml:AttributeValue>true</saml:AttributeValue></saml:Attribute></saml:AttributeStatement><ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#"><ds:SignedInfo><ds:CanonicalizationMethod Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#" /><ds:SignatureMethod Algorithm="http://www.w3.org/2001/04/xmldsig-more#rsa-sha256" /><ds:Reference URI="#_0c2f6953-48a5-40ca-92f6-3c613bdc0579"><ds:Transforms><ds:Transform Algorithm="http://www.w3.org/2000/09/xmldsig#enveloped-signature" /><ds:Transform Algorithm="http://www.w3.org/2001/10/xml-exc-c14n#" /></ds:Transforms><ds:DigestMethod Algorithm="http://www.w3.org/2001/04/xmlenc#sha256" /><ds:DigestValue>zuD1kD7SvjbASTm6n61CgDZgUe/bwi8ncnYTssIVai0=</ds:DigestValue></ds:Reference></ds:SignedInfo><ds:SignatureValue>Jcm4Bn43ifm9lYuHfREXM/C49u/QuiSgZMQ/ptweUK3UKw6yZVK4E6oCU25NzmNXp26fuftil30VqsyQaeZIuIb/22HQgZmFLwsvmbzMbNRp3cxiJ5dYLmTb99jsFzs/SqxfQMFJTUKN2CJhLgt33Ib928vBgC7TzP7aV6iKsgfCpX0ukEloVtmwbwSpSiVYTkU917lVmjmtIyY5Jn9D7wLg6rFcKM+739Z+Ft0Hr5LZC14jhJJ8JB0pYq8lA773J+f7+XF8CWXJqWTedpCWHpvdoQvopX8PHIPf1qANtuKBemHy0uXcB+shDW5W2dEfwWpxDTEP+e03Rt2cI118qw==</ds:SignatureValue><KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#"><X509Data><X509Certificate>MIIFKzCCBBOgAwIBAgIQAe1xT+EFUrGFCgROz+3tMjANBgkqhkiG9w0BAQsFADBNMQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMScwJQYDVQQDEx5EaWdpQ2VydCBTSEEyIFNlY3VyZSBTZXJ2ZXIgQ0EwHhcNMTQxMDEzMDAwMDAwWhcNMTUxMTA0MTIwMDAwWjCBhDELMAkGA1UEBhMCVVMxFTATBgNVBAgTDFBlbm5zeWx2YW5pYTEOMAwGA1UEBxMFRXh0b24xJjAkBgNVBAoTHUJlbnRsZXkgU3lzdGVtcywgSW5jb3Jwb3JhdGVkMQswCQYDVQQLEwJJVDEZMBcGA1UEAxMQYXV0aC5iZW50bGV5LmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPF+DcSZwV3PiexVlIAmxrOZBw6lI6nHcA8iO7EFD7yP/Z0706F0wKmxVsVm1iCt6TVdoD2DzB3HXaCY1CczbYLcZzBpgfbYN4Lkdby+UsTY3aHoaNvmYqtMwntDKtyI9j+Z+T6fMm/HcQ8EO/d0Bb49afO2RiBSq/k4fTD58MCjIMOa9KCoaLeCTu7IIA7RgVZw1DlIF6lJZSaISHHFhlK6lE9jjM1uxoOH1VZzEnutTj4puPOs3IhtCZAWF8vnRWDtDmKeEAjonZ1ng8WviEsFSY11cPaggWxyWgMhWV80i7sJn0nLEihCoqsxtQRf95r61zPM385xvXdqxYu8MskCAwEAAaOCAc0wggHJMB8GA1UdIwQYMBaAFA+AYRyCMWHVLyjnjUY4tCzhxtniMB0GA1UdDgQWBBTo8RZMp1LccsSyAcziGgvSru8eAjAbBgNVHREEFDASghBhdXRoLmJlbnRsZXkuY29tMA4GA1UdDwEB/wQEAwIFoDAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwawYDVR0fBGQwYjAvoC2gK4YpaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL3NzY2Etc2hhMi1nMy5jcmwwL6AtoCuGKWh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNvbS9zc2NhLXNoYTItZzMuY3JsMEIGA1UdIAQ7MDkwNwYJYIZIAYb9bAEBMCowKAYIKwYBBQUHAgEWHGh0dHBzOi8vd3d3LmRpZ2ljZXJ0LmNvbS9DUFMwfAYIKwYBBQUHAQEEcDBuMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5jb20wRgYIKwYBBQUHMAKGOmh0dHA6Ly9jYWNlcnRzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydFNIQTJTZWN1cmVTZXJ2ZXJDQS5jcnQwDAYDVR0TAQH/BAIwADANBgkqhkiG9w0BAQsFAAOCAQEAsnH+6e49UpUUq+m2zWxioWEewUU42ak9Kr2TTyYiFMwPeEmH09czp3Z5b2dEFt6wDSrTlGugO0SHDz8gI79hdckl/vi1lF0zzW8grMPJw5QxYoDtHgkOAMTBMZKWGnrE/dhr02EkjVmIdxVvWCJ/mt4GNJroTcFMHCRy9+rplHKA1n9fMXAHspmzILq8wHd/HkDT4sDHkoM4RJhNlzt3mKvyyknhFf7K0JjG/+Cp1+RQ/k5m1QZL1LaoiRkWzF31hQRTwkqYAoyr6rHCiDmoMWQgzH9TfbfX2zFcnOO1kv6euB5doDoUsb4ppof8hM67O5sBtKVwzUkRsP069NUGlg==</X509Certificate></X509Data></KeyInfo></ds:Signature></saml:Assertion>)xml");
    persistence->SetToken(std::make_shared<SamlToken>(expiredTokenStr));

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = client->SendGetChildrenRequest(ObjectId())->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Status::ReceivedError, result.GetError().GetStatus());
    EXPECT_EQ(WSError::Id::LoginFailed, result.GetError().GetId());
    }

TEST_F(WSRepositoryClientTests, DISABLED_SendCreateObjectRequest_ProjectWiseAndWSG1_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://bsw-construct2.bentley.com/ws";
    Utf8String repositoryId = "pw.PW";
    Credentials credentials("admin", "admin");

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(credentials);

    // Create in VR/upload/ folder
    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "className" : "document",
                "properties" : 
                    {
                    "name" : "TestUpload"
                    },
                "relationshipInstances" : 
                    [{
                    "relatedInstance" : 
                        {
                        "instanceId" : "acd8be01-f097-461d-837a-63afeea9a8ea",
                        "className" : "project"
                        }
                    }]
                }
            })");

    BeFileName filePath = StubFileWithSize(10000);
    auto onProgress = [=] (double bytesTransfered, double bytesTotal)
        {
        BeDebugLog(Utf8PrintfString("%.2f %% (%.2f kb)", bytesTransfered / bytesTotal, bytesTransfered / 1024).c_str());
        };

    auto result = client->SendCreateObjectRequest(objectCreationJson, filePath, onProgress)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

TEST_F(WSRepositoryClientTests, DISABLED_SendCreateObjectRequest_ProjectWiseAndWSG2_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://bsw-construct2.bentley.com/ws21";
    Utf8String repositoryId = "pw--BSW-CONSTRUCT.bentley.com~3APW";
    Credentials credentials("admin", "admin");

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(credentials);

    // Create in VR/upload/ folder
    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "schemaName" : "PW_WSG",
                "className" : "Document",
                "properties" : 
                    {
                    "name" : "TestUpload"
                    },
                "relationshipInstances" : 
                    [{
                    "schemaName" : "PW_WSG",
                    "className" : "DocumentParent",
                    "direction" : "forward",
                    "relatedInstance" : 
                        {
                        "schemaName" : "PW_WSG",
                        "className" : "Project",
                        "instanceId" : "acd8be01-f097-461d-837a-63afeea9a8ea"
                        }
                    }]
                }
            })");

    BeFileName filePath = StubFileWithSize(0);
    auto onProgress = [=] (double bytesTransfered, double bytesTotal)
        {
        BeDebugLog(Utf8PrintfString("%.2f %% (%.2f kb)", bytesTransfered / bytesTotal, bytesTransfered / 1024).c_str());
        };

    auto result = client->SendCreateObjectRequest(objectCreationJson, filePath, onProgress)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_FileLargerThanUploadChunks_UploadsEverythingSuccessfully)
    {
    // NOTE: This is testing HttpRequest::PerformAsync() implementation issue where reusing same CURLM handle
    // seems to cause transfer issues in TCP level and server side hangs.

    BeDebugLog(curl_version_info(CURLversion::CURLVERSION_FIRST)->version);
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://viltest2-7/ws22";

    Utf8String repositoryId = "Bentley.eB--viltest2-5~2CeB_Mobile";
    Credentials credentials("admin", "admin");

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(credentials);

    // Create in VR/upload/ folder
    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "className" : "File",
                "schemaName" : "eB",
                "properties" : 
                    {
                    "name" : "HiResImage.jpg"
                    },
                "relationshipInstances" : 
                    [{
                    "className" : "DocumentFiles",
                    "schemaName" : "eB",
                    "direction" : "backward",
                    "relatedInstance" : 
                        {
                        "instanceId" : "10093",
                        "className" : "Document",
                        "schemaName" : "eB"
                        }
                    }]
                }
            })");

    BeFileName filePath(StubFileWithSize(10 * 1000 * 1000));
    auto paralelUploadCount = 10;

    bset<AsyncTaskPtr<WSCreateObjectResult>> uploadTasks;
    bset<std::shared_ptr<AsyncTask>> tasks;

    for (int i = 0; i < paralelUploadCount; i++)
        {
        auto onProgress = [=] (double bytesTransfered, double bytesTotal)
            {
            BeDebugLog(Utf8PrintfString("#%d %.2f %% (%.2f KB) (%.0f Bytes)", i, (bytesTransfered / bytesTotal) * 100, bytesTransfered / 1024, bytesTransfered).c_str());
            };

        auto task = client->SendCreateObjectRequest(objectCreationJson, filePath, onProgress);
        uploadTasks.insert(task);
        }

    AsyncTask::WhenAll(tasks)->Wait();
    for (auto task : uploadTasks)
        {
        auto result = task->GetResult();

        // Verify that upload was successful and reached eB backend using False positive
        EXPECT_EQ(WSError::Id::ServerError, result.GetError().GetId());
        EXPECT_STREQ("Cannot insert duplicate file into document.", result.GetError().GetDisplayDescription().c_str());

        if ("Cannot insert duplicate file into document." != result.GetError().GetDisplayDescription())
            {
            BeDebugLog(result.GetError().GetDisplayMessage().c_str());
            BeDebugLog(result.GetError().GetDisplayDescription().c_str());
            }
        }
    }
