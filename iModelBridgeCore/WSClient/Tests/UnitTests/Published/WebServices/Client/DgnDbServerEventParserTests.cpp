/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Client/DgnDbServerEventParserTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnDbServerEventParserTests.h"
#include <Bentley/Base64Utilities.h>
#include <WebServices/Cache/Util/JsonUtil.h>
#include <DgnDbServer/Client/DgnDbServerEventParser.h>
#include <DgnDbServer/Client/DgnDbServerLockEvent.h>
#include <DgnDbServer/Client/DgnDbServerRevisionEvent.h>

using namespace ::testing;
using namespace ::std;

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_DGNDBSERVER

Utf8String StubHttpResponseEmpty()
    {
    return "";
    }

Utf8String StubHttpResponseEmptyJson()
    {
    return "{}";
    }

Utf8String StubHttpResponseInvalid()
    {
    return "abcd";
    }

Utf8String StubHttpResponseValidLockEvent()
    {
    return R"(
              @string3http://schemas.microsoft.com/2003/10/Serialization/™Ž
              {
              "Date":"SomeDate",
              "LockType":"SomeLockType",
              "repoId":"SomeRepoId",
              "userId":"SomeUserId",
              "ObjectId":"SomeObjectId",
              "LockLevel":"SomeLockLevel",
              "BriefcaseId":"SomeBriefcaseId",
              "ReleasedWithRevision":"SomeReleasedWithRevision"
              }
             )";
    }

Utf8String StubHttpResponseValidRevisionEvent()
    {
    return R"(
              @string3http://schemas.microsoft.com/2003/10/Serialization/™Ž
              {
              "Date":"SomeDate",
              "repoId":"SomeRepoId",
              "userId":"SomeUserId",
              "RevisionId":"RevisionId",
              "RevisionIndex":"SomeRevisionIndex"
              }
             )";
    }

Utf8String StubHttpResponseInvalidLockEvent()
    {
    return R"(
              @string3http://schemas.microsoft.com/2003/10/Serialization/™Ž
              {
              "Date":"SomeDate",
              "BriefcaseId":"SomeBriefcaseId",
              "ReleasedWithRevision":"SomeReleasedWithRevision"
              }
             )";
    }

Utf8String StubHttpResponseInvalidRevisionEvent()
    {
    return R"(
              @string3http://schemas.microsoft.com/2003/10/Serialization/™Ž
              {
              "Date":"SomeDate",
              "RevisionIndex":"SomeRevisionIndex"
              }
             )";
    }

Utf8CP StubHttpResponseEmptyContentType()
    {
    return "";
    }

Utf8CP StubHttpResponseInvalidContentType()
    {
    return "SomeContentType";
    }

Utf8CP StubHttpResponseValidLockEventContentType()
    {
    return IDgnDbServerEvent::GetEventName(DgnDbServerEventType::LockEvent).c_str();
    }

Utf8CP StubHttpResponseInvalidLockEventContentType()
    {
    return IDgnDbServerEvent::GetEventName(DgnDbServerEventType::UnknownEventType).c_str();
    }

Utf8CP StubHttpResponseValidRevisionEventContentType()
    {
    return IDgnDbServerEvent::GetEventName(DgnDbServerEventType::RevisionEvent).c_str();
    }

Utf8CP StubHttpResponseInvalidRevisionEventContentType()
    {
    return IDgnDbServerEvent::GetEventName(DgnDbServerEventType::UnknownEventType).c_str();
    }

void DgnDbServerEventParserTests::SetUp()
    {
    BaseMockHttpHandlerTest::SetUp();
    }

TEST_F(DgnDbServerEventParserTests, LockEventTests)
    {
    //Check for valid values as Json
    DgnDbServerEventParser *parser = new DgnDbServerEventParser();
    IDgnDbServerEventPtr validPtr = parser->ParseEventasJson(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidLockEvent());
    EXPECT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEventType::LockEvent, parser->GetEventType(validPtr));
    DgnDbServerLockEvent& lockEvent = dynamic_cast<DgnDbServerLockEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<IDgnDbServerEvent*>(&lockEvent)); //DgnDbServerLockEvent is a subclass of DgnDbServerEvent

    //Check for valid values as String
    validPtr = parser->ParseEventasString(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidLockEvent());
    EXPECT_NE(nullptr, validPtr);
    }

TEST_F(DgnDbServerEventParserTests, RevisionEventTests)
    {
    //Check for valid values
    DgnDbServerEventParser *parser = new DgnDbServerEventParser();
    IDgnDbServerEventPtr validPtr = parser->ParseEventasJson(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidRevisionEvent());
    EXPECT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEventType::RevisionEvent, parser->GetEventType(validPtr));
    DgnDbServerRevisionEvent& revisionEvent = dynamic_cast<DgnDbServerRevisionEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<IDgnDbServerEvent*>(&revisionEvent)); //DgnDbServerRevisionEvent is a subclass of DgnDbServerEvent

    //Check for valid values as String
    validPtr = parser->ParseEventasString(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidRevisionEvent());
    EXPECT_NE(nullptr, validPtr);
    }

TEST_F(DgnDbServerEventParserTests, InvalidEventTests)
    {
    DgnDbServerEventParser *parser = new DgnDbServerEventParser();
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseValidLockEventContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseValidLockEventContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidLockEvent()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidRevisionEvent()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidLockEvent()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidRevisionEvent()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidLockEvent()));
    }

TEST_F(DgnDbServerEventParserTests, InvalidContentTypeTests)
    {
    DgnDbServerEventParser *parser = new DgnDbServerEventParser();
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseEmptyContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseInvalidContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseEmptyContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseInvalidContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, parser->ParseEventasJson(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidRevisionEvent()));
    }