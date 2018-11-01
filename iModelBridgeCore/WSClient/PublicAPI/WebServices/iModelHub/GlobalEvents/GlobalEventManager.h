/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/GlobalEvents/GlobalEventManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/Client/WSChangeset.h>
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/iModelInfo.h>
#include <WebServices/Azure/AzureBlobStorageClient.h>
#include <WebServices/Azure/EventServiceClient.h>
#include <WebServices/Azure/AzureServiceBusSASDTO.h>
#include <WebServices/iModelHub/GlobalEvents/GlobalEvent.h>
#include <WebServices/iModelHub/GlobalEvents/GlobalEventSubscription.h>
#include <BeHttp/AuthenticationHandler.h>
#include <WebServices/iModelHub/EventsCommon/BaseEventManager.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

DEFINE_TASK_TYPEDEFS(AzureServiceBusSASDTOPtr, AzureServiceBusSASDTO);
DEFINE_TASK_TYPEDEFS(Http::Response, GlobalEventReponse);

DEFINE_POINTER_SUFFIX_TYPEDEFS(GlobalEventManager);

typedef RefCountedPtr<struct GlobalEventManager> GlobalEventManagerPtr;

//=======================================================================================
//! Manager for Global Events
//@bsiclass                                      Karolis.Uzkuraitis             04/2018
//=======================================================================================
struct GlobalEventManager : BaseEventManager
    {
private:
    friend struct GlobalConnection;
    friend struct GenericGlobalEvent;

    bmap<Utf8String, EventServiceClientPtr> m_eventServiceClients;

    GlobalEventManager(const IWSRepositoryClientPtr wsRepositoryClientPtr) : BaseEventManager(wsRepositoryClientPtr) {}

    GlobalEventManager() : BaseEventManager() {}

    GlobalEventManager(const GlobalEventManager&) = delete;

    GlobalEventSubscriptionTaskPtr SendEventChangesetRequest(std::shared_ptr<WSChangeset> changeset, ICancellationTokenPtr cancellationToken = nullptr) const;

    void UpdateSASTokenForSubscription(Utf8StringCR instanceId, const ICancellationTokenPtr cancellationToken = nullptr);

    static GlobalEventSubscriptionPtr CreateEventSubscriptionFromResponse(Utf8StringCR response);

    static Json::Value GenerateGlobalEventSubscriptionProperties(Utf8StringCR subscriptionId, GlobalEventTypeSet* eventTypes);

    static void EventSubscriptionToChangeSet
    (
        Utf8StringCR                    subscriptionId,
        GlobalEventTypeSet*             eventTypes,
        Utf8StringCR                    eventSubscriptionInstanceId,
        WSChangeset&                    changeset,
        const WSChangeset::ChangeState& changeState
    );

    //! Get EventSubscription with the given Event Types
    GlobalEventSubscriptionTaskPtr CreateEventSubscription(BeSQLite::BeGuidCR subscriptionId, GlobalEventTypeSet* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Update the EventSubscription to the given EventTypes
    GlobalEventSubscriptionTaskPtr UpdateEventServiceSubscriptionId(Utf8String intanceId, GlobalEventTypeSet* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    GlobalEventTaskPtr GetEventInternal(const GlobalEventSubscriptionId instanceId, bool longPooling, GetEventRequestType requestType, const ICancellationTokenPtr cancellationToken = nullptr);

    IMODELHUBCLIENT_EXPORT Json::Value GenerateEventSASJson() const override;
    
    IMODELHUBCLIENT_EXPORT bool IsSubscribedToEvents(EventServiceClientPtr eventServiceClient) const override;

public:
    virtual ~GlobalEventManager() {}

    //! Create or update subscription to specified global event types provided subscriptionId.
    //! @param[in] subscriptionId Id of subscription that user provides.
    //! @param[in] eventTypes List of events to subscribe to.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the GlobalEventSubscription result with InstanceId.
    //! @note Use this to get instanceId from subscriptionId for already existing subscriptions.
    IMODELHUBCLIENT_EXPORT GlobalEventSubscriptionTaskPtr SubscribeToEvents(BeSQLite::BeGuidCR subscriptionId, GlobalEventTypeSet* eventTypes, const ICancellationTokenPtr cancellationToken = nullptr);

    //! Update subscription to specified global event types provided instanceId of subscription.
    //! @param[in] instanceId IntanceId of subscription  See SubscribeToEvents().
    //! @param[in] eventTypes List of events to subscribe to.
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the new GlobalEventSubscription result.
    //! @note Additional convenience method to use with InstanceId. SubscribeToEvents() can be used to modify events too.
    IMODELHUBCLIENT_EXPORT GlobalEventSubscriptionTaskPtr ModifySubscription(const GlobalEventSubscriptionId instanceId, GlobalEventTypeSet* eventTypes, ICancellationTokenPtr cancellationToken = nullptr);

    //! Get Event from subscription provided instanceId. Event is immediately destroyed in the server.
    //! @param[in] instanceId IntanceId of subscription See SubscribeToEvents().
    //! @param[in] longPooling option to wait longer for events
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the Global event result.
    IMODELHUBCLIENT_EXPORT GlobalEventTaskPtr GetEvent(const GlobalEventSubscriptionId instanceId, bool longPooling = false, ICancellationTokenPtr cancellationToken = nullptr);

    //! Delete subscription.
    //! @param[in] instanceId IntanceId of subscription.
    //! @return Asynchronous task that has the status of subscription deletion as result.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr UnsubscribeEvents(const GlobalEventSubscriptionId instanceId) const;

    //! Peek Event from subscription provided instanceId. Event gets locked in the server until it is deleted using DeleteEvent() or lock expires.
    //! @param[in] instanceId IntanceId of subscription See SubscribeToEvents().
    //! @param[in] longPooling option to wait longer for events
    //! @param[in] cancellationToken
    //! @return Asynchronous task that has the Global event result.
    IMODELHUBCLIENT_EXPORT GlobalEventTaskPtr PeekEvent(const GlobalEventSubscriptionId instanceId, bool longPooling = false, const ICancellationTokenPtr cancellationToken = nullptr);

    //! Delete Event that was received using PeekEvent() and successfully handled.
    //! @param[in] globalEvent Global event to delete.
    //! @return Asynchronous task that has the status of event deletion as result.
    IMODELHUBCLIENT_EXPORT StatusTaskPtr DeleteEvent(GlobalEventPtr globalEvent);
    };

END_BENTLEY_IMODELHUB_NAMESPACE
