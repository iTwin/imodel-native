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
using ClassSchema = BulkInstanceWriter::Impl::ClassSchema;
using TableWriter = BulkInstanceWriter::Impl::TableWriter;
using PropertyWriter = BulkInstanceWriter::Impl::PropertyWriter;
using PendingProperty = BulkInstanceWriter::Impl::PendingProperty;
using StatementFactory = BulkInstanceWriter::Impl::StatementFactory;
using ClassWriter = BulkInstanceWriter::Impl::ClassWriter;
using LevelWriter = BulkInstanceWriter::Impl::LevelWriter;
using LevelKey = BulkInstanceWriter::Impl::LevelKey;
using ClassUpdatePlan = BulkInstanceWriter::Impl::ClassUpdatePlan;
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

//=======================================================================================
//! Clears a flag when it goes out of scope, so that an exception thrown by a caller's bind
//! callback cannot leave the writer permanently marked as busy.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct FlagGuard final {
private:
    bool& m_flag;

public:
    explicit FlagGuard(bool& flag) : m_flag(flag) { m_flag = true; }
    ~FlagGuard() { m_flag = false; }
    FlagGuard(FlagGuard const&) = delete;
    FlagGuard& operator=(FlagGuard const&) = delete;
};

//****************************** ClassSchema ***************************

//---------------------------------------------------------------------------------------
//! Groups the properties into hierarchy levels, ordered root -> leaf. A property belongs to
//! the class that declares it, following the base property chain to its root so that an
//! override groups with the property whose column it inherits.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ClassSchema::BuildLevels(ECN::ECClassCR concreteClass) {
    m_levels.clear();
    m_levelByPropertyIndex.assign(m_propertiesByIndex.size(), -1);

    struct Group final {
        ECN::ECClassCP m_class = nullptr;
        int m_depth = 0;
        std::vector<int> m_propertyIndices;
    };

    std::vector<Group> groups;
    for (size_t i = 0; i < m_propertiesByIndex.size(); ++i) {
        auto declaringClass = StatementFactory::GetDeclaringClass(concreteClass, m_propertiesByIndex[i]->GetProperty());
        if (declaringClass == nullptr)
            continue;

        auto it = std::find_if(groups.begin(), groups.end(), [declaringClass](Group const& g) { return g.m_class == declaringClass; });
        if (it == groups.end()) {
            Group group;
            group.m_class = declaringClass;
            // computed once per level, never inside the comparator
            group.m_depth = StatementFactory::GetClassDepth(*declaringClass);
            group.m_propertyIndices.push_back((int)i);
            groups.push_back(std::move(group));
            continue;
        }

        it->m_propertyIndices.push_back((int)i);
    }

    // root -> leaf. std::stable_sort keeps the discovery order of classes at the same depth,
    // which keeps the level order deterministic for a given class map.
    std::stable_sort(groups.begin(), groups.end(), [](Group const& lhs, Group const& rhs) { return lhs.m_depth < rhs.m_depth; });

    for (auto& group : groups) {
        Level level;
        level.m_class = group.m_class;
        level.m_propertyIndices = std::move(group.m_propertyIndices);
        for (auto propIndex : level.m_propertyIndices)
            m_levelByPropertyIndex[(size_t)propIndex] = (int)m_levels.size();

        m_levels.push_back(std::move(level));
    }
}

//****************************** TableWriter ***************************

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

//****************************** StatementFactory **********************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool StatementFactory::IsSystemIdPropertyMap(PropertyMap const& propMap) {
    return propMap.GetType() == PropertyMap::Type::ECInstanceId || propMap.GetType() == PropertyMap::Type::ECClassId;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool StatementFactory::IsWritable(PropertyMap const& propMap, Utf8StringCR timestampPropName, WriterOp op) {
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
DbTable const* StatementFactory::GetPropertyTable(ClassMapCR classMap, PropertyMap const& propMap) {
    if (propMap.IsData())
        return &propMap.GetAs<DataPropertyMap>().GetTable();

    // system property maps (constraint ids of link tables) live in the primary table
    return &classMap.GetPrimaryTable();
}

//---------------------------------------------------------------------------------------
//! The shallowest non mixin class in @p ecClass's ancestry that still carries @p name.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static ECN::ECClassCP FindShallowestNonMixinOwner(ECN::ECClassCR ecClass, Utf8CP name) {
    for (ECN::ECClassCP baseClass : ecClass.GetBaseClasses()) {
        if (baseClass->IsMixin())
            continue;

        if (baseClass->GetPropertyP(name, true) == nullptr)
            continue;

        return FindShallowestNonMixinOwner(*baseClass, name);
    }

    return &ecClass;
}

