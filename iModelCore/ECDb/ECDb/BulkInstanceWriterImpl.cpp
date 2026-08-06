/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSql/ECSqlStatementNoopImpls.h"
#include <algorithm>
#include <cinttypes>
#include <cstdarg>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

// ======================================================================================
using TableWriter = BulkInstanceWriter::Impl::TableWriter;
using PropertyWriter = BulkInstanceWriter::Impl::PropertyWriter;
using ClassWriter = BulkInstanceWriter::Impl::ClassWriter;
using WriteContext = BulkInstanceWriter::Impl::WriteContext;
using Writer = BulkInstanceWriter::Impl::Writer;
// ======================================================================================

//=======================================================================================
//! The binder factory reads the current ECSQL scope in order to decide whether a property
//! map that is mapped to a virtual column needs a no-op binder. As BulkInstanceWriter does
//! not go through the ECSQL parser it has no expression tree, so it pushes a synthetic
//! scope which only carries the expression type.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct SyntheticScopeExp final : Exp {
private:
    void _ToECSql(ECSqlRenderContext&) const override {}
    void _ToJson(BeJsValue, JsonFormat const&) const override {}
    Utf8String _ToString() const override { return "SyntheticScopeExp"; }

public:
    explicit SyntheticScopeExp(Exp::Type type) : Exp(type) {}
};

//=======================================================================================
//! Owns the synthetic scopes that are pushed on an ECSqlPrepareContext for the lifetime of
//! the binder creation of one table statement.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct SyntheticScopeGuard final {
private:
    ECSqlPrepareContext& m_ctx;
    SyntheticScopeExp m_statementExp;
    SyntheticScopeExp m_assignmentListExp;
    int m_pushCount = 0;

