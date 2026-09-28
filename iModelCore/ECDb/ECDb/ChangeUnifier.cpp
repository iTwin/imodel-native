/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <cstring>
#include <queue>
#include <unordered_map>
#include <unordered_set>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

namespace {

//=======================================================================================
// Merge key. Sorted numerically by root class id, then ECInstanceId, then stage (Old=0 < New=1).
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
// A rendered property value in a compact tagged binary encoding (see ValueCodec).
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
    bool m_hasMainRow = false;
    bool m_isIndirect = false; //!< from the first main-table row, or from the first row as long as no main-table row was seen
    std::vector<uint32_t> m_tables;
    std::vector<uint32_t> m_changeIndexes;
    std::vector<uint32_t> m_fetchedNames;
    std::vector<UnifiedProp> m_props;
};

using UnifiedEntryMap = std::unordered_map<UnifiedKey, UnifiedEntry, UnifiedKeyHash>;
using KeyedEntry = std::pair<UnifiedKey, UnifiedEntry>;

//=======================================================================================
// Byte-level helpers.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ByteWriter final {
    static void PutByte(std::vector<Byte>& out, Byte b) { out.push_back(b); }
    static void PutVarint(std::vector<Byte>& out, uint64_t v) {
        while (v >= 0x80) {
            out.push_back(static_cast<Byte>(v | 0x80));
            v >>= 7;
        }
        out.push_back(static_cast<Byte>(v));
    }
    static void PutFixed64(std::vector<Byte>& out, uint64_t v) {
        for (int i = 0; i < 8; ++i)
            out.push_back(static_cast<Byte>(v >> (8 * i)));
    }
    static void PutFixed32(std::vector<Byte>& out, uint32_t v) {
        for (int i = 0; i < 4; ++i)
            out.push_back(static_cast<Byte>(v >> (8 * i)));
    }
    static void PatchFixed32(std::vector<Byte>& out, size_t pos, uint32_t v) {
        for (int i = 0; i < 4; ++i)
            out[pos + i] = static_cast<Byte>(v >> (8 * i));
    }
    static void PutBytes(std::vector<Byte>& out, void const* data, size_t size) {
        PutVarint(out, size);
        if (size > 0) {
            Byte const* p = static_cast<Byte const*>(data);
            out.insert(out.end(), p, p + size);
        }
    }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ByteReader final {
private:
    Byte const* m_pos;
    Byte const* m_end;
    bool m_ok = true;

public:
    ByteReader(Byte const* data, size_t size) : m_pos(data), m_end(data + size) {}
    bool IsOk() const { return m_ok; }
    bool AtEnd() const { return m_pos >= m_end; }
    Byte GetByte() {
        if (m_pos >= m_end) { m_ok = false; return 0; }
        return *m_pos++;
    }
    uint64_t GetVarint() {
        uint64_t v = 0;
        for (int shift = 0; shift < 64; shift += 7) {
            if (m_pos >= m_end) { m_ok = false; return 0; }
            Byte b = *m_pos++;
            v |= static_cast<uint64_t>(b & 0x7F) << shift;
            if ((b & 0x80) == 0)
                return v;
        }
        m_ok = false;
        return 0;
    }
    uint64_t GetFixed64() {
        if (m_end - m_pos < 8) { m_ok = false; m_pos = m_end; return 0; }
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i)
            v |= static_cast<uint64_t>(m_pos[i]) << (8 * i);
        m_pos += 8;
        return v;
    }
    uint32_t GetFixed32() {
        if (m_end - m_pos < 4) { m_ok = false; m_pos = m_end; return 0; }
        uint32_t v = 0;
        for (int i = 0; i < 4; ++i)
            v |= static_cast<uint32_t>(m_pos[i]) << (8 * i);
        m_pos += 4;
        return v;
    }
    //! Returns a pointer to @p size bytes and advances, or nullptr if not enough bytes are left.
    Byte const* GetRaw(size_t size) {
        if (static_cast<size_t>(m_end - m_pos) < size) { m_ok = false; m_pos = m_end; return nullptr; }
        Byte const* p = m_pos;
        m_pos += size;
        return p;
    }
};

