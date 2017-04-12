/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/ChangeSetInfo.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnDbServer/Client/FileInfo.h>
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerResult.h>
#include <WebServices/Client/Response/WSObjectsReader.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES
typedef RefCountedPtr<struct DgnDbServerRevisionInfo> DgnDbServerRevisionInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbServerRevisionInfo);
DEFINE_TASK_TYPEDEFS(DgnDbServerRevisionInfoPtr, DgnDbServerRevisionInfo);
DEFINE_TASK_TYPEDEFS(bvector<DgnDbServerRevisionInfoPtr>, DgnDbServerRevisionsInfo);
typedef bvector<Dgn::DgnRevisionPtr> DgnRevisions;
DEFINE_TASK_TYPEDEFS(DgnRevisions, DgnRevisions);
DEFINE_TASK_TYPEDEFS(Dgn::DgnRevisionPtr, DgnRevision);

//=======================================================================================
//! Information about revision.
//@bsiclass                                      Algirdas.Mikoliunas            01/2017
//=======================================================================================
struct DgnDbServerRevisionInfo : RefCountedBase
{
public:
    enum ContainingChanges
        {
        Regular = 0,
        Schema = 1 // Revision contains minor schema changes
        };

private:
    Utf8String m_id;
    Utf8String m_parentRevisionId;
    Utf8String m_dbGuid;
    uint64_t   m_index;
    Utf8String m_description;
    uint64_t   m_fileSize;
    Utf8String m_userCreated;
    DateTime   m_pushDate;
    BeSQLite::BeBriefcaseId m_briefcaseId;
    ContainingChanges       m_containingChanges;

    friend struct DgnDbRepositoryConnection;
    friend struct DgnDbServerPreDownloadManager;
    DgnDbServerFileAccessKeyPtr m_fileAccessKey;
    bool                        m_containsFileAccessKey;

    bool GetContainsFileAccessKey() const {return m_containsFileAccessKey;}
    DgnDbServerFileAccessKeyPtr GetFileAccessKey() const {return m_fileAccessKey;}
    void SetFileAccessKey(DgnDbServerFileAccessKeyPtr fileAccessKey) {m_fileAccessKey = fileAccessKey; m_containsFileAccessKey = true;}

public:
    DgnDbServerRevisionInfo(Utf8String id, Utf8String parentRevisionId, Utf8String dbGuid, int64_t index,
        Utf8String description, int64_t fileSize, BeSQLite::BeBriefcaseId briefcaseId, Utf8String userCreated, DateTime pushDate, ContainingChanges containingChanges) 
        : m_id(id), m_parentRevisionId(parentRevisionId), m_dbGuid(dbGuid), m_index(index), m_description(description), m_fileSize(fileSize), 
        m_briefcaseId(briefcaseId), m_userCreated(userCreated), m_pushDate(pushDate), m_containingChanges(containingChanges) {}

    bool operator==(DgnDbServerRevisionInfoCR revision) const {return revision.GetId() == GetId();}
    static DgnDbServerRevisionInfoPtr Parse(WSObjectsReader::Instance instance);
    //! DEPRECATED: Use Parse from Instance
    static DgnDbServerRevisionInfoPtr Parse(JsonValueCR json);

    Utf8String GetId() const {return m_id;}
    Utf8String GetParentRevisionId() const {return m_parentRevisionId;}
    Utf8String GetDbGuid() const {return m_dbGuid;}
    uint64_t   GetIndex() const {return m_index;}
    Utf8String GetDescription() const {return m_description;}
    uint64_t   GetFileSize() const {return m_fileSize;}
    Utf8String GetUserCreated() const {return m_userCreated;}
    DateTime   GetPushDate() const {return m_pushDate;}
    ContainingChanges GetContainingChanges() const {return m_containingChanges;}
    BeSQLite::BeBriefcaseId GetBriefcaseId() const {return m_briefcaseId;}
};
END_BENTLEY_DGNDBSERVER_NAMESPACE
