/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPreparedStatement.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

#ifdef ECSQLPREPAREDSTATEMENT_REFACTOR

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
//    SingleECSqlPreparedStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus SingleECSqlPreparedStatement::_Prepare(ECSqlPrepareContext& ctx, Exp const& exp)
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
IECSqlBinder& SingleECSqlPreparedStatement::_GetBinder(int parameterIndex) const
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
int SingleECSqlPreparedStatement::_GetParameterIndex(Utf8CP parameterName) const
    {
    int index = m_parameterMap.GetIndexForName(parameterName);
    if (index <= 0)
        LOG.errorv("No parameter index found for parameter name :%s.", parameterName);

    return index;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
DbResult SingleECSqlPreparedStatement::DoStep()
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
ECSqlStatus SingleECSqlPreparedStatement::_ClearBindings()
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
ECSqlStatus SingleECSqlPreparedStatement::_Reset()
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
    for (std::unique_ptr<SingleContextTableECSqlPreparedStatement>& stmt : m_statements)
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
    for (std::unique_ptr<SingleContextTableECSqlPreparedStatement>& stmt : m_statements)
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
            for (std::unique_ptr<SingleContextTableECSqlPreparedStatement> const& stmt : m_statements)
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
    ECSqlStatus resetStatementStat = SingleECSqlPreparedStatement::_Reset();
    
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

    Policy policy = PolicyManager::GetPolicy(ClassIsValidInECSqlPolicyAssertion(classMap, m_type));
    if (!policy.IsSupported())
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report("Invalid ECClass in ECSQL: %s", policy.GetNotSupportedMessage().c_str());
        return ECSqlStatus::InvalidECSql;
        }

    const bool isEndTableRelationship = classMap.GetType() == ClassMap::Type::RelationshipEndTable;

    BeAssert(classMap.GetType() != ClassMap::Type::RelationshipEndTable || classMap.IsMappedToSingleTable() && "FK relationship mappings with multiple tables should have been caught before. They are not insertable");
    PropertyNameListExp const* propNameListExp = insertExp.GetPropertyNameListExp();
    ValueExpListExp const* valuesListExp = insertExp.GetValuesExp();

    if (classMap.IsRelationshipClassMap())
        {
        if (!propNameListExp->GetSpecialTokenExpIndexMap().Contains(ECSqlSystemPropertyInfo::SourceECInstanceId()) && !propNameListExp->GetSpecialTokenExpIndexMap().Contains(ECSqlSystemPropertyInfo::TargetECInstanceId()))
            {
            m_ecdb.GetECDbImplR().GetIssueReporter().Report("In an ECSQL INSERT statement against an ECRelationship class " ECDBSYS_PROP_SourceECInstanceId " and " ECDBSYS_PROP_TargetECInstanceId " must always be specified.");
            return ECSqlStatus::InvalidECSql;
            }
        }

    int idPropNameExpIx = -1;
    m_ecInstanceKeyHelper.Initialize(idPropNameExpIx, classMap, *propNameListExp, *valuesListExp);

    struct PropNameValueInfo
        {
    public:
        PropertyNameExp const* m_propNameExp = nullptr;
        ValueExp const* m_valueExp = nullptr;
        bool m_isIdPropNameExp = false;

        PropNameValueInfo(PropertyNameExp const& propNameExp, ValueExp const& valueExp, bool isIdPropNameExp) : m_propNameExp(&propNameExp), m_valueExp(&valueExp), m_isIdPropNameExp(isIdPropNameExp) {}
        };

    std::map<DbTable const*, std::vector<PropNameValueInfo>> expsByMappedTable;
    //Holds all tables per parameter index (index into the vector + 1) which are affected by the parameter
    std::vector<std::set<DbTable const*>> parameterIndexInTables;

    const size_t propNameCount = propNameListExp->GetChildrenCount();
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

        const bool isIdExp = idPropNameExpIx >= 0 && idPropNameExpIx == (int) i;
        PropNameValueInfo propNameValueInfo(*propNameExp, *valueExp, isIdExp);

        for (Exp const* exp : valueExp->Find(Exp::Type::Parameter, true /* recursive*/))
            {
            ParameterExp const& paramExp = exp->GetAs<ParameterExp>();
            const size_t zeroBasedParameterIndex = (size_t) (paramExp.GetParameterIndex() - 1);
            if (zeroBasedParameterIndex == parameterIndexInTables.size())
                {
                parameterIndexInTables.push_back(std::set<DbTable const*> {table});

                if (isIdExp && m_ecInstanceKeyHelper.GetMode() == ECInstanceKeyHelper::Mode::UserProvidedParameterExp)
                    {
                    std::unique_ptr<ProxyECInstanceIdECSqlBinder> proxyBinder = std::make_unique<ProxyECInstanceIdECSqlBinder>();
                    m_ecInstanceKeyHelper.SetUserProvidedParameterBinder(*proxyBinder);
                    m_proxyBinders.push_back(std::move(proxyBinder));
                    }
                else
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

    bool isPrimaryTable = true;
    bmap<DbTable const*, SingleContextTableECSqlPreparedStatement const*> perTableStatements;
    for (DbTable const* table : classMap.GetTables())
        {
        Utf8String propNameECSqlClause;
        Utf8String valuesECSqlClause;

        bool isFirstToken = true;

        if (!isPrimaryTable || m_ecInstanceKeyHelper.GetMode() == ECInstanceKeyHelper::Mode::NotUserProvided)
            {
            //if user didn't specify ECInstanceId in the ECSQL or if this is a secondary table ECSQL
            //we have to add the ECInstanceId ourselves to the ECSQL
            propNameECSqlClause.append(ECDBSYS_PROP_ECInstanceId);
            valuesECSqlClause.append(":" ECSQLSYS_PARAM_Id);
            isFirstToken = false;
            }

        auto it = expsByMappedTable.find(table);
        //If the ECSQL INSERT doesn't have expressions targeting this table.
        //we still have to insert a row for it (which will be empty except for the id and classid
        if (it != expsByMappedTable.end())
            {
            std::vector<PropNameValueInfo> const& propNameValueInfos = it->second;
            for (PropNameValueInfo const& propNameValueInfo : propNameValueInfos)
                {
                if (!isFirstToken)
                    {
                    propNameECSqlClause.append(",");
                    valuesECSqlClause.append(",");
                    }

                propNameECSqlClause.append(propNameValueInfo.m_propNameExp->ToECSql());

                propNameValueInfo.m_valueExp->ToECSql(ecsqlRenderCtx);
                if (isPrimaryTable && propNameValueInfo.m_isIdPropNameExp && m_ecInstanceKeyHelper.GetMode() == ECInstanceKeyHelper::Mode::UserProvidedNullExp)
                    {
                    //if user specified NULL for ECInstanceId ECDb auto-generates the id. Therefore we have
                    //to replace the NULL by a parameter
                    valuesECSqlClause.append(":" ECSQLSYS_PARAM_Id);
                    }
                else
                    valuesECSqlClause.append(ecsqlRenderCtx.GetECSql());

                ecsqlRenderCtx.ResetECSqlBuilder();
                isFirstToken = false;
                }
            }

        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO %s(%s) VALUES(%s)", classMap.GetClass().GetECSqlName().c_str(), propNameECSqlClause.c_str(),
                                                        valuesECSqlClause.c_str());

        ECSqlParser parser;
        std::unique_ptr<Exp> parseTree = parser.Parse(m_ecdb, ecsql.c_str());
        if (parseTree == nullptr)
            return ECSqlStatus::InvalidECSql;

        m_statements.push_back(std::make_unique<SingleContextTableECSqlPreparedStatement>(m_ecdb, ECSqlType::Insert, *table));

        SingleContextTableECSqlPreparedStatement& preparedStmt = *m_statements.back();
        ctx.Reset(preparedStmt);
        const ECSqlStatus stat = preparedStmt.Prepare(ctx, *parseTree, ecsql.c_str());
        if (!stat.IsSuccess())
            return stat;

        BeAssert(!preparedStmt.IsNoopInSqlite() && "ECSQL INSERT statements are not expected to translate to no-ops in SQLite. Instead this should have been caught and resulted in an error.");
        perTableStatements[table] = &preparedStmt;
        isPrimaryTable = false;
        }

    for (bpair<int, Exp::ECSqlRenderContext::ParameterNameInfo> const& parameterNameMapping : ecsqlRenderCtx.GetParameterIndexNameMap())
        {
        int paramIndex = parameterNameMapping.first;
        BeAssert(paramIndex >= 0 && paramIndex <= (int) m_proxyBinders.size());
        Utf8StringCR paramName = parameterNameMapping.second.m_name;
        if (!parameterNameMapping.second.m_isSystemGeneratedName)
            m_parameterNameMap[paramName] = paramIndex;

        const size_t zeroBasedParamIndex = (size_t) (paramIndex - 1);
        IProxyECSqlBinder& proxyBinder = *m_proxyBinders[zeroBasedParamIndex];
        BeAssert(zeroBasedParamIndex < parameterIndexInTables.size());
        std::set<DbTable const*> const& affectedTables = parameterIndexInTables[zeroBasedParamIndex];
        BeAssert(!affectedTables.empty());
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

    if (m_ecInstanceKeyHelper.IsEndTableRelationshipInsert())
        return StepForEndTableRelationship(instanceKey);

    if (m_ecInstanceKeyHelper.MustGenerateECInstanceId())
        {
        IECSqlBinder* idBinder = nullptr;
        if (m_ecInstanceKeyHelper.GetMode() == ECInstanceKeyHelper::Mode::UserProvidedParameterExp)
            idBinder = &m_ecInstanceKeyHelper.GetIdProxyBinder()->GetBinder();
        else
            {
            ECSqlBinder* binder = nullptr;
            if (!GetPrimaryTableECSqlStatement().GetParameterMap().TryGetBinder(binder, ECSQLSYS_PARAM_Id))
                {
                BeAssert(false);
                return BE_SQLITE_ERROR;
                }

            idBinder = binder;
            }

        BeAssert(idBinder != nullptr);

        ECInstanceId generatedId;
        const DbResult dbStat = GetECDb().GetECDbImplR().GetSequence(IdSequences::Key::InstanceId).GetNextValue(generatedId);
        if (BE_SQLITE_OK != dbStat)
            {
            ECDbLogger::LogSqliteError(GetECDb(), dbStat, "ECSqlStatement::Step failed: Could not generate an ECInstanceId.");
            return dbStat;
            }

        const ECSqlStatus stat = idBinder->BindId(generatedId);
        if (!stat.IsSuccess())
            {
            LOG.error("ECSqlStatement::Step failed: Could not bind the generated " ECDBSYS_PROP_ECInstanceId ".");
            return BE_SQLITE_ERROR;
            }
        }

    const DbResult stat = GetPrimaryTableECSqlStatement().DoStep();
    if (BE_SQLITE_DONE != stat)
        return stat;

    //retrieve id as it is returned from this method
    instanceKey = ECInstanceKey(m_ecInstanceKeyHelper.GetClassId(), ECInstanceId((uint64_t) GetECDb().GetLastInsertRowId()));

    const size_t leafStmtCount = m_statements.size();
    for (size_t i = 1; i < leafStmtCount; i++)
        {
        SingleContextTableECSqlPreparedStatement& leafECSqlStmt = *m_statements[i];
        ECSqlBinder* idBinder = nullptr;
        if (!leafECSqlStmt.GetParameterMap().TryGetBinder(idBinder, ECSQLSYS_PARAM_Id))
            {
            BeAssert(false);
            return BE_SQLITE_ERROR;
            }

        BeAssert(idBinder != nullptr);
        if (ECSqlStatus::Success != idBinder->BindId(instanceKey.GetInstanceId()))
            return BE_SQLITE_ERROR;

        const DbResult stat = leafECSqlStmt.DoStep();
        if (BE_SQLITE_DONE != stat)
            return stat;
        }

    return BE_SQLITE_DONE;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        04/17
//---------------------------------------------------------------------------------------
DbResult ECSqlInsertPreparedStatement::StepForEndTableRelationship(ECInstanceKey& instanceKey)
    {
    BeAssert(m_statements.size() == 1);
    ECInstanceKeyHelper::UpdateHook updateHook(GetECDb());
    const DbResult stat = GetPrimaryTableECSqlStatement().DoStep();
    if (BE_SQLITE_DONE != stat)
        return stat;

    if (GetECDb().GetModifiedRowCount() == 0)
        {
        //this can with inserting an end table relationship, as the INSERT really is an update. The SQLite update has a where exp
        //which checks that the FK of the row to be update is NULL. Therefore if the update doesn't affect anything it most likely
        //means that this ECSQL attempts to overwrite the FK which is not supported.
        GetECDb().GetECDbImplR().GetIssueReporter().Report("Could not insert the ECRelationship (%s). Either the source or target constraint's " ECDBSYS_PROP_ECInstanceId " does not exist or the source or target constraint's cardinality is violated.", GetECSql());
        return BE_SQLITE_CONSTRAINT_UNIQUE;
        }

    instanceKey = ECInstanceKey(m_ecInstanceKeyHelper.GetClassId(), updateHook.GetUpdatedRowid());
    return stat;
    }

//***************************************************************************************
//    ECSqlInsertPreparedStatement::ECInstanceKeyHelper
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
void ECSqlInsertPreparedStatement::ECInstanceKeyHelper::Initialize(int &idPropNameExpIx, ClassMap const& classMap, PropertyNameListExp const& propNameListExp, ValueExpListExp const& valueListExp)
    {
    m_classId = classMap.GetClass().GetId();

    //If this is an end table relationship the returned instance key is the same as the ECInstanceId of the row holding the FK
    //If the ECSQL includes an ECInstanceId exp it is ignored
    if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
        m_sysPropInfo = classMap.GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable ? &ECSqlSystemPropertyInfo::SourceECInstanceId() : &ECSqlSystemPropertyInfo::TargetECInstanceId();
    else
        m_sysPropInfo = &ECSqlSystemPropertyInfo::ECInstanceId();

    idPropNameExpIx = propNameListExp.GetSpecialTokenExpIndexMap().GetIndex(*m_sysPropInfo);

    if (idPropNameExpIx < 0)
        m_mode = Mode::NotUserProvided;
    else
        {
        ValueExp const* idValueExp = valueListExp.GetValueExp((size_t) idPropNameExpIx);
        BeAssert(idValueExp != nullptr);

        switch (idValueExp->GetType())
            {
                case Exp::Type::LiteralValue:
                {
                if (idValueExp->GetTypeInfo().GetKind() == ECSqlTypeInfo::Kind::Null)
                    m_mode = Mode::UserProvidedNullExp;
                else
                    m_mode = Mode::UserProvidedOtherExp;

                break;
                }

                case Exp::Type::Parameter:
                    m_mode = Mode::UserProvidedParameterExp;
                    break;

                default:
                    m_mode = Mode::UserProvidedOtherExp;
                    break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
//static
ECSqlInsertPreparedStatement::ECInstanceKeyHelper::Mode ECSqlInsertPreparedStatement::ECInstanceKeyHelper::DetermineMode(int& expIx, ECSqlSystemPropertyInfo const& sysPropInfo, PropertyNameListExp const& propNameListExp, ValueExpListExp const& valueListExp)
    {
    expIx = propNameListExp.GetSpecialTokenExpIndexMap().GetIndex(sysPropInfo);

    if (expIx < 0)
        return Mode::NotUserProvided;

    ValueExp const* idValueExp = valueListExp.GetValueExp((size_t) expIx);
    BeAssert(idValueExp != nullptr);

    switch (idValueExp->GetType())
        {
            case Exp::Type::LiteralValue:
            {
            if (idValueExp->GetTypeInfo().GetKind() == ECSqlTypeInfo::Kind::Null)
                return Mode::UserProvidedNullExp;

            return Mode::UserProvidedOtherExp;
            }

            case Exp::Type::Parameter:
                return Mode::UserProvidedParameterExp;

            default:
                return Mode::UserProvidedOtherExp;
        }
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
bool ECSqlInsertPreparedStatement::ECInstanceKeyHelper::MustGenerateECInstanceId() const
    {
    if (m_mode == Mode::UserProvidedOtherExp || IsEndTableRelationshipInsert())
        return false;

    if (m_mode == Mode::UserProvidedParameterExp)
        return m_idProxyBinder->IsBoundValueNull();

    return true;
    }


//***************************************************************************************
//    ECSqlUpdatePreparedStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlUpdatePreparedStatement::_Prepare(ECSqlPrepareContext& ctx, Exp const& exp)
    {
    UpdateStatementExp const& updateExp = exp.GetAs<UpdateStatementExp>();

    ClassNameExp const* classNameExp = updateExp.GetClassNameExp();
    const bool isPolymorphicUpdate = classNameExp->IsPolymorphic();
    ClassMap const& classMap = classNameExp->GetInfo().GetMap();

    Policy policy = PolicyManager::GetPolicy(ClassIsValidInECSqlPolicyAssertion(classMap, m_type, classNameExp->IsPolymorphic()));
    if (!policy.IsSupported())
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report("Invalid ECClass in ECSQL: %s", policy.GetNotSupportedMessage().c_str());
        return ECSqlStatus::InvalidECSql;
        }

    AssignmentListExp const* assignmentListExp = updateExp.GetAssignmentListExp();
    ECSqlStatus stat = CheckForReadonlyProperties(ctx, *assignmentListExp, updateExp);
    if (stat != ECSqlStatus::Success)
        return stat;

    SystemPropertyExpIndexMap const& specialTokenExpIndexMap = assignmentListExp->GetSpecialTokenExpIndexMap();
    if (specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::ECInstanceId()) || specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::ECClassId()))
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to prepare ECSQL '%s'. " ECDBSYS_PROP_ECInstanceId " or " ECDBSYS_PROP_ECClassId " are not allowed in SET clause of ECSQL UPDATE statement. ECDb does not support to modify those.",
                                                        updateExp.ToECSql().c_str());
        return ECSqlStatus::InvalidECSql;
        }

    if (classMap.IsRelationshipClassMap())
        {
        if (specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::SourceECInstanceId()) ||
            specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::SourceECClassId()) ||
            specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::TargetECInstanceId()) ||
            specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::TargetECClassId()))
            {
            m_ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to prepare ECSQL '%s'. " ECDBSYS_PROP_SourceECInstanceId ", " ECDBSYS_PROP_SourceECClassId ", " ECDBSYS_PROP_TargetECInstanceId ", or " ECDBSYS_PROP_TargetECClassId " are not allowed in the SET clause of ECSQL UPDATE statement. ECDb does not support to modify those as they are keys of the relationship. Instead delete the relationship and insert the desired new one.",
                                                            updateExp.ToECSql().c_str());
            return ECSqlStatus::InvalidECSql;
            }
        }


    std::map<DbTable const*, std::vector<AssignmentExp const*>> expsByMappedTable;
    //Holds all tables per parameter index (index into the vector + 1) which are affected by the parameter
    std::vector<std::set<DbTable const*>> parameterIndexInTables;

    for (Exp const* childExp : assignmentListExp->GetChildren())
        {
        AssignmentExp const& assignmentExp = childExp->GetAs<AssignmentExp>();
        PropertyNameExp const* lhsExp = assignmentExp.GetPropertyNameExp();
        PropertyMap const& lhsPropMap = lhsExp->GetPropertyMap();
        if (!lhsPropMap.IsData())
            {
            BeAssert(lhsPropMap.IsData());
            return ECSqlStatus::Error;
            }

        DbTable const& table = lhsPropMap.GetAs<DataPropertyMap>().GetTable();
        expsByMappedTable[&table].push_back(&assignmentExp);

        ValueExp const* rhsExp = assignmentExp.GetValueExp();

        //for now this just checks that the SET exp doesn't involve multiple tables
        for (Exp const* exp : rhsExp->Find(Exp::Type::PropertyName, true /* recursive*/))
            {
            PropertyNameExp const& propNameExp = exp->GetAs<PropertyNameExp>();
            if (propNameExp.IsPropertyRef())
                continue;

            GetTablesPropertyMapVisitor getTablesVisitor;
            if (SUCCESS != propNameExp.GetPropertyMap().AcceptVisitor(getTablesVisitor))
                {
                BeAssert(false);
                return ECSqlStatus::Error;
                }

            if (getTablesVisitor.GetTables().find(&table) == getTablesVisitor.GetTables().end())
                {
                m_ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to prepare ECSQL '%s'. The expression '%s' in the SET clause refers to different tables. This is not yet supported.",
                                                                updateExp.ToECSql().c_str(), assignmentExp.ToECSql().c_str());
                return ECSqlStatus::InvalidECSql;
                }
            }

        for (Exp const* exp : rhsExp->Find(Exp::Type::Parameter, true /* recursive*/))
            {
            ParameterExp const& paramExp = exp->GetAs<ParameterExp>();
            const size_t zeroBasedParameterIndex = (size_t) (paramExp.GetParameterIndex() - 1);
            if (zeroBasedParameterIndex == parameterIndexInTables.size())
                {
                parameterIndexInTables.push_back(std::set<DbTable const*> {&table});
                m_proxyBinders.push_back(std::make_unique<ProxyECSqlBinder>());
                }
            else if (zeroBasedParameterIndex < parameterIndexInTables.size())
                parameterIndexInTables[zeroBasedParameterIndex].insert(&table);
            else
                {
                BeAssert(false);
                }
            }
        }

    Exp::ECSqlRenderContext ecsqlRenderCtx(Exp::ECSqlRenderContext::Mode::GenerateNameForUnnamedParameter);

    WhereExp const* whereClause = updateExp.GetWhereClauseExp();
    if (whereClause != nullptr)
        {
        for (Exp const* exp : whereClause->Find(Exp::Type::Parameter, true /* recursive*/))
            {
            ParameterExp const& paramExp = exp->GetAs<ParameterExp>();
            const size_t zeroBasedParameterIndex = (size_t) (paramExp.GetParameterIndex() - 1);
            if (zeroBasedParameterIndex == parameterIndexInTables.size())
                {
                parameterIndexInTables.push_back({});
                m_proxyBinders.push_back(std::make_unique<ProxyECSqlBinder>());
                }
            }

        Utf8String whereClauseSelectorECSql("SELECT ECInstanceId FROM ");
        if (!classNameExp->IsPolymorphic())
            whereClauseSelectorECSql.append(" ONLY ");

        whereClauseSelectorECSql.append(classMap.GetClass().GetECSqlName()).append(" ");
        whereClause->ToECSql(ecsqlRenderCtx);
        whereClauseSelectorECSql.append(ecsqlRenderCtx.GetECSql());
        ecsqlRenderCtx.ResetECSqlBuilder();

        ECSqlParser parser;
        std::unique_ptr<Exp> parseTree = parser.Parse(m_ecdb, whereClauseSelectorECSql.c_str());
        if (parseTree == nullptr)
            return ECSqlStatus::InvalidECSql;

        m_whereClauseSelector = std::make_unique<ECSqlSelectPreparedStatement>(m_ecdb);
        ctx.Reset(*m_whereClauseSelector);
        const ECSqlStatus stat = m_whereClauseSelector->Prepare(ctx, *parseTree, whereClauseSelectorECSql.c_str());
        if (!stat.IsSuccess())
            return stat;
        }

    Utf8String optionsECSql;
    OptionsExp const* optionsClause = updateExp.GetOptionsClauseExp();
    if (optionsClause != nullptr)
        optionsECSql.assign(optionsClause->ToECSql());

    bmap<DbTable const*, SingleContextTableECSqlPreparedStatement const*> perTableStatements;

    for (std::pair<DbTable const*, std::vector<AssignmentExp const*>> const& kvPair : expsByMappedTable)
        {
        DbTable const* table = kvPair.first;
        Utf8String ecsql("UPDATE ");
        if (!isPolymorphicUpdate)
            ecsql.append("ONLY ");

        ecsql.append(classMap.GetClass().GetECSqlName()).append(" SET ");

        bool isFirstAssignment = true;
        for (AssignmentExp const* assignmentExp : kvPair.second)
            {
            if (!isFirstAssignment)
                ecsql.append(",");

            assignmentExp->ToECSql(ecsqlRenderCtx);
            ecsql.append(ecsqlRenderCtx.GetECSql());
            isFirstAssignment = false;
            ecsqlRenderCtx.ResetECSqlBuilder();
            }

        if (m_whereClauseSelector != nullptr)
            ecsql.append(" WHERE InVirtualSet(:" ECSQLSYS_PARAM_Id ",ECInstanceId)");

        if (optionsClause != nullptr)
            ecsql.append(" ").append(optionsECSql);

        ECSqlParser parser;
        std::unique_ptr<Exp> parseTree = parser.Parse(m_ecdb, ecsql.c_str());
        if (parseTree == nullptr)
            return ECSqlStatus::InvalidECSql;

        m_statements.push_back(std::make_unique<SingleContextTableECSqlPreparedStatement>(m_ecdb, ECSqlType::Update, *table));

        SingleContextTableECSqlPreparedStatement& preparedStmt = *m_statements.back();
        ctx.Reset(preparedStmt);
        const ECSqlStatus stat = preparedStmt.Prepare(ctx, *parseTree, ecsql.c_str());
        if (!stat.IsSuccess())
            return stat;

        //If one leaf statement turns out to be noop, we assume everything is a noop (WIP: should be asserted on)
        if (preparedStmt.IsNoopInSqlite())
            m_isNoopInSqlite = true;

        perTableStatements[table] = &preparedStmt;
        }
   
    for (bpair<int, Exp::ECSqlRenderContext::ParameterNameInfo> const& parameterNameMapping : ecsqlRenderCtx.GetParameterIndexNameMap())
        {
        int paramIndex = parameterNameMapping.first;
        BeAssert(paramIndex >= 0 && paramIndex <= (int) m_proxyBinders.size());
        Utf8StringCR paramName = parameterNameMapping.second.m_name;
        if (!parameterNameMapping.second.m_isSystemGeneratedName)
            m_parameterNameMap[paramName] = paramIndex;

        const size_t zeroBasedParamIndex = (size_t) (paramIndex - 1);
        IProxyECSqlBinder& proxyBinder = *m_proxyBinders[zeroBasedParamIndex];
        BeAssert(zeroBasedParamIndex < parameterIndexInTables.size());
        std::set<DbTable const*> const& affectedTables = parameterIndexInTables[zeroBasedParamIndex];
        if (affectedTables.empty())
            {
            ECSqlParameterMap const& parameterMap = m_whereClauseSelector->GetParameterMap();
            ECSqlBinder* binder = nullptr;
            if (!parameterMap.TryGetBinder(binder, paramName))
                {
                BeAssert(false);
                return ECSqlStatus::Error;
                }

            proxyBinder.AddBinder(*binder);
            }
        else
            {
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
        }

    return ECSqlStatus::Success;
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

    IdSet<BeInt64Id> idSet;
    if (m_whereClauseSelector != nullptr)
        {
        while (BE_SQLITE_ROW == m_whereClauseSelector->Step())
            {
            idSet.insert(m_whereClauseSelector->GetValue(0).GetId<BeInt64Id>());
            }

        m_whereClauseSelector->Reset();
        }

    for (std::unique_ptr<SingleContextTableECSqlPreparedStatement>& stmt : m_statements)
        {
        if (m_whereClauseSelector != nullptr)
            {
            ECSqlBinder* binder = nullptr;
            if (!stmt->GetParameterMap().TryGetBinder(binder, ECSQLSYS_PARAM_Id))
                {
                BeAssert(false);
                return BE_SQLITE_ERROR;
                }

            if (ECSqlStatus::Success != binder->BindVirtualSet(idSet))
                {
                BeAssert(false);
                return BE_SQLITE_ERROR;
                }
            }

        DbResult stat = stmt->DoStep();
        if (BE_SQLITE_DONE != stat)
            return stat;
        }

    return BE_SQLITE_DONE;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        04/17
//---------------------------------------------------------------------------------------
//static
ECSqlStatus ECSqlUpdatePreparedStatement::CheckForReadonlyProperties(ECSqlPrepareContext& ctx, AssignmentListExp const& assignmentListExp, UpdateStatementExp const& exp)
    {
    OptionsExp const* optionsExp = exp.GetOptionsClauseExp();
    if (optionsExp != nullptr && optionsExp->HasOption(OptionsExp::READONLYPROPERTIESAREUPDATABLE_OPTION))
        return ECSqlStatus::Success;

    for (Exp const* expr : assignmentListExp.GetChildren())
        {
        PropertyNameExp const* lhsOperandOfAssignmentExp = expr->GetAs<AssignmentExp>().GetPropertyNameExp();
        if (!lhsOperandOfAssignmentExp->IsPropertyRef())
            {
            ECPropertyCR prop = lhsOperandOfAssignmentExp->GetPropertyMap().GetProperty();

            if (prop.IsReadOnlyFlagSet() && prop.GetIsReadOnly() && !prop.IsCalculated())
                {
                ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report("The ECProperty '%s' is read-only. Read-only ECProperties cannot be modified by an ECSQL UPDATE statement. %s",
                                                                       prop.GetName().c_str(), exp.ToECSql().c_str());
                return ECSqlStatus::InvalidECSql;
                }
            }
        }

    return ECSqlStatus::Success;
    }

//***************************************************************************************
//    ECSqlDeletePreparedStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlDeletePreparedStatement::_Prepare(ECSqlPrepareContext& ctx, Exp const& exp)
    {
    DeleteStatementExp const& deleteExp = exp.GetAs<DeleteStatementExp>();
    ClassNameExp const* classNameExp = deleteExp.GetClassNameExp();
    ClassMap const& classMap = classNameExp->GetInfo().GetMap();

    Policy policy = PolicyManager::GetPolicy(ClassIsValidInECSqlPolicyAssertion(classMap, m_type, classNameExp->IsPolymorphic()));
    if (!policy.IsSupported())
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report("Invalid ECClass in ECSQL: %s", policy.GetNotSupportedMessage().c_str());
        return ECSqlStatus::InvalidECSql;
        }

    //WIP this will probably not be enough
    return SingleECSqlPreparedStatement::_Prepare(ctx, exp);
    }

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

