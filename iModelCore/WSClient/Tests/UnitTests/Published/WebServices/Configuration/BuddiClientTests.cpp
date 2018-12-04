/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Configuration/BuddiClientTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BuddiClientTests.h"

#include <WebServices/Configuration/BuddiClient.h>
#include <Bentley/BeDebugLog.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

Utf8String FormatXml(Utf8StringCR inputXml)
    {
    BeXmlStatus status;
    BeXmlDomPtr dom = BeXmlDom::CreateAndReadFromString(status, inputXml.c_str(), inputXml.size());

    EXPECT_TRUE(dom.IsValid());
    EXPECT_EQ(BeXmlStatus::BEXML_Success, status);

    Utf8String formattedXml;
    dom->ToString(formattedXml, BeXmlDom::TO_STRING_OPTION_Indent);
    return formattedXml;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuddiClientTests, GetRegions_Default_SendsPostSoapRequest)
    {
    BuddiClient client(GetHandlerPtr(), "http://test.com");

    GetHandler().ExpectOneRequest().ForAnyRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("http://test.com", request.GetUrl().c_str());

        Utf8String requestBody = request.GetRequestBody()->AsString();
        Utf8String expectedBody = R"xml(<?xml version="1.0" encoding="utf-8"?>
            <soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                           xmlns:xsd="http://www.w3.org/2001/XMLSchema"
                           xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
    <soap:Body>
        <GetRegions xmlns="http://tempuri.org/" />
    </soap:Body>
</soap:Envelope>)xml";

        EXPECT_STREQ(FormatXml(expectedBody).c_str(), FormatXml(requestBody).c_str());
        return StubHttpResponse();
        });

    client.GetRegions()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuddiClientTests, GetRegions_ResponseContainsRegionsXml_ReturnsRegions)
    {
    BuddiClient client(GetHandlerPtr());

    Utf8String bodyXML = R"xml(<?xml version="1.0" encoding="utf-8"?>
            <soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/" 
                           xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
                           xmlns:xsd="http://www.w3.org/2001/XMLSchema">
                <soap:Body>
                    <GetRegionsResponse xmlns="http://tempuri.org/">
                        <GetRegionsResult>
                            <regions xmlns="">
                                <region name="Region A" id="100" />
                                <region name="Region B" id="220" />
                            </regions>
                        </GetRegionsResult>
                    </GetRegionsResponse>
                </soap:Body>
            </soap:Envelope>)xml";

    GetHandler().ExpectOneRequest().ForAnyRequest(StubHttpResponse(HttpStatus::OK, bodyXML));

    auto result = client.GetRegions()->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(2, result.GetValue().size());

    EXPECT_EQ(100, result.GetValue()[0].GetId());
    EXPECT_STREQ("Region A", result.GetValue()[0].GetName().c_str());

    EXPECT_EQ(220, result.GetValue()[1].GetId());
    EXPECT_STREQ("Region B", result.GetValue()[1].GetName().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuddiClientTests, GetRegions_ReceivesEmptyXML_ReturnsUnxpectedError)
    {
    BuddiClient client(GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest(StubHttpResponse(HttpStatus::OK, ""));

    BeTest::SetFailOnAssert(false);
    auto result = client.GetRegions()->GetResult();
    BeTest::SetFailOnAssert(true);

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(BuddiError::UnxpectedError, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuddiClientTests, GetRegions_ReceivesXmlWithWrongFormat_ReturnsUnxpectedError)
    {
    BuddiClient client(GetHandlerPtr());

    Utf8String bodyXML = R"xml(<?xml version="1.0" encoding="utf-8"?>
            <soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/"
                           xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                           xmlns:xsd="http://www.w3.org/2001/XMLSchema">
                <soap:Body>
                    <GetRegionsResponse xmlns="http://tempuri.org/">
                        <GetRegionsResult>
                            <regions xmlns="">
                                <region name="Foo" id="NotAndIntegerId" />
                            </regions>
                        </GetRegionsResult>
                    </GetRegionsResponse>
                </soap:Body>
            </soap:Envelope>)xml";

    GetHandler().ExpectOneRequest().ForAnyRequest(StubHttpResponse(HttpStatus::OK, bodyXML));

    BeTest::SetFailOnAssert(false);
    auto result = client.GetRegions()->GetResult();
    BeTest::SetFailOnAssert(true);

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(BuddiError::UnxpectedError, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuddiClientTests, GetRegions_ReceivesInternalServerError_ReturnsUnxpectedErrorWithMessage)
    {
    BuddiClient client(GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest(StubHttpResponse(HttpStatus::InternalServerError));

    auto result = client.GetRegions()->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(BuddiError::UnxpectedError, result.GetError().GetStatus());
    EXPECT_FALSE(result.GetError().GetMessage().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuddiClientTests, GetRegions_ReceivesCouldNotConnect_ReturnsConnectionErrorWithMessage)
    {
    BuddiClient client(GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest(StubHttpResponse(ConnectionStatus::CouldNotConnect));

    auto result = client.GetRegions()->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(BuddiError::ConnectionError, result.GetError().GetStatus());
    EXPECT_FALSE(result.GetError().GetMessage().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuddiClientTests, GetUrl_NameAndRegionPassed_SendsPostSoapRequest)
    {
    BuddiClient client(GetHandlerPtr(), "http://test.com");

    GetHandler().ExpectOneRequest().ForAnyRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("http://test.com", request.GetUrl().c_str());

        Utf8String requestBody = request.GetRequestBody()->AsString();
        Utf8String expectedBody = R"xml(<?xml version="1.0" encoding="utf-8"?>
            <soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                           xmlns:xsd="http://www.w3.org/2001/XMLSchema"
                           xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
    <soap:Body>
        <GetUrl xmlns="http://tempuri.org/">
            <urlName>https://test/foo</urlName>
            <region>999</region>
        </GetUrl>
    </soap:Body>
</soap:Envelope>)xml";

        EXPECT_STREQ(FormatXml(expectedBody).c_str(), FormatXml(requestBody).c_str());
        return StubHttpResponse();
        });

    client.GetUrl("https://test/foo", 999)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuddiClientTests, GetUrl_ResponseContainsUrl_ReturnsUrl)
    {
    BuddiClient client(GetHandlerPtr());

    Utf8String responseBody = R"xml(<?xml version="1.0" encoding="utf-8"?>
            <soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
                           xmlns:xsd="http://www.w3.org/2001/XMLSchema" 
                           xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
              <soap:Body>
                <GetUrlResponse xmlns="http://tempuri.org/">
                  <GetUrlResult>https://test/foo</GetUrlResult>
                </GetUrlResponse>
              </soap:Body>
            </soap:Envelope>)xml";

    GetHandler().ExpectOneRequest().ForAnyRequest(StubHttpResponse(HttpStatus::OK, responseBody));

    auto result = client.GetUrl("Foo")->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_STREQ("https://test/foo", result.GetValue().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
#if !defined(BENTLEYCONFIG_OS_ANDROID) // TFS#894646
TEST_F(BuddiClientTests, GetUrl_ResponseContainsEmptyUrl_ReturnsUrlNotConfiguredErrorWithMessage)
    {
    BuddiClient client(GetHandlerPtr());

    Utf8String bodyXML = R"xml(<?xml version="1.0" encoding="utf-8"?>
            <soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
                           xmlns:xsd="http://www.w3.org/2001/XMLSchema" 
                           xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
              <soap:Body>
                <GetUrlResponse xmlns="http://tempuri.org/">
                  <GetUrlResult></GetUrlResult>
                </GetUrlResponse>
              </soap:Body>
            </soap:Envelope>)xml";

    GetHandler().ExpectOneRequest().ForAnyRequest(StubHttpResponse(HttpStatus::OK, bodyXML));

    auto result = client.GetUrl("Foo")->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(BuddiError::UrlNotConfigured, result.GetError().GetStatus());
    EXPECT_FALSE(result.GetError().GetMessage().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuddiClientTests, GetUrl_ResponseUrlNotFound_ReturnsUrlNotConfiguredErrorWithMessage)
    {
    BuddiClient client(GetHandlerPtr());

    Utf8String bodyXML = R"xml(<?xml version="1.0" encoding="utf-8"?>
            <soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
                           xmlns:xsd="http://www.w3.org/2001/XMLSchema" 
                           xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
              <soap:Body>
                <GetUrlResponse xmlns="http://tempuri.org/" />
              </soap:Body>
            </soap:Envelope>)xml";

    GetHandler().ExpectOneRequest().ForAnyRequest(StubHttpResponse(HttpStatus::OK, bodyXML));

    auto result = client.GetUrl("Foo")->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(BuddiError::UrlNotConfigured, result.GetError().GetStatus());
    EXPECT_FALSE(result.GetError().GetMessage().empty());
    }
#endif
