/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//================================[PropExistsFunc]=======================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<PropExistsFunc> PropExistsFunc::Create(ECDbCR ecdb) {
    return std::make_unique<PropExistsFunc>(ecdb);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PropExistsFunc::_ComputeScalar(Context& ctx, int nArgs, DbValue* args) {
    if (nArgs != 2) {
        ctx.SetResultError("prop_exists(I,S]) expect two args");
        return;
    }

    DbValue const& classIdVal = args[0];
    if (classIdVal.IsNull() || classIdVal.GetValueType() != DbValueType::IntegerVal )
        return;

    DbValue const& propVal = args[1];
    if (propVal.IsNull() || propVal.GetValueType() != DbValueType::TextVal )
        return;

    ECN::ECClassId classId(classIdVal.GetValueUInt64());
    ctx.SetResultInt(m_propMap.Exist(classId, propVal.GetValueText()) ? 1 : 0);
}

//===============================[ExtractPropFunc]=======================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ExtractPropFunc> ExtractPropFunc::Create(ECDbCR ecdb) {
    return std::make_unique<ExtractPropFunc>(ecdb);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ExtractPropFunc::_ComputeScalar(Context& ctx, int nArgs, DbValue* args) {
    if (nArgs != 3) {
        ctx.SetResultError("extract_prop(I,I,S]) expect three args");
        return;
    }

    DbValue const& classIdVal = args[0];
    if (classIdVal.IsNull() || classIdVal.GetValueType() != DbValueType::IntegerVal )
        return;

    DbValue const& instanceIdVal = args[1];
    if (instanceIdVal.IsNull() || instanceIdVal.GetValueType() != DbValueType::IntegerVal )
        return;

    DbValue const& accessStringVal = args[2];
    if (accessStringVal.IsNull() || accessStringVal.GetValueType() != DbValueType::TextVal )
        return;


    ECInstanceId instanceId(instanceIdVal.GetValueUInt64());
    ECN::ECClassId classId(classIdVal.GetValueUInt64());

    m_ecdb.GetInstanceReader().Seek(
        InstanceReader::Position(instanceId, classId, accessStringVal.GetValueText()),
        [&](InstanceReader::IRowContext const& row){
        auto& val = row.GetValue(0);
        if (val.IsNull()) {
            return;
        }
        auto& ci = val.GetColumnInfo();
        if (ci.GetDataType().IsPrimitive()){
            const auto type = ci.GetDataType().GetPrimitiveType();
            if (type == ECN::PrimitiveType::PRIMITIVETYPE_Binary) {
                int blobSize = 0;
                auto blob = val.GetBlob(&blobSize);
                ctx.SetResultBlob(blob, blobSize);
                return;
            }
            if (type == ECN::PrimitiveType::PRIMITIVETYPE_Boolean) {
                ctx.SetResultInt(val.GetBoolean()?1:0);
                return;
            }
            if (type == ECN::PrimitiveType::PRIMITIVETYPE_DateTime) {
                double jdt;
                val.GetDateTime().ToJulianDay(jdt);
                ctx.SetResultDouble(jdt);
                return;
            }
            if (type == ECN::PrimitiveType::PRIMITIVETYPE_Double) {
                ctx.SetResultDouble(val.GetDouble());
                return;
            }
            if (type == ECN::PrimitiveType::PRIMITIVETYPE_Integer ||
                type == ECN::PrimitiveType::PRIMITIVETYPE_Long) {
                ctx.SetResultInt64(val.GetInt64());
                return;
            }
            if (type == ECN::PrimitiveType::PRIMITIVETYPE_String) {
                ctx.SetResultText(val.GetText(), (int)strlen(val.GetText()), Context::CopyData::Yes );
                return;
            }
        }
        const auto json = row.GetJson().Stringify();
        ctx.SetResultText(json.c_str(), static_cast<int>(json.length()), Context::CopyData::Yes);
    });
}

