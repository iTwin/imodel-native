/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <cstddef>
#include "BeSQLite.h"

BESQLITE_TYPEDEFS(ChangeGroup);
BESQLITE_TYPEDEFS(ChangeSet);
BESQLITE_TYPEDEFS(ChangeStream);
BESQLITE_TYPEDEFS(DdlChanges);

struct sqlite3_rebaser;

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! An Iterator for a ChangeSet or a ChangeStream. This class is used to step through the individual
//! changes within a ChangeSet, ChangeStream or individual pages of a ChangeStream.
// @bsiclass
//=======================================================================================
struct Changes {

    struct Reader : RefCountedBase {
        static int ReadCallback(void* reader, void* data, int* pSize) { return (int)((Reader*)reader)->_Read((Byte*)data, pSize); }
        virtual DbResult _Read(Byte* data, int* pSize) = 0;
    };

private:
    ChangeStreamCR m_changeStream;
    mutable RefCountedPtr<Reader> m_reader;
    mutable SqlChangesetIterP m_iter = 0;
    bool m_invert;
    void Finalize() const;

public:
    //! Construct an iterator for a ChangeStream
    //! @remarks The ChangeStream needs to implement _Read to send the stream
    explicit Changes(ChangeStream const& changeStream, bool invert) : m_changeStream(changeStream), m_invert(invert) {}

    //! Copy constructor
    Changes(Changes const& other) : m_changeStream(other.m_changeStream), m_iter(0), m_invert(other.m_invert) { m_reader = nullptr; }

    BE_SQLITE_EXPORT ~Changes();

    //! A single change to a database row.
    struct Change {
        using iterator_category=std::input_iterator_tag;
        using value_type=Change const;
        using difference_type=std::ptrdiff_t;
        using pointer=Change const*;
        using reference=Change const&;

    private:
        bool m_isValid;
        SqlChangesetIterP m_iter;

        Utf8String FormatChange(Db const& db, Utf8CP tableName, DbOpcode opcode, int indirect, int detailLevel) const;

    public:
        Change(SqlChangesetIterP iter, bool isValid) {
            m_iter = iter;
            m_isValid = isValid;
        }
        //! get the "operation" that happened to this row.
        //! @param[out] tableName the name of the table to which the change was made. Changes within a ChangeSet are always
        //! sorted by table. So, all of the changes for a given table will appear in order before any changes to another table.
        //! However, it is not possible to predict the order of tables within a ChangeSet.
        //! @param[out] nCols the number of columns in the changed table.
        //! @param[out] opcode the opcode of the change. One of SQLITE_INSERT, SQLITE_DELETE, or SQLITE_UPDATE.
        //! @param[out] indirect true if the change was an indirect change.
        BE_SQLITE_EXPORT DbResult GetOperation(Utf8CP* tableName, int* nCols, DbOpcode* opcode, int* indirect) const;

        bool IsIndirect() const
            {
            int indirect;
            Utf8CP tableName;
            int nCols;
            DbOpcode opcode;
            auto rc = GetOperation(&tableName, &nCols, &opcode, &indirect);
            BeAssert(BE_SQLITE_OK == rc);
            return BE_SQLITE_OK == rc && 0 != indirect;
            }

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

        enum class Stage : bool { Old = 0, New = 1 };
        DbValue GetValue(int colNum, Stage stage) const { return (stage == Stage::Old) ? GetOldValue(colNum) : GetNewValue(colNum); }

        //! Get the total number of foreign key violations in the database (if there's a ForeignKey conflict)
        //! @param nConflicts The number of foreign key conflicts
        //! @return Returns BE_SQLITE_OK if called from the conflict handler callback. In all other cases
        //! this returns BE_SQLITE_MISUSE.
        BE_SQLITE_EXPORT DbResult GetFKeyConflicts(int* nConflicts) const;

        //! Move the iterator to the next entry in the ChangeSet. After the last valid entry, the entry will be invalid.
        BE_SQLITE_EXPORT Change& operator++();

        bool IsValid() const { return m_isValid; }

        Change const& operator*() const { return *this; }
        bool operator!=(Change const& rhs) const { return (m_iter != rhs.m_iter) || (m_isValid != rhs.m_isValid); }
        bool operator==(Change const& rhs) const { return (m_iter == rhs.m_iter) && (m_isValid == rhs.m_isValid); }

        //! Dump to stdout for debugging purposes.
        BE_SQLITE_EXPORT void Dump(Db const&, bool isPatchSet, bset<Utf8String>& tablesSeen, int detailLevel) const;
        BE_SQLITE_EXPORT void DumpCurrentValuesOfChangedColumns(Db const& db) const;

