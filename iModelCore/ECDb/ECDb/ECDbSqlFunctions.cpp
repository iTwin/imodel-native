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
    //1 - csInstanceId        [Required]
    //2 - AccessString        [Required]
    //3 - Operation           [Requied] inserted|updated_old|updated_new|deleted
    //4 - FallBackValue       [Required]
    if (nArgs < 4)
        {
        ctx.SetResultError("Not enough parameters");
        return;
        }

    //following should moved out 
    enum OpCode
        {
        OP_INSERTED = 1,
        OP_UPDATED_NEW = 2,
        OP_UPDATED_OLD = 3,
        OP_DELETED = 4
        };

    //Decode and verify parameters
    if (args[0].GetValueType() == DbValueType::IntegerVal)
        {
        ctx.SetResultError("Parameter one expect to be of integer type and cannot be null");
        return;
        }

    if (args[1].GetValueType() == DbValueType::TextVal)
        {
        ctx.SetResultError("Parameter two expect to be of text type and cannot be null");
        return;
        }

    if (args[2].GetValueType() == DbValueType::IntegerVal)
        {
        ctx.SetResultError("Parameter three expect to be of integer type and cannot be null");
        return;
        }

    const ECInstanceId csInstanceId = args[0].GetValueId<ECInstanceId>();
    Utf8CP accessString = args[1].GetValueText();
    const OpCode operation = (OpCode) args[2].GetValueInt();
    const DbValue& fallbackValue = args[3];

    //Optimize case do not need to query
    // Update + New is already part of db
    if (operation == OP_INSERTED || operation == OP_UPDATED_NEW) //OP_INSERTED | OP_UPDATED_NEW
        {
        ctx.SetResultValue(fallbackValue); //fallback is the current value of the column
        return;
        }

    //Use cached statement as this would be called very frequently
    CachedECSqlStatementPtr stmt = m_stmtCache.GetPreparedStatement(m_ecdb, "SELECT RawOldValue, CAST(TYPEOF(RawOldValue) AS TEXT) FROM cs.PropertyValue PV WHERE  PV.Instance.Id = ?1 AND PV.AccessString = ?2 ");
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
    :ScalarFunction("ChangedValue", 4, DbValueType::BlobVal), m_ecdb(ecdb), m_stmtCache(20) //it will never have 20 infact just one statement at any given time.
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