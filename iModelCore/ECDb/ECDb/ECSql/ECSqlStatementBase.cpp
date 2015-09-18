/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementBase.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlStatementBase.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
void ECSqlStatementBase::Finalize ()
    {
    m_preparedStatement = nullptr;
    BeAssert (!IsPrepared ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        01/14
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatementBase::Prepare (ECDbCR ecdb, Utf8CP ecsql)
    {
    return _Prepare (ecdb, ecsql);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatementBase::_Prepare (ECDbCR ecdb, Utf8CP ecsql)
    {
    if (IsPrepared())
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECSQL statement has already been prepared.");
        return ECSqlStatus::UserError;
        }

    if (Utf8String::IsNullOrEmpty(ecsql))
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECSQL string is empty.");
        return ECSqlStatus::InvalidECSql;
        }

    ECSqlPrepareContext prepareContext = _InitializePrepare (ecdb, ecsql);

    //Step 1: parse the ECSQL
    ECSqlParseTreePtr ecsqlParseTree = nullptr;
    ECSqlParser parser;
    if (SUCCESS != parser.Parse(ecsqlParseTree, ecdb, ecsql, prepareContext.GetClassMapViewMode()))
        {
        Finalize ();
        return ECSqlStatus::InvalidECSql;
        }

    //Step 2: translate into SQLite SQL and prepare SQLite statement
    ECSqlPreparedStatement& preparedStatement = CreatePreparedStatement (ecdb, *ecsqlParseTree);
    ECSqlStatus stat = preparedStatement.Prepare (prepareContext, *ecsqlParseTree, ecsql);
    if (stat != ECSqlStatus::Success)
        Finalize ();
    
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
bool ECSqlStatementBase::IsPrepared () const
    {
    return GetPreparedStatementP () != nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
IECSqlBinder& ECSqlStatementBase::GetBinder (int parameterIndex) const 
    {
    ECSqlStatus stat = FailIfNotPrepared ("Cannot call binding API on an unprepared ECSqlStatement.");
    if (stat != ECSqlStatus::Success)
        return NoopECSqlBinderFactory::GetBinder (stat);

    //Reports errors (not prepared yet, index out of bounds) and uses no-op binder in case of error
    return GetPreparedStatementP ()->GetBinder (parameterIndex);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
int ECSqlStatementBase::GetParameterIndex (Utf8CP parameterName) const 
    {
    ECSqlStatus stat = FailIfNotPrepared ("Cannot call binding API on an unprepared ECSqlStatement.");
    if (stat != ECSqlStatus::Success)
        return -1;

    //Reports errors (not prepared yet, index out of bounds) and uses no-op binder in case of error
    return GetPreparedStatementP ()->GetParameterIndex (parameterName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatementBase::ClearBindings ()
    {
    ECSqlStatus stat = FailIfNotPrepared ("Cannot call ClearBindings on an unprepared ECSqlStatement.");
    if (stat != ECSqlStatus::Success)
        return stat;

    return GetPreparedStatementP ()->ClearBindings ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStepStatus ECSqlStatementBase::Step ()
    {
    if (FailIfNotPrepared ("Cannot call Step on an unprepared ECSQL statement.") != ECSqlStatus::Success)
        return ECSqlStepStatus::Error;
    
    //for performance reasons ECSqlPreparedStatement::Step is not polymorphic (anymore). Cost
    //of virtual dispatch was eliminated by taking cost of caller having to downcast to each subclass type
    //and call non-virtual Step.
    const auto type = GetPreparedStatementP ()->GetType ();
    switch (type)
        {
            case ECSqlType::Select:
                return GetPreparedStatementP<ECSqlSelectPreparedStatement> ()->Step ();

            case ECSqlType::Insert:
                {
                ECInstanceKey key;
                return GetPreparedStatementP<ECSqlInsertPreparedStatement> ()->Step (key);
                }

            case ECSqlType::Update:
                return GetPreparedStatementP<ECSqlUpdatePreparedStatement> ()->Step ();

            case ECSqlType::Delete:
                return GetPreparedStatementP<ECSqlDeletePreparedStatement> ()->Step ();

            default:
                BeAssert (false && "Unhandled ECSqlType in ECSqlStatement::Step.");
                return ECSqlStepStatus::Error;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        11/13
//---------------------------------------------------------------------------------------
ECSqlStepStatus ECSqlStatementBase::Step (ECInstanceKey& ecInstanceKey)
    {
    if (FailIfWrongType (ECSqlType::Insert, "Only call Step(ECInstanceKey&) on an ECSQL INSERT statement.") != ECSqlStatus::Success)
        return ECSqlStepStatus::Error;

    return GetPreparedStatementP<ECSqlInsertPreparedStatement> ()->Step (ecInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatementBase::Reset ()
    {
    ECSqlStatus stat = FailIfNotPrepared ("Cannot call Reset on an unprepared ECSqlStatement.");
    if (stat != ECSqlStatus::Success)
        return stat;

    return GetPreparedStatementP ()->Reset ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
int ECSqlStatementBase::GetColumnCount () const
    {
    if (FailIfWrongType (ECSqlType::Select, "Cannot call query result API on an unprepared or non-SELECT ECSqlStatement.") != ECSqlStatus::Success)
        return -1;

    return GetPreparedStatementP<ECSqlSelectPreparedStatement> ()->GetColumnCount ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/14
//---------------------------------------------------------------------------------------
IECSqlValue const& ECSqlStatementBase::GetValue (int columnIndex) const
    {
    if (FailIfWrongType (ECSqlType::Select, "Cannot call query result API on an unprepared or non-SELECT ECSqlStatement.") != ECSqlStatus::Success)
        return NoopECSqlValue::GetSingleton ();

    //Reports errors (not prepared yet, index out of bounds) and uses no-op value in case of error
    return GetPreparedStatementP<ECSqlSelectPreparedStatement> ()->GetValue (columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        06/14
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatementBase::GetECSql () const
    {
    ECSqlStatus stat = FailIfNotPrepared ("Cannot call GetECSql on an unprepared ECSqlStatement.");
    if (stat != ECSqlStatus::Success)
        return nullptr;

    return GetPreparedStatementP ()->GetECSql ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatementBase::GetNativeSql() const
    {
    ECSqlStatus stat = FailIfNotPrepared ("Cannot call GetNativeSql on an unprepared ECSqlStatement.");
    if (stat != ECSqlStatus::Success)
        return nullptr;

    return GetPreparedStatementP ()->GetNativeSql ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        02/14
//---------------------------------------------------------------------------------------
ECDbCP ECSqlStatementBase::GetECDb () const
    {
    ECSqlStatus stat = FailIfNotPrepared ("Cannot call GetECDb on an unprepared ECSqlStatement.");
    if (stat != ECSqlStatus::Success)
        return nullptr;

    return &GetPreparedStatementP ()->GetECDb ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatementBase::FailIfNotPrepared (Utf8CP errorMessage) const
    {
    if (!IsPrepared())
        {
        LOG.error(errorMessage);
        return ECSqlStatus::UserError;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatementBase::FailIfWrongType (ECSqlType expectedType, Utf8CP errorMessage) const
    {
    ECSqlStatus stat = FailIfNotPrepared (errorMessage);
    if (stat != ECSqlStatus::Success)
        return stat;

    if (GetPreparedStatementP()->GetType() != expectedType)
        {
        GetECDb()->GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, errorMessage);
        return ECSqlStatus::UserError;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlPreparedStatement& ECSqlStatementBase::CreatePreparedStatement (ECDbCR ecdb, ECSqlParseTreeCR parseTree)
    {
    switch (parseTree.GetType ())
        {
        case Exp::Type::Select:
            m_preparedStatement = unique_ptr<ECSqlPreparedStatement> (new ECSqlSelectPreparedStatement (ecdb));
            break;

        case Exp::Type::Insert:
            m_preparedStatement = unique_ptr<ECSqlPreparedStatement> (new ECSqlInsertPreparedStatement (ecdb));
            break;

        case Exp::Type::Update:
            m_preparedStatement = unique_ptr<ECSqlPreparedStatement> (new ECSqlUpdatePreparedStatement (ecdb));
            break;

        case Exp::Type::Delete:
            m_preparedStatement = unique_ptr<ECSqlPreparedStatement> (new ECSqlDeletePreparedStatement (ecdb));
            break;

        default:
            BeAssert (false && "ECSqlParseTree is expected to only be of type Select, Insert, Update, Delete");
            break;
        }

    return *m_preparedStatement;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
