/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ChangeUnifierSpill.h"
#include "ChangeUnifierMap.h"
#include <unordered_map>
#include <unordered_set>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

namespace {

using UnifiedEntryMap = ChangeUnifierFlatMap<UnifiedKey, UnifiedEntry, UnifiedKeyHash>;

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

} // namespace

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangeUnifierImpl final {
private:
    enum class State { Appending, Stepping, Failed };

    struct TableInfo final {
        uint64_t m_rootClassId = 0;
        bool m_isOverflow = false;
    };

    static constexpr uint64_t PropOverheadBytes = sizeof(UnifiedProp);

    uint64_t m_memoryBudgetBytes;
    bset<Utf8String, CompareIUtf8Ascii> m_keepProps;
    State m_state = State::Appending;
    Utf8String m_lastError;

    //! Strings written for every instance, interned first so their ids are constants. Id 0 means "not interned".
    struct Fixed final { enum : uint32_t { Meta = 1, Tables, Op, Stage, ChangeIndexes, InstanceKey, FetchedNames, IsIndirect, Inserted, Updated, Deleted, Old, New }; };

    // interned strings, retained for the lifetime of the unifier (spill records reference them by id)
    std::vector<Utf8String> m_strings;
    std::unordered_map<Utf8String, uint32_t> m_stringIds;

    ECDb const* m_ecdb = nullptr; //!< ECDb of all readers, bound by the first AppendFrom; only used while appending
    std::unordered_map<Utf8String, TableInfo> m_tableCache;
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> m_classAncestors;

    UnifiedEntryMap m_entries;
    uint64_t m_estimatedBytes = 0; //!< heap bytes owned by the entries; the map's own storage is UnifiedEntryMap::GetMemoryBytes

    UnifierSpillFile m_spill;
    std::vector<KeyedEntry> m_sorted; //!< merged entries in key order when nothing was spilled
    size_t m_sortedPos = 0;
    KeyedEntry m_merged; //!< result of the current Step; kept so the spill merge can reuse its buffers
    KeyedEntry m_next; //!< entry with the same key as m_merged, folded into it

    // rapidjson never returns memory to its pool allocator, so the scratch document is dropped once it has rendered
    // this many values or about this many bytes; the memory budget does not cover it
    static constexpr uint32_t ScratchReuseLimit = 4096;
    static constexpr size_t ScratchByteLimit = 1024 * 1024;
    std::unique_ptr<BeJsDocument> m_scratch;
    uint32_t m_scratchUses = 0;
    size_t m_scratchBytes = 0;

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

    //! Heap bytes owned by @p entry. The entry struct itself lives in the map's storage.
    static uint64_t EstimateBytes(UnifiedEntry const& entry) {
        uint64_t bytes = sizeof(uint32_t) * (entry.m_tables.capacity() + entry.m_changeIndexes.capacity() + entry.m_fetchedNames.capacity());
        bytes += PropOverheadBytes * entry.m_props.capacity();
        for (auto const& prop : entry.m_props)
            bytes += prop.m_value.capacity();
        return bytes;
    }

    //! Merges @p rhs, which comes later in change order, into @p lhs.
    void MergeInto(UnifiedEntry& lhs, UnifiedEntry&& rhs) const {
        if (!lhs.HasMainRow() && rhs.HasMainRow()) {
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

    //! Reads the ECInstanceId, ECClassId and kept property values of one row stage into @p key and @p entry,
    //! which already holds the metadata of the row. Returns ERROR with @p err set on failure.
    BentleyStatus ReadStageValues(ChangesetReader const& reader, Changes::Change::Stage stage, ECSqlRowAdaptor const& adaptor, UnifiedKey& key, UnifiedEntry& entry, Utf8StringR err) {
        const int count = reader.GetColumnCount(stage);
        if (count <= ChangesetReader::ClassIdColumn) {
            Utf8String tableName;
            reader.GetTableName(tableName);
            err.Sprintf("ChangeUnifier: row of table '%s' has no ECInstanceId or ECClassId.", tableName.c_str());
            return ERROR;
        }
        key.m_instanceId = reader.GetValue(stage, ChangesetReader::InstanceIdColumn).GetId<ECInstanceId>().GetValueUnchecked();
        entry.m_classId = reader.GetValue(stage, ChangesetReader::ClassIdColumn).GetId<ECClassId>().GetValueUnchecked();
        for (int i = 0; i < count; ++i) {
            IECSqlValue const& val = reader.GetValue(stage, i);
            ECPropertyCP prop = val.GetColumnInfo().GetProperty();
            if (prop == nullptr)
                continue;
            Utf8StringCR propName = prop->GetName();
            if (!KeepProperty(propName)) // always true for ECInstanceId and ECClassId
                continue;
            const bool isClassIdProp = i == ChangesetReader::ClassIdColumn;

            if (m_scratch == nullptr) {
                m_scratch = std::make_unique<BeJsDocument>();
                m_scratchUses = 0;
                m_scratchBytes = 0;
            }
            if (SUCCESS != adaptor.RenderRowAsObject(*m_scratch, SingleValueRow(val))) {
                err.Sprintf("ChangeUnifier: failed to render property '%s'.", propName.c_str());
                return ERROR;
            }
            m_scratch->ForEachProperty([&](Utf8CP name, BeJsConst member) {
                UnifiedProp unifiedProp;
                unifiedProp.m_nameId = Intern(name);
                unifiedProp.m_isClassId = isClassIdProp;
                UnifiedValueCodec::Encode(unifiedProp.m_value, member);
                m_scratchBytes += unifiedProp.m_value.size(); // about the size of the rendered value
                entry.m_props.push_back(std::move(unifiedProp));
                return false;
            });
            if (++m_scratchUses >= ScratchReuseLimit || m_scratchBytes >= ScratchByteLimit)
                m_scratch.reset(); // also drops an oversized value right away
        }
        key.m_stage = static_cast<uint8_t>(stage == Changes::Change::Stage::New ? 1 : 0);
        return SUCCESS;
    }

    void AddEntry(UnifiedKey const& key, UnifiedEntry&& entry) {
        UnifiedEntry* existing = m_entries.Find(key);
        if (existing == nullptr) {
            m_estimatedBytes += EstimateBytes(entry);
            m_entries.Insert(key, std::move(entry));
            return;
        }
        const uint64_t before = EstimateBytes(*existing);
        MergeInto(*existing, std::move(entry));
        const uint64_t after = EstimateBytes(*existing);
        m_estimatedBytes = m_estimatedBytes - before + after;
    }

    std::vector<KeyedEntry> DrainSorted() {
        std::vector<KeyedEntry> sorted = m_entries.TakeEntries();
        m_estimatedBytes = 0;
        std::sort(sorted.begin(), sorted.end(), [](KeyedEntry const& lhs, KeyedEntry const& rhs) { return lhs.first < rhs.first; });
        return sorted;
    }

    DbResult Spill() {
        const DbResult rc = m_spill.WriteRun(DrainSorted(), m_lastError);
        return rc == BE_SQLITE_OK ? BE_SQLITE_OK : Fail(rc, m_lastError);
    }

    //! Ends appending. Without spilled runs the sorted map is the result; otherwise the map becomes the last run.
    DbResult Finalize() {
        if (!m_spill.HasRuns()) {
            m_sorted = DrainSorted();
            return BE_SQLITE_OK;
        }
        const DbResult rc = Spill();
        if (rc != BE_SQLITE_OK)
            return rc;
        if (SUCCESS != m_spill.StartMerge(m_memoryBudgetBytes, m_lastError))
            return Fail(BE_SQLITE_IOERR, m_lastError);
        return BE_SQLITE_OK;
    }

    //! Moves the next fully merged entry into m_merged. Returns BE_SQLITE_ROW, BE_SQLITE_DONE or an error.
    DbResult NextMerged() {
        if (!m_spill.HasRuns()) {
            if (m_sortedPos >= m_sorted.size())
                return BE_SQLITE_DONE;
            m_merged = std::move(m_sorted[m_sortedPos++]);
            return BE_SQLITE_ROW;
        }
        if (m_spill.PeekKey() == nullptr)
            return BE_SQLITE_DONE;
        if (SUCCESS != m_spill.Pop(m_merged, m_lastError))
            return Fail(BE_SQLITE_IOERR, m_lastError);
        for (UnifiedKey const* next = m_spill.PeekKey(); next != nullptr && *next == m_merged.first; next = m_spill.PeekKey()) {
            if (SUCCESS != m_spill.Pop(m_next, m_lastError))
                return Fail(BE_SQLITE_IOERR, m_lastError);
            MergeInto(m_merged.second, std::move(m_next.second));
        }
        return BE_SQLITE_ROW;
    }

    void WriteKey(ChangeUnifier::IInstanceWriter& out, uint32_t internedId) const {
        Utf8StringCR str = m_strings[internedId];
        out.Key(str.c_str(), str.size(), internedId);
    }

    void WriteString(ChangeUnifier::IInstanceWriter& out, uint32_t internedId) const {
        Utf8StringCR str = m_strings[internedId];
        out.String(str.c_str(), str.size(), internedId);
    }

    uint32_t OpStringId(UnifiedEntry::Op op) const {
        switch (op) {
            case UnifiedEntry::Op::Inserted: return Fixed::Inserted;
            case UnifiedEntry::Op::Deleted: return Fixed::Deleted;
            default: return Fixed::Updated;
        }
    }

    DbResult WriteInstance(KeyedEntry const& merged, ChangeUnifier::IInstanceWriter& out) {
        UnifiedKey const& key = merged.first;
        UnifiedEntry const& entry = merged.second;
        out.StartObject();
        for (auto const& prop : entry.m_props) {
            WriteKey(out, prop.m_nameId);
            if (SUCCESS != UnifiedValueCodec::Decode(prop.m_value, out))
                return Fail(BE_SQLITE_CORRUPT, "ChangeUnifier: failed to decode property value.");
        }
        WriteKey(out, Fixed::Meta);
        out.StartObject();
        WriteKey(out, Fixed::Tables);
        out.StartArray();
        for (uint32_t tableId : entry.m_tables)
            WriteString(out, tableId);
        out.EndArray();
        WriteKey(out, Fixed::Op);
        WriteString(out, OpStringId(entry.m_op));
        WriteKey(out, Fixed::Stage);
        WriteString(out, key.m_stage == 1 ? Fixed::New : Fixed::Old);
        WriteKey(out, Fixed::ChangeIndexes);
        out.StartArray();
        for (uint32_t changeIndex : entry.m_changeIndexes)
            out.Int64(changeIndex);
        out.EndArray();
        WriteKey(out, Fixed::InstanceKey);
        Utf8Char instanceKey[2 * BeInt64Id::ID_STRINGBUFFER_LENGTH];
        ECInstanceId(key.m_instanceId).ToString(instanceKey, BeInt64Id::UseHex::Yes);
        size_t length = strlen(instanceKey);
        instanceKey[length++] = '-';
        ECClassId(entry.m_classId).ToString(instanceKey + length, BeInt64Id::UseHex::Yes);
        out.String(instanceKey, strlen(instanceKey), 0);
        WriteKey(out, Fixed::FetchedNames);
        out.StartArray();
        for (uint32_t nameId : entry.m_fetchedNames)
            WriteString(out, nameId);
        out.EndArray();
        WriteKey(out, Fixed::IsIndirect);
        out.Bool(entry.m_isIndirect);
        out.EndObject();
        out.EndObject();
        return BE_SQLITE_ROW;
    }

public:
    explicit ChangeUnifierImpl(ChangeUnifier::Options const& options) : m_memoryBudgetBytes(options.m_memoryBudgetBytes) {
        static Utf8CP const s_fixedStrings[] = {"$meta", "tables", "op", "stage", "changeIndexes", "instanceKey", "changeFetchedPropNames",
            "isIndirectChange", "Inserted", "Updated", "Deleted", "Old", "New"};
        m_strings.emplace_back(); // id 0 is never handed out
        for (Utf8CP str : s_fixedStrings)
            Intern(str);
        BeAssert(m_strings[Fixed::New] == "New");
        for (auto const& name : options.m_propNames) {
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

    Utf8StringCR GetLastError() const { return m_lastError; }

    //! Merges the current row of @p reader. Rows of non-EC tables are skipped.
    DbResult AppendCurrentRow(ChangesetReader& reader, ECSqlRowAdaptor const& adaptor, uint32_t changeIndex) {
        bool isECTable = false;
        if (SUCCESS != reader.IsECTable(isECTable))
            return Fail(BE_SQLITE_ERROR, "ChangeUnifier: IsECTable() failed.");
        if (!isECTable)
            return BE_SQLITE_OK;

        Utf8String tableName;
        DbOpcode opcode;
        bool isIndirect = false;
        if (SUCCESS != reader.GetTableName(tableName) || SUCCESS != reader.GetOpcode(opcode) || SUCCESS != reader.IsIndirectChange(isIndirect))
            return Fail(BE_SQLITE_ERROR, "ChangeUnifier: failed to read the metadata of the current change.");
        std::vector<Utf8String> const* names = reader.GetChangeFetchedPropertyNames();
        if (names == nullptr)
            return Fail(BE_SQLITE_ERROR, "ChangeUnifier: failed to get change fetched property names.");

        Utf8String err;
        TableInfo const* tableInfo = GetTableInfo(*m_ecdb, tableName, err);
        if (tableInfo == nullptr)
            return Fail(BE_SQLITE_ERROR, err);
        const uint64_t rootClassId = tableInfo->m_rootClassId;

        // metadata shared by the Old and New stage of the row
        UnifiedEntry rowEntry;
        if (!tableInfo->m_isOverflow)
            rowEntry.m_op = opcode == DbOpcode::Insert ? UnifiedEntry::Op::Inserted : (opcode == DbOpcode::Delete ? UnifiedEntry::Op::Deleted : UnifiedEntry::Op::Updated);
        rowEntry.m_isIndirect = isIndirect;
        rowEntry.m_tables.push_back(Intern(tableName));
        rowEntry.m_changeIndexes.push_back(changeIndex);
        rowEntry.m_fetchedNames.reserve(names->size());
        for (auto const& name : *names)
            rowEntry.m_fetchedNames.push_back(Intern(name));

        for (Changes::Change::Stage stage : {Changes::Change::Stage::New, Changes::Change::Stage::Old}) {
            if (reader.GetColumnCount(stage) <= 0) // the reader leaves New empty for a delete and Old for an insert
                continue;
            UnifiedKey key;
            UnifiedEntry entry = rowEntry;
            if (SUCCESS != ReadStageValues(reader, stage, adaptor, key, entry, err))
                return Fail(BE_SQLITE_ERROR, err);
            key.m_rootClassId = rootClassId != 0 ? rootClassId : entry.m_classId;
            EnsureAncestors(*m_ecdb, entry.m_classId);
            AddEntry(key, std::move(entry));
            if (m_memoryBudgetBytes > 0 && m_estimatedBytes + m_entries.GetMemoryBytes() > m_memoryBudgetBytes) {
                const DbResult rc = Spill();
                if (rc != BE_SQLITE_OK)
                    return rc;
            }
        }
        return BE_SQLITE_OK;
    }

    DbResult AppendFrom(ChangesetReader& reader, JsReadOptions const& rowOptions) {
        if (m_state != State::Appending) {
            m_lastError = "ChangeUnifier: AppendFrom() cannot be called after Step() or a failure.";
            return BE_SQLITE_MISUSE;
        }
        ECDb const* ecdb = reader.GetECDb();
        if (ecdb == nullptr)
            return Fail(BE_SQLITE_MISUSE, "ChangeUnifier: reader is not open.");
        if (m_ecdb == nullptr) {
            m_ecdb = ecdb;
            m_spill.SetFileNameBase(ecdb->GetTempFileBaseName());
        } else if (m_ecdb != ecdb) {
            // merge keys, table and class caches are only meaningful within one ECDb
            m_lastError = "ChangeUnifier: all readers must use the same ECDb.";
            return BE_SQLITE_MISUSE;
        }

        ECSqlRowAdaptor adaptor(*ecdb, rowOptions);
        uint32_t changeIndex = 0; // counts every row returned by the reader, like ChangesetReader.changeIndex in core-backend
        for (DbResult rc = reader.Step(); rc != BE_SQLITE_DONE; rc = reader.Step()) {
            if (rc != BE_SQLITE_ROW)
                return Fail(rc, "ChangeUnifier: failed to step the ChangesetReader.");
            const DbResult appendRc = AppendCurrentRow(reader, adaptor, ++changeIndex);
            if (appendRc != BE_SQLITE_OK)
                return appendRc;
        }
        return BE_SQLITE_OK;
    }

    DbResult Step(ChangeUnifier::IInstanceWriter& writer) {
        if (m_state == State::Appending) {
            m_state = State::Stepping;
            const DbResult rc = Finalize();
            if (rc != BE_SQLITE_OK)
                return rc;
        }
        if (m_state != State::Stepping) {
            m_lastError = "ChangeUnifier: Step() cannot be called after a failure.";
            return BE_SQLITE_MISUSE;
        }
        const DbResult rc = NextMerged();
        if (rc == BE_SQLITE_DONE) { // keeps returning DONE: nothing is left to merge
            m_sorted.clear();
            m_spill.Delete();
            return BE_SQLITE_DONE;
        }
        if (rc != BE_SQLITE_ROW)
            return rc;
        return WriteInstance(m_merged, writer);
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangeUnifier::ChangeUnifier() : ChangeUnifier(Options()) {}

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
DbResult ChangeUnifier::AppendFrom(ChangesetReader& reader, JsReadOptions const& rowOptions) { return m_impl != nullptr ? m_impl->AppendFrom(reader, rowOptions) : BE_SQLITE_MISUSE; }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangeUnifier::Step(IInstanceWriter& writer) { return m_impl != nullptr ? m_impl->Step(writer) : BE_SQLITE_MISUSE; }

//---------------------------------------------------------------------------------------
// Dropping the implementation frees all memory and deletes the spill file.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangeUnifier::Close() { m_impl.reset(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR ChangeUnifier::GetLastError() const {
    static Utf8String const s_closed("ChangeUnifier: the unifier is closed.");
    return m_impl != nullptr ? m_impl->GetLastError() : s_closed;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
