/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once

#include <cstddef>
#include <functional>

#include "BeSQLite.h"

BESQLITE_TYPEDEFS(ChangeGroup);
BESQLITE_TYPEDEFS(ChangeSet);
BESQLITE_TYPEDEFS(ChangeStream);
BESQLITE_TYPEDEFS(DdlChanges);
BESQLITE_TYPEDEFS(ApplyChangesArgs);

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
        friend struct ChangeGroup;
        using iterator_category = std::input_iterator_tag;
        using value_type = Change const;
        using difference_type = std::ptrdiff_t;
        using pointer = Change const*;
        using reference = Change const&;
        struct ArrayView {
           private:
            Byte const* m_data;
            int m_size;

           public:
            ArrayView(Byte const* data, int size) : m_data(data), m_size(size) {}
            bool operator[](int i) const {
                if (i < 0 || i >= m_size)
                    return false;
                if (m_data == nullptr)
                    return false;
                return !m_data[i];
            };
            int Length() const { return m_size; }
        };

       private:
        bool m_isValid;
        mutable SqlChangesetIterP m_iter;
        mutable Utf8String m_tableName;
        mutable DbOpcode m_opcode;
        mutable int m_indirect;
        mutable int m_nCols;
        mutable Byte* m_primaryKeyColumns;
        mutable int m_primaryKeyColumnsCount;
        mutable int m_foreignKeyConflicts;
        Utf8String FormatChange(Db const& db, Utf8CP tableName, DbOpcode opcode, int indirect, int detailLevel) const;
        void LoadOperation() const;

       public:
        BE_SQLITE_EXPORT Change(SqlChangesetIterP iter, bool isValid);
        Utf8StringCR GetTableName() const { return m_tableName; }
        DbOpcode GetOpcode() const { return m_opcode; }
        bool IsDirect() const { return !m_indirect; }
        bool IsUpdate() const { return m_opcode == DbOpcode::Update; }
        bool IsInsert() const { return m_opcode == DbOpcode::Insert; }
        bool IsDelete() const { return m_opcode == DbOpcode::Delete; }
        bool IsIndirect() const { return m_indirect; }
        int GetColumnCount() const { return m_nCols; }
        int GetForeignKeyConflicts() const { return m_foreignKeyConflicts; }
        int GetPrimaryKeyColumnCount() const { return m_primaryKeyColumnsCount; }
        BE_SQLITE_EXPORT bool IsPrimaryKeyColumn(int colNum) const;

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

        enum class Stage : bool { Old = 0,
                                  New = 1 };
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
    BE_SQLITE_EXPORT ChangeGroup(DbCR, Utf8CP zDb = "main");
    /**
     * @brief Adds a change to the change group.
     *
     * This function adds a change to the change group.
     *
     * @param change The change to be added.
     * @return The result of the operation.
     */
    BE_SQLITE_EXPORT DbResult AddChange(Changes::Change const& change);
    /**
     * Filters the given change stream based on the provided filter function and populates the ifChangeGroup with the filtered changes.
     *
     * @param in The input change stream to be filtered.
     * @param filter The filter function that determines whether a change should be included in the filtered result.
     * @param ifChangeGroup The output change group that will contain the filtered changes.
     * @return The database result indicating the success or failure of the filtering operation.
     */
    BE_SQLITE_EXPORT static DbResult FilterIf(ChangeStreamR in, std::function<bool(Changes::Change const&)> filter, ChangeGroup& ifChangeGroup);

    /**
     * Filters the changes in the given change stream based on the provided filter function.
     * The filtered changes are then divided into two change groups: ifChangeGroup and elseChangeGroup.
     *
     * @param in The input change stream to filter.
     * @param filter The filter function used to determine if a change should be included in the filtered result.
     * @param ifChangeGroup The change group to store the filtered changes that pass the filter function.
     * @param elseChangeGroup The change group to store the filtered changes that do not pass the filter function.
     * @return The result of the filtering operation.
     */
    BE_SQLITE_EXPORT static DbResult FilterIfElse(ChangeStreamR in, std::function<bool(Changes::Change const&)> filter, ChangeGroup& ifChangeGroup, ChangeGroup& elseChangeGroup);
    BE_SQLITE_EXPORT void Finalize();
    ~ChangeGroup() { Finalize(); }
};

