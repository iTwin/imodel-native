/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/VersionInfo.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/Result.h>
#include <WebServices/Client/Response/WSObjectsReader.h>

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

        VersionInfo() {};
        VersionInfo(Utf8String id, Utf8String name, Utf8String description, Utf8String changeSetId, Utf8String userCreated, DateTime createdDate) 
            : m_id(id), m_name(name), m_description(description), m_changeSetId(changeSetId), m_userCreated(userCreated), m_createdDate(createdDate) {}

        Json::Value GenerateJson() const;
        static VersionInfoPtr ParseRapidJson(RapidJsonValueCR properties);
        static VersionInfoPtr Parse(WebServices::WSObjectsReader::Instance instance);

    public:
        VersionInfo(Utf8String name, Utf8String description, Utf8String changeSetId) : m_name(name), m_description(description), m_changeSetId(changeSetId) {}

        Utf8String GetId() const { return m_id; }
        Utf8String GetName() const { return m_name; }
        Utf8String GetDescription() const { return m_description; }
        Utf8String GetChangeSetId() const { return m_changeSetId; }
        Utf8String GetUserCreated() const { return m_userCreated; }
        DateTime   GetCreatedDate() const { return m_createdDate; }

        void SetName(Utf8String name) { m_name = name; }
        void SetDescription(Utf8String description) { m_description = description; }
    };
END_BENTLEY_IMODELHUB_NAMESPACE