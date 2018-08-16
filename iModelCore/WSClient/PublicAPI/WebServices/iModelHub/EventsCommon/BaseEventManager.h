/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/EventsCommon/BaseEventManager.h $
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
#include <WebServices/Azure/AzureServiceBusSASDTO.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

DEFINE_TASK_TYPEDEFS(AzureServiceBusSASDTOPtr, AzureServiceBusSASDTO)
DEFINE_TASK_TYPEDEFS(Http::Response, BaseEventReponse)

//=======================================================================================
//@bsiclass                                      Karolis.Uzkuraitis             04/2018
//=======================================================================================
struct BaseEventManager : RefCountedBase
    {
private:
    IWSRepositoryClientPtr      m_wsRepositoryClient;

protected:
    virtual ~BaseEventManager() {}

    AzureServiceBusSASDTOPtr    m_eventSAS;

    BaseEventManager(const IWSRepositoryClientPtr wsRepositoryClientPtr) : m_wsRepositoryClient(wsRepositoryClientPtr) {}

    BaseEventManager() {}

    virtual Json::Value GenerateEventSASJson() const = 0;
    virtual bool IsSubscribedToEvents(EventServiceClientPtr eventServiceClient) const = 0;

    static Json::Value GenerateEventSASJson(const Utf8CP schemaName, const Utf8CP className);
    static AzureServiceBusSASDTOPtr CreateEventSASFromResponse(JsonValueCR responseJson);
    static Json::Value EventSubscriptionResponseToJsonInstance(const Utf8String response, Utf8StringR subscriptionId);
    
    AzureServiceBusSASDTOTaskPtr GetEventServiceSASToken(const ICancellationTokenPtr cancellationToken) const;
    AsyncTaskPtr<WSChangesetResult> SendChangesetRequest(const std::shared_ptr<WSChangeset> changeset, const ICancellationTokenPtr cancellationToken) const;
    StatusTaskPtr SendDeleteObjectRequest(ObjectId eventSubscriptionObject) const;
    enum class GetEventRequestType
        {
        Destructive,
        Peek
        }; 
    BaseEventReponseTaskPtr GetEventServiceResponse
    (
        int numOfRetries,
        const bool longpolling,
        EventServiceClientPtr eventServiceClient,
        GetEventRequestType requestType,
        ICancellationTokenPtr cancellationToken
    );
    
    };

END_BENTLEY_IMODELHUB_NAMESPACE
