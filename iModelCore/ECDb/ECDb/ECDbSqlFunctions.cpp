/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSqlFunctions.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <Bentley/Base64Utilities.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
//static
BlobToBase64SqlFunction* BlobToBase64SqlFunction::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
//static
BlobToBase64SqlFunction& BlobToBase64SqlFunction::GetSingleton()
    {
    if (s_singleton == nullptr)
        s_singleton = new BlobToBase64SqlFunction();

    return *s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
void BlobToBase64SqlFunction::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& blobArg = args[0];
    if (blobArg.IsNull())
        {
        ctx.SetResultNull();
        return;
        }

    Byte const* blob = static_cast<Byte const*> (blobArg.GetValueBlob());
    const int byteCount = blobArg.GetValueBytes();
    BeAssert(byteCount >= 0);
    Utf8String str;
    Base64Utilities::Encode(str, blob, (size_t) byteCount);
    ctx.SetResultText(str.c_str(), (int) str.length(), Context::CopyData::Yes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
//static
Base64ToBlobSqlFunction* Base64ToBlobSqlFunction::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
//static
Base64ToBlobSqlFunction& Base64ToBlobSqlFunction::GetSingleton()
    {
    if (s_singleton == nullptr)
        s_singleton = new Base64ToBlobSqlFunction();

    return *s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
void Base64ToBlobSqlFunction::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& base64Arg = args[0];

    if (base64Arg.IsNull())
        {
        ctx.SetResultNull();
        return;
        }

    Utf8CP base64Str = base64Arg.GetValueText();
    if (!Base64Utilities::MatchesAlphabet(base64Str))
        {
        Utf8String error;
        error.Sprintf("Invalid argument for SQL function %s: '%s' is not a valid base64 string.", GetName(), base64Str);
        ctx.SetResultError(error.c_str());
        return;
        }

    int base64StrLen = base64Arg.GetValueBytes();
    if (base64StrLen < 0)
        base64StrLen = (int) strlen(base64Str);
    
    ByteStream blob;
    Base64Utilities::Decode(blob, base64Str, (size_t) base64StrLen);
    ctx.SetResultBlob(blob.data(), (int) blob.size(), DbFunction::Context::CopyData::Yes);
    }

//************************************************************************************
// ChangedValueFunction
//************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
std::map<Utf8CP, std::function<void(ChangedValueFunction::Context&, ECSqlStatement&)>, CompareIUtf8Ascii> ChangedValueFunction::s_setValueMap;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
ChangedValueFunction::ChangedValueFunction(ECDbCR ecdb) : ScalarFunction(SQLFUNC_ChangedValue, -1, DbValueType::BlobVal), m_ecdb(ecdb), m_stmtCache(10)
    {
    if (s_setValueMap.empty())
        {
        s_setValueMap.insert(std::make_pair("null", [] (Context& ctx, ECSqlStatement& stmt) { ctx.SetResultNull(); }));
        s_setValueMap.insert(std::make_pair("integer", [] (Context& ctx, ECSqlStatement& stmt) { ctx.SetResultInt64(stmt.GetValueInt64(0)); }));
        s_setValueMap.insert(std::make_pair("real", [] (Context& ctx, ECSqlStatement& stmt) { ctx.SetResultDouble(stmt.GetValueDouble(0)); }));
        s_setValueMap.insert(std::make_pair("text", [] (Context& ctx, ECSqlStatement& stmt)
            {
            int len = (int) strlen(stmt.GetValueText(0));
            ctx.SetResultText(stmt.GetValueText(0), len, Context::CopyData::Yes);
            }));
        s_setValueMap.insert(std::make_pair("blob", [] (Context& ctx, ECSqlStatement& stmt)
            {
            int len;
            const void* blob = stmt.GetValueBlob(0, &len);
            ctx.SetResultBlob(blob, len, Context::CopyData::Yes);
            }));
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
void ChangedValueFunction::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    if (nArgs != 4 && nArgs != 5)
        {
        ctx.SetResultError("Function " SQLFUNC_ChangedValue " expects 4 or 5 arguments.");
        return;
        }

    const int kInstanceId = 0;      // [Required]
    const int kAccessString = 1;    // [Required]
    const int kOperation = 2;       // [Required]
    const int kFallBackValue = 3;   // [Required]
    const int kStage = 4;           // [Optional]

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

    DbValue const& operationValue = args[2];
    if (operationValue.GetValueType() != DbValueType::IntegerVal)
        {
        ctx.SetResultError("Argument 3 of function " SQLFUNC_ChangedValue " is expected to be the operation and must be of integer type and cannot be null");
        return;
        }

    int operationVal = operationValue.GetValueInt();
    if (operationVal != Enum::ToInt(Operation::Insert) && operationVal != Enum::ToInt(Operation::Update) &&
        operationVal != Enum::ToInt(Operation::Delete))
        {
        Utf8String msg;
        msg.Sprintf("Argument 3 of function " SQLFUNC_ChangedValue " has an invalid operation value (%d)", operationVal);
        ctx.SetResultError(msg.c_str());
        return;
        }

    const Operation operation = Enum::FromInt<Operation>(operationVal);
    DbValue const& fallbackValue = args[3];

    Stage stage;
    switch (operation)
        {
            case Operation::Insert:
            {
            if (nArgs == 5)
                {
                Utf8String msg;
                msg.Sprintf("When passing %d (Inserted) as Operation the fifth parameter must not be specified.", operationVal);
                ctx.SetResultError(msg.c_str());
                return;
                }

            stage = Stage::New;
            break;
            }
            case Operation::Delete:
            {
            if (nArgs == 5)
                {
                Utf8String msg;
                msg.Sprintf("When passing %d (Deleted) as Operation the fifth parameter must not be specified.", operationVal);
                ctx.SetResultError(msg.c_str());
                return;
                }
            stage = Stage::Old;
            break;
            }
            case Operation::Update:
            {
            DbValue const* stageValue = nArgs == 5 ? &args[4] : nullptr;

            if (stageValue == nullptr || stageValue->GetValueType() != DbValueType::IntegerVal)
                {
                ctx.SetResultError("Argument 5 of function " SQLFUNC_ChangedValue " is expected to be specified when 'Update' was specified as operation (4th argument).");
                return;
                }

            const int stageVal = stageValue->GetValueInt();
            if (stageVal != Enum::ToInt(Stage::Old) && stageVal != Enum::ToInt(Stage::New))
                {
                Utf8String msg;
                msg.Sprintf("Argument 5 of function " SQLFUNC_ChangedValue " has an invalid value for Stage (%d)", stageVal);
                ctx.SetResultError(msg.c_str());
                return;
                }

            stage = Enum::FromInt<Stage>(stageVal);
            break;
            }

            default:
                BeAssert(false && "Should have been caught already");
                return;
        }

    Utf8CP ecsql = nullptr;
    if (stage == Stage::Old)
        ecsql = "SELECT RawOldValue, CAST(TYPEOF(RawOldValue) AS TEXT) FROM " ECDBCHANGE_CLASS_PropertyValueChange " WHERE InstanceChange.Id=? AND AccessString=?";
    else
        ecsql = "SELECT RawNewValue, CAST(TYPEOF(RawNewValue) AS TEXT) FROM " ECDBCHANGE_CLASS_PropertyValueChange " WHERE InstanceChange.Id=? AND AccessString=?";

    CachedECSqlStatementPtr stmt = m_stmtCache.GetPreparedStatement(m_ecdb, ecsql);
    if (stmt == nullptr)
        {
        ctx.SetResultError("Failed to prepare ECSQL statement in SQL function ChangedValue().");
        return;
        }

    stmt->BindId(1, instanceChangeId);
    stmt->BindText(2, accessString, IECSqlBinder::MakeCopy::No);

    if (stmt->Step() == BE_SQLITE_ROW)
        SetValue(ctx, *stmt);
    else
        ctx.SetResultValue(fallbackValue);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
void ChangedValueFunction::SetValue(Context& ctx, ECSqlStatement& stmt)
    {
    if (stmt.IsValueNull(0))
        {
        ctx.SetResultNull();
        return;
        }

    Utf8CP typeOf = stmt.GetValueText(1);
    s_setValueMap[typeOf](ctx, stmt);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE