/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/UserInfoManager.h>
#include "Logging.h"
#include "Utils.h"
#include <functional>
#include <Bentley/Tasks/WorkerThread.h>
#include <Bentley/Tasks/AsyncTask.h>

USING_NAMESPACE_BENTLEY_IMODELHUB

static bmap<Utf8String, UserInfoPtr> s_userInfoCache = bmap<Utf8String, UserInfoPtr>();
static BeMutex s_userInfoCacheMutex;

//---------------------------------------------------------------------------------------
//@bsimethod                                     Paulius.Valiunas              07/2017
//---------------------------------------------------------------------------------------
UserInfoTaskPtr UserInfoManager::QueryUserInfoById
(
Utf8StringCR userId,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "UserInfoManager::QueryUserInfoById";
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    InitializeUserCache(cancellationToken);

    auto it = s_userInfoCache.find(userId);
    if (it != s_userInfoCache.end())
        {
        return CreateCompletedAsyncTask<UserInfoResult>(UserInfoResult::Success(it->second));
        }

    ObjectId userInfoObject(ServerSchema::Schema::iModel, ServerSchema::Class::UserInfo, userId);
    WSQuery query(userInfoObject);

    return ExecuteWithRetry<UserInfoPtr>([=]()
        {
        //Execute query
        return m_repositoryClient->SendQueryRequestWithOptions(query, "", "", requestOptions, cancellationToken)->Then<UserInfoResult>
            ([=](const WSObjectsResult& result)
            {
            if (!result.IsSuccess())
                return UserInfoResult::Error(result.GetError());

            auto userInfoInstances = result.GetValue().GetInstances();
            if (userInfoInstances.IsEmpty())
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, "User does not exist.");
                return UserInfoResult::Error(Error::Id::UserDoesNotExist);
                }
            if (userInfoInstances.Size() > 1)
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, requestOptions, "Multiple users found.");
                return UserInfoResult::Error(Error::Id::Unknown);
                }
            auto user = UserInfo::Parse(*userInfoInstances.begin());

            BeMutexHolder lock(s_userInfoCacheMutex);
            if (s_userInfoCache.find(user->GetId()) == s_userInfoCache.end())
                s_userInfoCache.Insert(user->GetId(), user);
            lock.unlock();

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), requestOptions, "");
            return UserInfoResult::Success(user);
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Paulius.Valiunas              07/2017
//---------------------------------------------------------------------------------------
UsersInfoTaskPtr UserInfoManager::QueryAllUsersInfo
(
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "UserInfoManager::QueryAllUsersInfo";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::UserInfo);

    return ExecuteWithRetry<bvector<UserInfoPtr> >([=]()
        {
        //Execute query
        return m_repositoryClient->SendQueryRequestWithOptions(query, "", "", requestOptions, cancellationToken)
            ->Then<UsersInfoResult>
            ([=](const WSObjectsResult& result)
            {
            if (!result.IsSuccess())
                return UsersInfoResult::Error(result.GetError());

            s_userInfoCacheMutex.lock();
            bvector<UserInfoPtr> usersList;
            s_userInfoCache.clear();
            for (auto const& value : result.GetValue().GetInstances())
                {
                auto userInfo = UserInfo::Parse(value);
                usersList.push_back(userInfo);
                s_userInfoCache.Insert(userInfo->GetId(), userInfo);
                }
            s_userInfoCacheMutex.unlock();
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), requestOptions, "");
            return UsersInfoResult::Success(usersList);
            });
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
UsersInfoTaskPtr UserInfoManager::QueryUsersInfoByIds
(
bvector<Utf8String> userIds,
ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "UserInfoManager::QueryUsersInfoByIds";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    auto requestOptions = LogHelper::CreateiModelHubRequestOptions();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, requestOptions, "Method called.");

    InitializeUserCache();

    std::deque<ObjectId> locallyUnavailableUserObjects;
    bvector<UserInfoPtr> usersList;

    for (Utf8String userId : userIds)
        {
        auto it = s_userInfoCache.find(userId);
        if (it == s_userInfoCache.end())
            locallyUnavailableUserObjects.push_back(ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::UserInfo, userId));
        else
            usersList.push_back(it->second);
        }

    while (!locallyUnavailableUserObjects.empty())
        {
        WSQuery query = WSQuery(ServerSchema::Schema::iModel, ServerSchema::Class::UserInfo);
        query.AddFilterIdsIn(locallyUnavailableUserObjects);

        UsersInfoResultPtr queryUsersResult = ExecuteAsync(ExecuteWithRetry<bvector<UserInfoPtr> > ([=] ()
            {
            //Execute query
            return m_repositoryClient->SendQueryRequestWithOptions(query, "", "", requestOptions, cancellationToken)->Then<UsersInfoResult>
                ([=](const WSObjectsResult& result)
                {
                if (!result.IsSuccess())
                    return UsersInfoResult::Error(result.GetError());

                bvector<UserInfoPtr> usersList;
                for (auto const& value : result.GetValue().GetInstances())
                    {
                    auto userInfo = UserInfo::Parse(value);
                    usersList.push_back(userInfo);
                    }
                return UsersInfoResult::Success(usersList);
                });
            }));

        if (!queryUsersResult->IsSuccess())
            return CreateCompletedAsyncTask<UsersInfoResult>(UsersInfoResult::Error(queryUsersResult->GetError()));

        for (auto userInfo : queryUsersResult->GetValue())
            {
            usersList.push_back(userInfo);
            BeMutexHolder lock(s_userInfoCacheMutex);
            if (s_userInfoCache.find(userInfo->GetId()) == s_userInfoCache.end())
                s_userInfoCache.Insert(userInfo->GetId(), userInfo);
            lock.unlock();
            }
        }

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), requestOptions, "");

    return CreateCompletedAsyncTask<UsersInfoResult>(UsersInfoResult::Success(usersList));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
void UserInfoManager::InitializeUserCache
(
ICancellationTokenPtr cancellationToken
) const
    {
    if (s_userInfoCache.size() == 0)
        ExecuteAsync(QueryAllUsersInfo(cancellationToken));
    }