//=======================================================================================
// Encodes rendered JSON values into owned bytes and decodes them into any BeJsValue.
// @remarks Rendering once through ECSqlRowAdaptor into a BeJsDocument and storing the result
// as tagged bytes keeps the unifier independent of Napi, gives an exact memory estimate, and
// the bytes can be written to spill files as-is. Decoding writes through the same BeJsValue
// setters ECSqlRowAdaptor uses, so the resulting JavaScript values are identical to rendering
// the row directly (int64 stays int64, blobs become Uint8Array, ...).
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ValueCodec final {
    enum class Tag : Byte { Null = 0, False = 1, True = 2, Int64 = 3, UInt64 = 4, Double = 5, String = 6, Binary = 7, Array = 8, Object = 9 };

    static void Encode(std::vector<Byte>& out, BeJsConst val) {
        if (val.isNull()) {
            ByteWriter::PutByte(out, (Byte) Tag::Null);
            return;
        }
        if (val.isBool()) {
            ByteWriter::PutByte(out, (Byte) (val.asBool() ? Tag::True : Tag::False));
            return;
        }
        if (val.isNumeric()) {
            switch (val.GetNumericKind()) {
                case JsValueRef::NumericKind::Int64:
                    ByteWriter::PutByte(out, (Byte) Tag::Int64);
                    ByteWriter::PutFixed64(out, static_cast<uint64_t>(val.asInt64()));
                    return;
                case JsValueRef::NumericKind::UInt64:
                    ByteWriter::PutByte(out, (Byte) Tag::UInt64);
                    ByteWriter::PutFixed64(out, val.asUInt64());
                    return;
                default: {
                    double d = val.asDouble();
                    uint64_t bits;
                    static_assert(sizeof(bits) == sizeof(d), "unexpected double size");
                    memcpy(&bits, &d, sizeof(bits));
                    ByteWriter::PutByte(out, (Byte) Tag::Double);
                    ByteWriter::PutFixed64(out, bits);
                    return;
                }
            }
        }
        // isBinary must be checked before isString: a rapidjson document stores binary data as a base64 string with a header.
        if (val.isBinary()) {
            std::vector<Byte> data;
            val.GetBinary(data);
            ByteWriter::PutByte(out, (Byte) Tag::Binary);
            ByteWriter::PutBytes(out, data.data(), data.size());
            return;
        }
        if (val.isString()) {
            Utf8CP str = val.asCString();
            ByteWriter::PutByte(out, (Byte) Tag::String);
            ByteWriter::PutBytes(out, str, strlen(str));
            return;
        }
        if (val.isArray()) {
            ByteWriter::PutByte(out, (Byte) Tag::Array);
            size_t countPos = out.size();
            ByteWriter::PutFixed32(out, 0);
            uint32_t count = 0;
            val.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst member) {
                Encode(out, member);
                ++count;
                return false;
            });
            ByteWriter::PatchFixed32(out, countPos, count);
            return;
        }
        if (val.isObject()) {
            ByteWriter::PutByte(out, (Byte) Tag::Object);
            size_t countPos = out.size();
            ByteWriter::PutFixed32(out, 0);
            uint32_t count = 0;
            val.ForEachProperty([&](Utf8CP name, BeJsConst member) {
                ByteWriter::PutBytes(out, name, strlen(name));
                Encode(out, member);
                ++count;
                return false;
            });
            ByteWriter::PatchFixed32(out, countPos, count);
            return;
        }
        ByteWriter::PutByte(out, (Byte) Tag::Null);
    }

    static BentleyStatus Decode(ByteReader& reader, BeJsValue out) {
        Tag tag = (Tag) reader.GetByte();
        if (!reader.IsOk())
            return ERROR;
        switch (tag) {
            case Tag::Null:
                out.SetNull();
                return SUCCESS;
            case Tag::False:
                out = false;
                return SUCCESS;
            case Tag::True:
                out = true;
                return SUCCESS;
            case Tag::Int64:
                out = static_cast<int64_t>(reader.GetFixed64());
                return reader.IsOk() ? SUCCESS : ERROR;
            case Tag::UInt64:
                out = reader.GetFixed64();
                return reader.IsOk() ? SUCCESS : ERROR;
            case Tag::Double: {
                uint64_t bits = reader.GetFixed64();
                double d;
                memcpy(&d, &bits, sizeof(d));
                out = d;
                return reader.IsOk() ? SUCCESS : ERROR;
            }
            case Tag::String: {
                size_t len = static_cast<size_t>(reader.GetVarint());
                Byte const* p = reader.GetRaw(len);
                if (!reader.IsOk())
                    return ERROR;
                Utf8String str(reinterpret_cast<Utf8CP>(p), len);
                out = str.c_str();
                return SUCCESS;
            }
            case Tag::Binary: {
                size_t len = static_cast<size_t>(reader.GetVarint());
                Byte const* p = reader.GetRaw(len);
                if (!reader.IsOk())
                    return ERROR;
                out.SetBinary(p, len);
                return SUCCESS;
            }
            case Tag::Array: {
                uint32_t count = reader.GetFixed32();
                if (!reader.IsOk())
                    return ERROR;
                out.SetEmptyArray();
                for (uint32_t i = 0; i < count; ++i) {
                    if (SUCCESS != Decode(reader, out.appendValue()))
                        return ERROR;
                }
                return SUCCESS;
            }
            case Tag::Object: {
                uint32_t count = reader.GetFixed32();
                if (!reader.IsOk())
                    return ERROR;
                out.SetEmptyObject();
                for (uint32_t i = 0; i < count; ++i) {
                    size_t len = static_cast<size_t>(reader.GetVarint());
                    Byte const* p = reader.GetRaw(len);
                    if (!reader.IsOk())
                        return ERROR;
                    Utf8String name(reinterpret_cast<Utf8CP>(p), len);
                    if (SUCCESS != Decode(reader, out[name.c_str()]))
                        return ERROR;
                }
                return SUCCESS;
            }
        }
        return ERROR;
    }
};

//=======================================================================================
// IECSqlRow over a single value so that ECSqlRowAdaptor renders one column at a time.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SingleValueRow final : IECSqlRow {
private:
    IECSqlValue const& m_value;
public:
    explicit SingleValueRow(IECSqlValue const& value) : m_value(value) {}
    int GetColumnCount() const override { return 1; }
    IECSqlValue const& GetValue(int) const override { return m_value; }
};

