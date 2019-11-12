/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct EventsTests : public iModelTestsBase
    {
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();
        }
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                Algirdas.Mikoliunas                  02/2018
//---------------------------------------------------------------------------------------
void WaitForEventsCount(int currentCount1, int expectedCount1, int currentCount2, int expectedCount2)
    {
    for (int i = 0; i < 20; i++)
        {
        if (currentCount1 == expectedCount1 && currentCount2 == expectedCount2)
            break;

        BeThreadUtilities::BeSleep(500);
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Arvind.Venkateswaran                  08/2016
//---------------------------------------------------------------------------------------
TEST_F(EventsTests, SubscribeTests)
    {
    BriefcasePtr briefcase = AcquireAndOpenBriefcase();

    //Set EventTypes
    bset<Event::EventType> eventTypes;
    eventTypes.insert(Event::EventType::ChangeSetPostPushEvent);
    eventTypes.insert(Event::EventType::LockEvent);

    EventCallbackPtr callback = std::make_shared<EventCallback>([=] (EventPtr event) {});

    //Get result
    auto result = briefcase->SubscribeEventsCallback(&eventTypes, callback)->GetResult();
    ASSERT_SUCCESS(result);

    //Subscribe Again
    result = briefcase->SubscribeEventsCallback(&eventTypes, callback)->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::EventCallbackAlreadySubscribed, result.GetError().GetId());

    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback)->GetResult());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Arvind.Venkateswaran                  08/2016
//---------------------------------------------------------------------------------------
TEST_F(EventsTests, UnsubscribeTests)
    {
    BriefcasePtr briefcase = AcquireAndOpenBriefcase();
    briefcase->GetiModelConnection().GetChangeSetCacheManager().EnableBackgroundDownload()->GetResult();
    EventCallbackPtr callback = std::make_shared<EventCallback>([=] (EventPtr event) {});

    //Get result
    auto unsubscribeResult = briefcase->UnsubscribeEventsCallback(callback)->GetResult();
    ASSERT_FAILURE(unsubscribeResult);
    EXPECT_EQ(Error::Id::EventCallbackNotFound, unsubscribeResult.GetError().GetId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Arvind.Venkateswaran                  08/2016
//---------------------------------------------------------------------------------------
TEST_F(EventsTests, SingleCallbackTest)
    {
    BriefcasePtr briefcase = AcquireAndOpenBriefcase();

    bset<Event::EventType> eventTypes;
    eventTypes.insert(Event::EventType::CodeEvent);
    eventTypes.insert(Event::EventType::LockEvent);
    eventTypes.insert(Event::EventType::ChangeSetPrePushEvent);
    eventTypes.insert(Event::EventType::ChangeSetPostPushEvent);
    eventTypes.insert(Event::EventType::VersionEvent);
    eventTypes.insert(Event::EventType::VersionModifiedEvent);

    //Subscribe, unsubscribe
    EventCallbackPtr callback1 = std::make_shared<EventCallback>([=] (EventPtr event) {});
    auto result = briefcase->SubscribeEventsCallback(&eventTypes, callback1)->GetResult();
    ASSERT_SUCCESS(result);
    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback1)->GetResult());

    static int callbackNum = 0;
    static int codeEventCallbackNum = 0;
    static int lockEventCallbackNum = 0;
    static int changeSetPrePushEventCallbackNum = 0;
    static int changeSetPostPushEventCallbackNum = 0;
    static int versionEventCallbackNum = 0;
    static int versionModifiedEventCallbackNum = 0;

    EventCallbackPtr callback = std::make_shared<EventCallback>([=] (EventPtr event)
        {
        auto eventType = event->GetEventType();

        if (Event::EventType::CodeEvent == eventType)
            codeEventCallbackNum++;
        else if (Event::EventType::LockEvent == eventType)
            lockEventCallbackNum++;
        else if (Event::EventType::ChangeSetPrePushEvent == eventType)
            changeSetPrePushEventCallbackNum++;
        else if (Event::EventType::ChangeSetPostPushEvent == eventType)
            changeSetPostPushEventCallbackNum++;
        else if (Event::EventType::VersionEvent == eventType)
            versionEventCallbackNum++;
        else if (Event::EventType::VersionModifiedEvent == eventType)
            versionModifiedEventCallbackNum++;
        callbackNum++;
        });

    //Test events callbacks
    EXPECT_SUCCESS(briefcase->SubscribeEventsCallback(&eventTypes, callback)->GetResult());
    iModelHubHelpers::AddChangeSets(briefcase, 1, 0, false, true, "EventsTestsSingleCallbackTest");
    auto changeSets = s_connection->GetAllChangeSets()->GetResult().GetValue();
    EXPECT_FALSE(changeSets.empty());
    auto versionManager = s_connection->GetVersionsManager();
    VersionInfoPtr version = new VersionInfo("Name", "", changeSets.at(0)->GetId());
    auto versionResult = versionManager.CreateVersion(*version)->GetResult();
    EXPECT_SUCCESS(versionResult);
    version = versionResult.GetValue();
    version->SetName("NewName");
    EXPECT_SUCCESS(versionManager.UpdateVersion(*version)->GetResult());

    WaitForEventsCount(callbackNum, 13, 0, 0);

    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback)->GetResult());
    EXPECT_EQ(13, callbackNum);

    EXPECT_EQ(6, codeEventCallbackNum);
    EXPECT_EQ(3, lockEventCallbackNum);
    EXPECT_EQ(1, changeSetPrePushEventCallbackNum);
    EXPECT_EQ(1, changeSetPostPushEventCallbackNum);
    EXPECT_EQ(1, versionEventCallbackNum);
    EXPECT_EQ(1, versionModifiedEventCallbackNum);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Algirdas.Mikoliunas                  12/2016