//---------------------------------------------------------------------------------------
//! The level a property belongs to. A level has to be a class that actually owns storage,
//! so mixins are never levels: a mixin is an interface whose properties are merged into and
//! mapped by the entity class that implements it. The level is therefore the root most non
//! mixin class along the property's override chain, and when the property is only ever
//! declared by mixins it is the shallowest non mixin ancestor that carries it, which is the
//! class whose class map assigned the column.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECN::ECClassCP StatementFactory::GetDeclaringClass(ECN::ECClassCR concreteClass, ECN::ECPropertyCR prop) {
    ECN::ECClassCP rootMostNonMixin = nullptr;
    ECN::ECPropertyCP current = &prop;
    // guarded against a malformed cyclic override chain
    for (int guard = 0; guard < 256; ++guard) {
        if (!current->GetClass().IsMixin())
            rootMostNonMixin = &current->GetClass();

        auto base = current->GetBaseProperty();
        if (base == nullptr || base == current)
            break;

        current = base;
    }

    if (rootMostNonMixin != nullptr)
        return rootMostNonMixin;

    return FindShallowestNonMixinOwner(concreteClass, prop.GetName().c_str());
}

//---------------------------------------------------------------------------------------
//! Longest base class chain of @p ecClass. Mixins are skipped because they carry no storage
//! and would otherwise distort the root -> leaf order of the levels. Memoized because BIS
//! classes have several base classes each, which would make a plain recursion exponential.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static int GetClassDepthMemoized(ECN::ECClassCR ecClass, std::map<ECN::ECClassCP, int>& memo) {
    const auto it = memo.find(&ecClass);
    if (it != memo.end())
        return it->second;

    // seeded before recursing so that a malformed cyclic hierarchy terminates
    memo[&ecClass] = 0;

    int maxBaseDepth = -1;
    for (ECN::ECClassCP baseClass : ecClass.GetBaseClasses()) {
        if (baseClass->IsMixin())
            continue;

        const auto baseDepth = GetClassDepthMemoized(*baseClass, memo);
        if (baseDepth > maxBaseDepth)
            maxBaseDepth = baseDepth;
    }

    const int depth = maxBaseDepth + 1;
    memo[&ecClass] = depth;
    return depth;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
int StatementFactory::GetClassDepth(ECN::ECClassCR ecClass) {
    std::map<ECN::ECClassCP, int> memo;
    return GetClassDepthMemoized(ecClass, memo);
}

//---------------------------------------------------------------------------------------
//! Appends "<table>.<column>," of every non virtual column of the property map. Two classes
//! produce the same signature for a level exactly when they map the level's properties onto
//! the very same columns in the same order, which is precisely when the level's statement can
//! be shared between them.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
void StatementFactory::AppendColumnSignature(Utf8StringR signature, ClassMapCR classMap, PropertyMap const& propMap) {
    auto table = GetPropertyTable(classMap, propMap);
    if (table == nullptr)
        return;

    GetColumnsPropertyMapVisitor columnVisitor(*table, PropertyMap::Type::All);
    if (SUCCESS != propMap.AcceptVisitor(columnVisitor))
        return;

    for (auto col : columnVisitor.GetColumns()) {
        if (col->GetPersistenceType() == PersistenceType::Virtual)
            continue;

        signature.append(table->GetName()).append(".").append(col->GetName()).append(",");
    }
}

