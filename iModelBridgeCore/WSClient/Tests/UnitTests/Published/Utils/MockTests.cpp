/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/MockTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "MockTests.h"

#ifdef USE_GTEST

#include "../WebServices/Cache/MockCachingDataSource.h"
#include "../WebServices/Cache/MockQueryProvider.h"
#include "../WebServices/Cache/Persistence/MockChangeManager.h"
#include "../WebServices/Cache/Persistence/MockDataSourceCache.h"
#include "../WebServices/Cache/Persistence/MockRepositoryInfoStore.h"
#include "../WebServices/Cache/Util/MockECDbAdapter.h"
#include "../WebServices/Cache/Util/MockECDbSchemaChangeListener.h"
#include "../WebServices/Cache/Util/MockExtendedDataAdapter.h"
#include "../WebServices/Cache/Util/MockSelectProvider.h"
#include "../WebServices/Client/MockServerInfoListener.h"
#include "../WebServices/Client/MockWSClient.h"
#include "../WebServices/Client/MockWSRepositoryClient.h"
#include "../WebServices/Client/MockWSSchemaProvider.h"
#include "../WebServices/Configuration/MockBuddiClient.h"
#include "../WebServices/Connect/MockConnectAuthenticationPersistence.h"
#include "../WebServices/Connect/MockConnectTokenProvider.h"
#include "../WebServices/Connect/MockLocalState.h"
#include "../WebServices/Connect/MockImsClient.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(MockTests, Ctor_Default_Builds)
    {
    MockCachingDataSource();
    MockQueryProvider();
    MockECDbAdapter();
    MockECDbSchemaChangeListener();
    MockExtendedDataAdapter();
    MockSelectProvider();
    MockChangeManager();
    MockDataSourceCache();
    MockRepositoryInfoStore();
    MockServerInfoListener();
    MockWSClient();
    MockWSRepositoryClient();
    MockWSSchemaProvider();
    MockBuddiClient();
    MockConnectAuthenticationPersistence();
    MockConnectTokenProvider();
    MockLocalState();
    MockImsClient();
    }

#endif
