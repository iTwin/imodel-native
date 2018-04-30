/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/VersionInfo.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/Result.h>
#include <WebServices/Client/Response/WSObjectsReader.h>
#include <WebServices/iModelHub/Client/ThumbnailsManager.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
typedef RefCountedPtr<struct VersionInfo> VersionInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(VersionInfo);
DEFINE_TASK_TYPEDEFS(VersionInfoPtr, VersionInfo);
DEFINE_TASK_TYPEDEFS(bvector<VersionInfoPtr>, VersionsInfo);


struct VersionInfo : RefCountedBase
{
private:
    friend struct iModelConnection;
    friend struct VersionsManager;
    Utf8String m_id;
    Utf8String m_name;
    Utf8String m_description;
    Utf8String m_changeSetId;
    Utf8String m_userCreated;
    DateTime   m_createdDate;
    Utf8String m_smallThumbnailId;
    Utf8String m_largeThumbnailId;

    VersionInfo() {};
    VersionInfo(Utf8String id, Utf8String name, Utf8String description, Utf8String changeSetId, Utf8String userCreated, DateTime createdDate, 
                Utf8String smallThumbnailId, Utf8String largeThumbnailId)
        : m_id(id), m_name(name), m_description(description), m_changeSetId(changeSetId), m_userCreated(userCreated), m_createdDate(createdDate),
          m_smallThumbnailId(smallThumbnailId), m_largeThumbnailId(largeThumbnailId) {}

    Json::Value GenerateJson() const;
    static VersionInfoPtr ParseRapidJson(RapidJsonValueCR properties, Utf8String smallThumbnailId, Utf8String largeThumbnailId);
    static VersionInfoPtr Parse(WebServices::WSObjectsReader::Instance instance);

public:
    VersionInfo(Utf8String name, Utf8String description, Utf8String changeSetId) : 
        m_name(name), m_description(description), m_changeSetId(changeSetId) {}

    Utf8String GetId() const {return m_id;}
    Utf8String GetName() const {return m_name;}
    Utf8String GetDescription() const {return m_description;}
    Utf8String GetChangeSetId() const {return m_changeSetId;}
    Utf8String GetUserCreated() const {return m_userCreated;}
    DateTime   GetCreatedDate() const {return m_createdDate;}
    Utf8String GetSmallThumbnailId() const { return m_smallThumbnailId; }
    Utf8String GetLargeThumbnailId() const { return m_largeThumbnailId; }

    void SetName(Utf8String name) {m_name = name;}
    void SetDescription(Utf8String description) {m_description = description;}
};
END_BENTLEY_IMODELHUB_NAMESPACE