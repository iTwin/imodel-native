/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <GeomSerialization/GeomSerializationApi.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

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
    if (nArgs < 3) {
        ctx.SetResultError("extract_prop(I,I,I,S,P,I) expect atleast three args");
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

    unsigned int jsonFlags = 0;
    if (nArgs > 3) {
        DbValue const& jsonFlagsVal = args[3];
        if (jsonFlagsVal.GetValueType() == DbValueType::IntegerVal ) {
            jsonFlags = static_cast<unsigned int>(jsonFlagsVal.GetValueInt());
        }
    }

    // The 4th arg P is pointer to ECSqlStatement while 5th is columnIndex.
    ECSqlField* field = nullptr;
    ECSqlSelectPreparedStatement* stmt = nullptr;
    int columnInfoIndex = -1;
    if (nArgs == 6) {
        auto& ptrArg = args[4];
        if (ptrArg.FromBinding()) {
            stmt = (ECSqlSelectPreparedStatement*)ptrArg.GetValuePointer(ECSqlSelectPreparedStatement::SELECT_PTR_NAME);
            auto& colIdxArg = args[5];
            if (colIdxArg.GetNumericType() == DbValueType::IntegerVal) {
                columnInfoIndex = colIdxArg.GetValueInt();
                if (columnInfoIndex < 0 && columnInfoIndex >= stmt->GetColumnCount() && !stmt->GetValue(columnInfoIndex).GetColumnInfo().IsDynamic()) {
                    stmt = nullptr;
                } else {
                    field = stmt->GetField(columnInfoIndex);
                }
            }
        }
    }

    ECInstanceId instanceId(instanceIdVal.GetValueUInt64());
    ECN::ECClassId classId(classIdVal.GetValueUInt64());
    if (field) field->SetDynamicColumnInfo(ECSqlColumnInfo());
    m_ecdb.GetInstanceReader().Seek(
        InstanceReader::Position(instanceId, classId, accessStringVal.GetValueText()),
        [&](InstanceReader::IRowContext const& row, auto _){
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
                if (field) field->SetDynamicColumnInfo(ci);
                return;
            }
            if (type == ECN::PrimitiveType::PRIMITIVETYPE_Boolean) {
                ctx.SetResultInt(val.GetBoolean()?1:0);
                if (field) field->SetDynamicColumnInfo(ci);
                return;
            }
            if (type == ECN::PrimitiveType::PRIMITIVETYPE_DateTime) {
                ctx.SetResultDouble(val.GetDouble());
                if (field) field->SetDynamicColumnInfo(ci);
                return;
            }
            if (type == ECN::PrimitiveType::PRIMITIVETYPE_Double) {
                ctx.SetResultDouble(val.GetDouble());
                if (field) field->SetDynamicColumnInfo(ci);
                return;
            }
            if (type == ECN::PrimitiveType::PRIMITIVETYPE_Integer ||
                type == ECN::PrimitiveType::PRIMITIVETYPE_Long) {
                ctx.SetResultInt64(val.GetInt64());
                if (field) field->SetDynamicColumnInfo(ci);
                return;
            }
            if (type == ECN::PrimitiveType::PRIMITIVETYPE_String) {
                ctx.SetResultText(val.GetText(), (int)strlen(val.GetText()), Context::CopyData::Yes );
                if (field) field->SetDynamicColumnInfo(ci);
                return;
            }
        }

        JsReadOptions params;
        params.SetUseJsNames(jsonFlags & InstanceReader::FLAGS_UseJsPropertyNames);
        params.SetAbbreviateBlobs(!(jsonFlags & InstanceReader::FLAGS_DoNotTruncateBlobs));

        const auto json = row.GetJson(params).Stringify();
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
        ctx.SetResultError("extract_inst(I,I,I]) expect two args");
        return;
    }

    DbValue const& classIdVal = args[0];
    if (classIdVal.IsNull())
        return;


    DbValue const& instanceIdVal = args[1];
    if (instanceIdVal.IsNull() || instanceIdVal.GetValueType() != DbValueType::IntegerVal )
        return;


    unsigned int jsonFlags = 0;
    if (nArgs > 2) {
        DbValue const& jsonFlagsVal = args[2];
        if (jsonFlagsVal.GetValueType() == DbValueType::IntegerVal ) {
            jsonFlags = static_cast<unsigned int>(jsonFlagsVal.GetValueInt());
        }
    }

    JsReadOptions params;
    params.SetUseJsNames(jsonFlags & InstanceReader::FLAGS_UseJsPropertyNames);
    params.SetAbbreviateBlobs(!(jsonFlags & InstanceReader::FLAGS_DoNotTruncateBlobs));

    auto setResult = [&](InstanceReader::IRowContext const& row, auto _) {
        const auto json = row.GetJson(params).Stringify();
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
        ctx.SetResultError("ec_classid(X[,Y]) expects one or two args.");
        return;
        }

    DbValue const& a0 = args[0];
    if (a0.GetValueType() != DbValueType::TextVal)
        return;

    Utf8String tokenFirst = a0.GetValueText();
    const int tokenFirstLen = a0.GetValueBytes();
    Utf8String tokenSecond;
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

    Utf8String schemaNameOrAlias;
    Utf8String className;
    if (Utf8String::IsNullOrEmpty(tokenSecond.c_str()))
        {
        const auto delimiterPos = tokenFirst.find_first_of(".:");
        schemaNameOrAlias = tokenFirst.substr(0, delimiterPos);
        if (Utf8String::IsNullOrEmpty(schemaNameOrAlias.c_str()))
            return;

        if (delimiterPos != std::string::npos)
            className = tokenFirst.substr(delimiterPos + 1U, tokenFirstLen);
        if (Utf8String::IsNullOrEmpty(className.c_str()))
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
        ctx.SetResultError("ec_instanceof(C[,Y1,...]) expects more than one arg.");
        return;
        }

    ECN::ECClassId curId;
    IdSet<ECN::ECClassId> classIds;
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
            Utf8String name = arg.GetValueText();
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
            const auto delimiterPos = name.find_first_of(".:");
            Utf8String schemaNameOrAlias = name.substr(0, delimiterPos);
            if (Utf8String::IsNullOrEmpty(schemaNameOrAlias.c_str()))
                return;

            Utf8String className;
            if (delimiterPos != std::string::npos)
                className = name.substr(delimiterPos + 1U, name.length());
            if (Utf8String::IsNullOrEmpty(className.c_str()))
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

