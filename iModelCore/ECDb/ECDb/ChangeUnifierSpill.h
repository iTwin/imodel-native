/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbApi.h>
#include <queue>
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
// Encodes rendered JSON values into owned bytes and decodes them as IInstanceWriter events.
// @remarks Rendering once through ECSqlRowAdaptor into a BeJsDocument and storing the result
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
// External sort storage of the ChangeUnifier: sorted runs of entries written to one temporary
// file, read back as a single stream in key order (k-way merge).
// @remarks Entries with equal keys are returned in the order of the runs that contain them, i.e.
// in change order, so that the caller can fold them with its regular merge.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct UnifierSpillFile final {
private:
    //! Buffered reader over one run of the file.
    struct Cursor final {
        uint64_t m_filePos = 0;
        uint64_t m_fileEnd = 0;
        std::vector<Byte> m_buffer;
        size_t m_bufferPos = 0;
        bool m_hasCurrent = false;
        KeyedEntry m_current;
    };

    using HeapItem = std::pair<UnifiedKey, size_t>; // key, cursor index (earlier runs win ties)
    struct HeapGreater final {
        bool operator()(HeapItem const& lhs, HeapItem const& rhs) const {
            if (lhs.first != rhs.first)
                return rhs.first < lhs.first;
            return lhs.second > rhs.second;
        }
    };

    Utf8String m_fileNameBase; //!< the spill file is created as <base>-<guid>-unifier.spill
    BeFileName m_fileName;
    BeFile m_file;
    uint64_t m_fileSize = 0;
    std::vector<Cursor> m_cursors; //!< one per run
    std::priority_queue<HeapItem, std::vector<HeapItem>, HeapGreater> m_heap;
    std::vector<Byte> m_writeBuffer;
    size_t m_readChunkBytes = 0; //!< bytes read per refill of a cursor, set by StartMerge

    bool Fill(Cursor& cursor, size_t need);
    BentleyStatus Advance(Cursor& cursor);

public:
    UnifierSpillFile() {}
    ~UnifierSpillFile() { Delete(); }
    UnifierSpillFile(UnifierSpillFile const&) = delete;
    UnifierSpillFile& operator=(UnifierSpillFile const&) = delete;

    bool HasRuns() const { return !m_cursors.empty(); }

    //! Sets where the spill file is created, e.g. ECDb::GetTempFileBaseName(). Call before the first WriteRun.
    void SetFileNameBase(Utf8StringCR base) { m_fileNameBase = base; }

    //! Appends @p sorted as a new run. The file is created on the first call.
    //! @return BE_SQLITE_OK, BE_SQLITE_CANTOPEN or BE_SQLITE_IOERR with @p err set.
    DbResult WriteRun(std::vector<KeyedEntry> const& sorted, Utf8StringR err);

    //! Positions every run on its first entry. Call once, after the last WriteRun.
    //! The read buffers of all runs share @p memoryBudgetBytes, within fixed per-run bounds.
    BentleyStatus StartMerge(uint64_t memoryBudgetBytes, Utf8StringR err);

    //! Key of the next entry in merge order, or nullptr when all runs are exhausted.
    UnifiedKey const* PeekKey() const { return m_heap.empty() ? nullptr : &m_heap.top().first; }

    //! Swaps the next entry in merge order into @p out. The old content of @p out is reused to read the
    //! next record, so passing the same object every time avoids allocations. Requires PeekKey() != nullptr.
    BentleyStatus Pop(KeyedEntry& out, Utf8StringR err);

    //! Closes and deletes the file. Idempotent.
    void Delete();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