        void Dump(Db const& db, bool isPatchSet, int detailLevel) const {
            bset<Utf8String> tablesSeen;
            Dump(db, isPatchSet, tablesSeen, detailLevel);
        }

        //! Format the primary key columns of this change for debugging purposes.
        BE_SQLITE_EXPORT Utf8String FormatPrimarykeyColumns(bool isInsert, int detailLevel) const;
    };

    typedef Change const_iterator;

    //! Get the first entry in the ChangeSet.
    BE_SQLITE_EXPORT Change begin() const;

    //! Get the last entry in the ChangeSet.
    Change end() const { return Change(m_iter, false); }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChangeGroup : NonCopyableClass {
    friend struct ChangeSet;
    friend struct ChangeStream;

private:
    void* m_changegroup;
    bool m_containsEcSchemaChanges = false;

public:
    bool ContainsEcSchemaChanges() const { return m_containsEcSchemaChanges; }
    void SetContainsEcSchemaChanges() { m_containsEcSchemaChanges = true; }
    BE_SQLITE_EXPORT ChangeGroup();
    BE_SQLITE_EXPORT ~ChangeGroup();
};

//=======================================================================================
//! A set of "rebases" that hold the result of conflict resolutions during a call to ChangeSet::ApplyChanges from
//! a ChangeSet received from a remote session.
//! All ChangeSets for the local session should be "rebased" via calls to Rebaser::AddRebease + Rebaser::DoRebase
//! before sending to the server. This essentially moves the local ChangeSet to be "based on" the state of the
//! database AFTER the remote changes were made, rather than the state of the database at the start of the session.
// @bsiclass
//=======================================================================================
struct Rebase : NonCopyableClass {
    friend struct ChangeSet;
    friend struct ChangeStream;

private:
    int m_size = 0;
    void* m_data = nullptr;

public:
    Rebase() {}
    BE_SQLITE_EXPORT ~Rebase();
    bool HasData() const { return m_size != 0; }
    int GetSize() const { return m_size; }
    void* GetData() const { return m_data; }
};

//=======================================================================================
//! Tool to "rebase" a ChangeSet to become based on the state of the database "as of" a new state, different than the beginning
//! of the session in which the changes were recoreded. This is only necessary when remote changes are applied and conflicts are resolved.
//! See SQlite documentation on "Rebasing changesets" for complete explanation.
// @bsiclass
//=======================================================================================
struct Rebaser : NonCopyableClass {
private:
    sqlite3_rebaser* m_rebaser;

public:
    BE_SQLITE_EXPORT Rebaser();
    BE_SQLITE_EXPORT ~Rebaser();

    BE_SQLITE_EXPORT DbResult AddRebase(Rebase const& rebase);
    BE_SQLITE_EXPORT DbResult AddRebase(void const* data, int count);
    BE_SQLITE_EXPORT DbResult DoRebase(struct ChangeSet const& in, struct ChangeSet& out);
    BE_SQLITE_EXPORT DbResult DoRebase(struct ChangeStream const& in, struct ChangeStream& out);
};

//=======================================================================================
//! A base class for a streaming version of the ChangeSet. ChangeSets require that their
//! entire contents be stored in large memory buffers. This streaming version is meant to
//! be used in low memory environments where it is required to handle very large Change Sets.
// @bsiclass
//=======================================================================================
struct ChangeStream : NonCopyableClass {
    friend struct Changes;
    friend struct Rebaser;

    enum class SetType : bool { Full = 0, Patch = 1 };
    enum class ApplyChangesForTable : bool { No = 0, Yes = 1 };
    enum class ConflictCause : int { Data = 1, NotFound = 2, Conflict = 3, Constraint = 4, ForeignKey = 5 };
    enum class ConflictResolution : int { Skip = 0, Replace = 1, Abort = 2 };

protected:
    static int ConflictCallback(void* pCtx, int cause, SqlChangesetIterP iter);
    static int FilterTableCallback(void* pCtx, Utf8CP tableName);

    virtual ApplyChangesForTable _FilterTable(Utf8CP tableName) { return ApplyChangesForTable::Yes; }
    virtual ConflictResolution _OnConflict(ConflictCause clause, Changes::Change iter) = 0;

