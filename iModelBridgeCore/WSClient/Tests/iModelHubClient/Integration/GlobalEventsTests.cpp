/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/GlobalEventsTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"
#include <WebServices/iModelHub/Client/ClientHelper.h>
#include <WebServices/iModelHub/GlobalEvents/iModelCreatedEvent.h>
#include <WebServices/iModelHub/GlobalEvents/GlobalEventManager.h>
#include "LRPJobBackdoorAPI.h"
#include "RequestBehaviorOptions.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Uzkuraitis              05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExpectedEventIdentifier
    {
    GlobalEvent::GlobalEventType eventType = GlobalEvent::GlobalEventType::UnknownEventType;
    Utf8String projectId = nullptr;
    Utf8String iModelId = nullptr;
    ExpectedEventIdentifier(const GlobalEvent::GlobalEventType eventType, const Utf8String projectId, const Utf8String iModelId)
        : eventType(eventType),
        projectId(projectId),
        iModelId(iModelId)
        {
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Uzkuraitis              05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct GlobalEventsTests : IntegrationTestsBase
    {
    static ClientPtr s_serviceAccountClient;
    GlobalConnectionPtr m_globalConnection;
    GlobalEventManagerPtr m_eventManager;
    GlobalEventSubscriptionPtr m_subscription;
    GlobalEventSubscriptionId m_subscriptionId;

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Uzkuraitis              05/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();
        iModelHubHelpers::CreateClient(s_serviceAccountClient, IntegrationTestsSettings::Instance().GetValidServiceAccountCredentials());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Uzkuraitis              05/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Uzkuraitis              05/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetUp() override
        {
        IntegrationTestsBase::SetUp();

        bmap<Utf8String, Utf8String> requestOptions = bmap<Utf8String, Utf8String>();
        auto behaviourOptions = RequestBehaviorOptions();
        behaviourOptions.DisableOption(RequestBehaviorOptionsEnum::DisableGlobalEvents);
        requestOptions.insert(behaviourOptions.GetBehaviorOptionsResultPair());
        s_client->GlobalRequestOptions()->SetRequestOptions(requestOptions);
        m_globalConnection = s_serviceAccountClient->GlobalConnection()->GetResult().GetValue();
        m_eventManager = m_globalConnection->GetGlobalEventManager();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Uzkuraitis              05/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void TearDown() override
        {
        iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName());

        if(!Utf8String::IsNullOrEmpty(m_subscriptionId.c_str()))
            {
            m_eventManager->UnsubscribeEvents(m_subscriptionId);
            }
        IntegrationTestsBase::TearDown();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Uzkuraitis              05/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    iModelResult CreateiModel(const bool expectSuccess = true)
        {
        return IntegrationTestsBase::CreateEmptyiModel(GetTestiModelName(), expectSuccess);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              05/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void Subscribe(GlobalEventTypeSet* eventTypes)
        {
        const BeGuid newGuid(true);
        GlobalEventSubscriptionResult subscriptionResult = m_eventManager->SubscribeToEvents(newGuid, eventTypes)->GetResult();
        ASSERT_SUCCESS(subscriptionResult);
        m_subscription = subscriptionResult.GetValue();
        m_subscriptionId = m_subscription->GetSubscriptionInstanceId();
        };
    };

ClientPtr GlobalEventsTests::s_serviceAccountClient;

//---------------------------------------------------------------------------------------
//@bsimethod                                Karolis.Uzkuraitis                  05/2018
//---------------------------------------------------------------------------------------
void CheckAllEventsReceived(const Utf8String subscriptionInstanceId, GlobalEventManagerPtr eventManager, std::list<ExpectedEventIdentifier> &expectedEvents)
    {
    const clock_t timeStart = clock();
    clock_t timeLastEvent = timeStart;
    while (expectedEvents.size() > 0)
        {
        const auto currentDuration = (clock() - timeStart) / static_cast<double>(CLOCKS_PER_SEC);
        const auto lastEventDuration = (clock() - timeLastEvent) / static_cast<double>(CLOCKS_PER_SEC);
        if(currentDuration >= 180 || lastEventDuration >= 60)
            {
            // If it takes more than 3 minutes, abort.
            // If more than 1 minute has passed since last event, abort.
            EXPECT_TRUE(expectedEvents.empty());
            break;
            }

        GlobalEventTaskPtr eventResult = eventManager->GetEvent(subscriptionInstanceId);
        if(eventResult->GetResult().IsSuccess())
            {
            timeLastEvent = clock();
            auto gotEvent = eventResult->GetResult().GetValue();

            for (auto expectedEventIter = expectedEvents.begin(); expectedEventIter != expectedEvents.end(); ++expectedEventIter)
                {
                EXPECT_NE(GlobalEvent::UnknownEventType, expectedEventIter->eventType);
                if (expectedEventIter->eventType == gotEvent->GetEventType() &&
                    expectedEventIter->iModelId.EqualsI(gotEvent->GetiModelId()) &&
                    expectedEventIter->projectId.EqualsI(gotEvent->GetProjectId()))
                    {
                    expectedEvents.erase(expectedEventIter);
                    break;
                    }
                }
            }
        else
            {
            // Event receiving failed, wait 5 seconds and try again.
            BeDuration(BeDuration::Seconds(5)).Sleep();
            }
        }
    EXPECT_TRUE(expectedEvents.empty());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Karolis.Uzkuraitis                  05/2018
//---------------------------------------------------------------------------------------
TEST_F(GlobalEventsTests, SubscribeTests)
    {
    bset<GlobalEvent::GlobalEventType> eventTypes;
    eventTypes.insert(GlobalEvent::GlobalEventType::iModelCreatedEvent);

    Subscribe(&eventTypes);

    eventTypes.insert(GlobalEvent::GlobalEventType::ChangeSetCreatedEvent);
    GlobalEventSubscriptionResult modifiedSubscriptionResult = m_eventManager->ModifySubscription(m_subscriptionId, &eventTypes)->GetResult();
    EXPECT_SUCCESS(modifiedSubscriptionResult);

    StatusResult unsubscribeResult = m_eventManager->UnsubscribeEvents(m_subscriptionId)->GetResult();
    EXPECT_SUCCESS(unsubscribeResult);
    m_subscriptionId = "";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Karolis.Uzkuraitis                  05/2018
//---------------------------------------------------------------------------------------
TEST_F(GlobalEventsTests, EventGetTests)
    {
    bset<GlobalEvent::GlobalEventType> eventTypes;
    eventTypes.insert(GlobalEvent::GlobalEventType::iModelCreatedEvent);

    Subscribe(&eventTypes);

    std::list<ExpectedEventIdentifier> expectedEventsList;

    iModelResult createResult = CreateiModel();
    expectedEventsList.push_back(ExpectedEventIdentifier(GlobalEvent::GlobalEventType::iModelCreatedEvent, s_projectId, createResult.GetValue()->GetId()));
    iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName());

    createResult = CreateiModel();
    expectedEventsList.push_back(ExpectedEventIdentifier(GlobalEvent::GlobalEventType::iModelCreatedEvent, s_projectId, createResult.GetValue()->GetId()));

    CheckAllEventsReceived(m_subscriptionId, m_eventManager, expectedEventsList);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Karolis.Uzkuraitis                  05/2018
//---------------------------------------------------------------------------------------
TEST_F(GlobalEventsTests, GetMultipleEventTypes)
    {
    bset<GlobalEvent::GlobalEventType> eventTypes;
    eventTypes.insert(GlobalEvent::GlobalEventType::iModelCreatedEvent);
    eventTypes.insert(GlobalEvent::GlobalEventType::ChangeSetCreatedEvent);
    eventTypes.insert(GlobalEvent::GlobalEventType::NamedVersionCreatedEvent);
    eventTypes.insert(GlobalEvent::GlobalEventType::SoftiModelDeleteEvent);
    eventTypes.insert(GlobalEvent::GlobalEventType::HardiModelDeleteEvent);

    Subscribe(&eventTypes);

    std::list<ExpectedEventIdentifier> expectedEventsList;

    auto createResult = CreateiModel();
    expectedEventsList.push_back(ExpectedEventIdentifier(GlobalEvent::GlobalEventType::iModelCreatedEvent, s_projectId, createResult.GetValue()->GetId()));

    BriefcaseResult acquireResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, createResult.GetValue(), false);
    const auto iModelConnection = CreateiModelConnection(createResult.GetValue());
    BriefcasePtr briefcase = acquireResult.GetValue();
    auto model = CreateModel(GetTestInfo().name(), briefcase->GetDgnDb());
    CreateElement(*model, DgnCode(), false);

    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(briefcase, true, false));
    expectedEventsList.push_back(ExpectedEventIdentifier(GlobalEvent::GlobalEventType::ChangeSetCreatedEvent, s_projectId, createResult.GetValue()->GetId()));

    VersionInfoPtr version;
    iModelHubHelpers::CreateNamedVersion(version, iModelConnection, TestCodeName(), 1);
    expectedEventsList.push_back(ExpectedEventIdentifier(GlobalEvent::GlobalEventType::NamedVersionCreatedEvent, s_projectId, createResult.GetValue()->GetId()));

    iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName());
    expectedEventsList.push_back(ExpectedEventIdentifier(GlobalEvent::GlobalEventType::SoftiModelDeleteEvent, s_projectId, createResult.GetValue()->GetId()));

    IWSRepositoryClientPtr projectClient = nullptr;
    iModelHubHelpers::CreateProjectWSClient(projectClient, *s_client, s_projectId);
    ASSERT_NE(nullptr, projectClient);
    Utf8StringCR deleteArchivedJobId = LRPJobBackdoorAPI::ScheduleLRPJob(projectClient, "DeleteArchivedJob", createResult.GetValue()->GetId());

    bool deleteArchivedJobSuccessful = LRPJobBackdoorAPI::WaitForLRPJobToFinish(projectClient, deleteArchivedJobId);
    EXPECT_TRUE(deleteArchivedJobSuccessful);
    expectedEventsList.push_back(ExpectedEventIdentifier(GlobalEvent::GlobalEventType::HardiModelDeleteEvent, s_projectId, createResult.GetValue()->GetId()));

    CheckAllEventsReceived(m_subscriptionId, m_eventManager, expectedEventsList);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Karolis.Uzkuraitis                  08/2018
//---------------------------------------------------------------------------------------
TEST_F(GlobalEventsTests, EventPeekAndDelete)
    {
    bset<GlobalEvent::GlobalEventType> eventTypes;
    eventTypes.insert(GlobalEvent::GlobalEventType::iModelCreatedEvent);

    Subscribe(&eventTypes);

    iModelResult createResult = CreateiModel();
    iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName());

    // First peek should have an event to return
    auto peekedEventResult = m_eventManager->PeekEvent(m_subscriptionId, false)->GetResult();
    ASSERT_SUCCESS(peekedEventResult);
    auto peekedEvent = peekedEventResult.GetValue();
    EXPECT_EQ(GlobalEvent::GlobalEventType::iModelCreatedEvent, peekedEvent->GetEventType());

    // Delete event
    auto deletionResult = m_eventManager->DeleteEvent(peekedEvent)->GetResult();
    EXPECT_SUCCESS(deletionResult);

    // Second delete should fail
    deletionResult = m_eventManager->DeleteEvent(peekedEvent)->GetResult();
    EXPECT_FAILURE(deletionResult);

    // If second peek is successful, it should return a different event
    auto secondPeekedEventResult = m_eventManager->PeekEvent(m_subscriptionId, false)->GetResult();
    if (secondPeekedEventResult.IsSuccess())
        {
        auto secondPeekedEvent = secondPeekedEventResult.GetValue();
        EXPECT_EQ(GlobalEvent::GlobalEventType::iModelCreatedEvent, secondPeekedEvent->GetEventType());
        EXPECT_NE(peekedEvent->GetiModelId(), secondPeekedEvent->GetiModelId());
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Karolis.Uzkuraitis                  05/2018
//---------------------------------------------------------------------------------------
TEST_F(GlobalEventsTests, DeleteNotPeekedEvent)
    {
    bset<GlobalEvent::GlobalEventType> eventTypes;
    eventTypes.insert(GlobalEvent::GlobalEventType::iModelCreatedEvent);

    Subscribe(&eventTypes);

    auto createResult = CreateiModel();
    iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName());

    //Get event
    auto eventResult = m_eventManager->GetEvent(m_subscriptionId, false)->GetResult();
    ASSERT_SUCCESS(eventResult);
    auto gotEvent = eventResult.GetValue();
    EXPECT_EQ(GlobalEvent::GlobalEventType::iModelCreatedEvent, gotEvent->GetEventType());

    // Deleting event that was not peeked should fail
    auto deletionResult = m_eventManager->DeleteEvent(gotEvent)->GetResult();
    EXPECT_FAILURE(deletionResult);
    }
