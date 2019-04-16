/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Client/BaseFileInfo.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
struct FileInfo;
typedef RefCountedPtr<struct FileInfo> FileInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(FileInfo);

enum class InitializationState
    {
    Success,
    NotStarted,
    Scheduled,
    UnknownFailure,
    OutdatedFile,
    CodeTooLong,
    SeedFileIsBriefcase
    };

//=======================================================================================
//! Information about changeSet file that is on server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct FileInfo : BaseFileInfo
{
friend struct iModelConnection;
friend struct Client;

private:
    int32_t     m_index;
    Utf8String  m_mergedChangeSetId;
    Utf8String  m_userUploaded;
    DateTime    m_uploadedDate;
    InitializationState m_initialized;

    FileInfo() : m_index(-1) {}
    FileInfo(BeSQLite::BeGuid fileId) : BaseFileInfo(fileId), m_index(-1) {}
    IMODELHUBCLIENT_EXPORT FileInfo(Dgn::DgnDbCR db, Utf8StringCR description);

    static FileInfoPtr Create(BeSQLite::BeGuid fileId) { return FileInfoPtr(new FileInfo(fileId)); }
    static FileInfoPtr Parse(RapidJsonValueCR properties, FileInfoCR fileInfo = FileInfo());
    static FileInfoPtr Parse(WebServices::WSObjectsReader::Instance instance, FileInfoCR fileInfo = FileInfo());
    void ToPropertiesJson(JsonValueR json) const;
    WebServices::ObjectId GetObjectId() const;
public:
    static FileInfoPtr Create(Dgn::DgnDbCR db, Utf8StringCR description) {return FileInfoPtr(new FileInfo(db, description));}
    int32_t GetIndex() const {return m_index;} //!< Index of the file.
    Utf8StringCR GetMergedChangeSetId() const {return m_mergedChangeSetId;} //!< Get Last ChangeSet Id merged to the iModel.
    Utf8StringCR GetUserUploaded() const {return m_userUploaded;} //!< Name of the user that uploaded the file.
    DateTimeCR GetUploadedDate() const {return m_uploadedDate;} //!< Date when the file was uploaded.
    InitializationState GetInitialized() const {return m_initialized;} //!< State of file initialization.
};
END_BENTLEY_IMODELHUB_NAMESPACE
