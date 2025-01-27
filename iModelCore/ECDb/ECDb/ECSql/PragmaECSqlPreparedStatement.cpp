/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//================================================================================
// @bsiclass PragmaHelp
//================================================================================
struct PragmaHelp : PragmaManager::GlobalHandler {
    PragmaManager& m_mgr;
    PragmaHelp(PragmaManager& mgr):GlobalHandler("help","return list of pragma supported"),m_mgr(mgr){}
    ~PragmaHelp(){}

    virtual DbResult Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const&)  override {
            auto result = std::make_unique<StaticPragmaResult>(ecdb);
            result->AppendProperty("pragma", PRIMITIVETYPE_String);
            result->AppendProperty("type", PRIMITIVETYPE_String);
            result->AppendProperty("descr", PRIMITIVETYPE_String);
            result->FreezeSchemaChanges();
            for(auto& handlerType: m_mgr.m_handlers) {
                for (auto& handler : handlerType.second) {
                    auto row = result->AppendRow();
                    row.appendValue() = handler.second->GetName();
                    row.appendValue() = handler.second->GetTypeString();
                    row.appendValue() = handler.second->GetDescription();
                }
            }
            rowSet = std::move(result);
            return BE_SQLITE_OK;
    }

    virtual DbResult Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, PragmaManager::OptionsMap const&) override {
        ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0552, "PRAGMA %s is readonly.", GetName().c_str());
        rowSet = std::make_unique<StaticPragmaResult>(ecdb);
        rowSet->FreezeSchemaChanges();
        return BE_SQLITE_READONLY;
    }
    static std::unique_ptr<PragmaManager::Handler> Create (PragmaManager& mgr) { return std::make_unique<PragmaHelp>(mgr); }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void PragmaManager::InitSystemPragmas() {
    Register(PragmaHelp::Create(*this));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool PragmaManager::Handler::isExperimentalFeatureAllowed(ECDbCR conn, PragmaManager::OptionsMap const& map) const {
    return conn.GetECSqlConfig().GetExperimentalFeaturesEnabled() || map.find(OptionsExp::ENABLE_EXPERIMENTAL_FEATURES) != map.end();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8CP PragmaManager::Handler::GetTypeString() const {
    if (m_type == Type::Any) return "any";
    if (m_type == Type::Class) return "class";
    if (m_type == Type::Global) return "global";
    if (m_type == Type::Property) return "property";
    if (m_type == Type::Schema) return "schema";
    BeAssert(false);
    return nullptr;
 }

//================================================================================
//StaticPragmaResult
//================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BeJsValue StaticPragmaResult::AppendRow() {
    BeMutexHolder lock(GetMutex());
    if (GetColumnCount() ==0) {
        throw std::runtime_error("no columns added");
    }
    return m_doc.appendArray();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult StaticPragmaResult::_Step() {
    const auto maxRows = (int)m_doc.size();
    m_row = nullptr;
    if ((m_curRow == -1 && maxRows == 0) || (m_curRow >= maxRows)) {
        return BE_SQLITE_DONE;
    }
    if (++m_curRow >= maxRows) {
        return BE_SQLITE_DONE;
    }
    m_row = std::make_unique<BeJsValue>(m_doc[m_curRow]);
    return BE_SQLITE_ROW;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult StaticPragmaResult::_Reset() {
    m_curRow = -1;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult StaticPragmaResult::_Init() {
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BeJsValue* StaticPragmaResult::_CurrentRow() {
    return m_row.get();
}

//================================================================================
//PragmaManager
//================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaManager::DefaultGlobal(RowSet&, Utf8StringCR name, PragmaVal const&, Operation) const {
    GetECDb().GetImpl().Issues().ReportV(
        IssueSeverity::Error,
        IssueCategory::BusinessProperties,
        IssueType::ECSQL,
        ECDbIssueId::ECDb_0559,
        "Unrecognized pragma %s.",
        name.c_str()
    );
    return BE_SQLITE_ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaManager::DefaultSchema(RowSet&, Utf8StringCR name, PragmaVal const&, Operation, ECN::ECSchemaCR) const{
    GetECDb().GetImpl().Issues().ReportV(
        IssueSeverity::Error,
        IssueCategory::BusinessProperties,
        IssueType::ECSQL,
        ECDbIssueId::ECDb_0560,
        "Unrecognized pragma '%s' for schema object.",
        name.c_str()
    );

    return BE_SQLITE_ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaManager::DefaultClass(RowSet&, Utf8StringCR name, PragmaVal const&, Operation, ECN::ECClassCR) const{
    GetECDb().GetImpl().Issues().ReportV(
        IssueSeverity::Error,
        IssueCategory::BusinessProperties,
        IssueType::ECSQL,
        ECDbIssueId::ECDb_0561,
        "Unrecognized pragma '%s' for class object.",
        name.c_str()
    );
    return BE_SQLITE_ERROR;

}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaManager::DefaultProperty(RowSet&, Utf8StringCR name, PragmaVal const&, Operation, ECN::ECPropertyCR) const{
    GetECDb().GetImpl().Issues().ReportV(
        IssueSeverity::Error,
        IssueCategory::BusinessProperties,
        IssueType::ECSQL,
        ECDbIssueId::ECDb_0562,
        "Unrecognized pragma '%s' for property object.",
        name.c_str()
    );
    return BE_SQLITE_ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
PragmaManager::Handler* PragmaManager::GetHandler(Handler::Type type, Utf8StringCR name) const {
    auto tId = m_handlers.find(type);
    if (tId != m_handlers.end()) {
        auto& handlers = tId->second;
        auto it = handlers.find(name.c_str());
        if (it != handlers.end()) {
            return it->second.get();
        }
    }
    return nullptr;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaManager::DefaultAny(RowSet&, Utf8StringCR name, PragmaVal const&, Operation, std::vector<Utf8String> const& path) const{
    GetECDb().GetImpl().Issues().ReportV(
        IssueSeverity::Error,
        IssueCategory::BusinessProperties,
        IssueType::ECSQL,
        ECDbIssueId::ECDb_0563,
        "Unrecognized object path '%s' specified for pragma '%s'",
        BeStringUtilities::Join(path,".").c_str(),
        name.c_str()
    );
    return BE_SQLITE_ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus PragmaManager::Register(std::unique_ptr<Handler> handler) {
    if (handler == nullptr) {
        BeAssert(false);
        return ERROR;
    }
    auto type = handler->GetType();
    auto &name = handler->GetName();
    BeMutexHolder holder(m_ecdb.GetImpl().GetMutex());
    if (GetHandler(type, name) != nullptr) {
        GetECDb().GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECSQL,
            ECDbIssueId::ECDb_0564,
            "ECDb pragma handler with same type (%s) and name (%s) already exists.",
            handler->GetTypeString(),
            name.c_str()
        );
        return ERROR;
    }
    m_handlers[type][name.c_str()] = std::move(handler);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaManager::PrepareGlobal(RowSet& rowSet, Utf8StringCR name, PragmaVal const& val, Operation op, PragmaManager::OptionsMap const& options) const {
    auto handler = GetHandlerAs<GlobalHandler>(Handler::Type::Global, name);
    if (handler != nullptr) {
        if (op == Operation::Read) {
            return handler->Read(rowSet, GetECDb(), val, options);
        }
        return handler->Write(rowSet, GetECDb(), val, options);
    }
    return DefaultGlobal(rowSet, name, val, op);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaManager::PrepareSchema(RowSet& rowSet, Utf8StringCR name, PragmaVal const& val, Operation op, ECN::ECSchemaCR schema, PragmaManager::OptionsMap const& options) const{
    auto handler = GetHandlerAs<SchemaHandler>(Handler::Type::Schema, name);
    if (handler != nullptr) {
        if (op == Operation::Read) {
            return handler->Read(rowSet, GetECDb(), val, schema, options);
        }
        return handler->Write(rowSet, GetECDb(), val, schema, options);
    }
    return DefaultSchema(rowSet, name, val, op, schema);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaManager::PrepareClass(RowSet& rowSet, Utf8StringCR name, PragmaVal const& val, Operation op, ECN::ECClassCR cls, PragmaManager::OptionsMap const& options) const {
    auto handler = GetHandlerAs<ClassHandler>(Handler::Type::Class, name);
    if (handler != nullptr) {
        if (op == Operation::Read) {
            return handler->Read(rowSet, GetECDb(), val, cls, options);
        }
        return handler->Write(rowSet, GetECDb(), val, cls, options);
    }

    return DefaultClass(rowSet, name, val, op, cls);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaManager::Prepare(RowSet& rowset,PragmaStatementExp const& exp) const {
    OptionsMap optionsMap;
    if (auto opts = exp.GetOptions()) {
        optionsMap = exp.GetOptions()->GetOptionMap();
    }
    return Prepare(rowset, exp.GetName(), exp.GetValue(), exp.IsReadValue() ? Operation::Read : Operation::Write, exp.GetPathTokens(), optionsMap);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaManager::PrepareProperty(RowSet& rowSet, Utf8StringCR name, PragmaVal const& val, Operation op, ECN::ECPropertyCR prop, PragmaManager::OptionsMap const& options) const {
    auto handler = GetHandlerAs<PropertyHandler>(Handler::Type::Property, name);
    if (handler != nullptr) {
        if (op == Operation::Read) {
            return handler->Read(rowSet, GetECDb(), val, prop, options);
        }
        return handler->Write(rowSet, GetECDb(), val, prop, options);
    }
    return DefaultProperty(rowSet, name, val, op, prop);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaManager::PrepareAny(RowSet& rowSet, Utf8StringCR name, PragmaVal const& val, Operation op, std::vector<Utf8String> const& path, PragmaManager::OptionsMap const& options) const {
    auto handler = GetHandlerAs<AnyHandler>(Handler::Type::Any, name);
    if (handler != nullptr) {
        if (op == Operation::Read) {
            return handler->Read(rowSet, GetECDb(), val, path, options);
        }
        return handler->Write(rowSet, GetECDb(), val, path, options);
    }
    return DefaultAny(rowSet, name, val, op, path);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult  PragmaManager::Prepare(RowSet& rowset, Utf8StringCR name, PragmaVal const& val, Operation op, std::vector<Utf8String> const& path, PragmaManager::OptionsMap const& options) const {
    ECN::ECSchemaCP schemaP = nullptr;
    ECN::ECClassCP classP = nullptr;
    ECN::ECPropertyCP propertyP = nullptr;

    if (path.empty()) {
        return PrepareGlobal(rowset, name, val, op, options);
    }
    if (path.size() >= 4) {
        return PrepareAny(rowset,name, val, op, path, options);
    }
    if (path.size() >= 1) {
        schemaP = m_ecdb.Schemas().GetSchema(path[0], false, SchemaLookupMode::AutoDetect);
        if (schemaP == nullptr) {
            return PrepareAny(rowset,name, val, op, path, options);
        }
    }
    if (path.size() >= 2) {
        classP = m_ecdb.Schemas().GetClass(path[0], path[1], SchemaLookupMode::AutoDetect);
        if (classP == nullptr) {
            return PrepareAny(rowset,name, val, op, path, options);
        }
    }
    if (path.size() >= 3) {
        propertyP = classP->GetPropertyP(path[2]);
        if (propertyP == nullptr) {
            return PrepareAny(rowset,name, val, op, path, options);
        }
    }
    if (propertyP && classP && schemaP) {
        return PrepareProperty(rowset,name, val, op, *propertyP, options);
    } else if (!propertyP && classP && schemaP) {
        return PrepareClass(rowset,name, val, op, *classP, options);
    } else if (!propertyP && !classP && schemaP) {
        return PrepareSchema(rowset,name, val, op, *schemaP, options);
    }
    return PrepareAny(rowset,name, val, op, path, options);
}

//=======================================================================================
// PragmaResult
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaResult::Step() {
    BeMutexHolder lock(m_mutex);
    if (m_allowSchemaChange) {
        return BE_SQLITE_SCHEMA;
    }
    if (m_columns.empty()) {
        return BE_SQLITE_DONE;
    }
    if (!m_init) {
        const auto rc = _Init();
        if (rc != BE_SQLITE_OK) {
            return rc;
        }
        m_init = true;
    }
    return _Step();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaResult::Reset() {
    BeMutexHolder lock(m_mutex);
    if (m_allowSchemaChange) {
        return BE_SQLITE_SCHEMA;
    }

    const auto rc  = _Reset();
    if (rc == BE_SQLITE_OK) {
        m_init = false;
    }
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int PragmaResult::GetColumnCount() const {
    BeMutexHolder lock(m_mutex);
    return (int)m_columns.size();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void PragmaResult::FreezeSchemaChanges()  {
    BeMutexHolder lock(m_mutex);
    m_allowSchemaChange = false;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool PragmaResult::IsSchemaChangesAllowed() const {
    BeMutexHolder lock(m_mutex);
    return m_allowSchemaChange;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlValue const& PragmaResult::GetValue(int columnIndex) const {
    BeMutexHolder lock(m_mutex);
    if (columnIndex< 0 || columnIndex > GetColumnCount() - 1) {
        return NoopECSqlValue::GetSingleton();
    }
    return *m_columns[columnIndex];
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECN::ECPropertyCP PragmaResult::AppendProperty(Utf8StringCR name, ECN::PrimitiveType type) {
    BeMutexHolder lock(m_mutex);
    if (!m_allowSchemaChange) {
        BeAssert(false && "schema changes are not allowed");
        return nullptr;
    }
    ECN::PrimitiveECPropertyP property = nullptr;
    if (type == ECN::PRIMITIVETYPE_Boolean ||
        type == ECN::PRIMITIVETYPE_Double ||
        type == ECN::PRIMITIVETYPE_Integer||
        type == ECN::PRIMITIVETYPE_Long ||
        type == ECN::PRIMITIVETYPE_String) {
        if (m_class->CreatePrimitiveProperty(property, name, type) != ECObjectsStatus::Success)
            return nullptr;
    } else {
        BeAssert(false && "unsupported type. Only bool, double, integer, long and string are supported type");
        return nullptr;
    }
    DateTime::Info dateTimeInfo;
    ECSqlPropertyPath path;
    path.AddEntry(*property);
    ECSqlColumnInfo::RootClass rootClass(*m_class, "");
    ECSqlColumnInfo col(ECN::ECTypeDescriptor::CreatePrimitiveTypeDescriptor(property->GetType()), dateTimeInfo, nullptr, property, property, false, true, std::move(path), rootClass);
    m_columns.push_back(std::make_unique<Field> (col, (int)m_columns.size(), *this));
    property->SetDisplayLabel(name);
    return property;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
PragmaResult::PragmaResult(ECDbCR ecdb): m_ecdb(ecdb), m_init(false),m_allowSchemaChange(true){
    ECN::ECSchema::CreateSchema(m_schema, "pragma", "pragma", 1, 0, 0);
    m_schema->CreateEntityClass(m_class, "result_set");
}

//=======================================================================================
//PragmaResult::Field
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool PragmaResult::Field::_IsNull() const {
    BeMutexHolder lock(m_result.GetMutex());
    auto row = m_result._CurrentRow();
    return row == nullptr ? NoopECSqlValue::GetSingleton().IsNull() : row->operator[](m_columnIndex).isNull();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool PragmaResult::Field::_GetBoolean() const {
    BeMutexHolder lock(m_result.GetMutex());
    auto row = m_result._CurrentRow();
    return row == nullptr ? NoopECSqlValue::GetSingleton().GetBoolean() : row->operator[](m_columnIndex).asBool();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
double PragmaResult::Field::_GetDouble() const {
    BeMutexHolder lock(m_result.GetMutex());
    auto row = m_result._CurrentRow();
    return row == nullptr ? NoopECSqlValue::GetSingleton().GetDouble() : row->operator[](m_columnIndex).asDouble();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int PragmaResult::Field::_GetInt() const {
    BeMutexHolder lock(m_result.GetMutex());
    auto row = m_result._CurrentRow();
    return row == nullptr ? NoopECSqlValue::GetSingleton().GetInt() : row->operator[](m_columnIndex).asInt();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int64_t PragmaResult::Field::_GetInt64() const {
    BeMutexHolder lock(m_result.GetMutex());
    auto row = m_result._CurrentRow();
    return row == nullptr ? NoopECSqlValue::GetSingleton().GetInt64() : row->operator[](m_columnIndex).asInt64();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8CP PragmaResult::Field::_GetText() const {
    BeMutexHolder lock(m_result.GetMutex());
    auto row = m_result._CurrentRow();
    return row == nullptr ? NoopECSqlValue::GetSingleton().GetText() : row->operator[](m_columnIndex).asCString();
}

// unsupported value type
void const* PragmaResult::Field::_GetBlob(int* blobSize) const { return NoopECSqlValue::GetSingleton().GetBlob(blobSize); }
uint64_t PragmaResult::Field::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const{ return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata); }
double PragmaResult::Field::_GetDateTimeJulianDays(DateTime::Info& metadata) const { return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata); }
DPoint2d PragmaResult::Field::_GetPoint2d() const { return NoopECSqlValue::GetSingleton().GetPoint2d(); }
DPoint3d PragmaResult::Field::_GetPoint3d() const { return NoopECSqlValue::GetSingleton().GetPoint3d(); }
IGeometryPtr PragmaResult::Field::_GetGeometry() const { return NoopECSqlValue::GetSingleton().GetGeometry(); }
IECSqlValue const& PragmaResult::Field::_GetStructMemberValue(Utf8CP memberName) const { return NoopECSqlValue::GetSingleton(); }
IECSqlValueIterable const& PragmaResult::Field::_GetStructIterable() const{ return NoopECSqlValue::GetSingleton().GetStructIterable(); }
int PragmaResult::Field::_GetArrayLength() const { return NoopECSqlValue::GetSingleton().GetArrayLength(); }
IECSqlValueIterable const& PragmaResult::Field::_GetArrayIterable() const{ return NoopECSqlValue::GetSingleton().GetArrayIterable(); }

//=======================================================================================
//PragmaECSqlPreparedStatement
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& PragmaECSqlPreparedStatement::_GetBinder(int parameterIndex) const {
    ECSqlBinder* binder = nullptr;
    const ECSqlStatus stat = m_parameterMap.TryGetBinder(binder, parameterIndex);

    if (stat.IsSuccess())
        return *binder;

    if (stat == ECSqlStatus::Error)
        LOG.errorv("Parameter index %d passed to ECSqlStatement binding API is out of bounds.", parameterIndex);

    return NoopECSqlBinder::Get();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int PragmaECSqlPreparedStatement::_GetParameterIndex(Utf8CP parameterName) const {
    int index = m_parameterMap.GetIndexForName(parameterName);
    if (index <= 0)
        LOG.errorv("No parameter index found for parameter name: %s.", parameterName);

    return index;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int PragmaECSqlPreparedStatement::_TryGetParameterIndex(Utf8CP parameterName) const {
    return m_parameterMap.GetIndexForName(parameterName); // do not log an error on a missing parameter
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PragmaECSqlPreparedStatement::_ClearBindings() {
    if (m_resultSet == nullptr) {
        return ECSqlStatus(BE_SQLITE_MISMATCH);
    }
    m_parameterMap.OnClearBindings();
    return ECSqlStatus::Success;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PragmaECSqlPreparedStatement::_Reset() {
    if (m_resultSet == nullptr) {
        return ECSqlStatus(BE_SQLITE_MISMATCH);
    }
    const auto rc = m_resultSet->Reset();
    if (rc != BE_SQLITE_OK)
        return ECSqlStatus(rc);

    if(!m_isFirstStep)
        m_isFirstStep = true; // Will reset the flag when actually everything will be reset successfully if flag is false
    return ECSqlStatus::Success;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PragmaECSqlPreparedStatement::_Prepare(ECSqlPrepareContext& ctx, Exp const& exp) {
    auto& pragmaExp = exp.GetAs<PragmaStatementExp>();
    const auto rc = ctx.GetECDb().GetImpl().GetPragmaManager().Prepare(m_resultSet, pragmaExp);
    if (rc != BE_SQLITE_OK)
        return ECSqlStatus(rc);

    if (m_resultSet == nullptr) {
        BeAssert(m_resultSet != nullptr && "Must be never nullptr for successful prepare");
        return ECSqlStatus::Error;
    }
    return ECSqlStatus::Success;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaECSqlPreparedStatement::DoStep() {
    if (m_resultSet == nullptr) {
        return BE_SQLITE_MISMATCH;
    }

    if (SUCCESS != AssertIsValid())
        return BE_SQLITE_ERROR;

    if(m_isFirstStep)
        {
        if (!m_parameterMap.OnBeforeFirstStep().IsSuccess())
            return BE_SQLITE_ERROR;
        }
    
    DbResult res = m_resultSet->Step();
    // if step actually succeeded and returned BE_SQLITE_DONE or BE_SQLITE_ROW on the sqlite side then we set this flag to false if flag is true, if the returned value is something else like BE_SQLITE_SCHEMA or anything else we don't set the flag to false
    if((res == BE_SQLITE_DONE || res == BE_SQLITE_ROW) && m_isFirstStep)
        m_isFirstStep = false; 
    return res;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaECSqlPreparedStatement::Step() {
    return DoStep();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int PragmaECSqlPreparedStatement::GetColumnCount() const {
    if (m_resultSet == nullptr) {
        return 0;
    }
        return m_resultSet->GetColumnCount();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlValue const& PragmaECSqlPreparedStatement::GetValue(int columnIndex) const {
    if (m_resultSet == nullptr) {
        return NoopECSqlValue::GetSingleton();
    }
    return m_resultSet->GetValue(columnIndex);
}

//=======================================================================================
//PragmaVal
//=======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int64_t PragmaVal::GetInteger() const {
    if (IsInteger()) {
        return m_integer;
    }
    if (IsDouble()) {
        return static_cast<int64_t>(GetDouble());
    }
    if (IsBool()) {
        return static_cast<int64_t>(GetBool());
    }
    if (IsString()) {
        return std::stoll(GetString());
    }
    return 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
double PragmaVal::GetDouble() const {
    if(IsDouble()) {
        return m_double;
    }
    if (IsInteger()) {
        return static_cast<double>(GetInteger());
    }
    if (IsBool()) {
        return GetBool() ? 1 : 0;
    }
    if (IsString()) {
        return std::stod(GetString());
    }
    return 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool PragmaVal::GetBool() const {
    if(IsBool()) {
        return m_bool;
    }
    if(IsDouble()) {
        return GetDouble() != 0;
    }
    if (IsInteger()) {
        return GetInteger() != 0;
    }
    if (IsString()) {
        auto str = GetString();
        if (BeStringUtilities::StricmpAscii(str.c_str(), "true") == 0) {
            return true;
        }
        if (BeStringUtilities::StricmpAscii(str.c_str(), "false") == 0) {
            return false;
        }
        return std::stol(str) != 0;
    }
    return false;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::string PragmaVal::GetString() const {
    if(IsString() || IsName()) {
        return m_str;
    }
    if(IsDouble()) {
        return std::to_string(GetDouble());
    }
    if (IsInteger()) {
        return std::to_string(GetInteger());
    }
    if (IsBool()) {
        return std::to_string(GetBool());
    }
    if (IsName()) {
        return GetName();
    }
    return "";
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::string PragmaVal::GetName() const {
    if(IsName()) {
        return m_str;
    }
    return "";
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
PragmaVal const& PragmaVal::Null() {
    static PragmaVal s_null(Type::Null);
    return s_null;
}

END_BENTLEY_SQLITE_EC_NAMESPACE

