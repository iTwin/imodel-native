/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/EventsTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

    EventCallbackPtr callback = std::make_shared<EventCallback>([=] (EventPtr event)
        {
        auto eventType = event->GetEventType();

        if (Event::EventType::CodeEvent == eventType)
            codeEventCallbackNum++;
        if (Event::EventType::LockEvent == eventType)
            lockEventCallbackNum++;
        if (Event::EventType::ChangeSetPrePushEvent == eventType)
            changeSetPrePushEventCallbackNum++;
        if (Event::EventType::ChangeSetPostPushEvent == eventType)
            changeSetPostPushEventCallbackNum++;
        if (Event::EventType::VersionEvent == eventType)
            versionEventCallbackNum++;

        callbackNum++;
        });

    //Test events callbacks
    EXPECT_SUCCESS(briefcase->SubscribeEventsCallback(&eventTypes, callback)->GetResult());
    iModelHubHelpers::AddChangeSets(briefcase);
    auto changeSets = s_connection->GetAllChangeSets()->GetResult().GetValue();
    EXPECT_FALSE(changeSets.empty());
    auto versionManager = s_connection->GetVersionsManager();
    VersionInfoPtr version = new VersionInfo("Name", "", changeSets.at(0)->GetId());
    auto versionResult = versionManager.CreateVersion(*version)->GetResult();
    EXPECT_SUCCESS(versionResult);
    version = versionResult.GetValue();
    version->SetName("NewName");
    EXPECT_SUCCESS(versionManager.UpdateVersion(*version)->GetResult());

    BeThreadUtilities::BeSleep(3000);

    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback)->GetResult());
    EXPECT_EQ(10, callbackNum);

    EXPECT_EQ(2, codeEventCallbackNum);
    EXPECT_EQ(4, lockEventCallbackNum);
    EXPECT_EQ(1, changeSetPrePushEventCallbackNum);
    EXPECT_EQ(1, changeSetPostPushEventCallbackNum);
    EXPECT_EQ(2, versionEventCallbackNum);
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

    for (int i = 0; i < 20; i++)
        {
        if (4 == callbackNum1 && 1 == callbackNum2)
            break;

        BeThreadUtilities::BeSleep(1000);
        }

    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback1)->GetResult());
    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback2)->GetResult());
    EXPECT_EQ(4, callbackNum1);
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
    BeThreadUtilities::BeSleep(3000);

    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback1)->GetResult());
    EXPECT_SUCCESS(briefcase2->UnsubscribeEventsCallback(callback2)->GetResult());
    EXPECT_EQ(4, callbackNum1);
    EXPECT_EQ(4, callbackNum2);
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
    BeThreadUtilities::BeSleep(3000);

    EXPECT_SUCCESS(briefcase2->UnsubscribeEventsCallback(callback2)->GetResult());
    EXPECT_EQ(0, callbackNum1);
    EXPECT_EQ(4, callbackNum2);
    }
