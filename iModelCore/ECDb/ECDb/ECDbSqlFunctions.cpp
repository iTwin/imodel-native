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


std::map<Utf8CP, std::function<void(ChangedValue::Context&, ECSqlStatement&)>, CompareIUtf8Ascii> ChangedValue::s_setValueMap;
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/17
//+---------------+---------------+---------------+---------------+---------------+------
void ChangedValue::SetValue(Context& ctx, ECSqlStatement& stmt)
    {
    if (stmt.IsValueNull(0))
        {
        ctx.SetResultNull();
        return;
        }

    Utf8CP typeOf = stmt.GetValueText(1);
    s_setValueMap[typeOf](ctx, stmt);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/17
//+---------------+---------------+---------------+---------------+---------------+------
void ChangedValue::_ComputeScalar(Context& ctx, int nArgs, DbValue* args) 
    {
    if (nArgs < 4)
        {
        ctx.SetResultError("Not enough parameters");
        return;
        }

    if (nArgs > 5)
        {
        ctx.SetResultError("Too many parameters");
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
        ctx.SetResultError("Parameter one expect to be of integer type and cannot be null");
        return;
        }

    if (args[kAccessString].GetValueType() != DbValueType::TextVal)
        {
        ctx.SetResultError("Parameter two expect to be of text type and cannot be null");
        return;
        }

    if (args[kOperation].GetValueType() != DbValueType::IntegerVal)
        {
        ctx.SetResultError("Parameter three expect to be of integer type and cannot be null");
        return;
        }


    if (nArgs == 5 && args[kStage].GetValueType() != DbValueType::IntegerVal)
        {
        ctx.SetResultError("Parameter three expect to be of integer type and cannot be null");
        return;
        }

    const ECInstanceId csInstanceId = args[kInstanceId].GetValueId<ECInstanceId>();
    Utf8CP accessString = args[kAccessString].GetValueText();
    const ChangeSummaryV2::Operation operation = (ChangeSummaryV2::Operation) args[kOperation].GetValueInt();
    const DbValue& fallbackValue = args[kFallBackValue];

    ChangeSummaryV2::Stage stage;
    if (operation == ChangeSummaryV2::Operation::Inserted)
        stage = ChangeSummaryV2::Stage::New;
    else if (operation == ChangeSummaryV2::Operation::Deleted)
        stage = ChangeSummaryV2::Stage::Old;
    else if (operation == ChangeSummaryV2::Operation::Updated)
        stage = ChangeSummaryV2::Stage::Unset;
    else
        {
        ctx.SetResultError("'Opeartion' parameter has a invalid value.");
        return;
        }

    if (nArgs == 5)
        {
        stage = (ChangeSummaryV2::Stage) args[kStage].GetValueInt();        
        if (operation == ChangeSummaryV2::Operation::Inserted && stage != ChangeSummaryV2::Stage::New)
            {
            ctx.SetResultError("'Stage' arumgent for 'Insert' operation must must be set to 'New'");
            return;
            }
        else if (operation == ChangeSummaryV2::Operation::Deleted && stage != ChangeSummaryV2::Stage::Old)
            {
            ctx.SetResultError("'Stage' arumgent for 'Delete' operation must must be set to 'Old'");
            return;
            }            
        }

    CachedECSqlStatementPtr stmt;
    if (stage== ChangeSummaryV2::Stage::Old)
        stmt = m_stmtCache.GetPreparedStatement(m_ecdb, "SELECT RawOldValue, CAST(TYPEOF(RawOldValue) AS TEXT) FROM change.PropertyValue PV WHERE  PV.Instance.Id = ?1 AND PV.AccessString = ?2 ");
    else if (stage == ChangeSummaryV2::Stage::New)
        stmt = m_stmtCache.GetPreparedStatement(m_ecdb, "SELECT RawNewValue, CAST(TYPEOF(RawNewValue) AS TEXT) FROM change.PropertyValue PV WHERE  PV.Instance.Id = ?1 AND PV.AccessString = ?2 ");

    if (stmt == nullptr)
        {
        ctx.SetResultError("Failed to prepare ECSql statement necessary for ChangedValue()");
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
// @bsimethod                                   Krischan.Eberle                   11/17
//+---------------+---------------+---------------+---------------+---------------+------
ChangedValue::ChangedValue(ECDbR ecdb)
    :ScalarFunction("ChangedValue", -1, DbValueType::BlobVal), m_ecdb(ecdb), m_stmtCache(20) //it will never have 20 infact just one statement at any given time.
    {
    if (s_setValueMap.empty())
        {
        s_setValueMap.insert(std::make_pair("null", [] (Context& ctx, ECSqlStatement& stmt) { ctx.SetResultNull(); }));
        s_setValueMap.insert(std::make_pair("integer", [] (Context& ctx, ECSqlStatement& stmt) { ctx.SetResultInt64(stmt.GetValueInt64(0)); }));
        s_setValueMap.insert(std::make_pair("real", [] (Context& ctx, ECSqlStatement& stmt) { ctx.SetResultDouble(stmt.GetValueDouble(0)); }));
        s_setValueMap.insert(std::make_pair("text", [] (Context& ctx, ECSqlStatement& stmt)
            {
            int len = (int)strlen(stmt.GetValueText(0));
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
END_BENTLEY_SQLITE_EC_NAMESPACE