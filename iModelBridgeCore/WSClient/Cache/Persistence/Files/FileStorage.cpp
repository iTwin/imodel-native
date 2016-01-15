/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Files/FileStorage.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "FileStorage.h"

#include <WebServices/Cache/Util/FileUtil.h>

#include "../../Logging.h"
#include "../Core/CacheSchema.h"
#include "FileInfoManager.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
FileStorage::FileStorage
(
ECDbAdapter& dbAdapter,
WebServices::ECSqlStatementCache& statementCache,
CacheEnvironmentCR environment
) :
m_environment(environment)
    {
    ECClassCP sequenceClass = dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_Sequence);
    ECPropertyCP nameIndexProperty = sequenceClass->GetPropertyP(CLASS_Sequence_PROPERTY_LastCachedFileIndex);
    m_folderNameIncrementor = std::make_shared<ValueIncrementor>(dbAdapter.GetECDb(), statementCache, *nameIndexProperty);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileStorage::GetIsFilePersistent(bool& isPersistent, FileCache cacheLocation, FileInfoCR existingFileInfo)
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
BentleyStatus FileStorage::RollbackFile(BeFileNameCR backupPath, BeFileNameCR originalPath)
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
BentleyStatus FileStorage::ReplaceFileWithRollback(BeFileNameCR fileToRollback, BeFileNameCR moveFromFile, BeFileNameCR moveToFile, bool copyFile)
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
BeFileName FileStorage::CreateNewRelativeCachedFilePath(BeFileNameCR currentFilePath, bool isPersistent)
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

    BeFileName newAbsoluteFilePath = GetAbsoluteFilePath(isPersistent, newRelativeFilePath);

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
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName FileStorage::GetAbsoluteFilePath(bool isPersistent, BeFileNameCR relativePath)
    {
    BeFileName absolutePath;
    if (relativePath.empty())
        {
        return absolutePath;
        }

    WCharCP saveDirectory;
    if (isPersistent)
        {
        saveDirectory = m_environment.persistentFileCacheDir.c_str();
        }
    else
        {
        saveDirectory = m_environment.temporaryFileCacheDir.c_str();
        }

    absolutePath
        .AppendToPath(saveDirectory)
        .AppendToPath(relativePath);

    return absolutePath;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileStorage::SetFileCacheLocation(FileInfo& info, FileCache cacheLocation)
    {
    bool cachePersistentFile = false;
    if (SUCCESS != GetIsFilePersistent(cachePersistentFile, cacheLocation, info))
        {
        return ERROR;
        }

    if (!info.IsInCache())
        {
        info.SetFilePath(cachePersistentFile, BeFileName());
        return SUCCESS;
        }

    BeFileName oldFileAbsolutePath = info.GetFilePath();
    if (oldFileAbsolutePath.empty())
        {
        info.SetFilePath(cachePersistentFile, BeFileName());
        }
    else
        {
        BeFileName newFileRelativePath = CreateNewRelativeCachedFilePath(oldFileAbsolutePath, cachePersistentFile);
        if (newFileRelativePath.empty())
            {
            return ERROR;
            }

        info.SetFilePath(cachePersistentFile, newFileRelativePath);

        BeFileName newFileAbsolutePath = info.GetFilePath();

        if (!newFileAbsolutePath.Equals(oldFileAbsolutePath) &&
            oldFileAbsolutePath.DoesPathExist())
            {
            if (SUCCESS != ReplaceFileWithRollback(BeFileName(), oldFileAbsolutePath, newFileAbsolutePath, false))
                {
                return ERROR;
                }
            if (SUCCESS != RemoveContainingFolder(oldFileAbsolutePath))
                {
                return ERROR;
                }
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileStorage::CacheFile
(
FileInfo& info,
BeFileNameCR suppliedFileAbsolutePath,
Utf8CP cacheTag,
FileCache cacheLocation,
DateTimeCR cacheDateUtc,
bool copyFile
)
    {
    LOG.infov(L"Caching file: %ls", suppliedFileAbsolutePath.c_str());

    if ((FileCache::Persistent == cacheLocation || FileCache::ExistingOrPersistent == cacheLocation) && m_environment.persistentFileCacheDir.empty() ||
        (FileCache::Temporary == cacheLocation || FileCache::ExistingOrTemporary == cacheLocation) && m_environment.temporaryFileCacheDir.empty())
        {
        LOG.error("Invalid environment");
        BeAssert(false);
        return ERROR;
        }

    // Set new file path

    bool cachePersistentFile = false;
    if (SUCCESS != GetIsFilePersistent(cachePersistentFile, cacheLocation, info))
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

    if (!info.IsInCache())
        {
        info.SetFilePath(cachePersistentFile, newFileRelativePath);
        info.SetFileCacheDate(cacheDateUtc);
        info.SetFileCacheTag(cacheTag);
        }
    else
        {
        previouslyCachedFileAbsolutePath = info.GetFilePath();

        info.SetFilePath(cachePersistentFile, newFileRelativePath);
        info.SetFileCacheDate(cacheDateUtc);
        info.SetFileCacheTag(cacheTag);
        }

    newFileAbsolutePath = info.GetFilePath();

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
        if (SUCCESS != RemoveContainingFolder(previouslyCachedFileAbsolutePath))
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
BeFileName FileStorage::GetFileCacheFolderPath(BeFileName rootDir, WStringCR cacheName)
    {
    return rootDir.AppendToPath((cacheName + L"f").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheEnvironment FileStorage::CreateCacheEnvironment(BeFileNameCR cacheFilePath, CacheEnvironmentCR inputEnvironment)
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
BentleyStatus FileStorage::DeleteFileCacheDirectories(CacheEnvironmentCR fullEnvironment)
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileStorage::RemoveContainingFolder(BeFileNameCR filePath)
    {
    BeFileName directoryPath(BeFileName::GetDirectoryName(filePath));
    BeFileNameStatus status = BeFileName::EmptyAndRemoveDirectory(directoryPath);
    if (status != BeFileNameStatus::Success &&
        status != BeFileNameStatus::FileNotFound)
        {
        BeAssert(false);
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileStorage::CleanupCachedFile(BeFileNameCR filePath)
    {
    if (!filePath.empty())
        {
        if (SUCCESS != RemoveContainingFolder(filePath))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileStorage::RenameCachedFile(FileInfoR info, Utf8StringCR newFileName)
    {
    BeFileName oldPath = info.GetFilePath();
    BeFileName newPath = CreateNewFilePath(oldPath, newFileName);

    if (newPath.empty())
        {
        return ERROR;
        }

    if (oldPath == newPath)
        {
        return SUCCESS;
        }

    if (BeFileNameStatus::Success != BeFileName::BeMoveFile(oldPath, newPath))
        {
        return ERROR;
        }

    BeFileName dirPath = newPath.GetDirectoryName();
    dirPath.PopDir();

    BeFileName newRelativePath(newPath.substr(dirPath.size()));

    info.SetFilePath(info.IsFilePersistent(), newRelativePath);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName FileStorage::CreateNewFilePath(BeFileNameCR oldFilePath, Utf8String newFileName)
    {
    newFileName = FileUtil::SanitizeFileName(newFileName);
    newFileName = FileUtil::TruncateFileName(newFileName);

    BeFileName newFilePath = oldFilePath.GetDirectoryName();
    newFilePath.AppendToPath(BeFileName(newFileName));
    if (SUCCESS != FileUtil::TruncateFilePath(newFilePath))
        {
        return BeFileName();
        }

    return newFilePath;
    }