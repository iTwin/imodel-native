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
ChangedValueFunction::ChangedValueFunction(ECDbR ecdb) : ScalarFunction(SQLFUNC_ChangedValue, -1, DbValueType::BlobVal), m_ecdb(ecdb), m_stmtCache(10)
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
        ctx.SetResultError("Function ChangedValue expects 4 or 5 parameters.");
        return;
        }

    const int kInstanceId = 0;      // [Required]
    const int kAccessString = 1;    // [Required]
    const int kOperation = 2;       // [Required]
    const int kFallBackValue = 3;   // [Required]
    const int kStage = 4;           // [Optional]

    //Decode and verify parameters
    if (args[kInstanceId].GetValueType() != DbValueType::IntegerVal)
        {
        ctx.SetResultError("Parameter 1 is expected to be of integer type and cannot be null");
        return;
        }

    if (args[kAccessString].GetValueType() != DbValueType::TextVal)
        {
        ctx.SetResultError("Parameter 2 is expected to be of text type and cannot be null");
        return;
        }

    if (args[kOperation].GetValueType() != DbValueType::IntegerVal)
        {
        ctx.SetResultError("Parameter 3 is expected to be of integer type and cannot be null");
        return;
        }

    const ECInstanceId csInstanceId = args[kInstanceId].GetValueId<ECInstanceId>();
    Utf8CP accessString = args[kAccessString].GetValueText();
    int operationVal = args[kOperation].GetValueInt();
    if (operationVal != Enum::ToInt(Operation::Inserted) && operationVal != Enum::ToInt(Operation::Updated) &&
        operationVal != Enum::ToInt(Operation::Deleted))
        {
        Utf8String msg;
        msg.Sprintf("Invalid value for argument Operation", operationVal);
        ctx.SetResultError(msg.c_str());
        return;
        }

    const Operation operation = Enum::FromInt<Operation>(operationVal);
    DbValue const& fallbackValue = args[kFallBackValue];

    Stage stage;
    switch (operation)
        {
            case Operation::Inserted:
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
            case Operation::Deleted:
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
            case Operation::Updated:
            {
            if (nArgs != 5 || args[kStage].GetValueType() != DbValueType::IntegerVal)
                {
                Utf8String msg;
                msg.Sprintf("When passing %d (Updated) as Operation the fifth parameter must be specified and it must be of type Integer.", operationVal);
                ctx.SetResultError(msg.c_str());
                return;
                }

            const int stageVal = args[kStage].GetValueInt();
            if (stageVal != Enum::ToInt(Stage::Old) && stageVal != Enum::ToInt(Stage::New))
                {
                Utf8String msg;
                msg.Sprintf("Invalid value for argument Stage", stageVal);
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
        ecsql = "SELECT RawOldValue, CAST(TYPEOF(RawOldValue) AS TEXT) FROM change.PropertyValue WHERE Instance.Id=? AND AccessString=?";
    else
        ecsql = "SELECT RawNewValue, CAST(TYPEOF(RawNewValue) AS TEXT) FROM change.PropertyValue WHERE Instance.Id=? AND AccessString=?";

    CachedECSqlStatementPtr stmt = m_stmtCache.GetPreparedStatement(m_ecdb, ecsql);
    if (stmt == nullptr)
        {
        ctx.SetResultError("Failed to prepare ECSQL statement necessary for ChangedValue()");
        return;
        }

    stmt->BindId(1, csInstanceId);
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