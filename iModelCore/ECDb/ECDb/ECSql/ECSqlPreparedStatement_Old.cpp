/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPreparedStatement_Old.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//***************************************************************************************
//    ECSqlStatementState
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlPreparedStatement_Old::ECSqlPreparedStatement_Old(ECSqlType type, ECDbCR ecdb)
: m_type(type), m_ecdb(ecdb), m_isNoopInSqlite(false), m_parentOfJoinedTableECSqlStatement(nullptr) {}


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        01/16
//---------------------------------------------------------------------------------------
ParentOfJoinedTableECSqlStatement* ECSqlPreparedStatement_Old::CreateParentOfJoinedTableECSqlStatement(ECClassId joinedTableClassId)
    {
    BeAssert(m_parentOfJoinedTableECSqlStatement == nullptr && "CreateParentOfJoinedTableECSqlStatement expects the statement to not exist prior to this call.");
    m_parentOfJoinedTableECSqlStatement = std::unique_ptr<ParentOfJoinedTableECSqlStatement>(new ParentOfJoinedTableECSqlStatement(joinedTableClassId));
    return m_parentOfJoinedTableECSqlStatement.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        01/16
//---------------------------------------------------------------------------------------
ParentOfJoinedTableECSqlStatement* ECSqlPreparedStatement_Old::GetParentOfJoinedTableECSqlStatement() const
    {
    return m_parentOfJoinedTableECSqlStatement.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement_Old::Prepare(ECSqlPrepareContext& prepareContext, Exp const& exp, Utf8CP ecsql)
    {
    BeAssert(m_nativeSql.empty());

    if (GetType() != ECSqlType::Select && m_ecdb.IsReadonly())
        {
        LOG.error("ECDb file is opened read-only. For data-modifying ECSQL statements write access is needed.");
        return ECSqlStatus::Error;
        }

    //capture current clear cache counter so that we can invalidate the statement if another clear cache call 
    //occurred in the lifetime of the statement
    m_preparationClearCacheCounter = m_ecdb.GetECDbImplR().GetClearCacheCounter();

    Utf8String nativeSql;
    ECSqlStatus stat = ECSqlPreparer::Prepare(nativeSql, prepareContext, exp);
    if (!stat.IsSuccess())
        return stat;

    ECSqlPrepareContext::JoinedTableInfo const* info = prepareContext.GetJoinedTableInfo();
    if (info != nullptr)
        m_ecsql.assign(info->GetOrignalECSql());
    else
        m_ecsql.assign(ecsql);

    if (prepareContext.NativeStatementIsNoop())
        {
        m_isNoopInSqlite = true;
        m_nativeSql = "n/a";
        }
    else
        {
        //don't let BeSQLite log and assert on error (therefore use TryPrepare instead of Prepare)
        DbResult nativeSqlStat = GetSqliteStatementR().TryPrepare(m_ecdb, nativeSql.c_str());
        if (nativeSqlStat != BE_SQLITE_OK)
            {
            GetECDb().GetECDbImplR().GetIssueReporter().Report("Preparing the ECSQL '%s' failed. Underlying SQLite statement '%s' failed to prepare: %s %s", ecsql, nativeSql.c_str(),
                                                                   ECDb::InterpretDbResult(nativeSqlStat), GetECDb().GetLastError().c_str());

            //even if this is a SQLite error, we want this to be an InvalidECSql error as the reason usually
            //is a wrong ECSQL provided by the user.
            return ECSqlStatus::InvalidECSql;
            }

        ParentOfJoinedTableECSqlStatement* parentOfJoinedTableECSqlStmt = GetParentOfJoinedTableECSqlStatement();
        if (parentOfJoinedTableECSqlStmt != nullptr)
            m_nativeSql.assign(parentOfJoinedTableECSqlStmt->GetNativeSql()).append(";");

        m_nativeSql.append(nativeSql);
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& ECSqlPreparedStatement_Old::GetBinder(int parameterIndex)
    {
    if (SUCCESS != AssertIsValid())
        return NoopECSqlBinder::Get();

    ECSqlBinder* binder = nullptr;
    const ECSqlStatus stat = GetParameterMap().TryGetBinder(binder, parameterIndex);

    if (stat.IsSuccess() && !m_isNoopInSqlite)
        return *binder;

    if (stat == ECSqlStatus::Error)
        LOG.errorv("Parameter index %d passed to ECSqlStatement binding API is out of bounds.", parameterIndex);

    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
int ECSqlPreparedStatement_Old::GetParameterIndex(Utf8CP parameterName) const
    {
    if (SUCCESS != AssertIsValid())
        return -1;

    int index = GetParameterMap().GetIndexForName(parameterName);
    if (index <= 0)
        LOG.errorv("No parameter index found for parameter name :%s.", parameterName);

    return index;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement_Old::ClearBindings()
    {
    if (SUCCESS != AssertIsValid())
        return ECSqlStatus::Error;

    if (m_isNoopInSqlite)
        return ECSqlStatus::Success;

    if (auto joinedTableStmt = GetParentOfJoinedTableECSqlStatement())
        joinedTableStmt->ClearBindings();

    const DbResult nativeSqlStat = GetSqliteStatementR ().ClearBindings();
    GetParameterMapR ().OnClearBindings();

    if (nativeSqlStat != BE_SQLITE_OK)
        {
        ECDbLogger::LogSqliteError(GetECDb(), nativeSqlStat);
        return ECSqlStatus(nativeSqlStat);
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
DbResult ECSqlPreparedStatement_Old::DoStep()
    {
    if (SUCCESS != AssertIsValid())
        return BE_SQLITE_ERROR;

    if (!m_parameterMap.OnBeforeStep().IsSuccess())
        return BE_SQLITE_ERROR;
    
    const DbResult nativeSqlStatus = GetSqliteStatementR ().Step();
    if (BE_SQLITE_ROW != nativeSqlStatus && BE_SQLITE_DONE != nativeSqlStatus)
        {
        Utf8String msg;
        msg.Sprintf("Step failed for ECSQL '%s': SQLite Step failed [Native SQL: '%s'] with. Error:", GetECSql(), GetNativeSql());
        ECDbLogger::LogSqliteError(GetECDb(), nativeSqlStatus, msg.c_str());
        }
    return nativeSqlStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement_Old::Reset()
    {
    if (SUCCESS != AssertIsValid())
        return ECSqlStatus::Error;

    if (auto joinedTableStmt = GetParentOfJoinedTableECSqlStatement())
        joinedTableStmt->Reset();

    return _Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement_Old::DoReset()
    {
    const DbResult nativeSqlStat = GetSqliteStatementR ().Reset();
    if (nativeSqlStat != BE_SQLITE_OK)
        return ECSqlStatus(nativeSqlStat);

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlPreparedStatement_Old::AssertIsValid() const
    {
    if (m_preparationClearCacheCounter != m_ecdb.GetECDbImplR().GetClearCacheCounter())
        {
        LOG.errorv("The ECSqlStatement '%s' can no longer be used because an ECSchema import took place. ECSqlStatements are invalidated after an ECSchema import.",
                        m_ecsql.c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
Utf8CP ECSqlPreparedStatement_Old::GetNativeSql() const
    {
    if (m_isNoopInSqlite)
        LOG.warning("ECSqlStatement::GetNativeSql> No native SQL available. ECSQL translates to a no-op in native SQL.");

    return m_nativeSql.c_str();
    }


//***************************************************************************************
//    ECSqlSelectPreparedStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
DbResult ECSqlSelectPreparedStatement_Old::Step()
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
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlSelectPreparedStatement_Old::_Reset()
    {
    ECSqlStatus resetStatementStat = DoReset();
    //even if statement reset failed we still try to reset the fields to clean-up things as good as possible.
    ECSqlStatus fieldResetStat = ResetFields();

    if (resetStatementStat != ECSqlStatus::Success)
        return resetStatementStat;

    if (fieldResetStat != ECSqlStatus::Success)
        return fieldResetStat;

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
int ECSqlSelectPreparedStatement_Old::GetColumnCount() const
    {
    if (SUCCESS != AssertIsValid())
        return -1;

    return (int) (GetFields().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
IECSqlValue const& ECSqlSelectPreparedStatement_Old::GetValue(int columnIndex) const
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
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlSelectPreparedStatement_Old::ResetFields() const
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
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlSelectPreparedStatement_Old::OnAfterStep() const
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
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
void ECSqlSelectPreparedStatement_Old::AddField(std::unique_ptr<ECSqlField> field)
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
//    ECSqlInsertStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
DbResult ECSqlInsertPreparedStatement_Old::Step(ECInstanceKey& instanceKey)
    {
    if (SUCCESS != AssertIsValid())
        return BE_SQLITE_ERROR;

    if (IsNoopInSqlite())
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
    if (auto joinedTableStmt = GetParentOfJoinedTableECSqlStatement())
        {      
        if (auto binder = joinedTableStmt->GetECInstanceIdBinder())
            {
            binder->BindId(ecinstanceidOfInsert);
            }

        auto r = joinedTableStmt->Step();
        if (r != DbResult::BE_SQLITE_DONE)
            return r;
        }

    BeAssert(ecinstanceidOfInsert.IsValid());
    DbResult stat = DoStep();
    if (BE_SQLITE_DONE == stat)
        {
        if (checkModifiedRowCount && GetECDb().GetModifiedRowCount() == 0)
            {
            //this can only happen in a specific case with inserting an end table relationship, as there inserting really
            //means to update a row in the end table.
            GetECDb().GetECDbImplR().GetIssueReporter().Report("Could not insert the ECRelationship (%s). Either the source or target constraint's " ECDBSYS_PROP_ECInstanceId " does not exist or the source or target constraint's cardinality is violated.", GetECSql());
            return BE_SQLITE_CONSTRAINT_UNIQUE;
            }

        instanceKey = ECInstanceKey(m_ecInstanceKeyInfo.GetECClassId(), ecinstanceidOfInsert);
        return BE_SQLITE_DONE;
        }

    return stat;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlInsertPreparedStatement_Old::GenerateECInstanceIdAndBindToInsertStatement(ECInstanceId& generatedECInstanceId)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        07/14
//---------------------------------------------------------------------------------------
void ECSqlInsertPreparedStatement_Old::SetECInstanceKeyInfo(ECInstanceKeyInfo const& ecInstanceKeyInfo)
    {
    BeAssert(ecInstanceKeyInfo.GetECClassId().IsValid());
    m_ecInstanceKeyInfo = ecInstanceKeyInfo;
    }

//******************************************************************************************
// ECSqlUpdatePreparedStatement
//******************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan             04/14
//---------------------------------------------------------------------------------------
DbResult ECSqlUpdatePreparedStatement_Old::Step()
    {
    if (SUCCESS != AssertIsValid())
        return BE_SQLITE_ERROR;

    if (IsNoopInSqlite())
        return BE_SQLITE_DONE;

    if (auto parentOfJoinedTableStmt = GetParentOfJoinedTableECSqlStatement())
        {
        const DbResult status = parentOfJoinedTableStmt->Step();
        if (status != BE_SQLITE_DONE)
            return status;
        }

    return DoStep();
    }


//******************************************************************************************
// ECSqlDeletePreparedStatement
//******************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan             04/14
//---------------------------------------------------------------------------------------
DbResult ECSqlDeletePreparedStatement_Old::Step()
    {
    if (SUCCESS != AssertIsValid())
        return BE_SQLITE_ERROR;

    if (IsNoopInSqlite())
        return BE_SQLITE_DONE;

    return DoStep();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