//---------------------------------------------------------------------------------------
TEST_F(EventsTests, MultipleCallbacksTest)
    {
    //Prepare imodel and acquire a briefcase
    auto briefcase = AcquireAndOpenBriefcase();

    //Test LockEvent callbacks
    static int callbackNum1 = 0;
    EventCallbackPtr callback1 = std::make_shared<EventCallback>([=] (EventPtr event)
        {
        auto eventType = event->GetEventType();
        EXPECT_EQ(Event::EventType::LockEvent, eventType);
        callbackNum1++;
        });

    bset<Event::EventType> eventTypes;
    eventTypes.insert(Event::EventType::LockEvent);
    EXPECT_SUCCESS(briefcase->SubscribeEventsCallback(&eventTypes, callback1)->GetResult());

    //Test ChangeSetPostPushEvent callbacks
    static int callbackNum2 = 0;
    EventCallbackPtr callback2 = std::make_shared<EventCallback>([=] (EventPtr event)
        {
        auto eventType = event->GetEventType();
        EXPECT_EQ(Event::EventType::ChangeSetPostPushEvent, eventType);
        callbackNum2++;
        });

    bset<Event::EventType> eventTypes2;
    eventTypes2.insert(Event::EventType::ChangeSetPostPushEvent);
    EXPECT_SUCCESS(briefcase->SubscribeEventsCallback(&eventTypes2, callback2)->GetResult());

    // Act and wait for events
    iModelHubHelpers::AddChangeSets(briefcase);
    WaitForEventsCount(callbackNum1, 3, callbackNum2, 1);

    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback1)->GetResult());
    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback2)->GetResult());
    EXPECT_EQ(3, callbackNum1);
    EXPECT_EQ(1, callbackNum2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Algirdas.Mikoliunas                  12/2016
//---------------------------------------------------------------------------------------
TEST_F(EventsTests, MultipleBriefcasesTest)
    {
    //Prepare imodel and acquire a briefcase
    auto briefcase = AcquireAndOpenBriefcase();
    auto briefcase2 = AcquireAndOpenBriefcase();

    //Briefcase1 LockEvent callbacks
    static int callbackNum1 = 0;
    EventCallbackPtr callback1 = std::make_shared<EventCallback>([=] (EventPtr event)
        {
        auto eventType = event->GetEventType();
        EXPECT_EQ(Event::EventType::LockEvent, eventType);
        callbackNum1++;
        });

    bset<Event::EventType> eventTypes;
    eventTypes.insert(Event::EventType::LockEvent);
    EXPECT_SUCCESS(briefcase->SubscribeEventsCallback(&eventTypes, callback1)->GetResult());

    //Briefcase2 LockEvent callbacks
    static int callbackNum2 = 0;
    EventCallbackPtr callback2 = std::make_shared<EventCallback>([=] (EventPtr event)
        {
        auto eventType = event->GetEventType();
        EXPECT_EQ(Event::EventType::LockEvent, eventType);
        callbackNum2++;
        });

    bset<Event::EventType> eventTypes2;
    eventTypes2.insert(Event::EventType::LockEvent);
    EXPECT_SUCCESS(briefcase2->SubscribeEventsCallback(&eventTypes2, callback2)->GetResult());

    // Act and wait for events
    iModelHubHelpers::AddChangeSets(briefcase);
    WaitForEventsCount(callbackNum1, 2, callbackNum2, 2);

    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback1)->GetResult());
    EXPECT_SUCCESS(briefcase2->UnsubscribeEventsCallback(callback2)->GetResult());
    EXPECT_EQ(2, callbackNum1);
    EXPECT_EQ(2, callbackNum2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Algirdas.Mikoliunas                  12/2016
//---------------------------------------------------------------------------------------
TEST_F(EventsTests, UnsubscribedNotCalledTest)
    {
    //Prepare imodel and acquire a briefcase
    auto briefcase = AcquireAndOpenBriefcase();
    auto briefcase2 = AcquireAndOpenBriefcase();

    //Briefcase1 LockEvent callbacks
    static int callbackNum1 = 0;
    EventCallbackPtr callback1 = std::make_shared<EventCallback>([=] (EventPtr event)
        {
        auto eventType = event->GetEventType();
        EXPECT_EQ(Event::EventType::LockEvent, eventType);
        callbackNum1++;
        });

    bset<Event::EventType> eventTypes;
    eventTypes.insert(Event::EventType::LockEvent);
    EXPECT_SUCCESS(briefcase->SubscribeEventsCallback(&eventTypes, callback1)->GetResult());

    //Briefcase2 LockEvent callbacks
    static int callbackNum2 = 0;
    EventCallbackPtr callback2 = std::make_shared<EventCallback>([=] (EventPtr event)
        {
        auto eventType = event->GetEventType();
        EXPECT_EQ(Event::EventType::LockEvent, eventType);
        callbackNum2++;
        });

    bset<Event::EventType> eventTypes2;
    eventTypes2.insert(Event::EventType::LockEvent);
    EXPECT_SUCCESS(briefcase2->SubscribeEventsCallback(&eventTypes2, callback2)->GetResult());

    // Unsubscribe first callback
    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback1)->GetResult());

    // Act and wait for events
    iModelHubHelpers::AddChangeSets(briefcase);
    WaitForEventsCount(callbackNum1, 0, callbackNum2, 2);

    EXPECT_SUCCESS(briefcase2->UnsubscribeEventsCallback(callback2)->GetResult());
    EXPECT_EQ(0, callbackNum1);
    EXPECT_EQ(2, callbackNum2);
    }
