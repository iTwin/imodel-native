/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/ChangeSet.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "BeSQLite.h"

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! When enabled, this class maintains a list of "changed rows" (inserts, updates and deletes) for a BeSQLite::Db. This information is
//! stored in memory by this class. At appropriate boundaries, applications convert the contents of a ChangeTracker
//! into a ChangeSet that can be saved as a blob and recorded.
//! @note @li ChangeTrackers keep track of "net changes" so, for example, if a row is added and then subsequently deleted
//! there is no net change. Likewise, if a row is changed more than once, only the net changes are stored and
//! intermediate values are not.
//! @note @li A single database may have more than one ChangeTracker active.
//! @note @li You cannot use nested transactions (via the @ref Savepoint "Savepoint API)
//! if a ChangeTracker is enabled for that file.
// @bsiclass                                                    Keith.Bentley   05/11
//=======================================================================================
struct ChangeTracker : RefCountedBase
{
protected:
    bool            m_isTracking;
    Db*             m_db;
    SqlSessionP     m_session;
    Utf8String      m_name;

    friend struct Db;
    friend struct DbFile;
    enum class OnCommitStatus {Continue=0, Abort, Completed};
    enum class TrackChangesForTable : bool {No=0, Yes=1};

    BE_SQLITE_EXPORT DbResult CreateSession();
    virtual OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) = 0;
    virtual void _OnSettingsSave() {}
    virtual void _OnSettingsSaved() {}
    void SetDb(Db* db) {m_db = db;}
    Db* GetDb() {return m_db;}
    Utf8CP GetName() const {return m_name.c_str();}

public:
    ChangeTracker(Utf8CP name=NULL) : m_name(name) {m_session=0; m_db=0; m_isTracking=false;}
    virtual ~ChangeTracker() {EndTracking();}
    virtual TrackChangesForTable _FilterTable(Utf8CP tableName) {return TrackChangesForTable::Yes;}
    SqlSessionP GetSqlSession() {return m_session;}

    //! Track all of the differences between the Db of this ChangeTracker and a changed copy of that database.
    //! @param[out] errMsg  If not null, an explanatory error message is returned in case of failure
    //! @param[in] baseFile A different version of the same db
    //! @return BE_SQLITE_OK if hangeset was created; else a non-zero error status if the diff failed. Returns BE_SQLITE_MISMATCH if the two Dbs have different GUIDs.
    //! @note This function will return an error if the two files have different DbGuids. 'baseFile' must identify a version of Db.
    BE_SQLITE_EXPORT DbResult DifferenceToDb(Utf8StringP errMsg, BeFileNameCR baseFile);

    //! Turn off change tracking for a database.
    BE_SQLITE_EXPORT void EndTracking();

    //! Temporarily suspend or resume change tracking
    //! @param[in] val if true, enable tracking.
    BE_SQLITE_EXPORT bool EnableTracking(bool val);

    //! turn on or off the "indirect changes" flag. All changes are marked as either direct or indirect according to the state of this flag.
    //! @param[in] val if true, changes are marked as indirect.
    BE_SQLITE_EXPORT void SetIndirectChanges(bool val);

    //! Determine whether any changes have been tracked by this ChangeTracker.
    BE_SQLITE_EXPORT bool HasChanges();

    //! Clear the contents of this ChangeTracker and re-start it.
    void Restart() {EndTracking(); EnableTracking(true);}
    bool IsTracking() const {return m_isTracking;}

};

//=======================================================================================
//! An Iterator for a ChangeSet. This class is used to step through the individual changes within a ChangeSet.
// @bsiclass                                                    Keith.Bentley   05/11
//=======================================================================================
struct Changes
{
private:
    struct ChangeSet& m_changeset;
    mutable SqlChangesetIterP m_iter;
    void Finalize() const;

public:
    Changes(Changes const& other) : m_changeset(other.m_changeset) {m_iter=0;}

