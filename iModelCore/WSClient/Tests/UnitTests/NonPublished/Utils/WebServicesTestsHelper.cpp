/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "WebServicesTestsHelper.h"
#include <Bentley/BeDirectoryIterator.h>
#include <WebServices/Cache/Util/JsonUtil.h>
#include <WebServices/Client/ClientInfo.h>

void BackDoor::DgnClientFx_Device::Initialize()
    {
#if defined (__ANDROID__)
    // WIP06 - DgnClientFx is not initialized, need to initialize APIs seperately
    ClientInfo::CacheAndroidDeviceId ("TestDeviceId");
#endif
    }

BEGIN_BENTLEY_NAMESPACE
bool operator <= (const DateTime& lhs, const DateTime& rhs)
    {
    DateTime::CompareResult result = DateTime::Compare(lhs, rhs);
    EXPECT_TRUE(DateTime::CompareResult::Error != result);
    return result == DateTime::CompareResult::Equals || result == DateTime::CompareResult::EarlierThan;
    }

bool operator >= (const DateTime& lhs, const DateTime& rhs)
    {
    DateTime::CompareResult result = DateTime::Compare(lhs, rhs);
    EXPECT_TRUE(DateTime::CompareResult::Error != result);
    return result == DateTime::CompareResult::Equals || result == DateTime::CompareResult::LaterThan;
    }
END_BENTLEY_NAMESPACE

bool rapidjson::operator==(const rapidjson::Value& a, const rapidjson::Value& b)
    {
    return JsonUtil::AreValuesEqual(a, b);
    }

BEGIN_BENTLEY_NAMESPACE
namespace Json
    {
    bool operator==(Utf8CP a, const Value& b)
        {
        return a == b.asString();
        }
    }
END_BENTLEY_NAMESPACE

BEGIN_WSCLIENT_UNITTESTS_NAMESPACE

std::shared_ptr<rapidjson::Document> ToRapidJson(Utf8StringCR jsonString)
    {
    auto json = std::make_shared<rapidjson::Document>();
    if ("null" == jsonString)
        {
        json->SetNull();
        return json;
        }

    bool fail = json->Parse<0>(jsonString.c_str()).HasParseError();
    if (fail)
        {
        EXPECT_TRUE(false && "Check json string");
        }
    return json;
    }

Json::Value ToJson(Utf8StringCR jsonString)
    {
    Json::Value json;
    bool success = Json::Reader::Parse(jsonString, json);
    if (!success)
        {
        EXPECT_TRUE(false && "Check json string");
        }
    return json;
    }

JsonValuePtr ToJsonPtr(Utf8StringCR jsonString)
    {
    return std::make_shared<Json::Value>(ToJson(jsonString));
    }

std::string RapidJsonToString(const rapidjson::Value& json)
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

Response StubHttpResponse(ConnectionStatus status)
    {
    HttpStatus httpStatus = HttpStatus::None;
    if (status == ConnectionStatus::OK)
        {
        httpStatus = HttpStatus::OK;
        }
    return Response(HttpResponseContent::Create(HttpStringBody::Create()), "", status, httpStatus);
    }

Response StubHttpResponse(HttpStatus httpStatus, Utf8StringCR body, const std::map<Utf8String, Utf8String>& headers)
    {
    return StubHttpResponse(httpStatus, HttpStringBody::Create(body), headers);
    }

Response StubHttpResponse(HttpStatus httpStatus, HttpBodyPtr body, const std::map<Utf8String, Utf8String>& headers)
    {
    ConnectionStatus status = ConnectionStatus::OK;
    if (httpStatus == HttpStatus::None)
        {
        status = ConnectionStatus::CouldNotConnect;
        }
    auto content = HttpResponseContent::Create(body);
    for (const auto& header : headers)
        {
        content->GetHeaders().SetValue(header.first, header.second);
        }
    return Http::Response(content, "", status, httpStatus);
    }

