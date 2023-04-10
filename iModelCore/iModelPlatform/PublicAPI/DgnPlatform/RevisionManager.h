/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DgnDb.h"
#include "TxnManager.h"
#include <BeSQLite/BeLzma.h>
#include <BeSQLite/RevisionChangesFile.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChangesetInfo : RefCountedBase {
private:
    Utf8String m_id;
    int32_t m_index;
    Utf8String m_parentId;
    Utf8String m_dbGuid;
    bool m_ownsRevChangesFile;
    BeFileName m_revChangesFile;
    Utf8String m_userName;
    DateTime m_dateTime;
    Utf8String m_summary;

    static BeFileName BuildRevisionChangesPathname(Utf8String revisionId);

protected:
    ChangesetInfo(Utf8StringCR changesetId, int32_t changesetIndex, Utf8StringCR parentRevisionId, Utf8StringCR dbGuid) :
        m_id(changesetId), m_index(changesetIndex), m_parentId(parentRevisionId), m_dbGuid(dbGuid), m_ownsRevChangesFile(false) {}

    DGNPLATFORM_EXPORT ~ChangesetInfo();

public:
    //! Create a new DgnRevision object
    DGNPLATFORM_EXPORT static ChangesetInfoPtr Create(Utf8StringCR changesetId, int32_t changesetIndex, Utf8StringCR parentRevisionId, Utf8StringCR dbGuid, BeFileNameCP filename = nullptr);

    Utf8StringCR GetChangesetId() const { return m_id; }
    int32_t GetChangesetIndex() const { return m_index; }
    void SetChangesetIndex(int32_t index) { m_index = index; }

    //! Get the parent's Revision Id
    Utf8StringCR GetParentId() const { return m_parentId; }

    //! Get the GUID of the DB that the revision belongs to
    Utf8StringCR GetDbGuid() const { return m_dbGuid; }

    //! Get or set the name of the file that should contain the change contents of this revision
    //! @remarks A default path is setup based on the id of the revision
    BeFileNameCR GetRevisionChangesFile() const { return m_revChangesFile; }
    void SetRevisionChangesFile(BeFileNameCR revChangesFile) {
        m_revChangesFile = revChangesFile;
        m_ownsRevChangesFile = false;
    }

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

    //! Validate the contents of the revision
    //! @remarks Validates the contents of the ChangeStreamFile against the revision Id.
    DGNPLATFORM_EXPORT ChangesetStatus Validate(DgnDbCR dgndb) const;

    //! Dump to stdout for debugging purposes.
    DGNPLATFORM_EXPORT void Dump(DgnDbCR dgndb) const;
};

//=======================================================================================
//! Streams the contents of a file containing serialized change streams
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ChangesetFileReader : BeSQLite::ChangesetFileReaderBase {
private:
    DGNPLATFORM_EXPORT BeSQLite::ChangeSet::ConflictResolution _OnConflict(BeSQLite::ChangeSet::ConflictCause, BeSQLite::Changes::Change iter) override;

public:
    ChangesetFileReader(BeFileNameCR pathname, DgnDbCR dgndb) : BeSQLite::ChangesetFileReaderBase({pathname}, dgndb) {}
};

//=======================================================================================
//! ChangeSet used to apply revision changes to the local Db (sets up conflict handlers appropriately)
// @bsiclass
//=======================================================================================
struct ApplyRevisionChangeSet : BeSQLite::ChangeSet {
private:
    DgnDbCR m_dgndb;

public:
    ApplyRevisionChangeSet(DgnDbCR dgndb) : m_dgndb(dgndb) {}
    BeSQLite::ChangeSet::ConflictResolution _OnConflict(BeSQLite::ChangeSet::ConflictCause clause, BeSQLite::Changes::Change iter) override;
};

//=======================================================================================
//! Utility to download and upload revisions of changes to/from the DgnDb.
// @bsiclass
//=======================================================================================
struct RevisionManager : NonCopyableClass {
    friend struct TxnManager;
    friend struct ChangesetFileReader;
    friend struct ApplyRevisionChangeSet;
    friend struct DgnDb;
    friend struct DgnDomains;

private:
    DgnDbR m_dgndb;

    BeFileName m_tempRevisionPathname;
    ChangesetInfoPtr m_currentRevision;

    static BeFileName BuildTempRevisionPathname();
    ChangesetStatus SaveParentRevision(Utf8StringCR revisionId, int32_t changesetIndex);

    ChangesetStatus GroupChanges(BeSQLite::DdlChangesR, BeSQLite::ChangeGroupR, TxnManager::TxnId endTxnId) const;
    ChangesetInfoPtr CreateRevisionObject(BeFileNameCR tempRevisionPathname);
    ChangesetStatus WriteChangesToFile(BeFileNameCR pathname, BeSQLite::DdlChangesCR ddlChanges, BeSQLite::ChangeGroupCR dataChangeGroup, BeSQLite::Rebaser*);

