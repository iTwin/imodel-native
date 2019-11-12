/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/VersionsManager.h>
#include "Utils.h"
#include "Logging.h"

USING_NAMESPACE_BENTLEY_IMODELHUB


//---------------------------------------------------------------------------------------
//@bsimethod                                   Viktorija.Adomauskaite             02/2017
//---------------------------------------------------------------------------------------
WSQuery VersionsManager::CreateChangeSetsBetweenVersionsQuery(Utf8StringCR sourceVersionId, Utf8String destinationVersionId, 
                                                              BeSQLite::BeGuidCR fileId) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    Utf8String queryFilter;

    if (fileId.IsValid())
        queryFilter.Sprintf
        ("(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+and+%s+eq+'%s'",
         ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, sourceVersionId.c_str(),
         ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, destinationVersionId.c_str(),
         ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, destinationVersionId.c_str(),
         ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, sourceVersionId.c_str(),
         ServerSchema::Property::SeedFileId, fileId.ToString().c_str());
    else
        queryFilter.Sprintf
        ("(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')",
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
        queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'+and+%s+eq+'%s'", ServerSchema::Relationship::CumulativeChangeSet, 
                            ServerSchema::Class::Version, ServerSchema::Property::Id, versionId.c_str(),ServerSchema::Property::SeedFileId, 
                            fileId.ToString().c_str());
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
WSQuery VersionsManager::CreateChangeSetsBetweenVersionAndChangeSetQuery(Utf8StringCR versionId, Utf8StringCR changeSetId, 
                                                                         BeSQLite::BeGuidCR fileId) const
    {
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet);
    Utf8String queryFilter;

    if (Utf8String::IsNullOrEmpty(changeSetId.c_str()))
        {
        if (fileId.IsValid())
            queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'+and+%s+eq+'%s'",
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, 
                                versionId.c_str(),
                                ServerSchema::Property::SeedFileId, fileId.ToString().c_str());
        else
            queryFilter.Sprintf("%s-backward-%s.%s+eq+'%s'",
                                ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, 
                                versionId.c_str());
        }
    else
        {
        if (fileId.IsValid())
            queryFilter.Sprintf
            ("(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+and+%s+eq+'%s'",
             ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, versionId.c_str(),
             ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, changeSetId.c_str(),
             ServerSchema::Relationship::FollowingChangeSet, ServerSchema::Class::Version, ServerSchema::Property::Id, versionId.c_str(),
             ServerSchema::Relationship::CumulativeChangeSet, ServerSchema::Class::ChangeSet, ServerSchema::Property::Id, changeSetId.c_str(),
             ServerSchema::Property::SeedFileId, fileId.ToString().c_str());
        else
            queryFilter.Sprintf
            ("(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')+or+(%s-backward-%s.%s+eq+'%s'+and+%s-backward-%s.%s+eq+'%s')",
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
VersionsInfoTaskPtr VersionsManager::GetAllVersions(ICancellationTokenPtr cancellationToken, Thumbnail::Size thumbnailsToSelect) const
    {
    const Utf8String methodName = "VersionsManager::GetAllVersions";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::Version);

    Utf8String select = "*";
    if (thumbnailsToSelect & Thumbnail::Size::Small)
        Thumbnail::AddHasThumbnailSelect(select, Thumbnail::Size::Small);
    if (thumbnailsToSelect & Thumbnail::Size::Large)
        Thumbnail::AddHasThumbnailSelect(select, Thumbnail::Size::Large);
    query.SetSelect(select);

    return m_wsRepositoryClient->SendQueryRequestWithOptions(query, nullptr, nullptr, requestOptions, cancellationToken)->Then<VersionsInfoResult>
        ([=](const WSObjectsResult& versionInfoResult)
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
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, versionInfoResult.GetError().GetMessage().c_str());
            return VersionsInfoResult::Error(versionInfoResult.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
VersionInfoTaskPtr VersionsManager::GetVersionById(Utf8StringCR versionId, ICancellationTokenPtr cancellationToken, Thumbnail::Size thumbnailsToSelect) const
    {
    const Utf8String methodName = "VersionsManager::GetVersionById";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    WSQuery query(ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::Version, versionId));

    Utf8String select = "*";
    if (thumbnailsToSelect & Thumbnail::Size::Small)
        Thumbnail::AddHasThumbnailSelect(select, Thumbnail::Size::Small);
    if (thumbnailsToSelect & Thumbnail::Size::Large)
        Thumbnail::AddHasThumbnailSelect(select, Thumbnail::Size::Large);
    query.SetSelect(select);

    return m_wsRepositoryClient->SendQueryRequestWithOptions(query, nullptr, nullptr, requestOptions, cancellationToken)
        ->Then<VersionInfoResult>([=](WSObjectsResult const& versionInfoResult)
        {
        if (versionInfoResult.IsSuccess())
            {
            auto version = VersionInfo::Parse(*versionInfoResult.GetValue().GetInstances().begin());
            return VersionInfoResult::Success(version);
            }
        else
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, versionInfoResult.GetError().GetMessage().c_str());
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
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    auto jsonBody = version.GenerateJson();
    m_globalRequestOptionsPtr->InsertRequestOptions(jsonBody);

    return m_wsRepositoryClient->SendCreateObjectRequestWithOptions(jsonBody, BeFileName(), nullptr, requestOptions, cancellationToken)
        ->Then<VersionInfoResult>([=](const WSCreateObjectResult& versionInfoResult)
        {
        if (versionInfoResult.IsSuccess())
            {
            Json::Value json;
            versionInfoResult.GetValue().GetJson(json);
            JsonValueCR instance = json[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange][ServerSchema::Properties];
            auto versionInfo = VersionInfo::ParseRapidJson(ToRapidJson(instance), nullptr, nullptr);

            return VersionInfoResult::Success(versionInfo);
            }
        else
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, versionInfoResult.GetError().GetMessage().c_str());
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
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    Json::Value varsionJson = version.GenerateJson();

    return m_wsRepositoryClient->SendUpdateObjectRequestWithOptions(ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::Version, version.GetId()), 
                                                         varsionJson[ServerSchema::Instance][ServerSchema::Properties], nullptr, BeFileName(), 
                                                         nullptr, requestOptions, cancellationToken)
        ->Then<StatusResult>([=](const WSUpdateObjectResult& result)
        {
        if (result.IsSuccess())
            return StatusResult::Success();
        else
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, result.GetError().GetMessage().c_str());
            return StatusResult::Error(result.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr VersionsManager::GetVersionChangeSets(Utf8String versionId, BeSQLite::BeGuid fileId, 
                                                            ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "VersionsManager::GetVersionChangeSets";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (Utf8String::IsNullOrEmpty(versionId.c_str()))
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(Error::Id::InvalidVersion));

    return m_connection->GetChangeSetsFromQueryByChunks(CreateVersionChangeSetsQuery(versionId, fileId), false, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr VersionsManager::GetChangeSetsBetweenVersions(Utf8String firstVersionId, Utf8String secondVersionId, BeSQLite::BeGuid fileId, 
                                                                    ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "VersionsManager::GetChangeSetsBetweenVersions";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (Utf8String::IsNullOrEmpty(firstVersionId.c_str()) || Utf8String::IsNullOrEmpty(secondVersionId.c_str()))
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(Error::Id::InvalidVersion));

    return m_connection->GetChangeSetsFromQueryByChunks(CreateChangeSetsBetweenVersionsQuery(firstVersionId, secondVersionId, fileId), false,
                                                        cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr VersionsManager::GetChangeSetsAfterVersion(Utf8String versionId, BeSQLite::BeGuid fileId, ICancellationTokenPtr 
                                                                 cancellationToken) const
    {
    const Utf8String methodName = "VersionsManager::GetChangeSetsAfterVersion";

    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (Utf8String::IsNullOrEmpty(versionId.c_str()))
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(Error::Id::InvalidVersion));

    return m_connection->GetChangeSetsFromQueryByChunks(CreateChangeSetsAfterVersionQuery(versionId, fileId), false, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           02/2017
//---------------------------------------------------------------------------------------
ChangeSetsInfoTaskPtr VersionsManager::GetChangeSetsBetweenVersionAndChangeSet(Utf8String versionId, Utf8String changeSetId, BeSQLite::BeGuid fileId, 
                                                                               ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "VersionsManager::GetChangeSetsBetweenVersionAndChangeSet";

    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (Utf8String::IsNullOrEmpty(versionId.c_str()))
        return CreateCompletedAsyncTask(ChangeSetsInfoResult::Error(Error::Id::InvalidVersion));
    return m_connection->GetChangeSetsFromQueryByChunks(CreateChangeSetsBetweenVersionAndChangeSetQuery(versionId, changeSetId, fileId), false,
                                                        cancellationToken);
    }
