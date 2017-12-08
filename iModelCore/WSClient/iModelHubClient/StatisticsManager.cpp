/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/StatisticsManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
bvector<StatisticsProperties> propertiesToSelect,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "StatisticsManager::QueryUserStatistics";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    ObjectId userInfoObject(ServerSchema::Schema::iModel, ServerSchema::Class::UserInfo, userId);
    WSQuery query(userInfoObject);

    Utf8String select = "*";
    AddHasStatisticsSelect(select, propertiesToSelect);
    query.SetSelect(select);

    return ExecuteWithRetry<StatisticsInfoPtr>([=] ()
        {
        //Execute query
        return m_repositoryClient->SendQueryRequest(query, "", "", cancellationToken)->Then<StatisticsInfoResult>
            ([=] (const WSObjectsResult& result)
            {
            if (!result.IsSuccess())
                return StatisticsInfoResult::Error(result.GetError());

            auto userInfoInstances = result.GetValue().GetInstances();
            if (userInfoInstances.IsEmpty())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "User does not exist.");
                return StatisticsInfoResult::Error(Error::Id::UserDoesNotExist);
                }
            if (userInfoInstances.Size() > 1)
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Multiple users found.");
                return StatisticsInfoResult::Error(Error::Id::Unknown);
                }
            auto statistics = StatisticsInfo::ParseFromRelated(*userInfoInstances.begin());
            if (statistics.IsNull())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Failed to parse statistics instance.");
                return StatisticsInfoResult::Error(Error::Id::Unknown);
                }

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "");
            return StatisticsInfoResult::Success(statistics);
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis              11/2017
//---------------------------------------------------------------------------------------
MultipleStatisticsInfoTaskPtr StatisticsManager::QueryAllUsersStatistics
(
bvector<StatisticsProperties> propertiesToSelect,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "StatisticsManager::QueryAllUsersStatistics";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::UserInfo);

    Utf8String select = "*";
    AddHasStatisticsSelect(select, propertiesToSelect);
    query.SetSelect(select);

    return ExecuteWithRetry<bvector<StatisticsInfoPtr> >([=] ()
        {
        //Execute query
        return m_repositoryClient->SendQueryRequest(query, "", "", cancellationToken)
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
                    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Failed to parse statistics instance.");
                    return MultipleStatisticsInfoResult::Error(Error::Id::Unknown);
                    }

                statisticsList.push_back(statistics);
                }
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "");
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
bvector<StatisticsProperties> propertiesToSelect,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "StatisticsManager::QueryUsersStatistics";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

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
            return m_repositoryClient->SendQueryRequest(query, "", "", cancellationToken)->Then<StatusResult>
                ([&] (const WSObjectsResult& result) mutable
                {
                if (!result.IsSuccess())
                    return StatusResult::Error(result.GetError());

                for (auto value : result.GetValue().GetInstances())
                    {
                    auto statistics = StatisticsInfo::ParseFromRelated(value);
                    if (statistics.IsNull())
                        {
                        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Failed to parse statistics instance.");
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
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "");

    return CreateCompletedAsyncTask<MultipleStatisticsInfoResult>(MultipleStatisticsInfoResult::Success(statisticsList));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis               11/2017
//---------------------------------------------------------------------------------------
void StatisticsManager::AddHasStatisticsSelect(Utf8StringR selectString, bvector<StatisticsProperties> propertiesToSelect)
    { 
    // in case you add couple properties your select string will look like this: *,@x.BriefcasesCount,@x.OwnedLocksCount&@x=HasStatistics-forward-Statistics
    if (propertiesToSelect.capacity() == 0)
        {
        selectString.Sprintf("%s,@x.%s", selectString.c_str(), "*");
        }
    else
        {
        Utf8String stringPropertyValue;
        for (auto propertyValue : propertiesToSelect)
            {
            switch (propertyValue)
                {
                case StatisticsProperties::BriefcaseCount:
                    stringPropertyValue = ServerSchema::Property::BriefcasesCount;
                    break;
                case StatisticsProperties::OwnedLocksCount:
                    stringPropertyValue = ServerSchema::Property::OwnedLocksCount;
                    break;
                case StatisticsProperties::PushedChangeSetsCount:
                    stringPropertyValue = ServerSchema::Property::PushedChangeSetsCount;
                    break;
                case StatisticsProperties::LastChangeSetPushDate:
                    stringPropertyValue = ServerSchema::Property::LastChangeSetPushDate;
                    break;
                default:
                    BeAssert(false && "Update StatisticsManager::AddHasStatisticsSelect with the newest version of StatisticsProperties.");
                    continue;
                }

            selectString.Sprintf("%s,@x.%s", selectString.c_str(), stringPropertyValue);
            }
        }
    selectString.Sprintf("%s&@x=%s-forward-%s", selectString.c_str(), ServerSchema::Relationship::HasStatistics, ServerSchema::Class::Statistics);
    }