//=======================================================================================
// Serialization of entries into spill-run records: [uint32 length][payload].
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct EntrySerializer final {
    static void Write(std::vector<Byte>& out, UnifiedKey const& key, UnifiedEntry const& entry) {
        size_t lenPos = out.size();
        ByteWriter::PutFixed32(out, 0);
        size_t start = out.size();
        ByteWriter::PutFixed64(out, key.m_rootClassId);
        ByteWriter::PutFixed64(out, key.m_instanceId);
        ByteWriter::PutByte(out, key.m_stage);
        ByteWriter::PutFixed64(out, entry.m_classId);
        ByteWriter::PutByte(out, (Byte) entry.m_op);
        ByteWriter::PutByte(out, entry.m_hasMainRow ? 1 : 0);
        ByteWriter::PutByte(out, entry.m_isIndirect ? 1 : 0);
        auto putIds = [&](std::vector<uint32_t> const& ids) {
            ByteWriter::PutVarint(out, ids.size());
            for (uint32_t id : ids)
                ByteWriter::PutVarint(out, id);
        };
        putIds(entry.m_tables);
        putIds(entry.m_changeIndexes);
        putIds(entry.m_fetchedNames);
        ByteWriter::PutVarint(out, entry.m_props.size());
        for (auto const& prop : entry.m_props) {
            ByteWriter::PutVarint(out, prop.m_nameId);
            ByteWriter::PutByte(out, prop.m_isClassId ? 1 : 0);
            ByteWriter::PutBytes(out, prop.m_value.data(), prop.m_value.size());
        }
        ByteWriter::PatchFixed32(out, lenPos, static_cast<uint32_t>(out.size() - start));
    }

    static BentleyStatus Read(Byte const* data, size_t size, UnifiedKey& key, UnifiedEntry& entry) {
        ByteReader reader(data, size);
        key.m_rootClassId = reader.GetFixed64();
        key.m_instanceId = reader.GetFixed64();
        key.m_stage = reader.GetByte();
        entry = UnifiedEntry();
        entry.m_classId = reader.GetFixed64();
        entry.m_op = (UnifiedEntry::Op) reader.GetByte();
        entry.m_hasMainRow = reader.GetByte() != 0;
        entry.m_isIndirect = reader.GetByte() != 0;
        auto getIds = [&](std::vector<uint32_t>& ids) {
            uint64_t count = reader.GetVarint();
            if (!reader.IsOk() || count > size)
                return false;
            ids.reserve(static_cast<size_t>(count));
            for (uint64_t i = 0; i < count && reader.IsOk(); ++i)
                ids.push_back(static_cast<uint32_t>(reader.GetVarint()));
            return reader.IsOk();
        };
        if (!getIds(entry.m_tables) || !getIds(entry.m_changeIndexes) || !getIds(entry.m_fetchedNames))
            return ERROR;
        uint64_t propCount = reader.GetVarint();
        if (!reader.IsOk() || propCount > size)
            return ERROR;
        entry.m_props.resize(static_cast<size_t>(propCount));
        for (auto& prop : entry.m_props) {
            prop.m_nameId = static_cast<uint32_t>(reader.GetVarint());
            prop.m_isClassId = reader.GetByte() != 0;
            size_t len = static_cast<size_t>(reader.GetVarint());
            Byte const* p = reader.GetRaw(len);
            if (!reader.IsOk())
                return ERROR;
            prop.m_value.assign(p, p + len);
        }
        return reader.IsOk() ? SUCCESS : ERROR;
    }
};

