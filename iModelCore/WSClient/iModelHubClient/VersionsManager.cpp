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
//@bsimethod                                   Viktorija.Adomauskaite             02/2017
//---------------------------------------------------------------------------------------
WSQuery VersionsManager::CreateChangeSetsBetweenVersionsQuery(Utf8StringCR sourceVersionId, Utf8String destinationVersionId, BeSQLite::BeGuidCR fileId) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    Utf8String queryFilter;

    if (fileId.IsValid())
        queryFilter.Sprintf("(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+and+%s+eq+'%s'",
                            ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, sourceVersionId.c_str(),
                            ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, destinationVersionId.c_str(),
                            ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, destinationVersionId.c_str(),
                            ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, sourceVersionId.c_str(),
                            ServerSchema::Property::SeedFileId, fileId.ToString().c_str());
    else
        queryFilter.Sprintf("(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')",
                            ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, sourceVersionId.c_str(),
                            ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, destinationVersionId.c_str(),
                            ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, destinationVersionId.c_str(),
                            ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, sourceVersionId.c_str());

    query.SetFilter(queryFilter);

    return query;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Viktorija.Adomauskaite             02/2017
//---------------------------------------------------------------------------------------
WSQuery VersionsManager::CreateVersionChangeSetsQuery(Utf8StringCR versionId, BeSQLite::BeGuidCR fileId) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    Utf8String queryFilter;

    if (fileId.IsValid())
        queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'+and+%s+eq+'%s'", ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version,
                            ServerSchema::Property::Id, versionId.c_str(),
                            ServerSchema::Property::SeedFileId, fileId.ToString().c_str());
    else
        queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'", ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version,
                            ServerSchema::Property::Id, versionId.c_str());

    query.SetFilter(queryFilter);

    return query;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Viktorija.Adomauskaite             02/2017
//---------------------------------------------------------------------------------------
WSQuery VersionsManager::CreateChangeSetsAfterVersionQuery(Utf8StringCR versionId, BeSQLite::BeGuidCR fileId) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    Utf8String queryFilter;

    if (fileId.IsValid())
        queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'+and+%s+eq+'%s'", ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version,
                            ServerSchema::Property::Id, versionId.c_str(),
                            ServerSchema::Property::SeedFileId, fileId.ToString().c_str());
    else
        queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'", ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version,
                            ServerSchema::Property::Id, versionId.c_str());

    query.SetFilter(queryFilter);

    return query;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Viktorija.Adomauskaite             02/2017
//---------------------------------------------------------------------------------------
WSQuery VersionsManager::CreateChangeSetsBetweenVersionAndChangeSetQuery(Utf8StringCR versionId, Utf8StringCR changeSetId, BeSQLite::BeGuidCR fileId) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    Utf8String queryFilter;

    if (Utf8String::IsNullOrEmpty(changeSetId.c_str()))
        {
        if(fileId.IsValid())
            queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'+and+%s+eq+'%s'",
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, versionId.c_str(),
                                ServerSchema::Property::SeedFileId, fileId.ToString().c_str());
        else
            queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'",
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, versionId.c_str());
        }
    else
        {
        if (fileId.IsValid())
            queryFilter.Sprintf("(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+and+%s+eq+'%s'",
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, versionId.c_str(),
                                ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, changeSetId.c_str(),
                                ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, versionId.c_str(),
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, changeSetId.c_str(),
                                ServerSchema::Property::SeedFileId, fileId.ToString().c_str());
        else
            queryFilter.Sprintf("(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')",
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, versionId.c_str(),
                                ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, changeSetId.c_str(),
                                ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, versionId.c_str(),
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, changeSetId.c_str());
        }

    query.SetFilter(queryFilter);

    return query;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
VersionsInfoTaskPtr VersionsManager::GetAllVersions(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "VersionsManager::GetAllVersions";
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
    const Utf8String methodName = "VersionsManager::GetVersionById";
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
    const Utf8String methodName = "VersionsManager::CreateVersion";
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
    const Utf8String methodName = "VersionsManager::UpdateVersion";
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
ChangeSetsInfoTaskPtr VersionsManager::GetVersionChangeSets(Utf8String versionId, BeSQLite::BeGuid fileId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "VersionsManager::GetVersionChangeSets";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (Utf8String::IsNullOrEmpty(versionId.c_str()))
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(Error::Id::InvalidVersion));

    return m_connection->ChangeSetsFromQueryInternal(CreateVersionChangeSetsQuery(versionId, fileId), false, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr VersionsManager::GetChangeSetsBetweenVersions(Utf8String firstVersionId, Utf8String secondVersionId, BeSQLite::BeGuid fileId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "VersionsManager::GetChangeSetsBetweenVersions";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (Utf8String::IsNullOrEmpty(firstVersionId.c_str()) || Utf8String::IsNullOrEmpty(secondVersionId.c_str()))
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(Error::Id::InvalidVersion));

    return m_connection->ChangeSetsFromQueryInternal(CreateChangeSetsBetweenVersionsQuery(firstVersionId, secondVersionId, fileId), false, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr VersionsManager::GetChangeSetsAfterVersion(Utf8String versionId, BeSQLite::BeGuid fileId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "VersionsManager::GetChangeSetsAfterVersion";

    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (Utf8String::IsNullOrEmpty(versionId.c_str()))
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(Error::Id::InvalidVersion));

    return m_connection->ChangeSetsFromQueryInternal(CreateChangeSetsAfterVersionQuery(versionId, fileId), false, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr VersionsManager::GetChangeSetsBetweenVersionAndChangeSet(Utf8String versionId, Utf8String changeSetId, BeSQLite::BeGuid fileId, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "VersionsManager::GetChangeSetsBetweenVersionAndChangeSet";

    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (Utf8String::IsNullOrEmpty(versionId.c_str()))
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(Error::Id::InvalidVersion));

    return m_connection->ChangeSetsFromQueryInternal(CreateChangeSetsBetweenVersionAndChangeSetQuery(versionId, changeSetId, fileId), false, cancellationToken);
    }
