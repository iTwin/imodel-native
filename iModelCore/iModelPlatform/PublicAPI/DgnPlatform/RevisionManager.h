/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DgnDb.h"
#include "TxnManager.h"
#include <BeSQLite/BeLzma.h>
#include <BeSQLite/ChangesetFile.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChangesetProps : RefCountedBase {
    TxnManager::TxnId m_endTxnId;
    int64_t m_lastRebaseId = 0;
    Utf8String m_id;
    int32_t m_index;
    Utf8String m_parentId;
    Utf8String m_dbGuid;
    BeFileName m_fileName;
    Utf8String m_userName;
    DateTime m_dateTime;
    Utf8String m_summary;

    ChangesetProps(Utf8StringCR changesetId, int32_t changesetIndex, Utf8StringCR parentRevisionId, Utf8StringCR dbGuid, BeFileNameCR fileName) :
        m_id(changesetId), m_index(changesetIndex), m_parentId(parentRevisionId), m_dbGuid(dbGuid), m_fileName(fileName) {}

    Utf8StringCR GetChangesetId() const { return m_id; }
    int32_t GetChangesetIndex() const { return m_index; }
    void SetChangesetIndex(int32_t index) { m_index = index; }
    Utf8StringCR GetParentId() const { return m_parentId; }
    Utf8StringCR GetDbGuid() const { return m_dbGuid; }
    BeFileNameCR GetFileName() const { return m_fileName; }

    //! Get or set the user name
    Utf8StringCR GetUserName() const { return m_userName; }
    void SetUserName(Utf8CP userName) { m_userName = userName; }

    //! Get or set the time the revision was created (in UTC)
    DateTime GetDateTime() const { return m_dateTime; }
    void SetDateTime(DateTimeCR dateTime) { m_dateTime = dateTime; }

    //! Get or set the summary description for the revision
    Utf8StringCR GetSummary() const { return m_summary; }
    void SetSummary(Utf8CP summary) { m_summary = summary; }

    //! Determines if the revision contains schema changes
    DGNPLATFORM_EXPORT bool ContainsSchemaChanges(DgnDbCR dgndb) const;
    DGNPLATFORM_EXPORT void ValidateContent(DgnDbCR dgndb) const;
    DGNPLATFORM_EXPORT void Dump(DgnDbCR dgndb) const;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ChangesetFileReader : BeSQLite::ChangesetFileReaderBase {
private:
    DGNPLATFORM_EXPORT BeSQLite::ChangeSet::ConflictResolution _OnConflict(BeSQLite::ChangeSet::ConflictCause, BeSQLite::Changes::Change iter) override;

public:
    ChangesetFileReader(BeFileNameCR pathname, DgnDbCR dgndb) : BeSQLite::ChangesetFileReaderBase({pathname}, dgndb) {}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
