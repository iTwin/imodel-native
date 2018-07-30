/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/RevisionManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include "TxnManager.h"
#include <BeSQLite/BeLzma.h>
#include <BeSQLite/RevisionChangesFile.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! A data holder representing a revision of a DgnDb
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct DgnRevision : RefCountedBase
{
private:
    Utf8String m_id;
    Utf8String m_parentId;
    Utf8String m_initialParentId;
    Utf8String m_dbGuid;

    bool m_ownsRevChangesFile;
    BeFileName m_revChangesFile;

    Utf8String m_userName;
    DateTime m_dateTime;
    Utf8String m_summary;

    DgnCodeSet  m_assignedCodes;
    DgnCodeSet  m_discardedCodes;
    DgnLockSet  m_usedLocks;

    static BeFileName BuildRevisionChangesPathname(Utf8String revisionId);

protected:    
    //! Constructor
    DgnRevision(Utf8StringCR revisionId, Utf8StringCR parentRevisionId, Utf8StringCR dbGuid) : m_id(revisionId), m_parentId(parentRevisionId), m_dbGuid(dbGuid), m_ownsRevChangesFile(false) {}

    //! Destructor
    DGNPLATFORM_EXPORT ~DgnRevision();

public:
    //! Create a new DgnRevision object
    //! @param[out] status Optional error return status (pass null if status is not needed)
    //! @param[in] revisionId Revision Id of this revision.
    //! @param[in] parentRevisionId Revision Id of the parent revision (pass empty string if it's the first revision)
    //! @param[in] dbGuid GUID of the Db that the revision belongs to (used for validation)
    DGNPLATFORM_EXPORT static DgnRevisionPtr Create(RevisionStatus* status, Utf8StringCR revisionId, Utf8StringCR parentRevisionId, Utf8StringCR dbGuid);

    //! Get the Revision Id
    //! @remarks The Revision Id is a 40 character HEX string that is initialized based on the parentId and contents of the revision
    //! and the "change" contents of the revision. 
    Utf8StringCR GetId() const { return m_id; }

    //! Get the parent's Revision Id
    Utf8StringCR GetParentId() const { return m_parentId; }

    //! Get/Set the initial parent Revision Id (before any merges)
    //! @remarks This is only used for informational purposes to review change history. 
    Utf8StringCR GetInitialParentId() const { return m_initialParentId; }
    void SetInitialParentId(Utf8StringCR initialParentRevisionId) { m_initialParentId = initialParentRevisionId; }

    //! Get the GUID of the DB that the revision belongs to
    Utf8StringCR GetDbGuid() const { return m_dbGuid; }

    //! Get or set the name of the file that should contain the change contents of this revision
    //! @remarks A default path is setup based on the id of the revision
    BeFileNameCR GetRevisionChangesFile() const { return m_revChangesFile; }
    void SetRevisionChangesFile(BeFileNameCR revChangesFile) { m_revChangesFile = revChangesFile; m_ownsRevChangesFile = false; }

    //! Get or set the user name
    Utf8StringCR GetUserName() const { return m_userName; }
    void SetUserName(Utf8CP userName) { m_userName = userName; }

    //! Get or set the time the revision was created (in UTC)
    DateTime GetDateTime() const { return m_dateTime; }
    void SetDateTime(DateTimeCR dateTime) { m_dateTime = dateTime; }

    //! Get or set the summary description for the revision
    Utf8StringCR GetSummary() const { return m_summary; }
    void SetSummary(Utf8CP summary) { m_summary = summary; }

    //! Extract the set of locks which are required for this revision's changes
    DGNPLATFORM_EXPORT void ExtractLocks(DgnLockSet& usedLocks, DgnDbCR dgndb, bool extractInserted = true, bool avoidExclusiveModelElements = true) const;
    
    //! Extract the set of codes which were assigned to objects within this revision's changes
    DGNPLATFORM_EXPORT void ExtractCodes(DgnCodeSet& assignedCodes, DgnCodeSet& discardedCodes, DgnDbCR dgndb) const;

    //! Determines if the revision contains schema changes
    DGNPLATFORM_EXPORT bool ContainsSchemaChanges(DgnDbCR dgndb) const;

    //! Validate the contents of the revision
    //! @remarks Validates the contents of the ChangeStreamFile against the revision Id.
    DGNPLATFORM_EXPORT RevisionStatus Validate(DgnDbCR dgndb) const;

    //! Dump to stdout for debugging purposes.
    DGNPLATFORM_EXPORT void Dump(DgnDbCR dgndb) const;
};

