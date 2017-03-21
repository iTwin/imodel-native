/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Client/DgnDbServerEventParserTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifdef DGNDBSERVER_UNIT_TESTS
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
              "CodeAuthorityId":"SomeCodeAuthorityId",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "Namespace":"SomeNamespace",
              "Values":["SomeValue1", "SomeValue2", "SomeValue3"],
              "Reserved":"True",
              "Used":"False",
              "BriefcaseId":3,
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
              "BriefcaseId":7,
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
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::LockEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidLockEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidRevisionEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidRevisionEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidCodeEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::CodeEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidCodeEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidLocksDeletedContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::AllLocksDeletedEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidCodesDeletedContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::AllCodesDeletedEvent).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidLocksDeletedContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::UnknownEventType).c_str();
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidCodesDeletedContentType()
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
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseValidLockEvent());
    EXPECT_NE(nullptr, validPtr);
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
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseValidRevisionEvent());
    EXPECT_NE(nullptr, validPtr);
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
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseValidCodeEvent());
    EXPECT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::CodeEvent, validPtr->GetEventType());
    DgnDbServerCodeEvent& codeEvent1 = dynamic_cast<DgnDbServerCodeEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&codeEvent1)); //DgnDbServerCodeEvent is a subclass of DgnDbServerEvent
    std::shared_ptr<struct DgnDbServerCodeEvent> codeEvent2 = DgnDbServerEventParser::GetCodeEvent(validPtr);
    EXPECT_NE(nullptr, codeEvent2);
    EXPECT_STREQ(codeEvent1.GetCodeSpecId().c_str(), codeEvent2->GetCodeSpecId().c_str());
    bvector<Utf8String> values = { "SomeValue1", "SomeValue2", "SomeValue3" };
    EXPECT_EQ(values, codeEvent1.GetValues());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, LocksDeletedEventTests)
    {
    //Check for valid values
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseValidDeletedEvent());
    EXPECT_NE(nullptr, validPtr);
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
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseValidDeletedEvent());
    EXPECT_NE(nullptr, validPtr);
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
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalidLockEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalidRevisionEvent()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalidCodeEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLockEventContentType().c_str(), StubHttpResponseValidCodeEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseInvalidLockEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseInvalidRevisionEvent()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseInvalidCodeEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidRevisionEventContentType().c_str(), StubHttpResponseValidCodeEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidLockEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidRevisionEvent()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidCodeEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodeEventContentType().c_str(), StubHttpResponseValidRevisionEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidLockEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidRevisionEvent()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidCodeEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidLocksDeletedContentType().c_str(), StubHttpResponseValidRevisionEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidLockEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidRevisionEvent()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidCodeEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseValidCodesDeletedContentType().c_str(), StubHttpResponseValidRevisionEvent()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, InvalidContentTypeTests)
    {
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseEmptyContentType().c_str(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidContentType().c_str(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType().c_str(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidRevisionEventContentType().c_str(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType().c_str(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType().c_str(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType().c_str(), StubHttpResponseValidLockEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseEmptyContentType().c_str(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidContentType().c_str(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType().c_str(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidRevisionEventContentType().c_str(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType().c_str(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType().c_str(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType().c_str(), StubHttpResponseValidRevisionEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseEmptyContentType().c_str(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidContentType().c_str(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType().c_str(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidRevisionEventContentType().c_str(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType().c_str(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType().c_str(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType().c_str(), StubHttpResponseValidCodeEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseEmptyContentType().c_str(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidContentType().c_str(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLockEventContentType().c_str(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidRevisionEventContentType().c_str(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodeEventContentType().c_str(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidLocksDeletedContentType().c_str(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::ParseEvent(StubHttpResponseInvalidCodesDeletedContentType().c_str(), StubHttpResponseValidDeletedEvent()));
    }
#endif