Http::Response StubJsonHttpResponse(HttpStatus httpStatus, Utf8StringCR body, const std::map<Utf8String, Utf8String>& headers)
    {
    auto newHeaders = headers;
    newHeaders["Content-Type"] = REQUESTHEADER_ContentType_ApplicationJson;
    return StubHttpResponse(httpStatus, body, newHeaders);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Simonas.Mulevicius   07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Http::Response StubHttpResponseWithUrl(HttpStatus httpStatus, Utf8StringCR url, Utf8StringCR body)
    {
    auto content = HttpResponseContent::Create(HttpStringBody::Create(body));
    ConnectionStatus status = ConnectionStatus::OK;
    if (httpStatus == HttpStatus::None)
        status = ConnectionStatus::CouldNotConnect;

    return Http::Response(content, url.c_str(), status, httpStatus);
    }

WSQuery StubWSQuery()
    {
    return WSQuery("TestSchema", "TestClass");
    }

Http::Response StubWSErrorHttpResponse(HttpStatus status, Utf8StringCR errorId, Utf8StringCR message, Utf8StringCR description)
    {
    Json::Value errorJson;

    errorJson["errorId"] = errorId;
    errorJson["errorMessage"] = message;
    errorJson["errorDescription"] = description;

    return StubHttpResponse(status, errorJson.toStyledString(), {{"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
    }

WSInfo StubWSInfoWebApi(BeVersion webApiVersion, WSInfo::Type type)
    {
    BeVersion serverVersion;
    if (webApiVersion >= BeVersion(2, 9))
        {
        serverVersion = BeVersion(2, 9);
        }
    else if (webApiVersion >= BeVersion(2, 8))
        {
        serverVersion = BeVersion(2, 8);
        }
    else if (webApiVersion >= BeVersion(2, 7))
        {
        serverVersion = BeVersion(2, 7);
        }
    else if (webApiVersion >= BeVersion(2, 0))
        {
        serverVersion = BeVersion(2, 0);
        }
    return WSInfo(serverVersion, webApiVersion, type);
    }

WSRepository StubWSRepository(Utf8StringCR url, Utf8StringCR id)
    {
    WSRepository repository;
    repository.SetServerUrl(url);
    repository.SetId(id);
    return repository;
    }


Http::Response StubWSInfoHttpResponseWebApi20()
    {
    return StubWSInfoHttpResponseWebApi(BeVersion(2, 0));
    }

Http::Response StubWSInfoHttpResponseWebApi21()
    {
    return StubWSInfoHttpResponseWebApi(BeVersion(2, 1));
    }

Http::Response StubWSInfoHttpResponseWebApi22()
    {
    return StubWSInfoHttpResponseWebApi(BeVersion(2, 2));
    }

Http::Response StubWSInfoHttpResponseWebApi23()
    {
    return StubWSInfoHttpResponseWebApi(BeVersion(2, 3));
    }

Http::Response StubWSInfoHttpResponseWebApi24()
    {
    return StubWSInfoHttpResponseWebApi(BeVersion(2, 4));
    }

Http::Response StubWSInfoHttpResponseWebApi25()
    {
    return StubWSInfoHttpResponseWebApi(BeVersion(2, 5));
    }

Http::Response StubWSInfoHttpResponseWebApi26()
    {
    return StubWSInfoHttpResponseWebApi(BeVersion(2, 6));
    }

Http::Response StubWSInfoHttpResponseWebApi27()
    {
    return StubWSInfoHttpResponseWebApi(BeVersion(2, 7));
    }

Http::Response StubWSInfoHttpResponseWebApi28()
    {
    return StubWSInfoHttpResponseWebApi(BeVersion(2, 8));
    }

Http::Response StubWSInfoHttpResponseWebApi(BeVersion webApiVersion)
    {
    auto info = StubWSInfoWebApi(webApiVersion);
    Utf8PrintfString serverHeader(
        "Bentley-WSG/%s, Bentley-WebAPI/%d.%d",
        info.GetVersion().ToString().c_str(),
        info.GetWebApiVersion().GetMajor(),
        info.GetWebApiVersion().GetMinor()
        );
    return StubHttpResponse(HttpStatus::OK, "", {{"Server", serverHeader.c_str()}});
    }

void WriteStringToHttpBody(Utf8StringCR string, HttpBodyPtr body)
    {
    size_t bytesToWrite = string.length();
    size_t bytesWritten = 0;
    while (bytesToWrite)
        {
        auto written = body->Write(bytesWritten + string.c_str(), bytesToWrite);
        if (!written)
            {
            EXPECT_TRUE(false);
            break;
            }
        bytesWritten += written;
        bytesToWrite -= written;
        }
    }

Utf8String ReadHttpBody(HttpBodyPtr body)
    {
    Utf8String bodyStr;
    if (body.IsNull())
        {
        return bodyStr;
        }

    char buffer[1000];

    body->Open();

    size_t read;
    while (read = body->Read(buffer, 1000))
        {
        bodyStr += Utf8String(buffer, read);
        }

    body->Close();

    return bodyStr;
    }

WSError StubWSConnectionError()
    {
    return WSError(StubHttpResponse());
    }

WSError StubWSCanceledError()
    {
    return WSError(StubHttpResponse(ConnectionStatus::Canceled));
    }

WSError StubWSConflictError()
    {
    return WSError(StubHttpResponse(ConnectionStatus::Canceled));
    }

ClientInfoPtr StubClientInfo()
    {
    return std::shared_ptr<ClientInfo>(new ClientInfo("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", "TestAppProductId"));
    }

ClientInfoPtr StubValidClientInfo()
    {
    auto productId = "test"; // Fake for tets
    return std::shared_ptr<ClientInfo>(new ClientInfo("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId));
    }

ObjectId StubObjectId()
    {
    return ObjectId("TestSchema", "TestClass", "TestId");
    }

ECSchemaPtr ParseSchema(Utf8StringCR schemaXml, ECSchemaReadContextPtr context)
    {
    if (context.IsNull())
        {
        context = ECSchemaReadContext::CreateContext();
        context->AddSchemaPath(GetTestsAssetsDir().AppendToPath(L"/MobileUtilsAssets/ECSchemas/CacheSchemas/"));
        context->AddSchemaPath(GetTestsAssetsDir().AppendToPath(L"ECSchemas/ECDb"));
        }

    ECSchemaPtr schema;
    auto status = ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *context);

    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema.IsValid());

    if (schema.IsNull())
        throw std::exception(); // Failed to parse schema

    return schema;
    }

BeFileName GetTestsAssetsDir()
    {
    BeFileName path;

    BeTest::GetHost().GetDgnPlatformAssetsDirectory(path);
    return path;
    }

BeFileName GetTestsTempDir()
    {
    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    return path;
    }

BeFileName GetTestsOutputDir()
    {
    BeFileName path;
    BeTest::GetHost().GetOutputRoot(path);
    return path;
    }

BeFileName StubFilePath(Utf8StringCR customFileName)
    {
    BeFileName fileName;
    if (customFileName.empty())
        {
        fileName = BeFileName(BeGuid(true).ToString() + ".txt");
        }
    else
        {
        fileName = BeFileName(customFileName);
        }

    BeFileName filePath = GetTestsTempDir().AppendToPath(fileName);
    return filePath;
    }

BeFileName StubFile(Utf8StringCR content, Utf8StringCR customFileName)
    {
    BeFileName filePath = StubFilePath(customFileName);

    if (!filePath.GetDirectoryName().DoesPathExist())
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(filePath.GetDirectoryName()));

    BeFile file;
    file.Create(filePath);
    file.Write(nullptr, content.c_str(), static_cast<uint32_t>(content.length()));
    file.Close();

    return filePath;
    }

BeFileName StubFileWithSize(uint64_t bytesCount, Utf8StringCR customFileName)
    {
    BeFileName filePath = StubFilePath(customFileName);

    if (!filePath.GetDirectoryName().DoesPathExist())
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(filePath.GetDirectoryName()));

    BeFile file;
    EXPECT_EQ(BeFileStatus::Success, file.Create(filePath));

    uint64_t kbCount = bytesCount / 1024;
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

Utf8String SimpleReadFile(BeFileNameCR filePath)
    {
    bvector<Byte> fileContents;

    BeFile file;
    BeFileStatus status;

    status = file.Open(filePath, BeFileAccess::Read);
    EXPECT_EQ(BeFileStatus::Success, status);

    status = file.ReadEntireFile(fileContents);
    EXPECT_EQ(BeFileStatus::Success, status);

    status = file.Close();
    EXPECT_EQ(BeFileStatus::Success, status);

    Utf8String stringContents;
    stringContents.append(fileContents.begin(), fileContents.end());
    return stringContents;
    }

Utf8String SimpleReadByteStream(ByteStream stream)
    {
    Utf8String stringContents = "";

    for (auto& byte : stream)
        stringContents += byte;

    return stringContents;
    }

void SimpleWriteToFile(Utf8StringCR content, BeFileNameCR filePath)
    {
    if (!filePath.GetDirectoryName().DoesPathExist())
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(filePath.GetDirectoryName()));

    uint32_t written = 0;

    BeFile file;
    BeFileStatus status;

    status = file.Create(filePath, true);
    EXPECT_EQ(BeFileStatus::Success, status);

    status = file.Write(&written, content.c_str(), (uint32_t) content.size());
    EXPECT_EQ(BeFileStatus::Success, status);

    status = file.Close();
    EXPECT_EQ(BeFileStatus::Success, status);
    }

std::set<Utf8String> GetFolderContent(BeFileNameCR dir)
    {
    std::set<Utf8String> names;

    BeFileName name;
    bool isDir;
    for (BeDirectoryIterator it(dir); it.GetCurrentEntry(name, isDir, false) == SUCCESS; it.ToNext())
        {
        names.insert(Utf8String(name));
        }

    return names;
    }

END_WSCLIENT_UNITTESTS_NAMESPACE
