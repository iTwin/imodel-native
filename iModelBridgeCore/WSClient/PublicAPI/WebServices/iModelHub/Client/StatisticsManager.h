/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/StatisticsManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <Bentley/Tasks/CancellationToken.h>
#include <WebServices/iModelHub/Client/StatisticsInfo.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_TASKS
DEFINE_POINTER_SUFFIX_TYPEDEFS(StatisticsManager)

//=======================================================================================
//@bsistruct                                      Benas.Kikutis                 11/2017
//=======================================================================================
struct StatisticsManager : RefCountedBase
    {
    private:
        friend struct iModelConnection;
        WebServices::IWSRepositoryClientPtr m_repositoryClient;
        StatisticsManager(WebServices::IWSRepositoryClientPtr wsRepositoryClient) : m_repositoryClient(wsRepositoryClient) {};
        StatisticsManager() {};

        static void AddHasStatisticsSelect(Utf8StringR selectString, bvector<StatisticsProperties> propertiesToSelect = bvector<StatisticsProperties>());
    public:
        //! Returns all users statistics.
        //! @param[in] propertiesToSelect
        //! @param[in] cancellationToken
        IMODELHUBCLIENT_EXPORT MultipleStatisticsInfoTaskPtr QueryAllUsersStatistics(bvector<StatisticsProperties> propertiesToSelect = bvector<StatisticsProperties>(), ICancellationTokenPtr cancellationToken = nullptr) const;

        //! Returns statistics by user id.
        //! @param[in] userId
        //! @param[in] propertiesToSelect
        //! @param[in] cancellationToken
        IMODELHUBCLIENT_EXPORT StatisticsInfoTaskPtr QueryUserStatistics(Utf8StringCR userId, bvector<StatisticsProperties> propertiesToSelect = bvector<StatisticsProperties>(), ICancellationTokenPtr cancellationToken = nullptr) const;

        //! Returns multiple users statistics info by users ids.
        //! @param[in] usersIds
        //! @param[in] propertiesToSelect
        //! @param[in] cancellationToken
        IMODELHUBCLIENT_EXPORT MultipleStatisticsInfoTaskPtr QueryUsersStatistics(bvector<Utf8String> usersIds, bvector<StatisticsProperties> propertiesToSelect = bvector<StatisticsProperties>(), ICancellationTokenPtr cancellationToken = nullptr) const;
    };

END_BENTLEY_IMODELHUB_NAMESPACE
