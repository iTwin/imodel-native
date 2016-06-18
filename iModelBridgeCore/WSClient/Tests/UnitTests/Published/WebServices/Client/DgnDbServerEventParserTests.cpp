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
              @string3http://schemas.microsoft.com/2003/10/Serialization/��
              {
              "Date":"SomeDate",
              "LockType":"SomeLockType",
              "EventTopic":"SomeEventTopic",
              "UserId":"SomeUserId",
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
              @string3http://schemas.microsoft.com/2003/10/Serialization/��
              {
              "Date":"SomeDate",
              "EventTopic":"SomeEventTopic",
              "UserId":"SomeUserId",
              "RevisionId":"RevisionId",
              "RevisionIndex":"SomeRevisionIndex"
              }
             )";
    }

Utf8String StubHttpResponseInvalidLockEvent()
    {
    return R"(
              @string3http://schemas.microsoft.com/2003/10/Serialization/��
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
              @string3http://schemas.microsoft.com/2003/10/Serialization/��
              {
              "Date":"SomeDate",
              "RevisionIndex":"SomeRevisionIndex"
              }
             )";
    }

Utf8String StubHttpResponseValidEventSubscriptionResponse()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeInstanceId",
                                                    "schemaName" : "DgnDbServer",
                                                    "className" : "EventSubscription",
                                                    "properties" :
                                                                 {
                                                                 "Id":"SomeSubscriptionId",
                                                                 "TopicName" : "SomeTopicName",
                                                                 "EventTypes" : ["LockEvent", "RevisionEvent"]
                                                                 }
                                                    }
                              }
             }
             )";

    }
Utf8String StubHttpResponseValidEventSASResponse()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeInstanceId",
                                                    "schemaName" : "DgnDbServer",
                                                    "className" : "EventSAS",
                                                    "properties" :
                                                                 {
                                                                 "EventServiceSASToken":"SomeSASToken",
                                                                 "BaseAddress" : "SomeBaseAddress"
                                                                 }
                                                    }
                              }
             }
             )";
    }

Utf8String StubHttpResponseInvalidEventSubscriptionResponse()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeInstanceId",
                                                    "schemaName" : "DgnDbServer",
                                                    "className" : "EventSubscription",
                                                    "properties" :
                                                                 {
                                                                 }
                                                    }
                              }
             }
             )";
    }

Utf8String StubHttpResponseInvalidEventSASResponse()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeInstanceId",
                                                    "schemaName" : "DgnDbServer",
                                                    "className" : "EventSAS",
                                                    "properties" :
                                                                 {
                                                                 }
                                                    }
                              }
             }
             )";
    }

Utf8String StubGenerateValidEventSASJson()
    {
    return R"(
             {
             "instance":
                       {
			           "instanceId":"",
			           "schemaName":"DgnDbServer",
			           "className":"EventSAS",
			            "properties":
						            {
						            "BaseAddress":"",
						            "EventServiceSASToken":""
						            }
			            }
             } 
             )";
    }

Utf8String StubGenerateInvalidEventSASJson()
    {
    return R"(
             {
             "instance":
                       {
			           "instanceId":"",
			           "schemaName":"DgnDbServer",
			           "className":"EventSAS",
			            "properties":
						            {
						            }
			            }
             } 
             )";
    }

Utf8String StubGenerateValidEventSubscriptionJsonSingleEvent()
    {
    return R"(
             {
             "instance":
                       {
			           "instanceId":"",
			           "schemaName":"DgnDbServer",
			           "className":"EventSubscription",
			            "properties":
						            {
						            "Id":"",
						            "TopicName":"", 
						            "EventTypes": ["LockEvent"] 
						            }
			            }
             } 
             )";
    }

Utf8String StubGenerateValidEventSubscriptionJsonNoEvent()
    {
    return R"(
             {
             "instance":
                       {
			           "instanceId":"",
			           "schemaName":"DgnDbServer",
			           "className":"EventSubscription",
			            "properties":
						            {
						            "Id":"",
						            "TopicName":"", 
						            "EventTypes": [] 
						            }
			            }
             } 
             )";
    }

Utf8String StubGenerateInvalidEventSubscriptionJson()
    {
    return R"(
             {
             "instance":
                       {
			           "instanceId":"",
			           "schemaName":"DgnDbServer",
			           "className":"EventSubscription",
			            "properties":
						            {
						            }
			            }
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
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::LockEvent).c_str();
    }

Utf8CP StubHttpResponseInvalidLockEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::UnknownEventType).c_str();
    }

Utf8CP StubHttpResponseValidRevisionEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent).c_str();
    }

Utf8CP StubHttpResponseInvalidRevisionEventContentType()
    {
    return DgnDbServerEvent::Helper::GetEventNameFromEventType(DgnDbServerEvent::DgnDbServerEventType::UnknownEventType).c_str();
    }

void DgnDbServerEventParserTests::SetUp()
    {
    BaseMockHttpHandlerTest::SetUp();
    }

TEST_F(DgnDbServerEventParserTests, LockEventTests)
    {
    //Check for valid values as Json
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidLockEvent());
    EXPECT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::LockEvent, DgnDbServerEventParser::GetInstance().GetEventType(validPtr));
    DgnDbServerLockEvent& lockEvent = dynamic_cast<DgnDbServerLockEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&lockEvent)); //DgnDbServerLockEvent is a subclass of DgnDbServerEvent
    }

