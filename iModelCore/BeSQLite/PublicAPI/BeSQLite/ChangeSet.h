/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/ChangeSet.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "BeSQLite.h"

BESQLITE_TYPEDEFS(IByteArray);
BESQLITE_TYPEDEFS(ChangeGroup);
BESQLITE_TYPEDEFS(IChangeSet);
BESQLITE_TYPEDEFS(ChangeSet);
BESQLITE_TYPEDEFS(ChangeStream);
BESQLITE_TYPEDEFS(DbSchemaChangeSet);

struct sqlite3_rebaser;

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   1/17
//=======================================================================================
struct IByteArray
{
private:
    virtual int _GetSize() const = 0;
    virtual void const* _GetData() const = 0;

protected:
    IByteArray() = default;
    ~IByteArray() = default;

public:
    //! Get the number of bytes
    int GetSize() const {return _GetSize();}

    //! Get a pointer to the data
    void const* GetData() const {return _GetData();}
};

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   1/17
//=======================================================================================
struct DbSchemaChangeSet : IByteArray
{
private:
    Utf8String m_ddl;

    int _GetSize() const override final {return IsEmpty() ? 0 : (int) m_ddl.SizeInBytes();}
    void const* _GetData() const override final {return m_ddl.c_str();}

public:
    //! Create a new schema change set
    DbSchemaChangeSet(Utf8CP ddl = nullptr) {m_ddl.AssignOrClear(ddl);}

    //! Destructor
    ~DbSchemaChangeSet() = default;

    //! Add new DDL statements to the schema change set (separate multiple commands by ';')
    BE_SQLITE_EXPORT void AddDDL(Utf8CP ddl);

    //! Returns true if the schema change set is empty
    bool IsEmpty() const {return m_ddl.empty();}

    //! Clear the schema change set
    void Clear() {m_ddl.clear();}

    //! Return the contents of the schema change set
    Utf8StringCR ToString() const {return m_ddl;}

    //! Dump the contents
    BE_SQLITE_EXPORT void Dump(Utf8CP label) const;
};

//=======================================================================================
//! When enabled, this class maintains a list of changed rows (inserts, updates and deletes) for a BeSQLite::Db. This information is
//! stored in memory by this class. At appropriate boundaries, applications convert the contents of a ChangeTracker
//! into a ChangeSet that can be saved as a blob and recorded.
//! @note @li ChangeTrackers keep track of "net changes" so, for example, if a row is added and then subsequently deleted
//! there is no net change. Likewise, if a row is changed more than once, only the net changes are stored and
//! intermediate values are not.
//! @note @li A single database may have more than one ChangeTracker active.
//! @note @li You cannot use nested transactions (via the @ref Savepoint "Savepoint" API)
//! if a ChangeTracker is enabled for that file.
// @bsiclass                                                    Keith.Bentley   05/11
//=======================================================================================
struct ChangeTracker : RefCountedBase
{
friend struct Db;
friend struct DbFile;

private:
    DbSchemaChangeSet m_dbSchemaChanges;
    DbResult RecordDbSchemaChange(Utf8CP ddl);

protected:
    bool            m_isTracking;
    Db*             m_db;
    SqlSessionP     m_session;
    Utf8String      m_name;
    
    enum class OnCommitStatus {Continue=0, Abort, Completed};
    enum class TrackChangesForTable : bool {No=0, Yes=1};

    BE_SQLITE_EXPORT DbResult CreateSession();
    virtual OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) = 0;
    virtual void _OnCommitted(bool isCommit, Utf8CP operation) {}
    void SetDb(Db* db) {m_db = db;}
    Db* GetDb() {return m_db;}
    Utf8CP GetName() const {return m_name.c_str();}

    DbSchemaChangeSetCR GetDbSchemaChanges() const {return m_dbSchemaChanges;}

