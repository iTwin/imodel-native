/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/FileAccessKey.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/Result.h>
#include <DgnPlatform/DgnDb.h>
#include <Bentley/DateTime.h>
#include <WebServices/Client/Response/WSObjectsReader.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
typedef RefCountedPtr<struct FileAccessKey> FileAccessKeyPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(FileAccessKey);
DEFINE_TASK_TYPEDEFS(FileAccessKeyPtr, FileAccessKey);

//=======================================================================================
//! File access key info for Azure blobs.
//@bsiclass                                   Algirdas.Mikoliunas             02/2017
//! @private
//=======================================================================================
struct FileAccessKey : RefCountedBase
{
private:
    Utf8String  m_downloadUrl;
    Utf8String  m_uploadUrl;
    DateTime    m_createDate;

    friend struct iModelConnection;
    friend struct Client;
    friend struct BriefcaseInfo;

    static Utf8String GetProperty(RapidJsonValueCR properties, Utf8StringCR member);

    static FileAccessKeyPtr Parse(RapidJsonValueCR properties);
    static FileAccessKeyPtr ParseFromRelated(JsonValueCR json);
    static FileAccessKeyPtr ParseFromRelated(WebServices::WSObjectsReader::Instance instance);

public:
    //! Create FileAccessKey instance
    FileAccessKey() : m_downloadUrl(""), m_uploadUrl("") {}
    //! Select download access key
    static void AddDownloadAccessKeySelect(Utf8StringR);
    //! Select upload access key
    static void AddUploadAccessKeySelect(Utf8StringR);
    //! Url for upload
    Utf8StringCR GetUploadUrl() const {return m_uploadUrl;}
    //! Url for download
    Utf8StringCR GetDownloadUrl() const {return m_downloadUrl;}
};
END_BENTLEY_IMODELHUB_NAMESPACE
