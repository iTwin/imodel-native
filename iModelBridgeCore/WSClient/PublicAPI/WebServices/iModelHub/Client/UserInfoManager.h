/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/UserInfoManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <Bentley/Tasks/CancellationToken.h>
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
    UserInfoManager(WebServices::IWSRepositoryClientPtr wsRepositoryClient) : m_repositoryClient(wsRepositoryClient) {}
    UserInfoManager() {};

public:
    //! Returns all users info.
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT UsersInfoTaskPtr QueryUsersInfo(ICancellationTokenPtr cancellationToken = nullptr) const;

    //! Returns user info by id.
    //! @param[in] userId
    //! @param[in] cancellationToken
    IMODELHUBCLIENT_EXPORT UserInfoTaskPtr QueryUserInfoById(Utf8StringCR userId, ICancellationTokenPtr cancellationToken = nullptr) const;
};

END_BENTLEY_IMODELHUB_NAMESPACE
