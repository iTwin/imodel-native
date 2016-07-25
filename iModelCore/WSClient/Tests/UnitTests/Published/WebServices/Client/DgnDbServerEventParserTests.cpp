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
#include <DgnDbServer/Client/DgnDbServerDeletedEvent.h>

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
              "Values":["SomeValue1", "SomeValue2", "SomeValue3"],
              "State":"SomeState",
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
              @string3http://schemas.microsoft.com/2003/10/Serialization/™Ž
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
Utf8String StubHttpResponseInvalidLockEvent2()
{
	return R"(
              @string3http://schemas.microsoft.com/2003/10/Serialization/™Ž
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
Utf8String StubHttpResponseInvalidCodeEvent1()
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
Utf8String StubHttpResponseInvalidCodeEvent2()
{
	return R"(
              @string3http://schemas.microsoft.com/2003/10/Serialization/™Ž
              {
              "Date":"SomeDate",
              "CodeAuthorityId":"SomeCodeAuthorityId",
              "EventTopic":"SomeEventTopic",
              "FromEventSubscriptionId":"SomeFromEventSubscriptionId",
              "Namespace":"SomeNamespace",
              "Values":[],
              "State":"SomeState",
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
              @string3http://schemas.microsoft.com/2003/10/Serialization/™Ž
              {
              "Date":"SomeDate",
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
                                                    "instanceId":"SomeSubscriptionId",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSubscription",
                                                    "properties":
                                                                {
                                                                "EventTypes":["LockEvent", "RevisionEvent"]
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
                                                    "instanceId":"SomeSubscriptionId",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSubscription",
                                                    "properties":
                                                                {
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
Utf8String StubHttpResponseInvalidEventSubscriptionWSObjectResponse1()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeSubscriptionId",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSubscription",
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
Utf8String StubHttpResponseInvalidEventSubscriptionWSObjectResponse2()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeSubscriptionId",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSubscription",
                                                    "properties" :
                                                                 {
                                                                 "Help":""
                                                                 }
                                                    }
                              }
             }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidEventSubscriptionWSObjectResponse3()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeSubscriptionId",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSubscription",
                                                    "properties" :
                                                                 {
                                                                 "EventTypes":"YoYo"
                                                                 }
                                                    }
                              }
             }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidEventSubscriptionWSObjectResponse4()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSubscription",
                                                    "properties" :
                                                                 {
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
Utf8String StubHttpResponseInvalidEventSubscriptionWSChangeSetResponse1()
    {
    return R"(
             {
            "changedInstances":
                              [
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeSubscriptionId",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSubscription",
                                                    "properties":
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
Utf8String StubHttpResponseInvalidEventSubscriptionWSChangeSetResponse2()
    {
    return R"(
             {
            "changedInstances":
                              [
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeSubscriptionId",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSubscription",
                                                    "properties":
                                                                {
                                                                "Help":""
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
Utf8String StubHttpResponseInvalidEventSubscriptionWSChangeSetResponse3()
    {
    return R"(
             {
            "changedInstances":
                              [
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeSubscriptionId",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSubscription",
                                                    "properties":
                                                                {
                                                                "EventTypes":"YoYo"
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
Utf8String StubHttpResponseInvalidEventSubscriptionWSChangeSetResponse4()
    {
    return R"(
             {
            "changedInstances":
                              [
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSubscription",
                                                    "properties":
                                                                {
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
Utf8String StubHttpResponseInvalidEventSASResponse1()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeInstanceId",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSAS",
                                                    "properties":
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
Utf8String StubHttpResponseInvalidEventSASResponse2()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeInstanceId",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSAS",
                                                    "properties":
                                                                {
                                                                "Something1":"",
                                                                "Something2":""                 
                                                                }
                                                    }
                              }
             }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidEventSASResponse3()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeInstanceId",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSAS",
                                                    "properties":
                                                                {
                                                                "EventServiceSASToken":"",
                                                                "BaseAddress":""            
                                                                }
                                                    }
                              }
             }
             )";
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Utf8String StubHttpResponseInvalidEventSASResponse4()
    {
    return R"(
             {
            "changedInstance":
                              {
                              "change":"Created",
                              "instanceAfterChange" :
                                                    {
                                                    "instanceId":"SomeInstanceId",
                                                    "schemaName":"BIMCSRepository",
                                                    "className":"EventSAS",
                                                    "properties":
                                                                {
                                                                "EventServiceSASToken":"SomeSASToken",
                                                                "BaseAddress":""                
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
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidLockEvent());
    EXPECT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::LockEvent, validPtr->GetEventType());
    DgnDbServerLockEvent& lockEvent1 = dynamic_cast<DgnDbServerLockEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&lockEvent1)); //DgnDbServerLockEvent is a subclass of DgnDbServerEvent
    std::shared_ptr<struct DgnDbServerLockEvent> lockEvent2 = DgnDbServerEventParser::GetInstance().GetLockEvent(validPtr);
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
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidRevisionEvent());
    EXPECT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent, validPtr->GetEventType());
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
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::CodeEvent, validPtr->GetEventType());
    DgnDbServerCodeEvent& codeEvent1 = dynamic_cast<DgnDbServerCodeEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&codeEvent1)); //DgnDbServerCodeEvent is a subclass of DgnDbServerEvent
    std::shared_ptr<struct DgnDbServerCodeEvent> codeEvent2 = DgnDbServerEventParser::GetInstance().GetCodeEvent(validPtr);
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
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseValidDeletedEvent());
    EXPECT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::AllLocksDeletedEvent, validPtr->GetEventType());
    DgnDbServerDeletedEvent& deletedEvent1 = dynamic_cast<DgnDbServerDeletedEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&deletedEvent1)); //DgnDbServerDeletedEvent is a subclass of DgnDbServerEvent
    std::shared_ptr<struct DgnDbServerDeletedEvent> deletedEvent2 = DgnDbServerEventParser::GetInstance().GetDeletedEvent(validPtr);
    EXPECT_NE(nullptr, deletedEvent2);
    EXPECT_EQ(deletedEvent1.GetBriefcaseId(), deletedEvent2->GetBriefcaseId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, CodesDeletedEventTests)
    {
    //Check for valid values
    DgnDbServerEventPtr validPtr = DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseValidDeletedEvent());
    EXPECT_NE(nullptr, validPtr);
    EXPECT_EQ(DgnDbServerEvent::DgnDbServerEventType::AllCodesDeletedEvent, validPtr->GetEventType());
    DgnDbServerDeletedEvent& deletedEvent1 = dynamic_cast<DgnDbServerDeletedEvent&>(*validPtr);
    EXPECT_TRUE(dynamic_cast<DgnDbServerEvent::GenericEvent*>(&deletedEvent1)); //DgnDbServerDeletedEvent is a subclass of DgnDbServerEvent
    std::shared_ptr<struct DgnDbServerDeletedEvent> deletedEvent2 = DgnDbServerEventParser::GetInstance().GetDeletedEvent(validPtr);
    EXPECT_NE(nullptr, deletedEvent2);
    EXPECT_EQ(deletedEvent1.GetBriefcaseId(), deletedEvent2->GetBriefcaseId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, InvalidEventTests)
    {
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidLockEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidRevisionEvent()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidCodeEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLockEventContentType(), StubHttpResponseValidCodeEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidLockEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidRevisionEvent()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidCodeEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidRevisionEventContentType(), StubHttpResponseValidCodeEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalidLockEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalidRevisionEvent()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalidCodeEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodeEventContentType(), StubHttpResponseValidRevisionEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseInvalidLockEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseInvalidRevisionEvent()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseInvalidCodeEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidLocksDeletedContentType(), StubHttpResponseValidRevisionEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseEmpty()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseEmptyJson()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseInvalid()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseInvalidLockEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseInvalidLockEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseInvalidRevisionEvent()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseInvalidCodeEvent1()));
	EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseInvalidCodeEvent2()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseValidCodesDeletedContentType(), StubHttpResponseValidRevisionEvent()));
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
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidLocksDeletedContentType(), StubHttpResponseValidLockEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidCodesDeletedContentType(), StubHttpResponseValidLockEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseEmptyContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidRevisionEventContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidCodeEventContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidLocksDeletedContentType(), StubHttpResponseValidRevisionEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidCodesDeletedContentType(), StubHttpResponseValidRevisionEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseEmptyContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidRevisionEventContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidCodeEventContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidLocksDeletedContentType(), StubHttpResponseValidCodeEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidCodesDeletedContentType(), StubHttpResponseValidCodeEvent()));

    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseEmptyContentType(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidContentType(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidLockEventContentType(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidRevisionEventContentType(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidCodeEventContentType(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidLocksDeletedContentType(), StubHttpResponseValidDeletedEvent()));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEvent(StubHttpResponseInvalidCodesDeletedContentType(), StubHttpResponseValidDeletedEvent()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod									Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnDbServerEventParserTests, GenerateEventSubscriptionWSObjectJsonTest)
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
TEST_F(DgnDbServerEventParserTests, GenerateEventSASJsonTest)
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

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSubscriptionWSObjectResponse1(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSubscriptionWSObjectResponse2(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSubscriptionWSObjectResponse3(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSubscriptionWSObjectResponse4(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSubscriptionWSChangeSetResponse1(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSubscriptionWSChangeSetResponse2(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSubscriptionWSChangeSetResponse3(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSubscriptionWSChangeSetResponse4(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSubscription(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSASResponse1(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSAS(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSASResponse2(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSAS(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSASResponse3(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSAS(generatedStubJson));

    EXPECT_TRUE(reader.parse(StubHttpResponseInvalidEventSASResponse4(), generatedStubJson));
    EXPECT_EQ(nullptr, DgnDbServerEventParser::GetInstance().ParseEventSAS(generatedStubJson));
    }