//************************************************************************************
// XmlCAToJson
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<XmlCAToJson> XmlCAToJson::Create(SchemaManager const& schemaManager) { return std::unique_ptr<XmlCAToJson>(new XmlCAToJson(schemaManager));}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void XmlCAToJson::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    if(nArgs != 2)
        {
        ctx.SetResultError("SQL function " SQLFUNC_XmlCAToJson " failed: invalid number of arguments. Expected 2 arguments.");
        return;
        }

    DbValue const& idValue = args[0];
    if (idValue.IsNull() || idValue.GetValueType() != DbValueType::IntegerVal)
        {
        ctx.SetResultNull();
        return;
        }

    ECClassId caClassId = idValue.GetValueId<ECClassId>();
    ECClassCP caClass = m_schemaManager->GetClass(caClassId);
    if(caClass == nullptr)
        {
        ctx.SetResultError("SQL function " SQLFUNC_XmlCAToJson " failed: could not find custom attribute's class.");
        return;
        }

    DbValue const& xmlValue = args[1];
    if (xmlValue.IsNull() || xmlValue.GetValueType() != DbValueType::TextVal)
        {
        ctx.SetResultNull();
        return;
        }

    Utf8CP caXml = xmlValue.GetValueText();

    ECInstanceReadContextPtr readContext = ECInstanceReadContext::CreateContext(caClass->GetSchema());
    IECInstancePtr deserializedCa = nullptr;
    if (InstanceReadStatus::Success != IECInstance::ReadFromXmlString(deserializedCa, caXml, *readContext))
        {
        ctx.SetResultError("SQL function " SQLFUNC_XmlCAToJson " failed: unable to read custom attribute xml.");
        return;
        }

    BeJsDocument caJson;
    if (SUCCESS != JsonEcInstanceWriter::WriteInstanceToJson(caJson, *deserializedCa, nullptr, false))
        {
        ctx.SetResultError("SQL function " SQLFUNC_XmlCAToJson " failed: unable to serialize instance to json.");
        return;
        }

    Utf8String strVal = caJson.Stringify();
    const int len = (int) strlen(strVal.c_str());
    ctx.SetResultText(strVal.c_str(), len, Context::CopyData::Yes);
    }

//=================================[SupportInstanceQueryFunc]=======================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<SupportInstanceQueryFunc> SupportInstanceQueryFunc::Create(ECDbCR ecdb) {
    return std::make_unique<SupportInstanceQueryFunc>(ecdb);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SupportInstanceQueryFunc::_ComputeScalar(Context& ctx, int nArgs, DbValue* args) {
    if (nArgs != 1) {
        ctx.SetResultError("supports_instance_query(S|I) expects exactly one argument: class name (e.g. 'BisCore.Element') or class id.");
        return;
    }

    DbValue const& arg = args[0];
    if (arg.IsNull()) {
        ctx.SetResultInt(0);
        return;
    }

    ECN::ECClassCP ecClass = nullptr;
    if (arg.GetValueType() == DbValueType::TextVal) {
        Utf8String name = arg.GetValueText();
        const auto delimiterPos = name.find_first_of(".:");
        if (delimiterPos == std::string::npos) {
            ctx.SetResultError("supports_instance_query() expects class name in format 'SchemaNameOrAlias.ClassName' or 'SchemaNameOrAlias:ClassName'.");
            return;
        }
        Utf8String schemaNameOrAlias = name.substr(0, delimiterPos);
        Utf8String className = name.substr(delimiterPos + 1U, name.length());
        if (Utf8String::IsNullOrEmpty(schemaNameOrAlias.c_str()) || Utf8String::IsNullOrEmpty(className.c_str())) {
            ctx.SetResultError("supports_instance_query() expects class name in format 'SchemaNameOrAlias.ClassName' or 'SchemaNameOrAlias:ClassName'.");
            return;
        }
        // Look up class by schema name/alias + class name
        auto stmt = m_ecdb.GetCachedStatement("SELECT c.Id FROM ec_Class c JOIN ec_Schema s ON c.SchemaId = s.Id WHERE c.Name = ?1 AND (s.Name = ?2 OR s.Alias = ?2)");
        if (stmt == nullptr) {
            ctx.SetResultError("supports_instance_query() db is closed.");
            return;
        }
        stmt->BindText(1, className, Statement::MakeCopy::No);
        stmt->BindText(2, schemaNameOrAlias, Statement::MakeCopy::No);
        if (stmt->Step() != BE_SQLITE_ROW) {
            ctx.SetResultInt(0);
            return;
        }
        auto classId = stmt->GetValueId<ECN::ECClassId>(0);
        ecClass = m_ecdb.Schemas().GetClass(classId);
    } else if (arg.GetValueType() == DbValueType::IntegerVal) {
        ECN::ECClassId classId(arg.GetValueUInt64());
        ecClass = m_ecdb.Schemas().GetClass(classId);
    } else {
        ctx.SetResultError("supports_instance_query() expects a text class name or integer class id.");
        return;
    }

    if (ecClass == nullptr) {
        ctx.SetResultInt(0);
        return;
    }

    if (ecClass->IsMixin()) {
        ctx.SetResultInt(0);
        return;
    }

    auto const* classMap = m_ecdb.Schemas().Main().GetClassMap(*ecClass);
    if (classMap == nullptr) {
        ctx.SetResultInt(0);
        return;
    }

    auto mapType = classMap->GetType();
    if (mapType == ClassMap::Type::NotMapped || mapType == ClassMap::Type::RelationshipEndTable) {
        ctx.SetResultInt(0);
        return;
    }

    if (mapType == ClassMap::Type::Class || mapType == ClassMap::Type::RelationshipLinkTable) {
        ctx.SetResultInt(1);
        return;
    }

    ctx.SetResultInt(0);
}

