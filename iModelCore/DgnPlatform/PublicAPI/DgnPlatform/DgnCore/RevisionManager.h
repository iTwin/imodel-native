/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/RevisionManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
private:
    Utf8String m_id;
    Utf8String m_parentId;
    Utf8String m_initialParentId;
    Utf8String m_dbGuid;

    BeFileName m_changeStreamFile;

    Utf8String m_userName;
    DateTime m_dateTime;
    Utf8String m_summary;

    void SetChangeStreamFile(BeFileNameCR changeStreamFile) { m_changeStreamFile = changeStreamFile; }
    static BeFileName BuildChangeStreamPathname(Utf8String revisionId);

protected:    
    //! Constructor
    DgnRevision(Utf8StringCR revisionId, Utf8StringCR parentRevisionId, Utf8StringCR dbGuid) : m_id(revisionId), m_parentId(parentRevisionId), m_dbGuid(dbGuid) {}

    //! Destructor
    DGNPLATFORM_EXPORT ~DgnRevision();

public:
    //! Create a new DgnRevision object
    //! @param[in] revisionId Revision Id of this revision.
    //! @param[in] parentRevisionId Revision Id of the parent revision (pass empty string if it's the first revision)
    //! @param[in] dbGuid GUID of the Db that the revision belongs to (used for validation)
    DGNPLATFORM_EXPORT static DgnRevisionPtr Create(Utf8StringCR revisionId, Utf8StringCR parentRevisionId, Utf8StringCR dbGuid);

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

    //! Dump to stdout for debugging purposes.
    DGNPLATFORM_EXPORT void Dump(DgnDbCR dgndb) const;
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

    BentleyStatus SetParentRevisionId(Utf8StringCR revisionId);

    Utf8String GetInitialParentRevisionId() const;
    BentleyStatus UpdateInitialParentRevisionId();

    BentleyStatus GroupChanges(BeSQLite::ChangeGroup& changeGroup) const;
    DgnRevisionPtr CreateRevision(BeSQLite::ChangeGroup const& changeGroup);
    static BentleyStatus WriteChangesToFile(BeFileNameCR pathname, BeSQLite::ChangeGroup const& changeGroup);

public:
    //! Constructor
    RevisionManager(DgnDbR dgndb) : m_dgndb(dgndb) {}

    //! Destructor
    DGNPLATFORM_EXPORT ~RevisionManager();

    //! Get the DgnDb for this RevisionManager
    DgnDbR GetDgnDb() { return m_dgndb; }

    //! Return true if in the process of creating a revision
    bool IsCreatingRevision() const { return m_currentRevision.IsValid(); }

    //! Get the parent revision id of any changes in the DgnDb
    DGNPLATFORM_EXPORT Utf8String GetParentRevisionId() const;

    //! Merge an ordered collection of revisions to the Db
    //! @param[in] mergeRevisions Ordered collection of revisions to be applied
    //! @return SUCCESS if the revisions were successfully applied, ERROR otherwise. 
    DGNPLATFORM_EXPORT BentleyStatus MergeRevisions(bvector<DgnRevisionPtr> const& mergeRevisions);

    //! Start creating a new revision from the changes saved to the Db
    //! @return Newly created revision. Null if there was an error.
    //! @remarks 
    //! <ul>
    //! <li> The revision is initialized with an id, a parent id and a local file
    //! containing all the changes made since the previous revision. 
    //! <li> The revision must be finished or aborted with calls to FinishCreateRevision()
    //! or AbortCreateRevision()
    //! <li> While a revision is being created, transactions cannot be undone. 
    //! </ul>
    //! @see FinishCreateRevision, AbandonCreateRevision
    DGNPLATFORM_EXPORT DgnRevisionPtr StartCreateRevision();
    
    //! Finish creating a new revision
    //! @return SUCCESS if the revision was successfully finished, ERROR otherwise. 
    //! @remarks Upon successful return, the transaction table is flushed - so changes cannot be undone anymore
    //! @see StartCreateRevision
    DGNPLATFORM_EXPORT BentleyStatus FinishCreateRevision();

    //! Abandon creating a new revision
    //! @see StartCreateRevision
    DGNPLATFORM_EXPORT void AbandonCreateRevision();


};

END_BENTLEY_DGNPLATFORM_NAMESPACE
