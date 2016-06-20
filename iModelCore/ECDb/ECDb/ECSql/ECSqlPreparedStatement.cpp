/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPreparedStatement.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlPreparedStatement.h"
#include "ECSqlPreparer.h"
#include "ECSqlStatementNoopImpls.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//***************************************************************************************
//    ECSqlStatementState
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlPreparedStatement::ECSqlPreparedStatement(ECSqlType type, ECDbCR ecdb)
: m_type(type), m_ecdb(&ecdb), m_isNoopInSqlite(false), m_parentOfJoinedTableECSqlStatement(nullptr) {}


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        01/16
//---------------------------------------------------------------------------------------
ParentOfJoinedTableECSqlStatement* ECSqlPreparedStatement::CreateParentOfJoinedTableECSqlStatement(ECClassId joinedTableClassId)
    {
    BeAssert(m_parentOfJoinedTableECSqlStatement == nullptr && "CreateParentOfJoinedTableECSqlStatement expects the statement to not exist prior to this call.");
    m_parentOfJoinedTableECSqlStatement = std::unique_ptr<ParentOfJoinedTableECSqlStatement>(new ParentOfJoinedTableECSqlStatement(joinedTableClassId));
    return m_parentOfJoinedTableECSqlStatement.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        01/16
//---------------------------------------------------------------------------------------
ParentOfJoinedTableECSqlStatement* ECSqlPreparedStatement::GetParentOfJoinedTableECSqlStatement() const
    {
    return m_parentOfJoinedTableECSqlStatement.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement::Prepare(ECSqlPrepareContext& prepareContext, Exp const& exp, Utf8CP ecsql)
    {
    BeAssert(m_nativeSql.empty());
    ECDbCR ecdb = GetECDb();

    if (GetType() != ECSqlType::Select && ecdb.IsReadonly())
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECDb file is opened read-only. For data-modifying ECSQL statements write access is needed.");
        return ECSqlStatus::Error;
        }

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
        DbResult nativeSqlStat = GetSqliteStatementR().TryPrepare(ecdb, nativeSql.c_str());
        if (nativeSqlStat != BE_SQLITE_OK)
            {
            Utf8String errorMessage;
            errorMessage.Sprintf("Preparing the ECSQL '%s' failed. Underlying SQLite statement '%s' failed to prepare with error code", ecsql, nativeSql.c_str());
            GetIssueReporter().ReportSqliteIssue(ECDbIssueSeverity::Error, nativeSqlStat, errorMessage.c_str());
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
IECSqlBinder& ECSqlPreparedStatement::GetBinder(int parameterIndex)
    {
    ECSqlBinder* binder = nullptr;
    const ECSqlStatus stat = GetParameterMap().TryGetBinder(binder, parameterIndex);

    if (stat.IsSuccess() && !m_isNoopInSqlite)
        return *binder;

    if (stat == ECSqlStatus::Error)
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Parameter index %d passed to ECSqlStatement binding API is out of bounds.", parameterIndex);

    return NoopECSqlBinder::Get();
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

    if (auto joinedTableStmt = GetParentOfJoinedTableECSqlStatement())
        joinedTableStmt->ClearBindings();

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
        {
        Utf8String msg;
        msg.Sprintf("Step failed for ECSQL '%s': SQLite Step failed [Native SQL: '%s'] with. Error:", GetECSql(), GetNativeSql());
        GetIssueReporter().ReportSqliteIssue(ECDbIssueSeverity::Error, nativeSqlStatus, msg.c_str());
        }
    return nativeSqlStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement::Reset()
    {
    if (auto joinedTableStmt = GetParentOfJoinedTableECSqlStatement())
        joinedTableStmt->Reset();

    return _Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement::DoReset()
    {
    const DbResult nativeSqlStat = GetSqliteStatementR ().Reset();
    if (nativeSqlStat != BE_SQLITE_OK)
        return ECSqlStatus(nativeSqlStat);

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
Utf8CP ECSqlPreparedStatement::GetNativeSql() const
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
DbResult ECSqlSelectPreparedStatement::Step()
    {
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
// @bsimethod                                                Krischan.Eberle        10/13
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
        //user hasn't provided an ecinstanceid (neither literally nor through binding)
        if (m_isECInstanceIdAutogenerationDisabled)
            {
            GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Always must provide an ECInstanceId for the ECSQL %s as ECInstanceId auto-generation was disabled for the ECClass (via custom attribute).", GetECSql());
            return BE_SQLITE_ERROR;
            }

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
            GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Could not insert the ECRelationship (%s). Either the source or target constraint's ECInstanceId does not exist or the source or target constraint's cardinality is violated.", GetECSql());
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
    BeAssert(ecInstanceKeyInfo.GetECClassId().IsValid());
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan             04/14
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlNonSelectPreparedStatement::_Reset()
    {
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
    if (IsNoopInSqlite())
        return BE_SQLITE_DONE;

    return DoStep();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