//***************************************************************************************
//    CompoundECSqlPreparedStatement::ProxyECInstanceIdECSqlBinder
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECInstanceIdECSqlBinder::_BindBoolean(bool value)
    {
    LOG.error("Type mismatch. Cannot bind boolean value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECInstanceIdECSqlBinder::_BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy)
    {
    LOG.error("Type mismatch. Cannot bind Blob value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECInstanceIdECSqlBinder::_BindPoint2d(DPoint2dCR value)
    {
    LOG.error("Type mismatch. Cannot bind Point2d value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECInstanceIdECSqlBinder::_BindPoint3d(DPoint3dCR value)
    {
    LOG.error("Type mismatch. Cannot bind Point3d value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2016
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECInstanceIdECSqlBinder::_BindZeroBlob(int blobSize)
    {
    LOG.error("Type mismatch. Cannot bind Zeroblob value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECInstanceIdECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECInstanceIdECSqlBinder::_BindDateTime(uint64_t julianDayHns, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECInstanceIdECSqlBinder::_BindDouble(double value)
    {
    LOG.error("Type mismatch. Cannot bind double value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECInstanceIdECSqlBinder::_BindInt(int value)
    {
    LOG.error("Type mismatch. Cannot bind 32 bit integer value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
IECSqlBinder& CompoundECSqlPreparedStatement::ProxyECInstanceIdECSqlBinder::_BindStructMember(Utf8CP structMemberPropertyName)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to Id parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
IECSqlBinder& CompoundECSqlPreparedStatement::ProxyECInstanceIdECSqlBinder::_BindStructMember(ECN::ECPropertyId structMemberPropertyId)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to Id parameter.");
    return NoopECSqlBinder::Get();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
IECSqlBinder& CompoundECSqlPreparedStatement::ProxyECInstanceIdECSqlBinder::_AddArrayElement()
    {
    LOG.error("Type mismatch. Cannot bind array to Id parameter.");
    return NoopECSqlBinder::Get();
    }


//***************************************************************************************
//    CompoundECSqlPreparedStatement::ProxyECSqlBinder
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindNull()
    {
    for (IECSqlBinder* binder : m_binders)
        {
        ECSqlStatus stat = binder->BindNull();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindBoolean(bool value)
    {
    for (IECSqlBinder* binder : m_binders)
        {
        ECSqlStatus stat = binder->BindBoolean(value);
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy)
    {
    for (IECSqlBinder* binder : m_binders)
        {
        ECSqlStatus stat = binder->BindBlob(value, blobSize, makeCopy);
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindZeroBlob(int blobSize)
    {
    for (IECSqlBinder* binder : m_binders)
        {
        ECSqlStatus stat = binder->BindZeroBlob(blobSize);
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const& dtInfo)
    {
    for (IECSqlBinder* binder : m_binders)
        {
        ECSqlStatus stat = binder->BindDateTime(julianDay, dtInfo);
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const& dtInfo)
    {
    for (IECSqlBinder* binder : m_binders)
        {
        ECSqlStatus stat = binder->BindDateTime(julianDayMsec, dtInfo);
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindDouble(double value)
    {
    for (IECSqlBinder* binder : m_binders)
        {
        ECSqlStatus stat = binder->BindDouble(value);
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindInt(int value)
    {
    for (IECSqlBinder* binder : m_binders)
        {
        ECSqlStatus stat = binder->BindInt(value);
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindInt64(int64_t value)
    {
    for (IECSqlBinder* binder : m_binders)
        {
        ECSqlStatus stat = binder->BindInt64(value);
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindPoint2d(DPoint2dCR value)
    {
    for (IECSqlBinder* binder : m_binders)
        {
        ECSqlStatus stat = binder->BindPoint2d(value);
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindPoint3d(DPoint3dCR value)
    {
    for (IECSqlBinder* binder : m_binders)
        {
        ECSqlStatus stat = binder->BindPoint3d(value);
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
ECSqlStatus CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    for (IECSqlBinder* binder : m_binders)
        {
        ECSqlStatus stat = binder->BindText(value, makeCopy, byteCount);
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
IECSqlBinder& CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindStructMember(Utf8CP structMemberPropertyName)
    {
    auto it = m_structMemberProxyBindersByName.find(structMemberPropertyName);
    if (it != m_structMemberProxyBindersByName.end())
        return *it->second;

    auto ret = m_structMemberProxyBindersByName.insert(std::make_pair(structMemberPropertyName, std::make_unique<ProxyECSqlBinder>()));
    ProxyECSqlBinder& memberProxyBinder = *ret.first->second;

    for (IECSqlBinder* binderP : m_binders)
        {
        IECSqlBinder& binder = *binderP;
        memberProxyBinder.AddBinder(binder[structMemberPropertyName]);
        }

    return memberProxyBinder;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
IECSqlBinder& CompoundECSqlPreparedStatement::ProxyECSqlBinder::_BindStructMember(ECN::ECPropertyId structMemberPropertyId)
    {
    auto it = m_structMemberProxyBindersById.find(structMemberPropertyId);
    if (it != m_structMemberProxyBindersById.end())
        return *it->second;

    auto ret = m_structMemberProxyBindersById.insert(std::make_pair(structMemberPropertyId, std::make_unique<ProxyECSqlBinder>()));
    ProxyECSqlBinder& memberProxyBinder = *ret.first->second;

    for (IECSqlBinder* binderP : m_binders)
        {
        IECSqlBinder& binder = *binderP;
        memberProxyBinder.AddBinder(binder[structMemberPropertyId]);
        }

    return memberProxyBinder;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/17
//---------------------------------------------------------------------------------------
IECSqlBinder& CompoundECSqlPreparedStatement::ProxyECSqlBinder::_AddArrayElement()
    {
    m_arrayElementProxyBinder = std::make_unique<ProxyECSqlBinder>();

    for (IECSqlBinder* binderP : m_binders)
        {
        IECSqlBinder& binder = *binderP;
        m_arrayElementProxyBinder->AddBinder(binder.AddArrayElement());
        }

    return *m_arrayElementProxyBinder;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

#endif