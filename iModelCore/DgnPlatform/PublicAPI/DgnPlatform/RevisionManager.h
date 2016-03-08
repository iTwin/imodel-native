/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/RevisionManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include "TxnManager.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! A data holder representing a revision of a DgnDb
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct DgnRevision : RefCountedBase
{
    friend struct TxnManager;

    //! Options for additional information to include in a DgnRevision
    enum class Include
    {
        None = 0, //!< Include no additional data
        Locks = 1 << 0, //!< Include any locks required for the changes within a revision
        Codes = 1 << 1, //!< Include any DgnCodes which were assigned or discarded within a revision
        CodesAndLocks = Codes | Locks, //!< Include both locks and codes
        All = CodesAndLocks, //!< Include all additional information
    };

private:
    Utf8String m_id;
    Utf8String m_parentId;
    Utf8String m_initialParentId;
    Utf8String m_dbGuid;

    BeFileName m_changeStreamFile;

    Utf8String m_userName;
    DateTime m_dateTime;
    Utf8String m_summary;

    DgnCodeSet  m_assignedCodes;
    DgnCodeSet  m_discardedCodes;
    DgnLockSet  m_usedLocks;

    void SetChangeStreamFile(BeFileNameCR changeStreamFile) { m_changeStreamFile = changeStreamFile; }
    static BeFileName BuildChangeStreamPathname(Utf8String revisionId);

protected:    
    //! Constructor
    DgnRevision(Utf8StringCR revisionId, Utf8StringCR parentRevisionId, Utf8StringCR dbGuid) : m_id(revisionId), m_parentId(parentRevisionId), m_dbGuid(dbGuid) {}

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

    //! Get the name of the file that should contain the change contents of this revision
    BeFileNameCR GetChangeStreamFile() const { return m_changeStreamFile; }

    //! Get or set the user name
    Utf8StringCR GetUserName() const { return m_userName; }
    void SetUserName(Utf8CP userName) { m_userName = userName; }

    //! Get or set the time the revision was created (in UTC)
    DateTime GetDateTime() const { return m_dateTime; }
    void SetDateTime(DateTimeCR dateTime) { m_dateTime = dateTime; }

    //! Get or set the summary description for the revision
    Utf8StringCR GetSummary() const { return m_summary; }
    void SetSummary(Utf8CP summary) { m_summary = summary; }

    //! Validate the contents of the revision
    //! @remarks Validates the contents of the ChangeStreamFile against the revision Id.
    DGNPLATFORM_EXPORT RevisionStatus Validate(DgnDbCR dgndb) const;

    //! Dump to stdout for debugging purposes.
    DGNPLATFORM_EXPORT void Dump(DgnDbCR dgndb) const;

    //! Get the set of locks which are required for this revision's changes, if Include::Locks was specified
    DgnLockSet const& GetLocks() const { return m_usedLocks; }
    //! Get the set of codes which were assigned to objects within this revision's changes, if Include::Codes was specified
    DgnCodeSet const& GetAssignedCodes() const { return m_assignedCodes; }
    //! Get the set of codes which became discarded within this revision's changes, if Include::Codes was specified
    DgnCodeSet const& GetDiscardedCodes() const { return m_discardedCodes; }

    void IncludeChangeGroupData(BeSQLite::ChangeGroup& changeGroup, Include include, DgnDbR db); //!< @private
    void ExtractUsedLocks(DgnLockSet& locks); //!< @private
    void ExtractAssignedCodes(DgnCodeSet& codes); //!< @private
};

ENUM_IS_FLAGS(DgnRevision::Include);

