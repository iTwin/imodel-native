/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ChangeUnifierSpill.h"
#include <unordered_map>
#include <unordered_set>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

namespace {

using UnifiedEntryMap = std::unordered_map<UnifiedKey, UnifiedEntry, UnifiedKeyHash>;

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
    enum class State { Appending, Stepping, Done, Failed };

    struct TableInfo final {
        uint64_t m_rootClassId = 0;
        bool m_isOverflow = false;
    };

    static constexpr uint64_t EntryOverheadBytes = sizeof(UnifiedKey) + sizeof(UnifiedEntry) + 32; // hash node + bucket
    static constexpr uint64_t PropOverheadBytes = sizeof(UnifiedProp);

    ChangeUnifier::Options m_options;
    bset<Utf8String, CompareIUtf8Ascii> m_keepProps;
    State m_state = State::Appending;
    Utf8String m_lastError;

    // interned strings, retained for the lifetime of the unifier (spill records reference them by id)
    std::vector<Utf8String> m_strings;
    std::unordered_map<Utf8String, uint32_t> m_stringIds;

    ECDb const* m_ecdb = nullptr; //!< ECDb of the latest AppendFrom; the table cache belongs to it
    std::unordered_map<Utf8String, TableInfo> m_tableCache;
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> m_classAncestors;

    UnifiedEntryMap m_entries;
    uint64_t m_estimatedBytes = 0;

    UnifierSpillFile m_spill;
    std::vector<KeyedEntry> m_sorted; //!< merged entries in key order when nothing was spilled
    size_t m_sortedPos = 0;

    // rapidjson never returns memory to its pool allocator, so the scratch document is recreated periodically
    static constexpr uint32_t ScratchReuseLimit = 4096;
    std::unique_ptr<BeJsDocument> m_scratch;
    uint32_t m_scratchUses = 0;

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
                UnifiedValueCodec::Encode(unifiedProp.m_value, member);
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

    DbResult Spill() {
        const DbResult rc = m_spill.WriteRun(*m_ecdb, DrainSorted(), m_lastError);
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
        if (SUCCESS != m_spill.StartMerge(m_lastError))
            return Fail(BE_SQLITE_IOERR, m_lastError);
        return BE_SQLITE_OK;
    }

    //! Gets the next fully merged entry. Returns BE_SQLITE_ROW, BE_SQLITE_DONE or an error.
    DbResult NextMerged(KeyedEntry& out) {
        if (!m_spill.HasRuns()) {
            if (m_sortedPos >= m_sorted.size())
                return BE_SQLITE_DONE;
            out = std::move(m_sorted[m_sortedPos++]);
            return BE_SQLITE_ROW;
        }
        if (m_spill.PeekKey() == nullptr)
            return BE_SQLITE_DONE;
        if (SUCCESS != m_spill.Pop(out, m_lastError))
            return Fail(BE_SQLITE_IOERR, m_lastError);
        for (UnifiedKey const* next = m_spill.PeekKey(); next != nullptr && *next == out.first; next = m_spill.PeekKey()) {
            KeyedEntry more;
            if (SUCCESS != m_spill.Pop(more, m_lastError))
                return Fail(BE_SQLITE_IOERR, m_lastError);
            MergeInto(out.second, std::move(more.second));
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

    DbResult WriteInstance(KeyedEntry const& merged, BeJsValue out) {
        UnifiedKey const& key = merged.first;
        UnifiedEntry const& entry = merged.second;
        out.SetEmptyObject();
        for (auto const& prop : entry.m_props) {
            if (SUCCESS != UnifiedValueCodec::Decode(prop.m_value, out[m_strings[prop.m_nameId].c_str()]))
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
        const TableInfo info = *tableInfo;
        const uint32_t tableId = Intern(tableName);
        std::vector<uint32_t> fetchedNames;
        fetchedNames.reserve(names->size());
        for (auto const& name : *names)
            fetchedNames.push_back(Intern(name));

        const UnifiedEntry::Op op = opcode == DbOpcode::Insert ? UnifiedEntry::Op::Inserted : (opcode == DbOpcode::Delete ? UnifiedEntry::Op::Deleted : UnifiedEntry::Op::Updated);
        const bool isMain = !info.m_isOverflow;
        for (Changes::Change::Stage stage : {Changes::Change::Stage::New, Changes::Change::Stage::Old}) {
            const DbOpcode excluded = stage == Changes::Change::Stage::New ? DbOpcode::Delete : DbOpcode::Insert;
            if (opcode == excluded || reader.GetColumnCount(stage) <= 0)
                continue;
            UnifiedKey key;
            UnifiedEntry entry;
            if (SUCCESS != BuildRowEntry(reader, stage, adaptor, tableId, changeIndex, isMain, op, isIndirect, fetchedNames, key, entry, err))
                return Fail(BE_SQLITE_ERROR, err);
            key.m_rootClassId = info.m_rootClassId != 0 ? info.m_rootClassId : entry.m_classId;
            EnsureAncestors(*m_ecdb, entry.m_classId);
            AddEntry(key, std::move(entry));
            if (m_options.m_memoryBudgetBytes > 0 && m_estimatedBytes > m_options.m_memoryBudgetBytes) {
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
        if (m_ecdb != ecdb) {
            m_tableCache.clear();
            m_ecdb = ecdb;
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
            m_lastError = "ChangeUnifier: Step() cannot be called after a failure.";
            return BE_SQLITE_MISUSE;
        }
        KeyedEntry merged;
        const DbResult rc = NextMerged(merged);
        if (rc == BE_SQLITE_DONE) {
            m_state = State::Done;
            m_sorted.clear();
            m_spill.Delete();
            return BE_SQLITE_DONE;
        }
        if (rc != BE_SQLITE_ROW)
            return rc;
        return WriteInstance(merged, instance);
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
DbResult ChangeUnifier::Step(BeJsValue instance) { return m_impl != nullptr ? m_impl->Step(instance) : BE_SQLITE_MISUSE; }

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