    //! Application implements this to receive data from the system.
    //! @param[in] pData Points to a buffer containing the output data
    //! @param[in] nData Size of buffer
    //! @return BE_SQLITE_OK if the data has been successfully processed. Return BE_SQLITE_ERROR otherwise.
    virtual DbResult _Append(Byte const* pData, int nData) = 0;
    static int AppendCallback(void* pOut, const void* pData, int nData) { return (int)((ChangeStream*)pOut)->_Append((Byte const*)pData, nData); }

public:
    virtual bool _IsEmpty() const = 0;

    Changes GetChanges(bool invert = false) { return Changes(*this, invert); }
    BE_SQLITE_EXPORT DbResult FromChangeTrack(ChangeTracker& tracker, SetType setType = SetType::Full);
    BE_SQLITE_EXPORT DbResult FromChangeGroup(ChangeGroupCR changeGroup);
    BE_SQLITE_EXPORT DbResult ApplyChanges(DbR db, Rebase* rebase = nullptr, bool invert = false) const;
    BE_SQLITE_EXPORT DbResult ReadFrom(Changes::Reader& reader);
    BE_SQLITE_EXPORT DbResult InvertFrom(Changes::Reader& reader);

    //! Implement to handle conflicts when applying changes
    //! @see ApplyChanges
    ApplyChangesForTable FilterTable(Utf8CP tableName) { return _FilterTable(tableName); }

    //! Implement to filter out specific tables when applying changes
    //! @see ApplyChanges
    ConflictResolution OnConflict(ConflictCause cause, Changes::Change iter) { return _OnConflict(cause, iter); }

    virtual RefCountedPtr<Changes::Reader> _GetReader() const = 0;

    //! Stream changes to a ChangeGroup.
    //! @param[out] changeGroup ChangeGroup to stream to stream the contents to
    //! @return BE_SQLITE_OK if successful. Error status otherwise.
    BE_SQLITE_EXPORT DbResult AddToChangeGroup(ChangeGroup& changeGroup);

    //! Stream changes to an in-memory ChangeSet
    //! @param[out] changeSet ChangeSet to stream the contents to
    //! @param[in] invert Pass true if the input stream needs to be inverted
    //! @return BE_SQLITE_OK if successful. Error status otherwise.
    BE_SQLITE_EXPORT DbResult ToChangeSet(ChangeSet& changeSet, bool invert = false);

    //! Stream changes by concatenating two other ChangeStream-s
    //! @param[in] inStream1 First input stream
    //! @param[in] inStream2 Second input stream
    //! @return BE_SQLITE_OK if successful. Error status otherwise.
    BE_SQLITE_EXPORT DbResult FromConcatenatedChangeStreams(ChangeStream const& inStream1, ChangeStream const& inStream2);

    //! Dump the contents of this stream for debugging
    BE_SQLITE_EXPORT void Dump(Utf8CP label, DbCR db, bool isPatchSet = false, int detailLevel = 0) const;

    //! Get a description of a conflict cause for debugging purposes.
    BE_SQLITE_EXPORT static Utf8CP InterpretConflictCause(ConflictCause, int detailLevel = 0);
};

//=======================================================================================
//! A set of changes to database rows. A ChangeSet is a set of bytes that serializes everything
//! tracked by a ChangeTracker. It can be constructed from a ChangeTracker and then saved
//! to remember what happened. Then, a ChangeTracker can be recreated from a previously saved state. One
//! way to use a ChangeSet is to transmit it to another computer holding another copy of the same database and then
//! "Apply" those changes. In this way two sets of changes to database can be merged to form a single database
//! with both sets of changes.
//! Optionally, a ChangeSet may be "inverted" from its saved state - meaning it will hold a ChangeSet that reverses all
//! of the original changes (inserts become deletes and vice versa, updates restore the original values.)
//! In this way, ChangeSets can be used to implement application Undo.
// @bsiclass
//=======================================================================================
struct ChangeSet : ChangeStream {
    ChunkedArray m_data;

    struct Reader : Changes::Reader {
        ChunkedArray::Reader m_arrayReader;
        Reader(ChangeSet const& changeSet) : m_arrayReader(changeSet.m_data) {}
        DbResult _Read(Byte* data, int* pSize) override final {
            m_arrayReader.Read(data, pSize);
            return BE_SQLITE_OK;
        }
    };

    size_t GetSize() const { return m_data.m_size; }
    RefCountedPtr<Changes::Reader> _GetReader() const override final { return new Reader(*this); }
    void Clear() { m_data.Clear(); }

    DbResult _Append(Byte const* data, int size) final override {
        m_data.Append(data, size);
        return BE_SQLITE_OK;
    }
    ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override {
        BeAssert(false);
        return ChangeSet::ConflictResolution::Abort;
    }

