/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/VersionsManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/VersionsManager.h>
#include "Utils.h"
#include "Logging.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
VersionsInfoTaskPtr VersionsManager::GetAllVersions(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::GetAllVersions";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::Version);

    return m_wsRepositoryClient->SendQueryRequest(query, nullptr, nullptr, cancellationToken)->Then<VersionsInfoResult>
        ([=] (const WSObjectsResult& versionInfoResult)
        {
        if (versionInfoResult.IsSuccess())
            {
            bvector<VersionInfoPtr> versions;
            if (!versionInfoResult.GetValue().GetRapidJsonDocument().IsNull())
                {
                for (auto const& value : versionInfoResult.GetValue().GetInstances())
                    {
                    auto versionInfo = VersionInfo::Parse(value);

                    versions.push_back(versionInfo);
                    }
                }

            return VersionsInfoResult::Success(versions);
            }
        else
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, versionInfoResult.GetError().GetMessage().c_str());
            return VersionsInfoResult::Error(versionInfoResult.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
VersionInfoTaskPtr VersionsManager::GetVersionById(Utf8StringCR versionId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::GetVersionById";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return m_wsRepositoryClient->SendGetObjectRequest(ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::Version, versionId), nullptr, cancellationToken)
        ->Then<VersionInfoResult>([=] (WSObjectsResult const& versionInfoResult)
        {
        if (versionInfoResult.IsSuccess())
            {
            auto version = VersionInfo::Parse(*versionInfoResult.GetValue().GetInstances().begin());
            return VersionInfoResult::Success(version);
            }
        else
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, versionInfoResult.GetError().GetMessage().c_str());
            return VersionInfoResult::Error(versionInfoResult.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
VersionInfoTaskPtr VersionsManager::CreateVersion(VersionInfoR version, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::CreateVersion";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    return m_wsRepositoryClient->SendCreateObjectRequest(version.GenerateJson(), BeFileName(), nullptr, cancellationToken)
        ->Then<VersionInfoResult>([=] (const WSCreateObjectResult& versionInfoResult)
        {
        if (versionInfoResult.IsSuccess())
            {
            Json::Value json;
            versionInfoResult.GetValue().GetJson(json);
            JsonValueCR instance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::Properties];
            auto versionInfo = VersionInfo::ParseRapidJson(ToRapidJson(instance));

            return VersionInfoResult::Success(versionInfo);
            }
        else
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, versionInfoResult.GetError().GetMessage().c_str());
            return VersionInfoResult::Error(versionInfoResult.GetError());
            }
        });

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr VersionsManager::UpdateVersion(VersionInfoCR version, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::ModifyVersion";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    Json::Value varsionJson = version.GenerateJson();

    return m_wsRepositoryClient->SendUpdateObjectRequest(ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::Version, version.GetId()), varsionJson[ServerSchema::Instance][ServerSchema::Properties], nullptr, BeFileName(), nullptr, cancellationToken)
        ->Then<StatusResult>([=] (const WSUpdateObjectResult& result)
        {
        if (result.IsSuccess())
            return StatusResult::Success();
        else
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return StatusResult::Error(result.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr VersionsManager::GetVersionChangeSets(Utf8String versionId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::GetVersionChangeSets";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    auto versionResult = GetVersionById(versionId, cancellationToken)->GetResult();
    if (!versionResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, versionResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(versionResult.GetError()));
        }

    auto result = m_connection->GetChangeSetById(versionResult.GetValue()->GetChangeSetId())->GetResult();
    if (!result.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(result.GetError()));
        }

    uint64_t changeSetIndex = result.GetValue()->GetIndex();

    Utf8String filter;
    filter.Sprintf("%s+le+%I64d", ServerSchema::Property::Index, changeSetIndex);

    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    query.SetFilter(filter);

    return m_connection->ChangeSetsFromQueryInternal(query, false, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr VersionsManager::GetChangeSetsBetweenVersions(Utf8String firstVersionId, Utf8String secondVersionId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "iModelConnection::GetChangeSetsBetweenVersions";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    auto versionResult = GetVersionById(firstVersionId, cancellationToken)->GetResult();
    if (!versionResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, versionResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(versionResult.GetError()));
        }
    std::deque<ObjectId> changeSetIds;
    changeSetIds.push_back(ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet, versionResult.GetValue()->GetChangeSetId()));
    versionResult = GetVersionById(secondVersionId, cancellationToken)->GetResult();
    if (!versionResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, versionResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(versionResult.GetError()));
        }
    changeSetIds.push_back(ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet, versionResult.GetValue()->GetChangeSetId()));
    auto query = m_connection->CreateChangeSetsByIdQuery(changeSetIds);

    auto changeSetsQueryResult = m_connection->GetChangeSetsInternal(query, true, cancellationToken)->GetResult();
    if (!changeSetsQueryResult.IsSuccess())
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(changeSetsQueryResult.GetError()));

    uint64_t changeSetIndex1 = 0;
    uint64_t changeSetIndex2 = 0;
    auto changeSets = changeSetsQueryResult.GetValue();
    changeSetIndex1 = changeSets.at(0)->GetIndex();
    changeSetIndex2 = changeSets.at(1)->GetIndex();

    Utf8String filter;
    filter.Sprintf("%s+gt+%I64d+and+(%s+le+%I64d)", ServerSchema::Property::Index,
                   changeSetIndex1 < changeSetIndex2 ? changeSetIndex1 : changeSetIndex2,
                   ServerSchema::Property::Index,
                   changeSetIndex1 > changeSetIndex2 ? changeSetIndex1 : changeSetIndex2);
    query = WSQuery(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    query.SetFilter(filter);

    return m_connection->ChangeSetsFromQueryInternal(query, false, cancellationToken);
    }