//=======================================================================================
//! A base class for a streaming version of the ChangeSet. ChangeSets require that their
//! entire contents be stored in large memory buffers. This streaming version is meant to
//! be used in low memory environments where it is required to handle very large Change Sets.
// @bsiclass
//=======================================================================================
struct ChangeStream : NonCopyableClass {
    friend struct Changes;
    friend struct ApplyChangesArgs;

    enum class SetType : bool { Full = 0,
                                Patch = 1 };
    enum class FilterChangeAction : bool { Accept = 1,
                                           Skip = 0 };
    enum class ConflictCause : int { Data = 1,
                                     NotFound = 2,
                                     Conflict = 3,
                                     Constraint = 4,
                                     ForeignKey = 5 };
    enum class ConflictResolution : int { Skip = 0,
                                          Replace = 1,
                                          Abort = 2 };

   protected:
    mutable ApplyChangesArgs const* m_args = nullptr;
    static int ConflictCallback(void* pCtx, int cause, SqlChangesetIterP iter);
    static int FilterChangeCallback(void* pCtx, SqlChangesetIterP iter);
    virtual FilterChangeAction _FilterChange(Changes::Change const&) { return FilterChangeAction::Accept; }
    virtual ConflictResolution _OnConflict(ConflictCause clause, Changes::Change iter) = 0;

    //! Application implements this to receive data from the system.
    //! @param[in] pData Points to a buffer containing the output data
    //! @param[in] nData Size of buffer
    //! @return BE_SQLITE_OK if the data has been successfully processed. Return BE_SQLITE_ERROR otherwise.
    virtual DbResult _Append(Byte const* pData, int nData) = 0;
    static int AppendCallback(void* pOut, const void* pData, int nData) { return (int)((ChangeStream*)pOut)->_Append((Byte const*)pData, nData); }

   public:
    virtual bool _IsEmpty() const = 0;
    virtual ~ChangeStream() {}
    Changes GetChanges(bool invert = false) { return Changes(*this, invert); }
    BE_SQLITE_EXPORT DbResult FromChangeTrack(ChangeTracker& tracker, SetType setType = SetType::Full);
    BE_SQLITE_EXPORT DbResult FromChangeGroup(ChangeGroupCR changeGroup);
    BE_SQLITE_EXPORT DbResult ApplyChanges(DbR db, bool invert = false, bool ignoreNoop = false, bool fkNoAction = false) const;
    BE_SQLITE_EXPORT DbResult ApplyChanges(DbR db, ApplyChangesArgs const& args) const;
    BE_SQLITE_EXPORT DbResult ReadFrom(Changes::Reader& reader);
    BE_SQLITE_EXPORT DbResult InvertFrom(Changes::Reader& reader);

    //! Implement to handle conflicts when applying changes
    //! @see ApplyChanges
    FilterChangeAction FilterChange(Changes::Change const& change) { return _FilterChange(change); }

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
// @bsiclass
//=======================================================================================
struct ApplyChangesArgs {
    friend struct ChangeStream;

   private:
    bool m_invert;
    bool m_ignoreNoop;
    bool m_fkNoAction;
    bool m_abortOnAnyConflict;
    mutable int64_t m_filterRowCount;
    mutable int64_t m_conflictRowCount;
    std::function<ChangeStream::FilterChangeAction(Changes::Change const&)> m_filterChange;
    std::function<ChangeStream::ConflictResolution(ChangeStream::ConflictCause, Changes::Change)> m_conflictHandler;

   protected:
    static int ConflictCallback(void*, int, SqlChangesetIterP);
    static int FilterChangeCallback(void*, SqlChangesetIterP);
    ChangeStream::FilterChangeAction FilterChange(Changes::Change const& change) const;
    ChangeStream::ConflictResolution OnConflict(ChangeStream::ConflictCause cause, Changes::Change iter) const;

