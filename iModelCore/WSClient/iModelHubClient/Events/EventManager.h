/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Events/EventManager.h $
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
#include <BeHttp/AuthenticationHandler.h>
#include <WebServices/iModelHub/Client/UserInfoManager.h>
#include <WebServices/iModelHub/EventsCommon/BaseEventManager.h>
#include "WebServices/iModelHub/Events/EventSubscription.h"
#include "EventManager.h"

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

DEFINE_TASK_TYPEDEFS(AzureServiceBusSASDTOPtr, AzureServiceBusSASDTO);
DEFINE_TASK_TYPEDEFS(Http::Response, GlobalEventReponse);

//=======================================================================================
//! Manager for Events
//@bsiclass                                      Karolis.Uzkuraitis             04/2018
//=======================================================================================
struct EventManager : BaseEventManager
    {
private:
    friend struct iModelConnection;

    EventServiceClientPtr      m_eventServiceClient;
    EventSubscriptionPtr       m_eventSubscription;
    BeMutex                    m_eventServiceClientMutex;

    EventManager(const IWSRepositoryClientPtr& wsRepositoryClientPtr) : BaseEventManager(wsRepositoryClientPtr)
        {
        }

    //! Sets the EventSubscription in the EventServiceClient
    StatusTaskPtr SetEventSubscription(EventTypeSet* eventTypes, ICancellationTokenPtr cancellationToken = nullptr);

    //! Sets EventServiceClient.
    StatusTaskPtr SetEventServiceClient(EventTypeSet* eventTypes = nullptr, ICancellationTokenPtr cancellationToken = nullptr);

    static EventSubscriptionPtr CreateEventSubscriptionFromResponse(const Utf8String response);

    // This pointer needs to change to be generic
    EventSubscriptionTaskPtr SendEventChangesetRequest(std::shared_ptr<WSChangeset> changeset,
                                                        ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Get EventSubscription with the given Event Types
    EventSubscriptionTaskPtr GetEventServiceSubscriptionId(EventTypeSet* eventTypes = nullptr,
                                                            ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Update the EventSubscription to the given EventTypes
    EventSubscriptionTaskPtr UpdateEventServiceSubscriptionId(EventTypeSet* eventTypes = nullptr,
                                                                ICancellationTokenPtr cancellationToken = nullptr) const;

    Json::Value GenerateEventSASJson() const override;

    StatusTaskPtr UnsubscribeEvents();

public: 
    virtual ~EventManager() {}

    bool IsSubscribedToEvents(EventServiceClientPtr eventServiceClient) const override;

    EventTaskPtr GetEvent(bool longPolling = false, ICancellationTokenPtr cancellationToken = nullptr);
    };

END_BENTLEY_IMODELHUB_NAMESPACE
