/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/MobileUtilsTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeDebugLog.h>
#include <MobileDgn/MobileDgnCommon.h>
#include <MobileDgn/MobileDgnL10N.h>
#include <MobileDgn/Utils/Http/HttpRequest.h>
#include "MobileUtilsTests.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_MOBILEDGN
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

std::shared_ptr<rapidjson::Document> BentleyApi::WSC::UnitTests::ToRapidJson (Utf8StringCR jsonString)
    {
    auto json = std::make_shared<rapidjson::Document>();
    bool fail = json->Parse<rapidjson::kParseDefaultFlags>(jsonString.c_str()).HasParseError();
    BeAssert(!fail && "Check json string");
    return json;
    }

Json::Value BentleyApi::WSC::UnitTests::ToJson (Utf8StringCR jsonString)
    {
    Json::Value json;
    bool success = Json::Reader::Parse(jsonString, json);
    BeAssert(success && "Check json string");
    return json;
    }

std::string BentleyApi::WSC::UnitTests::RapidJsonToString(const rapidjson::Value& json)
    {
    if (json.IsNull())
        {
        return "null";
        }

    using namespace rapidjson;

    GenericStringBuffer<UTF8<>> buffer;
    Writer<GenericStringBuffer<UTF8<>>> writer(buffer);

    json.Accept(writer);

    return buffer.GetString();
    }

bool rapidjson::operator==(const rapidjson::Value& a, const rapidjson::Value& b)
    {
    return RapidJsonToString(a) == RapidJsonToString(b);
    }

ECSchemaPtr BentleyApi::WSC::UnitTests::ParseSchema (Utf8StringCR schemaXml, ECSchemaReadContextPtr context)
    {
    if (context.IsNull())
        {
        context = ECSchemaReadContext::CreateContext();
        context->AddSchemaPath(FSTest::GetAssetsDir().AppendToPath(L"/MobileUtilsAssets/ECSchemas/CacheSchemas/"));
        }

    ECSchemaPtr schema;
    auto status = ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *context);

    EXPECT_EQ(SchemaReadStatus::SCHEMA_READ_STATUS_Success, status);
    EXPECT_TRUE(schema.IsValid());

    return schema;
    }

TestAppPathProvider::TestAppPathProvider()
    {
    BeTest::GetHost().GetDocumentsRoot(m_documentsDirectory);
    BeTest::GetHost().GetTempDir(m_temporaryDirectory);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(m_platformAssetsDirectory);

    BeFileName outputRoot;
    BeTest::GetHost().GetOutputRoot(outputRoot);
    m_localStateDirectory = outputRoot;
    }

void WSClientBaseTest::SetUp()
    {
    // Init libs
    MobileDgnCommon::SetApplicationPathsProvider(&m_pathProvider);
    BeFileName::CreateNewDirectory(m_pathProvider.GetTemporaryDirectory());

    BeSQLiteLib::Initialize(m_pathProvider.GetTemporaryDirectory());
    BeSQLite::EC::ECDb::Initialize(m_pathProvider.GetTemporaryDirectory(), &m_pathProvider.GetAssetsRootDirectory());

    MobileDgnL10N::ReInitialize (MobileDgnL10N::GetDefaultFrameworkSqlangFiles (), MobileDgnL10N::GetDefaultFrameworkSqlangFiles ());
    }

void WSClientBaseTest::TearDown()
    {
    MobileDgnCommon::SetApplicationPathsProvider(nullptr);
    }

void WSClientBaseTest::SetUpTestCase()
    {}

void WSClientBaseTest::TearDownTestCase()
    {}

BaseMockHttpHandlerTest::BaseMockHttpHandlerTest() :
m_handler(std::make_shared<MockHttpHandler>()),
m_client(nullptr, m_handler)
    {}

HttpClientCR BaseMockHttpHandlerTest::GetClient() const
    {
    return m_client;
    }

MockHttpHandler& BaseMockHttpHandlerTest::GetHandler() const
    {
    return *m_handler;
    }

std::shared_ptr<MockHttpHandler> BaseMockHttpHandlerTest::GetHandlerPtr() const
    {
    return m_handler;
    }