public:
    SyntheticScopeGuard(ECSqlPrepareContext& ctx, BulkInstanceWriter::WriterOp op)
        : m_ctx(ctx),
          m_statementExp(op == BulkInstanceWriter::WriterOp::Insert ? Exp::Type::Insert : Exp::Type::Update),
          m_assignmentListExp(Exp::Type::AssignmentList) {
        m_ctx.PushScope(m_statementExp);
        ++m_pushCount;
        if (op == BulkInstanceWriter::WriterOp::Update) {
            // mirrors the ECSQL UPDATE SET clause scope so that virtual columns get no-op binders
            m_ctx.PushScope(m_assignmentListExp);
            ++m_pushCount;
        }
    }
    ~SyntheticScopeGuard() {
        while (m_pushCount-- > 0)
            m_ctx.PopScope();
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult TableWriter::Prepare(ECDbCR ecdb, Utf8StringCR sql) {
    const auto rc = GetSqliteStmt().Prepare(ecdb, sql.c_str());
    if (rc != BE_SQLITE_OK) {
        ECDbLogger::Get().errorv("BulkInstanceWriter: failed to prepare SQL '%s'", sql.c_str());
        return rc;
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void TableWriter::Reset() const {
    auto& stmt = GetSqliteStmt();
    stmt.Reset();
    stmt.ClearBindings();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PropertyWriter const* ClassWriter::FindProperty(Utf8CP name) const {
    if (name == nullptr)
        return nullptr;

    const auto it = m_propertyMap.find(name);
    return it == m_propertyMap.end() ? nullptr : it->second;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ClassWriter::GetPropertyIndex(Utf8CP name) const {
    const auto prop = FindProperty(name);
    return prop == nullptr ? -1 : prop->GetIndex();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PropertyWriter const* ClassWriter::GetProperty(int index) const {
    if (index < 0 || index >= (int)m_propertiesByIndex.size())
        return nullptr;

    return m_propertiesByIndex[(size_t)index];
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableWriter& ClassWriter::AddTable(TableWriter::Ptr table) {
    m_tables.push_back(std::move(table));
    return *m_tables.back();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PropertyWriter& ClassWriter::AddProperty(PropertyWriter::Ptr property) {
    m_properties.push_back(std::move(property));
    return *m_properties.back();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ClassWriter::BuildIndexes() {
    m_propertyMap.clear();
    m_propertiesByIndex.clear();

    int maxIndex = -1;
    for (auto const& prop : m_properties)
        maxIndex = std::max(maxIndex, prop->GetIndex());

    m_propertiesByIndex.resize((size_t)(maxIndex + 1), nullptr);
    for (auto const& prop : m_properties) {
        m_propertyMap.insert(std::make_pair(prop->GetName().c_str(), prop.get()));
        m_propertiesByIndex[(size_t)prop->GetIndex()] = prop.get();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ClassWriter::RegisterStepHooks() {
    m_bindersToCallOnBeforeStep.clear();
    m_bindersToCallOnClearBindings.clear();
    for (auto const& prop : m_properties) {
        auto& binder = prop->GetBinder();
        if (binder.HasToCallOnBeforeStep())
            m_bindersToCallOnBeforeStep.push_back(&binder);

        if (binder.HasToCallOnClearBindings())
            m_bindersToCallOnClearBindings.push_back(&binder);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ClassWriter::OnBeforeFirstStep() const {
    for (auto binder : m_bindersToCallOnBeforeStep) {
        const auto stat = binder->OnBeforeFirstStep();
        if (!stat.IsSuccess())
            return stat;
    }
    return ECSqlStatus::Success;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ClassWriter::OnClearBindings() const {
    for (auto binder : m_bindersToCallOnClearBindings)
        binder->OnClearBindings();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ClassWriter::ResetStatements() const {
    for (auto const& table : m_tables)
        table->Reset();

    OnClearBindings();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlBinder& WriteContext::_GetBinder(int propertyIndex) const {
    const auto prop = m_class.GetProperty(propertyIndex);
    if (prop == nullptr)
        return NoopECSqlBinder::Get();

    MarkWritten(propertyIndex);
    return prop->GetBinder();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlBinder* WriteContext::_FindBinder(Utf8CP propertyName) const {
    const auto prop = m_class.FindProperty(propertyName);
    if (prop == nullptr)
        return nullptr;

    MarkWritten(prop->GetIndex());
    return &prop->GetBinder();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECPropertyCP WriteContext::_GetProperty(int propertyIndex) const {
    const auto prop = m_class.GetProperty(propertyIndex);
    return prop == nullptr ? nullptr : &prop->GetProperty();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WriteContext::MarkWritten(int propertyIndex) const {
    if (propertyIndex < 0)
        return;

    const auto word = (size_t)(propertyIndex / Impl::kMaskBits);
    if (word >= m_masks.size())
        return;

    m_masks[word] |= (uint64_t)1 << (uint64_t)(propertyIndex % Impl::kMaskBits);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool WriteContext::IsWritten(int propertyIndex) const {
    if (propertyIndex < 0)
        return false;

    const auto word = (size_t)(propertyIndex / Impl::kMaskBits);
    if (word >= m_masks.size())
        return false;

    return (m_masks[word] & ((uint64_t)1 << (uint64_t)(propertyIndex % Impl::kMaskBits))) != 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool WriteContext::HasAnyWritten() const {
    for (auto mask : m_masks) {
        if (mask != 0)
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String ClassWriter::Factory::MakeMaskParamName(size_t wordIndex) {
    Utf8String name;
    name.Sprintf(":%s%d", Impl::kMaskParamPrefix, (int)wordIndex);
    return name;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ClassWriter::Factory::IsSystemIdPropertyMap(PropertyMap const& propMap) {
    return propMap.GetType() == PropertyMap::Type::ECInstanceId || propMap.GetType() == PropertyMap::Type::ECClassId;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ClassWriter::Factory::IsWritable(PropertyMap const& propMap, Utf8StringCR timestampPropName, WriterOp op) {
    // ECInstanceId is driven by the caller/options, ECClassId is a constant of the class map.
    if (IsSystemIdPropertyMap(propMap))
        return false;

    // owned by the database trigger
    if (!timestampPropName.empty() && timestampPropName.EqualsIAscii(propMap.GetName()))
        return false;

    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
DbTable const* ClassWriter::Factory::GetPropertyTable(ClassMapCR classMap, PropertyMap const& propMap) {
    if (propMap.IsData())
        return &propMap.GetAs<DataPropertyMap>().GetTable();

    // system property maps (constraint ids of link tables) live in the primary table
    return &classMap.GetPrimaryTable();
}

//---------------------------------------------------------------------------------------
//! Pairs the non virtual columns of a property map within one table with the sqlite
//! parameter names the binder generated for it. Both lists are produced by the very same
//! property map traversal order, and binders that map to a virtual column generate no
//! parameter name at all (no-op binders), so the two lists line up 1:1.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ClassWriter::Factory::CollectColumnBindings(std::vector<ColumnBinding>& bindings, PropertyMap const& propMap, DbTable const& table, ECSqlBinder const& binder, WriterOp op) {
    GetColumnsPropertyMapVisitor columnVisitor(table, PropertyMap::Type::All);
    if (SUCCESS != propMap.AcceptVisitor(columnVisitor))
        return ERROR;

    std::vector<DbColumn const*> writableColumns;
    for (auto col : columnVisitor.GetColumns()) {
        if (col->GetPersistenceType() == PersistenceType::Virtual)
            continue;

        writableColumns.push_back(col);
    }

    auto const& paramNames = binder.GetMappedSqlParameterNames();
    if (paramNames.empty()) {
        // no-op binder (e.g. a ConstraintECClassId that is not physically stored). Nothing to write.
        return SUCCESS;
    }

    if (writableColumns.size() != paramNames.size())
        return ERROR;

    for (size_t i = 0; i < writableColumns.size(); ++i) {
        ColumnBinding binding;
        binding.m_column = writableColumns[i];
        binding.m_paramName = paramNames[i];
        bindings.push_back(std::move(binding));
    }

    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ClassWriter::Ptr ClassWriter::Factory::Create(ECDbCR ecdb, ClassMapCR classMap, WriterOp op, Utf8StringR error) {
    auto const& ecClass = classMap.GetClass();
    if (classMap.GetType() == ClassMap::Type::RelationshipEndTable) {
        error.Sprintf("BulkInstanceWriter does not support foreign key (end table) relationship class '%s'. Use ECSQL or InstanceWriter instead.", ecClass.GetFullName());
        return nullptr;
    }

    if (classMap.GetPrimaryTable().GetType() == DbTable::Type::Virtual) {
        error.Sprintf("Class '%s' is not mapped to a real table and cannot be written.", ecClass.GetFullName());
        return nullptr;
    }

    Utf8String timestampPropName;
    if (auto ca = ecClass.GetCustomAttribute("CoreCustomAttributes", "ClassHasCurrentTimeStampProperty"); ca != nullptr) {
        ECValue v;
        if (ECObjectsStatus::Success == ca->GetValue(v, "PropertyName") && !v.IsNull())
            timestampPropName.assign(v.GetUtf8CP());
    }

    auto classWriter = std::make_unique<ClassWriter>(ecClass.GetId(), op);

    // Assign every writable root property map to the table it is mapped to. Indices are
    // assigned up-front because they are the bit positions of the update bitmask.
    struct PendingProperty final {
        PropertyMap const* m_propMap = nullptr;
        DbTable const* m_table = nullptr;
        int m_index = -1;
    };

    std::vector<PendingProperty> pendingProps;
    for (auto propMap : classMap.GetPropertyMaps()) {
        if (!IsWritable(*propMap, timestampPropName, op))
            continue;

        auto table = GetPropertyTable(classMap, *propMap);
        if (table == nullptr || table->GetType() == DbTable::Type::Virtual)
            continue;

        PendingProperty pending;
        pending.m_propMap = propMap;
        pending.m_table = table;
        pending.m_index = (int)pendingProps.size();
        pendingProps.push_back(pending);
    }

    const auto maskWordCount = (size_t)((pendingProps.size() + Impl::kMaskBits - 1) / Impl::kMaskBits);
    classWriter->SetMaskWordCount(maskWordCount);

    auto const& systemSchemaHelper = ecdb.Schemas().Main().GetSystemSchemaHelper();
    auto ecInstanceIdPropMap = classMap.GetECInstanceIdPropertyMap();
    if (ecInstanceIdPropMap == nullptr) {
        error.Sprintf("Class '%s' has no ECInstanceId property map.", ecClass.GetFullName());
        return nullptr;
    }

    auto ecClassIdPropMap = classMap.GetECClassIdPropertyMap();

    // ClassMap::GetTables() returns the primary table first, followed by joined/overflow tables.
    for (auto table : classMap.GetTables()) {
        if (table->GetType() == DbTable::Type::Virtual)
            continue;

        // the TableWriter is only handed to the ClassWriter once its statement is successfully
        // prepared. Its address is stable across the move into the ClassWriter, so PropertyWriters
        // may reference it before then.
        auto tableWriterPtr = std::make_unique<TableWriter>(ecdb, *table, op);
        auto& tableWriter = *tableWriterPtr;
        std::vector<std::unique_ptr<PropertyWriter>> tableProperties;

        ECSqlPrepareContext ctx(tableWriter.GetECSqlStmt(), ecdb, ecdb.GetImpl().Issues());
        SyntheticScopeGuard scopeGuard(ctx, op);

        // <column name, parameter name> pairs and the root property index each pair belongs to
        struct TableColumn final {
            DbColumn const* m_column = nullptr;
            Utf8String m_paramName;
            int m_propertyIndex = -1;
        };
        std::vector<TableColumn> tableColumns;

        for (auto const& pending : pendingProps) {
            if (pending.m_table != table)
                continue;

            ECSqlBinder::SqlParamNameGenerator paramNameGen(ctx, "");
            std::unique_ptr<ECSqlBinder> binder;
            if (pending.m_propMap->IsSystem()) {
                auto const& sysPropInfo = systemSchemaHelper.GetSystemPropertyInfo(pending.m_propMap->GetProperty());
                binder = ECSqlBinderFactory::CreateIdBinder(ctx, *pending.m_propMap, sysPropInfo, paramNameGen);
            } else {
                binder = ECSqlBinderFactory::CreateBinder(ctx, *pending.m_propMap, paramNameGen);
            }

            if (binder == nullptr) {
                error.Sprintf("Failed to create a binder for property '%s.%s'.", ecClass.GetFullName(), pending.m_propMap->GetName().c_str());
                return nullptr;
            }

            std::vector<ColumnBinding> bindings;
            if (SUCCESS != CollectColumnBindings(bindings, *pending.m_propMap, *table, *binder, op)) {
                error.Sprintf("Failed to map the columns of property '%s.%s' to its binder parameters.", ecClass.GetFullName(), pending.m_propMap->GetName().c_str());
                return nullptr;
            }

            if (bindings.empty()) {
                // no physical column in this table (virtual column / no-op binder). The binder is
                // still registered so that callers can look the property up; binding to it is a no-op.
                tableProperties.push_back(std::make_unique<PropertyWriter>(tableWriter, *pending.m_propMap, std::move(binder), pending.m_index));
                continue;
            }

            for (auto& binding : bindings) {
                TableColumn tableColumn;
                tableColumn.m_column = binding.m_column;
                tableColumn.m_paramName = std::move(binding.m_paramName);
                tableColumn.m_propertyIndex = pending.m_index;
                tableColumns.push_back(std::move(tableColumn));
            }

            tableProperties.push_back(std::make_unique<PropertyWriter>(tableWriter, *pending.m_propMap, std::move(binder), pending.m_index));
            tableWriter.AddPropertyIndex(pending.m_index);
        }

        tableWriter.SetHasDataColumns(!tableColumns.empty());

        // the id column of this table
        auto idPropMap = ecInstanceIdPropMap->FindDataPropertyMap(*table);
        if (idPropMap == nullptr) {
            error.Sprintf("Class '%s' has no ECInstanceId column in table '%s'.", ecClass.GetFullName(), table->GetName().c_str());
            return nullptr;
        }

        Utf8String idParamName(":");
        idParamName.append(Impl::kInstanceIdParamName);

        NativeSqlBuilder builder;
        std::vector<Utf8String> maskParamNames;
        if (op == WriterOp::Insert) {
            builder.Append("INSERT INTO ").AppendEscaped(table->GetName()).Append(" (");
            builder.AppendEscaped(idPropMap->GetColumn().GetName());

            // ECClassId is written as a literal, never as a parameter
            if (ecClassIdPropMap != nullptr) {
                if (auto classIdPropMap = ecClassIdPropMap->FindDataPropertyMap(*table);
                    classIdPropMap != nullptr && classIdPropMap->GetColumn().GetPersistenceType() != PersistenceType::Virtual) {
                    builder.AppendComma().AppendEscaped(classIdPropMap->GetColumn().GetName());
                }
            }

            for (auto const& tableColumn : tableColumns)
                builder.AppendComma().AppendEscaped(tableColumn.m_column->GetName());

            builder.Append(") VALUES (").Append(idParamName);

            if (ecClassIdPropMap != nullptr) {
                if (auto classIdPropMap = ecClassIdPropMap->FindDataPropertyMap(*table);
                    classIdPropMap != nullptr && classIdPropMap->GetColumn().GetPersistenceType() != PersistenceType::Virtual) {
                    builder.AppendComma().Append(ecClass.GetId());
                }
            }

            for (auto const& tableColumn : tableColumns)
                builder.AppendComma().Append(tableColumn.m_paramName);

            builder.AppendParenRight();
        } else {
            if (tableColumns.empty()) {
                // nothing updatable in this table
                continue;
            }

            for (size_t word = 0; word < maskWordCount; ++word)
                maskParamNames.push_back(MakeMaskParamName(word));

            builder.Append("UPDATE ").AppendEscaped(table->GetName()).Append(" SET ");
            bool isFirst = true;
            for (auto const& tableColumn : tableColumns) {
                if (!isFirst)
                    builder.AppendComma();
                isFirst = false;

                const auto word = (size_t)(tableColumn.m_propertyIndex / Impl::kMaskBits);
                const uint64_t bit = (uint64_t)1 << (uint64_t)(tableColumn.m_propertyIndex % Impl::kMaskBits);

                Utf8String bitLiteral;
                bitLiteral.Sprintf("%" PRIu64, bit);

                builder.AppendEscaped(tableColumn.m_column->GetName())
                    .Append("=IIF(")
                    .Append(maskParamNames[word])
                    .Append("&")
                    .Append(bitLiteral)
                    .AppendComma()
                    .Append(tableColumn.m_paramName)
                    .AppendComma()
                    .AppendEscaped(tableColumn.m_column->GetName())
                    .AppendParenRight();
            }

            builder.Append(" WHERE ").AppendEscaped(idPropMap->GetColumn().GetName()).Append("=").Append(idParamName);
        }

        // guard against the sqlite parameter limit
        const int paramBudget = ecdb.GetLimit(DbLimits::VariableNumber) / Impl::kParamBudgetDivisor;
        const int requiredParams = (int)tableColumns.size() + (int)maskParamNames.size() + 1;
        if (paramBudget > 0 && requiredParams > paramBudget) {
            error.Sprintf("Class '%s' requires %d SQLite parameters for table '%s' which exceeds the budget of %d.",
                          ecClass.GetFullName(), requiredParams, table->GetName().c_str(), paramBudget);
            return nullptr;
        }

        if (BE_SQLITE_OK != tableWriter.Prepare(ecdb, builder.GetSql())) {
            error.Sprintf("Failed to prepare the %s statement for class '%s' and table '%s'.",
                          op == WriterOp::Insert ? "INSERT" : "UPDATE", ecClass.GetFullName(), table->GetName().c_str());
            return nullptr;
        }

        auto& sqliteStmt = tableWriter.GetSqliteStmt();
        // sqlite3_bind_parameter_index requires the name including its leading ':' prefix.
        tableWriter.SetInstanceIdParamIndex(sqliteStmt.GetParameterIndex(idParamName.c_str()));

        std::vector<int> maskParamIndices;
        for (auto const& maskParamName : maskParamNames)
            maskParamIndices.push_back(sqliteStmt.GetParameterIndex(maskParamName.c_str()));
        tableWriter.SetMaskParamIndices(std::move(maskParamIndices));

        classWriter->AddTable(std::move(tableWriterPtr));
        for (auto& tableProperty : tableProperties)
            classWriter->AddProperty(std::move(tableProperty));
    }

    if (classWriter->GetTables().empty()) {
        error.Sprintf("Class '%s' has no writable table.", ecClass.GetFullName());
        return nullptr;
    }

    classWriter->BuildIndexes();
    classWriter->RegisterStepHooks();
    return classWriter;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void Writer::SetError(Utf8CP fmt, ...) const {
    va_list args;
    va_start(args, fmt);
    m_error.VSprintf(fmt, args);
    va_end(args);
    ECDbLogger::Get().errorv("BulkInstanceWriter: %s", m_error.c_str());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void Writer::Clear() const {
    BeMutexHolder holder(m_mutex);
    m_cache.clear();
    m_mru.clear();
    m_error.clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Writer::Writer(ECDbCR conn, uint32_t maxCache) : m_conn(conn), m_maxCache(maxCache == 0 ? 1 : maxCache) {
    m_mru.reserve(m_maxCache);
    // the cached statements and binders refer to class maps, so they must be dropped whenever the
    // ECDb caches are cleared (e.g. after a schema import).
    const_cast<ECDbR>(m_conn).AddECDbCacheClearListener(*this);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Writer::~Writer() {
    const_cast<ECDbR>(m_conn).RemoveECDbCacheClearListener(*this);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void Writer::TouchMru(CacheKey const& key) const {
    auto it = std::find(m_mru.begin(), m_mru.end(), key);
    if (it != m_mru.end())
        m_mru.erase(it);

    m_mru.push_back(key);
    while (m_mru.size() > (size_t)m_maxCache) {
        m_cache.erase(m_mru.front());
        m_mru.erase(m_mru.begin());
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ClassWriter const* Writer::GetOrAdd(ECN::ECClassId classId, WriterOp op) const {
    const CacheKey key(classId, op);
    const auto it = m_cache.find(key);
    if (it != m_cache.end()) {
        TouchMru(key);
        return it->second.get();
    }

    auto ecClass = m_conn.Schemas().GetClass(classId);
    if (ecClass == nullptr) {
        SetError("ECClass with id %s does not exist.", classId.ToHexStr().c_str());
        return nullptr;
    }

    if (!ecClass->IsEntityClass() && !ecClass->IsRelationshipClass()) {
        SetError("Class '%s' is neither an entity nor a relationship class and cannot be written.", ecClass->GetFullName());
        return nullptr;
    }

    auto classMap = m_conn.Schemas().Main().GetClassMap(*ecClass);
    if (classMap == nullptr) {
        SetError("Class '%s' is not mapped.", ecClass->GetFullName());
        return nullptr;
    }

    Utf8String error;
    auto classWriter = ClassWriter::Factory::Create(m_conn, *classMap, op, error);
    if (classWriter == nullptr) {
        SetError("%s", error.empty() ? "Failed to create the class writer." : error.c_str());
        return nullptr;
    }

    auto const newIt = m_cache.insert(std::make_pair(key, std::move(classWriter)));
    TouchMru(key);
    return newIt.first->second.get();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Writer::CheckWritePermission() const {
    const auto writeToken = m_conn.GetImpl().GetSettingsManager().GetCrudWriteToken();
    const auto policy = PolicyManager::GetPolicy(ECCrudPermissionPolicyAssertion(m_conn, true, writeToken));
    if (!policy.IsSupported()) {
        SetError("%s", policy.GetNotSupportedMessage().c_str());
        return BE_SQLITE_ERROR;
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool Writer::IsTableDirty(TableWriter const& table, WriteContext const& ctx) const {
    for (auto propertyIndex : table.GetPropertyIndices()) {
        if (ctx.IsWritten(propertyIndex))
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Writer::BindMasks(TableWriter const& table, WriteContext const& ctx) const {
    auto const& maskParamIndices = table.GetMaskParamIndices();
    auto const& masks = ctx.GetMasks();
    for (size_t i = 0; i < maskParamIndices.size(); ++i) {
        if (maskParamIndices[i] <= 0)
            continue; // this mask word is not referenced by this statement

        const uint64_t mask = i < masks.size() ? masks[i] : 0;
        const auto rc = table.GetSqliteStmt().BindInt64(maskParamIndices[i], (int64_t)mask);
        if (rc != BE_SQLITE_OK) {
            SetError("Failed to bind the update bitmask for table '%s'.", table.GetTable().GetName().c_str());
            return rc;
        }
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Writer::Insert(ECN::ECClassId classId, BindCallback const& callback, InsertOptions const& options, ECInstanceKey& key) const {
    BeMutexHolder holder(m_mutex);
    m_error.clear();
    key = ECInstanceKey();

    if (const auto rc = CheckWritePermission(); rc != BE_SQLITE_OK)
        return rc;

    auto classWriter = GetOrAdd(classId, WriterOp::Insert);
    if (classWriter == nullptr)
        return BE_SQLITE_ERROR;

    classWriter->ResetStatements();

    ECInstanceId instanceId = options.GetInstanceId();
    if (options.GetInstanceIdMode() == InsertOptions::InstanceIdMode::Auto || !instanceId.IsValid()) {
        // class writers are always resolved through the main schema manager, so the row always
        // goes into the main table space and the instance id sequence can be used.
        if (const auto rc = m_conn.GetImpl().GetInstanceIdSequence().GetNextValue(instanceId); rc != BE_SQLITE_OK) {
            SetError("Could not generate an ECInstanceId.");
            return rc;
        }
    }

    WriteContext ctx(*classWriter);
    if (callback != nullptr)
        callback(ctx);

    if (const auto stat = classWriter->OnBeforeFirstStep(); !stat.IsSuccess()) {
        if (m_error.empty())
            SetError("Failed to finalize the bound values before inserting an instance of class id %s.", classId.ToHexStr().c_str());

        classWriter->ResetStatements();
        return BE_SQLITE_ERROR;
    }

    for (auto const& table : classWriter->GetTables()) {
        auto& sqliteStmt = table->GetSqliteStmt();
        if (table->GetInstanceIdParamIndex() > 0) {
            if (const auto rc = sqliteStmt.BindId(table->GetInstanceIdParamIndex(), instanceId); rc != BE_SQLITE_OK) {
                SetError("Failed to bind the ECInstanceId for table '%s'.", table->GetTable().GetName().c_str());
                classWriter->ResetStatements();
                return rc;
            }
        }

        const auto rc = sqliteStmt.Step();
        if (rc != BE_SQLITE_DONE) {
            SetError("Failed to insert into table '%s': %s", table->GetTable().GetName().c_str(), BeSQLiteLib::GetErrorName(rc));
            classWriter->ResetStatements();
            return rc;
        }
    }

    classWriter->ResetStatements();
    key = ECInstanceKey(classId, instanceId);
    return BE_SQLITE_DONE;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Writer::Update(ECInstanceKeyCR key, BindCallback const& callback, UpdateOptions const& options) const {
    BeMutexHolder holder(m_mutex);
    m_error.clear();

    if (!key.IsValid()) {
        SetError("Cannot update an instance without a valid ECInstanceKey.");
        return BE_SQLITE_ERROR;
    }

    if (const auto rc = CheckWritePermission(); rc != BE_SQLITE_OK)
        return rc;

    auto classWriter = GetOrAdd(key.GetClassId(), WriterOp::Update);
    if (classWriter == nullptr)
        return BE_SQLITE_ERROR;

    classWriter->ResetStatements();

    WriteContext ctx(*classWriter);
    if (callback != nullptr)
        callback(ctx);

    if (!ctx.HasAnyWritten()) {
        // nothing to do, a partial update without any property is a no-op
        return BE_SQLITE_DONE;
    }

    if (const auto stat = classWriter->OnBeforeFirstStep(); !stat.IsSuccess()) {
        if (m_error.empty())
            SetError("Failed to finalize the bound values before updating instance %s.", key.GetInstanceId().ToHexStr().c_str());

        classWriter->ResetStatements();
        return BE_SQLITE_ERROR;
    }

    int totalModifiedRows = 0;
    for (auto const& table : classWriter->GetTables()) {
        if (!IsTableDirty(*table, ctx))
            continue;

        auto& sqliteStmt = table->GetSqliteStmt();
        if (const auto rc = BindMasks(*table, ctx); rc != BE_SQLITE_OK) {
            classWriter->ResetStatements();
            return rc;
        }

        if (table->GetInstanceIdParamIndex() > 0) {
            if (const auto rc = sqliteStmt.BindId(table->GetInstanceIdParamIndex(), key.GetInstanceId()); rc != BE_SQLITE_OK) {
                SetError("Failed to bind the ECInstanceId for table '%s'.", table->GetTable().GetName().c_str());
                classWriter->ResetStatements();
                return rc;
            }
        }

        const auto rc = sqliteStmt.Step();
        if (rc != BE_SQLITE_DONE) {
            SetError("Failed to update table '%s': %s", table->GetTable().GetName().c_str(), BeSQLiteLib::GetErrorName(rc));
            classWriter->ResetStatements();
            return rc;
        }

        totalModifiedRows += m_conn.GetModifiedRowCount();
    }

    classWriter->ResetStatements();

    if (options.GetFailIfNoRowChanged() && totalModifiedRows == 0) {
        SetError("Instance %s of class id %s does not exist.", key.GetInstanceId().ToHexStr().c_str(), key.GetClassId().ToHexStr().c_str());
        return BE_SQLITE_NOTFOUND;
    }

    return BE_SQLITE_DONE;
}

//****************************** BulkInstanceWriter ********************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BulkInstanceWriter::BulkInstanceWriter(ECDbCR ecdb, uint32_t cacheSize) : m_pImpl(new Impl(ecdb, cacheSize)) {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BulkInstanceWriter::~BulkInstanceWriter() {
    delete m_pImpl;
    m_pImpl = nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult BulkInstanceWriter::Insert(ECN::ECClassId classId, BindCallback callback, InsertOptions const& options, ECInstanceKey& key) {
    return m_pImpl->Insert(classId, callback, options, key);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult BulkInstanceWriter::Insert(ECN::ECClassId classId, BindCallback callback, InsertOptions const& options) {
    ECInstanceKey key;
    return m_pImpl->Insert(classId, callback, options, key);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult BulkInstanceWriter::Update(ECInstanceKeyCR key, BindCallback callback, UpdateOptions const& options) {
    return m_pImpl->Update(key, callback, options);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult BulkInstanceWriter::Update(ECInstanceKeyCR key, BindCallback callback) {
    UpdateOptions options;
    return m_pImpl->Update(key, callback, options);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR BulkInstanceWriter::GetLastError() const { return m_pImpl->GetLastError(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void BulkInstanceWriter::Reset() { m_pImpl->Reset(); }

END_BENTLEY_SQLITE_EC_NAMESPACE
