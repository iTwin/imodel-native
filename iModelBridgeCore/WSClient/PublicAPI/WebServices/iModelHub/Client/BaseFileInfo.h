/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Client/FileAccessKey.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

//=======================================================================================
//! Base information about file that is on server.
//@bsiclass                                      Andrius.Zonys                12/2017
//=======================================================================================
struct BaseFileInfo : RefCountedBase
{
private:
    BeSQLite::BeGuid    m_fileId;
    Utf8String          m_fileName;
    Utf8String          m_description;
    uint64_t            m_fileSize;
    bool                m_areFileDetailsAvailable;
    FileAccessKeyPtr    m_fileAccessKey;
    bool                m_containsFileAccessKey = false;

    friend struct iModelConnection;
    friend struct Client;
    friend struct BriefcaseInfo;
    friend struct FileInfo;

    BaseFileInfo() : m_fileSize(0), m_areFileDetailsAvailable(false) {}
    BaseFileInfo(BeSQLite::BeGuid fileId) : m_fileId(fileId), m_fileSize(0), m_areFileDetailsAvailable(false) {}
    BaseFileInfo(Dgn::DgnDbCR db, Utf8StringCR description);
    BaseFileInfo(RapidJsonValueCR properties); 

    bool GetContainsFileAccessKey() const {return m_containsFileAccessKey;}
    FileAccessKeyPtr GetFileAccessKey() const {return m_fileAccessKey;}
    void SetFileAccessKey(FileAccessKeyPtr fileAccessKey) {m_containsFileAccessKey = true; m_fileAccessKey = fileAccessKey;}

    static Utf8String GetProperty(RapidJsonValueCR properties, Utf8StringCR member);
    void UpdateBaseFileInfo(RapidJsonValueCR properties);
    bool AreFileDetailsAvailable() const { return m_areFileDetailsAvailable; }
public:
    //!< Db Guid of the file.
    BeSQLite::BeGuid GetFileId() const {return m_fileId;}

    //!< Name of the file.
    Utf8StringCR GetFileName() const {return m_fileName;}

     //!< Description of the file.
    Utf8StringCR GetDescription() const {return m_description;}

    //!< Size of the file.
    uint64_t GetSize() const {return m_fileSize;}
};
END_BENTLEY_IMODELHUB_NAMESPACE