//=======================================================================================
// Location of one sorted run inside the spill file.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SpillRun final {
    uint64_t m_offset = 0;
    uint64_t m_length = 0;
};

} // namespace

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangeUnifierImpl final {
private:
    enum class State { Appending, Stepping, Done, Closed, Failed };

    //=======================================================================================
    // Buffered reader over one run of the spill file, or over the in-memory remainder.
    // @bsiclass
    //+===============+===============+===============+===============+===============+======
    struct Source final {
        // spilled run
        uint64_t m_filePos = 0;
        uint64_t m_fileEnd = 0;
        std::vector<Byte> m_buffer;
        size_t m_bufferPos = 0;
        // in-memory remainder
        bool m_isRemainder = false;
        size_t m_remainderPos = 0;
        // current record
        bool m_hasCurrent = false;
        UnifiedKey m_key;
        UnifiedEntry m_entry;
    };

    struct TableInfo final {
        uint64_t m_rootClassId = 0;
        bool m_isOverflow = false;
    };

    static constexpr size_t SpillWriteChunkBytes = 1024 * 1024;
    static constexpr size_t SpillReadChunkBytes = 64 * 1024;
    static constexpr uint64_t EntryOverheadBytes = sizeof(UnifiedKey) + sizeof(UnifiedEntry) + 32; // hash node + bucket
    static constexpr uint64_t PropOverheadBytes = sizeof(UnifiedProp);

    ChangeUnifier::Options m_options;
    bset<Utf8String, CompareIUtf8Ascii> m_keepProps;
    State m_state = State::Appending;
    Utf8String m_lastError;

    // interned strings, retained for the lifetime of the unifier (spill records reference them by id)
    std::vector<Utf8String> m_strings;
    std::unordered_map<Utf8String, uint32_t> m_stringIds;

    ECDb const* m_tableCacheDb = nullptr;
    std::unordered_map<Utf8String, TableInfo> m_tableCache;
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> m_classAncestors;

    UnifiedEntryMap m_entries;
    uint64_t m_estimatedBytes = 0;

    BeFileName m_spillFileName;
    BeFile m_spillFile;
    uint64_t m_spillFileSize = 0;
    std::vector<SpillRun> m_runs;

    std::vector<KeyedEntry> m_remainder;
    std::vector<Source> m_sources;
    using HeapItem = std::pair<UnifiedKey, size_t>; // key, source index (earlier sources win ties)
    struct HeapGreater final {
        bool operator()(HeapItem const& lhs, HeapItem const& rhs) const {
            if (lhs.first != rhs.first)
                return rhs.first < lhs.first;
            return lhs.second > rhs.second;
        }
    };
    std::priority_queue<HeapItem, std::vector<HeapItem>, HeapGreater> m_heap;

    // rapidjson never returns memory to its pool allocator, so the scratch document is recreated periodically
    static constexpr uint32_t ScratchReuseLimit = 4096;
    std::unique_ptr<BeJsDocument> m_scratch;
    uint32_t m_scratchUses = 0;
    std::vector<Byte> m_ioBuffer;

private:
    DbResult Fail(DbResult rc, Utf8StringCR msg) {
        m_lastError = msg;
        LOG.error(msg.c_str());
        m_state = State::Failed;
        return rc;
    }

    uint32_t Intern(Utf8StringCR str) {
        auto it = m_stringIds.find(str);
        if (it != m_stringIds.end())
            return it->second;
        uint32_t id = static_cast<uint32_t>(m_strings.size());
        m_strings.push_back(str);
        m_stringIds.insert(std::make_pair(str, id));
        return id;
    }

    bool KeepProperty(Utf8StringCR propName) const {
        return m_keepProps.empty() || m_keepProps.find(propName) != m_keepProps.end();
    }

    TableInfo const* GetTableInfo(ECDbCR ecdb, Utf8StringCR tableName, Utf8StringR err) {
        if (m_tableCacheDb != &ecdb) {
            m_tableCache.clear();
            m_tableCacheDb = &ecdb;
        }
        auto it = m_tableCache.find(tableName);
        if (it != m_tableCache.end())
            return &it->second;

        DbTable const* table = ecdb.Schemas().Main().GetDbSchema().FindTable(tableName);
        if (table == nullptr) {
            err.Sprintf("ChangeUnifier: table '%s' not found in the ECDb mapping.", tableName.c_str());
            return nullptr;
        }
        TableInfo info;
        info.m_isOverflow = table->GetType() == DbTable::Type::Overflow;
        DbTable const* primary = table;
        while (primary->GetLinkNode().GetParent() != nullptr)
            primary = &primary->GetLinkNode().GetParent()->GetTable();
        if (primary->HasExclusiveRootECClass())
            info.m_rootClassId = primary->GetExclusiveRootECClassId().GetValue();
        else if (table->HasExclusiveRootECClass()) {
            LOG.warningv("ChangeUnifier: primary table '%s' of '%s' has no exclusive root class. Using the root class of '%s'.", primary->GetName().c_str(), tableName.c_str(), tableName.c_str());
            info.m_rootClassId = table->GetExclusiveRootECClassId().GetValue();
        } else {
            LOG.warningv("ChangeUnifier: table '%s' has no exclusive root class. Using the class id of each row as merge key.", tableName.c_str());
            info.m_rootClassId = 0;
        }
        return &m_tableCache.insert(std::make_pair(tableName, info)).first->second;
    }

    void CollectAncestors(ECClassCR ecClass, std::unordered_set<uint64_t>& ancestors) {
        for (ECClassCP baseClass : ecClass.GetBaseClasses()) {
            if (baseClass == nullptr)
                continue;
            if (ancestors.insert(baseClass->GetId().GetValue()).second)
                CollectAncestors(*baseClass, ancestors);
        }
    }

    void EnsureAncestors(ECDbCR ecdb, uint64_t classId) {
        if (classId == 0 || m_classAncestors.find(classId) != m_classAncestors.end())
            return;
        std::unordered_set<uint64_t> ancestors;
        ECClassCP ecClass = ecdb.Schemas().GetClass(ECClassId(classId));
        if (ecClass != nullptr)
            CollectAncestors(*ecClass, ancestors);
        else
            LOG.warningv("ChangeUnifier: ECClass with id %" PRIu64 " not found.", classId);
        m_classAncestors.insert(std::make_pair(classId, std::move(ancestors)));
    }

    //! Most derived of two class ids. If neither derives from the other, the current one is kept.
    uint64_t MostDerived(uint64_t current, uint64_t candidate) const {
        if (current == 0 || current == candidate)
            return candidate;
        if (candidate == 0)
            return current;
        auto it = m_classAncestors.find(candidate);
        if (it != m_classAncestors.end() && it->second.find(current) != it->second.end())
            return candidate;
        return current;
    }

    static uint64_t EstimateBytes(UnifiedEntry const& entry) {
        uint64_t bytes = EntryOverheadBytes;
        bytes += sizeof(uint32_t) * (entry.m_tables.capacity() + entry.m_changeIndexes.capacity() + entry.m_fetchedNames.capacity());
        bytes += PropOverheadBytes * entry.m_props.capacity();
        for (auto const& prop : entry.m_props)
            bytes += prop.m_value.capacity();
        return bytes;
    }

    //! Merges @p rhs, which comes later in change order, into @p lhs.
    void MergeInto(UnifiedEntry& lhs, UnifiedEntry&& rhs) const {
        if (!lhs.m_hasMainRow && rhs.m_hasMainRow) {
            lhs.m_hasMainRow = true;
            lhs.m_op = rhs.m_op;
            lhs.m_isIndirect = rhs.m_isIndirect;
        }
        lhs.m_tables.insert(lhs.m_tables.end(), rhs.m_tables.begin(), rhs.m_tables.end());
        lhs.m_changeIndexes.insert(lhs.m_changeIndexes.end(), rhs.m_changeIndexes.begin(), rhs.m_changeIndexes.end());
        for (uint32_t name : rhs.m_fetchedNames) {
            if (std::find(lhs.m_fetchedNames.begin(), lhs.m_fetchedNames.end(), name) == lhs.m_fetchedNames.end())
                lhs.m_fetchedNames.push_back(name);
        }
        const uint64_t best = MostDerived(lhs.m_classId, rhs.m_classId);
        for (auto& prop : rhs.m_props) {
            if (prop.m_isClassId && rhs.m_classId != best)
                continue; // keep the ECClassId value of the most derived class
            auto it = std::find_if(lhs.m_props.begin(), lhs.m_props.end(), [&](UnifiedProp const& p) { return p.m_nameId == prop.m_nameId; });
            if (it == lhs.m_props.end())
                lhs.m_props.push_back(std::move(prop));
            else
                it->m_value = std::move(prop.m_value);
        }
        lhs.m_classId = best;
    }

    //! Builds the entry of one row stage. Returns ERROR with @p err set on failure.
    BentleyStatus BuildRowEntry(ChangesetReader const& reader, Changes::Change::Stage stage, ECSqlRowAdaptor const& adaptor, uint32_t tableId, uint32_t changeIndex,
        bool isMain, UnifiedEntry::Op op, bool isIndirect, std::vector<uint32_t> const& fetchedNames, UnifiedKey& key, UnifiedEntry& entry, Utf8StringR err) {
        entry = UnifiedEntry();
        entry.m_hasMainRow = isMain;
        entry.m_op = isMain ? op : UnifiedEntry::Op::None;
        entry.m_isIndirect = isIndirect;
        entry.m_tables.push_back(tableId);
        entry.m_changeIndexes.push_back(changeIndex);
        entry.m_fetchedNames = fetchedNames;

        bool hasInstanceId = false;
        bool hasClassId = false;
        const int count = reader.GetColumnCount(stage);
        for (int i = 0; i < count; ++i) {
            IECSqlValue const& val = reader.GetValue(stage, i);
            ECPropertyCP prop = val.GetColumnInfo().GetProperty();
            if (prop == nullptr)
                continue;
            Utf8StringCR propName = prop->GetName();
            bool isClassIdProp = false;
            PrimitiveECPropertyCP primProp = prop->GetAsPrimitiveProperty();
            if (primProp != nullptr && !val.IsNull()) {
                const auto extType = ExtendedTypeHelper::GetExtendedType(primProp->GetExtendedTypeName());
                if (extType == ExtendedTypeHelper::ExtendedType::Id && propName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId)) {
                    key.m_instanceId = val.GetId<ECInstanceId>().GetValueUnchecked();
                    hasInstanceId = true;
                } else if (extType == ExtendedTypeHelper::ExtendedType::ClassId && propName.EqualsIAscii(ECDBSYS_PROP_ECClassId)) {
                    entry.m_classId = val.GetId<ECClassId>().GetValueUnchecked();
                    hasClassId = true;
                    isClassIdProp = true;
                }
            }
            if (!isClassIdProp && !propName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId) && !KeepProperty(propName))
                continue;

            if (m_scratch == nullptr || ++m_scratchUses > ScratchReuseLimit) {
                m_scratch = std::make_unique<BeJsDocument>();
                m_scratchUses = 1;
            }
            if (SUCCESS != adaptor.RenderRowAsObject(*m_scratch, SingleValueRow(val))) {
                err.Sprintf("ChangeUnifier: failed to render property '%s'.", propName.c_str());
                return ERROR;
            }
            m_scratch->ForEachProperty([&](Utf8CP name, BeJsConst member) {
                UnifiedProp unifiedProp;
                unifiedProp.m_nameId = Intern(name);
                unifiedProp.m_isClassId = isClassIdProp;
                ValueCodec::Encode(unifiedProp.m_value, member);
                entry.m_props.push_back(std::move(unifiedProp));
                return false;
            });
        }
        if (!hasInstanceId || !hasClassId) {
            Utf8String tableName;
            reader.GetTableName(tableName);
            err.Sprintf("ChangeUnifier: row of table '%s' has no ECInstanceId or ECClassId.", tableName.c_str());
            return ERROR;
        }
        key.m_stage = static_cast<uint8_t>(stage == Changes::Change::Stage::New ? 1 : 0);
        return SUCCESS;
    }

    void AddEntry(UnifiedKey const& key, UnifiedEntry&& entry) {
        auto it = m_entries.find(key);
        if (it == m_entries.end()) {
            m_estimatedBytes += EstimateBytes(entry);
            m_entries.insert(std::make_pair(key, std::move(entry)));
            return;
        }
        const uint64_t before = EstimateBytes(it->second);
        MergeInto(it->second, std::move(entry));
        const uint64_t after = EstimateBytes(it->second);
        m_estimatedBytes = m_estimatedBytes - before + after;
    }

    std::vector<KeyedEntry> DrainSorted() {
        std::vector<KeyedEntry> sorted;
        sorted.reserve(m_entries.size());
        for (auto& kv : m_entries)
            sorted.push_back(std::make_pair(kv.first, std::move(kv.second)));
        m_entries.clear();
        m_estimatedBytes = 0;
        std::sort(sorted.begin(), sorted.end(), [](KeyedEntry const& lhs, KeyedEntry const& rhs) { return lhs.first < rhs.first; });
        return sorted;
    }

    DbResult Spill(ECDbCR ecdb) {
        if (!m_spillFile.IsOpen()) {
            BeGuid guid(true);
            m_spillFileName = BeFileName(Utf8String(ecdb.GetTempFileBaseName() + "-" + guid.ToString() + "-unifier.spill"));
            if (BeFileStatus::Success != m_spillFile.Create(m_spillFileName.GetNameUtf8(), true))
                return Fail(BE_SQLITE_CANTOPEN, Utf8PrintfString("ChangeUnifier: failed to create spill file '%s'.", m_spillFileName.GetNameUtf8().c_str()));
            m_spillFileSize = 0;
        }
        std::vector<KeyedEntry> sorted = DrainSorted();
        SpillRun run;
        run.m_offset = m_spillFileSize;
        if (BeFileStatus::Success != m_spillFile.SetPointer(static_cast<int64_t>(m_spillFileSize), BeFileSeekOrigin::Begin))
            return Fail(BE_SQLITE_IOERR, "ChangeUnifier: failed to seek in spill file.");
        m_ioBuffer.clear();
        for (auto const& kv : sorted) {
            EntrySerializer::Write(m_ioBuffer, kv.first, kv.second);
            if (m_ioBuffer.size() >= SpillWriteChunkBytes) {
                if (BeFileStatus::Success != m_spillFile.WriteAll(m_ioBuffer.data(), m_ioBuffer.size()))
                    return Fail(BE_SQLITE_IOERR, "ChangeUnifier: failed to write spill file.");
                m_spillFileSize += m_ioBuffer.size();
                m_ioBuffer.clear();
            }
        }
        if (!m_ioBuffer.empty()) {
            if (BeFileStatus::Success != m_spillFile.WriteAll(m_ioBuffer.data(), m_ioBuffer.size()))
                return Fail(BE_SQLITE_IOERR, "ChangeUnifier: failed to write spill file.");
            m_spillFileSize += m_ioBuffer.size();
            m_ioBuffer.clear();
        }
        run.m_length = m_spillFileSize - run.m_offset;
        if (run.m_length > 0)
            m_runs.push_back(run);
        return BE_SQLITE_OK;
    }

    //! Makes sure @p source holds at least @p need unread bytes. Returns false if the run has fewer bytes left.
    bool FillSource(Source& source, size_t need) {
        size_t available = source.m_buffer.size() - source.m_bufferPos;
        if (available >= need)
            return true;
        if (source.m_bufferPos > 0) {
            source.m_buffer.erase(source.m_buffer.begin(), source.m_buffer.begin() + source.m_bufferPos);
            source.m_bufferPos = 0;
        }
        const uint64_t left = source.m_fileEnd - source.m_filePos;
        if (left < need - available)
            return false;
        const uint64_t toRead = std::min<uint64_t>(left, std::max<uint64_t>(need - available, static_cast<uint64_t>(SpillReadChunkBytes)));
        const size_t oldSize = source.m_buffer.size();
        source.m_buffer.resize(oldSize + static_cast<size_t>(toRead));
        if (BeFileStatus::Success != m_spillFile.SetPointer(static_cast<int64_t>(source.m_filePos), BeFileSeekOrigin::Begin))
            return false;
        size_t done = 0;
        while (done < toRead) {
            uint32_t chunk = static_cast<uint32_t>(std::min<uint64_t>(toRead - done, 0x40000000ull));
            uint32_t bytesRead = 0;
            if (BeFileStatus::Success != m_spillFile.Read(source.m_buffer.data() + oldSize + done, &bytesRead, chunk) || bytesRead == 0)
                return false;
            done += bytesRead;
        }
        source.m_filePos += toRead;
        return true;
    }

    //! Advances @p source to its next record. Returns ERROR on I/O or format errors.
    BentleyStatus AdvanceSource(Source& source) {
        source.m_hasCurrent = false;
        if (source.m_isRemainder) {
            if (source.m_remainderPos >= m_remainder.size())
                return SUCCESS;
            KeyedEntry& kv = m_remainder[source.m_remainderPos++];
            source.m_key = kv.first;
            source.m_entry = std::move(kv.second);
            source.m_hasCurrent = true;
            return SUCCESS;
        }
        if (source.m_filePos >= source.m_fileEnd && source.m_bufferPos >= source.m_buffer.size())
            return SUCCESS;
        if (!FillSource(source, 4))
            return ERROR;
        ByteReader lenReader(source.m_buffer.data() + source.m_bufferPos, 4);
        const uint32_t len = lenReader.GetFixed32();
        source.m_bufferPos += 4;
        if (!FillSource(source, len))
            return ERROR;
        if (SUCCESS != EntrySerializer::Read(source.m_buffer.data() + source.m_bufferPos, len, source.m_key, source.m_entry))
            return ERROR;
        source.m_bufferPos += len;
        source.m_hasCurrent = true;
        return SUCCESS;
    }

    DbResult Finalize() {
        m_remainder = DrainSorted();
        m_sources.clear();
        m_sources.resize(m_runs.size() + 1);
        for (size_t i = 0; i < m_runs.size(); ++i) {
            m_sources[i].m_filePos = m_runs[i].m_offset;
            m_sources[i].m_fileEnd = m_runs[i].m_offset + m_runs[i].m_length;
        }
        m_sources.back().m_isRemainder = true;
        for (size_t i = 0; i < m_sources.size(); ++i) {
            if (SUCCESS != AdvanceSource(m_sources[i]))
                return Fail(BE_SQLITE_IOERR, "ChangeUnifier: failed to read spill file.");
            if (m_sources[i].m_hasCurrent)
                m_heap.push(std::make_pair(m_sources[i].m_key, i));
        }
        return BE_SQLITE_OK;
    }

    //! Pops the next fully merged entry. Returns BE_SQLITE_ROW, BE_SQLITE_DONE or an error.
    DbResult NextMerged(UnifiedKey& key, UnifiedEntry& entry) {
        if (m_heap.empty())
            return BE_SQLITE_DONE;
        HeapItem top = m_heap.top();
        m_heap.pop();
        Source& first = m_sources[top.second];
        key = first.m_key;
        entry = std::move(first.m_entry);
        if (SUCCESS != AdvanceSource(first))
            return Fail(BE_SQLITE_IOERR, "ChangeUnifier: failed to read spill file.");
        if (first.m_hasCurrent)
            m_heap.push(std::make_pair(first.m_key, top.second));

        while (!m_heap.empty() && m_heap.top().first == key) {
            HeapItem next = m_heap.top();
            m_heap.pop();
            Source& source = m_sources[next.second];
            MergeInto(entry, std::move(source.m_entry));
            if (SUCCESS != AdvanceSource(source))
                return Fail(BE_SQLITE_IOERR, "ChangeUnifier: failed to read spill file.");
            if (source.m_hasCurrent)
                m_heap.push(std::make_pair(source.m_key, next.second));
        }
        return BE_SQLITE_ROW;
    }

    static Utf8CP OpToString(UnifiedEntry::Op op) {
        switch (op) {
            case UnifiedEntry::Op::Inserted: return "Inserted";
            case UnifiedEntry::Op::Deleted: return "Deleted";
            default: return "Updated";
        }
    }

    DbResult WriteInstance(UnifiedKey const& key, UnifiedEntry const& entry, BeJsValue out) {
        out.SetEmptyObject();
        for (auto const& prop : entry.m_props) {
            ByteReader reader(prop.m_value.data(), prop.m_value.size());
            if (SUCCESS != ValueCodec::Decode(reader, out[m_strings[prop.m_nameId].c_str()]))
                return Fail(BE_SQLITE_CORRUPT, "ChangeUnifier: failed to decode property value.");
        }
        BeJsValue meta = out["$meta"];
        meta.SetEmptyObject();
        BeJsValue tables = meta["tables"];
        tables.SetEmptyArray();
        for (uint32_t tableId : entry.m_tables)
            tables.appendValue() = m_strings[tableId].c_str();
        meta["op"] = OpToString(entry.m_op);
        meta["stage"] = key.m_stage == 1 ? "New" : "Old";
        BeJsValue changeIndexes = meta["changeIndexes"];
        changeIndexes.SetEmptyArray();
        for (uint32_t changeIndex : entry.m_changeIndexes)
            changeIndexes.appendValue() = changeIndex;
        Utf8String instanceKey;
        instanceKey.Sprintf("%s-%s", ECInstanceId(key.m_instanceId).ToHexStr().c_str(), ECClassId(entry.m_classId).ToHexStr().c_str());
        meta["instanceKey"] = instanceKey.c_str();
        BeJsValue fetched = meta["changeFetchedPropNames"];
        fetched.SetEmptyArray();
        for (uint32_t nameId : entry.m_fetchedNames)
            fetched.appendValue() = m_strings[nameId].c_str();
        meta["isIndirectChange"] = entry.m_isIndirect;
        return BE_SQLITE_ROW;
    }

    void DeleteSpillFile() {
        if (m_spillFile.IsOpen())
            m_spillFile.Close();
        if (!m_spillFileName.empty()) {
            if (m_spillFileName.DoesPathExist() && BeFileNameStatus::Success != m_spillFileName.BeDeleteFile())
                LOG.warningv("ChangeUnifier: failed to delete spill file '%s'.", m_spillFileName.GetNameUtf8().c_str());
            m_spillFileName.clear();
        }
    }

