/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <Bentley/CancellationToken.h>
#include <WebServices/iModelHub/Client/UserInfo.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_TASKS
DEFINE_POINTER_SUFFIX_TYPEDEFS(UserInfoManager)

//=======================================================================================
//@bsistruct                                      Paulius.Valiunas              07/2017
//=======================================================================================
struct UserInfoManager : RefCountedBase
{
private:
    friend struct iModelConnection;
    WebServices::IWSRepositoryClientPtr m_repositoryClient;
    void InitializeUserCache(ICancellationTokenPtr cancellationToken = nullptr) const;
    UserInfoManager(WebServices::IWSRepositoryClientPtr wsRepositoryClient) : m_repositoryClient(wsRepositoryClient) {};
    UserInfoManager() {};
public:
    //! Returns all users info.
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT UsersInfoTaskPtr QueryAllUsersInfo(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns user info by id.
    //! @param[in] userId
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT UserInfoTaskPtr QueryUserInfoById(Utf8StringCR userId, ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns multiple users info by ids.
    //! @param[in] userIds
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT UsersInfoTaskPtr QueryUsersInfoByIds(bvector<Utf8String> userIds, 
                                                                ICancellationTokenPtr cancellationToken = nullptr) const;
};

END_BENTLEY_IMODELHUB_NAMESPACE