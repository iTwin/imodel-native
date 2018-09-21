/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Events/EventManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "EventManager.h"
#include <WebServices/Client/WSChangeset.h>
#include "../Utils.h"
#include "../Logging.h"
#include <WebServices/iModelHub/Client/UserInfoManager.h>

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
EventSubscriptionPtr EventManager::CreateEventSubscriptionFromResponse(const Utf8String response)
    {
    Utf8String subscriptionId;
    auto instance = EventSubscriptionResponseToJsonInstance(response, subscriptionId);
    if (instance.isNull())
        return nullptr;

    EventTypeSet eventTypes;
    for (Json::ValueIterator itr = instance[ServerSchema::Properties][ServerSchema::Property::EventTypes].begin();
         itr != instance[ServerSchema::Properties][ServerSchema::Property::EventTypes].end(); ++itr)
        eventTypes.insert(Event::Helper::GetEventTypeFromEventName((*itr).asString().c_str()));

    return EventSubscription::Create(subscriptionId, eventTypes);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
//Returns true if same, else false
bool CompareEventTypes
(
    EventTypeSet* newEventTypes,
    EventTypeSet oldEventTypes
)
    {
    if (
        (newEventTypes == nullptr || newEventTypes->size() == 0) &&
        oldEventTypes.size() == 0
        )
        return true;

    if (
        oldEventTypes.size() > 0 &&
        (newEventTypes == nullptr || newEventTypes->size() != oldEventTypes.size())
        )
        return false;

    for (auto newEventType : *newEventTypes)
        {
        if (oldEventTypes.end() == oldEventTypes.find(newEventType))
            {
            return false;
            }
        }

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr EventManager::SetEventSubscription(EventTypeSet* eventTypes, ICancellationTokenPtr cancellationToken)
    {
    EventSubscriptionTaskPtr eventSubscription;
    StatusResultPtr finalResult = std::make_shared<StatusResult>();

    if (m_eventSubscription.IsNull())
        eventSubscription = GetEventServiceSubscriptionId(eventTypes, cancellationToken);
    else if (!CompareEventTypes(eventTypes, m_eventSubscription->GetEventTypes()))
        eventSubscription = UpdateEventServiceSubscriptionId(eventTypes, cancellationToken);
    else
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());

    return eventSubscription->Then([=] (EventSubscriptionResultCR subscriptionResult)
        {
        if (!subscriptionResult.IsSuccess())
            {
            finalResult->SetError(subscriptionResult.GetError());
            return;
            }

        m_eventSubscription = subscriptionResult.GetValue();
        BaseEventManager::GetEventServiceSASToken(cancellationToken)->Then([=] (AzureServiceBusSASDTOResultCR setResult)
            {
            if (!setResult.IsSuccess())
                {
                finalResult->SetError(setResult.GetError());
                return;
                }
            m_eventSAS = setResult.GetValue();

            finalResult->SetSuccess();
            });
        })->Then<StatusResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            05/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr EventManager::SetEventServiceClient
(
    EventTypeSet* eventTypes,
    ICancellationTokenPtr cancellationToken
)
    {
    return SetEventSubscription(eventTypes, cancellationToken)->Then<StatusResult>([=] (StatusResultCR subscribeResult)
        {
        if (!subscribeResult.IsSuccess())
            return StatusResult::Error(subscribeResult.GetError());

        BeMutexHolder lock(m_eventServiceClientMutex);

        if (m_eventServiceClient == nullptr)
            m_eventServiceClient = EventServiceClient::Create(m_eventSAS->GetBaseAddress(), m_eventSubscription->GetSubscriptionId());

        m_eventServiceClient->UpdateSASToken(m_eventSAS->GetSASToken());
        return StatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Arvind.Venkateswaran           07/2016
//---------------------------------------------------------------------------------------
EventSubscriptionTaskPtr EventManager::SendEventChangesetRequest
(
    const std::shared_ptr<WSChangeset> changeset,
    const ICancellationTokenPtr cancellationToken
) const
    {
    std::shared_ptr<EventSubscriptionResult> finalResult = std::make_shared<EventSubscriptionResult>();
    return SendChangesetRequest(changeset, cancellationToken)->Then([=] (const WSChangesetResult& result)
        {
        if (result.IsSuccess())
            {
            EventSubscriptionPtr ptr = CreateEventSubscriptionFromResponse(result.GetValue()->AsString().c_str());
            if (ptr == nullptr)
                {
                finalResult->SetError(Error::Id::NoSubscriptionFound);
                return;
                }

            finalResult->SetSuccess(ptr);
            }
        else
            {
            finalResult->SetError(result.GetError());
            }
        })->Then<EventSubscriptionResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Json::Value GenerateEventSubscriptionWSChangeSetJson(EventTypeSet* eventTypes)
    {
    Json::Value properties;
    properties[ServerSchema::Property::EventTypes] = Json::arrayValue;
    if (eventTypes != nullptr)
        {
        int i = 0;
        for (auto eventType : *eventTypes)
            {
            properties[ServerSchema::Property::EventTypes][i] = Event::Helper::GetEventNameFromEventType(eventType);
            i++;
            }
        }
    return properties;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Caleb.Shafer                   06/2016
//---------------------------------------------------------------------------------------
void SetEventSubscriptionJsonRequestToChangeSet
(
    EventTypeSet* eventTypes,
    const Utf8String                                 eventSubscriptionId,
    WSChangeset&                                     changeset,
    const WSChangeset::ChangeState&                  changeState
)
    {
    const ObjectId eventSubscriptionObject
        (
        ServerSchema::Schema::iModel,
        ServerSchema::Class::EventSubscription,
        eventSubscriptionId
        );

    changeset.AddInstance
        (
        eventSubscriptionObject,
        changeState,
        std::make_shared<Json::Value>(GenerateEventSubscriptionWSChangeSetJson(eventTypes))
        );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
EventSubscriptionTaskPtr EventManager::GetEventServiceSubscriptionId
(
    EventTypeSet* eventTypes,
    ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "EventManager::GetEventServiceSubscriptionId";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    const double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    const std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    SetEventSubscriptionJsonRequestToChangeSet(eventTypes, "", *changeset, WSChangeset::Created);

    std::shared_ptr<EventSubscriptionResult> finalResult = std::make_shared<EventSubscriptionResult>();
    return AsyncTaskAddMethodLogging<EventSubscriptionResult>(SendEventChangesetRequest(changeset, cancellationToken), methodName, start);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    06/2016
//---------------------------------------------------------------------------------------
EventSubscriptionTaskPtr EventManager::UpdateEventServiceSubscriptionId
(
    EventTypeSet* eventTypes,
    ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "EventManager::UpdateEventServiceSubscriptionId";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    const double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    const std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    SetEventSubscriptionJsonRequestToChangeSet(eventTypes, m_eventSubscription->GetSubscriptionId(), *changeset, WSChangeset::Modified);

    std::shared_ptr<EventSubscriptionResult> finalResult = std::make_shared<EventSubscriptionResult>();
    return AsyncTaskAddMethodLogging<EventSubscriptionResult>(SendEventChangesetRequest(changeset, cancellationToken), methodName, start);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
bool EventManager::IsSubscribedToEvents(EventServiceClientPtr) const
    {
    return m_eventServiceClient != nullptr && m_eventSubscription != nullptr && m_eventSAS != nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
Json::Value EventManager::GenerateEventSASJson() const
    {
    return BaseEventManager::GenerateEventSASJson(ServerSchema::Schema::iModel, ServerSchema::Class::EventSAS);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Arvind.Venkateswaran            07/2016
//---------------------------------------------------------------------------------------
EventTaskPtr EventManager::GetEvent
(
    bool longPolling,
    ICancellationTokenPtr cancellationToken
)
    {
    BeMutexHolder lock(m_eventServiceClientMutex);

    const Utf8String methodName = "EventManager::GetEvent";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (!IsSubscribedToEvents(m_eventServiceClient))
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Not subscribed to event service.");
        return CreateCompletedAsyncTask<EventResult>
            (EventResult::Error(Error::Id::NotSubscribedToEventService));
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    return AsyncTaskAddMethodLogging<EventResult>(GetEventServiceResponse(3, longPolling, m_eventServiceClient, GetEventRequestType::Destructive, cancellationToken)->Then<EventResult>
        ([=] (EventReponseResult& result)
        {
        if (result.IsSuccess())
            {
            Http::Response response = result.GetValue();
            EventPtr ptr = EventParser::ParseEvent(response.GetHeaders().GetContentType(), response.GetBody().AsString());
            if (ptr == nullptr)
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "No events found.");
                return EventResult::Error(Error::Id::NoEventsFound);
                }
            return EventResult::Success(ptr);
            }
        return EventResult::Error(result.GetError());
        }), methodName, start);
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                     Arvind.Venkateswaran            06/2016
//Todo: Add another method to only cancel GetEvent Operation and not the entire connection
//---------------------------------------------------------------------------------------
StatusTaskPtr  EventManager::UnsubscribeEvents()
    {
    BeMutexHolder lock(m_eventServiceClientMutex);

    const Utf8String methodName = "EventManager::UnsubscribeEvents";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (m_eventServiceClient != nullptr)
        m_eventServiceClient->CancelRequest();
    m_eventServiceClient = nullptr;
    m_eventSubscription = nullptr;
    m_eventSAS = nullptr;
    return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
    }
