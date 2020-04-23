/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/Briefcase.h>
#include "FileLock.h"
#include "Logging.h"
#include <WebServices/iModelHub/Events/ChangeSetPostPushEvent.h>
#include <WebServices/iModelHub/Client/ChangeSetCacheManager.h>
#include <Bentley/BeFileListIterator.h>
#include "Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             01/2017
//---------------------------------------------------------------------------------------
bool ChangeSetCacheManager::TryGetChangeSetFile(BeFileName changeSetFileName, Utf8String changeSetId) const
    {
    auto preDownloadPath = BuildChangeSetPredownloadPathname(changeSetId);

    FileLock fileLock(preDownloadPath);
    bool lockResult = fileLock.Lock();
    if (!lockResult)
        {
        BeAssert(false && "File lock failed");
        return false;
        }

    if (!preDownloadPath.DoesPathExist() || changeSetFileName.DoesPathExist())
        return false;

    auto moveResult = BeFileName::BeMoveFile(preDownloadPath, changeSetFileName);
    return BeFileNameStatus::Success == moveResult;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             09/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr ChangeSetCacheManager::EnableBackgroundDownload() const
    {
    if (m_preDownloadCallback)
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());

    EventTypeSet eventTypes;
    eventTypes.insert(Event::EventType::ChangeSetPostPushEvent);

    m_preDownloadCallback = std::make_shared<EventCallback>([=](EventPtr event)
        {
        auto changeSetEventPtr = EventParser::GetChangeSetPostPushEvent(event);
        auto changeSetId = changeSetEventPtr->GetChangeSetId();

        auto preDownloadPath = BuildChangeSetPredownloadPathname(changeSetId);
        FileLock fileLock(preDownloadPath);
        bool lockResult = fileLock.Lock();
        if (!lockResult)
            {
            BeAssert(false && "File lock failed");
            return;
            }

        if (changeSetEventPtr.IsValid() && m_connection)
            {
            PredownloadChangeSet(m_connection, changeSetId)->GetResult();
            }
        });
    return m_connection->SubscribeEventsCallback(&eventTypes, m_preDownloadCallback);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             09/2017
//---------------------------------------------------------------------------------------
StatusTaskPtr ChangeSetCacheManager::DisableBackgroundDownload() const
    {
    if (m_preDownloadCallback)
        {
        return m_connection->UnsubscribeEventsCallback(m_preDownloadCallback)->Then<StatusResult>([=](StatusResultCR unsubscribeResult)
            {
            if (unsubscribeResult.IsSuccess())
                {
                m_preDownloadCallback.reset();
                }
            return unsubscribeResult;
            });
        }

    return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Algirdas.Mikoliunas                01/2017
//---------------------------------------------------------------------------------------
BeFileName ChangeSetCacheManager::BuildChangeSetPredownloadPathname(Utf8String changeSetId)
    {
    BeFileName tempPathname;
    BentleyStatus status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempPathname, L"DgnDbRev\\PreDownload");
    BeAssert(SUCCESS == status && "Cannot get pre-download directory");
    tempPathname.AppendToPath(WString(changeSetId.c_str(), true).c_str());
    tempPathname.AppendExtension(L"cs");
    return tempPathname;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Algirdas.Mikoliunas                01/2017
//---------------------------------------------------------------------------------------
uint64_t ChangeSetCacheManager::GetCacheSize(BeFileName directoryName) const
    {
    BeFileListIterator fileIterator(directoryName, false);

    BeFileName tempFileName;
    uint64_t cacheSize = 0;

    while (SUCCESS == fileIterator.GetNextFileName(tempFileName))
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
bvector<BeFileName> ChangeSetCacheManager::GetOrderedCacheFiles(BeFileName directoryName) const
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
void ChangeSetCacheManager::CheckCacheSize(BeFileName changeSetFileName) const
    {
    auto directoryName = changeSetFileName.GetDirectoryName();
    directoryName.AppendToPath(L"*.cs");

    uint64_t cacheSize = GetCacheSize(directoryName);
    int maxCacheSize = m_preDownloadCacheSize;

    if (cacheSize > maxCacheSize)
        {
        // Delete oldest files from cache
        auto cacheFiles = GetOrderedCacheFiles(directoryName);
        for (auto it = cacheFiles.begin(); it != cacheFiles.end(); it++)
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
            if (0 != BeStringUtilities::Wcsicmp(currentFile.GetName(), changeSetFileName.GetName()))
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
StatusTaskPtr ChangeSetCacheManager::PredownloadChangeSet
(
iModelConnectionCP                imodelConnectionP,
Utf8StringCR                      changeSetId,
Http::Request::ProgressCallbackCR callback,
ICancellationTokenPtr             cancellationToken
) const
    {
    const Utf8String methodName = "ChangeSetCacheManager::PredownloadChangeSet";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (changeSetId.empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "ChangeSetId not specified.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::InvalidChangeSet));
        }

    auto changeSetPreDownloadPath = BuildChangeSetPredownloadPathname(changeSetId);
    if (changeSetPreDownloadPath.DoesPathExist())
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "ChangeSet file already downloaded, skipping download.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    std::shared_ptr<StatusResult> finalResult = std::make_shared<StatusResult>();

    return imodelConnectionP->GetChangeSetByIdInternal(changeSetId, true, cancellationToken)->Then([=](ChangeSetInfoResultCR changeSetResult)
        {
        if (!changeSetResult.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, changeSetResult.GetError().GetMessage().c_str());
            finalResult->SetError(changeSetResult.GetError());
            return;
            }

        auto changeSetPtr = changeSetResult.GetValue();
        ObjectId fileObject(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet, changeSetId);
        imodelConnectionP->DownloadFileInternal(changeSetPreDownloadPath, fileObject, changeSetPtr->GetFileAccessKey(), callback, cancellationToken)
            ->Then([=](const AzureResult& downloadResult)
            {
            if (!downloadResult.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, downloadResult.GetError().GetMessage().c_str());
                finalResult->SetError(downloadResult.GetError());
                return;
                }

            CheckCacheSize(changeSetPreDownloadPath);

            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
            finalResult->SetSuccess();
            });
        })->Then<StatusResult>([=]()
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
