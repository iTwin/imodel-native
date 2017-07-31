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
