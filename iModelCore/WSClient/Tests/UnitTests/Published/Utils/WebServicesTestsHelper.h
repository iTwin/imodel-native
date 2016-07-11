/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/WebServicesTestsHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "WebServicesUnitTests.h"

#include <Bentley/BeDebugLog.h>
#include <Bentley/BeTimeUtilities.h>
#include <WebServices/Client/WSError.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/Client/WSChangeset.h>
#include <WebServices/Client/ObjectId.h>

#include "../WebServices/Connect/StubLocalState.h"
#include "../WebServices/Configuration/StubBuddiClient.h"

#include "ValuePrinter.h"
#include "AsyncTestCheckpoint.h"
#include "StubInstances.h"
#include "WSClientBaseTest.h"
#include "BaseMockHttpHandlerTest.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_WSCLIENT_UNITTESTS

#ifndef EXPECT_CONTAINS
    #define EXPECT_CONTAINS(container, value)                                       \
        EXPECT_FALSE(std::find(container.begin(), container.end(), value) == container.end())
#endif

#ifndef EXPECT_NCONTAIN
    #define EXPECT_NCONTAIN(container, value)                                       \
        EXPECT_TRUE(std::find(container.begin(), container.end(), value) == container.end())
#endif

#ifndef EXPECT_BETWEEN
    #define EXPECT_BETWEEN(smallerValue, value, biggerValue)                        \
        EXPECT_LE(smallerValue, value);                                             \
        EXPECT_GE(biggerValue, value);
#endif

// Operator for comparisons
bool operator <= (const DateTime& lhs, const DateTime& rhs);
bool operator >= (const DateTime& lhs, const DateTime& rhs);

namespace rapidjson
    {
    bool operator==(const Value& a, const Value& b);
    }

// Comparsion operator for testing equality
BEGIN_BENTLEY_NAMESPACE
namespace Json
    {
    bool operator==(Utf8CP a, const Value& b);
    }
END_BENTLEY_NAMESPACE

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

Response StubHttpResponse(ConnectionStatus status = ConnectionStatus::CouldNotConnect);
Response StubHttpResponse(HttpStatus httpStatus, Utf8StringCR body = "", const std::map<Utf8String, Utf8String>& headers = std::map<Utf8String, Utf8String>());
Response StubHttpResponse(HttpStatus httpStatus, HttpBodyPtr body, const std::map<Utf8String, Utf8String>& headers = std::map<Utf8String, Utf8String>());
Response StubJsonHttpResponse(HttpStatus httpStatus, Utf8StringCR body = "", const std::map<Utf8String, Utf8String>& headers = std::map<Utf8String, Utf8String>());
Response StubHttpResponseWithUrl(HttpStatus httpStatus, Utf8StringCR url);

WSInfo StubWSInfoWebApi(BeVersion webApiVersion = BeVersion(1, 3), WSInfo::Type type = WSInfo::Type::BentleyWSG);
//! Stub WebApi 1.1 and BentleyConnect server
Response StubWSInfoHttpResponseBentleyConnectV1();
//! Stub WebApi 1.1 and BWSG server
Response StubWSInfoHttpResponseWebApi11();
//! Stub WebApi 1.2 and BWSG server
Response StubWSInfoHttpResponseWebApi12();
//! Stub WebApi 1.3 and BWSG server. Default for testing WSG 1.x client code
Response StubWSInfoHttpResponseWebApi13();
//! Stub WebApi 2.0 and BWSG server
Response StubWSInfoHttpResponseWebApi20();
//! Stub WebApi 2.1 and BWSG server
Response StubWSInfoHttpResponseWebApi21();
//! Stub WebApi 2.2 and BWSG server
Response StubWSInfoHttpResponseWebApi22();
//! Stub WebApi 2.4 and BWSG server
Response StubWSInfoHttpResponseWebApi24();
//! Stub WebApi version and BWSG server
Response StubWSInfoHttpResponseWebApi(BeVersion webApiVersion);

void WriteStringToHttpBody(Utf8StringCR string, HttpBodyPtr body);
Utf8String ReadHttpBody(HttpBodyPtr body);

WSInfo StubWSInfo(BeVersion version = BeVersion(1, 2), WSInfo::Type type = WSInfo::Type::BentleyWSG);
WSQuery StubWSQuery();

Response StubWSErrorHttpResponse(HttpStatus status, Utf8StringCR errorId = "", Utf8StringCR message = "", Utf8StringCR description = "");
Response StubWSInfoHttpResponseV1();
Response StubWSInfoHttpResponseV1BentleyConnect();
Response StubWSInfoHttpResponseV2();
Response StubWSInfoHttpResponse(BeVersion serverVersion);

WSError StubWSConnectionError();
WSError StubWSCanceledError();

ClientInfoPtr StubClientInfo();
ClientInfoPtr StubValidClientInfo();

ObjectId StubObjectId();

ECN::ECSchemaPtr ParseSchema(Utf8StringCR schemaXml, ECN::ECSchemaReadContextPtr context = nullptr);

BeFileName GetTestsAssetsDir();
BeFileName GetTestsTempDir();
BeFileName GetTestsOutputDir();

BeFileName StubFilePath(Utf8StringCR customFileName = "");
BeFileName StubFile(Utf8StringCR content = "TestContent", Utf8StringCR customFileName = "");
BeFileName StubFileWithSize(uint32_t bytesCount, Utf8StringCR customFileName = "");
Utf8String SimpleReadFile(BeFileNameCR filePath);
void SimpleWriteToFile(Utf8StringCR content, BeFileNameCR filePath);

END_WSCLIENT_UNITTESTS_NAMESPACE