    // If valid, currently creating a revision with all transactions up to *but* excluding this id. Rebase up to and *including* this rebaseId.
    ChangesetStatus SaveCurrentRevisionEndTxnId(TxnManager::TxnId txnId, int64_t rebaseId);
    ChangesetStatus DeleteCurrentRevisionEndTxnId();

    ChangesetInfoPtr CreateRevision(ChangesetStatus* outStatus, TxnManager::TxnId endTxnId, int64_t lastRebaseId);

    static BeSQLite::ChangeSet::ConflictResolution ConflictHandler(DgnDbCR dgndb, BeSQLite::ChangeSet::ConflictCause clause, BeSQLite::Changes::Change iter);

public:
    //! Constructor
    RevisionManager(DgnDbR dgndb);

    //! Destructor
    ~RevisionManager() {}

    //! Get the DgnDb for this RevisionManager
    DgnDbR GetDgnDb() { return m_dgndb; }

    void ClearSavedValues();

    //! Merge a single revision to the Db
    //! @param[in] revision The revision to be merged
    //! @return ChangesetStatus::Success if the revision was successfully merged, error status otherwise.
    DGNPLATFORM_EXPORT ChangesetStatus MergeRevision(ChangesetInfoCR revision);

    //! Get the Id of the last revision that was merged into or created. This is the parent for any new revisions that will be created from the briefcase.
    //! @remarks Returns an empty string if the BIM is in it's initial state (with no revisions), or if it's a standalone briefcase disconnected from the Hub.
    DGNPLATFORM_EXPORT Utf8String GetParentRevisionId() const;
    DGNPLATFORM_EXPORT void GetParentRevision(int32_t& index, Utf8StringR id) const;

    //! Start creating a new revision from the changes saved to the Db
    //! @return Newly created revision. Null if there was an error, or if
    //! there aren't any changes to create a revision.
    //! @remarks
    //! <ul>
    //! <li> The revision is initialized with an id, a parent id and a local file
    //! containing all the changes made since the previous revision.
    //! <li> The revision must be finished or aborted with calls to FinishCreateRevision()
    //! or AbortCreateRevision()
    //! <li> Unless AbandonCreateChangeset is subsequently called, transactions cannot be
    //! undone anymore.
    //! </ul>
    //! @param[out] status Optional (can pass null). Set to ChangesetStatus::Success if the revision was successfully
    //! finished or some error status otherwise.
    //! @see FinishCreateRevision, AbandonCreateChangeset
    DGNPLATFORM_EXPORT ChangesetInfoPtr StartCreateChangeset(ChangesetStatus* status = nullptr);

    //! Return true if in the process of creating a revision
    DGNPLATFORM_EXPORT bool IsCreatingRevision() const;

    //! Returns the revision currently being created
    //! @remarks Is valid only if in the process of creating a revision
    DGNPLATFORM_EXPORT ChangesetInfoPtr GetCreatingRevision();

    //! Finish creating a new revision
    //! @return ChangesetStatus::Success if the revision was successfully finished or some error status otherwise.
    //! @remarks Upon successful return, the transaction table is flushed and cannot be undone.
    //! @see StartCreateChangeset
    DGNPLATFORM_EXPORT ChangesetStatus FinishCreateRevision(int32_t changesetIndex);

    //! Abandon creating a new revision
    //! @see StartCreateChangeset
    DGNPLATFORM_EXPORT void AbandonCreateChangeset();

    //! Reverses a previously merged revision
    //! @param[in] revision The revision to be reversed. Must match the parent revision of the Db. i.e., the revisions
    //! must be reversed in the opposite order of how were merged.
    //! @return ChangesetStatus::Success if the revision was successfully reversed or some error status otherwise.
    //! @remarks After reversals no changes can be committed to the DgnDb unless all the reversed revisions are reinstated
    //! again. @see ReinstateRevision()
    DGNPLATFORM_EXPORT ChangesetStatus ReverseRevision(ChangesetInfoCR revision);

    //! Processes (merges, reverses or reinstates) a set of revisions
    //! @param[in] revisions Revisions to be processed
    //! @param[in] processOption Option to process, i.e., merge, reverse or reinstate the revisions
    //! @return ChangesetStatus::Success if the revision was successfully processed or some error status otherwise.
    DGNPLATFORM_EXPORT ChangesetStatus ProcessRevisions(bvector<ChangesetInfoCP> const& revisions, RevisionProcessOption processOption);

    DGNPLATFORM_EXPORT TxnManager::TxnId QueryCurrentRevisionEndTxnId() const; //!< @private
    int64_t QueryLastRebaseId() const;                                         //!< @private
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
