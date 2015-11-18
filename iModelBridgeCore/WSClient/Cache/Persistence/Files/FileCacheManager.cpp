/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Files/FileCacheManager.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "FileCacheManager.h"

#include <WebServices/Cache/Util/FileUtil.h>

#include "../../Logging.h"
#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
FileCacheManager::FileCacheManager
(
ECDbAdapter& dbAdapter,
ECSqlStatementCache& statementCache,
CacheEnvironmentCR environment,
FileInfoManager& fileInfoManager
) :
m_environment(environment),
m_fileInfoManager(&fileInfoManager)
    {
    ECClassCP fileCacheInfoClass = dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_FileCacheInfo);
    ECPropertyCP nameIndexProperty = fileCacheInfoClass->GetPropertyP(CLASS_FileCacheInfo_PROPERTY_LastCachedFileIndex);
    m_folderNameIncrementor = std::make_shared<ValueIncrementor>(dbAdapter.GetECDb(), statementCache, *nameIndexProperty);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GetIsFilePersistent(bool& isPersistent, FileCache cacheLocation, FileInfoCR existingFileInfo)
    {
    bool cachedFileExists = existingFileInfo.IsInCache();

    if (cachedFileExists && (FileCache::Existing == cacheLocation ||
        FileCache::ExistingOrPersistent == cacheLocation ||
        FileCache::ExistingOrTemporary == cacheLocation))
        {
        isPersistent = existingFileInfo.IsFilePersistent();
        }
    else if (FileCache::Persistent == cacheLocation ||
             (!cachedFileExists && FileCache::ExistingOrPersistent == cacheLocation))
        {
        isPersistent = true;
        }
    else if (FileCache::Temporary == cacheLocation ||
             (!cachedFileExists && FileCache::ExistingOrTemporary == cacheLocation))
        {
        isPersistent = false;
        }
    else
        {
        BeAssert(false && "Unknown cache option");
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RollbackFile(BeFileNameCR backupPath, BeFileNameCR originalPath)
    {
    BeFileNameStatus status = BeFileName::BeMoveFile(backupPath, originalPath);
    if (BeFileNameStatus::Success != status)
        {
        LOG.error("Could not restore old file from backup");
        BeAssert(false);
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ReplaceFileWithRollback(BeFileNameCR fileToRollback, BeFileNameCR moveFromFile, BeFileNameCR moveToFile, bool copyFile)
    {
    bool rollbackFileExists = fileToRollback.DoesPathExist();

    // Create file backup if any errors would occur
    BeFileName backupForRollbackFile(fileToRollback + L".DataSourceCache_Backup");

    bool oldBackupFileExists = backupForRollbackFile.DoesPathExist();
    if (oldBackupFileExists)
        {
        LOG.warning("Found old backup cached file. Removing it");
        if (BeFileNameStatus::Success != BeFileName::BeDeleteFile(backupForRollbackFile))
            {
            LOG.errorv(L"Could not delete existing backup cached file: %ls", backupForRollbackFile.c_str());
            BeAssert(false);
            return ERROR;
            }
        }
    // Create backup
    if (rollbackFileExists)
        {
        BeFileNameStatus status = BeFileName::BeMoveFile(fileToRollback, backupForRollbackFile);
        if (BeFileNameStatus::Success != status)
            {
            LOG.errorv(L"Could not create backup for existing file: %ls", fileToRollback.c_str());
            BeAssert(false);
            return ERROR;
            }
        }

    // Remove any duplicates that already exist
    if (moveToFile.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::BeDeleteFile(moveToFile))
            {
            if (rollbackFileExists)
                {
                RollbackFile(backupForRollbackFile, fileToRollback);
                }
            LOG.errorv(L"Could not delete existing file: %ls", moveToFile.c_str());
            BeAssert(false);
            return ERROR;
            }
        }

    BeFileNameStatus status;
    if (copyFile)
        {
        status = BeFileName::BeCopyFile(moveFromFile, moveToFile);
        }
    else
        {
        status = BeFileName::BeMoveFile(moveFromFile, moveToFile);
        }

    if (BeFileNameStatus::Success != status)
        {
        if (rollbackFileExists)
            {
            RollbackFile(backupForRollbackFile, fileToRollback);
            }
        LOG.errorv(L"Failed to cache file from: %ls to: %ls", moveFromFile.c_str(), moveToFile.c_str());
        BeAssert(false);
        return ERROR;
        }

    // Everything went with no errors, delete file backup
    if (rollbackFileExists)
        {
        BeFileNameStatus status = BeFileName::BeDeleteFile(backupForRollbackFile);
        if (BeFileNameStatus::Success != status)
            {
            LOG.errorv(L"Could not remove file backup: %ls", backupForRollbackFile.c_str());
            BeAssert(false);
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName FileCacheManager::CreateNewRelativeCachedFilePath(BeFileNameCR currentFilePath, bool isPersistent)
    {
    Utf8String containtingFolderNameUtf8;
    if (SUCCESS != m_folderNameIncrementor->IncrementWithoutSaving(containtingFolderNameUtf8))
        {
        return BeFileName();
        }
    BeFileName containtingFolderName(containtingFolderNameUtf8);

    BeFileName newRelativeFilePath;
    newRelativeFilePath.AppendToPath(containtingFolderName.c_str());
    newRelativeFilePath.AppendToPath(currentFilePath.GetFileNameAndExtension().c_str());

    BeFileName newAbsoluteFilePath = m_fileInfoManager->GetAbsoluteFilePath(isPersistent, newRelativeFilePath);

    if (SUCCESS != FileUtil::TruncateFilePath(newAbsoluteFilePath))
        {
        return BeFileName();
        }

    newRelativeFilePath.clear();
    newRelativeFilePath.AppendToPath(containtingFolderName.c_str());
    newRelativeFilePath.AppendToPath(newAbsoluteFilePath.GetFileNameAndExtension().c_str());

    BeFileName::CreateNewDirectory(newAbsoluteFilePath.GetDirectoryName());

    return newRelativeFilePath;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileCacheManager::SetFileCacheLocation(ECInstanceKeyCR instance, FileCache cacheLocation)
    {
    FileInfo fileInfo = m_fileInfoManager->ReadInfo(instance);

    bool cachePersistentFile = false;
    if (SUCCESS != GetIsFilePersistent(cachePersistentFile, cacheLocation, fileInfo))
        {
        return ERROR;
        }

    if (fileInfo.IsInCache())
        {
        BeFileName oldFileAbsolutePath = fileInfo.GetFilePath();
        if (oldFileAbsolutePath.empty())
            {
            fileInfo.SetFilePath(cachePersistentFile, BeFileName());
            }
        else
            {
            BeFileName newFileRelativePath = CreateNewRelativeCachedFilePath(oldFileAbsolutePath, cachePersistentFile);
            if (newFileRelativePath.empty())
                {
                return ERROR;
                }

            fileInfo.SetFilePath(cachePersistentFile, newFileRelativePath);

            BeFileName newFileAbsolutePath = fileInfo.GetFilePath();

            if (!newFileAbsolutePath.Equals(oldFileAbsolutePath) &&
                oldFileAbsolutePath.DoesPathExist())
                {
                if (SUCCESS != ReplaceFileWithRollback(BeFileName(), oldFileAbsolutePath, newFileAbsolutePath, false))
                    {
                    return ERROR;
                    }
                if (SUCCESS != m_fileInfoManager->RemoveContainingFolder(oldFileAbsolutePath))
                    {
                    return ERROR;
                    }
                }
            }
        }
    else
        {
        fileInfo.SetFilePath(cachePersistentFile, BeFileName());
        }

    if (SUCCESS != m_fileInfoManager->SaveInfo(fileInfo))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileCacheManager::CacheFile
(
ECInstanceKeyCR instance,
BeFileNameCR suppliedFileAbsolutePath,
Utf8CP cacheTag,
FileCache cacheLocation,
DateTimeCR cacheDateUtc,
bool copyFile
)
    {
    LOG.infov(L"Caching file: %ls", suppliedFileAbsolutePath.c_str());

    if (!instance.IsValid())
        {
        return ERROR;
        }
    if ((FileCache::Persistent == cacheLocation || FileCache::ExistingOrPersistent == cacheLocation) && m_environment.persistentFileCacheDir.empty() ||
        (FileCache::Temporary == cacheLocation || FileCache::ExistingOrTemporary == cacheLocation) && m_environment.temporaryFileCacheDir.empty())
        {
        LOG.error("Invalid environment");
        BeAssert(false);
        return ERROR;
        }

    FileInfo fileInfo = m_fileInfoManager->ReadInfo(instance);

    // Set new file path

    bool cachePersistentFile = false;
    if (SUCCESS != GetIsFilePersistent(cachePersistentFile, cacheLocation, fileInfo))
        {
        return ERROR;
        }

    BeFileName newFileRelativePath = CreateNewRelativeCachedFilePath(suppliedFileAbsolutePath, cachePersistentFile);
    if (newFileRelativePath.empty())
        {
        return ERROR;
        }

    // Save cached file info

    BeFileName previouslyCachedFileAbsolutePath;
    BeFileName newFileAbsolutePath;

    if (!fileInfo.IsInCache())
        {
        fileInfo.SetFilePath(cachePersistentFile, newFileRelativePath);
        fileInfo.SetFileCacheDate(cacheDateUtc);
        fileInfo.SetFileCacheTag(cacheTag);

        if (SUCCESS != m_fileInfoManager->SaveInfo(fileInfo))
            {
            return ERROR;
            }
        }
    else
        {
        previouslyCachedFileAbsolutePath = fileInfo.GetFilePath();

        fileInfo.SetFilePath(cachePersistentFile, newFileRelativePath);
        fileInfo.SetFileCacheDate(cacheDateUtc);
        fileInfo.SetFileCacheTag(cacheTag);

        if (SUCCESS != m_fileInfoManager->SaveInfo(fileInfo))
            {
            return ERROR;
            }
        }

    newFileAbsolutePath = fileInfo.GetFilePath();

    // Move or copy file
    if (!newFileAbsolutePath.Equals(suppliedFileAbsolutePath))
        {
        if (SUCCESS != ReplaceFileWithRollback(previouslyCachedFileAbsolutePath, suppliedFileAbsolutePath, newFileAbsolutePath, copyFile))
            {
            return ERROR;
            }
        }

    if (!previouslyCachedFileAbsolutePath.empty() && !previouslyCachedFileAbsolutePath.Equals(newFileAbsolutePath))
        {
        if (SUCCESS != m_fileInfoManager->RemoveContainingFolder(previouslyCachedFileAbsolutePath))
            {
            LOG.errorv(L"Cannot remove old containing folder: %ls", previouslyCachedFileAbsolutePath.c_str());
            BeAssert(false);
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName GetFileCacheFolderPath(BeFileName rootDir, WStringCR cacheName)
    {
    return rootDir.AppendToPath((cacheName + L"f").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheEnvironment FileCacheManager::CreateCacheEnvironment(BeFileNameCR cacheFilePath, CacheEnvironmentCR inputEnvironment)
    {
    CacheEnvironment fullEnvironment;
    if (inputEnvironment.persistentFileCacheDir.empty() || inputEnvironment.temporaryFileCacheDir.empty())
        {
        return fullEnvironment;
        }

    WString cacheName = BeFileName::GetFileNameAndExtension(cacheFilePath.c_str());

    if (cacheName == L":memory:")
        {
        cacheName = L"_in_memory_cache_";
        }

    fullEnvironment.persistentFileCacheDir = GetFileCacheFolderPath(inputEnvironment.persistentFileCacheDir, cacheName);
    fullEnvironment.temporaryFileCacheDir = GetFileCacheFolderPath(inputEnvironment.temporaryFileCacheDir, cacheName);

    return fullEnvironment;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileCacheManager::DeleteFileCacheDirectories(CacheEnvironmentCR fullEnvironment)
    {
    if (fullEnvironment.persistentFileCacheDir.DoesPathExist() &&
        BeFileNameStatus::Success != BeFileName::EmptyAndRemoveDirectory(fullEnvironment.persistentFileCacheDir))
        {
        BeAssert(false);
        return ERROR;
        }

    if (fullEnvironment.temporaryFileCacheDir.DoesPathExist() &&
        BeFileNameStatus::Success != BeFileName::EmptyAndRemoveDirectory(fullEnvironment.temporaryFileCacheDir))
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }
