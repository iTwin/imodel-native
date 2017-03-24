/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Client/DgnDbServerEventParserTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnDbServerEventParserTests.h"
#include <Bentley/Base64Utilities.h>
#include <WebServices/Cache/Util/JsonUtil.h>
#include <DgnDbServer/Client/Events/DgnDbServerEventParser.h>
#include <DgnDbServer/Client/Events/DgnDbServerLockEvent.h>
#include <DgnDbServer/Client/Events/DgnDbServerRevisionEvent.h>
#include <DgnDbServer/Client/Events/DgnDbServerCodeEvent.h>
#include <DgnDbServer/Client/Events/DgnDbServerDeletedEvent.h>

using namespace ::testing;
using namespace ::std;

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseEmpty()
    {
    return "";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseEmptyJson()
    {
    return "{}";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalid()
    {
    return "abcd";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidLockEvent()
    {
    return R"(
              {
              "Date":"SomeDate",
              "LockType":"SomeLockType",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "ObjectIds":["SomeObjectId1", "SomeObjectId2", "SomeObjectId3"],
              "LockLevel":"SomeLockLevel",
              "BriefcaseId":"SomeBriefcaseId",
              "ReleasedWithRevision":"SomeReleasedWithRevision"
              }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidRevisionEvent()
    {
    return R"(
              {
              "Date":"SomeDate",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "RevisionId":"RevisionId",
              "RevisionIndex":"SomeRevisionIndex",
              "BriefcaseId":"SomeBriefcaseId"
              }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidCodeEvent()
    {
    return R"(
              {
              "Date":"SomeDate",
              "CodeAuthorityId":"SomeCodeAuthorityId",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "Namespace":"SomeNamespace",
              "Values":["SomeValue1", "SomeValue2", "SomeValue3"],
              "Reserved":"True",
              "Used":"False",
              "BriefcaseId":"SomeBriefcaseId",
              "UsedWithRevision":"SomeUsedWithRevision"
              }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidDeletedEvent()
    {
    return R"(
              {
              "Date":"SomeDate",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "BriefcaseId":"SomeBriefcaseId"
              }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidLockEvent1()
    {
    return R"(
              {
              "Date":"SomeDate",
              "BriefcaseId":"SomeBriefcaseId",
              "ReleasedWithRevision":"SomeReleasedWithRevision"
              }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidLockEvent2()
{
    return R"(
              {
              "Date":"SomeDate",
              "LockType":"SomeLockType",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "ObjectIds":[],
              "LockLevel":"SomeLockLevel",
              "BriefcaseId":"SomeBriefcaseId",
              "ReleasedWithRevision":"SomeReleasedWithRevision"
              }
             )";
}

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidRevisionEvent()
    {
    return R"(
              {
              "Date":"SomeDate",
              "RevisionIndex":"SomeRevisionIndex"
              }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidCodeEvent1()
    {
    return R"(
              {
              "Date":"SomeDate",
              "CodeAuthorityId":"SomeLockType",
              "EventTopic":"SomeEventTopic"
              }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidCodeEvent2()
{
    return R"(
              {
              "Date":"SomeDate",
              "CodeAuthorityId":"SomeCodeAuthorityId",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "Namespace":"SomeNamespace",
              "Values":[],
              "Reserved":"True",
              "Used":"False",
              "BriefcaseId":"SomeBriefcaseId",
              "UsedWithRevision":"SomeUsedWithRevision"
              }
             )";
}

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidDeletedEvent()
    {
    return R"(
              {
              "Date":"SomeDate",
              "EventTopic":"SomeEventTopic"
              }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8CP StubHttpResponseEmptyContentType()
    {
    return "";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8CP StubHttpResponseInvalidContentType()
    {
    return "SomeContentType";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8CP StubHttpResponseValidLockEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::LockEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8CP StubHttpResponseInvalidLockEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8CP StubHttpResponseValidRevisionEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8CP StubHttpResponseInvalidRevisionEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8CP StubHttpResponseValidCodeEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::CodeEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8CP StubHttpResponseInvalidCodeEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8CP StubHttpResponseValidLocksDeletedContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::AllLocksDeletedEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8CP StubHttpResponseValidCodesDeletedContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::AllCodesDeletedEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8CP StubHttpResponseInvalidLocksDeletedContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8CP StubHttpResponseInvalidCodesDeletedContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
void DgnDbServerEventParserTests::SetUp()
    {
    BaseMockHttpHandlerTest::SetUp();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, LockEventTests)
    {
    //Check for valid values as Json
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidLockEvent());
    ASSERT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::LockEvent, validPtr->GetEventType());
    DgnDbServerLockEvent& lockEvent1 = dynamic_cast<DgnDbServerLockEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&lockEvent1)); //DgnDbServerLockEvent is a subclass of DgnDbServerEvent
    std::shared_ptr<struct DgnDbServerLockEvent> lockEvent2 = DgnDbServerEventParser::GetLockEvent(validPtr);
    EXPECT_NE(nullptr, lockEvent2);
    EXPECT_EQ(lockEvent1.GetLockType(), lockEvent2->GetLockType());
    bvector<Utf8String> objectIds = { "SomeObjectId1", "SomeObjectId2", "SomeObjectId3" };
    EXPECT_EQ(objectIds, lockEvent1.GetObjectIds());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, RevisionEventTests)
    {
    //Check for valid values
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidRevisionEvent());
    ASSERT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent, validPtr->GetEventType());
    DgnDbServerRevisionEvent& revisionEvent1 = dynamic_cast<DgnDbServerRevisionEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&revisionEvent1)); //DgnDbServerRevisionEvent is a subclass of DgnDbServerEvent
    std::shared_ptr<struct DgnDbServerRevisionEvent> revisionEvent2 = DgnDbServerEventParser::GetRevisionEvent(validPtr);
    EXPECT_NE(nullptr, revisionEvent2);
    EXPECT_EQ(revisionEvent1.GetRevisionId(), revisionEvent2->GetRevisionId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, CodeEventTests)
    {
    //Check for valid values
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseValidCodeEvent());
    ASSERT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::CodeEvent, validPtr->GetEventType());
    DgnDbServerCodeEvent& codeEvent1 = dynamic_cast<DgnDbServerCodeEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&codeEvent1)); //DgnDbServerCodeEvent is a subclass of DgnDbServerEvent
    std::shared_ptr<struct DgnDbServerCodeEvent> codeEvent2 = DgnDbServerEventParser::GetCodeEvent(validPtr);
    EXPECT_NE(nullptr, codeEvent2);
    EXPECT_EQ(codeEvent1.GetCodeAuthorityId(), codeEvent2->GetCodeAuthorityId());
    bvector<Utf8String> values = { "SomeValue1", "SomeValue2", "SomeValue3" };
    EXPECT_EQ(values, codeEvent1.GetValues());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, LocksDeletedEventTests)
    {
    //Check for valid values
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseValidDeletedEvent());
    ASSERT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::AllLocksDeletedEvent, validPtr->GetEventType());
    DgnDbServerDeletedEvent& deletedEvent1 = dynamic_cast<DgnDbServerDeletedEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&deletedEvent1)); //DgnDbServerDeletedEvent is a subclass of DgnDbServerEvent
    std::shared_ptr<struct DgnDbServerDeletedEvent> deletedEvent2 = DgnDbServerEventParser::GetDeletedEvent(validPtr);
    EXPECT_NE(nullptr, deletedEvent2);
    EXPECT_EQ(deletedEvent1.GetBriefcaseId(), deletedEvent2->GetBriefcaseId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, CodesDeletedEventTests)
    {
    //Check for valid values
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseValidDeletedEvent());
    ASSERT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::AllCodesDeletedEvent, validPtr->GetEventType());
    DgnDbServerDeletedEvent& deletedEvent1 = dynamic_cast<DgnDbServerDeletedEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&deletedEvent1)); //DgnDbServerDeletedEvent is a subclass of DgnDbServerEvent
    std::shared_ptr<struct DgnDbServerDeletedEvent> deletedEvent2 = DgnDbServerEventParser::GetDeletedEvent(validPtr);
    EXPECT_NE(nullptr, deletedEvent2);
    EXPECT_EQ(deletedEvent1.GetBriefcaseId(), deletedEvent2->GetBriefcaseId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, InvalidEventTests)
    {
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidLockEvent1()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidCodeEvent1()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidCodeEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidLockEvent1()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidCodeEvent1()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidCodeEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalidLockEvent1()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalidCodeEvent1()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseValidRevisionEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseInvalidLockEvent1()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseInvalidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseInvalidCodeEvent1()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseValidRevisionEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseInvalidLockEvent1()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseInvalidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseInvalidCodeEvent1()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseValidRevisionEvent()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, InvalidContentTypeTests)
    {
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseEmptyContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidRevisionEventContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType(), StubHttpResponseValidLockEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseEmptyContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidRevisionEventContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType(), StubHttpResponseValidRevisionEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseEmptyContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidRevisionEventContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType(), StubHttpResponseValidCodeEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseEmptyContentType(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidContentType(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidRevisionEventContentType(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType(), StubHttpResponseValidDeletedEvent()));
    }
