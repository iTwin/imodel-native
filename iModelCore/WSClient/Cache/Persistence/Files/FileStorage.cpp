/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Files/FileStorage.cpp $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "FileStorage.h"

#include <Bentley/BeDirectoryIterator.h>
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
    bool backupNeeded = fileToRollback.DoesPathExist() && fileToRollback != moveFromFile;

    // Create file backup if any errors would occur
    BeFileName backupForRollbackFile(fileToRollback + L".DataSourceCache_Backup");

    bool oldBackupFileExists = backupForRollbackFile.DoesPathExist();
    if (oldBackupFileExists)
        {
        LOG.warning("Found old backup cached file. Removing it");
        if (BeFileNameStatus::Success != BeFileName::BeDeleteFile(backupForRollbackFile))
            {
            LOG.errorv(L"Could not delete existing backup cached file: %ls", backupForRollbackFile.c_str());
            return ERROR;
            }
        }

    // Create backup
    if (backupNeeded)
        {
        BeFileNameStatus status = BeFileName::BeMoveFile(fileToRollback, backupForRollbackFile);
        if (BeFileNameStatus::Success != status)
            {
            LOG.errorv(L"Could not create backup for existing file: %ls", fileToRollback.c_str());
            return ERROR;
            }
        }

    // Remove any duplicates that already exist
    if (moveToFile.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::BeDeleteFile(moveToFile))
            {
            if (backupNeeded)
                {
                RollbackFile(backupForRollbackFile, fileToRollback);
                }
            LOG.errorv(L"Could not delete existing file: %ls", moveToFile.c_str());
            return ERROR;
            }
        }

    BeFileName directory = moveToFile.GetDirectoryName();
    if (!BeFileName::DoesPathExist(directory))
        {
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(directory))
            return ERROR;
        }

    BeFileNameStatus status;
    if (copyFile)
        {
        status = BeFileName::BeCopyFile(moveFromFile, moveToFile);
        }
    else
        {
        status = BeFileName::BeMoveFile(moveFromFile, moveToFile);

        //Me might not have access to move file
        if (status != BeFileNameStatus::Success &&
            status != BeFileNameStatus::IllegalName &&
            status != BeFileNameStatus::AlreadyExists &&
            status != BeFileNameStatus::CantCreate &&
            status != BeFileNameStatus::FileNotFound &&
            status != BeFileNameStatus::AccessViolation)
            {
            status = BeFileName::BeCopyFile(moveFromFile, moveToFile);

            if (status == BeFileNameStatus::Success)
                {
                status = BeFileName::BeDeleteFile(moveFromFile);
                if (status != BeFileNameStatus::Success)
                    BeFileName::BeDeleteFile(moveToFile);
                }
            }
        }

    if (BeFileNameStatus::Success != status)
        {
        if (backupNeeded)
            {
            RollbackFile(backupForRollbackFile, fileToRollback);
            }
        LOG.errorv(L"Failed to cache file from: %ls to: %ls (Error: %d)", moveFromFile.c_str(), moveToFile.c_str(), status);
        return ERROR;
        }

    // Everything went with no errors, delete file backup
    if (backupNeeded)
        {
        BeFileNameStatus status = BeFileName::BeDeleteFile(backupForRollbackFile);
        if (BeFileNameStatus::Success != status)
            {
            LOG.errorv(L"Could not remove file backup: %ls", backupForRollbackFile.c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName FileStorage::CreateNewRelativeCacheDir()
    {
    Utf8String containingFolderName;
    if (SUCCESS != m_folderNameIncrementor->IncrementWithoutSaving(containingFolderName))
        containingFolderName.clear();
    return BeFileName(containingFolderName);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileStorage::FixFileNameIfNeeded(FileCache location, BeFileNameCR relativeDir, Utf8String& fileName)
    {
    fileName = FileUtil::SanitizeFileName(fileName);
    fileName = FileUtil::TruncateFileName(fileName);

    if (fileName.empty())
        return ERROR;

    BeFileName rootPath = m_environment.GetPath(location);

    BeFileName absolutePath = rootPath;
    absolutePath.AppendToPath(relativeDir);
    absolutePath.AppendToPath(BeFileName(fileName));

    if (SUCCESS != FileUtil::TruncateFilePath(absolutePath))
        return ERROR;

    fileName = Utf8String(absolutePath.GetFileNameAndExtension());
    if (fileName.empty())
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName FileStorage::GetAbsoluteFilePath(FileCache location, BeFileNameCR relativePath)
    {
    BeFileName absolutePath;
    if (relativePath.empty())
        return absolutePath;

    absolutePath
        .AppendToPath(m_environment.GetPath(location))
        .AppendToPath(relativePath);

    return absolutePath;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileStorage::SetFileCacheLocation(FileInfo& info, FileCache location, BeFileNameCP externalRelativeDir)
    {
    location = info.GetNewLocation(location);

    if (FileCache::External == location && nullptr == externalRelativeDir ||
        FileCache::External != location && nullptr != externalRelativeDir && !externalRelativeDir->empty())
        {
        BeAssert(false);
        return ERROR;
        }

    if (info.GetFilePath().DoesPathExist())
        {
        return StoreFile(info, info.GetFilePath(), location, externalRelativeDir, false);
        }

    // File not cached, setup future location
    BeFileName relativeDir;
    if (FileCache::External == location)
        relativeDir = *externalRelativeDir;

    info.SetFilePath(location, relativeDir, nullptr);

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileStorage::CacheFile(FileInfo& info, BeFileNameCR filePath, Utf8CP cacheTag, FileCache location, bool copyFile)
    {
    location = info.GetNewLocation(location);

    LOG.infov(L"Caching file: %ls", filePath.c_str());

    if (SUCCESS != StoreFile(info, filePath, location, nullptr, copyFile))
        return ERROR;

    info.SetFileCacheTag(cacheTag);
    info.SetFileUpdateDate(DateTime::GetCurrentTimeUtc());

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileStorage::StoreFile(FileInfoR info, BeFileNameCR filePathIn, FileCache newLocation, BeFileNameCP newExternalRelativeDirIn, bool copyFile)
    {
    if (FileCache::Persistent == newLocation && m_environment.persistentFileCacheDir.empty() ||
        FileCache::Temporary == newLocation && m_environment.temporaryFileCacheDir.empty())
        {
        LOG.error("Invalid environment");
        BeAssert(false);
        return ERROR;
        }

    // Set new path
    FileCache oldLocation = info.GetLocation();
    BeFileName oldRelativePath = info.GetRelativePath();

    BeFileName newRelativeDir;
    if (FileCache::External == newLocation)
        {
        if (nullptr != newExternalRelativeDirIn)
            {
            newRelativeDir = *newExternalRelativeDirIn;
            }
        else if (FileCache::External == oldLocation)
            {
            newRelativeDir = oldRelativePath.GetDirectoryName();
            }
        }
    else if (FileCache::External != oldLocation && !oldRelativePath.empty())
        {
        newRelativeDir = oldRelativePath.GetDirectoryName();
        }
    else
        {
        newRelativeDir = CreateNewRelativeCacheDir();
        if (newRelativeDir.empty())
            return ERROR;
        }

    Utf8String newFileName(filePathIn.GetFileNameAndExtension());
    if (SUCCESS != FixFileNameIfNeeded(newLocation, newRelativeDir, newFileName))
        return ERROR;

    BeFileName oldAbsolutePath = info.GetFilePath();
    info.SetFilePath(newLocation, newRelativeDir, newFileName);
    BeFileName newAbsolutePath = info.GetFilePath();

    // Store file
    if (!newAbsolutePath.Equals(filePathIn))
        {
        if (SUCCESS != ReplaceFileWithRollback(oldAbsolutePath, filePathIn, newAbsolutePath, copyFile))
            {
            BeAssert(false);
            return ERROR;
            }
        }

    // Remove old file
    if (!oldAbsolutePath.empty() && !oldAbsolutePath.Equals(newAbsolutePath))
        {
        if (CacheStatus::OK != RemoveStoredFile(oldAbsolutePath, oldLocation, oldRelativePath, &newAbsolutePath))
            return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName FileStorage::CreateFileStoragePath(BeFileName rootDir, WStringCR cacheName)
    {
    return rootDir.AppendToPath((cacheName + L"f").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheEnvironment FileStorage::CreateCacheEnvironment(BeFileNameCR cacheFilePath, CacheEnvironmentCR baseEnvironment)
    {
    CacheEnvironment fullEnvironment;
    if (baseEnvironment.persistentFileCacheDir.empty() || baseEnvironment.temporaryFileCacheDir.empty())
        {
        return fullEnvironment;
        }

    WString cacheName = BeFileName::GetFileNameAndExtension(cacheFilePath.c_str());

    if (cacheName == L":memory:")
        cacheName = L"_in_memory_cache_";

    fullEnvironment.persistentFileCacheDir = CreateFileStoragePath(baseEnvironment.persistentFileCacheDir, cacheName);
    fullEnvironment.temporaryFileCacheDir = CreateFileStoragePath(baseEnvironment.temporaryFileCacheDir, cacheName);
    fullEnvironment.externalFileCacheDir = baseEnvironment.externalFileCacheDir;

    if (fullEnvironment.externalFileCacheDir.empty())
        {
        fullEnvironment.externalFileCacheDir = fullEnvironment.persistentFileCacheDir;
        fullEnvironment.externalFileCacheDir.AppendToPath(L"ext");
        }

    fullEnvironment.persistentFileCacheDir.AppendSeparator();
    fullEnvironment.temporaryFileCacheDir.AppendSeparator();
    fullEnvironment.externalFileCacheDir.AppendSeparator();

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
* @bsimethod                                                    Vincas.Razma    06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CacheStatus FileStorage::RemoveStoredFile(FileInfoCR info)
    {
    return RemoveStoredFile(info.GetFilePath(), info.GetLocation(), info.GetRelativePath());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CacheStatus FileStorage::RemoveStoredFile(BeFileNameCR filePath, FileCache location, BeFileNameCR relativePath, BeFileNameCP newFilePath) const
    {
    if (filePath.empty())
        return CacheStatus::OK;

    if (filePath.DoesPathExist() && BeFileNameStatus::Success != BeFileName::BeDeleteFile(filePath))
        {
        BeFile file;
        if (BeFileStatus::AccessViolationError == file.Open(filePath, BeFileAccess::ReadWrite))
            return CacheStatus::FileLocked;

        BeAssert(false);
        return CacheStatus::Error;
        }

    // Check if new file is in same folder
    if (nullptr != newFilePath && newFilePath->GetDirectoryName() == filePath.GetDirectoryName())
        return CacheStatus::OK;

    // Do folder cleanup
    if (FileCache::External == location)
        {
        CleanupDirsNotContainingFiles(m_environment.externalFileCacheDir, relativePath.GetDirectoryName());
        return CacheStatus::OK;
        }

    BeFileNameStatus status = BeFileName::EmptyAndRemoveDirectory(filePath.GetDirectoryName());
    if (status != BeFileNameStatus::Success &&
        status != BeFileNameStatus::FileNotFound)
        {
        BeAssert(false);
        return CacheStatus::Error;
        }

    return CacheStatus::OK;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FileStorage::CleanupDirsNotContainingFiles(BeFileNameCR baseDir, BeFileName relativeDir)
    {
    if (relativeDir.empty())
        return;

    BeFileName dir = baseDir;
    dir.AppendToPath(relativeDir);

    if (dir.DoesPathExist())
        {
        if (DoesDirContainFiles(dir))
            return;

        if (BeFileNameStatus::Success != BeFileName::EmptyAndRemoveDirectory(dir))
            {
            BeAssert(false);
            return;
            }
        }

    BeFileName parentRelativeDir = BeFileName(relativeDir).PopDir();
    if (parentRelativeDir == relativeDir)
        return;

    CleanupDirsNotContainingFiles(baseDir, parentRelativeDir);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FileStorage::DoesDirContainFiles(BeFileNameCR dir)
    {
    BeFileName subPath;
    bool isDir;
    for (BeDirectoryIterator it(dir); it.GetCurrentEntry(subPath, isDir) == SUCCESS; it.ToNext())
        {
        if (!isDir)
            return true;

        if (DoesDirContainFiles(subPath))
            return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileStorage::RenameCachedFile(FileInfoR info, Utf8String newFileName)
    {
    BeFileName oldPath = info.GetFilePath();
    if (oldPath.empty())
        return ERROR;

    BeFileName relativeDir = info.GetRelativePath().GetDirectoryName();
    if (SUCCESS != FixFileNameIfNeeded(info.GetLocation(), relativeDir, newFileName))
        return ERROR;

    info.SetFilePath(info.GetLocation(), relativeDir, newFileName);
    BeFileName newPath = info.GetFilePath();
    if (newPath.empty())
        return ERROR;

    if (oldPath == newPath)
        return SUCCESS;

    if (BeFileNameStatus::Success != BeFileName::BeMoveFile(oldPath, newPath))
        return ERROR;

    return SUCCESS;
    }