//=======================================================================================
//! Streams the contents of multiple files containing serialized change streams
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ChangeStreamFileReader : BeSQLite::ChangeStream
{
private:
    DgnDbCR m_dgndb; // Used only for debugging
    bvector<BeFileName> m_pathnames;

    BeFile m_currentFile;
    int m_currentFileIndex = -1;
    uint64_t m_currentTotalBytes = 0;
    uint64_t m_currentByteIndex = 0;

    BentleyStatus CloseCurrentFile();
    BentleyStatus OpenNextFile(bool& completedAllFiles);
    BentleyStatus ReadNextPage(void *pData, int *pnData);

    bool IsCurrentFileComplete() const;

    DGNPLATFORM_EXPORT virtual BeSQLite::DbResult _InputPage(void *pData, int *pnData) override;
    DGNPLATFORM_EXPORT virtual void _Reset() override;
    DGNPLATFORM_EXPORT virtual BeSQLite::ChangeSet::ConflictResolution _OnConflict(BeSQLite::ChangeSet::ConflictCause clause, BeSQLite::Changes::Change iter);

public:
    ChangeStreamFileReader(bvector<BeFileName> pathnames, DgnDbCR dgnDb) : m_pathnames(pathnames), m_dgndb(dgnDb) {}
    ChangeStreamFileReader(BeFileNameCR pathname, DgnDbCR dgnDb) : m_dgndb(dgnDb) { m_pathnames.push_back(pathname); }
    ~ChangeStreamFileReader() {}
};

//=======================================================================================
//! Utility to download and upload revisions of changes to/from the DgnDb. 
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct RevisionManager : NonCopyableClass
{
friend struct TxnManager;

private:
    DgnDbR m_dgndb;

    DgnRevisionPtr m_currentRevision;
    TxnManager::TxnId m_currentRevisionEndTxnId; // If valid, currently creating a revision with all transactions upto *but* excluding this id

    RevisionStatus SetParentRevisionId(Utf8StringCR revisionId);

    Utf8String GetInitialParentRevisionId() const;
    RevisionStatus UpdateInitialParentRevisionId();

    RevisionStatus GroupChanges(BeSQLite::ChangeGroup& changeGroup) const;
    DgnRevisionPtr CreateRevisionObject(RevisionStatus* outStatus, BeSQLite::ChangeGroup& changeGroup, DgnRevision::Include include);
    RevisionStatus WriteChangesToFile(BeFileNameCR pathname, BeSQLite::ChangeGroup& changeGroup);

public:
    //! Constructor
    RevisionManager(DgnDbR dgndb) : m_dgndb(dgndb) {}

    //! Destructor
    DGNPLATFORM_EXPORT ~RevisionManager();

    //! Get the DgnDb for this RevisionManager
    DgnDbR GetDgnDb() { return m_dgndb; }

    //! Get the parent revision id of any changes in the DgnDb
    DGNPLATFORM_EXPORT Utf8String GetParentRevisionId() const;

    //! Merge a single revision to the Db
    //! @param[in] revision The revision to be merged
    //! @return RevisionStatus::Success if the revision was successfully merged, error status otherwise. 
    DGNPLATFORM_EXPORT RevisionStatus MergeRevision(DgnRevisionCR revision);

    //! Returns true if a revision can be created. 
    DGNPLATFORM_EXPORT bool CanCreateRevision() const;

    //! Start creating a new revision from the changes saved to the Db
    //! @return Newly created revision. Null if there was an error.
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
    DGNPLATFORM_EXPORT DgnRevisionPtr StartCreateRevision(RevisionStatus* status = nullptr, DgnRevision::Include = DgnRevision::Include::All);
    
    //! Return true if in the process of creating a revision
    bool IsCreatingRevision() const { return m_currentRevision.IsValid(); }

    //! Finish creating a new revision
    //! @return RevisionStatus::Success if the revision was successfully finished or some error status otherwise.
    //! @remarks Upon successful return, the transaction table is flushed and cannot be undone. 
    //! @see StartCreateRevision
    DGNPLATFORM_EXPORT RevisionStatus FinishCreateRevision();

    //! Abandon creating a new revision
    //! @see StartCreateRevision
    DGNPLATFORM_EXPORT void AbandonCreateRevision();

    TxnManager::TxnId GetCurrentRevisionEndTxnId() const; //!< @private
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