//---------------------------------------------------------------------------------------
//! Pairs the non virtual columns of a property map within one table with the sqlite
//! parameter names the binder generated for it. Both lists are produced by the very same
//! property map traversal order, and binders that map to a virtual column generate no
//! parameter name at all (no-op binders), so the two lists line up 1:1.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus StatementFactory::CollectColumnBindings(std::vector<ColumnBinding>& bindings, PropertyMap const& propMap, DbTable const& table, ECSqlBinder const& binder, WriterOp op) {
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
//! Root property maps the writer may write, in the order that defines their index. Every
//! caller runs this, so the index space is stable.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
void StatementFactory::CollectWritableProperties(ClassMapCR classMap, WriterOp op, std::vector<PropertyMap const*>& propMaps) {
    Utf8String timestampPropName;
    if (auto ca = classMap.GetClass().GetCustomAttribute("CoreCustomAttributes", "ClassHasCurrentTimeStampProperty"); ca != nullptr) {
        ECValue v;
        if (ECObjectsStatus::Success == ca->GetValue(v, "PropertyName") && !v.IsNull())
            timestampPropName.assign(v.GetUtf8CP());
    }

    for (auto propMap : classMap.GetPropertyMaps()) {
        if (!IsWritable(*propMap, timestampPropName, op))
            continue;

        auto table = GetPropertyTable(classMap, *propMap);
        if (table == nullptr || table->GetType() == DbTable::Type::Virtual)
            continue;

        propMaps.push_back(propMap);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
void StatementFactory::CollectPendingProperties(ClassMapCR classMap, WriterOp op, std::vector<PendingProperty>& pendingProps) {
    std::vector<PropertyMap const*> writableProps;
    CollectWritableProperties(classMap, op, writableProps);

    for (size_t i = 0; i < writableProps.size(); ++i) {
        PendingProperty pending;
        pending.m_propMap = writableProps[i];
        pending.m_table = GetPropertyTable(classMap, *writableProps[i]);
        pending.m_index = (int)i;
        pendingProps.push_back(pending);
    }
}

//---------------------------------------------------------------------------------------
//! Creates the binders of @p pending, renders one statement per table they are mapped to and
//! prepares it. Shared by the INSERT writer and by every UPDATE level segment; the only
//! difference between the two is the statement that is rendered.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus StatementFactory::Build(ECDbCR ecdb, ClassMapCR classMap, WriterOp op, std::vector<PendingProperty> const& pendingProps,
                                      std::vector<TableWriter::Ptr>& outTables, std::vector<PropertyWriter::Ptr>& outProperties, Utf8StringR error) {
    auto const& ecClass = classMap.GetClass();
    auto const& systemSchemaHelper = ecdb.Schemas().Main().GetSystemSchemaHelper();
    auto ecInstanceIdPropMap = classMap.GetECInstanceIdPropertyMap();
    if (ecInstanceIdPropMap == nullptr) {
        error.Sprintf("Class '%s' has no ECInstanceId property map.", ecClass.GetFullName());
        return ERROR;
    }

    auto ecClassIdPropMap = classMap.GetECClassIdPropertyMap();

    // ClassMap::GetTables() returns the primary table first, followed by joined/overflow tables.
    for (auto table : classMap.GetTables()) {
        if (table->GetType() == DbTable::Type::Virtual)
            continue;

        // the TableWriter is only appended to the output once its statement is successfully
        // prepared. Its address is stable across the move, so PropertyWriters may reference it
        // before then.
        auto tableWriterPtr = std::make_unique<TableWriter>(ecdb, *table, op);
        auto& tableWriter = *tableWriterPtr;
        std::vector<PropertyWriter::Ptr> tableProperties;

        ECSqlPrepareContext ctx(tableWriter.GetECSqlStmt(), ecdb, ecdb.GetImpl().Issues());
        SyntheticScopeGuard scopeGuard(ctx, op);

        // <column name, parameter name> pairs
        struct TableColumn final {
            DbColumn const* m_column = nullptr;
            Utf8String m_paramName;
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
                return ERROR;
            }

            std::vector<ColumnBinding> bindings;
            if (SUCCESS != CollectColumnBindings(bindings, *pending.m_propMap, *table, *binder, op)) {
                error.Sprintf("Failed to map the columns of property '%s.%s' to its binder parameters.", ecClass.GetFullName(), pending.m_propMap->GetName().c_str());
                return ERROR;
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
                tableColumns.push_back(std::move(tableColumn));
            }

            tableProperties.push_back(std::make_unique<PropertyWriter>(tableWriter, *pending.m_propMap, std::move(binder), pending.m_index));
        }

        // the id column of this table
        auto idPropMap = ecInstanceIdPropMap->FindDataPropertyMap(*table);
        if (idPropMap == nullptr) {
            error.Sprintf("Class '%s' has no ECInstanceId column in table '%s'.", ecClass.GetFullName(), table->GetName().c_str());
            return ERROR;
        }

        Utf8String idParamName(":");
        idParamName.append(Impl::kInstanceIdParamName);

        NativeSqlBuilder builder;
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
                // nothing of this segment lives in this table
                continue;
            }

            // the statement covers exactly the columns of the properties it was built for, so an
            // untouched hierarchy level's columns are never mentioned and keep their value.
            builder.Append("UPDATE ").AppendEscaped(table->GetName()).Append(" SET ");
            bool isFirst = true;
            for (auto const& tableColumn : tableColumns) {
                if (!isFirst)
                    builder.AppendComma();
                isFirst = false;

                builder.AppendEscaped(tableColumn.m_column->GetName()).Append("=").Append(tableColumn.m_paramName);
            }

            builder.Append(" WHERE ").AppendEscaped(idPropMap->GetColumn().GetName()).Append("=").Append(idParamName);
        }

        // guard against the sqlite parameter limit
        const int paramBudget = ecdb.GetLimit(DbLimits::VariableNumber) / Impl::kParamBudgetDivisor;
        const int requiredParams = (int)tableColumns.size() + 1;
        if (paramBudget > 0 && requiredParams > paramBudget) {
            error.Sprintf("Class '%s' requires %d SQLite parameters for table '%s' which exceeds the budget of %d.",
                          ecClass.GetFullName(), requiredParams, table->GetName().c_str(), paramBudget);
            return ERROR;
        }

        if (BE_SQLITE_OK != tableWriter.Prepare(ecdb, builder.GetSql())) {
            error.Sprintf("Failed to prepare the %s statement for class '%s' and table '%s'.",
                          op == WriterOp::Insert ? "INSERT" : "UPDATE", ecClass.GetFullName(), table->GetName().c_str());
            return ERROR;
        }

        auto& sqliteStmt = tableWriter.GetSqliteStmt();
        // sqlite3_bind_parameter_index requires the name including its leading ':' prefix.
        tableWriter.SetInstanceIdParamIndex(sqliteStmt.GetParameterIndex(idParamName.c_str()));

        outTables.push_back(std::move(tableWriterPtr));
        for (auto& tableProperty : tableProperties)
            outProperties.push_back(std::move(tableProperty));
    }

    return SUCCESS;
}

