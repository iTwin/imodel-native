/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/Events_Tests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"
#include "IntegrationTestsHelper.h"
#include <WebServices/iModelHub/Client/Client.h>
#include <WebServices/iModelHub/Client/Configuration.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeThread.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

EventTaskPtr s_eventResult;
static BeMutex s_eventResultLock;

//---------------------------------------------------------------------------------------
//@bsimethod                                     Arvind.Venkateswaran         08/2016
//---------------------------------------------------------------------------------------
struct EventTests : public IntegrationTestsBase
    {
    ClientPtr    m_client;
    iModelInfoPtr m_imodel;
    iModelConnectionPtr m_imodelConnection;

    virtual void SetUp() override
        {
        IntegrationTestsBase::SetUp();
        auto proxy   = ProxyHttpHandler::GetFiddlerProxyIfReachable();
        m_client     = SetUpClient(IntegrationTestSettings::Instance().GetValidAdminCredentials(), proxy);
        m_imodel = CreateNewiModel(*m_client, nullptr);
        m_imodelConnection = ConnectToiModel(*m_client, m_imodel);
        m_pHost->SetRepositoryAdmin(m_client->GetiModelAdmin());
        }

    virtual void TearDown() override
        {
        DeleteiModel(m_projectId, *m_client, *m_imodel);
        m_client = nullptr;
        IntegrationTestsBase::TearDown();
        }

    BriefcasePtr AcquireBriefcase()
        {
        return IntegrationTestsBase::AcquireBriefcase(*m_client, *m_imodel);
        }

    void CreateAndPushModel(BriefcasePtr briefcase)
        {
        auto model = CreateModel("TestModel1", briefcase->GetDgnDb());
        briefcase->GetDgnDb().SaveChanges();
        auto pushResult = briefcase->PullMergeAndPush();
        EXPECT_SUCCESS(pushResult->GetResult());
        }
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                Arvind.Venkateswaran                  08/2016
//---------------------------------------------------------------------------------------
TEST_F(EventTests, SubscribeTests)
    {
    //Prepare imodel and acquire a briefcase
    auto briefcase = AcquireBriefcase();
    
    //Set EventTypes
    bset<Event::EventType> eventTypes;
    eventTypes.insert(Event::EventType::ChangeSetPostPushEvent);
    eventTypes.insert(Event::EventType::LockEvent);

    EventCallbackPtr callback = std::make_shared<EventCallback>([=](EventPtr event) {});

    //Get result
    auto result = briefcase->SubscribeEventsCallback(&eventTypes, callback)->GetResult();
    EXPECT_SUCCESS(result);

    //Subscribe Again
    result = briefcase->SubscribeEventsCallback(&eventTypes, callback)->GetResult();
    EXPECT_EQ(Error::Id::EventCallbackAlreadySubscribed, result.GetError().GetId());
    
    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback)->GetResult());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Arvind.Venkateswaran                  08/2016
