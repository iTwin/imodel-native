/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/FileInfo.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/Result.h>
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnDb.h>
#include <Bentley/DateTime.h>
#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/Response/WSObjectsReader.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
struct FileInfo;
typedef RefCountedPtr<struct FileInfo> FileInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(FileInfo);

typedef RefCountedPtr<struct FileAccessKey> FileAccessKeyPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(FileAccessKey);
DEFINE_TASK_TYPEDEFS(FileAccessKeyPtr, FileAccessKey);

enum class InitializationState
	{
	Success,
	NotStarted,
	Scheduled,
    UnknownFailure,
	OutdatedFile,
    IncorrectFileId
	};

//=======================================================================================
//! Information about changeSet file that is on server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct FileInfo : RefCountedBase
{
friend struct iModelConnection;
friend struct Client;

private:
    int32_t     m_index;
    Utf8String  m_fileName;
    BeSQLite::BeGuid      m_fileId;
    Utf8String  m_mergedChangeSetId;
    Utf8String  m_description;
    uint64_t    m_fileSize;
    Utf8String  m_userUploaded;
    DateTime    m_uploadedDate;
    bool        m_areFileDetailsAvailable;
    InitializationState m_initialized;
    FileAccessKeyPtr m_fileAccessKey;
    bool        m_containsFileAccessKey = false;

    FileInfo() : m_index(-1), m_fileSize(0), m_areFileDetailsAvailable(false) {}
    FileInfo(BeSQLite::BeGuid fileId) : m_fileId(fileId), m_areFileDetailsAvailable(false), m_index(-1), m_fileSize(0) {}
    IMODELHUBCLIENT_EXPORT FileInfo(Dgn::DgnDbCR db, Utf8StringCR description);
    FileInfo(int32_t index, Utf8StringCR fileName, Utf8StringCR fileId, Utf8StringCR mergedChangeSetId,
        Utf8StringCR description, uint64_t size, Utf8StringCR user, DateTimeCR date);

    static FileInfoPtr Parse(RapidJsonValueCR properties, Utf8StringCR instanceId, FileInfoCR fileInfo = FileInfo());

    bool GetContainsFileAccessKey() const {return m_containsFileAccessKey;}
    FileAccessKeyPtr GetFileAccessKey() const {return m_fileAccessKey;}
    void SetFileAccessKey(FileAccessKeyPtr fileAccessKey) {m_containsFileAccessKey = true; m_fileAccessKey = fileAccessKey;}

    static FileInfoPtr Create(BeSQLite::BeGuid fileId) { return FileInfoPtr(new FileInfo(fileId)); }
    static FileInfoPtr Parse(WebServices::WSObjectsReader::Instance instance, FileInfoCR fileInfo = FileInfo());
    void ToPropertiesJson(JsonValueR json) const;
    WebServices::ObjectId GetObjectId() const;
    bool AreFileDetailsAvailable() const { return m_areFileDetailsAvailable; }
public:
    static FileInfoPtr Create(Dgn::DgnDbCR db, Utf8StringCR description) {return FileInfoPtr(new FileInfo(db, description));}
    int32_t GetIndex() const {return m_index;} //!< Index of the file.
    Utf8StringCR GetFileName() const {return m_fileName;} //!< Name of the file.
    BeSQLite::BeGuid GetFileId() const {return m_fileId;} //!< Db Guid of the file.
    Utf8StringCR GetMergedChangeSetId() const {return m_mergedChangeSetId;} //!< Get Last ChangeSet Id merged to the iModel.
    Utf8StringCR GetDescription() const {return m_description;} //!< Description of the file.
    uint64_t GetSize() const {return m_fileSize;} //!< Size of the file.
    Utf8StringCR GetUserUploaded() const {return m_userUploaded;} //!< Name of the user that uploaded the file.
    DateTimeCR GetUploadedDate() const {return m_uploadedDate;} //!< Date when the file was uploaded.
    InitializationState GetInitialized() const {return m_initialized;} //!< State of file initialization.
};

//=======================================================================================
//! File access key info for Azure blobs.
//@bsiclass                                   Algirdas.Mikoliunas             02/2017
//! @private
//=======================================================================================
struct FileAccessKey : RefCountedBase
{
private:
    bool m_valueSet = false;
    Utf8String  m_downloadUrl;
    Utf8String  m_uploadUrl;
    DateTime    m_createDate;

    friend struct iModelConnection;
    friend Client;

    static Utf8String GetProperty(RapidJsonValueCR properties, Utf8StringCR member);

    static FileAccessKeyPtr Parse(RapidJsonValueCR properties);
    static FileAccessKeyPtr ParseFromRelated(JsonValueCR json);
    static FileAccessKeyPtr ParseFromRelated(WebServices::WSObjectsReader::Instance instance);

public:
    //! Select download access key
    static void AddDownloadAccessKeySelect(Utf8StringR);
    //! Select upload access key
    static void AddUploadAccessKeySelect(Utf8StringR);
    //! Url for upload
    Utf8StringCR GetUploadUrl() const {return m_uploadUrl;}
    //! Url for download
    Utf8StringCR GetDownloadUrl() const {return m_downloadUrl;}
    bool GetValueSet() const {return m_valueSet;}
};

struct NotUsedFileAccessKey : FileAccessKey
    {};
END_BENTLEY_IMODELHUB_NAMESPACE
