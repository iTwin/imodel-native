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
ECSqlPreparedStatement::ECSqlPreparedStatement (ECSqlType type, ECDbCR ecdb, ECSqlStatusContext& statusContext)
: m_type (type), m_ecdb (&ecdb), m_isNoopInSqlite (false), m_statusContext (statusContext),m_isNothingToUpdate(false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement::Prepare (ECSqlPrepareContext& prepareContext, ECSqlParseTreeCR ecsqlParseTree, Utf8CP ecsql)
    {
    auto const& ecdb = GetECDb ();

    if (GetType () != ECSqlType::Select && ecdb.IsReadonly ())
        return GetStatusContextR ().SetError (ECSqlStatus::UserError, "ECDb file is opened read-only. For data-modifying ECSQL statements write access is needed.");

    Utf8String nativeSql;
    auto stat = ECSqlPreparer::Prepare (nativeSql, prepareContext, ecsqlParseTree);
    if (stat != ECSqlStatus::Success)
        return stat;

    m_ecsql = Utf8String (ecsql);

    if (prepareContext.NativeStatementIsNoop ())
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
            errorMessage.Sprintf ("Preparing the SQLite statement '%s' failed with error code", nativeSql.c_str ());
            return GetStatusContextR ().SetError (&ecdb, nativeSqlStat, errorMessage.c_str ());
            }
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& ECSqlPreparedStatement::GetBinder (int parameterIndex)
    {
    ECSqlBinder* binder = nullptr;
    const auto stat = GetParameterMap ().TryGetBinder (binder, parameterIndex);

    if (stat == ECSqlStatus::Success && !m_isNoopInSqlite)
        return *binder;

    if (stat == ECSqlStatus::IndexOutOfBounds)
        GetStatusContextR ().SetError (stat, "Parameter index passed to ECSqlStatement binding API is out of bounds.");
    else
        GetStatusContextR ().SetError (stat, nullptr);

    return NoopECSqlBinderFactory::GetBinder (stat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
int ECSqlPreparedStatement::GetParameterIndex (Utf8CP parameterName) const
    {
    auto index = GetParameterMap ().GetIndexForName (parameterName);
    if (index <= 0)
        GetStatusContextR ().SetError (ECSqlStatus::IndexOutOfBounds, "No parameter index found for parameter name :%s.", parameterName);

    return index;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement::ClearBindings ()
    {
    if (m_isNoopInSqlite)
        {
        ResetStatus ();
        return ECSqlStatus::Success;
        }

    auto nativeSqlStat = GetSqliteStatementR ().ClearBindings ();
    GetParameterMapR ().OnClearBindings ();

    if (nativeSqlStat != BE_SQLITE_OK)
        return GetStatusContextR ().SetError (&GetECDb (), nativeSqlStat);
    else
        return ResetStatus ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        04/15
//---------------------------------------------------------------------------------------
void ECSqlPreparedStatement::OnBeforeStep()
    {

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStepStatus ECSqlPreparedStatement::DoStep ()
    {
    if (m_parameterMap.OnBeforeStep () != ECSqlStatus::Success)
        return ECSqlStepStatus::Error;
    
    const auto nativeSqlStatus = GetSqliteStatementR ().Step ();
    switch (nativeSqlStatus)
        {
        case BE_SQLITE_ROW:
            return ECSqlStepStatus::HasRow;
        case BE_SQLITE_DONE:
            return ECSqlStepStatus::Done;

        default:
            {
            GetStatusContextR ().SetError (&GetECDb (), nativeSqlStatus);
            return ECSqlStepStatus::Error;
            }
        };
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement::Reset ()
    {
    return _Reset ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement::DoReset ()
    {
    auto nativeSqlStat = GetSqliteStatementR ().Reset ();
    if (nativeSqlStat != BE_SQLITE_OK)
        return GetStatusContextR ().SetError (&GetECDb (), nativeSqlStat);

    return ResetStatus ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
Utf8CP ECSqlPreparedStatement::GetECSql () const
    {
    return m_ecsql.c_str ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
Utf8CP ECSqlPreparedStatement::GetNativeSql () const
    {
    if (m_isNoopInSqlite)
        {
        LOG.warning ("ECSqlStatement::GetNativeSql> No native SQL available. ECSQL translates to a no-op in native SQL.");
        return "n/a";
        }

    return GetSqliteStatementR ().GetSql ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
void ECSqlPreparedStatement::AddKeepAliveSchema (ECN::ECSchemaCR schema)
    {
    for (auto& keepAlive : m_keepAliveSchemas)
    if (&schema == keepAlive.get ())
        return;

    m_keepAliveSchemas.push_back (const_cast<ECSchemaP>(&schema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlPreparedStatement::ResetStatus () const
    {
    return GetStatusContextR ().Reset ();
    }


//***************************************************************************************
//    ECSqlSelectPreparedStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStepStatus ECSqlSelectPreparedStatement::Step ()
    {
    OnBeforeStep();

    const auto stat = DoStep ();
    if (stat == ECSqlStepStatus::HasRow)
        {
        if (InitFields () != ECSqlStatus::Success)
            return ECSqlStepStatus::Error;
        }

    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlSelectPreparedStatement::_Reset ()
    {
    auto resetStatementStat = DoReset ();
    //even if statement reset failed we still try to reset the fields to clean-up things as good as possible.
    auto fieldResetStat = ResetFields ();

    if (resetStatementStat != ECSqlStatus::Success)
        return resetStatementStat;

    if (fieldResetStat != ECSqlStatus::Success)
        return fieldResetStat;

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
int ECSqlSelectPreparedStatement::GetColumnCount () const
    {
    return static_cast<int> (GetFields ().size ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
IECSqlValue const& ECSqlSelectPreparedStatement::GetValue (int columnIndex) const
    {
    if (columnIndex < 0 || columnIndex >= static_cast<int> (m_fields.size ()))
        {
        GetStatusContextR ().SetError (ECSqlStatus::IndexOutOfBounds, "Column index '%d' is out of bounds.", columnIndex);
        return NoopECSqlValue::GetSingleton ();
        }

    ResetStatus ();
    auto const& field = m_fields[columnIndex];
    return *field;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlSelectPreparedStatement::ResetFields () const
    {
    for (std::unique_ptr<ECSqlField> const& field : m_fields)
        {
        auto stat = field->Reset (GetStatusContextR ());
        if (stat != ECSqlStatus::Success)
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlSelectPreparedStatement::InitFields () const
    {
    for (std::unique_ptr<ECSqlField> const& field : m_fields)
        {
        auto stat = field->Init (GetStatusContextR ());
        if (stat != ECSqlStatus::Success)
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
void ECSqlSelectPreparedStatement::AddField (std::unique_ptr<ECSqlField> field)
    {
    BeAssert (field != nullptr);
    if (field != nullptr)
        {
        m_fields.push_back (std::move (field));
        }
    else
        {
        BeAssert (false && "Field is null");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
DynamicSelectClauseECClass& ECSqlSelectPreparedStatement::GetDynamicSelectClauseECClassR ()
    {
    return m_dynamicSelectClauseECClass;
    }


//***************************************************************************************
//    ECSqlInsertStatement
//***************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlInsertPreparedStatement::ECSqlInsertPreparedStatement (ECDbCR ecdb, ECSqlStatusContext& statusContext)
: ECSqlNonSelectPreparedStatement (ECSqlType::Insert, ecdb, statusContext)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStepStatus ECSqlInsertPreparedStatement::Step (ECInstanceKey& instanceKey)
    {
    OnBeforeStep();


    if (IsNoopInSqlite ())
        {

        return ECSqlStepStatus::Done;
        }         

    ECInstanceId ecinstanceidOfInsert;
    //GetModifiedRowCount can be costly, so we want to avoid calling it when not needed.
    //For regular case where native SQL is an insert statement with an ECInstanceId
    //generated by ECDb, we do not need to call GetModifiedRowCount (as it is expected to be 1).
    //For some special cases though (end table relationship insert), the native SQL is not an insert,
    //but an update, for which we need to check whether a row was updated or not to tell between
    //success and error.
    bool checkModifiedRowCount = true; 
    if (m_ecInstanceKeyInfo.HasUserProvidedECInstanceId ())
        ecinstanceidOfInsert = m_ecInstanceKeyInfo.GetUserProvidedECInstanceId ();
    else
        {
        //user hasn't provided an ecinstanceid (neither literally nor through binding) -> auto generate it
        if (GenerateECInstanceIdAndBindToInsertStatement (ecinstanceidOfInsert) != ECSqlStatus::Success)
            return ECSqlStepStatus::Error;

        checkModifiedRowCount = false;
        }

    //reset the ecinstanceid from key info for the next execution (if it was bound, and is no literal)
    m_ecInstanceKeyInfo.ResetBoundECInstanceId ();

    BeAssert (ecinstanceidOfInsert.IsValid ());
    auto stat = DoStep ();
    if (stat == ECSqlStepStatus::Done)
        {
        if (checkModifiedRowCount && GetECDb ().GetModifiedRowCount () == 0)
            {
            //this can only happen in a specific case with inserting an end table relationship, as there inserting really
            //means to update a row in the end table.
            GetStatusContextR ().SetError (ECSqlStatus::UserError, "Could not insert the ECRelationship. Either the source or target constraint's ECInstanceId does not exist or the source or target constraint's cardinality is violated.");
            return ECSqlStepStatus::Error;
            }


        instanceKey = ECInstanceKey (m_ecInstanceKeyInfo.GetECClassId (), ecinstanceidOfInsert);

        if (GetStepTasks ().HasAnyTask ())
            {
            if (GetStepTasks ().ExecuteAfterStepTaskList (ecinstanceidOfInsert) == ECSqlStepStatus::Error)
                {
                //error handling already done in child call
                BeAssert (GetStatusContextR ().GetStatus () != ECSqlStatus::Success && " Child call GetStepTasks ().ExecuteAfterStepTaskList is expected to have set the status before returning.");
                return ECSqlStepStatus::Error;
                }
            }

        return ECSqlStepStatus::Done;
        }
    else
        //error status already set by child calls
        return ECSqlStepStatus::Error;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlInsertPreparedStatement::GenerateECInstanceIdAndBindToInsertStatement (ECInstanceId& generatedECInstanceId)
    {
    ECSqlBinder* ecinstanceidBinder = m_ecInstanceKeyInfo.GetECInstanceIdBinder ();
    BeAssert (ecinstanceidBinder != nullptr);

    auto dbStat = GetECDb ().GetECDbImplR ().GetECInstanceIdSequence ().GetNextValue<ECInstanceId> (generatedECInstanceId);
    if (dbStat != BE_SQLITE_OK)
        return GetStatusContextR ().SetError (&GetECDb (), dbStat, "ECSqlStatement::Step failed: Could not generate an ECInstanceId.");

    auto stat = ecinstanceidBinder->BindId (generatedECInstanceId);
    if (stat != ECSqlStatus::Success)
        return GetStatusContextR ().SetError (stat, "ECSqlStatement::Step failed: Could not bind the generated ECInstanceId.");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        07/14
//---------------------------------------------------------------------------------------
void ECSqlInsertPreparedStatement::SetECInstanceKeyInfo (ECInstanceKeyInfo const& ecInstanceKeyInfo)
    {
    BeAssert (ecInstanceKeyInfo.GetECClassId () > 0LL);
    m_ecInstanceKeyInfo = ecInstanceKeyInfo;
    }

//******************************************************************************************
// ECSqlUpdatePreparedStatement
//******************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan             04/14
//---------------------------------------------------------------------------------------
ECSqlUpdatePreparedStatement::ECSqlUpdatePreparedStatement (ECDbCR ecdb, ECSqlStatusContext& statusContext)
: ECSqlNonSelectPreparedStatement (ECSqlType::Update, ecdb, statusContext)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan             04/14
//---------------------------------------------------------------------------------------
ECSqlStepStatus ECSqlUpdatePreparedStatement::Step ()
    {
    OnBeforeStep();

    ECSqlStepStatus status = ECSqlStepStatus::Done;
    if (!IsNoopInSqlite ())
        {
        if (GetStepTasks ().ExecuteBeforeStepTaskList () == ECSqlStepStatus::Error)
            return ECSqlStepStatus::Error;

        if (!IsNothingToUpdate())
            {
            status = DoStep();
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan             04/14
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlNonSelectPreparedStatement::_Reset ()
    {
    if (GetStepTasks ().HasSelector ())
        GetStepTasks ().ResetSelector ();

    return DoReset ();
    }


//******************************************************************************************
// ECSqlDeletePreparedStatement
//******************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan             04/14
//---------------------------------------------------------------------------------------
ECSqlDeletePreparedStatement::ECSqlDeletePreparedStatement (ECDbCR ecdb, ECSqlStatusContext& statusContext)
: ECSqlNonSelectPreparedStatement (ECSqlType::Delete, ecdb, statusContext)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan             04/14
//---------------------------------------------------------------------------------------
ECSqlStepStatus ECSqlDeletePreparedStatement::Step ()
    {
    OnBeforeStep();

    ECSqlStepStatus status = ECSqlStepStatus::Done;
    if (!IsNoopInSqlite ())
        {
        if (GetStepTasks ().ExecuteBeforeStepTaskList () == ECSqlStepStatus::Error)
            return ECSqlStepStatus::Error;

        status = DoStep ();
        }


    return status;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