public:
    ChangeTracker(Utf8CP name=NULL) : m_name(name) {m_session=0; m_db=0; m_isTracking=false;}
    virtual ~ChangeTracker() {EndTracking();}

    virtual TrackChangesForTable _FilterTable(Utf8CP tableName) {return TrackChangesForTable::Yes;}
    SqlSessionP GetSqlSession() {return m_session;}

    //! Track all of the differences between the Db of this ChangeTracker and a changed copy of that database.
    //! @param[out] errMsg  If not null, an explanatory error message is returned in case of failure
    //! @param[in] baseFile A different version of the same db
    //! @return BE_SQLITE_OK if changeset was created; else a non-zero error status if the diff failed. Returns BE_SQLITE_MISMATCH if the two Dbs have different GUIDs.
    //! @note This function will return an error if the two files have different DbGuids. 'baseFile' must identify a version of Db.
    BE_SQLITE_EXPORT DbResult DifferenceToDb(Utf8StringP errMsg, BeFileNameCR baseFile);

    //! Turn off change tracking for a database.
    BE_SQLITE_EXPORT void EndTracking();

    //! Temporarily suspend or resume change tracking
    //! @param[in] val if true, enable tracking.
    BE_SQLITE_EXPORT bool EnableTracking(bool val);

    //! All changes are marked as either direct or indirect according to the mode in which the ChangeTracker is currently operating.
    enum class Mode
    {
        Direct = 0, //!< All changes are marked as "direct".
        Indirect = 1, //!< All changes are marked as "indirect".
    };

    //! Query the current state of the "indirect changes" flag. All changes are marked as either direct or indirect according to the state of this flag.
    BE_SQLITE_EXPORT Mode GetMode() const;

    //! turn on or off the "indirect changes" flag. All changes are marked as either direct or indirect according to the state of this flag.
    //! @param[in] mode the new mode.
    BE_SQLITE_EXPORT void SetMode(Mode mode);

    //! Determine whether any changes (schema or data) have been tracked by this ChangeTracker.
    BE_SQLITE_EXPORT bool HasChanges() const;

    //! Determine whether any data changes tracked by this ChangeTracker
    BE_SQLITE_EXPORT bool HasDataChanges() const;

    //! Determine whether any db-schema changes have beeen tracked by this ChangeTracker
    bool HasDbSchemaChanges() const {return !m_dbSchemaChanges.IsEmpty();} 

    //! Clear the contents of this ChangeTracker and re-start it.
    void Restart() {EndTracking(); EnableTracking(true);}
    bool IsTracking() const {return m_isTracking;}
};

//=======================================================================================
//! An Iterator for a ChangeSet or a ChangeStream. This class is used to step through the individual 
//! changes within a ChangeSet, ChangeStream or individual pages of a ChangeStream.
// @bsiclass                                                    Keith.Bentley   05/11
//=======================================================================================
struct Changes
{
private:
    ChangeStream* m_changeStream = nullptr;
    int    m_size = 0;
    void*  m_data = nullptr;
    mutable SqlChangesetIterP m_iter = 0;
    bool m_invert = false;
    void Finalize() const;

public:
    //! Construct an iterator for a ChangeSet
    BE_SQLITE_EXPORT explicit Changes(ChangeSet const& changeSet, bool invert);

    //! Construct an iterator for a ChangeStream
    //! @remarks The ChangeStream needs to implement _InputPage to send the stream
    Changes(ChangeStream& changeStream, bool invert) : m_changeStream(&changeStream), m_invert(invert) {}

    //! Construct an iterator for a page of changes in a ChangeStream
    Changes(void* data, int size, bool invert) : m_data(data), m_size(size), m_invert(invert)  {}

    //! Copy constructor
    Changes(Changes const& other) : m_data(other.m_data), m_size(other.m_size), m_changeStream(other.m_changeStream), m_iter(0), m_invert(other.m_invert) {}

    BE_SQLITE_EXPORT ~Changes();

    //! A single change to a database row.
    struct Change : std::iterator<std::input_iterator_tag, Change const>
        {
        private:
            bool  m_isValid;
            SqlChangesetIterP m_iter;

            Utf8String FormatChange(Db const& db, Utf8CP tableName, DbOpcode opcode, int indirect, int detailLevel) const;

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
            
            //! Get the total number of foreign key violations in the database (if there's a ForeignKey conflict)
            //! @param nConflicts The number of foreign key conflicts
            //! @return Returns BE_SQLITE_OK if called from the conflict handler callback. In all other cases 
            //! this returns BE_SQLITE_MISUSE. 
            BE_SQLITE_EXPORT DbResult GetFKeyConflicts(int *nConflicts) const;

            //! Move the iterator to the next entry in the ChangeSet. After the last valid entry, the entry will be invalid.
            BE_SQLITE_EXPORT Change& operator++();

            bool IsValid() const {return m_isValid;}

            Change const& operator* () const {return *this;}
            bool operator!=(Change const& rhs) const {return (m_iter != rhs.m_iter) || (m_isValid != rhs.m_isValid);}
            bool operator==(Change const& rhs) const {return (m_iter == rhs.m_iter) && (m_isValid == rhs.m_isValid);}

            //! Dump to stdout for debugging purposes.
            BE_SQLITE_EXPORT void Dump(Db const&, bool isPatchSet, bset<Utf8String>& tablesSeen, int detailLevel) const;
            BE_SQLITE_EXPORT void DumpCurrentValuesOfChangedColumns(Db const& db) const;

