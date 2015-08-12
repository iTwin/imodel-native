/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Persistence/IRepositoryInfoStore.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <MobileDgn/Utils/Threading/AsyncTask.h>
#include <WebServices/Cache/Persistence/IDataSourceCache.h>
#include <WebServices/Client/WSClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE IRepositoryInfoStore
    {
    public:
        virtual ~IRepositoryInfoStore()
            {};

        virtual BentleyStatus CacheServerInfo(IDataSourceCache& cache, WSInfoCR info) = 0;

        //! Read info from database or return runtime copy without accessing cache
        virtual WSInfoCR GetServerInfo(IDataSourceCache& cache) = 0;

        virtual BentleyStatus SetCacheInitialized(IDataSourceCache& cache) = 0;
        virtual bool IsCacheInitialized(IDataSourceCache& cache) = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
