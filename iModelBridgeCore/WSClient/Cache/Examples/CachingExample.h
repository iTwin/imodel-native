/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Examples/CachingExample.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/CachingDataSource.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    00/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct CachingExample
    {
    protected:
        BeFileName                  m_workDir;
        SimpleCancellationTokenPtr  m_cancellationToken;

        void Cleanup();

        virtual ClientInfoPtr GetClientInfo();
        virtual CacheEnvironment GetCacheEnvironment();
        virtual Utf8String GetWSErrorMessage(WSErrorCR error);
        virtual Utf8String GetCachingStatusString(CachingDataSource::ErrorCR error);
        virtual void HandleGetRepositoriesSuccessResponse(const bvector<WSRepository>& repositories);
        virtual void HandleObjects(JsonValueCR data);
        virtual void HandleError(WSErrorCR error);
        virtual void HandleError(CachingDataSource::ErrorCR error);

    public:
        //! Create example
        //! @param workDir directory for test files to be created and deleted after example is done
        CachingExample(BeFileName workDir);
        ~CachingExample();

        //! Run all prepared code examples.
        //! Be sure to initialize APIs before executing caching code:
        //!     [Required] BeSQLite::EC::ECDb::Initialize()
        //!     [Required] DgnClientFx {platform specific} ::Initialize()
        //!     [Required] DgnClientFxL10N::Initialize() (error message localization needs SQLang file built from HttpError.xliff.h)
        //!     [Optional] HttpClient::InitializeNetworkActivityCallback()
        void RunExamples();

        //! Run examples one by one

        void RunExampleGetRepositories();
        void RunExampleCreateCache();

        void RunExamplesUseCache(CachingDataSourcePtr dataSource);

        AsyncTaskPtr<void> RunExampleGetObject(CachingDataSourcePtr dataSource);
        AsyncTaskPtr<void> RunExampleGetFile(CachingDataSourcePtr dataSource);
        AsyncTaskPtr<void> RunExampleGetNavigation(CachingDataSourcePtr dataSource);
        AsyncTaskPtr<void> RunExampleQuery(CachingDataSourcePtr dataSource);
        AsyncTaskPtr<void> RunExampleGetServerInfo(CachingDataSourcePtr dataSource);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
