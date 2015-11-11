/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Utils/WebServicesTestsHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "WebServicesUnitTests.h"

#include <Bentley/BeDebugLog.h>
#include <WebServices/Client/WSError.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/Client/WSChangeset.h>
#include <WebServices/Client/ObjectId.h>

#include "../WebServices/Connect/StubLocalState.h"
#include "../WebServices/Configuration/StubBuddiClient.h"

#include "ValuePrinter.h"
#include "StubInstances.h"
#include "WSClientBaseTest.h"
#include "BaseMockHttpHandlerTest.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_WSCLIENT_UNITTESTS

#define EXPECT_CONTAINS(container, value)                                       \
    EXPECT_FALSE(std::find(container.begin(), container.end(), value) == container.end())

#define EXPECT_NCONTAIN(container, value)                                       \
    EXPECT_TRUE(std::find(container.begin(), container.end(), value) == container.end())

#define EXPECT_BETWEEN(smallerValue, value, biggerValue)                        \
    EXPECT_LE(smallerValue, value);                                             \
    EXPECT_GE(biggerValue, value);

namespace rapidjson
    {
    bool operator==(const Value& a, const Value& b);
    }

BEGIN_WSCLIENT_UNITTESTS_NAMESPACE

std::shared_ptr<rapidjson::Document> ToRapidJson(Utf8StringCR jsonString);

Json::Value ToJson(Utf8StringCR jsonString);
JsonValuePtr ToJsonPtr(Utf8StringCR jsonString);
std::string RapidJsonToString(const rapidjson::Value& json);

template<typename T>
bvector<T> StubBVector(T element)
    {
    bvector<T> vector;
    vector.push_back(element);
    return vector;
    }

template<typename T>
bvector<T> StubBVector(std::initializer_list<T> list)
    {
    bvector<T> vector(list.begin(), list.end());
    return vector;
    }

template<typename T>
bset<T> StubBSet(std::initializer_list<T> list)
    {
    bset<T> set(list.begin(), list.end());
    return set;
    }

template<typename A, typename B>
bpair<A, B> StubBPair(A a, B b)
    {
    return bpair<A, B>(a, b);
    }

HttpResponse StubHttpResponse(ConnectionStatus status = ConnectionStatus::CouldNotConnect);
HttpResponse StubHttpResponse(HttpStatus httpStatus, Utf8StringCR body = "", const std::map<Utf8String, Utf8String>& headers = std::map<Utf8String, Utf8String>());
HttpResponse StubHttpResponse(HttpStatus httpStatus, HttpBodyPtr body, const std::map<Utf8String, Utf8String>& headers = std::map<Utf8String, Utf8String>());
HttpResponse StubJsonHttpResponse(HttpStatus httpStatus, Utf8StringCR body = "", const std::map<Utf8String, Utf8String>& headers = std::map<Utf8String, Utf8String>());
HttpResponse StubHttpResponseWithUrl(HttpStatus httpStatus, Utf8StringCR url);

WSInfo StubWSInfoWebApi(BeVersion webApiVersion = BeVersion(1, 3), WSInfo::Type type = WSInfo::Type::BentleyWSG);
//! Stub WebApi 1.1 and BentleyConnect server
HttpResponse StubWSInfoHttpResponseBentleyConnectV1();
//! Stub WebApi 1.1 and BWSG server
HttpResponse StubWSInfoHttpResponseWebApi11();
//! Stub WebApi 1.2 and BWSG server
HttpResponse StubWSInfoHttpResponseWebApi12();
//! Stub WebApi 1.3 and BWSG server. Default for testing WSG 1.x client code
HttpResponse StubWSInfoHttpResponseWebApi13();
//! Stub WebApi 2.0 and BWSG server
HttpResponse StubWSInfoHttpResponseWebApi20();
//! Stub WebApi 2.1 and BWSG server
HttpResponse StubWSInfoHttpResponseWebApi21();
//! Stub WebApi 2.2 and BWSG server
HttpResponse StubWSInfoHttpResponseWebApi22();
//! Stub WebApi version and BWSG server
HttpResponse StubWSInfoHttpResponseWebApi(BeVersion webApiVersion);

void WriteStringToHttpBody(Utf8StringCR string, HttpBodyPtr body);
Utf8String ReadHttpBody(HttpBodyPtr body);

WSInfo StubWSInfo(BeVersion version = BeVersion(1, 2), WSInfo::Type type = WSInfo::Type::BentleyWSG);

HttpResponse StubWSErrorHttpResponse(HttpStatus status, Utf8StringCR errorId, Utf8StringCR message = "", Utf8StringCR description = "");
HttpResponse StubWSInfoHttpResponseV1();
HttpResponse StubWSInfoHttpResponseV1BentleyConnect();
HttpResponse StubWSInfoHttpResponseV2();
HttpResponse StubWSInfoHttpResponse(BeVersion serverVersion);

WSError StubWSConnectionError();
WSError StubWSCanceledError();

ClientInfoPtr StubClientInfo();

ECN::ECSchemaPtr ParseSchema(Utf8StringCR schemaXml, ECN::ECSchemaReadContextPtr context = nullptr);

BeFileName GetTestsAssetsDir();
BeFileName GetTestsTempDir();
BeFileName StubFilePath(Utf8StringCR customFileName = "");
BeFileName StubFile(Utf8StringCR content = "TestContent", Utf8StringCR customFileName = "");
BeFileName StubFileWithSize(uint32_t bytesCount, Utf8StringCR customFileName = "");
Utf8String SimpleReadFile(BeFileNameCR filePath);
void SimpleWriteToFile(Utf8StringCR content, BeFileNameCR filePath);

END_WSCLIENT_UNITTESTS_NAMESPACE