//=======================================================================================
//! Streams the contents of a file containing serialized change streams
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RevisionChangesFileReader : BeSQLite::RevisionChangesFileReaderBase
{
private:
    DGNPLATFORM_EXPORT BeSQLite::ChangeSet::ConflictResolution _OnConflict(BeSQLite::ChangeSet::ConflictCause, BeSQLite::Changes::Change iter) override;

public:
    RevisionChangesFileReader(BeFileNameCR pathname, DgnDbCR dgndb) : BeSQLite::RevisionChangesFileReaderBase({pathname}, dgndb) {}
};

//=======================================================================================
//! ChangeSet used to apply revision changes to the local Db (sets up conflict handlers appropriately)
// @bsiclass                                                 Ramanujam.Raman   02/17
//=======================================================================================
struct ApplyRevisionChangeSet : BeSQLite::ChangeSet
{
private:
    DgnDbCR m_dgndb;
public:
    ApplyRevisionChangeSet(DgnDbCR dgndb) : m_dgndb(dgndb) {}
    BeSQLite::ChangeSet::ConflictResolution _OnConflict(BeSQLite::ChangeSet::ConflictCause clause, BeSQLite::Changes::Change iter) override;
};

//=======================================================================================
//! Utility to download and upload revisions of changes to/from the DgnDb. 
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct RevisionManager : NonCopyableClass
{
friend struct TxnManager;
friend struct RevisionChangesFileReader;
friend struct ApplyRevisionChangeSet;
friend struct DgnDb;
friend struct DgnDomains;

private:
    DgnDbR m_dgndb;

    BeFileName m_tempRevisionPathname;
    DgnRevisionPtr m_currentRevision;

    static BeFileName BuildTempRevisionPathname();
    RevisionStatus SaveParentRevisionId(Utf8StringCR revisionId);
    RevisionStatus SaveReversedRevisionId(Utf8StringCR revisionId);
    RevisionStatus DeleteReversedRevisionId();

    Utf8String QueryInitialParentRevisionId() const;
    RevisionStatus SaveInitialParentRevisionId(Utf8StringCR revisionId);
    RevisionStatus UpdateInitialParentRevisionId();

    RevisionStatus GroupChanges(BeSQLite::DbSchemaChangeSetR schemaChangeSet, BeSQLite::ChangeGroupR dataChangeGroup, TxnManager::TxnId endTxnId) const;
    DgnRevisionPtr CreateRevisionObject(RevisionStatus* outStatus, BeFileNameCR tempRevisionPathname);
    RevisionStatus WriteChangesToFile(BeFileNameCR pathname, BeSQLite::DbSchemaChangeSetCR schemaChangeSet, BeSQLite::ChangeGroupCR dataChangeGroup, BeSQLite::Rebaser*);

    // If valid, currently creating a revision with all transactions upto *but* excluding this id. Rebase up to and *including* this rebaseId.
    RevisionStatus SaveCurrentRevisionEndTxnId(TxnManager::TxnId txnId, int64_t rebaseId);
    RevisionStatus DeleteCurrentRevisionEndTxnId();

    DgnRevisionPtr CreateRevision(RevisionStatus* outStatus, TxnManager::TxnId endTxnId, int64_t lastRebaseId);
    RevisionStatus DoMergeRevision(DgnRevisionCR revision);
    RevisionStatus DoReverseRevision(DgnRevisionCR revision);
    RevisionStatus DoReinstateRevision(DgnRevisionCR revision);
    RevisionStatus DoProcessRevisions(bvector<DgnRevisionCP> const& revisions, RevisionProcessOption processOption);

    BeSQLite::DbResult SaveContainsSchemaChanges();
    BeSQLite::DbResult ClearContainsSchemaChanges();
    bool QueryContainsSchemaChanges() const;

    static BeSQLite::ChangeSet::ConflictResolution ConflictHandler(DgnDbCR dgndb, BeSQLite::ChangeSet::ConflictCause clause, BeSQLite::Changes::Change iter);
public:
    //! Constructor
    RevisionManager(DgnDbR dgndb);

    //! Destructor
    ~RevisionManager() {}

    //! Get the DgnDb for this RevisionManager
    DgnDbR GetDgnDb() { return m_dgndb; }

    //! Merge a single revision to the Db
    //! @param[in] revision The revision to be merged
    //! @return RevisionStatus::Success if the revision was successfully merged, error status otherwise. 
    DGNPLATFORM_EXPORT RevisionStatus MergeRevision(DgnRevisionCR revision);

    //! Get the Id of the last revision that was merged into or created from the BIM. This is the parent for any new revisions that will be created from the BIM.
	//! @remarks Returns an empty string if the BIM is in it's initial state (with no revisions), or if it's a standalone briefcase disconnected from the Hub.
    DGNPLATFORM_EXPORT Utf8String GetParentRevisionId() const;

    //! Start creating a new revision from the changes saved to the Db
    //! @return Newly created revision. Null if there was an error, or if 
    //! there aren't any changes to create a revision. 
    //! @remarks 
    //! <ul>
    //! <li> The revision is initialized with an id, a parent id and a local file
    //! containing all the changes made since the previous revision. 
    //! <li> The revision must be finished or aborted with calls to FinishCreateRevision()
    //! or AbortCreateRevision()
    //! <li> Unless AbandonCreateRevision is subsequently called, transactions cannot be
    //! undone anymore. 
    //! </ul>
    //! @param[out] status Optional (can pass null). Set to RevisionStatus::Success if the revision was successfully 
    //! finished or some error status otherwise.
    //! @see FinishCreateRevision, AbandonCreateRevision
    DGNPLATFORM_EXPORT DgnRevisionPtr StartCreateRevision(RevisionStatus* status = nullptr);
    
    //! Return true if in the process of creating a revision
    DGNPLATFORM_EXPORT bool IsCreatingRevision() const;
        
    //! Returns the revision currently being created
    //! @remarks Is valid only if in the process of creating a revision
    DGNPLATFORM_EXPORT DgnRevisionPtr GetCreatingRevision();

    //! Finish creating a new revision
    //! @return RevisionStatus::Success if the revision was successfully finished or some error status otherwise.
    //! @remarks Upon successful return, the transaction table is flushed and cannot be undone. 
    //! @see StartCreateRevision
    DGNPLATFORM_EXPORT RevisionStatus FinishCreateRevision();

    //! Abandon creating a new revision
    //! @see StartCreateRevision
    DGNPLATFORM_EXPORT void AbandonCreateRevision();

    //! Reverses a previously merged revision
    //! @param[in] revision The revision to be reversed. Must match the parent revision of the Db. i.e., the revisions
    //! must be reversed in the opposite order of how were merged. 
    //! @return RevisionStatus::Success if the revision was successfully reversed or some error status otherwise.
    //! @remarks After reversals no changes can be committed to the DgnDb unless all the reversed revisions are reinstated
    //! again. @see ReinstateRevision()
    DGNPLATFORM_EXPORT RevisionStatus ReverseRevision(DgnRevisionCR revision);

    //! Reinstates a previously reversed revision
    //! @param[in] revision The revision to be reinstated. The parent of the revision must match the parent revision of the 
    //! Db. i.e., the revisions must be reinstated in the opposite order of how they were reversed. 
    //! @return RevisionStatus::Success if the revision was successfully reinstated or some error status otherwise.
    //! @remarks After reinstating all the revisions, the user can make changes to the DgnDb and create new revisions
    //! again. @see ReverseRevision()
    DGNPLATFORM_EXPORT RevisionStatus ReinstateRevision(DgnRevisionCR revision);

    //! Processes (merges, reverses or reinstates) a set of revisions
    //! @param[in] revisions Revisions to be processed
    //! @param[in] processOption Option to process, i.e., merge, reverse or reinstate the revisions
    //! @return RevisionStatus::Success if the revision was successfully processed or some error status otherwise.
    DGNPLATFORM_EXPORT RevisionStatus ProcessRevisions(bvector<DgnRevisionCP> const& revisions, RevisionProcessOption processOption);

    //! Checks if the Db has reversed revisions
    //! @see GetReversedRevisionId()
    DGNPLATFORM_EXPORT bool HasReversedRevisions() const;

    //! Get the last revision that the Db was reversed to
    //! @remarks Returns an empty string if there aren't any reversed revisions. 
    //! @see HasReversedRevisions()
    DGNPLATFORM_EXPORT Utf8String GetReversedRevisionId() const;

    TxnManager::TxnId QueryCurrentRevisionEndTxnId() const; //!< @private
    int64_t QueryLastRebaseId() const; //!< @private
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