            void Dump(Db const& db, bool isPatchSet, int detailLevel) const {bset<Utf8String> tablesSeen; Dump(db, isPatchSet, tablesSeen, detailLevel);}

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
    friend struct ChangeStream;
private:
    void*  m_changegroup;
    
public:
    BE_SQLITE_EXPORT ChangeGroup();
    BE_SQLITE_EXPORT ~ChangeGroup();
    BE_SQLITE_EXPORT DbResult AddChanges(int size, void const* data);
};

//=======================================================================================
//! A set of "rebases" that hold the result of conflict resolutions during a call to ChangeSet::ApplyChanges from
//! a ChangeSet received from a remote session. 
//! All ChangeSets for the local session should be "rebased" via calls to Rebaser::AddRebease + Rebaser::DoRebase
//! before sending to the server. This essentially moves the local ChangeSet to be "based on" the state of the 
//! database AFTER the remote changes were made, rather than the state of the database at the start of the session.
// @bsiclass                                                    Keith.Bentley   03/18
//=======================================================================================
struct Rebase : NonCopyableClass
{
    friend struct ChangeSet;
    friend struct ChangeStream;
private:
    int m_size = 0;
    void* m_data = nullptr;
public:
    Rebase() {}
    BE_SQLITE_EXPORT ~Rebase();
    bool HasData() const {return m_size != 0;}
    int GetSize() const {return m_size;}
    void* GetData() const {return m_data;}
};

//=======================================================================================
//! Tool to "rebase" a ChangeSet to become based on the state of the database "as of" a new state, different than the beginning
//! of the session in which the changes were recoreded. This is only necessary when remote changes are applied and conflicts are resolved. 
//! See SQlite documentation on "Rebasing changesets" for complete explanation.
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
struct Rebaser : NonCopyableClass
{
private:
    sqlite3_rebaser* m_rebaser;
    
public:
    BE_SQLITE_EXPORT Rebaser();
    BE_SQLITE_EXPORT ~Rebaser();
    
    BE_SQLITE_EXPORT void AddRebase(Rebase const& rebase);
    BE_SQLITE_EXPORT void AddRebase(void const* data, int count);
    BE_SQLITE_EXPORT DbResult DoRebase(struct ChangeSet const&in, struct ChangeSet& out);
    BE_SQLITE_EXPORT DbResult DoRebase(struct ChangeStream const& in, struct ChangeStream& out);

};

//=======================================================================================
// @bsiclass                                                     Paul.Connelly   11/15
//=======================================================================================
struct IChangeSet
{
public:
    enum class SetType : bool {Full=0, Patch=1};
    enum class ApplyChangesForTable : bool {No=0, Yes=1};
    enum class ConflictCause : int {Data=1, NotFound=2, Conflict=3, Constraint=4, ForeignKey=5};
    enum class ConflictResolution : int {Skip=0, Replace=1, Abort=2};

private:
    virtual DbResult _FromChangeTrack(ChangeTracker& tracker, SetType setType) = 0;
    virtual DbResult _FromChangeGroup(ChangeGroupCR changeGroup) = 0;
    virtual DbResult _ApplyChanges(DbR db, Rebase* rebase, bool invert) = 0;
    virtual Changes _GetChanges(bool invert) = 0;
    virtual bool _IsEmpty() const = 0;
protected:
    ~IChangeSet() = default;
    
    virtual ApplyChangesForTable _FilterTable(Utf8CP tableName) {return ApplyChangesForTable::Yes;}
    virtual ConflictResolution _OnConflict(ConflictCause clause, Changes::Change iter) = 0;

public:
    //! Implement to handle conflicts when applying changes
    //! @see ApplyChanges
    ApplyChangesForTable FilterTable(Utf8CP tableName) {return _FilterTable(tableName);}

    //! Implement to filter out specific tables when applying changes
    //! @see ApplyChanges
    ConflictResolution OnConflict(ConflictCause cause, Changes::Change iter) {return _OnConflict(cause, iter);}

    //! Create a ChangeSet/ChangeStream from a ChangeTracker. The ChangeSet can then be saved persistently.
    //! @param[in] tracker  ChangeTracker from which to create ChangeSet or PatchSet
    //! @param[in] setType  whether to create a full ChangeSet or just a PatchSet
    //! @return BE_SQLITE_OK if successful. Error status otherwise. 
    //! @remarks If using a ChangeStream, implement _OutputPage to receive the stream
    DbResult FromChangeTrack(ChangeTracker& tracker, SetType setType=SetType::Full) {return _FromChangeTrack(tracker, setType);}