//==================================[ExtractInstFunc]=======================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ExtractInstFunc> ExtractInstFunc::Create(ECDbCR ecdb) {
    return std::make_unique<ExtractInstFunc>(ecdb);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ExtractInstFunc::_ComputeScalar(Context& ctx, int nArgs, DbValue* args) {
    if (nArgs < 2 || nArgs > 3) {
        ctx.SetResultError("extract_inst(I,I]) expect two args");
        return;
    }

    DbValue const& classIdVal = args[0];
    if (classIdVal.IsNull())
        return;


    DbValue const& instanceIdVal = args[1];
    if (instanceIdVal.IsNull() || instanceIdVal.GetValueType() != DbValueType::IntegerVal )
        return;

    InstanceReader::JsonParams opts;
    if (nArgs == 3) {
        DbValue const& optsVal = args[2];
        if (optsVal.IsNull() || optsVal.GetValueType() != DbValueType::TextVal){
            return;
        }
    }

    auto setResult = [&](InstanceReader::IRowContext const& row){
        const auto json = row.GetJson().Stringify();
        ctx.SetResultText(json.c_str(), static_cast<int>(json.length()), Context::CopyData::Yes);
    };

    ECInstanceId instanceId(instanceIdVal.GetValueUInt64());
    if (classIdVal.GetValueType() == DbValueType::IntegerVal) {
        ECN::ECClassId classId(classIdVal.GetValueUInt64());
        m_ecdb.GetInstanceReader().Seek(
            InstanceReader::Position(instanceId,classId, nullptr),
            setResult);
        return;
    }

    if (classIdVal.GetValueType() == DbValueType::TextVal) {
        m_ecdb.GetInstanceReader().Seek(
            InstanceReader::Position(instanceId, classIdVal.GetValueText(), nullptr),
            setResult);

    }
}

