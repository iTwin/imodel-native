/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "WebServices/iModelHub/GlobalEvents/GlobalEventManager.h"
#include <WebServices/Client/WSChangeset.h>
#include "../Utils.h"
#include "../Logging.h"
#include <WebServices/iModelHub/Client/UserInfoManager.h>
#include "WebServices/iModelHub/GlobalEvents/GlobalEventParser.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
void GlobalEventManager::UpdateSASTokenForSubscription(Utf8StringCR instanceId, const ICancellationTokenPtr cancellationToken)
    {
    if(nullptr == m_eventSAS)
        {
        auto result = GetEventServiceSASToken(cancellationToken)->GetResult();
        if (result.IsSuccess())
            {
            m_eventSAS = result.GetValue();
            }
        }

    if (m_eventServiceClients.find(instanceId) == m_eventServiceClients.end())
        m_eventServiceClients[instanceId] = EventServiceClient::Create(m_eventSAS->GetBaseAddress(), instanceId);

    m_eventServiceClients[instanceId]->UpdateSASToken(m_eventSAS->GetSASToken());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalEventSubscriptionTaskPtr GlobalEventManager::ModifySubscription(const GlobalEventSubscriptionId instanceId, GlobalEventTypeSet* eventTypes, const ICancellationTokenPtr cancellationToken)
    {
    GlobalEventSubscriptionTaskPtr eventSubscription = UpdateEventServiceSubscriptionId(instanceId, eventTypes, cancellationToken);

    eventSubscription->Then<GlobalEventSubscriptionResult>([=] (GlobalEventSubscriptionResultCR result)
        {
        if (!result.IsSuccess())
            return result;

        UpdateSASTokenForSubscription(instanceId);

        return result;
        });

    return eventSubscription;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalEventTaskPtr GlobalEventManager::GetEvent(const GlobalEventSubscriptionId instanceId, bool longPooling, const ICancellationTokenPtr cancellationToken)
    {
    const Utf8String methodName = "GlobalEventManager::GetEvent";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    const double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    return AsyncTaskAddMethodLogging<GlobalEventResult>(GetEventInternal(instanceId, longPooling, GetEventRequestType::Destructive, cancellationToken), methodName, start);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalEventTaskPtr GlobalEventManager::PeekEvent(const GlobalEventSubscriptionId instanceId, bool longPooling, const ICancellationTokenPtr cancellationToken)
    {
    const Utf8String methodName = "GlobalEventManager::PeekEvent";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    const double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    return AsyncTaskAddMethodLogging<GlobalEventResult>(GetEventInternal(instanceId, longPooling, GetEventRequestType::Peek, cancellationToken), methodName, start);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            08/2018
//---------------------------------------------------------------------------------------
GlobalEventTaskPtr GlobalEventManager::GetEventInternal(const GlobalEventSubscriptionId instanceId, bool longPooling, GetEventRequestType requestType, const ICancellationTokenPtr cancellationToken)
    {
    const Utf8String methodName = "GlobalEventManager::GetEventInternal";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    const double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    UpdateSASTokenForSubscription(instanceId);

    return GetEventServiceResponse(3, longPooling, m_eventServiceClients[instanceId], requestType, cancellationToken)->Then<GlobalEventResult>
        ([=] (EventReponseResult& result)
        {
        if (result.IsSuccess())
            {
            Http::Response response = result.GetValue();
            GlobalEventPtr ptr = GlobalEventParser::ParseEvent(response.GetHeaders().GetContentType(), response.GetBody().AsString(), response.GetHeaders().GetLocation());
            if (ptr == nullptr)
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "No events found.");
                return GlobalEventResult::Error(Error::Id::NoEventsFound);
                }
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, static_cast<float>(end - start), "");
            return GlobalEventResult::Success(ptr);
            }
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
        return GlobalEventResult::Error(result.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            08/2018
//---------------------------------------------------------------------------------------
StatusTaskPtr GlobalEventManager::DeleteEvent(GlobalEventPtr globalEvent)
    {
    const Utf8String methodName = "GlobalEventManager::DeleteEvent";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    const double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    if(globalEvent.IsNull() || globalEvent->m_lockUrl.size() == 0)
        {
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::EventInvalid));
        }

    auto subscriptionId = globalEvent->GetFromEventSubscriptionId();
    UpdateSASTokenForSubscription(subscriptionId);

    return AsyncTaskAddMethodLogging<StatusResult>(m_eventServiceClients[subscriptionId]->MakeDeleteEventRequest(globalEvent->m_lockUrl)->Then<StatusResult>
        ([=] (EventServiceResult const& result)
        {
        if (!result.IsSuccess())
            {
            return StatusResult::Error(result.GetError());
            }
        return StatusResult::Success();
        }), methodName, start);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
Json::Value GlobalEventManager::GenerateGlobalEventSubscriptionProperties(Utf8StringCR subscriptionId, GlobalEventTypeSet* eventTypes)
    {
    Json::Value properties;
    properties[ServerSchema::Property::SubscriptionId] = subscriptionId;
    properties[ServerSchema::Property::EventTypes] = Json::arrayValue;
    if (eventTypes != nullptr)
        {
        int i = 0;
        for (auto eventType : *eventTypes)
            {
            properties[ServerSchema::Property::EventTypes][i] = GlobalEvent::Helper::GetEventNameFromEventType(eventType);
            i++;
            }
        }
    return properties;
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
void GlobalEventManager::EventSubscriptionToChangeSet
(
    Utf8StringCR                    subscriptionId,
    GlobalEventTypeSet*             eventTypes,
    Utf8StringCR                    eventSubscriptionInstanceId,
    WSChangeset&                    changeset,
    const WSChangeset::ChangeState& changeState
)
    {
    const ObjectId eventSubscriptionObject
    (
        ServerSchema::Schema::Global,
        ServerSchema::Class::GlobalEventSubscription,
        eventSubscriptionInstanceId
    );

    changeset.AddInstance
    (
        eventSubscriptionObject,
        changeState,
        std::make_shared<Json::Value>(GenerateGlobalEventSubscriptionProperties(subscriptionId, eventTypes))
    );
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalEventSubscriptionTaskPtr GlobalEventManager::CreateEventSubscription
(
    BeGuidCR subscriptionId,
    GlobalEventTypeSet* eventTypes,
    const ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "GlobalEventManager::CreateEventSubscription";
    const double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    const std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    EventSubscriptionToChangeSet(subscriptionId.ToString(), eventTypes, "", *changeset, WSChangeset::Created);

    std::shared_ptr<GlobalEventSubscriptionResult> finalResult = std::make_shared<GlobalEventSubscriptionResult>();
    return AsyncTaskAddMethodLogging<GlobalEventSubscriptionResult>(SendEventChangesetRequest(changeset, cancellationToken), methodName, start);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalEventSubscriptionTaskPtr GlobalEventManager::UpdateEventServiceSubscriptionId
(
    const Utf8String intanceId,
    GlobalEventTypeSet* eventTypes,
    const ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "GlobalEventManager::UpdateEventServiceSubscriptionId";
    LogHelper::Log(LOG_DEBUG, methodName, "Method called.");
    const double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    const std::shared_ptr<WSChangeset> changeset(new WSChangeset());
    EventSubscriptionToChangeSet(nullptr, eventTypes, intanceId, *changeset, WSChangeset::Modified);

    std::shared_ptr<GlobalEventSubscriptionResult> finalResult = std::make_shared<GlobalEventSubscriptionResult>();
    return AsyncTaskAddMethodLogging<GlobalEventSubscriptionResult>(SendEventChangesetRequest(changeset, cancellationToken), methodName, start);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalEventSubscriptionPtr GlobalEventManager::CreateEventSubscriptionFromResponse(Utf8StringCR response)
    {
    Utf8String subscriptionInstanceId;
    auto instance = EventSubscriptionResponseToJsonInstance(response, subscriptionInstanceId);
    if (instance.isNull())
        return nullptr;

    GlobalEventTypeSet eventTypes;
    for (Json::ValueIterator itr = instance[ServerSchema::Properties][ServerSchema::Property::EventTypes].begin();
         itr != instance[ServerSchema::Properties][ServerSchema::Property::EventTypes].end(); ++itr)
        eventTypes.insert(GlobalEvent::Helper::GetEventTypeFromEventName((*itr).asString().c_str()));

    return GlobalEventSubscription::Create(subscriptionInstanceId, eventTypes);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalEventSubscriptionTaskPtr GlobalEventManager::SendEventChangesetRequest
(
    const std::shared_ptr<WSChangeset> changeset,
    const ICancellationTokenPtr cancellationToken
) const
    {
    std::shared_ptr<GlobalEventSubscriptionResult> finalResult = std::make_shared<GlobalEventSubscriptionResult>();
    return SendChangesetRequest(changeset, cancellationToken)->Then([=] (const WSChangesetResult& result)
        {
        if (result.IsSuccess())
            {
            GlobalEventSubscriptionPtr ptr = CreateEventSubscriptionFromResponse(result.GetValue()->AsString().c_str());
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
        })->Then<GlobalEventSubscriptionResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             05/2018
//---------------------------------------------------------------------------------------
Json::Value GlobalEventManager::GenerateEventSASJson() const
    {
    return BaseEventManager::GenerateEventSASJson(ServerSchema::Schema::Global, ServerSchema::Class::GlobalEventSAS);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             05/2018
//---------------------------------------------------------------------------------------
bool GlobalEventManager::IsSubscribedToEvents(const EventServiceClientPtr eventServiceClient) const
    {
    return m_eventSAS != nullptr && eventServiceClient != nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             04/2018
//---------------------------------------------------------------------------------------
GlobalEventSubscriptionTaskPtr GlobalEventManager::SubscribeToEvents(BeGuidCR subscriptionId, GlobalEventTypeSet* eventTypes, const ICancellationTokenPtr cancellationToken)
    {
    const Utf8String methodName = "GlobalConnection::SubscribeToEvents";
    LogHelper::Log(LOG_DEBUG, methodName, "Method called.");
    const double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    GlobalEventSubscriptionTaskPtr eventSubscription = CreateEventSubscription(subscriptionId, eventTypes, cancellationToken);

    return AsyncTaskAddMethodLogging<GlobalEventSubscriptionResult>(eventSubscription->Then<GlobalEventSubscriptionResult>([=] (GlobalEventSubscriptionResultCR result)
        {
        if (!result.IsSuccess())
            return result;

        UpdateSASTokenForSubscription(result.GetValue()->GetSubscriptionInstanceId());
        return result;
        }), methodName, start);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             04/2018
//---------------------------------------------------------------------------------------
StatusTaskPtr GlobalEventManager::UnsubscribeEvents(const GlobalEventSubscriptionId instanceId) const
    {
    const Utf8String methodName = "GlobalConnection::UnsubscribeEvents";
    LogHelper::Log(LOG_DEBUG, methodName, "Method called.");
    const double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    const ObjectId eventSubscriptionObject
    (
        ServerSchema::Schema::Global,
        ServerSchema::Class::GlobalEventSubscription,
        instanceId
    );

    return AsyncTaskAddMethodLogging<StatusResult>(SendDeleteObjectRequest(eventSubscriptionObject), methodName, start);
    }
