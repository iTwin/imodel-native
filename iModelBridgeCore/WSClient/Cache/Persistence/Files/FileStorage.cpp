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
FileStorage::FileStorage(ECDbAdapter& dbAdapter, ECSqlStatementCache& statementCache, CacheEnvironmentCR environment) :
m_environment(environment)
    {
    ECClassCP fileCacheInfoClass = dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_FileCacheInfo);
    ECPropertyCP nameIndexProperty = fileCacheInfoClass->GetPropertyP(CLASS_FileCacheInfo_PROPERTY_LastCachedFileIndex);
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
        if (rollbackFileExists)
            {
            RollbackFile(backupForRollbackFile, fileToRollback);
            }
        LOG.errorv(L"Failed to cache file from: %ls to: %ls (Error: %d)", moveFromFile.c_str(), moveToFile.c_str(), status);
        return ERROR;
        }

    // Everything went with no errors, delete file backup
    if (rollbackFileExists)
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
BeFileName FileStorage::CreateNewRelativeCachedFilePath(Utf8StringCR fileName, FileCache location)
    {
    Utf8String containtingFolderNameUtf8;
    if (SUCCESS != m_folderNameIncrementor->IncrementWithoutSaving(containtingFolderNameUtf8))
        return BeFileName();

    BeFileName containtingFolderName(containtingFolderNameUtf8);

    BeFileName newRelativeFilePath;
    newRelativeFilePath.AppendToPath(containtingFolderName.c_str());
    newRelativeFilePath.AppendToPath(BeFileName(fileName));

    BeFileName newAbsoluteFilePath = GetAbsoluteFilePath(location, newRelativeFilePath);

    if (SUCCESS != FileUtil::TruncateFilePath(newAbsoluteFilePath))
        return BeFileName();

    newRelativeFilePath.clear();
    newRelativeFilePath.AppendToPath(containtingFolderName.c_str());
    newRelativeFilePath.AppendToPath(newAbsoluteFilePath.GetFileNameAndExtension().c_str());

    BeFileName::CreateNewDirectory(newAbsoluteFilePath.GetDirectoryName());

    return newRelativeFilePath;
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
        .AppendToPath(GetCacheEnvironmentRootPath(location))
        .AppendToPath(relativePath);

    return absolutePath;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR FileStorage::GetCacheEnvironmentRootPath(FileCache location)
    {
    if (FileCache::Persistent == location)
        {
        return m_environment.persistentFileCacheDir;
        }
    else if (FileCache::Temporary == location)
        {
        return m_environment.temporaryFileCacheDir;
        }
    else if (FileCache::External == location)
        {
        return m_environment.externalFileCacheDir;
        }

    BeAssert(false);
    static BeFileName empty;
    return empty;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileStorage::SetFileCacheLocation(FileInfo& info, FileCache location, BeFileNameCP externalRelativePath)
    {
    if (FileCache::External == location && nullptr == externalRelativePath ||
        FileCache::External != location && nullptr != externalRelativePath && !externalRelativePath->empty())
        {
        BeAssert(false);
        return ERROR;
        }

    FileCache oldFileLocation = info.GetLocation();
    BeFileName oldFileAbsolutePath = info.GetFilePath();

    if (oldFileAbsolutePath.empty())
        {
        BeFileName newFileRelativePath;
        if (FileCache::External == location)
            {
            newFileRelativePath = *externalRelativePath;
            newFileRelativePath.AppendToPath(L"placeholder");
            }
        info.SetFilePath(location, newFileRelativePath, nullptr);
        return SUCCESS;
        }

    BeFileName newFileRelativePath;
    if (FileCache::External == location)
        {
        newFileRelativePath = *externalRelativePath;
        newFileRelativePath.AppendToPath(BeFileName(info.GetFileName()));
        }
    else
        {
        newFileRelativePath = CreateNewRelativeCachedFilePath(info.GetFileName(), location);
        }

    if (newFileRelativePath.empty())
        return ERROR;

    info.SetFilePath(location, newFileRelativePath, info.GetFileName());

    BeFileName newFileAbsolutePath = info.GetFilePath();

    if (newFileAbsolutePath.Equals(oldFileAbsolutePath) || !oldFileAbsolutePath.DoesPathExist())
        return SUCCESS;

    if (SUCCESS != ReplaceFileWithRollback(BeFileName(), oldFileAbsolutePath, newFileAbsolutePath, false))
        {
        BeAssert(false);
        return ERROR;
        }

    if (SUCCESS != CleanupCachedFile(oldFileAbsolutePath, oldFileLocation))
        return ERROR;

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
FileCache location,
bool copyFile
)
    {
    LOG.infov(L"Caching file: %ls", suppliedFileAbsolutePath.c_str());

    if (FileCache::Persistent == location && m_environment.persistentFileCacheDir.empty() ||
        FileCache::Temporary == location && m_environment.temporaryFileCacheDir.empty())
        {
        LOG.error("Invalid environment");
        BeAssert(false);
        return ERROR;
        }

    // Set new file path
    Utf8String fileName(suppliedFileAbsolutePath.GetFileNameAndExtension());

    BeFileName newFileRelativePath;
    if (FileCache::External == location)
        {
        newFileRelativePath = info.GetRelativePath().GetDirectoryName();
        newFileRelativePath.AppendToPath(BeFileName(fileName));
        }
    else
        {
        newFileRelativePath = CreateNewRelativeCachedFilePath(fileName, location);
        }

    if (newFileRelativePath.empty())
        return ERROR;

    // Save cached file info
    FileCache oldFileLocation = info.GetLocation();
    BeFileName oldFileAbsolutePath;
    BeFileName newFileAbsolutePath;

    if (!info.GetFileName().empty())
        oldFileAbsolutePath = info.GetFilePath();

    info.SetFilePath(location, newFileRelativePath, fileName);
    info.SetFileCacheTag(cacheTag);

    newFileAbsolutePath = info.GetFilePath();

    // Move or copy file
    if (!newFileAbsolutePath.Equals(suppliedFileAbsolutePath))
        {
        if (SUCCESS != ReplaceFileWithRollback(oldFileAbsolutePath, suppliedFileAbsolutePath, newFileAbsolutePath, copyFile))
            {
            BeAssert(false);
            return ERROR;
            }
        }

    if (!oldFileAbsolutePath.empty() && !oldFileAbsolutePath.Equals(newFileAbsolutePath))
        {
        if (SUCCESS != CleanupCachedFile(oldFileAbsolutePath, oldFileLocation))
            return ERROR;
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
        cacheName = L"_in_memory_cache_";

    fullEnvironment.persistentFileCacheDir = GetFileCacheFolderPath(inputEnvironment.persistentFileCacheDir, cacheName);
    fullEnvironment.temporaryFileCacheDir = GetFileCacheFolderPath(inputEnvironment.temporaryFileCacheDir, cacheName);
    fullEnvironment.externalFileCacheDir = inputEnvironment.externalFileCacheDir;

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
BentleyStatus FileStorage::CleanupCachedFile(FileInfoCR info)
    {
    return CleanupCachedFile(info.GetFilePath(), info.GetLocation());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileStorage::CleanupCachedFile(BeFileNameCR filePath, FileCache location)
    {
    if (filePath.empty())
        return SUCCESS;

    if (FileCache::External == location)
        {
        if (filePath.DoesPathExist() && BeFileNameStatus::Success != BeFileName::BeDeleteFile(filePath))
            {
            BeAssert(false);
            return ERROR;
            }
        }
    else
        {
        BeFileName directoryPath(BeFileName::GetDirectoryName(filePath));
        BeFileNameStatus status = BeFileName::EmptyAndRemoveDirectory(directoryPath);
        if (status != BeFileNameStatus::Success &&
            status != BeFileNameStatus::FileNotFound)
            {
            BeAssert(false);
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
        return ERROR;

    if (oldPath == newPath)
        return SUCCESS;

    if (BeFileNameStatus::Success != BeFileName::BeMoveFile(oldPath, newPath))
        return ERROR;

    BeFileName rootPath = GetCacheEnvironmentRootPath(info.GetLocation());
    BeFileName newRelativePath(newPath.substr(rootPath.size()));

    info.SetFilePath(info.GetLocation(), newRelativePath, newFileName);
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
        return BeFileName();

    return newFilePath;
    }