//---------------------------------------------------------------------------------------
TEST_F(EventTests, UnsubscribeTests)
    {
    Configuration::SetPredownloadChangeSetsEnabled(true);

    //Prepare imodel and acquire a briefcase
    auto briefcase = AcquireBriefcase();

    EventCallbackPtr callback = std::make_shared<EventCallback>([=](EventPtr event) {});

    //Get result
    auto unsubscribeResult = briefcase->UnsubscribeEventsCallback(callback)->GetResult();
    EXPECT_FALSE(unsubscribeResult.IsSuccess());
    EXPECT_EQ(Error::Id::EventCallbackNotFound, unsubscribeResult.GetError().GetId());

    Configuration::SetPredownloadChangeSetsEnabled(false);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Arvind.Venkateswaran                  08/2016
//---------------------------------------------------------------------------------------
TEST_F(EventTests, SingleCallbackTest)
    {
    //Prepare imodel and acquire a briefcase
    auto briefcase = AcquireBriefcase();

    bset<Event::EventType> eventTypes;
    eventTypes.insert(Event::EventType::CodeEvent);
    eventTypes.insert(Event::EventType::LockEvent);
    eventTypes.insert(Event::EventType::ChangeSetPrePushEvent);
    eventTypes.insert(Event::EventType::ChangeSetPostPushEvent);
    eventTypes.insert(Event::EventType::VersionEvent);

    //Subscribe, unsubscribe
    EventCallbackPtr callback1 = std::make_shared<EventCallback>([=](EventPtr event) {});
    auto result = briefcase->SubscribeEventsCallback(&eventTypes, callback1)->GetResult();
    EXPECT_SUCCESS(result);
    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback1)->GetResult());

    static int callbackNum = 0;
    static int codeEventCallbackNum = 0;
    static int lockEventCallbackNum = 0;
    static int changeSetPrePushEventCallbackNum = 0;
    static int changeSetPostPushEventCallbackNum = 0;
    static int versionEventCallbackNum = 0;

    EventCallbackPtr callback = std::make_shared<EventCallback>([=](EventPtr event) {
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
    CreateAndPushModel(briefcase);
    auto changeSets = m_imodelConnection->GetAllChangeSets()->GetResult().GetValue();
    EXPECT_FALSE(changeSets.empty());
    auto versionManager = m_imodelConnection->GetVersionsManager();
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
TEST_F(EventTests, MultipleCallbacksTest)
    {
    //Prepare imodel and acquire a briefcase
    auto briefcase = AcquireBriefcase();

    //Test LockEvent callbacks
    static int callbackNum1 = 0;
    EventCallbackPtr callback1 = std::make_shared<EventCallback>([=](EventPtr event)
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
    EventCallbackPtr callback2 = std::make_shared<EventCallback>([=](EventPtr event)
        {
        auto eventType = event->GetEventType();
        EXPECT_EQ(Event::EventType::ChangeSetPostPushEvent, eventType);
        callbackNum2++;
        });

    bset<Event::EventType> eventTypes2;
    eventTypes2.insert(Event::EventType::ChangeSetPostPushEvent);
    EXPECT_SUCCESS(briefcase->SubscribeEventsCallback(&eventTypes2, callback2)->GetResult());

    // Act and wait for events
    CreateAndPushModel(briefcase);
    BeThreadUtilities::BeSleep(3000);

    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback1)->GetResult());
    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback2)->GetResult());
    EXPECT_EQ(4, callbackNum1);
    EXPECT_EQ(1, callbackNum2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Algirdas.Mikoliunas                  12/2016
//---------------------------------------------------------------------------------------
TEST_F(EventTests, MultipleBriefcasesTest)
    {
    //Prepare imodel and acquire a briefcase
    auto briefcase = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();

    //Briefcase1 LockEvent callbacks
    static int callbackNum1 = 0;
    EventCallbackPtr callback1 = std::make_shared<EventCallback>([=](EventPtr event)
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
    EventCallbackPtr callback2 = std::make_shared<EventCallback>([=](EventPtr event)
        {
        auto eventType = event->GetEventType();
        EXPECT_EQ(Event::EventType::LockEvent, eventType);
        callbackNum2++;
        });

    bset<Event::EventType> eventTypes2;
    eventTypes2.insert(Event::EventType::LockEvent);
    EXPECT_SUCCESS(briefcase2->SubscribeEventsCallback(&eventTypes2, callback2)->GetResult());

    // Act and wait for events
    CreateAndPushModel(briefcase);
    BeThreadUtilities::BeSleep(3000);

    EXPECT_SUCCESS(briefcase->UnsubscribeEventsCallback(callback1)->GetResult());
    EXPECT_SUCCESS(briefcase2->UnsubscribeEventsCallback(callback2)->GetResult());
    EXPECT_EQ(4, callbackNum1);
    EXPECT_EQ(4, callbackNum2);
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                Algirdas.Mikoliunas                  12/2016
//---------------------------------------------------------------------------------------
TEST_F(EventTests, UnsubscribedNotCalledTest)
    {
    //Prepare imodel and acquire a briefcase
    auto briefcase = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();

    //Briefcase1 LockEvent callbacks
    static int callbackNum1 = 0;
    EventCallbackPtr callback1 = std::make_shared<EventCallback>([=](EventPtr event)
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
    EventCallbackPtr callback2 = std::make_shared<EventCallback>([=](EventPtr event)
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
    CreateAndPushModel(briefcase);
    BeThreadUtilities::BeSleep(3000);

    EXPECT_SUCCESS(briefcase2->UnsubscribeEventsCallback(callback2)->GetResult());
    EXPECT_EQ(0, callbackNum1);
    EXPECT_EQ(4, callbackNum2);
    }