//=================================[ClassNameFunc]=======================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ClassNameFunc::ClassNameFunc(DbCR db) : ScalarFunction(SQLFUNC_ClassName, -1, DbValueType::TextVal), m_db(&db)
    {
    m_options["s:c"] = FormatOptions::s_semicolon_c;
    m_options["a:c"] = FormatOptions::a_semicolon_c;
    m_options["s.c"] = FormatOptions::s_dot_c;
    m_options["a.c"] = FormatOptions::a_dot_c;
    m_options["a"] = FormatOptions::a;
    m_options["s"] = FormatOptions::s;
    m_options["c"] = FormatOptions::c;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ClassNameFunc::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    if (nArgs < 1 || nArgs > 2)
        {
        ctx.SetResultError("ec_classname(I[,S]) expect one or two args");
        return;
        }

    DbValue const& classId = args[0];
    if (classId.IsNull())
        return;

    auto fmt = FormatOptions::default_fmt;
    if (nArgs == 2)
        {
        DbValue const& fmtOption = args[1];
        if (!fmtOption.IsNull())
            {
            if (fmtOption.GetValueType() == DbValueType::TextVal)
                {
                auto it = m_options.find(fmtOption.GetValueText());
                if (it == m_options.end())
                    return;

                fmt = it->second;
                }
            else if (fmtOption.GetValueType() == DbValueType::IntegerVal)
                {
                const auto fmtId = fmtOption.GetValueInt();
                if (fmtId < (int)FormatOptions::s_semicolon_c && fmtId > (int)FormatOptions::a_dot_c)
                    return;

                fmt = (FormatOptions)fmtId;
                }
            }
        }

    auto stmt = m_db->GetCachedStatement(R"sql(
        select
               s.name  || ':' || c.name sn_semicolon_cn,
               s.alias || ':' || c.name sa_semicolon_cn,
               s.name                   sn,
               s.alias                  sa,
               c.name                   cn,
               s.name  || '.' || c.name sn_dot_cn,
               s.alias || '.' || c.name sa_dot_cn
        from ec_class c join ec_schema s on c.SchemaId = s.Id where c.Id = ?)sql");
    if (stmt == nullptr)
        {
        ctx.SetResultError("ec_classname(I[,S]) db is close");
        return;
        }
    stmt->BindUInt64(1, classId.GetValueUInt64());
    if (stmt->Step() != BE_SQLITE_ROW)
        return;

    const auto strOut = stmt->GetValueText((int)fmt);
    const auto strSize = stmt->GetColumnBytes((int)fmt);
    ctx.SetResultText(strOut, strSize, DbFunction::Context::CopyData::Yes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
 std::unique_ptr<ClassNameFunc> ClassNameFunc::Create(DbCR db) { return std::unique_ptr<ClassNameFunc>(new ClassNameFunc(db));}

//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ClassIdFunc::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    if (nArgs < 1 || nArgs > 2)
        {
        ctx.SetResultError("ec_classid(X[,Y]) expect one or two args.");
        return;
        }

    DbValue const& a0 = args[0];
    if (a0.GetValueType() != DbValueType::TextVal)
        return;

    Utf8CP tokenFirst = a0.GetValueText();
    const int tokenFirstLen = a0.GetValueBytes();
    Utf8Char buffer[512];
    BeStringUtilities::Strncpy(buffer, tokenFirst, tokenFirstLen);
    Utf8CP tokenSecond = nullptr;
    if (nArgs == 2)
        {
        DbValue const& a1 = args[1];
        if (!a1.IsNull())
            {
            if (a1.GetValueType() != DbValueType::TextVal)
                return;

            tokenSecond = a1.GetValueText();
            }
        }

    Utf8CP schemaNameOrAlias;
    Utf8CP className;
    Utf8CP delimiters = ".:";
    Utf8P ctxTok;
    if (!tokenSecond)
        {
        schemaNameOrAlias = BeStringUtilities::Strtok(buffer, delimiters, &ctxTok);
        if (schemaNameOrAlias == nullptr)
            return;

        className = BeStringUtilities::Strtok (nullptr, delimiters, &ctxTok);
        if (className == nullptr)
            return;
        }
    else
        {
        schemaNameOrAlias = tokenFirst;
        className = tokenSecond;
        }

    auto stmt = m_db->GetCachedStatement("select c.id from ec_class c join ec_schema s on c.SchemaId = s.Id where c.name= ?1 and (s.name =?2 or s.alias = ?2)");
    if (stmt == nullptr)
        {
        ctx.SetResultError("ec_class() db is close");
        return;
        }

    stmt->BindText(1, className, Statement::MakeCopy::No);
    stmt->BindText(2, schemaNameOrAlias, Statement::MakeCopy::No);

    if (stmt->Step() != BE_SQLITE_ROW)
        return;

    ctx.SetResultInt64(stmt->GetValueInt64(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ClassIdFunc> ClassIdFunc::Create(DbCR db) { return std::unique_ptr<ClassIdFunc>(new ClassIdFunc(db));}

//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void InstanceOfFunc::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    if (nArgs <= 1)
        {
        ctx.SetResultError("ec_instanceof(C[,Y1,...]) expect one arg.");
        return;
        }

    ECN::ECClassId curId;
    IdSet<ECN::ECClassId> classIds;
    Utf8CP delimiters = ".:";
    Utf8Char buffer[512];
    for (int i = 0; i < nArgs; i++)
        {
        DbValue const &arg = args[i];
        if (arg.IsNull())
            continue;

        if (arg.GetValueType() == DbValueType::IntegerVal)
            {
            if (i == 0)
                curId = ECN::ECClassId(arg.GetValueUInt64());
            else
                classIds.insert(ECN::ECClassId(arg.GetValueUInt64()));
            }
        if (arg.GetValueType() == DbValueType::TextVal)
            {
            auto name = arg.GetValueText();
            auto it = m_cache.find(name);
            if (it != m_cache.end())
                {
                if (i == 0)
                    curId = it->second;
                else
                    classIds.insert(it->second);
                continue;
                }

            // resolve name into classid
            Utf8P ctxTok;
            BeStringUtilities::Strncpy(buffer, name, arg.GetValueBytes());
            auto schemaNameOrAlias = BeStringUtilities::Strtok(buffer, delimiters, &ctxTok);
            if (schemaNameOrAlias == nullptr)
                return;

            auto className = BeStringUtilities::Strtok(nullptr, delimiters, &ctxTok);
            if (className == nullptr)
                return;

            auto stmt = m_db->GetCachedStatement("select c.id from ec_class c join ec_schema s on c.SchemaId = s.Id where c.name= ?1 and (s.name =?2 or s.alias = ?2)");
            if (stmt == nullptr)
                {
                ctx.SetResultError("ec_instanceof() db is close");
                return;
                }
            stmt->BindText(1, className, Statement::MakeCopy::No);
            stmt->BindText(2, schemaNameOrAlias, Statement::MakeCopy::No);
            if (stmt->Step() != BE_SQLITE_ROW)
                return;

            auto cid = stmt->GetValueId<ECN::ECClassId>(0);
            m_cache[name] = cid;
            if (i == 0)
                curId = cid;
            else
                classIds.insert(cid);
            }
        }


    auto stmt = m_db->GetCachedStatement("select classid from ec_cache_ClassHierarchy where InVirtualSet(?,baseClassId) and classid=? limit 1");
    if (stmt == nullptr)
        {
        ctx.SetResultError("ec_instanceof() db is close");
        return;
        }
    stmt->BindVirtualSet(1, classIds);
    stmt->BindId(2, curId);
    ctx.SetResultInt(stmt->Step() == BE_SQLITE_ROW ? 1 : 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<InstanceOfFunc> InstanceOfFunc::Create(DbCR db) { return std::unique_ptr<InstanceOfFunc>(new InstanceOfFunc(db));}

//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
StrToGuid& StrToGuid::GetSingleton()
    {
    static StrToGuid s_singleton;
    return s_singleton;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void StrToGuid::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& v = args[0];
    if (v.IsNull() || v.GetValueType() != DbValueType::TextVal)
        {
        ctx.SetResultNull();
        return;
        }

    BeGuid guid;
    if (guid.FromString(v.GetValueText()) != BentleyStatus::SUCCESS)
        {
        ctx.SetResultNull();
        return;
        }

    ctx.SetResultBlob(&guid, sizeof(BeGuid), DbFunction::Context::CopyData::Yes);
    }


//************************************************************************************
// GuidToStr
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------

GuidToStr& GuidToStr::GetSingleton()
    {
    static GuidToStr s_singleton;
    return s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void GuidToStr::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& v = args[0];
    if (v.IsNull() || v.GetValueType() != DbValueType::BlobVal || v.GetValueBytes() != sizeof(BeGuid))
        {
        ctx.SetResultNull();
        return;
        }

    BeGuid guid;
    memcpy(&guid, v.GetValueBlob(), sizeof(BeGuid));
    Utf8String str = guid.ToString();
    ctx.SetResultText(str.c_str(), static_cast<int>(str.size()), DbFunction::Context::CopyData::Yes);
    }

//************************************************************************************
// IdToHex
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------

IdToHex& IdToHex::GetSingleton()
    {
    static IdToHex s_singleton;
    return s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void IdToHex::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& v = args[0];
    if (v.IsNull() || v.GetValueType() != DbValueType::IntegerVal)
        {
        ctx.SetResultNull();
        return;
        }

    static const size_t stringBufferLength = 19;
    Utf8Char stringBuffer[stringBufferLength];
    BeStringUtilities::FormatUInt64(stringBuffer, stringBufferLength, v.GetValueUInt64(), HexFormatOptions::IncludePrefix);
    ctx.SetResultText(stringBuffer, (int) strlen(stringBuffer), Context::CopyData::Yes);
    }

//************************************************************************************
// HexToId
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------

HexToId& HexToId::GetSingleton()
    {
    static HexToId s_singleton;
    return s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void HexToId::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    DbValue const& v = args[0];
    if (v.IsNull() || v.GetValueType() != DbValueType::TextVal)
        {
        ctx.SetResultNull();
        return;
        }
    ctx.SetResultInt64(BeStringUtilities::ParseHex(v.GetValueText(), nullptr));
    }
//************************************************************************************
// ChangedValueStateToOpCodeSqlFunction
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ChangedValueStateToOpCodeSqlFunction& ChangedValueStateToOpCodeSqlFunction::GetSingleton()
    {
    static ChangedValueStateToOpCodeSqlFunction s_singleton;
    return s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
        state = ChangeManager::ToChangedValueState(stateValue.GetValueInt());
    else
        state = ChangeManager::ToChangedValueState(stateValue.GetValueText());

    Nullable<ChangeOpCode> opCode;
    if (state.IsNull() || (opCode = ChangeManager::DetermineOpCodeFromChangedValueState(state.Value())).IsNull())
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
// @bsimethod
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
        state = ChangeManager::ToChangedValueState(changedValueStateValue.GetValueInt());
    else
        state = ChangeManager::ToChangedValueState(changedValueStateValue.GetValueText());

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
        ecsql = "SELECT RawOldValue,TYPEOF(RawOldValue) FROM " ECSCHEMA_ALIAS_ECDbChange "." ECDBCHANGE_CLASS_PropertyValueChange " WHERE InstanceChange.Id=? AND AccessString=?";
    else
        ecsql = "SELECT RawNewValue,TYPEOF(RawNewValue) FROM " ECSCHEMA_ALIAS_ECDbChange "." ECDBCHANGE_CLASS_PropertyValueChange " WHERE InstanceChange.Id=? AND AccessString=?";

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

    if (valType[0] == 'i' /*BeStringUtilities::StricmpAscii("integer", valType) == 0*/)
        {
        BeAssert(BeStringUtilities::StricmpAscii("integer", valType) == 0);
        ctx.SetResultInt64(stmt->GetValueInt64(0));
        return;
        }

    if (valType[0] == 'r' /*BeStringUtilities::StricmpAscii("real", valType) == 0*/)
        {
        BeAssert(BeStringUtilities::StricmpAscii("real", valType) == 0);
        ctx.SetResultDouble(stmt->GetValueDouble(0));
        return;
        }

    if (valType[0] == 't' /*BeStringUtilities::StricmpAscii("text", valType) == 0*/)
        {
        BeAssert(BeStringUtilities::StricmpAscii("text", valType) == 0);
        Utf8CP strVal = stmt->GetValueText(0);
        const int len = (int) strlen(strVal);
        ctx.SetResultText(strVal, len, Context::CopyData::Yes);
        return;
        }

    if (valType[0] == 'b' /*BeStringUtilities::StricmpAscii("blob", valType) == 0*/)
        {
        BeAssert(BeStringUtilities::StricmpAscii("blob", valType) == 0);
        int blobSize = -1;
        void const* blob = stmt->GetValueBlob(0, &blobSize);
        ctx.SetResultBlob(blob, blobSize, Context::CopyData::Yes);
        return;
        }

    ctx.SetResultError(SqlPrintfString("SQL function " SQLFUNC_ChangedValue " failed: executing the ECSQL '%s' returned an unsupported data type (%s).", stmt->GetECSql(), valType));
    }

END_BENTLEY_SQLITE_EC_NAMESPACE