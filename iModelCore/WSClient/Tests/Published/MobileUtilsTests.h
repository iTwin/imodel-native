/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/MobileUtilsTests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <MobileDgn/MobileDgnCommon.h>
#include <MobileDgn/Utils/Http/IHttpHandler.h>
#include <MobileDgn/Utils/Http/HttpRequest.h>
#include <MobileDgn/Utils/Http/HttpResponse.h>
#include <MobileDgn/Utils/Http/HttpClient.h>
#include "ValuePrinter.h"

#define BEGIN_WSCLIENT_UNIT_TESTS_NAMESPACE    BEGIN_BENTLEY_NAMESPACE namespace WSC { namespace UnitTests {
#define END_WSCLIENT_UNIT_TESTS_NAMESPACE      } } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_WSCLIENT_UNIT_TESTS    using namespace BentleyApi::WSC::UnitTests;

BEGIN_WSCLIENT_UNIT_TESTS_NAMESPACE
END_WSCLIENT_UNIT_TESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS
USING_NAMESPACE_WSCLIENT_UNIT_TESTS

BEGIN_WSCLIENT_UNIT_TESTS_NAMESPACE

std::shared_ptr<rapidjson::Document> ToRapidJson (Utf8StringCR jsonString);
Json::Value ToJson (Utf8StringCR jsonString);

ECN::ECSchemaPtr ParseSchema (Utf8StringCR schemaXml, ECN::ECSchemaReadContextPtr context = nullptr);

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestAppPathProvider : MobileDgn::IApplicationPathsProvider
    {
    private:
        BeFileName m_documentsDirectory;
        BeFileName m_temporaryDirectory;
        BeFileName m_platformAssetsDirectory;
        BeFileName m_localStateDirectory;

    protected:
        virtual BeFileNameCR _GetDocumentsDirectory () const  override
            {
            return m_documentsDirectory;
            }
        virtual BeFileNameCR _GetTemporaryDirectory () const override
            {
            return m_temporaryDirectory;
            }
        virtual BeFileNameCR _GetCachesDirectory () const override
            {
            return m_temporaryDirectory;
            }
        virtual BeFileNameCR _GetLocalStateDirectory () const override
            {
            return m_localStateDirectory;
            }
        virtual BeFileNameCR _GetAssetsRootDirectory () const override
            {
            return m_platformAssetsDirectory;
            }
        virtual BeFileNameCR _GetMarkupSeedFilePath () const override
            {
            static BeFileName s_blank;
            return s_blank; 
            }

    public:
        TestAppPathProvider ();
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MobileUtilsTest : ::testing::Test
    {
    TestAppPathProvider m_pathProvider;
    virtual void SetUp () override;
    virtual void TearDown () override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockHttpHandler : public IHttpHandler
    {
    public:
        typedef std::function<HttpResponse (HttpRequestCR)> OnResponseCallback;

    private:
        int64_t m_expectedRequests;

        std::map<uint32_t, OnResponseCallback> m_onSpecificRequestMap;
        OnResponseCallback m_onAnyRequestCallback;

        uint32_t m_perfomedRequests;

    public:
        MockHttpHandler ();
        virtual ~MockHttpHandler () override;
        virtual AsyncTaskPtr<HttpResponse> PerformRequest (HttpRequestCR request) override;

        uint32_t GetRequestsPerformed () const;

        MockHttpHandler& ExpectOneRequest ();
        MockHttpHandler& ExpectRequests (uint32_t count);

        MockHttpHandler& ForRequest (uint32_t requestNumber, OnResponseCallback callback);
        MockHttpHandler& ForRequest (uint32_t requestNumber, HttpResponseCR response);

        MockHttpHandler& ForFirstRequest (OnResponseCallback callback);
        MockHttpHandler& ForFirstRequest (HttpResponseCR response);

        MockHttpHandler& ForAnyRequest (OnResponseCallback callback);
        MockHttpHandler& ForAnyRequest (HttpResponseCR response);
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
class BaseMockHttpHandlerTest : public MobileUtilsTest
    {
    private:
        std::shared_ptr<MockHttpHandler>    m_handler;
        HttpClient                          m_client;

    public:
        BaseMockHttpHandlerTest ();
        HttpClientCR GetClient () const;
        MockHttpHandler& GetHandler () const;
        std::shared_ptr<MockHttpHandler> GetHandlerPtr () const;
    };

//--------------------------------------------------------------------------------------+
// @bsiclass                                                     Vincas.Razma    08/2014
// Class for testing against File System
//---------------+---------------+---------------+---------------+---------------+------+
class FSTest
    {
    public:
        static BeFileName GetAssetsDir ();
        static BeFileName GetTempDir ();
        static BeFileName StubFilePath (Utf8StringCR customFileName = "");
        static BeFileName StubFile (Utf8StringCR content = "TestContent", Utf8StringCR customFileName = "");
        static Utf8String ReadFile (BeFileNameCR filePath);
        static void WriteToFile (Utf8StringCR content, BeFileNameCR filePath);
    };

END_WSCLIENT_UNIT_TESTS_NAMESPACE
