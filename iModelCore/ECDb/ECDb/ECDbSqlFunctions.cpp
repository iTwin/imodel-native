/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSqlFunctions.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//************************************************************************************
// ChangedValueStateToOpCodeSqlFunction
//************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                       11/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ChangedValueStateToOpCodeSqlFunction* ChangedValueStateToOpCodeSqlFunction::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                       11/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ChangedValueStateToOpCodeSqlFunction& ChangedValueStateToOpCodeSqlFunction::GetSingleton()
    {
    if (s_singleton == nullptr)
        s_singleton = new ChangedValueStateToOpCodeSqlFunction();

    return *s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                       11/17
//+---------------+---------------+---------------+---------------+---------------+------
void ChangedValueStateToOpCodeSqlFunction::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& stateValue = args[0];
    if (stateValue.GetValueType() != DbValueType::IntegerVal && stateValue.GetValueType() != DbValueType::TextVal)
        {
        ctx.SetResultError("Argument 1 of function " SQLFUNC_ChangedValueStateToOpCode " is expected to be the ChangedValueState and must be of integer or text type and cannot be null");
        return;
        }

    const bool isIntegerVal = stateValue.GetValueType() == DbValueType::IntegerVal;
    Nullable<ChangedValueState> state;
    if (isIntegerVal)
        state = ChangeSummaryHelper::ToChangedValueState(stateValue.GetValueInt());
    else
        state = ChangeSummaryHelper::ToChangedValueState(stateValue.GetValueText());

    Nullable<ChangeOpCode> opCode;
    if (state.IsNull() || (opCode = ChangeSummaryHelper::DetermineOpCodeFromChangedValueState(state.Value())).IsNull())
        {
        Utf8String msg;
        if (isIntegerVal)
            msg.Sprintf("Argument 1 of function " SQLFUNC_ChangedValueStateToOpCode " has an invalid ChangedValueState value (%d)", stateValue.GetValueInt());
        else
            msg.Sprintf("Argument 1 of function " SQLFUNC_ChangedValueStateToOpCode " has an invalid ChangedValueState value (%s)", stateValue.GetValueText());

        ctx.SetResultError(msg.c_str());
        return;
        }
        
    ctx.SetResultInt(Enum::ToInt(opCode.Value()));
    }

//************************************************************************************
// ChangedValueSqlFunction
//************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
void ChangedValueSqlFunction::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    //Decode and verify parameters
    DbValue const& instanceChangeIdValue = args[0];
    if (instanceChangeIdValue.GetValueType() != DbValueType::IntegerVal)
        {
        ctx.SetResultError("Argument 1 of function " SQLFUNC_ChangedValue " is expected to be the InstanceChange ECInstanceId and must be of integer type and cannot be null");
        return;
        }

    const ECInstanceId instanceChangeId = instanceChangeIdValue.GetValueId<ECInstanceId>();

    DbValue const& accessStringValue = args[1];
    if (accessStringValue.GetValueType() != DbValueType::TextVal)
        {
        ctx.SetResultError("Argument 2 of function " SQLFUNC_ChangedValue " is expected to be the property access string and must be of text type and cannot be null");
        return;
        }

    Utf8CP accessString = accessStringValue.GetValueText();

    DbValue const& changedValueStateValue = args[2];
    if (changedValueStateValue.GetValueType() != DbValueType::IntegerVal && changedValueStateValue.GetValueType() != DbValueType::TextVal)
        {
        ctx.SetResultError("Argument 3 of function " SQLFUNC_ChangedValue " is expected to be the ChangedValueState and must be of integer or text type and cannot be null");
        return;
        }

    const bool stateIsIntegerVal = changedValueStateValue.GetValueType() == DbValueType::IntegerVal;
    Nullable<ChangedValueState> state;
    if (stateIsIntegerVal)
        state = ChangeSummaryHelper::ToChangedValueState(changedValueStateValue.GetValueInt());
    else
        state = ChangeSummaryHelper::ToChangedValueState(changedValueStateValue.GetValueText());

    if (state.IsNull())
        {
        Utf8String msg;
        if (stateIsIntegerVal)
            msg.Sprintf("Argument 3 of function " SQLFUNC_ChangedValue " has an invalid ChangedValueState value (%d)", changedValueStateValue.GetValueInt());
        else
            msg.Sprintf("Argument 3 of function " SQLFUNC_ChangedValue " has an invalid ChangedValueState value (%s)", changedValueStateValue.GetValueText());

        ctx.SetResultError(msg.c_str());
        return;
        }

    DbValue const& fallbackValue = args[3];


    Utf8CP ecsql = nullptr;
    if (state == ChangedValueState::BeforeUpdate || state == ChangedValueState::BeforeDelete)
        ecsql = "SELECT RawOldValue, CAST(TYPEOF(RawOldValue) AS TEXT) FROM " ECSCHEMA_ALIAS_ECDbChangeSummaries "." ECDBCHANGE_CLASS_PropertyValueChange " WHERE InstanceChange.Id=? AND AccessString=?";
    else
        ecsql = "SELECT RawNewValue, CAST(TYPEOF(RawNewValue) AS TEXT) FROM " ECSCHEMA_ALIAS_ECDbChangeSummaries "." ECDBCHANGE_CLASS_PropertyValueChange " WHERE InstanceChange.Id=? AND AccessString=?";

    CachedECSqlStatementPtr stmt = m_statementCache.GetPreparedStatement(m_ecdb, ecsql);
    if (stmt == nullptr)
        {
        Utf8String msg;
        msg.Sprintf("SQL function " SQLFUNC_ChangedValue " failed: could not prepare ECSQL '%s'.", ecsql);
        ctx.SetResultError(msg.c_str());
        return;
        }

    stmt->BindId(1, instanceChangeId);
    stmt->BindText(2, accessString, IECSqlBinder::MakeCopy::No);

    if (stmt->Step() != BE_SQLITE_ROW)
        {
        ctx.SetResultValue(fallbackValue);
        return;
        }

    if (stmt->IsValueNull(0))
        {
        ctx.SetResultNull();
        return;
        }

    Utf8CP valType = stmt->GetValueText(1);

    if (BeStringUtilities::StricmpAscii("integer", valType) == 0)
        {
        ctx.SetResultInt64(stmt->GetValueInt64(0));
        return;
        }

    if (BeStringUtilities::StricmpAscii("real", valType) == 0)
        {
        ctx.SetResultDouble(stmt->GetValueDouble(0));
        return;
        }

    if (BeStringUtilities::StricmpAscii("text", valType) == 0)
        {
        Utf8CP strVal = stmt->GetValueText(0);
        const int len = (int) strlen(strVal);
        ctx.SetResultText(strVal, len, Context::CopyData::Yes);
        return;
        }

    if (BeStringUtilities::StricmpAscii("blob", valType) == 0)
        {
        int blobSize = -1;
        void const* blob = stmt->GetValueBlob(0, &blobSize);
        ctx.SetResultBlob(blob, blobSize, Context::CopyData::Yes);
        return;
        }

    Utf8String msg;
    msg.Sprintf("SQL function " SQLFUNC_ChangedValue " failed: executing the ECSQL '%s' returned an unsupported data type (%s).", stmt->GetECSql(), valType);
    ctx.SetResultError(msg.c_str());
    }

END_BENTLEY_SQLITE_EC_NAMESPACE