/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Client/DgnDbServerEventParserTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifdef DGNDBSERVER_UNIT_TESTS
#include "EventParserTests.h"
#include <Bentley/Base64Utilities.h>
#include <WebServices/Cache/Util/JsonUtil.h>
#include <WebServices/iModelHub/Events/EventParser.h>
#include <WebServices/iModelHub/Events/LockEvent.h>
#include <WebServices/iModelHub/Events/ChangeSetPostPushEvent.h>
#include <WebServices/iModelHub/Events/CodeEvent.h>
#include <WebServices/iModelHub/Events/DeletedEvent.h>

using namespace ::testing;
using namespace ::std;

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB

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
              "BriefcaseId":1,
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
              "BriefcaseId":2
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
              "EventTopic":"SomeEventTopic",
			  "CodeSpecId":"SomeCodeSpecId",
			  "CodeScope":"SomeCodeScope",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "Values":["SomeValue1", "SomeValue2", "SomeValue3"],
              "State":1,
			  "BriefcaseId":2
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
              "BriefcaseId":4
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
              "BriefcaseId":5,
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
              "BriefcaseId":6,
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
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "Values":[],
              "BriefcaseId":7,
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
Utf8String StubHttpResponseEmptyContentType()
    {
    return "";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidContentType()
    {
    return "SomeContentType";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidLockEventContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::LockEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidLockEventContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidRevisionEventContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::RevisionEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidRevisionEventContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidCodeEventContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::CodeEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidCodeEventContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidLocksDeletedContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::AllLocksDeletedEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidCodesDeletedContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::AllCodesDeletedEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidLocksDeletedContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidCodesDeletedContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
void EventParserTests::SetUp()
    {
    BaseMockHttpHandlerTest::SetUp();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(EventParserTests, LockEventTests)
    {
    //Check for valid values as Json
    EventPtr validPtr = EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseValidLockEvent());
    ASSERT_TRUE(validPtr.IsValid());
    EXPECT_EQ(Event::EventType::LockEvent, validPtr->GetEventType());
    LockEvent& lockEvent1 = dynamic_cast<LockEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<Event::GenericEvent*>(&lockEvent1)); //LockEvent is a subclass of Event
    RefCountedPtr<struct LockEvent> lockEvent2 = EventParser::GetLockEvent(validPtr);
    EXPECT_TRUE(lockEvent2.IsValid());
    EXPECT_EQ(lockEvent1.GetLockType(), lockEvent2->GetLockType());
    bvector<Utf8String> objectIds = { "SomeObjectId1", "SomeObjectId2", "SomeObjectId3" };
    EXPECT_EQ(objectIds, lockEvent1.GetObjectIds());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(EventParserTests, RevisionEventTests)
    {
    //Check for valid values
    EventPtr validPtr = EventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseValidRevisionEvent());
    ASSERT_TRUE(validPtr.IsValid());
    EXPECT_EQ(Event::EventType::RevisionEvent, validPtr->GetEventType());
    ChangeSetPostPushEvent& revisionEvent1 = dynamic_cast<ChangeSetPostPushEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<Event::GenericEvent*>(&revisionEvent1)); //ChangeSetPostPushEvent is a subclass of Event
    RefCountedPtr<struct ChangeSetPostPushEvent> revisionEvent2 = EventParser::GetRevisionEvent(validPtr);
    EXPECT_TRUE(revisionEvent2.IsValid());
    EXPECT_EQ(revisionEvent1.GetRevisionId(), revisionEvent2->GetRevisionId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(EventParserTests, CodeEventTests)
    {
    //Check for valid values
    EventPtr validPtr = EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseValidCodeEvent());
    ASSERT_TRUE(validPtr.IsValid());
    EXPECT_EQ(Event::EventType::CodeEvent, validPtr->GetEventType());
    CodeEvent& codeEvent1 = dynamic_cast<CodeEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<Event::GenericEvent*>(&codeEvent1)); //CodeEvent is a subclass of Event
    RefCountedPtr<struct CodeEvent> codeEvent2 = EventParser::GetCodeEvent(validPtr);
    EXPECT_TRUE(codeEvent2.IsValid());
    EXPECT_STREQ(codeEvent1.GetCodeSpecId().c_str(), codeEvent2->GetCodeSpecId().c_str());
    bvector<Utf8String> values = { "SomeValue1", "SomeValue2", "SomeValue3" };
    EXPECT_EQ(values, codeEvent1.GetValues());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
TEST_F(EventParserTests, LocksDeletedEventTests)
    {
    //Check for valid values
    EventPtr validPtr = EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseValidDeletedEvent());
    ASSERT_TRUE(validPtr.IsValid());
    EXPECT_EQ(Event::EventType::AllLocksDeletedEvent, validPtr->GetEventType());
    DeletedEvent& deletedEvent1 = dynamic_cast<DeletedEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<Event::GenericEvent*>(&deletedEvent1)); //DeletedEvent is a subclass of Event
    RefCountedPtr<struct DeletedEvent> deletedEvent2 = EventParser::GetDeletedEvent(validPtr);
    EXPECT_TRUE(deletedEvent2.IsValid());
    EXPECT_EQ(deletedEvent1.GetBriefcaseId(), deletedEvent2->GetBriefcaseId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
TEST_F(EventParserTests, CodesDeletedEventTests)
    {
    //Check for valid values
    EventPtr validPtr = EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseValidDeletedEvent());
    ASSERT_TRUE(validPtr.IsValid());
    EXPECT_EQ(Event::EventType::AllCodesDeletedEvent, validPtr->GetEventType());
    DeletedEvent& deletedEvent1 = dynamic_cast<DeletedEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<Event::GenericEvent*>(&deletedEvent1)); //DeletedEvent is a subclass of Event
    RefCountedPtr<struct DeletedEvent> deletedEvent2 = EventParser::GetDeletedEvent(validPtr);
    EXPECT_TRUE(deletedEvent2.IsValid());
    EXPECT_EQ(deletedEvent1.GetBriefcaseId(), deletedEvent2->GetBriefcaseId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(EventParserTests, InvalidEventTests)
    {
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseEmpty()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseEmptyJson()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalid()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalidLockEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalidLockEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalidRevisionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalidCodeEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalidCodeEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseValidRevisionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseEmpty()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseEmptyJson()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseInvalid()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseInvalidLockEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseInvalidLockEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseInvalidRevisionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseInvalidCodeEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseInvalidCodeEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseEmpty()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseEmptyJson()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalid()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidLockEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidLockEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidRevisionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidCodeEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidCodeEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseValidRevisionEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseEmpty()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseEmptyJson()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalid()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidLockEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidLockEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidRevisionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidCodeEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidCodeEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseValidRevisionEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseEmpty()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseEmptyJson()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalid()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidLockEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidLockEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidRevisionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidCodeEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidCodeEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseValidRevisionEvent()).IsNull());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(EventParserTests, InvalidContentTypeTests)
    {
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseEmptyContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidRevisionEventContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseEmptyContentType().c_str(), StubHttpResponseValidRevisionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidContentType().c_str(), StubHttpResponseValidRevisionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType().c_str(), StubHttpResponseValidRevisionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidRevisionEventContentType().c_str(), StubHttpResponseValidRevisionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType().c_str(), StubHttpResponseValidRevisionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType().c_str(), StubHttpResponseValidRevisionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType().c_str(), StubHttpResponseValidRevisionEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseEmptyContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidRevisionEventContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseEmptyContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidRevisionEventContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());
    }
#endif