    ChangeSet(ChangeSet&& other) { *this = std::move(other); }
    ChangeSet& operator=(ChangeSet&& other) {
        m_data = std::move(other.m_data);
        return *this;
    }

    //! construct a blank, empty ChangeSet
    ChangeSet(int chunkSize = 64 * 1024) : m_data(chunkSize) {}
    virtual ~ChangeSet() { Clear(); }

    BE_SQLITE_EXPORT DbResult Invert();
    BE_SQLITE_EXPORT DbResult ConcatenateWith(ChangeSet const& second);

    //! Determine whether this ChangeSet holds valid data or not.
    bool IsValid() const { return 0 != GetSize(); }
    bool _IsEmpty() const override final { return 0 == GetSize(); }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DdlChanges : ChangeSet {

    void Append(Utf8CP ddl) {
        if (ddl)
            _Append((Byte const*)ddl, (int)strlen(ddl));
    }

    //! Create a new ddl change set
    DdlChanges(Utf8CP ddl = nullptr, int chunkSize = 4 * 1024) : ChangeSet(chunkSize) { Append(ddl); }

    //! Add new DDL statements to the schema change set (separate multiple commands by ';')
    BE_SQLITE_EXPORT void AddDDL(Utf8CP ddl);

    //! Return the contents of the schema change set
    BE_SQLITE_EXPORT Utf8String ToString() const;

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
// @bsiclass
//=======================================================================================
struct ChangeTracker : RefCountedBase {
    friend struct Db;
    friend struct DbFile;

private:
    DbResult RecordDbSchemaChange(Utf8CP ddl);
    mutable int m_enableChangesetSizeStats;
protected:
    DdlChanges m_ddlChanges;
    bool m_isTracking;
    bool m_hasEcSchemaChanges = false;
    Db* m_db;
    SqlSessionP m_session;
    Utf8String m_name;

    enum class OnCommitStatus { Commit = 0, Abort=1, Completed=2, NoChanges=3 };
    enum class TrackChangesForTable : bool { No = 0, Yes = 1 };

    BE_SQLITE_EXPORT DbResult CreateSession();
    virtual OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) = 0;
    virtual void _OnCommitted(bool isCommit, Utf8CP operation) {}
    void SetDb(Db* db) { m_db = db; }
    Db* GetDb() { return m_db; }
    Utf8CP GetName() const { return m_name.c_str(); }

public:
    ChangeTracker(Utf8CP name = NULL) : m_name(name), m_enableChangesetSizeStats(0) {
        m_session = 0;
        m_db = 0;
        m_isTracking = false;
    }
    virtual ~ChangeTracker() { EndTracking(); }

    virtual TrackChangesForTable _FilterTable(Utf8CP tableName) { return TrackChangesForTable::Yes; }
    SqlSessionP GetSqlSession() { return m_session; }

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
    enum class Mode {
        Direct = 0,   //!< All changes are marked as "direct".
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

    //! Return An Upper-limit For The Size Of The Changeset.
    //! By default return 0 unless EnableChangesetSizeStats() is used to enable it.
    BE_SQLITE_EXPORT int64_t GetChangesetSize() const;

    //! Configure tracker to keep stats about maximum size of changeset.
    //! Its disable by default.
    BE_SQLITE_EXPORT DbResult EnableChangesetSizeStats(bool) const;

    /* Return current memory allocated by session extension. This can be use to how much tracking information is stored in session extension.
     * This is not same as memory used by a changeset when created. Session extension has following behaviour for each type DML operation.
     *  DELETE: All deletes are recorded with all the values in session extension.
     *  UPDATE: Only old values are recored in session extension.
     *  INSERT: Only primary key is recored in session extension.
     * When changeset is created the new values for Update and Insert are taken from db so actual size of changeset might be very different from session extension.
    **/
    BE_SQLITE_EXPORT int64_t GetMemoryUsed() const;

    //! Determine whether any ddl changes have been tracked by this ChangeTracker
    bool HasDdlChanges() const { return !m_ddlChanges._IsEmpty(); }

    //! Clear the contents of this ChangeTracker and re-start it.
    void Restart() {
        EndTracking();
        EnableTracking(true);
    }
    bool IsTracking() const { return m_isTracking; }

    bool HasEcSchemaChanges() const { return m_hasEcSchemaChanges; }
    void SetHasEcSchemaChanges(bool val) {m_hasEcSchemaChanges = val;}
};

END_BENTLEY_SQLITE_NAMESPACE