//************************************************************************************
// ECSqlCastFunc
//************************************************************************************
namespace
{
using JsonWriter = rapidjson::Writer<rapidjson::StringBuffer>;

//---------------------------------------------------------------------------------------
// Parses a complete decimal or 0x-prefixed hexadecimal unsigned 64 bit integer. No sign, whitespace or trailing characters.
//---------------------------------------------------------------------------------------
bool ParseIdText(uint64_t& id, Utf8CP str)
    {
    if (Utf8String::IsNullOrEmpty(str))
        return false;

    int base = 10;
    Utf8CP digits = str;
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        {
        base = 16;
        digits = str + 2;
        }

    if (*digits == '\0')
        return false;

    uint64_t val = 0;
    for (Utf8CP p = digits; *p != '\0'; ++p)
        {
        int d;
        const char c = *p;
        if (c >= '0' && c <= '9')
            d = c - '0';
        else if (base == 16 && c >= 'a' && c <= 'f')
            d = c - 'a' + 10;
        else if (base == 16 && c >= 'A' && c <= 'F')
            d = c - 'A' + 10;
        else
            return false;

        if (val > (UINT64_MAX - (uint64_t) d) / (uint64_t) base)
            return false; //overflow

        val = val * base + d;
        }

    id = val;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void WriteId(JsonWriter& w, uint64_t id)
    {
    Utf8Char buf[19];
    BeStringUtilities::FormatUInt64(buf, 19, id, HexFormatOptions::IncludePrefix);
    w.String(buf);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ParseDateTimeLayout(DateTime::Info& info, Utf8CP code)
    {
    if (0 == strcmp(code, "dtU")) info = DateTime::Info::CreateForDateTime(DateTime::Kind::Utc);
    else if (0 == strcmp(code, "dtL")) info = DateTime::Info::CreateForDateTime(DateTime::Kind::Local);
    else if (0 == strcmp(code, "dtN")) info = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified);
    else if (0 == strcmp(code, "dtD")) info = DateTime::Info::CreateForDate();
    else if (0 == strcmp(code, "dtT")) info = DateTime::Info::CreateForTimeOfDay();
    else return false;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool JulianDayToText(Utf8StringR str, double jd, DateTime::Info const& info)
    {
    if (!std::isfinite(jd))
        return false;

    DateTime dt = DateTime::FromJulianDay(jd, info);
    if (!dt.IsValid())
        return false;

    str = dt.ToString();
    return !str.empty();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool GeometryBlobToJson(Utf8StringR json, void const* blob, int blobSize)
    {
    if (blob == nullptr || blobSize <= 0)
        return false;

    IGeometryPtr geom = BentleyGeometryFlatBuffer::BytesToGeometrySafe((Byte const*) blob, (size_t) blobSize, true);
    if (geom == nullptr)
        return false;

    return IModelJson::TryGeometryToIModelJsonString(json, *geom) && !json.empty();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool GuidBlobToText(Utf8StringR str, void const* blob, int blobSize)
    {
    if (blob == nullptr || blobSize != (int) sizeof(BeGuid))
        return false;

    BeGuid guid;
    memcpy(&guid, blob, sizeof(BeGuid));
    str = guid.ToString();
    return true;
    }

//---------------------------------------------------------------------------------------
// Writes raw JSON text as nested value (not as a string)
//---------------------------------------------------------------------------------------
bool WriteRawJson(JsonWriter& w, Utf8StringCR json)
    {
    rapidjson::Document doc;
    if (doc.Parse<rapidjson::kParseIterativeFlag>(json.c_str()).HasParseError())
        return false;

    return doc.Accept(w);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool WritePoint(JsonWriter& w, double const* coords, int dims)
    {
    static Utf8CP const s_names[] = {"X", "Y", "Z"};
    for (int i = 0; i < dims; i++)
        {
        if (!std::isfinite(coords[i]))
            return false;
        }

    w.StartObject();
    for (int i = 0; i < dims; i++)
        {
        w.Key(s_names[i]);
        w.Double(coords[i]);
        }
    w.EndObject();
    return true;
    }

//=======================================================================================
// Renders SQL values described by a layout
//=======================================================================================
struct ColumnRenderer final
    {
    private:
        DbValue* m_args;
        int m_nArgs;
        int m_pos;

        static bool RenderArrayJson(JsonWriter&, RapidJsonValueCR layout, Utf8CP internalJson);

    public:
        ColumnRenderer(DbValue* args, int nArgs, int startPos) : m_args(args), m_nArgs(nArgs), m_pos(startPos) {}
        int GetPosition() const { return m_pos; }

        //! Renders the value at the current position. Returns false on layout/arity mismatch.
        //! isNull is set to true if the value is NULL (and nothing was written).
        bool Render(JsonWriter& w, bool& isNull, RapidJsonValueCR layout, bool writeMemberKey, Utf8CP memberName);
        static bool RenderElement(JsonWriter&, RapidJsonValueCR layout, RapidJsonValueCR element);
        static bool RenderArray(JsonWriter& w, RapidJsonValueCR elementLayout, Utf8CP internalJson);
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ColumnRenderer::Render(JsonWriter& w, bool& isNull, RapidJsonValueCR layout, bool writeMemberKey, Utf8CP memberName)
    {
    isNull = false;
    if (layout.IsObject() && layout.HasMember("m"))
        {
        //struct: render into a separate buffer, as all-null structs must be omitted
        RapidJsonValueCR members = layout["m"];
        rapidjson::StringBuffer buf;
        JsonWriter sw(buf);
        sw.StartObject();
        bool anyMember = false;
        for (rapidjson::SizeType i = 0; i < members.Size(); i++)
            {
            RapidJsonValueCR member = members[i];
            bool memberIsNull = false;
            if (!Render(sw, memberIsNull, member[1], true, member[0].GetString()))
                return false;

            anyMember |= !memberIsNull;
            }
        sw.EndObject();
        if (!anyMember)
            {
            isNull = true;
            return true;
            }

        if (writeMemberKey)
            w.Key(memberName);

        return w.RawValue(buf.GetString(), buf.GetSize(), rapidjson::kObjectType);
        }

    if (layout.IsObject() && layout.HasMember("a"))
        {
        if (m_pos >= m_nArgs)
            return false;

        DbValue const& v = m_args[m_pos++];
        if (v.IsNull() || v.GetValueType() != DbValueType::TextVal)
            {
            isNull = true;
            return true;
            }

        rapidjson::StringBuffer buf;
        JsonWriter sw(buf);
        if (!RenderArray(sw, layout["a"], v.GetValueText()))
            {
            isNull = true;
            return true;
            }

        if (writeMemberKey)
            w.Key(memberName);

        return w.RawValue(buf.GetString(), buf.GetSize(), rapidjson::kArrayType);
        }

    if (!layout.IsString())
        return false;

    Utf8CP code = layout.GetString();
    const bool isPoint2d = 0 == strcmp(code, ECSqlCastFunc::LayoutPoint2d());
    if (isPoint2d || 0 == strcmp(code, ECSqlCastFunc::LayoutPoint3d()))
        {
        const int dims = isPoint2d ? 2 : 3;
        if (m_pos + dims > m_nArgs)
            return false;

        double coords[3];
        for (int i = 0; i < dims; i++)
            {
            DbValue const& c = m_args[m_pos + i];
            if (c.IsNull())
                isNull = true;
            else
                coords[i] = c.GetValueDouble();
            }

        m_pos += dims;
        if (isNull)
            return true;

        rapidjson::StringBuffer buf;
        JsonWriter sw(buf);
        if (!WritePoint(sw, coords, dims))
            {
            isNull = true;
            return true;
            }

        if (writeMemberKey)
            w.Key(memberName);

        return w.RawValue(buf.GetString(), buf.GetSize(), rapidjson::kObjectType);
        }

    if (m_pos >= m_nArgs)
        return false;

    DbValue const& v = m_args[m_pos++];
    if (v.IsNull())
        {
        isNull = true;
        return true;
        }

    rapidjson::StringBuffer buf;
    JsonWriter sw(buf);
    DateTime::Info dtInfo;
    if (0 == strcmp(code, ECSqlCastFunc::LayoutInt()) || 0 == strcmp(code, ECSqlCastFunc::LayoutLong()))
        sw.Int64(v.GetValueInt64());
    else if (0 == strcmp(code, ECSqlCastFunc::LayoutId()))
        WriteId(sw, v.GetValueUInt64());
    else if (0 == strcmp(code, ECSqlCastFunc::LayoutDouble()))
        {
        const double d = v.GetValueDouble();
        if (!std::isfinite(d))
            {
            isNull = true;
            return true;
            }
        sw.Double(d);
        }
    else if (0 == strcmp(code, ECSqlCastFunc::LayoutBool()))
        sw.Bool(v.GetValueInt64() != 0);
    else if (0 == strcmp(code, ECSqlCastFunc::LayoutString()))
        sw.String(v.GetValueText());
    else if (0 == strcmp(code, ECSqlCastFunc::LayoutBinary()))
        {
        Utf8String base64;
        Base64Utilities::Encode(base64, (Byte const*) v.GetValueBlob(), (size_t) v.GetValueBytes());
        sw.String(base64.c_str());
        }
    else if (0 == strcmp(code, ECSqlCastFunc::LayoutGuid()))
        {
        Utf8String str;
        if (!GuidBlobToText(str, v.GetValueBlob(), v.GetValueBytes()))
            {
            isNull = true;
            return true;
            }
        sw.String(str.c_str());
        }
    else if (0 == strcmp(code, ECSqlCastFunc::LayoutGeometry()))
        {
        Utf8String json;
        if (!GeometryBlobToJson(json, v.GetValueBlob(), v.GetValueBytes()) || !WriteRawJson(sw, json))
            {
            isNull = true;
            return true;
            }
        }
    else if (ParseDateTimeLayout(dtInfo, code))
        {
        Utf8String str;
        if (v.GetValueType() == DbValueType::TextVal)
            str = v.GetValueText();
        else if (!JulianDayToText(str, v.GetValueDouble(), dtInfo))
            {
            isNull = true;
            return true;
            }
        sw.String(str.c_str());
        }
    else
        return false;

    if (writeMemberKey)
        w.Key(memberName);

    return w.RawValue(buf.GetString(), buf.GetSize(), rapidjson::kNullType);
    }

//---------------------------------------------------------------------------------------
// Renders an element of the internal JSON array representation
//---------------------------------------------------------------------------------------
bool ColumnRenderer::RenderElement(JsonWriter& w, RapidJsonValueCR layout, RapidJsonValueCR element)
    {
    if (element.IsNull())
        return w.Null();

    if (layout.IsObject() && layout.HasMember("m"))
        {
        if (!element.IsObject())
            return w.Null();

        RapidJsonValueCR members = layout["m"];
        w.StartObject();
        for (rapidjson::SizeType i = 0; i < members.Size(); i++)
            {
            Utf8CP name = members[i][0].GetString();
            auto it = element.FindMember(name);
            if (it == element.MemberEnd() || it->value.IsNull())
                continue;

            w.Key(name);
            if (!RenderElement(w, members[i][1], it->value))
                return false;
            }
        return w.EndObject();
        }

    if (layout.IsObject() && layout.HasMember("a"))
        {
        if (!element.IsArray())
            return w.Null();

        w.StartArray();
        for (rapidjson::SizeType i = 0; i < element.Size(); i++)
            {
            if (!RenderElement(w, layout["a"], element[i]))
                return false;
            }
        return w.EndArray();
        }

    if (!layout.IsString())
        return false;

    Utf8CP code = layout.GetString();
    const bool isPoint2d = 0 == strcmp(code, ECSqlCastFunc::LayoutPoint2d());
    if (isPoint2d || 0 == strcmp(code, ECSqlCastFunc::LayoutPoint3d()))
        {
        static Utf8CP const s_names[] = {"x", "y", "z"};
        const int dims = isPoint2d ? 2 : 3;
        double coords[3];
        if (!element.IsObject())
            return w.Null();

        for (int i = 0; i < dims; i++)
            {
            auto it = element.FindMember(s_names[i]);
            if (it == element.MemberEnd() || !it->value.IsNumber())
                return w.Null();
            coords[i] = it->value.GetDouble();
            }

        rapidjson::StringBuffer buf;
        JsonWriter sw(buf);
        if (!WritePoint(sw, coords, dims))
            return w.Null();

        return w.RawValue(buf.GetString(), buf.GetSize(), rapidjson::kObjectType);
        }

    DateTime::Info dtInfo;
    if (0 == strcmp(code, ECSqlCastFunc::LayoutId()))
        {
        if (element.IsUint64())
            WriteId(w, element.GetUint64());
        else if (element.IsInt64())
            WriteId(w, (uint64_t) element.GetInt64());
        else
            return w.Null();
        return true;
        }

    if (0 == strcmp(code, ECSqlCastFunc::LayoutGuid()) || 0 == strcmp(code, ECSqlCastFunc::LayoutGeometry()))
        {
        if (!element.IsString())
            return w.Null();

        bvector<Byte> bytes;
        Base64Utilities::Decode(bytes, element.GetString(), (size_t) element.GetStringLength());
        Utf8String str;
        if (0 == strcmp(code, ECSqlCastFunc::LayoutGuid()))
            {
            if (!GuidBlobToText(str, bytes.data(), (int) bytes.size()))
                return w.Null();
            return w.String(str.c_str());
            }

        if (!GeometryBlobToJson(str, bytes.data(), (int) bytes.size()))
            return w.Null();

        rapidjson::Document doc;
        if (doc.Parse<rapidjson::kParseIterativeFlag>(str.c_str()).HasParseError())
            return w.Null();
        return doc.Accept(w);
        }

    if (ParseDateTimeLayout(dtInfo, code))
        {
        Utf8String str;
        if (element.IsString())
            return w.String(element.GetString());

        if (!element.IsNumber() || !JulianDayToText(str, element.GetDouble(), dtInfo))
            return w.Null();
        return w.String(str.c_str());
        }

    if (0 == strcmp(code, ECSqlCastFunc::LayoutDouble()) && element.IsNumber() && !std::isfinite(element.GetDouble()))
        return w.Null();

    //int, long, double, bool, string, binary (base64 string) are stored in their JSON representation already
    if (element.IsObject() || element.IsArray())
        return w.Null();

    return element.Accept(w);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ColumnRenderer::RenderArray(JsonWriter& w, RapidJsonValueCR elementLayout, Utf8CP internalJson)
    {
    rapidjson::Document doc;
    if (doc.Parse<rapidjson::kParseIterativeFlag>(internalJson).HasParseError() || !doc.IsArray())
        return false;

    w.StartArray();
    for (rapidjson::SizeType i = 0; i < doc.Size(); i++)
        {
        if (!RenderElement(w, elementLayout, doc[i]))
            return false;
        }
    return w.EndArray();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SetResultText(DbFunction::Context& ctx, Utf8CP str, size_t len)
    {
    ctx.SetResultText(str, (int) len, DbFunction::Context::CopyData::Yes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool IsValidLayout(RapidJsonValueCR layout, int depth)
    {
    // layouts are generated by ECDb; the depth limit only guards against malformed input
    if (depth > 64)
        return false;

    if (layout.IsString())
        return true;

    if (!layout.IsObject())
        return false;

    auto arr = layout.FindMember("a");
    if (arr != layout.MemberEnd())
        return IsValidLayout(arr->value, depth + 1);

    auto members = layout.FindMember("m");
    if (members == layout.MemberEnd() || !members->value.IsArray())
        return false;

    for (RapidJsonValueCR member : members->value.GetArray())
        {
        if (!member.IsArray() || member.Size() != 2 || !member[0].IsString() || !IsValidLayout(member[1], depth + 1))
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool TryParseLayout(rapidjson::Document& layout, Utf8CP text)
    {
    return !layout.Parse<rapidjson::kParseIterativeFlag>(text).HasParseError() && IsValidLayout(layout, 0);
    }

//---------------------------------------------------------------------------------------
// Layouts are constant per statement, so parsed layouts are cached per thread instead of
// being re-parsed on every row. Returns nullptr if the layout is invalid.
// @bsimethod
//---------------------------------------------------------------------------------------
rapidjson::Value const* GetCachedLayout(DbValue const& v)
    {
    if (v.IsNull() || v.GetValueType() != DbValueType::TextVal)
        return nullptr;

    struct Entry
        {
        Utf8String m_text;
        std::unique_ptr<rapidjson::Document> m_layout; // nullptr if invalid
        };

    static constexpr size_t s_maxEntries = 16;
    thread_local std::vector<Entry> s_cache;
    thread_local size_t s_next = 0;

    Utf8CP text = v.GetValueText();
    for (Entry const& e : s_cache)
        {
        if (e.m_text.Equals(text))
            return e.m_layout.get();
        }

    auto layout = std::make_unique<rapidjson::Document>();
    if (!TryParseLayout(*layout, text))
        layout = nullptr;

    Entry entry {Utf8String(text), std::move(layout)};
    rapidjson::Value const* result = entry.m_layout.get();
    if (s_cache.size() < s_maxEntries)
        s_cache.push_back(std::move(entry));
    else
        {
        s_cache[s_next] = std::move(entry);
        s_next = (s_next + 1) % s_maxEntries;
        }

    return result;
    }

//---------------------------------------------------------------------------------------
// text_pt is called once per coordinate with the same text, so the last parsed point is
// cached per thread. Returns false if the text is not a valid point of the given dimension.
// @bsimethod
//---------------------------------------------------------------------------------------
bool ParsePointText(Utf8CP text, int dims, double (&coords)[3])
    {
    thread_local Utf8String s_text;
    thread_local int s_dims = 0;
    thread_local bool s_valid = false;
    thread_local double s_coords[3] = {0, 0, 0};

    if (s_dims == dims && s_text.Equals(text))
        {
        std::copy(std::begin(s_coords), std::end(s_coords), std::begin(coords));
        return s_valid;
        }

    bool valid = false;
    double parsed[3] = {0, 0, 0};
    rapidjson::Document doc;
    if (!doc.Parse<rapidjson::kParseIterativeFlag>(text).HasParseError() && doc.IsObject())
        {
        // the whole point is invalid if any required coordinate is invalid
        static Utf8CP const s_upper[] = {"X", "Y", "Z"};
        static Utf8CP const s_lower[] = {"x", "y", "z"};
        valid = true;
        for (int i = 0; i < dims; i++)
            {
            auto it = doc.FindMember(s_upper[i]);
            if (it == doc.MemberEnd())
                it = doc.FindMember(s_lower[i]);

            if (it == doc.MemberEnd() || !it->value.IsNumber() || !std::isfinite(it->value.GetDouble()))
                {
                valid = false;
                break;
                }

            parsed[i] = it->value.GetDouble();
            }
        }

    s_text.assign(text);
    s_dims = dims;
    s_valid = valid;
    std::copy(std::begin(parsed), std::end(parsed), std::begin(s_coords));
    std::copy(std::begin(parsed), std::end(parsed), std::begin(coords));
    return valid;
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
ECSqlCastFunc& ECSqlCastFunc::GetSingleton()
    {
    static ECSqlCastFunc s_singleton;
    return s_singleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
Utf8CP ECSqlCastFunc::LayoutDateTime(DateTime::Info const& info)
    {
    if (!info.IsValid())
        return "dtN";

    switch (info.GetComponent())
        {
            case DateTime::Component::Date:
                return "dtD";
            case DateTime::Component::TimeOfDay:
                return "dtT";
            default:
                break;
        }

    switch (info.GetKind())
        {
            case DateTime::Kind::Utc:
                return "dtU";
            case DateTime::Kind::Local:
                return "dtL";
            default:
                return "dtN";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECSqlCastFunc::_ComputeScalar(Context& ctx, int nArgs, DbValue* args)
    {
    if (nArgs < 2 || args[0].GetValueType() != DbValueType::TextVal)
        {
        ctx.SetResultError(SQLFUNC_ECSqlCast " expects an operation name and at least one argument.");
        return;
        }

    Utf8CP op = args[0].GetValueText();
    DbValue const& v = args[1];

    // multi-value operations handle NULLs themselves
    if (0 == strcmp(op, "struct_text"))
        {
        rapidjson::Value const* layout = GetCachedLayout(v);
        if (layout == nullptr)
            {
            ctx.SetResultError(SQLFUNC_ECSqlCast ": invalid struct layout.");
            return;
            }

        rapidjson::StringBuffer buf;
        JsonWriter w(buf);
        ColumnRenderer renderer(args, nArgs, 2);
        bool isNull = false;
        if (!renderer.Render(w, isNull, *layout, false, nullptr) || renderer.GetPosition() != nArgs)
            {
            ctx.SetResultError(SQLFUNC_ECSqlCast ": struct value does not match its layout.");
            return;
            }

        if (isNull)
            ctx.SetResultNull();
        else
            SetResultText(ctx, buf.GetString(), buf.GetSize());
        return;
        }

    if (0 == strcmp(op, "pt_text"))
        {
        const int dims = nArgs - 1;
        if (dims != 2 && dims != 3)
            {
            ctx.SetResultError(SQLFUNC_ECSqlCast ": pt_text expects 2 or 3 coordinates.");
            return;
            }

        double coords[3];
        for (int i = 0; i < dims; i++)
            {
            if (args[1 + i].IsNull())
                {
                ctx.SetResultNull();
                return;
                }
            coords[i] = args[1 + i].GetValueDouble();
            }

        rapidjson::StringBuffer buf;
        JsonWriter w(buf);
        if (!WritePoint(w, coords, dims))
            {
            ctx.SetResultNull();
            return;
            }

        SetResultText(ctx, buf.GetString(), buf.GetSize());
        return;
        }

    if (0 == strcmp(op, "nav_text"))
        {
        if (v.IsNull())
            {
            ctx.SetResultNull();
            return;
            }

        rapidjson::StringBuffer buf;
        JsonWriter w(buf);
        w.StartObject();
        w.Key("Id");
        WriteId(w, v.GetValueUInt64());
        if (nArgs > 2 && !args[2].IsNull())
            {
            w.Key("RelECClassId");
            WriteId(w, args[2].GetValueUInt64());
            }
        w.EndObject();
        SetResultText(ctx, buf.GetString(), buf.GetSize());
        return;
        }

    if (0 == strcmp(op, "arr_text"))
        {
        rapidjson::Value const* layout = nArgs == 3 ? GetCachedLayout(v) : nullptr;
        if (layout == nullptr)
            {
            ctx.SetResultError(SQLFUNC_ECSqlCast ": invalid array layout.");
            return;
            }

        DbValue const& arr = args[2];
        if (arr.IsNull() || arr.GetValueType() != DbValueType::TextVal)
            {
            ctx.SetResultNull();
            return;
            }

        rapidjson::StringBuffer buf;
        JsonWriter w(buf);
        if (!ColumnRenderer::RenderArray(w, *layout, arr.GetValueText()))
            {
            ctx.SetResultNull();
            return;
            }

        SetResultText(ctx, buf.GetString(), buf.GetSize());
        return;
        }

    if (v.IsNull())
        {
        ctx.SetResultNull();
        return;
        }

    const DbValueType type = v.GetValueType();
    // These parsers consume C strings; do not let an embedded NUL hide trailing input.
    if (type == DbValueType::TextVal && memchr(v.GetValueText(), '\0', v.GetValueBytes()) != nullptr &&
        (0 == strcmp(op, "id") || 0 == strcmp(op, "json") || 0 == strcmp(op, "guid") ||
         0 == strcmp(op, "text_bool") || 0 == strcmp(op, "text_dt") || 0 == strcmp(op, "text_pt") || 0 == strcmp(op, "enum")))
        {
        ctx.SetResultNull();
        return;
        }

    if (0 == strcmp(op, "id"))
        {
        if (type == DbValueType::IntegerVal)
            {
            ctx.SetResultInt64(v.GetValueInt64());
            return;
            }

        uint64_t id = 0;
        if (type == DbValueType::TextVal && ParseIdText(id, v.GetValueText()))
            ctx.SetResultInt64((int64_t) id);
        else
            ctx.SetResultNull();
        return;
        }

    if (0 == strcmp(op, "id_text"))
        {
        if (type != DbValueType::IntegerVal)
            {
            ctx.SetResultValue(v);
            return;
            }

        Utf8String str;
        str.Sprintf("%" PRIu64, (uint64_t) v.GetValueInt64());
        SetResultText(ctx, str.c_str(), str.size());
        return;
        }

    if (0 == strcmp(op, "json"))
        {
        if (type != DbValueType::TextVal)
            {
            ctx.SetResultNull();
            return;
            }

        rapidjson::Reader reader;
        rapidjson::BaseReaderHandler<> handler;
        rapidjson::StringStream stream(v.GetValueText());
        if (reader.Parse<rapidjson::kParseIterativeFlag>(stream, handler).IsError())
            ctx.SetResultNull();
        else
            ctx.SetResultValue(v);
        return;
        }

    if (0 == strcmp(op, "guid"))
        {
        if (type == DbValueType::BlobVal && v.GetValueBytes() == (int) sizeof(BeGuid))
            {
            ctx.SetResultValue(v);
            return;
            }

        BeGuid guid;
        if (type == DbValueType::TextVal && guid.FromString(v.GetValueText()) == SUCCESS)
            ctx.SetResultBlob(&guid, sizeof(BeGuid), Context::CopyData::Yes);
        else
            ctx.SetResultNull();
        return;
        }

    if (0 == strcmp(op, "guid_text"))
        {
        Utf8String str;
        if (type == DbValueType::BlobVal && GuidBlobToText(str, v.GetValueBlob(), v.GetValueBytes()))
            SetResultText(ctx, str.c_str(), str.size());
        else
            ctx.SetResultNull();
        return;
        }

    if (0 == strcmp(op, "base64"))
        {
        if (type != DbValueType::BlobVal)
            {
            ctx.SetResultValue(v);
            return;
            }

        Utf8String base64;
        Base64Utilities::Encode(base64, (Byte const*) v.GetValueBlob(), (size_t) v.GetValueBytes());
        SetResultText(ctx, base64.c_str(), base64.size());
        return;
        }

    if (0 == strcmp(op, "bool_text"))
        {
        if (type == DbValueType::TextVal)
            {
            ctx.SetResultValue(v);
            return;
            }

        Utf8CP str = v.GetValueDouble() != 0.0 ? "true" : "false";
        SetResultText(ctx, str, strlen(str));
        return;
        }

    if (0 == strcmp(op, "text_bool"))
        {
        if (type == DbValueType::IntegerVal || type == DbValueType::FloatVal)
            {
            ctx.SetResultInt(v.GetValueDouble() != 0.0 ? 1 : 0);
            return;
            }

        if (type != DbValueType::TextVal)
            {
            ctx.SetResultNull();
            return;
            }

        Utf8String str(v.GetValueText());
        str.Trim();
        if (str.EqualsIAscii("true"))
            ctx.SetResultInt(1);
        else if (str.EqualsIAscii("false"))
            ctx.SetResultInt(0);
        else
            {
            // numeric text keeps the legacy semantics (non-zero is true), anything else is invalid
            char* end = nullptr;
            const double d = str.empty() ? 0.0 : strtod(str.c_str(), &end);
            if (str.empty() || end == nullptr || *end != '\0' || !std::isfinite(d))
                ctx.SetResultNull();
            else
                ctx.SetResultInt(d != 0.0 ? 1 : 0);
            }
        return;
        }

    if (0 == strcmp(op, "dt_text"))
        {
        if (type == DbValueType::TextVal)
            {
            ctx.SetResultValue(v);
            return;
            }

        DateTime::Info info = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified);
        if (nArgs > 2 && args[2].GetValueType() == DbValueType::TextVal)
            ParseDateTimeLayout(info, args[2].GetValueText());

        Utf8String str;
        if (type != DbValueType::BlobVal && JulianDayToText(str, v.GetValueDouble(), info))
            SetResultText(ctx, str.c_str(), str.size());
        else
            ctx.SetResultNull();
        return;
        }

    if (0 == strcmp(op, "text_dt"))
        {
        if (type == DbValueType::IntegerVal || type == DbValueType::FloatVal)
            {
            ctx.SetResultValue(v);
            return;
            }

        if (type != DbValueType::TextVal)
            {
            ctx.SetResultNull();
            return;
            }

        DateTime dt = DateTime::FromString(v.GetValueText());
        double jd = 0;
        if (!dt.IsValid() || dt.ToJulianDay(jd) != SUCCESS)
            ctx.SetResultNull();
        else
            ctx.SetResultDouble(jd);
        return;
        }

    if (0 == strcmp(op, "geom_text"))
        {
        Utf8String json;
        if (type == DbValueType::BlobVal && GeometryBlobToJson(json, v.GetValueBlob(), v.GetValueBytes()))
            SetResultText(ctx, json.c_str(), json.size());
        else
            ctx.SetResultNull();
        return;
        }

    if (0 == strcmp(op, "text_pt"))
        {
        // _ecsql_cast('text_pt', text, coordinateIndex, dimensions)
        if (nArgs != 4 || type != DbValueType::TextVal)
            {
            ctx.SetResultNull();
            return;
            }

        const int index = args[2].GetValueInt();
        const int dims = args[3].GetValueInt();
        double coords[3];
        if (index < 0 || index >= dims || dims < 2 || dims > 3 || !ParsePointText(v.GetValueText(), dims, coords))
            {
            ctx.SetResultNull();
            return;
            }

        ctx.SetResultDouble(coords[index]);
        return;
        }

    if (0 == strcmp(op, "enum"))
        {
        // _ecsql_cast('enum', value, enumeratorValue1, ...): NULL if value is not one of the enumerator values
        for (int i = 2; i < nArgs; i++)
            {
            DbValue const& e = args[i];
            if (e.GetValueType() == DbValueType::IntegerVal)
                {
                int64_t intValue = 0;
                bool isInt = false;
                if (type == DbValueType::IntegerVal)
                    {
                    intValue = v.GetValueInt64();
                    isInt = true;
                    }
                else if (type == DbValueType::FloatVal)
                    {
                    const double d = v.GetValueDouble();
                    isInt = std::isfinite(d) && d == std::trunc(d) && std::fabs(d) < 9.2e18;
                    intValue = isInt ? (int64_t) d : 0;
                    }
                else if (type == DbValueType::TextVal)
                    {
                    Utf8String str(v.GetValueText());
                    str.Trim();
                    char* end = nullptr;
                    errno = 0;
                    intValue = str.empty() ? 0 : strtoll(str.c_str(), &end, 10);
                    isInt = !str.empty() && end != nullptr && *end == '\0' && errno == 0;
                    }

                if (isInt && intValue == e.GetValueInt64())
                    {
                    ctx.SetResultInt64(intValue);
                    return;
                    }
                }
            else if (e.GetValueType() == DbValueType::TextVal && type == DbValueType::TextVal && 0 == strcmp(v.GetValueText(), e.GetValueText()))
                {
                ctx.SetResultValue(v);
                return;
                }
            }

        ctx.SetResultNull();
        return;
        }

    Utf8PrintfString err(SQLFUNC_ECSqlCast ": unknown operation '%s'.", op);
    ctx.SetResultError(err.c_str());
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
