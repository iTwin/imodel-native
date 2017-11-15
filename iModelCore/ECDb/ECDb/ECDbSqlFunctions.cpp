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
// ToInstanceOpFuntion
//************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                       11/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ToInstanceOpFuntion* ToInstanceOpFuntion::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                       11/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ToInstanceOpFuntion& ToInstanceOpFuntion::GetSingleton()
    {
    if (s_singleton == nullptr)
        s_singleton = new ToInstanceOpFuntion();

    return *s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                       11/17
//+---------------+---------------+---------------+---------------+---------------+------
void ToInstanceOpFuntion::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& operationValue = args[0];
    if (operationValue.GetValueType() != DbValueType::IntegerVal && operationValue.GetValueType() != DbValueType::TextVal)
        {
        ctx.SetResultError("Argument 1 of function " SQLFUNC_ToInstanceOp " is expected to be the operation and must be of integer or text type and cannot be null");
        return;
        }

    ChangeSummaryHelper::PropertyValueOp propValOp;
    if (operationValue.GetValueType() == DbValueType::IntegerVal)
        {
        int operationVal = operationValue.GetValueInt();
        if (!ChangeSummaryHelper::IsValidPropertyValueOp(operationVal))
            {
            Utf8String msg;
            msg.Sprintf("Argument 1 of function " SQLFUNC_ToInstanceOp " has an invalid operation value (%d)", operationVal);
            ctx.SetResultError(msg.c_str());
            return;
            }

        propValOp = Enum::FromInt<ChangeSummaryHelper::PropertyValueOp>(operationVal);
        }
    else
        {
        Utf8CP operationVal = operationValue.GetValueText();
        if (!ChangeSummaryHelper::TryParsePropertyValueOp(propValOp, operationVal))
            {
            Utf8String msg;
            msg.Sprintf("Argument 3 of function " SQLFUNC_ToInstanceOp " has an invalid operation value (%s)", operationVal);
            ctx.SetResultError(msg.c_str());
            return;
            }
        }

    ctx.SetResultInt(Enum::ToInt(ChangeSummaryHelper::ToInstanceOp(propValOp)));
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
ChangedValueFunction::ChangedValueFunction(ECDbCR ecdb) : ECDbSystemScalarFunction(ecdb, SQLFUNC_ChangedValue, 4, DbValueType::BlobVal)
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
    const int kInstanceId = 0;  // [Required]
    const int kAccessString = 1;    // [Required]
    const int kOperation = 2;       // [Required]
    const int kFallBackValue = 3;   // [Required]

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
    if (operationValue.GetValueType() != DbValueType::IntegerVal && operationValue.GetValueType() != DbValueType::TextVal)
        {
        ctx.SetResultError("Argument 3 of function " SQLFUNC_ChangedValue " is expected to be the operation and must be of integer or text type and cannot be null");
        return;
        }

    ChangeSummaryHelper::PropertyValueOp propValOp;
    if (operationValue.GetValueType() == DbValueType::IntegerVal)
        {
        int operationVal = operationValue.GetValueInt();
        if (!ChangeSummaryHelper::IsValidPropertyValueOp(operationVal))
            {
            Utf8String msg;
            msg.Sprintf("Argument 3 of function " SQLFUNC_ChangedValue " has an invalid operation value (%d)", operationVal);
            ctx.SetResultError(msg.c_str());
            return;
            }

        propValOp = Enum::FromInt<ChangeSummaryHelper::PropertyValueOp>(operationVal);
        }
    else
        {
        Utf8CP operationVal = operationValue.GetValueText();
        if (!ChangeSummaryHelper::TryParsePropertyValueOp(propValOp, operationVal))
            {
            Utf8String msg;
            msg.Sprintf("Argument 3 of function " SQLFUNC_ChangedValue " has an invalid operation value (%s)", operationVal);
            ctx.SetResultError(msg.c_str());
            return;
            }
        }

    DbValue const& fallbackValue = args[3];
    Utf8CP ecsql = nullptr;
    if (ChangeSummaryHelper::OpReferToOldValue(propValOp))
        ecsql = "SELECT RawOldValue, CAST(TYPEOF(RawOldValue) AS TEXT) FROM " ECSCHEMA_ALIAS_ECDbChangeSummaries "." ECDBCHANGE_CLASS_PropertyValueChange " WHERE InstanceChange.Id=? AND AccessString=?";
    else
        ecsql = "SELECT RawNewValue, CAST(TYPEOF(RawNewValue) AS TEXT) FROM " ECSCHEMA_ALIAS_ECDbChangeSummaries "." ECDBCHANGE_CLASS_PropertyValueChange " WHERE InstanceChange.Id=? AND AccessString=?";

    CachedECSqlStatementPtr stmt = GetPreparedStatement(ecsql);
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


//************************************************************************************
// ECDbSystemScalarFunction
//************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
CachedECSqlStatementPtr ECDbSystemScalarFunction::GetPreparedStatement(Utf8CP ecsql) const
    {
    return m_stmtCache.GetPreparedStatement(m_ecdb, ecsql);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
ECDbSystemScalarFunction::ECDbSystemScalarFunction(ECDbCR ecdb, Utf8CP name, int nArgs, DbValueType returnType)
    : ScalarFunction(name, nArgs, returnType), m_ecdb(ecdb), m_stmtCache(10)
    {}


//************************************************************************************
// ChangeSummaryHelper
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
std::set<ChangeSummaryHelper::PropertyValueOp> ChangeSummaryHelper::s_validPropertyValueOp =
    {
    PropertyValueOp::Inserted,
    PropertyValueOp::Deleted,
    PropertyValueOp::UpdatedOld,
    PropertyValueOp::UpdatedNew
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
std::set<ChangeSummaryHelper::InstanceOp> ChangeSummaryHelper::s_validInstanceOp =
    {
    InstanceOp::Inserted,
    InstanceOp::Deleted,
    InstanceOp::Updated,
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
std::map<Utf8CP, ChangeSummaryHelper::PropertyValueOp, CompareIUtf8Ascii> ChangeSummaryHelper::s_stringToPropertyValueOp =
    {
            {"inserted",  PropertyValueOp::Inserted},
            {"deleted",  PropertyValueOp::Deleted},
            {"updated.new",  PropertyValueOp::UpdatedNew},
            {"updated.old",  PropertyValueOp::UpdatedOld},
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
std::map<ChangeSummaryHelper::PropertyValueOp, ChangeSummaryHelper::InstanceOp> ChangeSummaryHelper::s_propertyValueToInstanceOp =
    {
            {PropertyValueOp::Inserted,InstanceOp::Inserted},
            {PropertyValueOp::Deleted,InstanceOp::Deleted},
            {PropertyValueOp::UpdatedNew,InstanceOp::Updated},
            {PropertyValueOp::UpdatedOld,InstanceOp::Updated},
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangeSummaryHelper::IsValidInstanceOp(int i)
    {
    return s_validInstanceOp.find(Enum::FromInt<InstanceOp>(i)) != s_validInstanceOp.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangeSummaryHelper::IsValidPropertyValueOp(int i)
    {
    return s_validPropertyValueOp.find(Enum::FromInt<PropertyValueOp>(i)) != s_validPropertyValueOp.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangeSummaryHelper::TryParsePropertyValueOp(PropertyValueOp& op, Utf8CP v)
    {
    auto itor = s_stringToPropertyValueOp.find(v);
    if (itor == s_stringToPropertyValueOp.end())
        return false;

    op = itor->second;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangeSummaryHelper::OpReferToOldValue(PropertyValueOp op)
    {
    return op == PropertyValueOp::Deleted || op == PropertyValueOp::UpdatedOld;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan              11/17
//+---------------+---------------+---------------+---------------+---------------+------
ChangeSummaryHelper::InstanceOp ChangeSummaryHelper::ToInstanceOp(PropertyValueOp op)
    {
    return s_propertyValueToInstanceOp[op];
    }
END_BENTLEY_SQLITE_EC_NAMESPACE