/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/StatisticsManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/StatisticsManager.h>
#include "Logging.h"
#include "Utils.h"
#include <functional>
#include <Bentley/Tasks/WorkerThread.h>
#include <Bentley/Tasks/AsyncTask.h>

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis              11/2017
//---------------------------------------------------------------------------------------
StatisticsInfoTaskPtr StatisticsManager::QueryUserStatistics
(
Utf8StringCR userId,
StatisticsProperties propertiesToSelect,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "StatisticsManager::QueryUserStatistics";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    ObjectId userInfoObject(ServerSchema::Schema::iModel, ServerSchema::Class::UserInfo, userId);
    WSQuery query(userInfoObject);

    Utf8String select = "*";
    AddHasStatisticsSelect(select, propertiesToSelect);
    query.SetSelect(select);

    return ExecuteWithRetry<StatisticsInfoPtr>([=] ()
        {
        //Execute query
        return m_repositoryClient->SendQueryRequestWithOptions(query, "", "", requestOptions, cancellationToken)->Then<StatisticsInfoResult>
            ([=] (const WSObjectsResult& result)
            {
            if (!result.IsSuccess())
                return StatisticsInfoResult::Error(result.GetError());

            auto userInfoInstances = result.GetValue().GetInstances();
            if (userInfoInstances.IsEmpty())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, "User does not exist.");
                return StatisticsInfoResult::Error(Error::Id::UserDoesNotExist);
                }
            if (userInfoInstances.Size() > 1)
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, "Multiple users found.");
                return StatisticsInfoResult::Error(Error::Id::Unknown);
                }
            auto statistics = StatisticsInfo::ParseFromRelated(*userInfoInstances.begin());
            if (statistics.IsNull())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, "Failed to parse statistics instance.");
                return StatisticsInfoResult::Error(Error::Id::Unknown);
                }

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), requestOptions, "");
            return StatisticsInfoResult::Success(statistics);
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis              11/2017
//---------------------------------------------------------------------------------------
MultipleStatisticsInfoTaskPtr StatisticsManager::QueryAllUsersStatistics
(
StatisticsProperties propertiesToSelect,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "StatisticsManager::QueryAllUsersStatistics";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::UserInfo);

    Utf8String select = "*";
    AddHasStatisticsSelect(select, propertiesToSelect);
    query.SetSelect(select);

    return ExecuteWithRetry<bvector<StatisticsInfoPtr> >([=] ()
        {
        //Execute query
        return m_repositoryClient->SendQueryRequestWithOptions(query, "", "", requestOptions, cancellationToken)
            ->Then<MultipleStatisticsInfoResult>
            ([=] (const WSObjectsResult& result)
            {
            if (!result.IsSuccess())
                return MultipleStatisticsInfoResult::Error(result.GetError());

            bvector<StatisticsInfoPtr> statisticsList;
            for (auto value : result.GetValue().GetInstances())
                {
                auto statistics = StatisticsInfo::ParseFromRelated(value);
                if (statistics.IsNull())
                    {
                    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, "Failed to parse statistics instance.");
                    return MultipleStatisticsInfoResult::Error(Error::Id::Unknown);
                    }

                statisticsList.push_back(statistics);
                }
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), requestOptions, "");
            return MultipleStatisticsInfoResult::Success(statisticsList);
            });
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Benas.Kikutis               11/2017
//---------------------------------------------------------------------------------------
MultipleStatisticsInfoTaskPtr StatisticsManager::QueryUsersStatistics
(
bvector<Utf8String> usersIds,
StatisticsProperties propertiesToSelect,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "StatisticsManager::QueryUsersStatistics";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    std::deque<ObjectId> usersIdsQueue;
    for (auto userId : usersIds)
        usersIdsQueue.push_back(ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::UserInfo, userId));
    bvector<StatisticsInfoPtr> statisticsList;

    while (!usersIdsQueue.empty())
        {
        WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::UserInfo);

        Utf8String select = "*";
        AddHasStatisticsSelect(select, propertiesToSelect);
        query.SetSelect(select);
        query.AddFilterIdsIn(usersIdsQueue);

        auto queryStatisticsResult = ExecuteWithRetry<void>([&] ()
            {
            //Execute query
            return m_repositoryClient->SendQueryRequestWithOptions(query, "", "", requestOptions, cancellationToken)->Then<StatusResult>
                ([&] (const WSObjectsResult& result) mutable
                {
                if (!result.IsSuccess())
                    return StatusResult::Error(result.GetError());

                for (auto value : result.GetValue().GetInstances())
                    {
                    auto statistics = StatisticsInfo::ParseFromRelated(value);
                    if (statistics.IsNull())
                        {
                        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, "Failed to parse statistics instance.");
                        return StatusResult::Error(Error::Id::Unknown);
                        }

                    statisticsList.push_back(statistics);
                    }
                return StatusResult::Success();
                });
            });

        if (!queryStatisticsResult->GetResult().IsSuccess())
            return CreateCompletedAsyncTask<MultipleStatisticsInfoResult>(MultipleStatisticsInfoResult::Error(queryStatisticsResult->GetResult().GetError()));
        }

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), requestOptions, "");

    return CreateCompletedAsyncTask<MultipleStatisticsInfoResult>(MultipleStatisticsInfoResult::Success(statisticsList));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis               11/2017
//---------------------------------------------------------------------------------------
void StatisticsManager::AddHasStatisticsSelect(Utf8StringR selectString, StatisticsProperties propertiesToSelect)
    { 
    // in case you add couple properties your select string will look like this: *,@x.BriefcasesCount,@x.OwnedLocksCount&@x=HasStatistics-forward-Statistics
    if (StatisticsProperties::All == propertiesToSelect)
        {
        selectString.Sprintf("%s,@x.%s", selectString.c_str(), "*");
        }
    else
        {
        if (0 != (StatisticsProperties::BriefcaseCount & propertiesToSelect))
            selectString.Sprintf("%s,@x.%s", selectString.c_str(), ServerSchema::Property::BriefcasesCount);
        if (0 != (StatisticsProperties::OwnedLocksCount & propertiesToSelect))
            selectString.Sprintf("%s,@x.%s", selectString.c_str(), ServerSchema::Property::OwnedLocksCount);
        if (0 != (StatisticsProperties::PushedChangeSetsCount & propertiesToSelect))
            selectString.Sprintf("%s,@x.%s", selectString.c_str(), ServerSchema::Property::PushedChangeSetsCount);
        if (0 != (StatisticsProperties::LastChangeSetPushDate & propertiesToSelect))
            selectString.Sprintf("%s,@x.%s", selectString.c_str(), ServerSchema::Property::LastChangeSetPushDate);
        }
    selectString.Sprintf("%s&@x=%s-forward-%s", selectString.c_str(), ServerSchema::Relationship::HasStatistics, ServerSchema::Class::Statistics);
    }

