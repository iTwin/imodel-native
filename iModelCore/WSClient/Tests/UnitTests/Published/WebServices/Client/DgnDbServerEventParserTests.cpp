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
#include <DgnDbServer/Client/DgnDbServerCodeEvent.h>

using namespace ::testing;
using namespace ::std;

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
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
              @string3http://schemas.microsoft.com/2003/10/Serialization/™Ž
              {
              "Date":"SomeDate",
              "LockType":"SomeLockType",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "ObjectId":"SomeObjectId",
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
              @string3http://schemas.microsoft.com/2003/10/Serialization/™Ž
              {
              "Date":"SomeDate",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "RevisionId":"RevisionId",
              "RevisionIndex":"SomeRevisionIndex"
              }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidCodeEvent()
    {
    return R"(
              @string3http://schemas.microsoft.com/2003/10/Serialization/™Ž
              {
              "Date":"SomeDate",
              "CodeAuthorityId":"SomeCodeAuthorityId",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "Namespace":"SomeNamespace",
              "Value":"SomeValue",
              "State":"SomeState",
              "BriefcaseId":"SomeBriefcaseId",
              "UsedWithRevision":"SomeUsedWithRevision"
              }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidCodeEvent()
    {
    return R"(
              @string3http://schemas.microsoft.com/2003/10/Serialization/™Ž
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
Utf8String StubHttpResponseValidEventSubscriptionWSObjectResponse()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeInstanceId",
                                                    "schemaName" : "BIMCSRepository",
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

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseValidEventSubscriptionWSChangeSetResponse()
    {
    return R"(
             {
            "changedInstances":
                              [
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeInstanceId",
                                                    "schemaName" : "BIMCSRepository",
                                                    "className" : "EventSubscription",
                                                    "properties" :
                                                                 {
                                                                 "Id":"SomeSubscriptionId",
                                                                 "TopicName" : "SomeTopicName",
                                                                 "EventTypes" : ["LockEvent", "RevisionEvent"]
                                                                 }
                                                    }
                              }
                              ]
             }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
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
                                                    "schemaName" : "BIMCSRepository",
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

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidEventSubscriptionWSObjectResponse()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeInstanceId",
                                                    "schemaName" : "BIMCSRepository",
                                                    "className" : "EventSubscription",
                                                    "properties" :
                                                                 {
                                                                 }
                                                    }
                              }
             }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidEventSubscriptionWSChangeSetResponse()
    {
    return R"(
             {
            "changedInstances":
                              [
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeInstanceId",
                                                    "schemaName" : "BIMCSRepository",
                                                    "className" : "EventSubscription",
                                                    "properties" :
                                                                 {
                                                                 }
                                                    }
                              }
                              ]
             }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
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
                                                    "schemaName" : "BIMCSRepository",
                                                    "className" : "EventSAS",
                                                    "properties" :
                                                                 {
                                                                 }
                                                    }
                              }
             }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubGenerateValidEventSASJson()
    {
    return R"(
             {
             "instance":
                       {
			           "instanceId":"",
			           "schemaName":"BIMCSRepository",
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

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubGenerateInvalidEventSASJson()
    {
    return R"(
             {
             "instance":
                       {
			           "instanceId":"",
			           "schemaName":"BIMCSRepository",
			           "className":"EventSAS",
			            "properties":
						            {
						            }
			            }
             } 
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubGenerateValidEventSubscriptionWSObjectJsonSingleEvent()
    {
    return R"(
             {
             "instance":
                       {
			           "instanceId":"",
			           "schemaName":"BIMCSRepository",
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

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubGenerateValidEventSubscriptionWSObjectJsonNoEvent()
    {
    return R"(
             {
             "instance":
                       {
			           "instanceId":"",
			           "schemaName":"BIMCSRepository",
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

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubGenerateValidEventSubscriptionWSChangeSetJsonSingleEvent()
    {
    return R"(
			 {
			 "Id":"",
			 "TopicName":"", 
			 "EventTypes": ["LockEvent"]
			 }          
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubGenerateValidEventSubscriptionWSChangeSetJsonNoEvent()
    {
    return R"(
			 {
			 "Id":"",
			 "TopicName":"", 
			 "EventTypes": [] 
			 }                   
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubGenerateInvalidEventSubscriptionWSObjectJson()
    {
    return R"(
             {
             "instance":
                       {
			           "instanceId":"",
			           "schemaName":"BIMCSRepository",
			           "className":"EventSubscription",
			           "properties":
						           {
						           }
			           }
             } 
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubGenerateInvalidEventSubscriptionWSChangeSetJson()
    {
    return R"(
             {
			 "properties":
						 {
						 }	           
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
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidLockEvent());
    EXPECT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::LockEvent, DgnDbServerEventParser::GetInstance().GetEventType(validPtr));
    DgnDbServerLockEvent& lockEvent1 = dynamic_cast<DgnDbServerLockEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&lockEvent1)); //DgnDbServerLockEvent is a subclass of DgnDbServerEvent
    std::shared_ptr<struct DgnDbServerLockEvent> lockEvent2 = DgnDbServerEventParser::GetInstance().GetLockEvent(validPtr);
    EXPECT_NE(nullptr, lockEvent2);
    EXPECT_EQ(lockEvent1.GetLockType(), lockEvent2->GetLockType());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, RevisionEventTests)
    {
    //Check for valid values
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidRevisionEvent());
    EXPECT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent, DgnDbServerEventParser::GetInstance().GetEventType(validPtr));
    DgnDbServerRevisionEvent& revisionEvent1 = dynamic_cast<DgnDbServerRevisionEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&revisionEvent1)); //DgnDbServerRevisionEvent is a subclass of DgnDbServerEvent
    std::shared_ptr<struct DgnDbServerRevisionEvent> revisionEvent2 = DgnDbServerEventParser::GetInstance().GetRevisionEvent(validPtr);
    EXPECT_NE(nullptr, revisionEvent2);
    EXPECT_EQ(revisionEvent1.GetRevisionId(), revisionEvent2->GetRevisionId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, CodeEventTests)
    {
    //Check for valid values
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseValidCodeEvent());
    EXPECT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::CodeEvent, DgnDbServerEventParser::GetInstance().GetEventType(validPtr));
    DgnDbServerCodeEvent& codeEvent1 = dynamic_cast<DgnDbServerCodeEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&codeEvent1)); //DgnDbServerCodeEvent is a subclass of DgnDbServerEvent
    std::shared_ptr<struct DgnDbServerCodeEvent> codeEvent2 = DgnDbServerEventParser::GetInstance().GetCodeEvent(validPtr);
    EXPECT_NE(nullptr, codeEvent2);
    EXPECT_EQ(codeEvent1.GetCodeAuthorityId(), codeEvent2->GetCodeAuthorityId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, InvalidEventTests)
    {
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidCodeEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidCodeEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseValidRevisionEvent()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, InvalidContentTypeTests)
    {
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseEmptyContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidRevisionEventContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidCodeEventContentType(), StubHttpResponseValidLockEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseEmptyContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidRevisionEventContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidCodeEventContentType(), StubHttpResponseValidRevisionEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseEmptyContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidRevisionEventContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidCodeEventContentType(), StubHttpResponseValidCodeEvent()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, GenerateEventSubscriptionWSObjectJsonTest) //problem
    {
    Json::Reader reader;
    Json::Value generatedStubJson(Json::objectValue);

    //No event
    EXPECT_TRUE(reader.parse(StubGenerateValidEventSubscriptionWSObjectJsonNoEvent(), generatedStubJson));
    Json::Value actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionWSObjectJson();
    EXPECT_EQ(0, actualGeneratedJson.compare(generatedStubJson));

    //Single Event
    EXPECT_TRUE(reader.parse(StubGenerateValidEventSubscriptionWSObjectJsonSingleEvent(), generatedStubJson));
    bvector<DgnDbServerEvent::DgnDbServerEventType> eventTypes;
    eventTypes.push_back(DgnDbServerEvent::DgnDbServerEventType::LockEvent);
    actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionWSObjectJson(&eventTypes);
    EXPECT_EQ(0, actualGeneratedJson.compare(generatedStubJson));
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, GenerateEventSubscriptionWSChangesetTest)
    {
    Json::Reader reader;
    Json::Value generatedStubJson(Json::objectValue);

    //No event
    EXPECT_TRUE(reader.parse(StubGenerateValidEventSubscriptionWSChangeSetJsonNoEvent(), generatedStubJson));
    Json::Value actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionWSChangeSetJson();
    EXPECT_EQ(0, actualGeneratedJson.compare(generatedStubJson));

    //Single Event
    EXPECT_TRUE(reader.parse(StubGenerateValidEventSubscriptionWSChangeSetJsonSingleEvent(), generatedStubJson));
    bvector<DgnDbServerEvent::DgnDbServerEventType> eventTypes;
    eventTypes.push_back(DgnDbServerEvent::DgnDbServerEventType::LockEvent);
    actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionWSChangeSetJson(&eventTypes);
    EXPECT_EQ(0, actualGeneratedJson.compare(generatedStubJson));
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, GenerateEventSASJsonTest) //problem
    {
    Json::Reader reader;
    Json::Value generatedStubJson(Json::objectValue);

    EXPECT_TRUE(reader.parse(StubGenerateValidEventSASJson(), generatedStubJson));
    Json::Value actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSASJson();
    EXPECT_EQ(0, actualGeneratedJson.compare(generatedStubJson));
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
//Need more error cases
TEST_F(DgnDbServerEventParserTests, InvalidGenerateJsonTests)
    {
    Json::Reader reader;
    Json::Value generatedStubJson(Json::objectValue);

    //Invalid SAS generation
    EXPECT_TRUE(reader.parse(StubGenerateInvalidEventSASJson(), generatedStubJson));
    Json::Value actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSASJson();
    EXPECT_NE(0, actualGeneratedJson.compare(generatedStubJson));

    //Invalid WSObject Subscription Generation
    EXPECT_TRUE(reader.parse(StubGenerateInvalidEventSubscriptionWSObjectJson(), generatedStubJson));
    actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionWSObjectJson();
    EXPECT_NE(0, actualGeneratedJson.compare(generatedStubJson));

    bvector<DgnDbServerEvent::DgnDbServerEventType> eventTypes1, eventTypes2;
    eventTypes1.push_back(DgnDbServerEvent::DgnDbServerEventType::LockEvent);
    actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionWSObjectJson(&eventTypes1);
    EXPECT_NE(0, actualGeneratedJson.compare(generatedStubJson));

    eventTypes2.push_back(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent);
    actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionWSObjectJson(&eventTypes2);
    EXPECT_NE(0, actualGeneratedJson.compare(generatedStubJson));

    //Invalid WSChangeSet Subscription Generation
    EXPECT_TRUE(reader.parse(StubGenerateInvalidEventSubscriptionWSChangeSetJson(), generatedStubJson));
    actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionWSChangeSetJson();
    EXPECT_NE(0, actualGeneratedJson.compare(generatedStubJson));

    actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionWSChangeSetJson(&eventTypes1);
    EXPECT_NE(0, actualGeneratedJson.compare(generatedStubJson));

    actualGeneratedJson = DgnDbServerEventParser::GetInstance().GenerateEventSubscriptionWSChangeSetJson(&eventTypes2);
    EXPECT_NE(0, actualGeneratedJson.compare(generatedStubJson));
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, EventSubscriptionWSObjectResponseTest)
    {
    Json::Reader reader;
    Json::Value generatedStubJson(Json::objectValue);

    EXPECT_TRUE(reader.parse(StubHttpResponseValidEventSubscriptionWSObjectResponse(), generatedStubJson));
    DgnDbServerEventSubscriptionPtr ptr = DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson);
    EXPECT_NE(nullptr, ptr);
    EXPECT_EQ(0, BeStringUtilities::Stricmp("SomeSubscriptionId", ptr->GetSubscriptionId().c_str()));
    EXPECT_EQ(0, BeStringUtilities::Stricmp("SomeTopicName", ptr->GetTopicName().c_str()));
    bvector<DgnDbServerEvent::DgnDbServerEventType> eventTypes = ptr->GetEventTypes();
    bool isSuccess = false;
    for (auto eventType : eventTypes)
        {
        if  (
            DgnDbServerEvent::DgnDbServerEventType::LockEvent == eventType || 
            DgnDbServerEvent::DgnDbServerEventType::RevisionEvent == eventType
            )
            isSuccess = true;       
        else
            {
            isSuccess = false;
            break;
            }
        }
    EXPECT_TRUE(isSuccess);
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, EventSubscriptionWSChangeSetResponseTest)
    {
    Json::Reader reader;
    Json::Value generatedStubJson(Json::objectValue);

    EXPECT_TRUE(reader.parse(StubHttpResponseValidEventSubscriptionWSChangeSetResponse(), generatedStubJson));
    DgnDbServerEventSubscriptionPtr ptr = DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson);
    EXPECT_NE(nullptr, ptr);
    EXPECT_EQ(0, BeStringUtilities::Stricmp("SomeSubscriptionId", ptr->GetSubscriptionId().c_str()));
    EXPECT_EQ(0, BeStringUtilities::Stricmp("SomeTopicName", ptr->GetTopicName().c_str()));
    bvector<DgnDbServerEvent::DgnDbServerEventType> eventTypes = ptr->GetEventTypes();
    bool isSuccess = false;
    for (auto eventType : eventTypes)
        {
        if (
            DgnDbServerEvent::DgnDbServerEventType::LockEvent == eventType ||
            DgnDbServerEvent::DgnDbServerEventType::RevisionEvent == eventType
            )
            isSuccess = true;
        else
            {
            isSuccess = false;
            break;
            }
        }
    EXPECT_TRUE(isSuccess);
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
//Need more error cases -- Add more Invalid Stub responses
TEST_F(DgnDbServerEventParserTests, InvalidEventSubscriptionAndSASResponseTests)
    {
    Json::Reader reader;
    Json::Value generatedStubJson(Json::objectValue);

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSubscriptionWSObjectResponse(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSubscriptionWSChangeSetResponse(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSASResponse(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSAS(generatedStubJson));
    }