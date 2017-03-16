/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPreparedStatement.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//***************************************************************************************
//    IECSqlPreparedStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlPreparedStatement::Prepare(ECSqlPrepareContext& ctx, Exp const& exp, Utf8CP ecsql)
    {
    if (m_type != ECSqlType::Select && m_ecdb.IsReadonly())
        {
        LOG.error("ECDb file is opened read-only. For data-modifying ECSQL statements write access is needed.");
        return ECSqlStatus::Error;
        }

    //capture current clear cache counter so that we can invalidate the statement if another clear cache call 
    //occurred in the lifetime of the statement
    m_preparationClearCacheCounter = m_ecdb.GetECDbImplR().GetClearCacheCounter();
    m_ecsql.assign(ecsql);
    return _Prepare(ctx, exp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
IECSqlBinder& IECSqlPreparedStatement::GetBinder(int parameterIndex) const
    {
    if (SUCCESS != AssertIsValid())
        return NoopECSqlBinder::Get();

    return _GetBinder(parameterIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
int IECSqlPreparedStatement::GetParameterIndex(Utf8CP parameterName) const
    {
    if (SUCCESS != AssertIsValid())
        return -1;

    return _GetParameterIndex(parameterName);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlPreparedStatement::Reset()
    {
    if (SUCCESS != AssertIsValid())
        return ECSqlStatus::Error;

    if (m_isNoopInSqlite)
        return ECSqlStatus::Success;

    return _Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
Utf8CP IECSqlPreparedStatement::GetNativeSql() const
    {
    if (m_isNoopInSqlite)
        return "";

    return _GetNativeSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlPreparedStatement::ClearBindings()
    {
    if (SUCCESS != AssertIsValid())
        return ECSqlStatus::Error;

    if (m_isNoopInSqlite)
        return ECSqlStatus::Success;

    return _ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
BentleyStatus IECSqlPreparedStatement::AssertIsValid() const
    {
    if (m_preparationClearCacheCounter != m_ecdb.GetECDbImplR().GetClearCacheCounter())
        {
        LOG.errorv("The ECSqlStatement '%s' can no longer be used because an ECSchema import took place. ECSqlStatements are invalidated after an ECSchema import.",
                   m_ecsql.c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//***************************************************************************************
//    LeafECSqlPreparedStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus LeafECSqlPreparedStatement::_Prepare(ECSqlPrepareContext& ctx, Exp const& exp)
    {
    Utf8String nativeSql;
    ECSqlStatus stat = ECSqlPreparer::Prepare(nativeSql, ctx, exp);
    if (!stat.IsSuccess())
        return stat;

    if (ctx.NativeStatementIsNoop())
        {
        m_isNoopInSqlite = true;
        return ECSqlStatus::Success;
        }

    //don't let BeSQLite log and assert on error (therefore use TryPrepare instead of Prepare)
    DbResult nativeSqlStat = m_sqliteStatement.TryPrepare(m_ecdb, nativeSql.c_str());
    if (nativeSqlStat != BE_SQLITE_OK)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report("Preparing the ECSQL '%s' failed. Underlying SQLite statement '%s' failed to prepare: %s %s", GetECSql(), nativeSql.c_str(),
                                                        ECDb::InterpretDbResult(nativeSqlStat), m_ecdb.GetLastError().c_str());

        //even if this is a SQLite error, we want this to be an InvalidECSql error as the reason usually
        //is a wrong ECSQL provided by the user.
        return ECSqlStatus::InvalidECSql;
        }

    return ECSqlStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
IECSqlBinder& LeafECSqlPreparedStatement::_GetBinder(int parameterIndex) const
    {
    ECSqlBinder* binder = nullptr;
    const ECSqlStatus stat = m_parameterMap.TryGetBinder(binder, parameterIndex);

    if (stat.IsSuccess())
        return *binder;

    if (stat == ECSqlStatus::Error)
        LOG.errorv("Parameter index %d passed to ECSqlStatement binding API is out of bounds.", parameterIndex);

    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
int LeafECSqlPreparedStatement::_GetParameterIndex(Utf8CP parameterName) const
    {
    int index = m_parameterMap.GetIndexForName(parameterName);
    if (index <= 0)
        LOG.errorv("No parameter index found for parameter name :%s.", parameterName);

    return index;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
DbResult LeafECSqlPreparedStatement::DoStep()
    {
    if (SUCCESS != AssertIsValid())
        return BE_SQLITE_ERROR;

    if (!m_parameterMap.OnBeforeStep().IsSuccess())
        return BE_SQLITE_ERROR;

    const DbResult nativeSqlStatus = m_sqliteStatement.Step();
    if (BE_SQLITE_ROW != nativeSqlStatus && BE_SQLITE_DONE != nativeSqlStatus)
        {
        Utf8String msg;
        msg.Sprintf("Step failed for ECSQL '%s': SQLite Step failed [Native SQL: '%s'] with. Error:", GetECSql(), GetNativeSql());
        ECDbLogger::LogSqliteError(m_ecdb, nativeSqlStatus, msg.c_str());
        }

    return nativeSqlStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
ECSqlStatus LeafECSqlPreparedStatement::_ClearBindings()
    {
    const DbResult nativeSqlStat = m_sqliteStatement.ClearBindings();
    m_parameterMap.OnClearBindings();

    if (nativeSqlStat != BE_SQLITE_OK)
        {
        ECDbLogger::LogSqliteError(m_ecdb, nativeSqlStat);
        return ECSqlStatus(nativeSqlStat);
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus LeafECSqlPreparedStatement::_Reset()
    {
    const DbResult nativeSqlStat = m_sqliteStatement.Reset();
    if (nativeSqlStat != BE_SQLITE_OK)
        return ECSqlStatus(nativeSqlStat);

    return ECSqlStatus::Success;
    }

//***************************************************************************************
//    CompoundECSqlPreparedStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
IECSqlBinder& CompoundECSqlPreparedStatement::_GetBinder(int parameterIndex) const
    {
    if (parameterIndex <= 0 || parameterIndex > (int) m_proxyBinders.size())
        {
        LOG.errorv("Parameter index %d passed to ECSqlStatement binding API is out of bounds.", parameterIndex);
        return NoopECSqlBinder::Get();
        }

    return *m_proxyBinders[(size_t) (parameterIndex - 1)];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
int CompoundECSqlPreparedStatement::_GetParameterIndex(Utf8CP parameterName) const
    {
    auto it = m_parameterNameMap.find(parameterName);
    if (it == m_parameterNameMap.end())
        {
        LOG.errorv("No parameter index found for parameter name :%s.", parameterName);
        return -1;
        }

    return it->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::_ClearBindings()
    {
    ECSqlStatus totalStat = ECSqlStatus::Success;
    for (std::unique_ptr<LeafECSqlPreparedStatement>& stmt : m_statements)
        {
        const ECSqlStatus stat = stmt->ClearBindings();
        //in case of error, we continue to clear all bindings, but capture just the first error
        //and return that
        if (!stat.IsSuccess() && totalStat.IsSuccess())
            totalStat = stat;
        }

    return totalStat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::_Reset()
    {
    if (SUCCESS != AssertIsValid())
        return ECSqlStatus::Error;

    if (m_isNoopInSqlite)
        return ECSqlStatus::Success;

    ECSqlStatus totalStat = ECSqlStatus::Success;
    for (std::unique_ptr<LeafECSqlPreparedStatement>& stmt : m_statements)
        {
        ECSqlStatus stat = stmt->Reset();
        //in case of error, we continue to clear all bindings, but capture just the first error
        //and return that
        if (!stat.IsSuccess() && totalStat.IsSuccess())
            totalStat = stat;
        }

    return totalStat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
Utf8CP CompoundECSqlPreparedStatement::_GetNativeSql() const
    {
    if (m_compoundNativeSql.empty())
        {
        if (m_isNoopInSqlite)
            m_compoundNativeSql.assign("no-op in native SQL");
        else
            {
            bool isFirstStmt = true;
            for (std::unique_ptr<LeafECSqlPreparedStatement> const& stmt : m_statements)
                {
                if (!isFirstStmt)
                    m_compoundNativeSql.append(";");

                m_compoundNativeSql.append(stmt->GetNativeSql());
                isFirstStmt = false;
                }
            }
        }

    return m_compoundNativeSql.c_str();
    }


//***************************************************************************************
//    ECSqlSelectPreparedStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
DbResult ECSqlSelectPreparedStatement::Step()
    {
    if (SUCCESS != AssertIsValid())
        return BE_SQLITE_ERROR;

    const DbResult stat = DoStep();
    if (BE_SQLITE_ROW == stat)
        {
        if (!OnAfterStep().IsSuccess())
            return BE_SQLITE_ERROR;
        }

    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlSelectPreparedStatement::_Reset()
    {
    ECSqlStatus resetStatementStat = LeafECSqlPreparedStatement::_Reset();
    
    //even if statement reset failed we still try to reset the fields to clean-up things as good as possible.
    ECSqlStatus fieldResetStat = ResetFields();

    if (resetStatementStat != ECSqlStatus::Success)
        return resetStatementStat;

    if (fieldResetStat != ECSqlStatus::Success)
        return fieldResetStat;

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
int ECSqlSelectPreparedStatement::GetColumnCount() const
    {
    if (SUCCESS != AssertIsValid())
        return -1;

    return (int) m_fields.size();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
IECSqlValue const& ECSqlSelectPreparedStatement::GetValue(int columnIndex) const
    {
    if (SUCCESS != AssertIsValid())
        return NoopECSqlValue::GetSingleton();

    if (columnIndex < 0 || columnIndex >= (int) (m_fields.size()))
        {
        LOG.errorv("Column index '%d' is out of bounds.", columnIndex);
        return NoopECSqlValue::GetSingleton();
        }

    std::unique_ptr<ECSqlField> const& field = m_fields[columnIndex];
    return *field;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlSelectPreparedStatement::ResetFields() const
    {
    for (ECSqlField* field : m_fieldsRequiringReset)
        {
        ECSqlStatus stat = field->OnAfterReset();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlSelectPreparedStatement::OnAfterStep() const
    {
    for (ECSqlField* field : m_fieldsRequiringOnAfterStep)
        {
        ECSqlStatus stat = field->OnAfterStep();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
void ECSqlSelectPreparedStatement::AddField(std::unique_ptr<ECSqlField> field)
    {
    BeAssert(field != nullptr);
    if (field != nullptr)
        {
        if (field->RequiresOnAfterStep())
            m_fieldsRequiringOnAfterStep.push_back(field.get());

        if (field->RequiresOnAfterReset())
            m_fieldsRequiringReset.push_back(field.get());

        m_fields.push_back(std::move(field));
        }
    }


//***************************************************************************************
//    ECSqlInsertPreparedStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlInsertPreparedStatement::_Prepare(ECSqlPrepareContext& ctx, Exp const& exp)
    {
    InsertStatementExp const& insertExp = exp.GetAs<InsertStatementExp>();
    ClassMap const& classMap = insertExp.GetClassNameExp()->GetInfo().GetMap();

    if (!classMap.GetMapStrategy().IsTablePerHierarchy() || classMap.GetTables().size() == 1)
        {
        //FK relationships can also map to multiple tables, they are treated differently
        m_statements.push_back(std::make_unique<LeafECSqlPreparedStatement>(m_ecdb, m_type));
        return m_statements[0]->Prepare(ctx, exp, GetECSql());
        }

    PropertyNameListExp const* propNameListExp = insertExp.GetPropertyNameListExp();
    ValueExpListExp const* valuesListExp = insertExp.GetValuesExp();
    
    const int ecInstanceIdPropNameIx = propNameListExp->GetSpecialTokenExpIndexMap().GetIndex(ECSqlSystemPropertyInfo::ECInstanceId());

    const size_t propNameCount = propNameListExp->GetChildrenCount();

    struct PropNameValueInfo
        {
    public:
        PropertyNameExp const* m_propNameExp = nullptr;
        ValueExp const* m_valueExp = nullptr;

        PropNameValueInfo(PropertyNameExp const& propNameExp, ValueExp const& valueExp) : m_propNameExp(&propNameExp), m_valueExp(&valueExp) {}
        };

    std::map<DbTable const*, std::vector<PropNameValueInfo>> expsByMappedTable;
    //Holds all tables per parameter index (index into the vector + 1) which are affected by the parameter
    std::vector<std::set<DbTable const*>> parameterIndexInTables;

    for (size_t i = 0; i < propNameCount; i++)
        {
        PropertyNameExp const* propNameExp = propNameListExp->GetPropertyNameExp(i);
        if (propNameExp->IsPropertyRef())
            continue;

        PropertyMap const* propertyMap = propNameExp->GetTypeInfo().GetPropertyMap();
        DbTable const* table = nullptr;
        if (propertyMap->IsData()) // sys props are treated separately
            table = &propertyMap->GetAs<DataPropertyMap>().GetTable();
        else
            {
            BeAssert(propertyMap->GetType() == PropertyMap::Type::ECInstanceId ||
                     propertyMap->GetType() == PropertyMap::Type::ConstraintECInstanceId ||
                     propertyMap->GetType() == PropertyMap::Type::ConstraintECClassId);
            table = &propertyMap->GetClassMap().GetPrimaryTable();
            }

        ValueExp const* valueExp = valuesListExp->GetValueExp(i);

        PropNameValueInfo propNameValueInfo(*propNameExp, *valueExp);

        for (Exp const* exp : valueExp->Find(Exp::Type::Parameter, true /* recursive*/))
            {
            ParameterExp const& paramExp = exp->GetAs<ParameterExp>();
            const size_t zeroBasedParameterIndex = (size_t) (paramExp.GetParameterIndex() - 1);
            if (zeroBasedParameterIndex == parameterIndexInTables.size())
                {
                parameterIndexInTables.push_back(std::set<DbTable const*> {table});
                m_proxyBinders.push_back(std::make_unique<ProxyECSqlBinder>());
                }
            else if (zeroBasedParameterIndex < parameterIndexInTables.size())
                parameterIndexInTables[zeroBasedParameterIndex].insert(table);
            else
                {
                BeAssert(false);
                }
            }

        expsByMappedTable[table].push_back(propNameValueInfo);
        }

    Exp::ECSqlRenderContext ecsqlRenderCtx(Exp::ECSqlRenderContext::Mode::GenerateNameForUnnamedParameter);

    //for each table, a separate ECSQL is created
    bool isPrimaryTable = true;
    bmap<DbTable const*, LeafECSqlPreparedStatement const*> perTableStatements;
    for (DbTable const* table : classMap.GetTables())
        {
        auto it = expsByMappedTable.find(table);
        BeAssert(it != expsByMappedTable.end());
        std::vector<PropNameValueInfo> const& propNameValueInfos = it->second;

        Utf8String propNameECSqlClause;
        Utf8String valuesECSqlClause;

        bool isFirstToken = true;

        if (!isPrimaryTable || ecInstanceIdPropNameIx <= 0)
            {
            //if user didn't specify ECInstanceId in the ECSQL or if this is a secondary table ECSQL
            //we have to add the ECInstanceId ourselves to the ECSQL
            propNameECSqlClause.append(ECDBSYS_PROP_ECInstanceId);
            valuesECSqlClause.append(":" ECSQLSYS_PARAM_Id);
            isFirstToken = false;
            }

        for (PropNameValueInfo const& propNameValueInfo : propNameValueInfos)
            {
            if (!isFirstToken)
                {
                propNameECSqlClause.append(",");
                valuesECSqlClause.append(",");
                }

            propNameECSqlClause.append(propNameValueInfo.m_propNameExp->ToECSql());

            propNameValueInfo.m_valueExp->ToECSql(ecsqlRenderCtx);
            valuesECSqlClause.append(ecsqlRenderCtx.GetECSql());
            ecsqlRenderCtx.ResetECSqlBuilder();
            isFirstToken = false;
            }

        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO %s(%s) VALUES(%s)", classMap.GetClass().GetECSqlName().c_str(), propNameECSqlClause.c_str(),
                                                        valuesECSqlClause.c_str());

        ECSqlParser parser;
        std::unique_ptr<Exp> parseTree = parser.Parse(m_ecdb, ecsql.c_str());
        if (parseTree == nullptr)
            return ECSqlStatus::InvalidECSql;

        m_statements.push_back(std::make_unique<LeafECSqlPreparedStatement>(m_ecdb, m_type));

        LeafECSqlPreparedStatement& preparedStmt = *m_statements.back();
        const ECSqlStatus stat = preparedStmt.Prepare(ctx, *parseTree, ecsql.c_str());
        if (!stat.IsSuccess())
            return stat;

        perTableStatements[table] = &preparedStmt;
        isPrimaryTable = false;
        }

    for (bpair<int, bpair<Utf8String, bool>> const& parameterNameMapping : ecsqlRenderCtx.GetParameterIndexNameMap())
        {
        int paramIndex = parameterNameMapping.first;
        BeAssert(paramIndex >= 0 && paramIndex <= (int) m_proxyBinders.size());
        Utf8StringCR paramName = parameterNameMapping.second.first;
        const size_t zeroBasedParamIndex = (size_t) (paramIndex - 1);
        ProxyECSqlBinder& proxyBinder = *m_proxyBinders[zeroBasedParamIndex];
        BeAssert(zeroBasedParamIndex < parameterIndexInTables.size());
        std::set<DbTable const*> const& affectedTables = parameterIndexInTables[zeroBasedParamIndex];
        for (DbTable const* affectedTable : affectedTables)
            {
            BeAssert(perTableStatements.find(affectedTable) != perTableStatements.end());
            ECSqlParameterMap const& leafStatementParameterMap = perTableStatements[affectedTable]->GetParameterMap();
            ECSqlBinder* binder = nullptr;
            if (!leafStatementParameterMap.TryGetBinder(binder, paramName))
                {
                BeAssert(false);
                return ECSqlStatus::Error;
                }

            proxyBinder.AddBinder(*binder);
            }
        }
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
DbResult ECSqlInsertPreparedStatement::Step(ECInstanceKey& instanceKey)
    {
    if (SUCCESS != AssertIsValid())
        return BE_SQLITE_ERROR;

    if (m_isNoopInSqlite)
        return BE_SQLITE_DONE;

    ECInstanceId ecinstanceidOfInsert;
    //GetModifiedRowCount can be costly, so we want to avoid calling it when not needed.
    //For regular case where native SQL is an insert statement with an ECInstanceId
    //generated by ECDb, we do not need to call GetModifiedRowCount (as it is expected to be 1).
    //For some special cases though (end table relationship insert), the native SQL is not an insert,
    //but an update, for which we need to check whether a row was updated or not to tell between
    //success and error.
    bool checkModifiedRowCount = true;
    if (m_ecInstanceKeyInfo.HasUserProvidedECInstanceId())
        ecinstanceidOfInsert = m_ecInstanceKeyInfo.GetUserProvidedECInstanceId();
    else
        {
        //user hasn't provided an ecinstanceid (neither literally nor through binding)
        if (GenerateECInstanceIdAndBindToInsertStatement(ecinstanceidOfInsert) != ECSqlStatus::Success)
            return BE_SQLITE_ERROR;

        checkModifiedRowCount = false;
        }


    //reset the ecinstanceid from key info for the next execution (if it was bound, and is no literal)
    m_ecInstanceKeyInfo.ResetBoundECInstanceId();

    BeAssert(ecinstanceidOfInsert.IsValid());

    //LeafECSqlPreparedStatement& primaryTableStmt = *m_statements[0];
    //IECSqlBinder& idBinder = primaryTableStmt.GetECInstanceIdBinder();
    //if (ECSqlStatus::Success != idBinder.BindId(ecinstanceidOfInsert))
    //    return BE_SQLITE_ERROR;

    for (std::unique_ptr<LeafECSqlPreparedStatement>& stmt : m_statements)
        {
        DbResult stat = stmt->DoStep();
        if (BE_SQLITE_DONE != stat)
            return stat;
        }

    if (checkModifiedRowCount && m_statements.size() == 1 && GetECDb().GetModifiedRowCount() == 0)
        {
        //this can only happen in a specific case with inserting an end table relationship, as there inserting really
        //means to update a row in the end table. (and in that case the compound stmt consists of a single stmt
        GetECDb().GetECDbImplR().GetIssueReporter().Report("Could not insert the ECRelationship (%s). Either the source or target constraint's " ECDBSYS_PROP_ECInstanceId " does not exist or the source or target constraint's cardinality is violated.", GetECSql());
        return BE_SQLITE_CONSTRAINT_UNIQUE;
        }

    instanceKey = ECInstanceKey(m_ecInstanceKeyInfo.GetECClassId(), ecinstanceidOfInsert);
    return BE_SQLITE_DONE;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlInsertPreparedStatement::GenerateECInstanceIdAndBindToInsertStatement(ECInstanceId& generatedECInstanceId)
    {
    ECSqlBinder* ecinstanceidBinder = m_ecInstanceKeyInfo.GetECInstanceIdBinder();
    BeAssert(ecinstanceidBinder != nullptr);

    DbResult dbStat = GetECDb().GetECDbImplR().GetSequence(IdSequences::ECInstanceId).GetNextValue(generatedECInstanceId);
    if (dbStat != BE_SQLITE_OK)
        {
        ECDbLogger::LogSqliteError(GetECDb(), dbStat, "ECSqlStatement::Step failed: Could not generate an " ECDBSYS_PROP_ECInstanceId ".");
        return ECSqlStatus(dbStat);
        }

    const ECSqlStatus stat = ecinstanceidBinder->BindId(generatedECInstanceId);
    if (!stat.IsSuccess())
        LOG.error("ECSqlStatement::Step failed: Could not bind the generated " ECDBSYS_PROP_ECInstanceId ".");

    return stat;
    }

//***************************************************************************************
//    ECSqlUpdatePreparedStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlUpdatePreparedStatement::_Prepare(ECSqlPrepareContext& ctx, Exp const& exp)
    {
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
DbResult ECSqlUpdatePreparedStatement::Step()
    {
    if (SUCCESS != AssertIsValid())
        return BE_SQLITE_ERROR;

    if (m_isNoopInSqlite)
        return BE_SQLITE_DONE;

    for (std::unique_ptr<LeafECSqlPreparedStatement>& stmt : m_statements)
        {
        DbResult stat = stmt->DoStep();
        if (BE_SQLITE_DONE != stat)
            return stat;
        }

    return BE_SQLITE_DONE;
    }



//***************************************************************************************
//    ECSqlDeletePreparedStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
DbResult ECSqlDeletePreparedStatement::Step()
    {
    if (SUCCESS != AssertIsValid())
        return BE_SQLITE_ERROR;

    if (m_isNoopInSqlite)
        return BE_SQLITE_DONE;

    return DoStep();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE