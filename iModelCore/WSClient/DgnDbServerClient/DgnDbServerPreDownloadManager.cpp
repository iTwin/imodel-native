/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerPreDownloadManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbBriefcase.h>
#include "DgnDbServerPreDownloadManager.h"
#include <DgnDbServer/Client/Logging.h>
#include <DgnDbServer/Client/Events/DgnDbServerRevisionEvent.h>
#include <Bentley/BeFileListIterator.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
std::shared_ptr<BeMutex> DgnDbServerPreDownloadManager::GetRevisionMutex(Utf8String revisionId)
    {
    BeMutexHolder lock(m_revisionMutex);

    auto existingMutex = m_revisionMutexSet.find(revisionId);
    if (existingMutex != m_revisionMutexSet.end())
        return existingMutex->second;

    std::shared_ptr<BeMutex> mutex = std::make_shared<BeMutex>();
    m_revisionMutexSet.Insert(revisionId, mutex);
    return mutex;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
bool DgnDbServerPreDownloadManager::TryGetRevisionFile(BeFileName revisionFileName, Utf8String revisionId)
    {
    auto preDownloadPath = BuildRevisionPreDownloadPathname(revisionId);
    
    auto mutex = GetRevisionMutex(revisionId);
    BeMutexHolder lock(*mutex);

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

        auto mutex = GetRevisionMutex(revisionId);
        BeMutexHolder lock(*mutex);
        if (revisionEventPtr && repositoryConnectionP)
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
    directoryName.AppendToPath(L"*");

    uint64_t cacheSize = GetCacheSize(directoryName);
    if (cacheSize > s_maxCacheSize)
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

            // Delete file from cache
            BeFileNameStatus deleteStatus = currentFile.BeDeleteFile();
            BeAssert(BeFileNameStatus::Success == deleteStatus);
            cacheFiles.erase(it);

            if (cacheSize <= s_maxCacheSize)
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

    return repositoryConnectionP->GetRevisionInfoById(revisionId, cancellationToken)->Then([=](DgnDbServerRevisionInfoResultCR revisionResult)
        {
        if (!revisionResult.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, revisionResult.GetError().GetMessage().c_str());
            finalResult->SetError(revisionResult.GetError());
            return;
            }

        auto revisionPtr = revisionResult.GetValue();
        repositoryConnectionP->DownloadRevisionFileInternal(revisionId, revisionPtr->GetUrl(), revisionPreDownloadPath, callback, cancellationToken)->Then([=](DgnDbServerStatusResultCR downloadResult)
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
