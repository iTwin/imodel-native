/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <iostream>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PreparedECChangesetReader::PreparedECChangesetReader(ECDbCR ecdb)
    : m_ecdb(ecdb), m_currentChange(nullptr, false)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::OpenFile(Utf8StringCR changesetFile, bool invert, Mode mode) {
    if(IsOpen()) {
        LOG.errorv("Attempting to open a file on an already open PreparedECChangesetReader.");
        return BE_SQLITE_ERROR;
    }
    BeFileName input;
    input.AppendUtf8(changesetFile.c_str());

    if (!input.DoesPathExist())
        return BE_SQLITE_CANTOPEN;

    bvector<BeFileName> files{input};
    auto reader = std::make_unique<ChangesetFileReaderBase>(files);
    DdlChanges ddlChanges;
    bool hasSchemaChanges;
    reader->MakeReader()->GetSchemaChanges(hasSchemaChanges, ddlChanges);
    if (!ddlChanges._IsEmpty())
        m_ddl = ddlChanges.ToString();

    return Open(std::move(reader), invert, mode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::Open(std::unique_ptr<ChangeStream> changeStream, bool invert, Mode mode) {
    if(IsOpen()) {
        LOG.errorv("Attempting to open a file on an already open PreparedECChangesetReader.");
        return BE_SQLITE_ERROR;
    }
    if (m_changeStream != nullptr)
        return BE_SQLITE_ERROR;

    if (changeStream == nullptr)
        return BE_SQLITE_ERROR;

    m_invert = invert;
    m_mode   = mode;
    m_changeStream = std::move(changeStream);
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::OpenGroup(T_Utf8StringVector const& files, Db const& db, bool invert, Mode mode) {
    if(IsOpen()) {
        LOG.errorv("Attempting to open a file on an already open PreparedECChangesetReader.");
        return BE_SQLITE_ERROR;
    }
    m_changeGroup = std::make_unique<ChangeGroup>(db);
    DdlChanges ddlGroup;
    for (auto& changesetFile : files) {
        BeFileName inputFile(changesetFile);
        if (!inputFile.DoesPathExist())
            return BE_SQLITE_CANTOPEN;

        bvector<BeFileName> fileVec{inputFile};
        ChangesetFileReaderBase reader(fileVec);
        bool containsSchemaChanges;
        DdlChanges ddlChanges;
        if (BE_SQLITE_OK != reader.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges))
            return BE_SQLITE_ERROR;

        for (auto& ddl : ddlChanges.GetDDLs()) {
            ddlGroup.AddDDL(ddl.c_str());
        }
        if (BE_SQLITE_OK != reader.AddToChangeGroup(*m_changeGroup))
            return BE_SQLITE_ERROR;
    }

    m_changeStream = std::make_unique<ChangeSet>();
    if (BE_SQLITE_OK != m_changeStream->FromChangeGroup(*m_changeGroup))
        return BE_SQLITE_ERROR;

    m_ddl    = ddlGroup.ToString();
    m_invert = invert;
    m_mode   = mode;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedECChangesetReader::clearFields() {
    m_fields.clear();
    m_changedProps.clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedECChangesetReader::Close() {
    m_currentChange = Changes::Change(nullptr, false);
    m_changes = nullptr;
    m_changeStream = nullptr;
    m_changeGroup = nullptr;
    m_invert = false;
    m_ddl.clear();
    clearFields();
}


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::Step() {
    if(!IsOpen()) {
        LOG.errorv("Attempting to step a closed PreparedECChangesetReader.");
        return BE_SQLITE_ERROR;
    }
    if (m_changes == nullptr) {
        m_changes = std::make_unique<Changes>(*m_changeStream, m_invert);
        m_currentChange = m_changes->begin();
    } else {
        if(m_currentChange.IsValid()) ++m_currentChange;
    }
    auto stat = m_currentChange.IsValid() ? BE_SQLITE_ROW : BE_SQLITE_DONE;
    if(ReFetchValues() != BE_SQLITE_OK)
        return BE_SQLITE_ERROR;
    return stat;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::ReFetchValues() {
    clearFields();
    if (m_currentChange.IsValid()) {
        DbSchema const& dbSchema = m_ecdb.Schemas().Main().GetDbSchema();
        Utf8String tableName;
        if(GetTableName(tableName) != BE_SQLITE_OK)
            return BE_SQLITE_ERROR;

        DbTable const* dbTable = dbSchema.FindTable(tableName);
        if (dbTable == nullptr) {
            LOG.errorv("Table '%s' not found in schema.", tableName.c_str());
            return BE_SQLITE_ERROR;
        }

        m_fields.try_emplace(Stage::New);
        m_fields.try_emplace(Stage::Old);

        bool isECTable = false;
        if(IsECTable(isECTable) != BE_SQLITE_OK)
            return BE_SQLITE_ERROR;
        if(!isECTable) {
            LOG.infov("Table '%s' is not an EC table. Skipping creating fields", tableName.c_str());
            return BE_SQLITE_OK;
        }

        DbOpcode opCode;
        if(GetOpcode(opCode) != BE_SQLITE_OK)
            return BE_SQLITE_ERROR;

        const bool includeInstanceId = (opCode == DbOpcode::Insert || opCode == DbOpcode::Delete);
        if(opCode != DbOpcode::Delete) {
            ColumnValueMap newValues;
            if (GetColumnValues(Stage::New, newValues) != BE_SQLITE_OK)
                return BE_SQLITE_ERROR;
            DumpColumnValues(newValues);
            if (ChangesetFieldFactory::Create(m_ecdb, *dbTable, newValues, m_fields.at(Stage::New), m_mode,
                                              includeInstanceId, m_changedProps) != BE_SQLITE_OK)
                return BE_SQLITE_ERROR;
        }
        if(opCode != DbOpcode::Insert) {
            ColumnValueMap oldValues;
            if (GetColumnValues(Stage::Old, oldValues) != BE_SQLITE_OK)
                return BE_SQLITE_ERROR;
            DumpColumnValues(oldValues);
            if (ChangesetFieldFactory::Create(m_ecdb, *dbTable, oldValues, m_fields.at(Stage::Old), m_mode,
                                              includeInstanceId, m_changedProps) != BE_SQLITE_OK)
                return BE_SQLITE_ERROR;
        }
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::GetColumnValues(Stage stage, ColumnValueMap& outMap) const {
    if (!IsOpen()) {
        LOG.errorv("Attempting to get column values from a closed PreparedECChangesetReader.");
        return BE_SQLITE_ERROR;
    }
    if (!IsStepped()) {
        LOG.errorv("Attempting to get column values from a PreparedECChangesetReader that has not been stepped or is on an invalid change.");
        return BE_SQLITE_ERROR;
    }
    Utf8String tableName;
    if (GetTableName(tableName) != BE_SQLITE_OK)
        return BE_SQLITE_ERROR;

    DbSchema const& dbSchema = m_ecdb.Schemas().Main().GetDbSchema();
    DbTable const* dbTable = dbSchema.FindTable(tableName);
    if (dbTable == nullptr) {
        LOG.errorv("Table '%s' not found in schema.", tableName.c_str());
        return BE_SQLITE_ERROR;
    }

    outMap.clear();
    int colIdx = 0;
    for (DbColumn const* col : dbTable->GetColumns()) {
        if (col->IsVirtual())
            continue;  // virtual columns are absent from the changeset
        DbValue val = m_currentChange.GetValue(colIdx, stage);
        if (!val.IsValid() && m_currentChange.IsPrimaryKeyColumn(colIdx)) {
            // SQLite changesets store PK column values only in the Old slot, even for UPDATE.
            // Mirror the TS SqliteChangesetReader behaviour (includePrimaryKeyInUpdateNew):
            // always include the PK value regardless of which stage is being built.
            val = m_currentChange.GetOldValue(colIdx);
        }
        if (val.IsValid())
            outMap.emplace(col->GetName(), val);
        ++colIdx;
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedECChangesetReader::DumpColumnValues(ColumnValueMap const& map) {
    for (auto const& [key, val] : map)
        std::cout << key.c_str() << " = " << (val.IsNull() ? "NULL" : (val.GetValueText() ? val.GetValueText() : "(blob)")) << std::endl;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int PreparedECChangesetReader::GetColumnCount(Stage stage) const {
    if(!IsOpen())
    {
        LOG.warningv("Attempting to get column count from a closed PreparedECChangesetReader.");
        return 0;
    }
    if(!IsStepped())
    {
        LOG.warningv("Attempting to get column count from a PreparedECChangesetReader that has not been stepped.");
        return 0;
    }
    return m_fields.find(stage) != m_fields.end() ? static_cast<int>(m_fields.at(stage).size()) : 0;
}

DbResult PreparedECChangesetReader::GetTableName(Utf8StringR tableName) const {
    if (!IsOpen())
        {
        LOG.errorv("Attempting to get table name from a closed PreparedECChangesetReader.");
        return BE_SQLITE_ERROR;
        }
    if (!IsStepped())
        {
        LOG.errorv("Attempting to get table name from a PreparedECChangesetReader that has not been stepped or is on an invalid change.");
        return BE_SQLITE_ERROR;
        }
    tableName = m_currentChange.GetTableName();
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::GetOpcode(DbOpcode& opcode) const {
    if (!IsOpen())
        {
        LOG.errorv("Attempting to get opcode from a closed PreparedECChangesetReader.");
        return BE_SQLITE_ERROR;
        }
    if (!IsStepped())
        {
        LOG.errorv("Attempting to get opcode from a PreparedECChangesetReader that has not been stepped or is on an invalid change.");
        return BE_SQLITE_ERROR;
        }
    opcode = m_currentChange.GetOpcode();
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& PreparedECChangesetReader::GetValue(Stage stage, int columnIndex) const {
    if (!IsOpen())
        {
        LOG.warningv("Attempting to get value from a closed PreparedECChangesetReader.");
        return NoopECSqlValue::GetSingleton();
        }
    if (!IsStepped())
        {
        LOG.warningv("Attempting to get value from a PreparedECChangesetReader that has not been stepped or is on an invalid change.");
        return NoopECSqlValue::GetSingleton();
        }
    int size = GetColumnCount(stage);
    if (columnIndex < 0 || columnIndex >= size) {
        LOG.warningv("Column index %d is out of range for table.", columnIndex);
        return NoopECSqlValue::GetSingleton();
    }
    return *m_fields.at(stage).at(columnIndex);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::GetInstanceKey(Stage stage, Utf8StringR key) const {
    if (!IsOpen())
        {
        LOG.errorv("Attempting to get instance key from a closed PreparedECChangesetReader.");
        return BE_SQLITE_ERROR;
        }
    if (!IsStepped())
        {
        LOG.errorv("Attempting to get instance key from a PreparedECChangesetReader that has not been stepped or is on an invalid change.");
        return BE_SQLITE_ERROR;
        }
    const int count = GetColumnCount(stage);
    Utf8String instanceId;
    Utf8String classId;
    for (int i = 0; i < count; ++i)
        {
        IECSqlValue const& val = GetValue(stage, i);
        if (val.IsNull())
            continue;
        auto const* prop = val.GetColumnInfo().GetProperty();
        if (prop == nullptr)
            continue;
        auto const* primProp = prop->GetAsPrimitiveProperty();
        if (primProp == nullptr)
            continue;
        const auto extType = ExtendedTypeHelper::GetExtendedType(primProp->GetExtendedTypeName());
        Utf8StringCR propName = prop->GetName();
        if (extType == ExtendedTypeHelper::ExtendedType::Id && propName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId))
            instanceId = val.GetId<ECInstanceId>().ToHexStr();
        else if (extType == ExtendedTypeHelper::ExtendedType::ClassId && propName.EqualsIAscii(ECDBSYS_PROP_ECClassId))
            classId = val.GetId<ECN::ECClassId>().ToHexStr();
        }
    if(instanceId.empty() || classId.empty())
        {
        LOG.warningv("Could not find both ECInstanceId and ECClassId for stage %s of current change. Instance key cannot be constructed.", stage == Stage::New ? "New" : "Old");
        key.clear();
        return BE_SQLITE_OK;
        }
    key.Sprintf("%s-%s", instanceId.c_str(), classId.c_str());
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::IsECTable(bool& isECTable) const {
    if (!IsOpen())
        {
        LOG.errorv("Attempting to check IsECTable on a closed PreparedECChangesetReader.");
        return BE_SQLITE_ERROR;
        }
    if (!IsStepped())
        {
        LOG.errorv("Attempting to check IsECTable on a PreparedECChangesetReader that has not been stepped or is on an invalid change.");
        return BE_SQLITE_ERROR;
        }
    
    Utf8String tableName;
    if(GetTableName(tableName) != BE_SQLITE_OK)
        return BE_SQLITE_ERROR;
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT 1 FROM ec_Table WHERE Name=?");
    if (stmt == nullptr)
        return BE_SQLITE_ERROR;
    stmt->BindText(1, tableName.c_str(), Statement::MakeCopy::No);
    DbResult rc = stmt->Step();
    if (rc != BE_SQLITE_ROW && rc != BE_SQLITE_DONE)
        return rc;
    isECTable = (rc == BE_SQLITE_ROW);
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::GetChangedPropertyNames(std::unordered_set<Utf8String>& out) const {
    if(!IsOpen())
        {
        LOG.errorv("Attempting to get changed property names from a closed PreparedECChangesetReader.");
        return BE_SQLITE_ERROR;
        }
    if(!IsStepped())
        {
        LOG.errorv("Attempting to get changed property names from a PreparedECChangesetReader that has not been stepped or is on an invalid change.");
        return BE_SQLITE_ERROR;
        }
    out.clear();
    out = m_changedProps;
    return BE_SQLITE_OK;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
