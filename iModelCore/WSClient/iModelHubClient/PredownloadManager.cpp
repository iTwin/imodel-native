/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/PredownloadManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbBriefcase.h>
#include "DgnDbServerPreDownloadManager.h"
#include <DgnDbServer/Client/Logging.h>
#include <DgnDbServer/Client/Events/DgnDbServerRevisionEvent.h>
#include <DgnDbServer/Client/DgnDbServerConfiguration.h>
#include <Bentley/BeFileListIterator.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
bool DgnDbServerPreDownloadManager::TryGetRevisionFile(BeFileName revisionFileName, Utf8String revisionId)
    {
    auto preDownloadPath = BuildRevisionPreDownloadPathname(revisionId);
    
    FileLock fileLock(preDownloadPath);
    bool lockResult = fileLock.Lock();
    if (!lockResult)
        {
        BeAssert(false && "File lock failed");
        return false;
        }

    if (!preDownloadPath.DoesPathExist() || revisionFileName.DoesPathExist())
        return false;

    auto moveResult = BeFileName::BeMoveFile(preDownloadPath, revisionFileName);
    return BeFileNameStatus::Success == moveResult;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
void DgnDbServerPreDownloadManager::SubscribeRevisionsDownload(DgnDbRepositoryConnectionP repositoryConnectionP)
    {
    DgnDbServerEventTypeSet eventTypes;
    eventTypes.insert(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent);

    auto preDownloadCallback = std::make_shared<DgnDbServerEventCallback>([=](DgnDbServerEventPtr event)
        {
        auto revisionEventPtr = DgnDbServerEventParser::GetRevisionEvent(event);
        auto revisionId = revisionEventPtr->GetRevisionId();

        auto preDownloadPath = BuildRevisionPreDownloadPathname(revisionId);
        FileLock fileLock(preDownloadPath);
        bool lockResult = fileLock.Lock();
        if (!lockResult)
            {
            BeAssert(false && "File lock failed");
            return;
            }
            
        if (revisionEventPtr.IsValid() && repositoryConnectionP)
            {
            PreDownloadRevision(repositoryConnectionP, revisionId)->GetResult();
            }
        });
    repositoryConnectionP->SubscribeEventsCallback(&eventTypes, preDownloadCallback);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Algirdas.Mikoliunas                01/2017
//---------------------------------------------------------------------------------------
BeFileName DgnDbServerPreDownloadManager::BuildRevisionPreDownloadPathname(Utf8String revisionId)
    {
    BeFileName tempPathname;
    BentleyStatus status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempPathname, L"DgnDbRev\\PreDownload");
    BeAssert(SUCCESS == status && "Cannot get pre-download directory");
    tempPathname.AppendToPath(WString(revisionId.c_str(), true).c_str());
    tempPathname.AppendExtension(L"rev");
    return tempPathname;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Algirdas.Mikoliunas                01/2017
//---------------------------------------------------------------------------------------
uint64_t DgnDbServerPreDownloadManager::GetCacheSize(BeFileName directoryName) const
    {
    BeFileListIterator fileIterator(directoryName, false);

    BeFileName tempFileName;
    uint64_t cacheSize = 0;

    while(SUCCESS == fileIterator.GetNextFileName(tempFileName))
        {
        uint64_t fileSize;
        BeFileNameStatus status = tempFileName.GetFileSize(fileSize);
        BeAssert(BeFileNameStatus::Success == status);
        cacheSize += fileSize;
        }

    return cacheSize;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Algirdas.Mikoliunas                01/2017
//---------------------------------------------------------------------------------------
bvector<BeFileName> DgnDbServerPreDownloadManager::GetOrderedCacheFiles(BeFileName directoryName) const
    {
    BeFileListIterator fileIterator(directoryName, false);
    BeFileName tempFileName;
    bvector<BeFileName> cacheFiles;

    while (SUCCESS == fileIterator.GetNextFileName(tempFileName))
        {
        cacheFiles.push_back(tempFileName);
        }

    std::sort(cacheFiles.begin(), cacheFiles.end(), [](BeFileName a, BeFileName b)
        {
        time_t aCreateTime, bCreateTime;
        a.GetFileTime(&aCreateTime, nullptr, nullptr);
        b.GetFileTime(&bCreateTime, nullptr, nullptr);

        return aCreateTime < bCreateTime;
        });

    return cacheFiles;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Algirdas.Mikoliunas                01/2017
//---------------------------------------------------------------------------------------
void DgnDbServerPreDownloadManager::CheckCacheSize(BeFileName revisionFileName) const
    {
    auto directoryName = revisionFileName.GetDirectoryName();
    directoryName.AppendToPath(L"*.rev");

    uint64_t cacheSize = GetCacheSize(directoryName);
    int maxCacheSize = DgnDbServerConfiguration::GetPreDownloadRevisionsCacheSize();

    if (cacheSize > maxCacheSize)
        {
        // Delete oldest files from cache
        auto cacheFiles = GetOrderedCacheFiles(directoryName);
        for (auto it = cacheFiles.begin(); it != cacheFiles.end(); it ++)
            {
            if (cacheFiles.size() <= 1)
                break;

            auto currentFile = *it;
            uint64_t currentFileSize;

            // Update remaining cache size
            currentFile.GetFileSize(currentFileSize);
            cacheSize -= currentFileSize;
            
            // Get lock for the file
            FileLock lock(currentFile);
            if (0 != BeStringUtilities::Wcsicmp(currentFile.GetName(), revisionFileName.GetName()))
                {
                if (!lock.Lock())
                    {
                    BeAssert(false && "Could not get lock for the file.");
                    break;
                    }
                }

            // Delete file from cache
            BeFileNameStatus deleteStatus = currentFile.BeDeleteFile();
            BeAssert(BeFileNameStatus::Success == deleteStatus);
            cacheFiles.erase(it);

            if (cacheSize <= maxCacheSize)
                break;
            }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbServerPreDownloadManager::PreDownloadRevision
(
DgnDbRepositoryConnectionP        repositoryConnectionP,
Utf8StringCR                      revisionId,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr             cancellationToken
) const
    {
    const Utf8String methodName = "DgnDbServerPreDownloadManager::PreDownloadRevision";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (revisionId.empty())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Revision Id not specified.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::InvalidRevision));
        }

    auto revisionPreDownloadPath = BuildRevisionPreDownloadPathname(revisionId);
    if (revisionPreDownloadPath.DoesPathExist())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Revision file already downloaded, skipping download.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Success());
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<DgnDbServerStatusResult> finalResult = std::make_shared<DgnDbServerStatusResult>();

    return repositoryConnectionP->GetRevisionByIdInternal(revisionId, true, cancellationToken)->Then([=](DgnDbServerRevisionInfoResultCR revisionResult)
        {
        if (!revisionResult.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, revisionResult.GetError().GetMessage().c_str());
            finalResult->SetError(revisionResult.GetError());
            return;
            }

        auto revisionPtr = revisionResult.GetValue();
        ObjectId fileObject(ServerSchema::Schema::Repository, ServerSchema::Class::Revision, revisionId);
        repositoryConnectionP->DownloadFileInternal(revisionPreDownloadPath, fileObject, revisionPtr->GetFileAccessKey(), callback, cancellationToken)->Then([=](DgnDbServerStatusResultCR downloadResult)
            {
            if (!downloadResult.IsSuccess())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, downloadResult.GetError().GetMessage().c_str());
                finalResult->SetError(downloadResult.GetError());
                return;
                }

            CheckCacheSize(revisionPreDownloadPath);

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            finalResult->SetSuccess();
            });
        })->Then<DgnDbServerStatusResult>([=]()
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
bool FileLock::Lock()
    {
    BeFile lockFile;

    int maxIterations = 100;
    while (maxIterations > 0)
        {
        if (BeFileStatus::Success == lockFile.Create(m_lockFilePath, false))
            {
            m_locked = true;
            return true;
            }

        BeThreadUtilities::BeSleep(50);
        maxIterations--;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
FileLock::~FileLock()
    {
    if (m_locked && m_lockFilePath.DoesPathExist())
        {
        m_lockFilePath.BeDeleteFile();
        }
    }
