/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/FileInfo.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
typedef std::shared_ptr<struct FileInfo> FileInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(FileInfo);

//=======================================================================================
//! Information about revision file that is on server.
//@bsiclass                                      Karolis.Dziedzelis             10/2015
//=======================================================================================
struct FileInfo
{
//__PUBLISH_SECTION_END__
private:
    int32_t     m_index;
    Utf8String  m_fileName;
    BeGuid      m_fileId;
    Utf8String  m_mergedRevisionId;
    Utf8String  m_description;
    Utf8String  m_fileUrl;
    uint64_t    m_fileSize;
    Utf8String  m_userUploaded;
    DateTime    m_uploadedDate;
    bool        m_areFileDetailsAvailable;
    bool        m_initialized;

    FileInfo(BeGuid fileId);
    FileInfo(Dgn::DgnDbCR db, Utf8StringCR description);
    FileInfo(int32_t index, Utf8StringCR fileName, Utf8StringCR fileId, Utf8StringCR mergedRevisionId,
             Utf8StringCR description, Utf8StringCR url, uint64_t size, Utf8StringCR user, DateTimeCR date);

    static FileInfoPtr Parse(RapidJsonValueCR properties, Utf8StringCR instanceId, FileInfoCR fileInfo = FileInfo());
//__PUBLISH_SECTION_START__
public:
    //__PUBLISH_SECTION_END__
    FileInfo();

    static FileInfoPtr Create(BeGuid fileId);
    //! DEPRECATED: Use Parsing from Instance
    static FileInfoPtr Parse(JsonValueCR json, FileInfoCR fileInfo = FileInfo());
    static FileInfoPtr Parse(WSObjectsReader::Instance instance, FileInfoCR fileInfo = FileInfo());
    void ToPropertiesJson(JsonValueR json) const;
    WebServices::ObjectId GetObjectId() const;
    bool AreFileDetailsAvailable() const;
    //__PUBLISH_SECTION_START__

    DGNDBSERVERCLIENT_EXPORT static FileInfoPtr Create(Dgn::DgnDbCR db, Utf8StringCR description);
    DGNDBSERVERCLIENT_EXPORT int32_t      GetIndex() const; //!< Index of the file.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetFileName() const; //!< Name of the file.
    DGNDBSERVERCLIENT_EXPORT BeGuid       GetFileId() const; //!< Db Guid of the file.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetMergedRevisionId() const; //!< Get Last Revision Id merged to the repository.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetDescription() const; //!< Description of the file.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetFileURL() const; //!< URL of the file.
    DGNDBSERVERCLIENT_EXPORT uint64_t     GetSize() const; //!< Size of the file.
    DGNDBSERVERCLIENT_EXPORT Utf8StringCR GetUserUploaded() const; //!< Name of the user that uploaded the file.
    DGNDBSERVERCLIENT_EXPORT DateTimeCR   GetUploadedDate() const; //!< Date when the file was uploaded.
    DGNDBSERVERCLIENT_EXPORT bool         GetInitialized() const; //!< Flag if file is initialized.
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
