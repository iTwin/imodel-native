/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/FileInfo.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerResult.h>
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnDb.h>
#include <Bentley/DateTime.h>
#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/Response/WSObjectsReader.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_WEBSERVICES
typedef RefCountedPtr<struct FileInfo> FileInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(FileInfo);

typedef RefCountedPtr<struct DgnDbServerFileAccessKey> DgnDbServerFileAccessKeyPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbServerFileAccessKey);
DEFINE_TASK_TYPEDEFS(DgnDbServerFileAccessKeyPtr, DgnDbServerFileAccessKey);

enum class InitializationState
	{
	Success,
	NotStarted,
	Scheduled,
	Failed,
	OutdatedFile
	};

//=======================================================================================
//! Information about revision file that is on server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct FileInfo : RefCountedBase
{
friend struct DgnDbRepositoryConnection;

private:
    int32_t     m_index;
    Utf8String  m_fileName;
    BeGuid      m_fileId;
    Utf8String  m_mergedRevisionId;
    Utf8String  m_description;
    uint64_t    m_fileSize;
    Utf8String  m_userUploaded;
    DateTime    m_uploadedDate;
    bool        m_areFileDetailsAvailable;
    InitializationState m_initialized;
    DgnDbServerFileAccessKeyPtr m_fileAccessKey;
    bool        m_containsFileAccessKey;

    FileInfo(BeGuid fileId) : m_fileId(fileId), m_areFileDetailsAvailable(false), m_index(-1), m_fileSize(0) {}
    DGNDBSERVERCLIENT_EXPORT FileInfo(Dgn::DgnDbCR db, Utf8StringCR description);
    DGNDBSERVERCLIENT_EXPORT FileInfo(int32_t index, Utf8StringCR fileName, Utf8StringCR fileId, Utf8StringCR mergedRevisionId,
        Utf8StringCR description, uint64_t size, Utf8StringCR user, DateTimeCR date);

    DGNDBSERVERCLIENT_EXPORT static FileInfoPtr Parse(RapidJsonValueCR properties, Utf8StringCR instanceId, FileInfoCR fileInfo = FileInfo());

    bool GetContainsFileAccessKey() const {return m_containsFileAccessKey;}
    DgnDbServerFileAccessKeyPtr GetFileAccessKey() const {return m_fileAccessKey;}
    void SetFileAccessKey(DgnDbServerFileAccessKeyPtr fileAccessKey) {m_containsFileAccessKey = true; m_fileAccessKey = fileAccessKey;}

public:
    FileInfo() : m_index(-1), m_fileSize(0), m_areFileDetailsAvailable(false) {}

    static FileInfoPtr Create(BeGuid fileId) {return FileInfoPtr(new FileInfo(fileId));}
    //! DEPRECATED: Use Parsing from Instance
    DGNDBSERVERCLIENT_EXPORT static FileInfoPtr Parse(JsonValueCR json, FileInfoCR fileInfo = FileInfo());
    DGNDBSERVERCLIENT_EXPORT static FileInfoPtr Parse(WSObjectsReader::Instance instance, FileInfoCR fileInfo = FileInfo());
    DGNDBSERVERCLIENT_EXPORT void ToPropertiesJson(JsonValueR json) const;
    DGNDBSERVERCLIENT_EXPORT WebServices::ObjectId GetObjectId() const;
    bool AreFileDetailsAvailable() const {return m_areFileDetailsAvailable;}

    static FileInfoPtr Create(Dgn::DgnDbCR db, Utf8StringCR description) {return FileInfoPtr(new FileInfo(db, description));}
    int32_t GetIndex() const {return m_index;} //!< Index of the file.
    Utf8StringCR GetFileName() const {return m_fileName;} //!< Name of the file.
    BeGuid GetFileId() const {return m_fileId;} //!< Db Guid of the file.
    Utf8StringCR GetMergedRevisionId() const {return m_mergedRevisionId;} //!< Get Last Revision Id merged to the repository.
    Utf8StringCR GetDescription() const {return m_description;} //!< Description of the file.
    uint64_t GetSize() const {return m_fileSize;} //!< Size of the file.
    Utf8StringCR GetUserUploaded() const {return m_userUploaded;} //!< Name of the user that uploaded the file.
    DateTimeCR GetUploadedDate() const {return m_uploadedDate;} //!< Date when the file was uploaded.
    InitializationState GetInitialized() const {return m_initialized;} //!< State of file initialization.
};

//=======================================================================================
//! File access key info for Azure blobs.
//@bsiclass                                   Algirdas.Mikoliunas             02/2017
//=======================================================================================
struct DgnDbServerFileAccessKey : RefCountedBase
{
private:
    Utf8String  m_downloadUrl;
    Utf8String  m_uploadUrl;
    DateTime    m_createDate;

    friend struct DgnDbRepositoryConnection;

    static Utf8String GetProperty(RapidJsonValueCR properties, Utf8StringCR member);

    static DgnDbServerFileAccessKeyPtr Parse(RapidJsonValueCR properties);
    static DgnDbServerFileAccessKeyPtr ParseFromRelated(JsonValueCR json);
    static DgnDbServerFileAccessKeyPtr ParseFromRelated(WSObjectsReader::Instance instance);

public:
    //! Select download access key
    static void AddDownloadAccessKeySelect(Utf8StringR);
    //! Select upload access key
    static void AddUploadAccessKeySelect(Utf8StringR);
    //! Url for upload
    Utf8StringCR GetUploadUrl() const {return m_uploadUrl;}
    //! Url for download
    Utf8StringCR GetDownloadUrl() const {return m_downloadUrl;}
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
