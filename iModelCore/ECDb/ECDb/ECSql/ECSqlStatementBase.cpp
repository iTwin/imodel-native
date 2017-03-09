/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementBase.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlStatementBase.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        01/14
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatementBase::Prepare(ECDbCR ecdb, Utf8CP ecsql, ECCrudWriteToken const* token)
    {
    if (IsPrepared())
        {
        LOG.error("ECSQL statement has already been prepared.");
        return ECSqlStatus::Error;
        }

    if (Utf8String::IsNullOrEmpty(ecsql))
        {
        LOG.error("ECSQL string is empty.");
        return ECSqlStatus::InvalidECSql;
        }

    ECSqlPrepareContext prepareContext = _InitializePrepare(ecdb, token);
    return _Prepare(prepareContext, ecsql);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatementBase::_Prepare(ECSqlPrepareContext& ctx, Utf8CP ecsql)
    {
    //Step 1: parse the ECSQL
    ECSqlParser parser;
    std::unique_ptr<Exp> exp = parser.Parse(ctx.GetECDb(), ecsql);
    if (exp == nullptr)
        {
        Finalize();
        return ECSqlStatus::InvalidECSql;
        }

    //establish joinTable context if any
    ECSqlPrepareContext::JoinedTableInfo const* joinedTableInfo = ctx.TrySetupJoinedTableInfo(*exp, ecsql);
    if (joinedTableInfo != nullptr)
        {
        if (joinedTableInfo->HasJoinedTableECSql()) //in case joinTable update it is possible that current could be null
            ecsql = joinedTableInfo->GetJoinedTableECSql();
        else
            ecsql = joinedTableInfo->GetParentOfJoinedTableECSql();

        //reparse
        exp = parser.Parse(ctx.GetECDb(), ecsql);
        if (exp == nullptr)
            {
            Finalize();
            return ECSqlStatus::InvalidECSql;
            }
        }


    //Step 2: translate into SQLite SQL and prepare SQLite statement
    ECSqlPreparedStatement_Old& preparedStatement = CreatePreparedStatement(ctx.GetECDb(), *exp);

    ECDbPolicy policy = ECDbPolicyManager::GetPolicy(ECCrudPermissionPolicyAssertion(ctx.GetECDb(), preparedStatement.GetType() != ECSqlType::Select, ctx.GetWriteToken()));
    if (!policy.IsSupported())
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(policy.GetNotSupportedMessage().c_str());
        Finalize();
        return ECSqlStatus::Error;
        }

    ECSqlStatus stat = preparedStatement.Prepare(ctx, *exp, ecsql);
    if (!stat.IsSuccess())
        Finalize();

    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
IECSqlBinder& ECSqlStatementBase::GetBinder(int parameterIndex) const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call binding API on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return NoopECSqlBinder::Get();

    //Reports errors (not prepared yet, index out of bounds) and uses no-op binder in case of error
    return GetPreparedStatementP()->GetBinder(parameterIndex);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
int ECSqlStatementBase::GetParameterIndex(Utf8CP parameterName) const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call binding API on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return -1;

    //Reports errors (not prepared yet, index out of bounds) and uses no-op binder in case of error
    return GetPreparedStatementP()->GetParameterIndex(parameterName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatementBase::ClearBindings()
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call ClearBindings on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return stat;

    return GetPreparedStatementP()->ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
DbResult ECSqlStatementBase::Step()
    {
    if (!FailIfNotPrepared("Cannot call Step on an unprepared ECSQL statement.").IsSuccess())
        return BE_SQLITE_ERROR;

    //for performance reasons ECSqlPreparedStatement::Step is not polymorphic (anymore). Cost
    //of virtual dispatch was eliminated by taking cost of caller having to downcast to each subclass type
    //and call non-virtual Step.
    const ECSqlType ecsqlType = GetPreparedStatementP()->GetType();
    switch (ecsqlType)
        {
            case ECSqlType::Select:
                return GetPreparedStatementP<ECSqlSelectPreparedStatement_Old>()->Step();

            case ECSqlType::Insert:
            {
            ECInstanceKey key;
            return GetPreparedStatementP<ECSqlInsertPreparedStatement_Old>()->Step(key);
            }

            case ECSqlType::Update:
                return GetPreparedStatementP<ECSqlUpdatePreparedStatement_Old>()->Step();

            case ECSqlType::Delete:
                return GetPreparedStatementP<ECSqlDeletePreparedStatement_Old>()->Step();

            default:
                BeAssert(false && "Unhandled ECSqlType in ECSqlStatement::Step.");
                return BE_SQLITE_ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        11/13
//---------------------------------------------------------------------------------------
DbResult ECSqlStatementBase::Step(ECInstanceKey& ecInstanceKey)
    {
    if (!FailIfWrongType(ECSqlType::Insert, "Only call Step(ECInstanceKey&) on an ECSQL INSERT statement.").IsSuccess())
        return BE_SQLITE_ERROR;

    return GetPreparedStatementP<ECSqlInsertPreparedStatement_Old>()->Step(ecInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatementBase::Reset()
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call Reset on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return stat;

    return GetPreparedStatementP()->Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
int ECSqlStatementBase::GetColumnCount() const
    {
    if (FailIfWrongType(ECSqlType::Select, "Cannot call query result API on an unprepared or non-SELECT ECSqlStatement.") != ECSqlStatus::Success)
        return -1;

    return GetPreparedStatementP<ECSqlSelectPreparedStatement_Old>()->GetColumnCount();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        03/14
//---------------------------------------------------------------------------------------
IECSqlValue const& ECSqlStatementBase::GetValue(int columnIndex) const
    {
    if (FailIfWrongType(ECSqlType::Select, "Cannot call query result API on an unprepared or non-SELECT ECSqlStatement.") != ECSqlStatus::Success)
        return NoopECSqlValue::GetSingleton();

    //Reports errors (not prepared yet, index out of bounds) and uses no-op value in case of error
    return GetPreparedStatementP<ECSqlSelectPreparedStatement_Old>()->GetValue(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        06/14
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatementBase::GetECSql() const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call GetECSql on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return nullptr;

    return GetPreparedStatementP()->GetECSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        10/13
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatementBase::GetNativeSql() const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call GetNativeSql on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return nullptr;

    return GetPreparedStatementP()->GetNativeSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        02/14
//---------------------------------------------------------------------------------------
ECDb const* ECSqlStatementBase::GetECDb() const
    {
    ECSqlStatus stat = FailIfNotPrepared("Cannot call GetECDb on an unprepared ECSqlStatement.");
    if (!stat.IsSuccess())
        return nullptr;

    return &GetPreparedStatementP()->GetECDb();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatementBase::FailIfNotPrepared(Utf8CP errorMessage) const
    {
    if (!IsPrepared())
        {
        LOG.error(errorMessage);
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatementBase::FailIfWrongType(ECSqlType expectedType, Utf8CP errorMessage) const
    {
    ECSqlStatus stat = FailIfNotPrepared(errorMessage);
    if (!stat.IsSuccess())
        return stat;

    if (GetPreparedStatementP()->GetType() != expectedType)
        {
        GetECDb()->GetECDbImplR().GetIssueReporter().Report(errorMessage);
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle        12/13
//---------------------------------------------------------------------------------------
ECSqlPreparedStatement_Old& ECSqlStatementBase::CreatePreparedStatement(ECDbCR ecdb, Exp const& exp)
    {
    switch (exp.GetType())
        {
            case Exp::Type::Select:
                m_preparedStatement = std::unique_ptr<ECSqlPreparedStatement_Old>(new ECSqlSelectPreparedStatement_Old(ecdb));
                break;

            case Exp::Type::Insert:
                m_preparedStatement = std::unique_ptr<ECSqlPreparedStatement_Old>(new ECSqlInsertPreparedStatement_Old(ecdb));
                break;

            case Exp::Type::Update:
                m_preparedStatement = std::unique_ptr<ECSqlPreparedStatement_Old>(new ECSqlUpdatePreparedStatement_Old(ecdb));
                break;

            case Exp::Type::Delete:
                m_preparedStatement = std::unique_ptr<ECSqlPreparedStatement_Old>(new ECSqlDeletePreparedStatement_Old(ecdb));
                break;

            default:
                BeAssert(false && "ECSqlParseTree is expected to only be of type Select, Insert, Update, Delete");
                break;
        }

    return *m_preparedStatement;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
