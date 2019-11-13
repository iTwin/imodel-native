/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/GlobalEvents/GlobalConnection.h>
#include "../Utils.h"
#include <WebServices/iModelHub/Client/UserInfoManager.h>
#include <WebServices/iModelHub/EventsCommon/BaseEventManager.h>
#include <WebServices/iModelHub/Client/Result.h>
#include "../Logging.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Json::Value BaseEventManager::GenerateEventSASJson(const Utf8CP schemaName, const Utf8CP className)
    {
    Json::Value request = Json::objectValue;
    JsonValueR instance = request[ServerSchema::Instance] = Json::objectValue;
    instance[ServerSchema::InstanceId] = "";
    instance[ServerSchema::SchemaName] = schemaName;
    instance[ServerSchema::ClassName] = className;
    instance[ServerSchema::Properties] = Json::objectValue;
    JsonValueR properties = instance[ServerSchema::Properties];
    properties[ServerSchema::Property::BaseAddress] = "";
    properties[ServerSchema::Property::EventServiceSASToken] = "";
    return request;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
AzureServiceBusSASDTOPtr BaseEventManager::CreateEventSASFromResponse(JsonValueCR responseJson)
    {
    if (responseJson.isNull() || responseJson.empty())
        return nullptr;

    if (!responseJson.isMember(ServerSchema::ChangedInstance) ||
        !responseJson[ServerSchema::ChangedInstance].isMember(ServerSchema::InstanceAfterChange))
        return nullptr;

    Json::Value instance = responseJson[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];

    Utf8String sasToken = instance[ServerSchema::Properties][ServerSchema::Property::EventServiceSASToken].asString();
    Utf8String baseAddress = instance[ServerSchema::Properties][ServerSchema::Property::BaseAddress].asString();
    if (Utf8String::IsNullOrEmpty(sasToken.c_str()) || Utf8String::IsNullOrEmpty(baseAddress.c_str()))
        return nullptr;

    return AzureServiceBusSASDTO::Create(sasToken, baseAddress);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
AzureServiceBusSASDTOTaskPtr BaseEventManager::GetEventServiceSASToken(const ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "BaseEventManager::GetEventServiceSASToken";
    //POST to https://{server}/{version}/Repositories/DgnDbServer--{repoId}/DgnDbServer/EventSAS
    std::shared_ptr<AzureServiceBusSASDTOResult> finalResult = std::make_shared<AzureServiceBusSASDTOResult>();

    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, requestOptions, "Creating event service SAS token.");

    return m_wsRepositoryClient->SendCreateObjectRequestWithOptions(GenerateEventSASJson(), BeFileName(), nullptr, requestOptions, cancellationToken)
        ->Then([=] (const WSCreateObjectResult& result)
        {
        if (result.IsSuccess())
            {
            Json::Value json;
            result.GetValue().GetJson(json);
            AzureServiceBusSASDTOPtr ptr = CreateEventSASFromResponse(json);
            if (ptr == nullptr)
                {
                finalResult->SetError(Error::Id::NoSASFound);
                return;
                }

            finalResult->SetSuccess(ptr);
            }
        else
            {
            finalResult->SetError(result.GetError());
            }
        })->Then<AzureServiceBusSASDTOResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            06/2016
//---------------------------------------------------------------------------------------
Json::Value BaseEventManager::EventSubscriptionResponseToJsonInstance(const Utf8String response, Utf8StringR subscriptionId)
    {
    Json::Reader reader;
    Json::Value responseJson(Json::objectValue);
    if (!reader.parse(response, responseJson) && !responseJson.isArray())
        return nullptr;

    if (responseJson.isNull() || responseJson.empty())
        return nullptr;

    if (!responseJson.isMember(ServerSchema::ChangedInstances) ||
        responseJson[ServerSchema::ChangedInstances].empty() ||
        !responseJson[ServerSchema::ChangedInstances][0].isMember(ServerSchema::InstanceAfterChange))
        return nullptr;

    Json::Value instance = responseJson[ServerSchema::ChangedInstances][0][ServerSchema::InstanceAfterChange];

    if (!instance.isMember(ServerSchema::InstanceId))
        return nullptr;

    subscriptionId = instance[ServerSchema::InstanceId].asString();

    if (
        !instance[ServerSchema::Properties].isMember(ServerSchema::Property::EventTypes) ||
        !instance[ServerSchema::Properties][ServerSchema::Property::EventTypes].isArray()
        )
        return nullptr;

    return instance;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             05/2018
//---------------------------------------------------------------------------------------
AsyncTaskPtr<WSChangesetResult> BaseEventManager::SendChangesetRequest(const std::shared_ptr<WSChangeset> changeset, const ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "BaseEventManager::SendChangesetRequest";

    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, requestOptions, "Sending changeset request.");

    const HttpStringBodyPtr request = HttpStringBody::Create(changeset->ToRequestString());
    return m_wsRepositoryClient->SendChangesetRequestWithOptions(request, nullptr, requestOptions, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             05/2018
//---------------------------------------------------------------------------------------
StatusTaskPtr BaseEventManager::SendDeleteObjectRequest(const ObjectId eventSubscriptionObject) const
    {
    const Utf8String methodName = "BaseEventManager::SendDeleteObjectRequest";

    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, requestOptions, "Deleting subscription %s.", eventSubscriptionObject.ToString().c_str());

    return m_wsRepositoryClient->SendDeleteObjectRequestWithOptions(eventSubscriptionObject, requestOptions)->Then<StatusResult>([=] (WSVoidResult const& deleteResult)
        {
        if (!deleteResult.IsSuccess())
            {
            return StatusResult::Error(deleteResult.GetError());
            }
        return StatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Arvind.Venkateswaran            08/2016
//---------------------------------------------------------------------------------------
BaseEventReponseTaskPtr BaseEventManager::GetEventServiceResponse
(
    int numOfRetries,
    const bool longpolling,
    EventServiceClientPtr eventServiceClient,
    GetEventRequestType requestType,
    ICancellationTokenPtr cancellationToken
)
    {
    const Utf8String methodName = "GlobalEventClientManager::GetEventServiceResponse";

    if (!IsSubscribedToEvents(eventServiceClient))
        {
        LogHelper::Log(LOG_ERROR, methodName, "Not subscribed to event service.");
        return CreateCompletedAsyncTask<BaseEventReponseResult>
            (BaseEventReponseResult::Error(Error::Id::NotSubscribedToEventService));
        }

    BaseEventReponseResultPtr finalResult = std::make_shared<BaseEventReponseResult>();

    AsyncTaskPtr<EventServiceResult> eventRequestTask = nullptr;
    if(requestType == GetEventRequestType::Destructive)
        {
        eventRequestTask = eventServiceClient->MakeReceiveDeleteRequest(longpolling);
        }
    else if (requestType == GetEventRequestType::Peek)
        {
        eventRequestTask = eventServiceClient->MakeReceivePeekRequest(longpolling);
        }

    return eventRequestTask->Then([=] (const EventServiceResult& result)
        {
        if (result.IsSuccess())
            {
            Response response = result.GetValue();
            if (response.GetHttpStatus() != HttpStatus::OK && response.GetHttpStatus() != HttpStatus::Created)
                {
                LogHelper::Log(LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
                finalResult->SetError(result.GetError());
                return;
                }

            finalResult->SetSuccess(response);
            return;
            }
        else
            {
            HttpStatus status = result.GetError().GetHttpStatus();
            if (status == HttpStatus::NoContent || status == HttpStatus::None)
                {
                LogHelper::Log(LOG_WARNING, methodName, "No events found.");
                finalResult->SetError(Error::Id::NoEventsFound);
                return;
                }
            else if (status == HttpStatus::Unauthorized && numOfRetries > 0)
                {
                GetEventServiceSASToken(cancellationToken)->Then([=] (AzureServiceBusSASDTOResultCR setResult)
                    {
                    if (!setResult.IsSuccess())
                        {
                        finalResult->SetError(setResult.GetError());
                        return;
                        }

                    m_eventSAS = setResult.GetValue();
                    eventServiceClient->UpdateSASToken(m_eventSAS->GetSASToken());

                    int nextLoopValue = numOfRetries - 1;
                    GetEventServiceResponse(nextLoopValue, longpolling, eventServiceClient, requestType, cancellationToken)->Then([=] (BaseEventReponseResultCR currentResult)
                        {
                        if (!currentResult.IsSuccess())
                            {
                            finalResult->SetError(currentResult.GetError());
                            return;
                            }

                        finalResult->SetSuccess(currentResult.GetValue());
                        });
                    });
                }
            else if (status == HttpStatus::NotFound || status == HttpStatus::Gone)
                {
                LogHelper::Log(LOG_WARNING, methodName, "Event subscription does not exist: %s", result.GetError().GetMessage().c_str());
                finalResult->SetError(Error::Id::NoSubscriptionFound);
                return;
                }
            else
                {
                LogHelper::Log(LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
                finalResult->SetError(result.GetError());
                return;
                }
            }
        })->Then<BaseEventReponseResult>([=]
            {
            return *finalResult;
            });
    }