//****************************** ClassWriter ***************************

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
void ClassWriter::BuildIndexes() {
    m_propertiesByIndex.clear();
    m_propertiesByIndex.resize((size_t)m_schema->GetPropertyCount(), nullptr);
    for (auto const& prop : m_properties) {
        if (prop->GetIndex() >= 0 && prop->GetIndex() < (int)m_propertiesByIndex.size())
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
void ClassWriter::ResetStatements() const {
    for (auto const& table : m_tables)
        table->Reset();

    for (auto binder : m_bindersToCallOnClearBindings)
        binder->OnClearBindings();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ClassSchema::Ptr ClassWriter::Factory::CreateSchema(ClassMapCR classMap, WriterOp op, Utf8StringR error) {
    auto const& ecClass = classMap.GetClass();
    if (classMap.GetType() == ClassMap::Type::RelationshipEndTable) {
        error.Sprintf("BulkInstanceWriter does not support foreign key (end table) relationship class '%s'. Use ECSQL or InstanceWriter instead.", ecClass.GetFullName());
        return nullptr;
    }

    if (classMap.GetPrimaryTable().GetType() == DbTable::Type::Virtual) {
        error.Sprintf("Class '%s' is not mapped to a real table and cannot be written.", ecClass.GetFullName());
        return nullptr;
    }

    std::vector<PropertyMap const*> propMaps;
    StatementFactory::CollectWritableProperties(classMap, op, propMaps);

    auto schema = std::make_shared<ClassSchema>(ecClass.GetId());
    for (auto propMap : propMaps)
        schema->Add(*propMap);

    schema->BuildLevels(ecClass);
    return schema;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ClassWriter::Ptr ClassWriter::Factory::Create(ECDbCR ecdb, ClassMapCR classMap, ClassSchema::Ptr schema, Utf8StringR error) {
    auto const& ecClass = classMap.GetClass();
    if (classMap.GetType() == ClassMap::Type::RelationshipEndTable) {
        error.Sprintf("BulkInstanceWriter does not support foreign key (end table) relationship class '%s'. Use ECSQL or InstanceWriter instead.", ecClass.GetFullName());
        return nullptr;
    }

    if (classMap.GetPrimaryTable().GetType() == DbTable::Type::Virtual) {
        error.Sprintf("Class '%s' is not mapped to a real table and cannot be written.", ecClass.GetFullName());
        return nullptr;
    }

    std::vector<PendingProperty> pendingProps;
    StatementFactory::CollectPendingProperties(classMap, WriterOp::Insert, pendingProps);

    std::vector<TableWriter::Ptr> tables;
    std::vector<PropertyWriter::Ptr> properties;
    if (SUCCESS != StatementFactory::Build(ecdb, classMap, WriterOp::Insert, pendingProps, tables, properties, error))
        return nullptr;

    if (tables.empty()) {
        error.Sprintf("Class '%s' has no writable table.", ecClass.GetFullName());
        return nullptr;
    }

    auto classWriter = std::make_shared<ClassWriter>(ecClass.GetId(), std::move(schema));
    classWriter->SetTables(std::move(tables));
    classWriter->SetProperties(std::move(properties));
    classWriter->BuildIndexes();
    classWriter->RegisterStepHooks();
    return classWriter;
}

//****************************** LevelWriter ***************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void LevelWriter::BuildIndexes() {
    m_propertiesByName.clear();
    for (auto const& prop : m_properties)
        m_propertiesByName[prop->GetName().c_str()] = prop.get();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void LevelWriter::RegisterStepHooks() {
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
ECSqlStatus LevelWriter::OnBeforeFirstStep() const {
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
void LevelWriter::ResetStatements() const {
    for (auto const& table : m_tables)
        table->Reset();

    for (auto binder : m_bindersToCallOnClearBindings)
        binder->OnClearBindings();
}

//---------------------------------------------------------------------------------------
//! Builds the UPDATE statement(s) of one hierarchy level. The level's properties may still
//! span the primary table and an overflow table, so this can produce more than one statement.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
LevelWriter::Ptr LevelWriter::Factory::Create(ECDbCR ecdb, ClassMapCR classMap, ECN::ECClassCR levelClass, std::vector<PendingProperty> const& pendingProps, Utf8StringR error) {
    std::vector<TableWriter::Ptr> tables;
    std::vector<PropertyWriter::Ptr> properties;
    if (SUCCESS != StatementFactory::Build(ecdb, classMap, WriterOp::Update, pendingProps, tables, properties, error))
        return nullptr;

    auto levelWriter = std::make_shared<LevelWriter>(levelClass.GetId());
    levelWriter->SetTables(std::move(tables));
    levelWriter->SetProperties(std::move(properties));
    levelWriter->BuildIndexes();
    levelWriter->RegisterStepHooks();
    return levelWriter;
}

//****************************** ClassUpdatePlan ***********************

//---------------------------------------------------------------------------------------
//! Resolves the property index space of the class onto the binders of the level segments.
//! Segments are shared, so their own property indices are meaningless here and the properties
//! are matched by name, which is unique within a class.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ClassUpdatePlan::BuildIndexes() {
    const auto propertyCount = (size_t)m_schema->GetPropertyCount();
    m_levelSlotByPropertyIndex.assign(propertyCount, -1);
    m_propertiesByIndex.assign(propertyCount, nullptr);

    for (size_t slot = 0; slot < m_levels.size(); ++slot) {
        auto level = m_levels[slot].get();
        if (level == nullptr)
            continue;

        auto schemaLevel = m_schema->GetLevel((int)slot);
        if (schemaLevel == nullptr)
            continue;

        for (auto propIndex : schemaLevel->m_propertyIndices) {
            if (propIndex < 0 || propIndex >= (int)propertyCount)
                continue;

            auto propMap = m_schema->GetPropertyMap(propIndex);
            if (propMap == nullptr)
                continue;

            m_levelSlotByPropertyIndex[(size_t)propIndex] = (int)slot;
            m_propertiesByIndex[(size_t)propIndex] = level->FindProperty(propMap->GetName().c_str());
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ClassUpdatePlan::ResetStatements() const {
    for (auto const& level : m_levels) {
        if (level != nullptr)
            level->ResetStatements();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ClassUpdatePlan::Ptr ClassUpdatePlan::Factory::Create(ECDbCR ecdb, ClassMapCR classMap, ClassSchema::Ptr schema, LevelResolver const& levelResolver, Utf8StringR error) {
    auto const& ecClass = classMap.GetClass();
    if (classMap.GetType() == ClassMap::Type::RelationshipEndTable) {
        error.Sprintf("BulkInstanceWriter does not support foreign key (end table) relationship class '%s'. Use ECSQL or InstanceWriter instead.", ecClass.GetFullName());
        return nullptr;
    }

    if (classMap.GetPrimaryTable().GetType() == DbTable::Type::Virtual) {
        error.Sprintf("Class '%s' is not mapped to a real table and cannot be written.", ecClass.GetFullName());
        return nullptr;
    }

    std::vector<PendingProperty> allPending;
    StatementFactory::CollectPendingProperties(classMap, WriterOp::Update, allPending);

    std::vector<LevelWriter::Ptr> levels;
    levels.reserve((size_t)schema->GetLevelCount());

    for (auto const& schemaLevel : schema->GetLevels()) {
        std::vector<PendingProperty> levelPending;
        Utf8String columnSignature;
        for (auto propIndex : schemaLevel.m_propertyIndices) {
            auto it = std::find_if(allPending.begin(), allPending.end(), [propIndex](PendingProperty const& p) { return p.m_index == propIndex; });
            if (it == allPending.end())
                continue;

            levelPending.push_back(*it);
            StatementFactory::AppendColumnSignature(columnSignature, classMap, *it->m_propMap);
        }

        if (levelPending.empty() || columnSignature.empty()) {
            // the level has no physically writable column, so there is nothing to step for it
            levels.push_back(nullptr);
            continue;
        }

        auto levelWriter = levelResolver(LevelKey(schemaLevel.m_class->GetId(), std::move(columnSignature)), *schemaLevel.m_class, levelPending);
        if (levelWriter == nullptr) {
            if (error.empty())
                error.Sprintf("Failed to create the update statement of level '%s' of class '%s'.", schemaLevel.m_class->GetFullName(), ecClass.GetFullName());

            return nullptr;
        }

        levels.push_back(levelWriter->IsEmpty() ? nullptr : levelWriter);
    }

    auto plan = std::make_shared<ClassUpdatePlan>(ecClass.GetId(), std::move(schema));
    plan->SetLevels(std::move(levels));
    plan->BuildIndexes();
    return plan;
}

//****************************** WriteContext **************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WriteContext::MarkWritten(int propertyIndex) const {
    m_anyWritten = true;
    if (m_plan == nullptr)
        return;

    const auto slot = m_plan->GetLevelSlotOf(propertyIndex);
    if (slot >= 0 && slot < (int)m_dirtyLevels.size())
        m_dirtyLevels[(size_t)slot] = true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlBinder& WriteContext::_GetBinder(int propertyIndex) const {
    if (propertyIndex < 0 || propertyIndex >= m_schema.GetPropertyCount())
        return NoopECSqlBinder::Get();

    MarkWritten(propertyIndex);

    PropertyWriter const* prop = nullptr;
    if (m_plan != nullptr)
        prop = m_plan->GetProperty(propertyIndex);
    else if (m_insertWriter != nullptr)
        prop = m_insertWriter->GetProperty(propertyIndex);

    // no binder means the property maps to no physical column, so writing it is a no-op
    if (prop == nullptr)
        return NoopECSqlBinder::Get();

    return prop->GetBinder();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlBinder* WriteContext::_FindBinder(Utf8CP propertyName) const {
    const auto index = m_schema.GetIndexOf(propertyName);
    if (index < 0)
        return nullptr;

    return &_GetBinder(index);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECPropertyCP WriteContext::_GetProperty(int propertyIndex) const {
    const auto propMap = m_schema.GetPropertyMap(propertyIndex);
    return propMap == nullptr ? nullptr : &propMap->GetProperty();
}

//****************************** Writer ********************************

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
    m_schemaCache.clear();
    m_schemaMru.clear();
    m_insertCache.clear();
    m_insertMru.clear();
    m_planCache.clear();
    m_planMru.clear();
    m_levelCache.clear();
    m_levelMru.clear();
    m_error.clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Writer::Writer(ECDbCR conn, uint32_t maxCache) : m_conn(conn), m_maxCache(maxCache == 0 ? 1 : maxCache) {
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
ClassMap const* Writer::GetClassMap(ECN::ECClassId classId) const {
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

    return classMap;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ClassSchema::Ptr Writer::GetOrAddSchema(ECN::ECClassId classId, ClassMapCR classMap, WriterOp op) const {
    // Insert and Update select the same property maps, so one schema per class is enough.
    const auto it = m_schemaCache.find(classId);
    if (it != m_schemaCache.end()) {
        TouchMru(m_schemaCache, m_schemaMru, classId, (size_t)m_maxCache);
        return it->second;
    }

    Utf8String error;
    auto schema = ClassWriter::Factory::CreateSchema(classMap, op, error);
    if (schema == nullptr) {
        SetError("%s", error.empty() ? "Failed to describe the class." : error.c_str());
        return nullptr;
    }

    m_schemaCache[classId] = schema;
    TouchMru(m_schemaCache, m_schemaMru, classId, (size_t)m_maxCache);
    return schema;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ClassWriter const* Writer::GetOrAddInsertWriter(ECN::ECClassId classId, ClassMapCR classMap) const {
    const auto it = m_insertCache.find(classId);
    if (it != m_insertCache.end()) {
        TouchMru(m_insertCache, m_insertMru, classId, (size_t)m_maxCache);
        return it->second.get();
    }

    auto schema = GetOrAddSchema(classId, classMap, WriterOp::Insert);
    if (schema == nullptr)
        return nullptr;

    Utf8String error;
    auto classWriter = ClassWriter::Factory::Create(m_conn, classMap, std::move(schema), error);
    if (classWriter == nullptr) {
        SetError("%s", error.empty() ? "Failed to create the class writer." : error.c_str());
        return nullptr;
    }

    auto const newIt = m_insertCache.insert(std::make_pair(classId, std::move(classWriter)));
    TouchMru(m_insertCache, m_insertMru, classId, (size_t)m_maxCache);
    return newIt.first->second.get();
}

//---------------------------------------------------------------------------------------
//! Level segments are shared between every class that maps the level onto the same columns,
//! which the LevelKey's column signature captures exactly.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
LevelWriter::Ptr Writer::GetOrAddLevel(ClassMapCR classMap, LevelKey const& key, ECN::ECClassCR levelClass, std::vector<PendingProperty> const& pending) const {
    const auto it = m_levelCache.find(key);
    if (it != m_levelCache.end()) {
        TouchMru(m_levelCache, m_levelMru, key, (size_t)m_maxCache * Impl::kLevelCacheFactor);
        return it->second;
    }

    Utf8String error;
    auto levelWriter = LevelWriter::Factory::Create(m_conn, classMap, levelClass, pending, error);
    if (levelWriter == nullptr) {
        SetError("%s", error.empty() ? "Failed to create the level writer." : error.c_str());
        return nullptr;
    }

    m_levelCache.insert(std::make_pair(key, levelWriter));
    TouchMru(m_levelCache, m_levelMru, key, (size_t)m_maxCache * Impl::kLevelCacheFactor);
    return levelWriter;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ClassUpdatePlan const* Writer::GetOrAddUpdatePlan(ECN::ECClassId classId, ClassMapCR classMap) const {
    const auto it = m_planCache.find(classId);
    if (it != m_planCache.end()) {
        TouchMru(m_planCache, m_planMru, classId, (size_t)m_maxCache);
        return it->second.get();
    }

    auto schema = GetOrAddSchema(classId, classMap, WriterOp::Update);
    if (schema == nullptr)
        return nullptr;

    Utf8String error;
    auto const levelResolver = [&](LevelKey const& key, ECN::ECClassCR levelClass, std::vector<PendingProperty> const& pending) {
        return GetOrAddLevel(classMap, key, levelClass, pending);
    };

    auto plan = ClassUpdatePlan::Factory::Create(m_conn, classMap, std::move(schema), levelResolver, error);
    if (plan == nullptr) {
        if (m_error.empty())
            SetError("%s", error.empty() ? "Failed to create the update plan." : error.c_str());

        return nullptr;
    }

    auto const newIt = m_planCache.insert(std::make_pair(classId, std::move(plan)));
    TouchMru(m_planCache, m_planMru, classId, (size_t)m_maxCache);
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
DbResult Writer::Insert(ECN::ECClassId classId, BindCallback const& callback, InsertOptions const& options, ECInstanceKey& key) const {
    BeMutexHolder holder(m_mutex);
    m_error.clear();
    key = ECInstanceKey();

    if (m_isWriting) {
        SetError("BulkInstanceWriter does not support writing from inside a bind callback.");
        return BE_SQLITE_ERROR;
    }

    if (const auto rc = CheckWritePermission(); rc != BE_SQLITE_OK)
        return rc;

    auto classMap = GetClassMap(classId);
    if (classMap == nullptr)
        return BE_SQLITE_ERROR;

    // an INSERT always writes every column, so there is exactly one statement set per class
    auto classWriter = GetOrAddInsertWriter(classId, *classMap);
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

    {
        FlagGuard writing(m_isWriting);
        // the writer's own schema, which may differ from a freshly re-created one after an
        // MRU eviction. The two are structurally identical, but the binders belong to this one.
        WriteContext ctx(classWriter->GetSchema(), *classWriter);
        if (callback != nullptr)
            callback(ctx);
    }

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
    // the reader caches the last seeked row, so it has to be told that the row has changed
    m_conn.GetInstanceReader().InvalidateSeekPos(key);
    return BE_SQLITE_DONE;
}

//---------------------------------------------------------------------------------------
//! Runs the UPDATE statements of the hierarchy levels the callback wrote into, root -> leaf.
//! Because the statements do not depend on the set of written properties, the callback is
//! always invoked exactly once and can bind straight into the level segments.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Writer::Update(ECInstanceKeyCR key, BindCallback const& callback, UpdateOptions const& options) const {
    BeMutexHolder holder(m_mutex);
    m_error.clear();

    if (!key.IsValid()) {
        SetError("Cannot update an instance without a valid ECInstanceKey.");
        return BE_SQLITE_ERROR;
    }

    if (m_isWriting) {
        // level statements are shared, so a nested write would corrupt the outer write's bindings
        SetError("BulkInstanceWriter does not support writing from inside a bind callback.");
        return BE_SQLITE_ERROR;
    }

    if (const auto rc = CheckWritePermission(); rc != BE_SQLITE_OK)
        return rc;

    const auto classId = key.GetClassId();
    auto classMap = GetClassMap(classId);
    if (classMap == nullptr)
        return BE_SQLITE_ERROR;

    auto plan = GetOrAddUpdatePlan(classId, *classMap);
    if (plan == nullptr)
        return BE_SQLITE_ERROR;

    plan->ResetStatements();

    // the plan's own schema, which may differ from a freshly re-created one after an MRU
    // eviction. The two are structurally identical, but the level slots belong to this one.
    auto const& planSchema = plan->GetSchema();

    std::vector<bool> dirtyLevels;
    {
        FlagGuard writing(m_isWriting);
        WriteContext ctx(planSchema, *plan);
        if (callback != nullptr)
            callback(ctx);

        dirtyLevels = ctx.GetDirtyLevels();
    }

    // A full update writes the whole instance, so every level runs. ForceLevels()/ForceLevel()
    // additionally run levels the callback did not touch, whose properties then become NULL.
    // Their purpose is to fire database triggers on a level that would otherwise be skipped.
    bool anyLevelRuns = false;
    for (size_t slot = 0; slot < dirtyLevels.size(); ++slot) {
        if (!dirtyLevels[slot]) {
            auto levelClass = planSchema.GetLevelClass((int)slot);
            if (options.IsFullUpdate() || (levelClass != nullptr && options.IsLevelForced(levelClass->GetId())))
                dirtyLevels[slot] = true;
        }

        if (dirtyLevels[slot] && plan->GetLevel((int)slot) != nullptr)
            anyLevelRuns = true;
    }

    if (!anyLevelRuns) {
        // nothing to do. A partial update that wrote no property is a no-op, and so is any update
        // of a class without a physically writable column.
        plan->ResetStatements();
        return BE_SQLITE_DONE;
    }

    bool anyRowModified = false;
    for (size_t slot = 0; slot < dirtyLevels.size(); ++slot) {
        if (!dirtyLevels[slot])
            continue;

        auto level = plan->GetLevel((int)slot);
        if (level == nullptr)
            continue;

        if (const auto stat = level->OnBeforeFirstStep(); !stat.IsSuccess()) {
            if (m_error.empty())
                SetError("Failed to finalize the bound values before updating instance %s.", key.GetInstanceId().ToHexStr().c_str());

            plan->ResetStatements();
            return BE_SQLITE_ERROR;
        }

        for (auto const& table : level->GetTables()) {
            auto& sqliteStmt = table->GetSqliteStmt();
            if (table->GetInstanceIdParamIndex() > 0) {
                if (const auto rc = sqliteStmt.BindId(table->GetInstanceIdParamIndex(), key.GetInstanceId()); rc != BE_SQLITE_OK) {
                    SetError("Failed to bind the ECInstanceId for table '%s'.", table->GetTable().GetName().c_str());
                    plan->ResetStatements();
                    return rc;
                }
            }

            const auto rc = sqliteStmt.Step();
            if (rc != BE_SQLITE_DONE) {
                SetError("Failed to update table '%s': %s", table->GetTable().GetName().c_str(), BeSQLiteLib::GetErrorName(rc));
                plan->ResetStatements();
                return rc;
            }

            // every level targets the same row, so one modified row anywhere means it exists
            if (m_conn.GetModifiedRowCount() > 0)
                anyRowModified = true;
        }
    }

    plan->ResetStatements();

    // the reader caches the last seeked row, so it has to be told that the row has changed
    m_conn.GetInstanceReader().InvalidateSeekPos(key);

    if (options.GetFailIfNoRowChanged() && !anyRowModified) {
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
    return m_pImpl->Update(key, callback, UpdateOptions());
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
