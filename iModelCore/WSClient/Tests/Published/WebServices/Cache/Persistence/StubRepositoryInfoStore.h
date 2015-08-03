/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Persistence/StubRepositoryInfoStore.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Persistence/IRepositoryInfoStore.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubRepositoryInfoStore : public IRepositoryInfoStore
    {
    private:
        WSInfo m_info;

    public:
        StubRepositoryInfoStore(WSInfo info = WSInfo()) : m_info(info)
            {};

        BentleyStatus CacheServerInfo(IDataSourceCache& cache, WSInfoCR info) override
            {
            m_info = info;
            return SUCCESS;
            };

        WSInfoCR GetServerInfo(IDataSourceCache& cache) override
            {
            return m_info;
            };

        BentleyStatus SetCacheInitialized(IDataSourceCache& cache) override
            {
            return SUCCESS;
            };

        bool IsCacheInitialized(IDataSourceCache& cache) override
            {
            return true;
            };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
