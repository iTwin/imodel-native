/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "EventParserTests.h"
#include <Bentley/Base64Utilities.h>
#include <WebServices/iModelHub/Events/EventParser.h>
#include <WebServices/iModelHub/Events/LockEvent.h>
#include <WebServices/iModelHub/Events/ChangeSetPostPushEvent.h>
#include <WebServices/iModelHub/Events/CodeEvent.h>
#include <WebServices/iModelHub/Events/DeletedEvent.h>
#include <WebServices/iModelHub/Events/VersionEvent.h>

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
              "ReleasedWithChangeSet":"SomeReleasedWithChangeSet"
              }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidChangeSetPostPushEvent()
    {
    return R"(
              {
              "Date":"SomeDate",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "ChangeSetId":"ChangeSetId",
              "ChangeSetIndex":"SomeChangeSetIndex",
			  "BriefcaseId":1
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
              "CodeSpecId":"SomeCodeSpecId",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "CodeScope":"SomeNamespace",
              "Values":["SomeValue1", "SomeValue2", "SomeValue3"],
              "State":1,
              "BriefcaseId":1
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
              "BriefcaseId":1
              }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Andrius.Zonys                   05/2019
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidVersionEvent()
    {
    return R"(
              {
              "Date":"SomeDate",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "VersionId":"SomeVersionId",
              "VersionName":"SomeVersionName",
              "ChangeSetId":"SomeChangeSetId"
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
              "BriefcaseId":1,
              "ReleasedWithChangeSet":"SomeReleasedWithChangeSet"
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
              "BriefcaseId":1,
              "ReleasedWithChangeSet":"SomeReleasedWithChangeSet"
              }
             )";
}

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidChangeSetPostPushEvent()
    {
    return R"(
              {
              "Date":"SomeDate",
              "ChangeSetIndex":"SomeChangeSetIndex"
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
              "CodeSpecId":"SomeCodeSpec",
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
              "CodeSpecId":"SomeCodeSpecId",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "CodeScope":"SomeNamespace",
              "Values":[],
              "State":1,
              "BriefcaseId":1
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
//@bsimethod									Andrius.Zonys                   05/2019
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidVersionEvent()
    {
    return R"(
              {
              "Date":"SomeDate",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "VersionId":"SomeVersionId"
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
Utf8String StubHttpResponseValidChangeSetPostPushEventContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::ChangeSetPostPushEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidChangeSetPostPushEventContentType()
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
//@bsimethod									Andrius.Zonys                   05/2019
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidVersionEventContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::VersionEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Andrius.Zonys                   05/2019
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidVersionEventContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Andrius.Zonys                   05/2019
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidVersionModifiedEventContentType()
    {
    return Event::Helper::GetEventNameFromEventType(Event::EventType::VersionModifiedEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Andrius.Zonys                   05/2019
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidVersionModifiedEventContentType()
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
    EXPECT_TRUE(validPtr.IsValid());
    EXPECT_EQ(Event::EventType::LockEvent, validPtr->GetEventType());
    LockEvent& lockEvent1 = dynamic_cast<LockEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<Event::GenericEvent*>(&lockEvent1)); //LockEvent is a subclass of Event
    RefCountedPtr<LockEvent> lockEvent2 = EventParser::GetLockEvent(validPtr);
    EXPECT_TRUE(lockEvent2.IsValid());
    EXPECT_EQ(lockEvent1.GetLockType(), lockEvent2->GetLockType());
	bvector<Utf8String> objectIds = { "SomeObjectId1", "SomeObjectId2", "SomeObjectId3" };
	EXPECT_EQ(objectIds, lockEvent1.GetObjectIds());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(EventParserTests, ChangeSetPostPushEventTests)
    {
    //Check for valid values
    EventPtr validPtr = EventParser::ParseEvent(StubHttpResponseValidChangeSetPostPushEventContentType().c_str(), StubHttpResponseValidChangeSetPostPushEvent());
    EXPECT_TRUE(validPtr.IsValid());
    EXPECT_EQ(Event::EventType::ChangeSetPostPushEvent, validPtr->GetEventType());
    ChangeSetPostPushEvent& changeSetEvent1 = dynamic_cast<ChangeSetPostPushEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<Event::GenericEvent*>(&changeSetEvent1)); //ChangeSetPostPushEvent is a subclass of Event
    RefCountedPtr<struct ChangeSetPostPushEvent> changeSetEvent2 = EventParser::GetChangeSetPostPushEvent(validPtr);
    EXPECT_TRUE(changeSetEvent2.IsValid());
    EXPECT_EQ(changeSetEvent1.GetChangeSetId(), changeSetEvent2->GetChangeSetId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(EventParserTests, CodeEventTests)
    {
    //Check for valid values
    EventPtr validPtr = EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseValidCodeEvent());
    EXPECT_TRUE(validPtr.IsValid());
    EXPECT_EQ(Event::EventType::CodeEvent, validPtr->GetEventType());
    CodeEvent& codeEvent1 = dynamic_cast<CodeEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<Event::GenericEvent*>(&codeEvent1)); //CodeEvent is a subclass of Event
    RefCountedPtr<struct CodeEvent> codeEvent2 = EventParser::GetCodeEvent(validPtr);
    EXPECT_TRUE(codeEvent2.IsValid());
    EXPECT_EQ(codeEvent1.GetCodeSpecId(), codeEvent2->GetCodeSpecId());
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
    EXPECT_TRUE(validPtr.IsValid());
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
    EXPECT_TRUE(validPtr.IsValid());
    EXPECT_EQ(Event::EventType::AllCodesDeletedEvent, validPtr->GetEventType());
    DeletedEvent& deletedEvent1 = dynamic_cast<DeletedEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<Event::GenericEvent*>(&deletedEvent1)); //DeletedEvent is a subclass of Event
    RefCountedPtr<struct DeletedEvent> deletedEvent2 = EventParser::GetDeletedEvent(validPtr);
    EXPECT_TRUE(deletedEvent2.IsValid());
    EXPECT_EQ(deletedEvent1.GetBriefcaseId(), deletedEvent2->GetBriefcaseId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Andrius.Zonys                   05/2019
//---------------------------------------------------------------------------------------
TEST_F(EventParserTests, VersionEventTests)
    {
    //Check for valid values
    EventPtr validPtr = EventParser::ParseEvent(StubHttpResponseValidVersionEventContentType().c_str(), StubHttpResponseValidVersionEvent());
    EXPECT_TRUE(validPtr.IsValid());
    EXPECT_EQ(Event::EventType::VersionEvent, validPtr->GetEventType());
    VersionEvent& versionEvent1 = dynamic_cast<VersionEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<Event::GenericEvent*>(&versionEvent1)); //VersionEvent is a subclass of Event
    RefCountedPtr<struct VersionEvent> versionEvent2 = EventParser::GetVersionEvent(validPtr);
    EXPECT_TRUE(versionEvent2.IsValid());
    EXPECT_EQ(versionEvent1.GetVersionId(), versionEvent2->GetVersionId());
    EXPECT_EQ(versionEvent1.GetVersionName(), versionEvent2->GetVersionName());
    EXPECT_EQ(versionEvent1.GetChangeSetId(), versionEvent2->GetChangeSetId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Andrius.Zonys                   05/2019
//---------------------------------------------------------------------------------------
TEST_F(EventParserTests, VersionModifiedEventTests)
    {
    //Check for valid values
    EventPtr validPtr = EventParser::ParseEvent(StubHttpResponseValidVersionModifiedEventContentType().c_str(), StubHttpResponseValidVersionEvent());
    EXPECT_TRUE(validPtr.IsValid());
    EXPECT_EQ(Event::EventType::VersionModifiedEvent, validPtr->GetEventType());
    VersionEvent& versionEvent1 = dynamic_cast<VersionEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<Event::GenericEvent*>(&versionEvent1)); //VersionEvent is a subclass of Event
    RefCountedPtr<struct VersionEvent> versionEvent2 = EventParser::GetVersionEvent(validPtr);
    EXPECT_TRUE(versionEvent2.IsValid());
    EXPECT_EQ(versionEvent1.GetVersionId(), versionEvent2->GetVersionId());
    EXPECT_EQ(versionEvent1.GetVersionName(), versionEvent2->GetVersionName());
    EXPECT_EQ(versionEvent1.GetChangeSetId(), versionEvent2->GetChangeSetId());
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
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalidCodeEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalidCodeEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseValidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidChangeSetPostPushEventContentType().c_str(), StubHttpResponseEmpty()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidChangeSetPostPushEventContentType().c_str(), StubHttpResponseEmptyJson()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidChangeSetPostPushEventContentType().c_str(), StubHttpResponseInvalid()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidChangeSetPostPushEventContentType().c_str(), StubHttpResponseInvalidLockEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidChangeSetPostPushEventContentType().c_str(), StubHttpResponseInvalidLockEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidChangeSetPostPushEventContentType().c_str(), StubHttpResponseInvalidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidChangeSetPostPushEventContentType().c_str(), StubHttpResponseInvalidCodeEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidChangeSetPostPushEventContentType().c_str(), StubHttpResponseInvalidCodeEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidChangeSetPostPushEventContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidChangeSetPostPushEventContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseEmpty()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseEmptyJson()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalid()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidLockEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidLockEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidCodeEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidCodeEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseValidChangeSetPostPushEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseEmpty()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseEmptyJson()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalid()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidLockEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidLockEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidCodeEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidCodeEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseValidChangeSetPostPushEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseEmpty()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseEmptyJson()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalid()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidLockEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidLockEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidCodeEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidCodeEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseValidChangeSetPostPushEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionEventContentType().c_str(), StubHttpResponseEmpty()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionEventContentType().c_str(), StubHttpResponseEmptyJson()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionEventContentType().c_str(), StubHttpResponseInvalid()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionEventContentType().c_str(), StubHttpResponseInvalidLockEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionEventContentType().c_str(), StubHttpResponseInvalidLockEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionEventContentType().c_str(), StubHttpResponseInvalidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionEventContentType().c_str(), StubHttpResponseInvalidCodeEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionEventContentType().c_str(), StubHttpResponseInvalidCodeEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionEventContentType().c_str(), StubHttpResponseInvalidVersionEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionModifiedEventContentType().c_str(), StubHttpResponseEmpty()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionModifiedEventContentType().c_str(), StubHttpResponseEmptyJson()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionModifiedEventContentType().c_str(), StubHttpResponseInvalid()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionModifiedEventContentType().c_str(), StubHttpResponseInvalidLockEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionModifiedEventContentType().c_str(), StubHttpResponseInvalidLockEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionModifiedEventContentType().c_str(), StubHttpResponseInvalidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionModifiedEventContentType().c_str(), StubHttpResponseInvalidCodeEvent1()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionModifiedEventContentType().c_str(), StubHttpResponseInvalidCodeEvent2()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseValidVersionModifiedEventContentType().c_str(), StubHttpResponseInvalidVersionEvent()).IsNull());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(EventParserTests, InvalidContentTypeTests)
    {
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseEmptyContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidChangeSetPostPushEventContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType().c_str(), StubHttpResponseValidLockEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseEmptyContentType().c_str(), StubHttpResponseValidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidContentType().c_str(), StubHttpResponseValidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType().c_str(), StubHttpResponseValidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidChangeSetPostPushEventContentType().c_str(), StubHttpResponseValidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType().c_str(), StubHttpResponseValidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType().c_str(), StubHttpResponseValidChangeSetPostPushEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType().c_str(), StubHttpResponseValidChangeSetPostPushEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseEmptyContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidChangeSetPostPushEventContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType().c_str(), StubHttpResponseValidCodeEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseEmptyContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidChangeSetPostPushEventContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType().c_str(), StubHttpResponseValidDeletedEvent()).IsNull());

    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseEmptyContentType().c_str(), StubHttpResponseValidVersionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidContentType().c_str(), StubHttpResponseValidVersionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType().c_str(), StubHttpResponseValidVersionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidChangeSetPostPushEventContentType().c_str(), StubHttpResponseValidVersionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType().c_str(), StubHttpResponseValidVersionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType().c_str(), StubHttpResponseValidVersionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType().c_str(), StubHttpResponseValidVersionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidVersionEventContentType().c_str(), StubHttpResponseValidVersionEvent()).IsNull());
    EXPECT_TRUE(EventParser::ParseEvent(StubHttpResponseInvalidVersionModifiedEventContentType().c_str(), StubHttpResponseValidVersionEvent()).IsNull());
    }
