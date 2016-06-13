/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Files/FileInfo.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "FileInfo.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../Core/CacheSchema.h"
#include "../Core/ECDbFileInfoSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
FileInfo::FileInfo() :
m_pathProvider(nullptr)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
FileInfo::FileInfo
(
JsonValueCR cachedFileInfoJson,
JsonValueCR externalFileInfoJson,
CachedInstanceKey cachedInstanceKey,
IAbsolutePathProvider* pathProvider
) :
ChangeInfo(cachedFileInfoJson),
m_cachedInstanceKey(cachedInstanceKey),
m_externalFileInfoJson(externalFileInfoJson),
m_pathProvider(pathProvider)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FileInfo::IsValid() const
    {
    return m_cachedInstanceKey.IsValid();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedInstanceKeyCR FileInfo::GetCachedInstanceKey() const
    {
    return m_cachedInstanceKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName FileInfo::GetFilePath() const
    {
    if (nullptr == m_pathProvider)
        return BeFileName();

    if (GetFileName().empty())
        return BeFileName();

    return m_pathProvider->GetAbsoluteFilePath(GetLocation(), GetRelativePath());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void FileInfo::SetFilePath(FileCache location, BeFileName relativeDir, Utf8StringCR fileName)
    {
    if (!relativeDir.empty())
        relativeDir.AppendSeparator();

    relativeDir.AppendToPath(BeFileName(fileName));

    SetLocation(location);
    SetRelativePath(relativeDir);
    SetFileName(fileName);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName FileInfo::GetRelativePath() const
    {
    return BeFileName(m_externalFileInfoJson[CLASS_ExternalFileInfo_PROPERTY_RelativePath].asString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void FileInfo::SetRelativePath(BeFileNameCR relativePath)
    {
    m_externalFileInfoJson[CLASS_ExternalFileInfo_PROPERTY_RelativePath] = relativePath.GetNameUtf8();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String FileInfo::GetFileName() const
    {
    return m_externalFileInfoJson[CLASS_FileInfo_PROPERTY_Name].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void FileInfo::SetFileName(Utf8StringCR fileName)
    {
    m_externalFileInfoJson[CLASS_FileInfo_PROPERTY_Name] = fileName;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String FileInfo::GetFileCacheTag() const
    {
    BeFileName path = GetFilePath();
    if (path.empty() || !path.DoesPathExist())
        return nullptr;

    return m_infoJson[CLASS_CachedFileInfo_PROPERTY_CacheTag].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void FileInfo::SetFileCacheTag(Utf8StringCR tag)
    {
    m_infoJson[CLASS_CachedFileInfo_PROPERTY_CacheTag] = tag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
FileCache FileInfo::GetLocation(FileCache defaultLocation) const
    {
    JsonValueCR idJson = m_externalFileInfoJson[CLASS_ExternalFileInfo_PROPERTY_RootFolder];
    if (!idJson.isInt())
        return defaultLocation;

    return static_cast<FileCache>(idJson.asInt());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void FileInfo::SetLocation(FileCache location)
    {
    m_externalFileInfoJson[CLASS_ExternalFileInfo_PROPERTY_RootFolder] = static_cast<int>(location);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime FileInfo::GetFileCacheDate() const
    {
    BeFileName path = GetFilePath();
    if (path.empty() || !path.DoesPathExist())
        return DateTime();

    return BeJsonUtilities::DateTimeFromValue(m_infoJson[CLASS_CachedFileInfo_PROPERTY_CacheDate]);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void FileInfo::SetFileCacheDate(DateTimeCR utcDate)
    {
    m_infoJson[CLASS_CachedFileInfo_PROPERTY_CacheDate] = ECDbHelper::UtcDateToString(utcDate);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime FileInfo::GetFileUpdateDate() const
    {
    return BeJsonUtilities::DateTimeFromValue(m_infoJson[CLASS_CachedFileInfo_PROPERTY_UpdateDate]);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              
+---------------+---------------+---------------+---------------+---------------+------*/
void FileInfo::SetFileUpdateDate(DateTimeCR utcDate)
    {
    m_infoJson[CLASS_CachedFileInfo_PROPERTY_UpdateDate] = ECDbHelper::UtcDateToString(utcDate);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueCR FileInfo::GetExternalFileInfoJson() const
    {
    return m_externalFileInfoJson;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueR FileInfo::GetExternalFileInfoJson()
    {
    return m_externalFileInfoJson;
    }
