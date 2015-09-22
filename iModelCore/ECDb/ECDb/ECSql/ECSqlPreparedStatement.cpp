/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPreparedStatement.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlPreparedStatement.h"
#include "ECSqlPreparer.h"
#include "ECSqlStatementNoopImpls.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//***************************************************************************************
//    ECSqlStatementState
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlPreparedStatement::ECSqlPreparedStatement(ECSqlType type, ECDbCR ecdb)
: m_type(type), m_ecdb(&ecdb), m_isNoopInSqlite(false), m_isNothingToUpdate(false) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement::Prepare(ECSqlPrepareContext& prepareContext, ECSqlParseTreeCR ecsqlParseTree, Utf8CP ecsql)
    {
    auto const& ecdb = GetECDb();

    if (GetType() != ECSqlType::Select && ecdb.IsReadonly())
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECDb file is opened read-only. For data-modifying ECSQL statements write access is needed.");
        return ECSqlStatus::Error;
        }

    Utf8String nativeSql;
    auto stat = ECSqlPreparer::Prepare(nativeSql, prepareContext, ecsqlParseTree);
    if (!stat.IsSuccess())
        return stat;

    m_ecsql = Utf8String(ecsql);

    if (prepareContext.NativeStatementIsNoop())
        m_isNoopInSqlite = true;
    else if (prepareContext.NativeNothingToUpdate())
        {
        m_isNothingToUpdate = true;
        }
    else
        {
        //don't let BeSQLite log and assert on error (therefore use TryPrepare instead of Prepare)
        const auto nativeSqlStat = GetSqliteStatementR().TryPrepare(ecdb, nativeSql.c_str());
        if (nativeSqlStat != BE_SQLITE_OK)
            {
            Utf8String errorMessage;
            errorMessage.Sprintf("Preparing the SQLite statement '%s' failed with error code", nativeSql.c_str());
            GetIssueReporter().ReportSqliteIssue(ECDbIssueSeverity::Error, nativeSqlStat, errorMessage.c_str());
            return ECSqlStatus(nativeSqlStat);
            }
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& ECSqlPreparedStatement::GetBinder(int parameterIndex)
    {
    ECSqlBinder* binder = nullptr;
    const ECSqlStatus stat = GetParameterMap().TryGetBinder(binder, parameterIndex);

    if (stat.IsSuccess() && !m_isNoopInSqlite)
        return *binder;

    if (stat == ECSqlStatus::Error)
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Parameter index %d passed to ECSqlStatement binding API is out of bounds.", parameterIndex);

    return NoopECSqlBinderFactory::GetBinder(stat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
int ECSqlPreparedStatement::GetParameterIndex(Utf8CP parameterName) const
    {
    int index = GetParameterMap().GetIndexForName(parameterName);
    if (index <= 0)
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "No parameter index found for parameter name :%s.", parameterName);

    return index;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement::ClearBindings()
    {
    if (m_isNoopInSqlite)
        return ECSqlStatus::Success;

    const DbResult nativeSqlStat = GetSqliteStatementR ().ClearBindings();
    GetParameterMapR ().OnClearBindings();

    if (nativeSqlStat != BE_SQLITE_OK)
        {
        GetIssueReporter().ReportSqliteIssue(ECDbIssueSeverity::Error, nativeSqlStat);
        return ECSqlStatus(nativeSqlStat);
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
DbResult ECSqlPreparedStatement::DoStep()
    {
    if (!m_parameterMap.OnBeforeStep().IsSuccess())
        return BE_SQLITE_ERROR;
    
    const DbResult nativeSqlStatus = GetSqliteStatementR ().Step();
    if (BE_SQLITE_ROW != nativeSqlStatus && BE_SQLITE_DONE != nativeSqlStatus)
        GetIssueReporter().ReportSqliteIssue(ECDbIssueSeverity::Error, nativeSqlStatus);

    return nativeSqlStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement::Reset()
    {
    return _Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement::DoReset()
    {
    DbResult nativeSqlStat = GetSqliteStatementR ().Reset();
    if (nativeSqlStat != BE_SQLITE_OK)
        {
        GetIssueReporter().ReportSqliteIssue(ECDbIssueSeverity::Error, nativeSqlStat);
        return ECSqlStatus(nativeSqlStat);
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
Utf8CP ECSqlPreparedStatement::GetNativeSql() const
    {
    if (m_isNoopInSqlite)
        {
        LOG.warning("ECSqlStatement::GetNativeSql> No native SQL available. ECSQL translates to a no-op in native SQL.");
        return "n/a";
        }

    return GetSqliteStatementR ().GetSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
void ECSqlPreparedStatement::AddKeepAliveSchema(ECN::ECSchemaCR schema)
    {
    for (auto& keepAlive : m_keepAliveSchemas)
    if (&schema == keepAlive.get())
        return;

    m_keepAliveSchemas.push_back(const_cast<ECSchemaP>(&schema));
    }


//***************************************************************************************
//    ECSqlSelectPreparedStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
DbResult ECSqlSelectPreparedStatement::Step()
    {
    const DbResult stat = DoStep();
    if (BE_SQLITE_ROW == stat)
        {
        if (!InitFields().IsSuccess())
            return BE_SQLITE_ERROR;
        }

    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlSelectPreparedStatement::_Reset()
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
int ECSqlSelectPreparedStatement::GetColumnCount() const
    {
    return (int) (GetFields().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
IECSqlValue const& ECSqlSelectPreparedStatement::GetValue(int columnIndex) const
    {
    if (columnIndex < 0 || columnIndex >= (int) (m_fields.size()))
        {
        GetIssueReporter().Report(ECDbIssueSeverity::Error, "Column index '%d' is out of bounds.", columnIndex);
        return NoopECSqlValue::GetSingleton();
        }

    std::unique_ptr<ECSqlField> const& field = m_fields[columnIndex];
    return *field;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlSelectPreparedStatement::ResetFields() const
    {
    for (std::unique_ptr<ECSqlField> const& field : m_fields)
        {
        ECSqlStatus stat = field->Reset();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlSelectPreparedStatement::InitFields() const
    {
    for (std::unique_ptr<ECSqlField> const& field : m_fields)
        {
        ECSqlStatus stat = field->Init();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
void ECSqlSelectPreparedStatement::AddField(std::unique_ptr<ECSqlField> field)
    {
    BeAssert(field != nullptr);
    if (field != nullptr)
        {
        m_fields.push_back(std::move(field));
        }
    else
        {
        BeAssert(false && "Field is null");
        }
    }


//***************************************************************************************
//    ECSqlInsertStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
DbResult ECSqlInsertPreparedStatement::Step(ECInstanceKey& instanceKey)
    {
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
        //user hasn't provided an ecinstanceid (neither literally nor through binding) -> auto generate it
        if (GenerateECInstanceIdAndBindToInsertStatement(ecinstanceidOfInsert) != ECSqlStatus::Success)
            return BE_SQLITE_ERROR;

        checkModifiedRowCount = false;
        }

    //reset the ecinstanceid from key info for the next execution (if it was bound, and is no literal)
    m_ecInstanceKeyInfo.ResetBoundECInstanceId();

    BeAssert(ecinstanceidOfInsert.IsValid());
    DbResult stat = DoStep();
    if (BE_SQLITE_DONE == stat)
        {
        if (checkModifiedRowCount && GetECDb().GetModifiedRowCount() == 0)
            {
            //this can only happen in a specific case with inserting an end table relationship, as there inserting really
            //means to update a row in the end table.
            GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Could not insert the ECRelationship. Either the source or target constraint's ECInstanceId does not exist or the source or target constraint's cardinality is violated.");
            return BE_SQLITE_ERROR;
            }

        instanceKey = ECInstanceKey(m_ecInstanceKeyInfo.GetECClassId(), ecinstanceidOfInsert);

        if (GetStepTasks().HasAnyTask())
            {
            stat = GetStepTasks().ExecuteAfterStepTaskList(ecinstanceidOfInsert);
            if (BE_SQLITE_OK != stat)
                return stat;
            }

        return BE_SQLITE_DONE;
        }

    return stat;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlInsertPreparedStatement::GenerateECInstanceIdAndBindToInsertStatement(ECInstanceId& generatedECInstanceId)
    {
    ECSqlBinder* ecinstanceidBinder = m_ecInstanceKeyInfo.GetECInstanceIdBinder();
    BeAssert(ecinstanceidBinder != nullptr);

    DbResult dbStat = GetECDb().GetECDbImplR ().GetECInstanceIdSequence().GetNextValue<ECInstanceId> (generatedECInstanceId);
    if (dbStat != BE_SQLITE_OK)
        {
        GetIssueReporter().ReportSqliteIssue(ECDbIssueSeverity::Error, dbStat, "ECSqlStatement::Step failed: Could not generate an ECInstanceId.");
        return ECSqlStatus(dbStat);
        }

    const ECSqlStatus stat = ecinstanceidBinder->BindId(generatedECInstanceId);
    if (!stat.IsSuccess())
        GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECSqlStatement::Step failed: Could not bind the generated ECInstanceId.");

    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        07/14
//---------------------------------------------------------------------------------------
void ECSqlInsertPreparedStatement::SetECInstanceKeyInfo(ECInstanceKeyInfo const& ecInstanceKeyInfo)
    {
    BeAssert(ecInstanceKeyInfo.GetECClassId() > ECClass::UNSET_ECCLASSID);
    m_ecInstanceKeyInfo = ecInstanceKeyInfo;
    }

//******************************************************************************************
// ECSqlUpdatePreparedStatement
//******************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan             04/14
//---------------------------------------------------------------------------------------
DbResult ECSqlUpdatePreparedStatement::Step()
    {
    if (!IsNoopInSqlite())
        {
        DbResult status = GetStepTasks().ExecuteBeforeStepTaskList();
        if (BE_SQLITE_OK != status)
            return status;

        if (!IsNothingToUpdate())
            return DoStep();
        }

    return BE_SQLITE_DONE;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan             04/14
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlNonSelectPreparedStatement::_Reset()
    {
    if (GetStepTasks().HasSelector())
        GetStepTasks().ResetSelector();

    return DoReset();
    }


//******************************************************************************************
// ECSqlDeletePreparedStatement
//******************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan             04/14
//---------------------------------------------------------------------------------------
DbResult ECSqlDeletePreparedStatement::Step()
    {
    if (!IsNoopInSqlite())
        {
        const DbResult status = GetStepTasks().ExecuteBeforeStepTaskList();
        if (BE_SQLITE_OK != status)
            return status;

        return DoStep();
        }

    return BE_SQLITE_DONE;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