    //! Create a ChangeSet/ChangeStream by merging the contents of a ChangeGroup
    //! @param[in] changeGroup ChangeGroup to be merged together. 
    //! @return BE_SQLITE_OK if successful. Error status otherwise. 
    //! @remarks If using a ChangeStream, implement _OutputPage to receive the stream
    DbResult FromChangeGroup(ChangeGroupCR changeGroup) {return _FromChangeGroup(changeGroup);}

    //! Apply all of the changes in this IChangeSet to the supplied database.
    //! @param[in] db the database to which the changes are applied.
    //! @param[in] rebase a Rebase object to record conflict resolutions
    //! @param[in] invert invert the changeset and apply
    //! @return BE_SQLITE_OK if successful. Error status otherwise. 
    //! @remarks If using a ChangeStream, implement _InputPage to send the stream. 
    //! If the apply fails, it's upto the caller to call AbandonChanges() to abandon the 
    //! transaction containing partially applied changes. 
    DbResult ApplyChanges(DbR db, Rebase* rebase = nullptr, bool invert = false) {return _ApplyChanges(db, rebase, invert);}

    //! Returns a Changes object for iterating over the changes contained within this IChangeSet
    //! @param[in] invert reverse the changeset when interating
    //! @remarks If using a ChangeStream, implement _InputPage to send the stream
    Changes GetChanges(bool invert = false) {return _GetChanges(invert);}

    //! Returns true if change is not empty
    bool IsEmpty() const {return _IsEmpty();}

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
struct ChangeSet : NonCopyableClass, IChangeSet, IByteArray
{
    friend struct Rebaser;
private:

    int    m_size;
    void*  m_changeset;

    int _GetSize() const override {return m_size;}
    void const* _GetData() const override {return m_changeset;}

    BE_SQLITE_EXPORT DbResult _FromChangeTrack(ChangeTracker& tracker, SetType setType) override final;
    BE_SQLITE_EXPORT DbResult _FromChangeGroup(ChangeGroupCR changeGroup) override final;
    BE_SQLITE_EXPORT DbResult _ApplyChanges(DbR db, Rebase*, bool) override final;
    bool _IsEmpty() const override final { return !m_changeset || !m_size; }
    Changes _GetChanges(bool invert) override final {return Changes(*this, invert);}    
public:
    //! construct a blank, empty ChangeSet
    ChangeSet() {m_size=0; m_changeset=nullptr;}
    virtual ~ChangeSet() {Free();}

    //! Free the data held by this ChangeSet.
    //! @note Normally the destructor will call Free. After this call the ChangeSet is invalid.
    BE_SQLITE_EXPORT void Free();

    BE_SQLITE_EXPORT DbResult Invert();

    //! Re-create this ChangeSet from data from a previously saved ChangeSet.
    //! @return BE_SQLITE_OK if successful. Error status otherwise. 
    BE_SQLITE_EXPORT DbResult FromData(int size, void const* data, bool invert);

    //! Concatenate this ChangeSet with a second ChangeSet
    //! @param[in] second The change set to concatenate
    //! @return BE_SQLITE_OK if successful. Error status otherwise. 
    BE_SQLITE_EXPORT DbResult ConcatenateWith(ChangeSet const& second);

    //! Determine whether this ChangeSet holds valid data or not.
    bool IsValid() {return 0 != m_changeset;}

    //! Dump to stdout for debugging purposes.
    BE_SQLITE_EXPORT void Dump(Utf8CP label, Db const&, bool isPatchSet=false, int detailLevel=0) const;

