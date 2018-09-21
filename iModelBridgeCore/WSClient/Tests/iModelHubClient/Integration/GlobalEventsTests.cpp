/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/GlobalEventsTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    DgnDbPtr m_db;
    bset<Utf8String> m_unsubscribeSubscriptionInstances;
    static ClientPtr s_serviceAccountClient;

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
        m_db = CreateTestDb();

        bmap<Utf8String, Utf8String> requestOptions = bmap<Utf8String, Utf8String>();
        auto behaviourOptions = RequestBehaviorOptions();
        behaviourOptions.DisableOption(RequestBehaviorOptionsEnum::DisableGlobalEvents);
        requestOptions.insert(behaviourOptions.GetBehaviorOptionsResultPair());
        s_client->GlobalRequestOptions()->SetRequestOptions(requestOptions);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Uzkuraitis              05/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void TearDown() override
        {
        if (m_db.IsValid())
            m_db = nullptr;
        iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName());

        if(m_unsubscribeSubscriptionInstances.size() > 0)
            {
            const auto globalConnection = s_serviceAccountClient->GlobalConnection()->GetResult().GetValue();
            auto eventManager = globalConnection->GetGlobalEventManager();
            for (const Utf8String subscriptionInstanceId : m_unsubscribeSubscriptionInstances)
                {
                eventManager->UnsubscribeEvents(subscriptionInstanceId);
                }
            }
        IntegrationTestsBase::TearDown();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Uzkuraitis              05/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    iModelResult CreateiModel(const bool expectSuccess = true) const
        {
        return IntegrationTestsBase::CreateiModel(m_db, expectSuccess);
        }
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
    auto globalConnection = s_serviceAccountClient->GlobalConnection()->GetResult().GetValue();
    bset<GlobalEvent::GlobalEventType> eventTypes;
    eventTypes.insert(GlobalEvent::GlobalEventType::iModelCreatedEvent);
    auto eventManager = globalConnection->GetGlobalEventManager();

    const BeGuid newGuid(true);
    auto subscrResult = eventManager->SubscribeToEvents(newGuid, &eventTypes);
    EXPECT_SUCCESS(subscrResult->GetResult());
    m_unsubscribeSubscriptionInstances.insert(subscrResult->GetResult().GetValue()->GetSubscriptionInstanceId());

    eventTypes.insert(GlobalEvent::GlobalEventType::ChangeSetCreatedEvent);
    auto modifiedSubscriptionResult = eventManager->ModifySubscription(subscrResult->GetResult().GetValue()->GetSubscriptionInstanceId(), &eventTypes);
    EXPECT_SUCCESS(modifiedSubscriptionResult->GetResult());

    auto unsubscribeResult = eventManager->UnsubscribeEvents(subscrResult->GetResult().GetValue()->GetSubscriptionInstanceId());
    EXPECT_SUCCESS(unsubscribeResult->GetResult());
    m_unsubscribeSubscriptionInstances.erase(subscrResult->GetResult().GetValue()->GetSubscriptionInstanceId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Karolis.Uzkuraitis                  05/2018
//---------------------------------------------------------------------------------------
TEST_F(GlobalEventsTests, EventGetTests)
    {
    auto globalConnection = s_serviceAccountClient->GlobalConnection()->GetResult().GetValue();
    bset<GlobalEvent::GlobalEventType> eventTypes;
    eventTypes.insert(GlobalEvent::GlobalEventType::iModelCreatedEvent);
    auto eventManager = globalConnection->GetGlobalEventManager();

    const BeGuid newGuid(true);
    auto subscrResult = eventManager->SubscribeToEvents(newGuid, &eventTypes);
    EXPECT_SUCCESS(subscrResult->GetResult());
    m_unsubscribeSubscriptionInstances.insert(subscrResult->GetResult().GetValue()->GetSubscriptionInstanceId());

    std::list<ExpectedEventIdentifier> expectedEventsList;

    iModelResult createResult = CreateiModel();
    expectedEventsList.push_back(ExpectedEventIdentifier(GlobalEvent::GlobalEventType::iModelCreatedEvent, s_projectId, createResult.GetValue()->GetId()));
    iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName());

    createResult = CreateiModel();
    expectedEventsList.push_back(ExpectedEventIdentifier(GlobalEvent::GlobalEventType::iModelCreatedEvent, s_projectId, createResult.GetValue()->GetId()));

    CheckAllEventsReceived(subscrResult->GetResult().GetValue()->GetSubscriptionInstanceId(), eventManager, expectedEventsList);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Karolis.Uzkuraitis                  05/2018
//---------------------------------------------------------------------------------------
TEST_F(GlobalEventsTests, GetMultipleEventTypes)
    {
    auto globalConnection = s_serviceAccountClient->GlobalConnection()->GetResult().GetValue();
    bset<GlobalEvent::GlobalEventType> eventTypes;
    eventTypes.insert(GlobalEvent::GlobalEventType::iModelCreatedEvent);
    eventTypes.insert(GlobalEvent::GlobalEventType::ChangeSetCreatedEvent);
    eventTypes.insert(GlobalEvent::GlobalEventType::NamedVersionCreatedEvent);
    eventTypes.insert(GlobalEvent::GlobalEventType::SoftiModelDeleteEvent);
    eventTypes.insert(GlobalEvent::GlobalEventType::HardiModelDeleteEvent);
    auto eventManager = globalConnection->GetGlobalEventManager();

    const BeGuid newGuid(true);
    auto subscrResult = eventManager->SubscribeToEvents(newGuid, &eventTypes);
    EXPECT_SUCCESS(subscrResult->GetResult());
    m_unsubscribeSubscriptionInstances.insert(subscrResult->GetResult().GetValue()->GetSubscriptionInstanceId());

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
    ASSERT_TRUE(nullptr != projectClient);
    Utf8StringCR deleteArchivedJobId = LRPJobBackdoorAPI::ScheduleLRPJob(projectClient, "DeleteArchivedJob", createResult.GetValue()->GetId());

    bool deleteArchivedJobSuccessful = LRPJobBackdoorAPI::WaitForLRPJobToFinish(projectClient, deleteArchivedJobId);
    EXPECT_TRUE(deleteArchivedJobSuccessful);
    expectedEventsList.push_back(ExpectedEventIdentifier(GlobalEvent::GlobalEventType::HardiModelDeleteEvent, s_projectId, createResult.GetValue()->GetId()));

    CheckAllEventsReceived(subscrResult->GetResult().GetValue()->GetSubscriptionInstanceId(), eventManager, expectedEventsList);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Karolis.Uzkuraitis                  08/2018
//---------------------------------------------------------------------------------------
TEST_F(GlobalEventsTests, EventPeekAndDelete)
    {
    auto globalConnection = s_serviceAccountClient->GlobalConnection()->GetResult().GetValue();
    bset<GlobalEvent::GlobalEventType> eventTypes;
    eventTypes.insert(GlobalEvent::GlobalEventType::iModelCreatedEvent);
    auto eventManager = globalConnection->GetGlobalEventManager();

    const BeGuid newGuid(true);
    auto subscrResult = eventManager->SubscribeToEvents(newGuid, &eventTypes);
    auto subscriptionId = subscrResult->GetResult().GetValue()->GetSubscriptionInstanceId();
    EXPECT_SUCCESS(subscrResult->GetResult());
    m_unsubscribeSubscriptionInstances.insert(subscriptionId);

    iModelResult createResult = CreateiModel();
    ExpectedEventIdentifier eventIdentifier = ExpectedEventIdentifier(GlobalEvent::GlobalEventType::iModelCreatedEvent, s_projectId, createResult.GetValue()->GetId());
    iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName());

    // First peek should return the same event
    auto peekedEventResult = eventManager->PeekEvent(subscriptionId, false)->GetResult();
    auto peekedEvent = peekedEventResult.GetValue();
    EXPECT_EQ(eventIdentifier.eventType, peekedEvent->GetEventType());
    EXPECT_EQ(eventIdentifier.iModelId, peekedEvent->GetiModelId());
    EXPECT_EQ(eventIdentifier.projectId, peekedEvent->GetProjectId());

    // Delete event
    auto deletionResult = eventManager->DeleteEvent(peekedEvent)->GetResult();
    EXPECT_SUCCESS(deletionResult);

    // Second delete should fail
    deletionResult = eventManager->DeleteEvent(peekedEvent)->GetResult();
    EXPECT_FAILURE(deletionResult);

    // Next peek should return nothing
    peekedEventResult = eventManager->PeekEvent(subscriptionId, false)->GetResult();
    EXPECT_FAILURE(peekedEventResult);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Karolis.Uzkuraitis                  05/2018
//---------------------------------------------------------------------------------------
TEST_F(GlobalEventsTests, DeleteNotPeekedEvent)
    {
    auto globalConnection = s_serviceAccountClient->GlobalConnection()->GetResult().GetValue();
    bset<GlobalEvent::GlobalEventType> eventTypes;
    eventTypes.insert(GlobalEvent::GlobalEventType::iModelCreatedEvent);
    auto eventManager = globalConnection->GetGlobalEventManager();

    const BeGuid newGuid(true);
    auto subscrResult = eventManager->SubscribeToEvents(newGuid, &eventTypes);
    EXPECT_SUCCESS(subscrResult->GetResult());
    auto subscriptionId = subscrResult->GetResult().GetValue()->GetSubscriptionInstanceId();
    m_unsubscribeSubscriptionInstances.insert(subscriptionId);

    auto createResult = CreateiModel();
    ExpectedEventIdentifier eventIdentifier = ExpectedEventIdentifier(GlobalEvent::GlobalEventType::iModelCreatedEvent, s_projectId, createResult.GetValue()->GetId());
    iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName());

    //Get event
    auto eventResult = eventManager->GetEvent(subscriptionId, false)->GetResult();
    auto gotEvent = eventResult.GetValue();
    EXPECT_EQ(eventIdentifier.eventType, gotEvent->GetEventType());
    EXPECT_EQ(eventIdentifier.iModelId, gotEvent->GetiModelId());
    EXPECT_EQ(eventIdentifier.projectId, gotEvent->GetProjectId());

    // Deleting event that was not peeked should fail
    auto deletionResult = eventManager->DeleteEvent(gotEvent)->GetResult();
    EXPECT_FAILURE(deletionResult);
    }