#define EXPECTED_COUNT_ANY -1

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler::MockHttpHandler()
    {
    m_perfomedRequests = 0;
    m_expectedRequests = EXPECTED_COUNT_ANY;
    m_onAnyRequestCallback = [&] (HttpRequestCR request)
        {
        Utf8PrintfString message
            (
            "\n"
            "Uninteresting HttpRequest was performed: %s \n"
            "Got %u requests.",
            request.GetUrl().c_str(),
            m_perfomedRequests
            );
        BeDebugLog(message.c_str());

        return HttpResponse(HttpResponseContent::Create(HttpStringBody::Create()), "", ConnectionStatus::CouldNotConnect, HttpStatus::None);
        };
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler::~MockHttpHandler()
    {
    if (m_expectedRequests != EXPECTED_COUNT_ANY)
        {
        EXPECT_EQ(m_expectedRequests, m_perfomedRequests);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
uint32_t MockHttpHandler::GetRequestsPerformed() const
    {
    return m_perfomedRequests;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<HttpResponse> MockHttpHandler::PerformRequest(HttpRequestCR request)
    {
    auto task = std::make_shared<PackagedAsyncTask<HttpResponse>>([&] ()
        {
        EXPECT_LT(m_perfomedRequests, std::numeric_limits<uint32_t>::max());
        m_perfomedRequests++;

        if (m_expectedRequests != EXPECTED_COUNT_ANY && m_expectedRequests < m_perfomedRequests)
            {
            Utf8PrintfString message
                (
                "\n"
                "Unexpected HttpRequest: %s \n"
                "Expected %lld requests. Got %u requests.",
                request.GetUrl().c_str(),
                m_expectedRequests,
                m_perfomedRequests
                );
            BeDebugLog(message.c_str());
            }

        if (m_onSpecificRequestMap.find(m_perfomedRequests) != m_onSpecificRequestMap.end())
            {
            return m_onSpecificRequestMap[m_perfomedRequests](request);
            }

        return m_onAnyRequestCallback(request);
        });
    task->Execute();
    return task;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ExpectRequests(uint32_t count)
    {
    m_expectedRequests = count;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ExpectOneRequest()
    {
    m_expectedRequests = 1;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler&  MockHttpHandler::ForRequest(uint32_t requestNumber, OnResponseCallback callback)
    {
    EXPECT_NE(0, requestNumber);
    m_onSpecificRequestMap[requestNumber] = callback;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler&  MockHttpHandler::ForRequest(uint32_t requestNumber, HttpResponseCR response)
    {
    return ForRequest(requestNumber, [=] (HttpRequestCR)
        {
        return response;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler&  MockHttpHandler::ForFirstRequest(OnResponseCallback callback)
    {
    return ForRequest(1, callback);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler&  MockHttpHandler::ForFirstRequest(HttpResponseCR response)
    {
    return ForRequest(1, response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ForAnyRequest(OnResponseCallback callback)
    {
    EXPECT_TRUE(nullptr != callback);
    m_onAnyRequestCallback = callback;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& MockHttpHandler::ForAnyRequest(HttpResponseCR response)
    {
    return ForAnyRequest([=] (HttpRequestCR)
        {
        return response;
        });
    }

BeFileName FSTest::GetAssetsDir()
    {
    BeFileName path;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(path);
    return path;
    }

BeFileName FSTest::GetTempDir()
    {
    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    return path;
    }

BeFileName FSTest::StubFilePath(Utf8StringCR customFileName)
    {
    BeFileName fileName;
    if (customFileName.empty())
        {
        BeSQLite::BeSQLiteLib::Initialize(GetTempDir());
        fileName = BeFileName(BeGuid().ToString() + ".txt");
        }
    else
        {
        fileName = BeFileName(customFileName);
        }

    BeFileName filePath = GetTempDir().AppendToPath(fileName);
    return filePath;
    }

BeFileName FSTest::StubFile(Utf8StringCR content, Utf8StringCR customFileName)
    {
    BeFileName filePath = StubFilePath(customFileName);

    BeFile file;
    file.Create(filePath);
    file.Write(nullptr, content.c_str(), static_cast<uint32_t>(content.length()));
    file.Close();

    return filePath;
    }

BeFileName FSTest::StubFileWithSize(uint32_t bytesCount, Utf8StringCR customFileName)
    {
    BeFileName filePath = StubFilePath();

    BeFile file;
    EXPECT_EQ(BeFileStatus::Success, file.Create(filePath));

    uint32_t kbCount = bytesCount / 1024;
    char kbBuffer[1024];
    memset(kbBuffer, 'X', 1024);

    while (kbCount--)
        {
        EXPECT_EQ(BeFileStatus::Success, file.Write(nullptr, kbBuffer, 1024));
        }
    EXPECT_EQ(BeFileStatus::Success, file.Write(nullptr, kbBuffer, bytesCount % 1024));
    EXPECT_EQ(BeFileStatus::Success, file.Close());
    return filePath;
    }

Utf8String FSTest::ReadFile(BeFileNameCR filePath)
    {
    bvector<Byte> fileContents;

    BeFile file;
    BeFileStatus status;

    status = file.Open(filePath, BeFileAccess::Read);
    BeAssert(status == BeFileStatus::Success);

    status = file.ReadEntireFile(fileContents);
    BeAssert(status == BeFileStatus::Success);

    status = file.Close();
    BeAssert(status == BeFileStatus::Success);

    Utf8String stringContents;
    stringContents.append(fileContents.begin(), fileContents.end());
    return stringContents;
    }

void FSTest::WriteToFile(Utf8StringCR content, BeFileNameCR filePath)
    {
    uint32_t written = 0;

    BeFile file;
    BeFileStatus status;

    status = file.Create(filePath, true);
    BeAssert(status == BeFileStatus::Success);

    status = file.Write(&written, content.c_str(), (uint32_t) content.size());
    BeAssert(status == BeFileStatus::Success);

    status = file.Close();
    BeAssert(status == BeFileStatus::Success);
    }
