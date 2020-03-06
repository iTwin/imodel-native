/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ClientInternal.h"
#include <WebServices/iModelBank/LocalhostStorageClient.h>

const IAzureBlobStorageClientFactory LocalhostStorageClient::Factory = []()
    {
    return std::static_pointer_cast<IAzureBlobStorageClient>(LocalhostStorageClient::Create());
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Marius.Balcaitis   01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus LocalhostStorageClient::FileURLToPath(Utf8StringCR url, BeFileNameR path, Utf8StringR error) const
    {
    BeUri uri(url);
    if (!uri.IsValid())
        {
        error = "Invalid file URI.";
        return BeFileNameStatus::IllegalName;
        }

    if (!uri.GetScheme().EqualsI("file"))
        {
        error = "Invalid file URI scheme.";
        return BeFileNameStatus::IllegalName;
        }

    if (!Utf8String::IsNullOrEmpty(uri.GetAuthority().c_str()))
        {
        error = "File URI with authority is not supported.";
        return BeFileNameStatus::IllegalName;
        }

    if (!Utf8String::IsNullOrEmpty(uri.GetHost().c_str()))
        {
        error = "File URI with host is not supported.";
        return BeFileNameStatus::IllegalName;
        }

    if (!Utf8String::IsNullOrEmpty(uri.GetUserInfo().c_str()))
        {
        error = "File URI with user info is not supported.";
        return BeFileNameStatus::IllegalName;
        }

    if (!Utf8String::IsNullOrEmpty(uri.GetFragment().c_str()))
        {
        error = "File URI with fragment is not supported.";
        return BeFileNameStatus::IllegalName;
        }

    if (!Utf8String::IsNullOrEmpty(uri.GetQuery().c_str()))
        {
        error = "File URI with query is not supported.";
        return BeFileNameStatus::IllegalName;
        }

    if (uri.GetPort() != -1)
        {
        error = "File URI with port is not supported.";
        return BeFileNameStatus::IllegalName;
        }

    path.SetNameUtf8(BeUri::UnescapeString(uri.GetPath()));
    if (path.IsDirectory())
        {
        error = "File URI does not point to a file.";
        path.SetNameUtf8("");
        return BeFileNameStatus::IllegalName;
        }

#if defined (BENTLEYCONFIG_OS_WINDOWS)
    if (path.StartsWith(L"\\"))
        path.Trim(L"\\");
    else if (path.StartsWith(L"/"))
        path.Trim(L"/");
#endif

    return BeFileNameStatus::Success;;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Marius.Balcaitis   01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
AzureResult LocalhostStorageClient::OpStatusToAzureResult(BeFileNameStatus status) const
    {
    switch (status)
        {
        case BentleyM0200::BeFileNameStatus::Success:
            return AzureResult::Success(AzureFileResponse());
        case BentleyM0200::BeFileNameStatus::IllegalName:
            return AzureResult::Error(AzureError(ConnectionStatus::OK, HttpStatus::BadRequest, "BadRequest", "Illegal file name.", ""));
        case BentleyM0200::BeFileNameStatus::AlreadyExists:
            return AzureResult::Error(AzureError(ConnectionStatus::OK, HttpStatus::Conflict, "Conflict", "File already exists.", ""));
        case BentleyM0200::BeFileNameStatus::CantCreate:
            return AzureResult::Error(AzureError(ConnectionStatus::OK, HttpStatus::InternalServerError, "InternalServerError", "Could not create file.", ""));
        case BentleyM0200::BeFileNameStatus::FileNotFound:
            return AzureResult::Error(AzureError(ConnectionStatus::OK, HttpStatus::NotFound, "NotFound", "File does not exist.", ""));
        case BentleyM0200::BeFileNameStatus::CantDeleteFile:
            return AzureResult::Error(AzureError(ConnectionStatus::OK, HttpStatus::InternalServerError, "InternalServerError", "Could not delete file.", ""));
        case BentleyM0200::BeFileNameStatus::AccessViolation:
            return AzureResult::Error(AzureError(ConnectionStatus::OK, HttpStatus::Forbidden, "Forbidden", "Not enough permissions.", ""));
        case BentleyM0200::BeFileNameStatus::CantDeleteDir:
            return AzureResult::Error(AzureError(ConnectionStatus::OK, HttpStatus::InternalServerError, "InternalServerError", "Could not delete directory.", ""));
        case BentleyM0200::BeFileNameStatus::UnknownError:
        default:
            return AzureResult::Error(AzureError(ConnectionStatus::OK, HttpStatus::InternalServerError, "InternalServerError", "Unknown error.", ""));
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Marius.Balcaitis   01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AzureResult> LocalhostStorageClient::CancelCopy(BeFileNameCP destFile) const
    {
    if (destFile)
        destFile->BeDeleteFile();
    return CreateCompletedAsyncTask(
        AzureResult::Error(
            AzureError(ConnectionStatus::Canceled, HttpStatus::None, "", "", "")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Marius.Balcaitis   01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AzureResult> LocalhostStorageClient::CreateAsyncTask(ICancellationTokenPtr ct, AzureResult const& result, BeFileNameCP destFile) const
    {
    return (ct && ct->IsCanceled()) ? CancelCopy(destFile) : CreateCompletedAsyncTask(result);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Marius.Balcaitis   01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AzureResult> LocalhostStorageClient::CopyFile
    (
    bool isUrlSrc,
    Utf8StringCR url,
    BeFileNameCR filePath,
    Http::Request::ProgressCallbackCR progressCallback,
    IAzureBlobStorageClient::RequestOptionsPtr options,
    ICancellationTokenPtr ct
    ) const
    {
    BeFileName urlPath;
    Utf8String error;
    if (FileURLToPath(url, urlPath, error) != BeFileNameStatus::Success)
        {
        return CreateAsyncTask(ct, AzureResult::Error(AzureError(ConnectionStatus::Canceled, HttpStatus::BadRequest, "BadRequest", error, "")), nullptr);
        }

    auto srcPath = isUrlSrc ? urlPath : filePath;
    auto destPath = isUrlSrc ? filePath : urlPath;

    if (ct && ct->IsCanceled())
        return CancelCopy(nullptr);

    auto status = BeFileName::BeCopyFile(srcPath, destPath);

    if (ct && ct->IsCanceled())
        return CancelCopy(&destPath);

    if (status == BeFileNameStatus::Success && progressCallback)
        {
        uint64_t size = 0;
        if (srcPath.GetFileSize(size) == BeFileNameStatus::Success)
            progressCallback(size, size);
        else
            progressCallback(1, 1);
        }

    return CreateAsyncTask(ct, OpStatusToAzureResult(status), &destPath);
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Marius.Balcaitis   01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AzureResult> LocalhostStorageClient::SendGetFileRequest
    (
    Utf8StringCR url,
    BeFileNameCR filePath,
    Http::Request::ProgressCallbackCR progressCallback,
    IAzureBlobStorageClient::RequestOptionsPtr options,
    ICancellationTokenPtr ct,
    int maxRetries
    ) const
    {
    return CopyFile(true, url, filePath, progressCallback, options, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Marius.Balcaitis   01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AzureResult> LocalhostStorageClient::SendUpdateFileRequest
    (
    Utf8StringCR url,
    BeFileNameCR filePath,
    Http::Request::ProgressCallbackCR progressCallback,
    IAzureBlobStorageClient::RequestOptionsPtr options,
    ICancellationTokenPtr ct
    ) const
    {
    return CopyFile(false, url, filePath, progressCallback, options, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Marius.Balcaitis   01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
LocalhostStorageClientPtr LocalhostStorageClient::Create()
    {
    return LocalhostStorageClientPtr(new LocalhostStorageClient());
    }