    //! Get a description of a conflict cause for debugging purposes.
    BE_SQLITE_EXPORT static Utf8CP InterpretConflictCause(ConflictCause, int detailLevel=0);
};

//=======================================================================================
//! A ChangeSet implementation for where conflicts are just not expected
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct AbortOnConflictChangeSet : BeSQLite::ChangeSet
{
private:
    ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override final
        {
        BeAssert(false);
        return ChangeSet::ConflictResolution::Abort;
        }
};

//=======================================================================================
//! A base class for a streaming version of the ChangeSet. ChangeSets require that their 
//! entire contents be stored in large memory buffers. This streaming version is meant to
//! be used in low memory environments where it is required to handle very large Change Sets. 
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct ChangeStream : NonCopyableClass, IChangeSet
{
    friend struct Changes;
    friend struct Rebaser;
private:
    static int OutputCallback(void *pOut, const void *pData, int nData);
    static int InputCallback(void *pIn, void *pData, int *pnData);
    static int ConflictCallback(void *pCtx, int cause, SqlChangesetIterP iter);
    static int FilterTableCallback(void *pCtx, Utf8CP tableName);

    // Resets the change stream, and is internally called at the 
    // end of various change stream operations
    void Reset() {_Reset();}

    BE_SQLITE_EXPORT DbResult _FromChangeTrack(ChangeTracker& tracker, SetType setType) override final;
    BE_SQLITE_EXPORT DbResult _FromChangeGroup(ChangeGroupCR changeGroup) override final;
    BE_SQLITE_EXPORT DbResult _ApplyChanges(DbR db, Rebase*, bool) override final;
    BE_SQLITE_EXPORT bool _IsEmpty() const override final;
    Changes _GetChanges(bool invert) override final {return Changes(*this, invert);}

    //! Application implements this to supply input to the system. 
    //! @param[out] pData Buffer to copy data into. 
    //! @param[in,out] pnData System sets this to the size of the buffer. Implementation sets it to the 
    //! actual number of bytes copied. If the input is exhausted implementation should set this to 0. 
    //! @return BE_SQLITE_OK if successfully copied data. Return BE_SQLITE_ERROR otherwise. 
    virtual DbResult _InputPage(void *pData, int *pnData) {return BE_SQLITE_OK;}

    //! Application implements this to receive data from the system. 
    //! @param[in] pData Points to a buffer containing the output data
    //! @param[in] nData Size of buffer
    //! @return BE_SQLITE_OK if the data has been successfully processed. Return BE_SQLITE_ERROR otherwise. 
    virtual DbResult _OutputPage(const void *pData, int nData) {return BE_SQLITE_OK;}

    //! Override to reset any state of the change stream
    //! @remarks Called at end of various change stream operations, and is used by application to reset the stream and 
    //! dispose resources as necessary. 
    virtual void _Reset() {}

protected:
    ChangeStream() = default;
    ~ChangeStream() = default;

public:
    BE_SQLITE_EXPORT static DbResult TransferBytesBetweenStreams(ChangeStream& inStream, ChangeStream& outStream);
    
    //! Stream changes to a ChangeGroup. 
    //! @param[out] changeGroup ChangeGroup to stream to stream the contents to
    //! @remarks Implement _InputPage to send the stream
    //! @return BE_SQLITE_OK if successful. Error status otherwise. 
    BE_SQLITE_EXPORT DbResult ToChangeGroup(ChangeGroup& changeGroup);

    //! Stream changes to an in-memory ChangeSet
    //! @param[out] changeSet ChangeSet to stream the contents to
    //! @param[in] invert Pass true if the input stream needs to be inverted
    //! @remarks Implement _InputPage to send the stream
    //! @return BE_SQLITE_OK if successful. Error status otherwise. 
    BE_SQLITE_EXPORT DbResult ToChangeSet(ChangeSet& changeSet, bool invert = false);

    //! Stream changes from another ChangeStream
    //! @param[in] inStream Another stream that provides input
    //! @param[in] invert Pass true if the input stream needs to be inverted
    //! @remarks Implement _OutputPage to receive the input stream. The
    //! input stream needs to implement _InputPage to send the stream. 
    //! @return BE_SQLITE_OK if successful. Error status otherwise. 
    BE_SQLITE_EXPORT DbResult FromChangeStream(ChangeStream& inStream, bool invert = false);

    //! Stream changes to another ChangeStream
    //! @param[out] outStream Another stream that accepts the output
    //! @param[in] invert Pass true if this stream needs to be inverted
    //! @remarks Implement _InputPage to send this stream. The output stream
    //! needs to implement _OutputPage to receive this stream. 
    //! @return BE_SQLITE_OK if successful. Error status otherwise. 
    BE_SQLITE_EXPORT DbResult ToChangeStream(ChangeStream& outStream, bool invert = false);

    //! Stream changes by concatenating two other ChangeStream-s
    //! @param[in] inStream1 First input stream
    //! @param[in] inStream2 Second input stream
    //! @remarks Implement _OutputPage to receive the concatenated stream. The input streams
    //! need to implement _InputPage to send the streams. 
    //! @return BE_SQLITE_OK if successful. Error status otherwise. 
    BE_SQLITE_EXPORT DbResult FromConcatenatedChangeStreams(ChangeStream& inStream1, ChangeStream& inStream2);
    
    //! Dump the contents of this stream for debugging
    BE_SQLITE_EXPORT void Dump(Utf8CP label, DbCR db, bool isPatchSet = false, int detailLevel = 0);
};

END_BENTLEY_SQLITE_NAMESPACE