public:
    explicit ChangeUnifierImpl(ChangeUnifier::Options const& options) : m_options(options) {
        for (auto const& name : m_options.m_propNames) {
            m_keepProps.insert(name);
            if (name.EqualsIAscii("Source")) {
                m_keepProps.insert(ECDBSYS_PROP_SourceECInstanceId);
                m_keepProps.insert(ECDBSYS_PROP_SourceECClassId);
            } else if (name.EqualsIAscii("Target")) {
                m_keepProps.insert(ECDBSYS_PROP_TargetECInstanceId);
                m_keepProps.insert(ECDBSYS_PROP_TargetECClassId);
            }
        }
        if (!m_keepProps.empty()) {
            m_keepProps.insert(ECDBSYS_PROP_ECInstanceId);
            m_keepProps.insert(ECDBSYS_PROP_ECClassId);
        }
    }

    ~ChangeUnifierImpl() { DeleteSpillFile(); }

    Utf8StringCR GetLastError() const { return m_lastError; }

    DbResult AppendFrom(ChangesetReader& reader, JsReadOptions const& rowOptions) {
        if (m_state != State::Appending) {
            m_lastError = "ChangeUnifier: AppendFrom() cannot be called after Step(), Close() or a failure.";
            return BE_SQLITE_MISUSE;
        }
        ECDb const* ecdb = reader.GetECDb();
        if (ecdb == nullptr)
            return Fail(BE_SQLITE_MISUSE, "ChangeUnifier: reader is not open.");

        ECSqlRowAdaptor adaptor(*ecdb, rowOptions);
        std::vector<uint32_t> fetchedNames;
        uint32_t changeIndex = 0;
        Utf8String err;
        Utf8String tableName;
        while (true) {
            const DbResult rc = reader.Step();
            if (rc == BE_SQLITE_DONE)
                break;
            if (rc != BE_SQLITE_ROW)
                return Fail(rc, "ChangeUnifier: failed to step the ChangesetReader.");
            ++changeIndex; // counts every row returned by the reader, like ChangesetReader.changeIndex in core-backend

            bool isECTable = false;
            if (SUCCESS != reader.IsECTable(isECTable))
                return Fail(BE_SQLITE_ERROR, "ChangeUnifier: IsECTable() failed.");
            if (!isECTable)
                continue;

            DbOpcode opcode;
            bool isIndirect = false;
            if (SUCCESS != reader.GetTableName(tableName) || SUCCESS != reader.GetOpcode(opcode) || SUCCESS != reader.IsIndirectChange(isIndirect))
                return Fail(BE_SQLITE_ERROR, "ChangeUnifier: failed to read the metadata of the current change.");
            std::vector<Utf8String> const* names = reader.GetChangeFetchedPropertyNames();
            if (names == nullptr)
                return Fail(BE_SQLITE_ERROR, "ChangeUnifier: failed to get change fetched property names.");

            TableInfo const* tableInfo = GetTableInfo(*ecdb, tableName, err);
            if (tableInfo == nullptr)
                return Fail(BE_SQLITE_ERROR, err);
            const TableInfo info = *tableInfo;
            const uint32_t tableId = Intern(tableName);
            fetchedNames.clear();
            for (auto const& name : *names)
                fetchedNames.push_back(Intern(name));

            const UnifiedEntry::Op op = opcode == DbOpcode::Insert ? UnifiedEntry::Op::Inserted : (opcode == DbOpcode::Delete ? UnifiedEntry::Op::Deleted : UnifiedEntry::Op::Updated);
            const bool isMain = !info.m_isOverflow;
            Changes::Change::Stage stages[2];
            int stageCount = 0;
            if (opcode != DbOpcode::Delete && reader.GetColumnCount(Changes::Change::Stage::New) > 0)
                stages[stageCount++] = Changes::Change::Stage::New;
            if (opcode != DbOpcode::Insert && reader.GetColumnCount(Changes::Change::Stage::Old) > 0)
                stages[stageCount++] = Changes::Change::Stage::Old;

            for (int s = 0; s < stageCount; ++s) {
                UnifiedKey key;
                UnifiedEntry entry;
                if (SUCCESS != BuildRowEntry(reader, stages[s], adaptor, tableId, changeIndex, isMain, op, isIndirect, fetchedNames, key, entry, err))
                    return Fail(BE_SQLITE_ERROR, err);
                key.m_rootClassId = info.m_rootClassId != 0 ? info.m_rootClassId : entry.m_classId;
                EnsureAncestors(*ecdb, entry.m_classId);
                AddEntry(key, std::move(entry));
                if (m_options.m_memoryBudgetBytes > 0 && m_estimatedBytes > m_options.m_memoryBudgetBytes) {
                    const DbResult spillRc = Spill(*ecdb);
                    if (spillRc != BE_SQLITE_OK)
                        return spillRc;
                }
            }
        }
        return BE_SQLITE_OK;
    }

    DbResult Step(BeJsValue instance) {
        if (m_state == State::Appending) {
            m_state = State::Stepping;
            const DbResult rc = Finalize();
            if (rc != BE_SQLITE_OK)
                return rc;
        }
        if (m_state == State::Done)
            return BE_SQLITE_DONE;
        if (m_state != State::Stepping) {
            m_lastError = "ChangeUnifier: Step() cannot be called after Close() or a failure.";
            return BE_SQLITE_MISUSE;
        }
        UnifiedKey key;
        UnifiedEntry entry;
        const DbResult rc = NextMerged(key, entry);
        if (rc == BE_SQLITE_DONE) {
            m_state = State::Done;
            m_remainder.clear();
            m_sources.clear();
            DeleteSpillFile();
            return BE_SQLITE_DONE;
        }
        if (rc != BE_SQLITE_ROW)
            return rc;
        return WriteInstance(key, entry, instance);
    }

    void Close() {
        if (m_state == State::Closed)
            return;
        m_state = State::Closed;
        m_entries.clear();
        m_estimatedBytes = 0;
        m_remainder.clear();
        m_sources.clear();
        m_heap = decltype(m_heap)();
        m_runs.clear();
        m_tableCache.clear();
        m_tableCacheDb = nullptr;
        m_classAncestors.clear();
        m_strings.clear();
        m_stringIds.clear();
        m_scratch.reset();
        DeleteSpillFile();
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangeUnifier::ChangeUnifier() : m_impl(std::make_unique<ChangeUnifierImpl>(Options())) {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangeUnifier::ChangeUnifier(Options const& options) : m_impl(std::make_unique<ChangeUnifierImpl>(options)) {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangeUnifier::~ChangeUnifier() {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangeUnifier::AppendFrom(ChangesetReader& reader, JsReadOptions const& rowOptions) { return m_impl->AppendFrom(reader, rowOptions); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangeUnifier::Step(BeJsValue instance) { return m_impl->Step(instance); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangeUnifier::Close() { m_impl->Close(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR ChangeUnifier::GetLastError() const { return m_impl->GetLastError(); }

END_BENTLEY_SQLITE_EC_NAMESPACE
