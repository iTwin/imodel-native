/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbApi.h>
#include <memory>
#include <vector>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// Merge key of the ChangeUnifier. Sorted numerically by root class id, then ECInstanceId,
// then stage (Old=0 < New=1).
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct UnifiedKey final {
    uint64_t m_rootClassId = 0;
    uint64_t m_instanceId = 0;
    uint8_t m_stage = 0;

    bool operator==(UnifiedKey const& rhs) const { return m_rootClassId == rhs.m_rootClassId && m_instanceId == rhs.m_instanceId && m_stage == rhs.m_stage; }
    bool operator!=(UnifiedKey const& rhs) const { return !(*this == rhs); }
    bool operator<(UnifiedKey const& rhs) const {
        if (m_rootClassId != rhs.m_rootClassId)
            return m_rootClassId < rhs.m_rootClassId;
        if (m_instanceId != rhs.m_instanceId)
            return m_instanceId < rhs.m_instanceId;
        return m_stage < rhs.m_stage;
    }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct UnifiedKeyHash final {
    size_t operator()(UnifiedKey const& key) const {
        uint64_t h = key.m_instanceId * 0x9E3779B97F4A7C15ull;
        h ^= (key.m_rootClassId + 0x632BE59BD9B4E019ull + (h << 6) + (h >> 2));
        h ^= (static_cast<uint64_t>(key.m_stage) + 0x85EBCA77C2B2AE63ull + (h << 6) + (h >> 2));
        return static_cast<size_t>(h ^ (h >> 32));
    }
};

//=======================================================================================
// A rendered property value in the tagged binary encoding of UnifiedValueCodec.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct UnifiedProp final {
    uint32_t m_nameId = 0;
    bool m_isClassId = false; //!< true if this is the rendered ECClassId system property
    std::vector<Byte> m_value;
};

//=======================================================================================
// Merged data of one instance and stage. Strings (table names, property names, fetched
// property names) are interned ids so that entries are small and trivially serializable.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct UnifiedEntry final {
    enum class Op : uint8_t { None = 0, Inserted = 1, Updated = 2, Deleted = 3 };

    uint64_t m_classId = 0; //!< most derived ECClassId seen so far
    Op m_op = Op::None; //!< op of the first main-table row, None if no main-table row was seen
    bool m_isIndirect = false; //!< from the first main-table row, or from the first row as long as no main-table row was seen
    std::vector<uint32_t> m_tables;
    std::vector<uint32_t> m_changeIndexes;
    std::vector<uint32_t> m_fetchedNames;
    std::vector<UnifiedProp> m_props;

    bool HasMainRow() const { return m_op != Op::None; }
};

using KeyedEntry = std::pair<UnifiedKey, UnifiedEntry>;

//=======================================================================================
// A JSON-like value that ECSqlRowAdaptor renders one property into, through BeJsValue.
// @remarks Unlike a BeJsDocument, it keeps binary data as binary instead of a string with the
// "encoding=base64;" header, so a string value that starts with that header stays a string.
// Clear() releases the memory of the previous value.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct RenderedValue final {
    struct Node;

private:
    std::unique_ptr<Node> m_root;

public:
    RenderedValue();
    ~RenderedValue();
    RenderedValue(RenderedValue const&) = delete;
    RenderedValue& operator=(RenderedValue const&) = delete;

    //! The root value, to render into and to read back.
    BeJsValue Root();
    void Clear();
};

//=======================================================================================
// Encodes rendered JSON values into owned bytes and decodes them as IInstanceWriter events.
// @remarks Rendering once through ECSqlRowAdaptor into a RenderedValue and storing the result
// as tagged bytes keeps the unifier independent of Napi, gives an exact memory estimate, and
// the bytes can be written to spill files as-is. Decoding emits the same value kinds that
// ECSqlRowAdaptor produced (int64 stays int64, blobs stay binary, ...), so a writer can rebuild
// exactly the value a ChangesetReader row would have.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct UnifiedValueCodec final {
    static void Encode(std::vector<Byte>& out, BeJsConst val);
    static BentleyStatus Decode(std::vector<Byte> const& in, ChangeUnifier::IInstanceWriter& out);
};

//=======================================================================================
// External sort storage of the ChangeUnifier: each sorted run of entries is inserted into a SQLite temporary
// database, in a WITHOUT ROWID table whose primary key is (root class id, ECInstanceId, stage, run). The rows
// are stored in primary key order, so they are read back in key order without a sort.
// @remarks The database is created with Db::CreateNewDb(nullptr), so SQLite places it in its temp directory and
// deletes it when it is closed, or when the process ends. Entries with equal keys are returned in the order of
// the runs that contain them, i.e. in change order, so that the caller can fold them with its regular merge.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct UnifierSpillFile final {
private:
    Db m_db;
    Statement m_insert;
    Statement m_select;
    uint32_t m_runCount = 0;
    bool m_hasNext = false; //!< m_select is positioned on a row
    UnifiedKey m_nextKey;
    std::vector<Byte> m_writeBuffer;

    DbResult Open(Utf8StringR err);
    BentleyStatus StepSelect(Utf8StringR err);

public:
    UnifierSpillFile() {}
    ~UnifierSpillFile() { Delete(); }
    UnifierSpillFile(UnifierSpillFile const&) = delete;
    UnifierSpillFile& operator=(UnifierSpillFile const&) = delete;

    bool HasRuns() const { return m_runCount > 0; }

    //! Inserts @p sorted as a new run, in one transaction. The database is created on the first call.
    //! @return BE_SQLITE_OK, or the SQLite error with @p err set.
    DbResult WriteRun(std::vector<KeyedEntry> const& sorted, Utf8StringR err);

    //! Starts reading all runs in key order. Call once, after the last WriteRun. SQLite's page cache, not
    //! @p memoryBudgetBytes, bounds the memory used to read the runs back.
    BentleyStatus StartMerge(uint64_t memoryBudgetBytes, Utf8StringR err);

    //! Key of the next entry in merge order, or nullptr when all runs are exhausted.
    UnifiedKey const* PeekKey() const { return m_hasNext ? &m_nextKey : nullptr; }

    //! Reads the next entry in merge order into @p out, reusing the capacity of its vectors. Requires PeekKey() != nullptr.
    BentleyStatus Pop(KeyedEntry& out, Utf8StringR err);

    //! Closes the database, which deletes it. Idempotent.
    void Delete();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
