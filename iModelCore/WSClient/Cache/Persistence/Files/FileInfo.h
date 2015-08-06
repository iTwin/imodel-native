/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Files/FileInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>
#include "../Changes/ChangeInfo.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileInfo : public ChangeInfo
    {
    public:
        struct IAbsolutePathProvider;

    public:
        static const int PersistentRootFolderId;
        static const int TemporaryRootFolderId;

    private:
        Json::Value m_externalFileInfoJson;
        ECInstanceKey m_instanceKey;
        IAbsolutePathProvider* m_pathProvider;

    private:
        BeFileName GetRelativePath() const;
        void SetRelativePath(BeFileNameCR path);
        void SetIsPersistent(bool isPersistent);

    public:
        FileInfo();
        FileInfo
            (
            JsonValueCR cachedFileInfoJson,
            JsonValueCR externalFileInfoJson,
            ECInstanceKeyCR instanceKey,
            IAbsolutePathProvider* pathProvider
            );

        // Get absolute file path
        BeFileName GetFilePath() const;
        // Set file path
        void SetFilePath(bool isPersistent, BeFileNameCR relativePath);

        // Return cache tag if file is found on disk
        Utf8String GetFileCacheTag() const;
        void SetFileCacheTag(Utf8StringCR tag);

        bool IsFilePersistent() const;

        DateTime GetFileCacheDate() const;
        void SetFileCacheDate(DateTimeCR utcDate);

        ECInstanceKeyCR GetInstanceKey() const;

        JsonValueCR GetExternalFileInfoJson() const;
        JsonValueR GetExternalFileInfoJson();
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileInfo::IAbsolutePathProvider
    {
    ~IAbsolutePathProvider()
        {};
    virtual BeFileName GetAbsoluteFilePath(bool isPersistent, BeFileNameCR relativePath) const = 0;
    };

typedef FileInfo& FileInfoR;
typedef const FileInfo& FileInfoCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
