/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/UserInfoManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/UserInfoManager.h>
#include "Logging.h"
#include "Utils.h"
#include <functional>

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Paulius.Valiunas              07/2017
//---------------------------------------------------------------------------------------
UserInfoTaskPtr UserInfoManager::QueryUserInfoById
(
    Utf8StringCR userId,
    ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryUserInfoById";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    ObjectId userInfoObject(ServerSchema::Schema::iModel, ServerSchema::Class::UserInfo, userId);
    WSQuery query(userInfoObject);

    return ExecutionManager::ExecuteWithRetry<UserInfoPtr>([=] ()
        {
        //Execute query
        return m_repositoryClient->SendQueryRequest(query, "", "", cancellationToken)->Then<UserInfoResult>
            ([=] (const WSObjectsResult& result)
            {
            if (result.IsSuccess())
                {
                auto userInfoInstances = result.GetValue().GetInstances();
                if (userInfoInstances.IsEmpty())
                    {
                    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "User does not exist.");
                    return UserInfoResult::Error(Error::Id::UserDoesNotExist);
                    }
                if (userInfoInstances.Size() > 1)
                    {
                    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Multiple users found.");
                    return UserInfoResult::Error(Error::Id::Unknown);
                    }
                auto user = UserInfo::Parse(*userInfoInstances.begin());

                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "");
                return UserInfoResult::Success(user);
                }

            return UserInfoResult::Error(result.GetError());
            });
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Paulius.Valiunas              07/2017
//---------------------------------------------------------------------------------------
UsersInfoTaskPtr UserInfoManager::QueryUsersInfo
(
    ICancellationTokenPtr cancellationToken
) const
    {
    const Utf8String methodName = "iModelConnection::QueryUsersInfo";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    WSQuery query(ServerSchema::Schema::iModel, ServerSchema::Class::UserInfo);

    return ExecutionManager::ExecuteWithRetry<bvector<UserInfoPtr> >([=] ()
        {
        //Execute query
        return m_repositoryClient->SendQueryRequest(query, "", "", cancellationToken)->Then<UsersInfoResult>
            ([=] (const WSObjectsResult& result)
            {
            if (result.IsSuccess())
                {
                bvector<UserInfoPtr> usersList;
                for (auto const& value : result.GetValue().GetInstances())
                    {
                    auto userInfo = UserInfo::Parse(value);
                    usersList.push_back(userInfo);
                    }
                double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "");
                return UsersInfoResult::Success(usersList);
                }

            return UsersInfoResult::Error(result.GetError());
            });
        });
    }