   public:
    ApplyChangesArgs() : m_invert(false), m_ignoreNoop(false), m_fkNoAction(false), m_filterChange(nullptr), m_conflictHandler(nullptr), m_filterRowCount(0), m_conflictRowCount(0), m_abortOnAnyConflict(false) {}
    ApplyChangesArgs& SetAbortOnAnyConflict(bool abortOnAnyConflict) {
        m_abortOnAnyConflict = abortOnAnyConflict;
        return *this;
    }
    ApplyChangesArgs& SetInvert(bool invert) {
        m_invert = invert;
        return *this;
    }
    ApplyChangesArgs& SetIgnoreNoop(bool ignoreNoop) {
        m_ignoreNoop = ignoreNoop;
        return *this;
    }
    ApplyChangesArgs& SetFkNoAction(bool fkNoAction) {
        m_fkNoAction = fkNoAction;
        return *this;
    }
    ApplyChangesArgs& SetFilterChange(std::function<ChangeStream::FilterChangeAction(Changes::Change const&)> filterChange) {
        m_filterChange = filterChange;
        return *this;
    }
    BE_SQLITE_EXPORT ApplyChangesArgs& ApplyOnlySchemaChanges();
    BE_SQLITE_EXPORT ApplyChangesArgs& ApplyOnlyDataChanges();
    ApplyChangesArgs& ApplyAnyChanges() {
        m_filterChange = nullptr;
        return *this;
    }
    ApplyChangesArgs& SetConflictHandler(std::function<ChangeStream::ConflictResolution(ChangeStream::ConflictCause, Changes::Change)> conflictHandler) {
        m_conflictHandler = conflictHandler;
        return *this;
    }
    bool GetInvert() const { return m_invert; }
    bool GetAbortOnAnyConflict() const { return m_abortOnAnyConflict; }
    bool GetIgnoreNoop() const { return m_ignoreNoop; }
    bool GetFkNoAction() const { return m_fkNoAction; }
    int64_t GetFilterRowCount() const { return m_filterRowCount; }
    int64_t GetConflictRowCount() const { return m_conflictRowCount; }
    bool HasFilterChange() const { return m_filterChange != nullptr; }
    bool HasConflictHandler() const { return m_conflictHandler != nullptr; }
    static ApplyChangesArgs Default() { return ApplyChangesArgs(); }
    BE_SQLITE_EXPORT static bool IsSchemaTable(Utf8CP tableName);
    BE_SQLITE_EXPORT static bool IsSchemaChange(Changes::Change const& change);
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
    // For debugging sqlite issues
    BE_SQLITE_EXPORT DbResult Write(Utf8StringCR pathname) const;
    // For debugging sqlite issues
    BE_SQLITE_EXPORT DbResult Read(Utf8StringCR pathname);

    //! Determine whether this ChangeSet holds valid data or not.
    bool IsValid() const { return 0 != GetSize(); }
    bool _IsEmpty() const override final { return 0 == GetSize(); }
};

//=======================================================================================
// @bsiclass
// For debugging sqlite issues
//=======================================================================================
struct ChangesetFile : ChangeStream {
    Utf8String m_fileName;
    BeFile m_file;
    struct Reader : Changes::Reader {
        BeFile m_file;
        Reader(ChangesetFile const& changeSet) {
            m_file.Open(changeSet.m_fileName, BeFileAccess::Read);
        }
        DbResult _Read(Byte* data, int* pSize) override final {
            auto sz = (uint32_t)(*pSize);
            return m_file.Read(data, &sz, sz) == BeFileStatus::Success ? BE_SQLITE_OK : BE_SQLITE_ERROR;
        }
    };

    RefCountedPtr<Changes::Reader> _GetReader() const override final { return new Reader(*this); }
    ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override {
        BeAssert(false);
        return ChangeSet::ConflictResolution::Abort;
    }

    bool IsValid() const { return 0 != GetSize(); }
    bool _IsEmpty() const override final { return 0 == GetSize(); }

    BE_SQLITE_EXPORT size_t GetSize() const;
    BE_SQLITE_EXPORT DbResult _Append(Byte const* data, int size) final override;
    BE_SQLITE_EXPORT ChangesetFile(Utf8String name);
    BE_SQLITE_EXPORT virtual ~ChangesetFile();
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

    //! Return the contents of the schema change set
    BE_SQLITE_EXPORT bvector<Utf8String> GetDDLs() const;

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
    Db* m_db;
    SqlSessionP m_session;
    Utf8String m_name;

    enum class OnCommitStatus { Commit = 0,
                                Abort = 1,
                                Completed = 2,
                                NoChanges = 3,
                                RebaseInProgress = 4,
                                PropagateChangesFailed = 5 };
    enum class TrackChangesForTable : bool { No = 0,
                                             Yes = 1 };

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
        Direct = 0,    //!< All changes are marked as "direct".
        Indirect = 1,  //!< All changes are marked as "indirect".
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
};

END_BENTLEY_SQLITE_NAMESPACE
