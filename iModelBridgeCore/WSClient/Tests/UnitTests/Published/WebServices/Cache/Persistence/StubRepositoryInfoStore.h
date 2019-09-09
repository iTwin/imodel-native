/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
struct StubRepositoryInfoStore : IRepositoryInfoStore
    {
    private:
        WSInfo m_info;
        WSRepository m_repository;

    public:
        StubRepositoryInfoStore(WSInfo info = WSInfo()) : m_info(info)
            {};

        BentleyStatus CacheServerInfo(IDataSourceCache& cache, WSInfoCR info) override
            {
            m_info = info;
            return SUCCESS;
            };

        BentleyStatus CacheRepositoryInfo(IDataSourceCache& cache, WSRepositoryCR info) override
            {
            m_repository = info;
            return SUCCESS;
            };

        BentleyStatus PrepareInfo(IDataSourceCache& cache) override
            {
            return SUCCESS;
            };

        WSInfo GetServerInfo() override
            {
            return m_info;
            };

        WSRepository GetRepositoryInfo() override
            {
            return m_repository;
            }

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