    //! Construct an iterator for a Changeset
    Changes(ChangeSet& changeset) : m_changeset(changeset) {m_iter=0;};
    BE_SQLITE_EXPORT ~Changes();

    //! A single change to a database row.
    struct Change : std::iterator<std::input_iterator_tag, Change const>
        {
        private:
            bool  m_isValid;
            SqlChangesetIterP m_iter;

        public:
            Change(SqlChangesetIterP iter, bool isValid) {m_iter=iter; m_isValid=isValid;}
            //! get the "operation" that happened to this row.
            //! @param[out] tableName the name of the table to which the change was made. Changes within a ChangeSet are always
            //! sorted by table. So, all of the changes for a given table will appear in order before any changes to another table.
            //! However, it is not possible to predict the order of tables within a ChangeSet.
            //! @param[out] nCols the number of columns in the changed table.
            //! @param[out] opcode the opcode of the change. One of SQLITE_INSERT, SQLITE_DELETE, or SQLITE_UPDATE.
            //! @param[out] indirect true if the change was an indirect change.
            BE_SQLITE_EXPORT DbResult GetOperation(Utf8CP* tableName, int* nCols, DbOpcode* opcode, int* indirect) const;

            //! get the columns that form the primary key for the changed row.
            BE_SQLITE_EXPORT DbResult GetPrimaryKeyColumns(Byte** cols, int* nCols) const;

            //! get the previous value of a changed column.
            //! @param colNum the column number desired.
            //! @note this method is only valid if the opcode is either SQLITE_DELETE or SQLITE_UPDATE.
            //! For SQLITE_UPDATE, any column that was not affected by the change will be invalid.
            BE_SQLITE_EXPORT DbValue GetOldValue(int colNum) const;

            //! get the new value of a changed column.
            //! @param colNum the column number desired.
            //! @note this method is only valid if the opcode is either SQLITE_INSERT or SQLITE_UPDATE.
            //! For SQLITE_UPDATE, any column that was not affected by the change will be invalid.
            BE_SQLITE_EXPORT DbValue GetNewValue(int colNum) const;

            enum class Stage : bool {Old=0, New=1};
            DbValue GetValue(int colNum, Stage stage) const {return (stage==Stage::Old) ? GetOldValue(colNum) : GetNewValue(colNum);}

            //! Move the iterator to the next entry in the ChangeSet. After the last valid entry, the entry will be invalid.
            BE_SQLITE_EXPORT Change& operator++();

            bool IsValid() const {return m_isValid;}

            Change const& operator* () const {return *this;}
            bool operator!=(Change const& rhs) const {return (m_iter != rhs.m_iter) || (m_isValid != rhs.m_isValid);}
            bool operator==(Change const& rhs) const {return (m_iter == rhs.m_iter) && (m_isValid == rhs.m_isValid);}

            //! Dump to stdout for debugging purposes.
            BE_SQLITE_EXPORT void Dump(Db const&, bool isPatchSet, bset<Utf8String>& tablesSeen, int detailLevel) const;

            void Dump(Db const& db, bool isPatchSet, int detailLevel) const {bset<Utf8String> tablesSeen; Dump(db, isPatchSet, tablesSeen, detailLevel);}

            //! Dump one or more columns to stdout for debugging purposes.
            BE_SQLITE_EXPORT void DumpColumns(int startCol, int endCol, Changes::Change::Stage stage, bvector<Utf8String> const& columns, int detailLevel) const;

            //! Format the primary key columns of this change for debugging purposes.
            BE_SQLITE_EXPORT Utf8String FormatPrimarykeyColumns(bool isInsert, int detailLevel) const;
        };

    typedef Change const_iterator;

    //! Get the first entry in the ChangeSet.
    BE_SQLITE_EXPORT Change begin() const;

