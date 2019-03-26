/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/ChangeSetInfo.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/iModelHub/Client/FileInfo.h>
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/Result.h>
#include <WebServices/Client/Response/WSObjectsReader.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
typedef RefCountedPtr<struct ChangeSetInfo> ChangeSetInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(ChangeSetInfo);
DEFINE_TASK_TYPEDEFS(ChangeSetInfoPtr, ChangeSetInfo);
DEFINE_TASK_TYPEDEFS(bvector<ChangeSetInfoPtr>, ChangeSetsInfo);
typedef bvector<Dgn::DgnRevisionPtr> ChangeSets;
DEFINE_TASK_TYPEDEFS(ChangeSets, ChangeSets);
DEFINE_TASK_TYPEDEFS(Dgn::DgnRevisionPtr, ChangeSet);

//=======================================================================================
//! Information about changeSet.
//@bsiclass                                      Algirdas.Mikoliunas            01/2017
//=======================================================================================
struct ChangeSetInfo : RefCountedBase
{
public:
    enum ContainingChanges
        {
        Regular = 0,
        Schema = 1 // ChangeSet contains minor schema changes
        };

private:
    friend struct iModelConnection;
    friend struct ChangeSetCacheManager;
    Utf8String m_id;
    Utf8String m_parentChangeSetId;
    Utf8String m_dbGuid;
    uint64_t   m_index;
    Utf8String m_description;
    uint64_t   m_fileSize;
    Utf8String m_userCreated;
    DateTime   m_pushDate;
    BeSQLite::BeBriefcaseId m_briefcaseId;
    ContainingChanges       m_containingChanges;
    FileAccessKeyPtr        m_fileAccessKey;
    bool                    m_containsFileAccessKey = false;

    ChangeSetInfo(Utf8String id, Utf8String parentChangeSetId, Utf8String dbGuid, int64_t index,
                  Utf8String description, int64_t fileSize, BeSQLite::BeBriefcaseId briefcaseId, Utf8String userCreated, DateTime pushDate, 
                  ContainingChanges containingChanges) : m_id(id), m_parentChangeSetId(parentChangeSetId), m_dbGuid(dbGuid), m_index(index), 
                  m_description(description), m_fileSize(fileSize), m_briefcaseId(briefcaseId), m_userCreated(userCreated), m_pushDate(pushDate), 
                  m_containingChanges(containingChanges) {}

    bool GetContainsFileAccessKey() const {return m_containsFileAccessKey;}
    FileAccessKeyPtr GetFileAccessKey() const {return m_fileAccessKey;}
    void SetFileAccessKey(FileAccessKeyPtr fileAccessKey) {m_fileAccessKey = fileAccessKey; m_containsFileAccessKey = true;}

    bool operator==(ChangeSetInfoCR changeSet) const {return changeSet.GetId() == GetId();}
    static ChangeSetInfoPtr ParseRapidJson(RapidJsonValueCR properties);
    static ChangeSetInfoPtr Parse(WebServices::WSObjectsReader::Instance instance);
public:
    Utf8String GetId() const {return m_id;}
    Utf8String GetParentChangeSetId() const {return m_parentChangeSetId;}
    Utf8String GetDbGuid() const {return m_dbGuid;}
    uint64_t   GetIndex() const {return m_index;}
    Utf8String GetDescription() const {return m_description;}
    uint64_t   GetFileSize() const {return m_fileSize;}
    Utf8String GetUserCreated() const {return m_userCreated;}
    DateTime   GetPushDate() const {return m_pushDate;}
    ContainingChanges GetContainingChanges() const {return m_containingChanges;}
    BeSQLite::BeBriefcaseId GetBriefcaseId() const {return m_briefcaseId;}
};
END_BENTLEY_IMODELHUB_NAMESPACE
