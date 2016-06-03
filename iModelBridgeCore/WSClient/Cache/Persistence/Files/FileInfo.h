/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Files/FileInfo.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>
#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>
#include "../Changes/ChangeInfo.h"
#include "../Instances/ObjectInfo.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileInfo : public ChangeInfo
    {
    public:
        struct IAbsolutePathProvider;

    private:
        CachedInstanceKey m_cachedInstanceKey;
        Json::Value m_externalFileInfoJson;
        IAbsolutePathProvider* m_pathProvider;

    private:
        void SetRelativePath(BeFileNameCR relativePath);
        void SetFileName(Utf8StringCR fileName);
        void SetLocation(FileCache location);

    public:
        FileInfo();
        FileInfo
            (
            JsonValueCR cachedFileInfoJson,
            JsonValueCR externalFileInfoJson,
            CachedInstanceKey cachedInstanceKey,
            IAbsolutePathProvider* pathProvider
            );

        bool IsValid() const;
            
        //! Get name of the file. 
        Utf8String GetFileName() const;
        //! Get absolute file path
        BeFileName GetFilePath() const;
        
        //! Set file path. Set fileName to empty if file is not yet cached
        void SetFilePath(FileCache location, BeFileNameCR relativePath, Utf8StringCR fileName);
        //! Get relative path for the file
        BeFileName GetRelativePath() const;

        //! Set cache location
        FileCache GetLocation(FileCache defaultLocation = FileCache::Temporary) const;

        //! Return cache tag if file is found on disk
        Utf8String GetFileCacheTag() const;
        void SetFileCacheTag(Utf8StringCR tag);

        //! Get date when file was last synced
        DateTime GetFileCacheDate() const;
        void SetFileCacheDate(DateTimeCR utcDate);

        CachedInstanceKeyCR GetCachedInstanceKey() const;

        JsonValueCR GetExternalFileInfoJson() const;
        JsonValueR GetExternalFileInfoJson();
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileInfo::IAbsolutePathProvider
    {
    ~IAbsolutePathProvider() {};
    virtual BeFileName GetAbsoluteFilePath(FileCache location, BeFileNameCR relativePath) const = 0;
    };

typedef FileInfo& FileInfoR;
typedef const FileInfo& FileInfoCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