    //! Get the last entry in the ChangeSet.
    Change end() const {return Change(m_iter, false);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
struct ChangeGroup : NonCopyableClass
{
    friend struct ChangeSet;

private:
    void*  m_changegroup;
    
public:
    BE_SQLITE_EXPORT ChangeGroup();
    BE_SQLITE_EXPORT ~ChangeGroup();
    BE_SQLITE_EXPORT DbResult AddChanges(int size, void const* data);
};

//=======================================================================================
//! A set of changes to database rows. A ChangeSet is a contiguous set of bytes that serializes everything
//! tracked by a ChangeTracker. It can be constructed from a ChangeTracker and then saved
//! to remember what happened. Then, a ChangeTracker can be recreated from a previously saved state. One
//! way to use a ChangeSet is to transmit it to another computer holding another copy of the same database and then
//! "Apply" those changes. In this way two sets of changes to database can be merged to form a single database
//! with both sets of changes.
//! Optionally, a ChangeSet may be "inverted" from its saved state - meaning it will hold a ChangeSet that reverses all
//! of the original changes (inserts become deletes and vice versa, updates restore the original values.)
//! In this way, ChangeSets can be used to implement application Undo.
// @bsiclass                                                    Keith.Bentley   05/11
//=======================================================================================
struct ChangeSet : NonCopyableClass
{
    enum class SetType : bool {Full=0, Patch=1};
    enum class ApplyChangesForTable : bool {No=0, Yes=1};
    enum class ConflictCause : int {Data=1, NotFound=2, Conflict=3, Constraint=4, ForeignKey=5};
    enum class ConflictResolution : int {Skip=0, Replace=1, Abort=2};

private:
    int    m_size;
    void*  m_changeset;

public:
    virtual ApplyChangesForTable _FilterTable(Utf8CP tableName) {return ApplyChangesForTable::Yes;}
    virtual ConflictResolution _OnConflict(ConflictCause clause, Changes::Change iter) = 0;

public:
    //! construct a blank, empty ChangeSet
    ChangeSet() {m_size=0; m_changeset=nullptr;}
    ~ChangeSet() {Free();}

    //! Free the data held by this ChangeSet.
    //! @note Normally the destructor will call Free. After this call the ChangeSet is invalid.
    BE_SQLITE_EXPORT void Free();

    BE_SQLITE_EXPORT DbResult Invert();

    //! Re-create this ChangeSet from data from a previously saved ChangeSet.
    BE_SQLITE_EXPORT DbResult FromData(int size, void const* data, bool invert);

    //! Create a ChangeSet or PatchSet from a ChangeTracker. The ChangeSet can then be saved persistently.
    //! @param[in] tracker  ChangeTracker from which to create ChangeSet or PatchSet
    //! @param[in] setType  whether to create a full ChangeSet or just a PatchSet
    BE_SQLITE_EXPORT DbResult FromChangeTrack(ChangeTracker& tracker, SetType setType=SetType::Full);

    BE_SQLITE_EXPORT DbResult FromChangeGroup(ChangeGroup& changegroup);

    //! Apply all of the changes in a ChangeSet to the supplied database.
    //! @param[in] db the database to which the changes are applied.
    BE_SQLITE_EXPORT DbResult ApplyChanges(DbR db);

    BE_SQLITE_EXPORT DbResult ConcatenateWith(ChangeSet const& second);

    //! Get the number of bytes in this ChangeSet.
    int GetSize() const {return m_size;}

    //! Get a pointer to the data for this ChangeSet.
    void const* GetData() const {return m_changeset;}

    //! Determine whether this ChangeSet holds valid data or not.
    bool IsValid() {return 0 != m_changeset;}

    //! Dump to stdout for debugging purposes.
    BE_SQLITE_EXPORT void Dump(Db const&, bool isPatchSet=false, int detailLevel=0) const;

    //! Get a description of a conflict cause for debugging purposes.
    BE_SQLITE_EXPORT static Utf8String InterpretConflictCause(ChangeSet::ConflictCause);
};


END_BENTLEY_SQLITE_NAMESPACE