TEST_F(DgnDbServerEventParserTests, RevisionEventTests)
    {
    //Check for valid values
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidRevisionEvent());
    EXPECT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent, DgnDbServerEventParser::GetInstance().GetEventType(validPtr));
    DgnDbServerRevisionEvent& revisionEvent = dynamic_cast<DgnDbServerRevisionEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&revisionEvent)); //DgnDbServerRevisionEvent is a subclass of DgnDbServerEvent
    }

TEST_F(DgnDbServerEventParserTests, InvalidEventTests)
    {
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidLockEvent()));
    }

TEST_F(DgnDbServerEventParserTests, InvalidContentTypeTests)
    {
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseEmptyContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseEmptyContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidRevisionEvent()));
    }

TEST_F(DgnDbServerEventParserTests, GenerateEventSubscriptionJsonTest)
    {
    Json::Reader reader;
    Json::Value generatedStubJson(Json::objectValue);

    //No event
    EXPECT_TRUE(reader.parse(StubGenerateValidEventSubscriptionJsonNoEvent(), generatedStubJson));
    Json::Value actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionJson();
    EXPECT_EQ(0, actualGeneratedJson.compare(generatedStubJson));

    //Single Event
    EXPECT_TRUE(reader.parse(StubGenerateValidEventSubscriptionJsonSingleEvent(), generatedStubJson));
    bvector<DgnDbServerEvent::DgnDbServerEventType> eventTypes;
    eventTypes.push_back(DgnDbServerEvent::DgnDbServerEventType::LockEvent);
    actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionJson(&eventTypes);
    EXPECT_EQ(0, actualGeneratedJson.compare(generatedStubJson));
    }

TEST_F(DgnDbServerEventParserTests, GenerateEventSASJsonTest)
    {
    Json::Reader reader;
    Json::Value generatedStubJson(Json::objectValue);

    EXPECT_TRUE(reader.parse(StubGenerateValidEventSASJson(), generatedStubJson));
    Json::Value actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSASJson();
    EXPECT_EQ(0, actualGeneratedJson.compare(generatedStubJson));
    }

//Need more error cases
TEST_F(DgnDbServerEventParserTests, InvalidGenerateJsonTests)
    {
    Json::Reader reader;
    Json::Value generatedStubJson(Json::objectValue);

    //Invalid SAS generation
    EXPECT_TRUE(reader.parse(StubGenerateInvalidEventSASJson(), generatedStubJson));
    Json::Value actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSASJson();
    EXPECT_NE(0, actualGeneratedJson.compare(generatedStubJson));

    //Invalid Subscription Generation
    EXPECT_TRUE(reader.parse(StubGenerateInvalidEventSubscriptionJson(), generatedStubJson));
    actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionJson();
    EXPECT_NE(0, actualGeneratedJson.compare(generatedStubJson));

    bvector<DgnDbServerEvent::DgnDbServerEventType> eventTypes1, eventTypes2;
    eventTypes1.push_back(DgnDbServerEvent::DgnDbServerEventType::LockEvent);
    actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionJson(&eventTypes1);
    EXPECT_NE(0, actualGeneratedJson.compare(generatedStubJson));

    eventTypes2.push_back(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent);
    actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionJson(&eventTypes2);
    EXPECT_NE(0, actualGeneratedJson.compare(generatedStubJson));
    }

TEST_F(DgnDbServerEventParserTests, EventSubscriptionResponseTest)
    {
    Json::Reader reader;
    Json::Value generatedStubJson(Json::objectValue);

    EXPECT_TRUE(reader.parse(StubHttpResponseValidEventSubscriptionResponse(), generatedStubJson));
    DgnDbServerEventSubscriptionPtr ptr = DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson);
    EXPECT_NE(nullptr, ptr);
    EXPECT_EQ(0, BeStringUtilities::Stricmp("SomeSubscriptionId", ptr->GetSubscriptionId().c_str()));
    EXPECT_EQ(0, BeStringUtilities::Stricmp("SomeTopicName", ptr->GetTopicName().c_str()));
    bvector<DgnDbServerEvent::DgnDbServerEventType> eventTypes = ptr->GetEventTypes();
    bool isSuccess = false;
    for (auto eventType : eventTypes)
        {
        if (DgnDbServerEvent::DgnDbServerEventType::LockEvent == eventType)
            {
            isSuccess = true;
            }
        else if (DgnDbServerEvent::DgnDbServerEventType::RevisionEvent == eventType)
            {
            isSuccess = true;
            }
        else
            {
            isSuccess = false;
            break;
            }
        }
    EXPECT_TRUE(isSuccess);
    }

TEST_F(DgnDbServerEventParserTests, EventSASResponseTest)
    {
    Json::Reader reader;
    Json::Value generatedStubJson(Json::objectValue);

    EXPECT_TRUE(reader.parse(StubHttpResponseValidEventSASResponse(), generatedStubJson));
    DgnDbServerEventSASPtr ptr = DgnDbServerEventParser::GetInstance().ParseEventSAS(generatedStubJson);
    EXPECT_NE(nullptr, ptr);
    EXPECT_EQ(0, BeStringUtilities::Stricmp("SomeSASToken", ptr->GetSASToken().c_str()));
    EXPECT_EQ(0, BeStringUtilities::Stricmp("SomeBaseAddress", ptr->GetBaseAddress().c_str()));
    }

//Need more error cases -- Add more Invalid Stub responses
TEST_F(DgnDbServerEventParserTests, InvalidEventResponseTests)
    {
    Json::Reader reader;
    Json::Value generatedStubJson(Json::objectValue);

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSubscriptionResponse(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSASResponse(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSAS(generatedStubJson));
    }