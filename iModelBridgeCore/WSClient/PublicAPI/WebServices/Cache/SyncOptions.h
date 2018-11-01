/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/SyncOptions.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Client/RequestOptions.h>
#include <ECDb/ECDbApi.h>
#include <Bentley/CancellationToken.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct SyncOptions
    {
    public:
        typedef std::function<void(ECInstanceKeyCR)> FileUploadCallback;
        
    private:
        bool m_useChangesets;
        size_t m_maxChangesetSizeBytes;
        size_t m_maxChangesetInstances;
        RequestOptions::FailureStrategy m_failureStrategy = RequestOptions::FailureStrategy::Default;

        bmap<ECInstanceKey, ICancellationTokenPtr> m_fileCancellationTokens;
        FileUploadCallback m_fileUploadFinishCallback;
        
    public:
        WSCACHE_EXPORT SyncOptions();

        //! Set to use $changeset support (if available) when syncing instances to server.
        //! This allows for multiple instances synced with one request.
        //! Notes:
        //!     Should not be used when it is expected that some of synced instances could fail as whole changeset will need to be resynced then.
        //!     If any of instances in changeset fails - whole sync would be interrupted and error returned.
        //! Default - false
        WSCACHE_EXPORT void SetUseChangesets(bool useChangesets);
        WSCACHE_EXPORT bool GetUseChangesets() const;

        //! Maximum size of request to send to server in one changeset (if enabled). Specify zero for unlimited size.
        //! Default - 4MB
        WSCACHE_EXPORT void SetMaxChangesetSize(size_t bytes);
        WSCACHE_EXPORT size_t GetMaxChangesetSize() const;

        //! Maximum count of instances to send to server in one changeset (if enabled). Specify zero for unlimited count.
        //! Could be useful when too much instances sent could bottleneck on server side.
        //! Default - 0 (unlimited).
        WSCACHE_EXPORT void SetMaxChangesetInstanceCount(size_t count);
        WSCACHE_EXPORT size_t GetMaxChangesetInstanceCount() const;
        
        //! Set failure strategy when using $changeset support (if available).
        void SetFailureStrategy(RequestOptions::FailureStrategy strategy) { m_failureStrategy = strategy; };
        RequestOptions::FailureStrategy GetFailureStrategy() const { return m_failureStrategy; };

        WSCACHE_EXPORT void AddFileCancellationToken(ECInstanceKeyCR fileKey, ICancellationTokenPtr token) { m_fileCancellationTokens[fileKey] = token; };
        WSCACHE_EXPORT ICancellationTokenPtr GetFileCancellationToken(ECInstanceKeyCR fileKey) const
            {
            auto pair = m_fileCancellationTokens.find(fileKey);
            return (pair == m_fileCancellationTokens.end() ? nullptr : pair->second);
            };
        
        WSCACHE_EXPORT void SetFileUploadFinishCallback(FileUploadCallback callback) { m_fileUploadFinishCallback = callback; };
        WSCACHE_EXPORT FileUploadCallback& GetFileUploadFinishCallback() { return m_fileUploadFinishCallback; };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
