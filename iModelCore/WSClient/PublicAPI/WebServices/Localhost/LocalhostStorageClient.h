/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Azure/AzureBlobStorageClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_TASKS

//--------------------------------------------------------------------------------------+
// WebServices Client API for connecting to Azure blob storage.
//--------------------------------------------------------------------------------------+
typedef std::shared_ptr<struct LocalhostStorageClient> LocalhostStorageClientPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                  Marius.Balcaitis   01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalhostStorageClient : public IAzureBlobStorageClient
{
private:
    //! Converts URL with file scheme to local path. URLs with network locations, i.e. host, port and user info are not supported.
    BeFileNameStatus FileURLToPath(Utf8StringCR url, BeFileNameR path, Utf8StringR error) const;

    //! Converts copy operation status to Azure result.
    AzureResult OpStatusToAzureResult(BeFileNameStatus status) const;

    //! Cancels current copy operation.
    AsyncTaskPtr<AzureResult> CancelCopy(BeFileNameCP destFile) const;

    //! Creates async task for current copy operation.
    AsyncTaskPtr<AzureResult> CreateAsyncTask(ICancellationTokenPtr ct, AzureResult const& result, BeFileNameCP destFile) const;

    //! Copy locally stored files.
    AsyncTaskPtr<AzureResult> CopyFile
        (
        bool isUrlSrc,
        Utf8StringCR url,
        BeFileNameCR filePath,
        Http::Request::ProgressCallbackCR progressCallback = nullptr,
        RequestOptionsPtr options = nullptr,
        ICancellationTokenPtr ct = nullptr
        ) const;

public:
    //! Factory for creating this client instances.
    WSCLIENT_EXPORT static const IAzureBlobStorageClientFactory Factory;

    //! Creates instance of this storage client.
    WSCLIENT_EXPORT static LocalhostStorageClientPtr Create();

    //! Download file from Azure blob that is stored locally
    WSCLIENT_EXPORT AsyncTaskPtr<AzureResult> SendGetFileRequest
        (
        Utf8StringCR url,
        BeFileNameCR filePath,
        Http::Request::ProgressCallbackCR progressCallback = nullptr,
        RequestOptionsPtr options = nullptr,
        ICancellationTokenPtr ct = nullptr
        ) const override;

    //! Update file in Azure blob that is stored locally.
    WSCLIENT_EXPORT AsyncTaskPtr<AzureResult> SendUpdateFileRequest
        (
        Utf8StringCR url,
        BeFileNameCR filePath,
        Http::Request::ProgressCallbackCR progressCallback = nullptr,
        RequestOptionsPtr options = nullptr,
        ICancellationTokenPtr ct = nullptr
        ) const override;
};

END_BENTLEY_WEBSERVICES_NAMESPACE
