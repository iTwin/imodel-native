/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PreparedChangesetReader::PreparedChangesetReader(ECDbCR ecdb)
    : m_ecdb(ecdb), m_currentChange(nullptr, false)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::OpenChangesetFile(Utf8StringCR changesetFile, bool invert, PropertyFilter mode) {
    if(IsOpen()) {
        LOG.errorv("Attempting to open a file on an already open PreparedChangesetReader.");
        return BE_SQLITE_ERROR;
    }
    BeFileName input;
    input.AppendUtf8(changesetFile.c_str());

    if (!input.DoesPathExist() || input.IsDirectory())
        return BE_SQLITE_CANTOPEN;

    bvector<BeFileName> files{input};
    auto reader = std::make_unique<ChangesetFileReaderBase>(files);

    return Open(std::move(reader), invert, mode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::Open(std::unique_ptr<ChangeStream> changeStream, bool invert, PropertyFilter propertyFilter) {
    if(IsOpen()) {
        LOG.errorv("Attempting to open on an already open PreparedChangesetReader.");
        return BE_SQLITE_ERROR;
    }

    if (changeStream == nullptr)
        return BE_SQLITE_ERROR;

    m_invert = invert;
    m_propertyFilter = propertyFilter;
    m_changeStream = std::move(changeStream);
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::OpenInMemoryChangeset(std::unique_ptr<ChangeSet> changeSet, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    if (IsOpen()) {
        LOG.errorv("Attempting to open a changeset on an already open PreparedChangesetReader.");
        return BE_SQLITE_ERROR;
    }

    if (!changeSet || changeSet->_IsEmpty())
        return BE_SQLITE_ERROR;

    if (changeSet->GetSize() > spillThreshold) {
        // OpenInMemoryChangeset has no DDL context, so the temp file is written with empty DDL.
        ChangeGroup tempGroup;
        if (BE_SQLITE_OK != changeSet->AddToChangeGroup(tempGroup))
            return BE_SQLITE_ERROR;
        changeSet.reset(); // release the ChangeSet RAM before writing
        DdlChanges emptyDdl;
        if (BE_SQLITE_OK != WriteGroupToFile(tempGroup, emptyDdl, tempGroup.ContainsEcSchemaChanges(), m_tempGroupFile))
            return BE_SQLITE_ERROR;
        tempGroup.Finalize(); // release the ChangeGroup RAM before opening the file
        return OpenChangesetFile(m_tempGroupFile.GetNameUtf8(), invert, propertyFilter);
    }

    return Open(std::move(changeSet), invert, propertyFilter);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::OpenChangeGroup(T_Utf8StringVector const& files, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    if (IsOpen()) {
        LOG.errorv("Attempting to open a group on an already open PreparedChangesetReader.");
        return BE_SQLITE_ERROR;
    }

    ChangeGroup group(m_ecdb);
    DdlChanges groupDdl;
    bool containsSchemaChanges = false;
    for (auto& changesetFile : files) {
        BeFileName inputFile(changesetFile);
        if (!inputFile.DoesPathExist() || inputFile.IsDirectory())
            return BE_SQLITE_CANTOPEN;

        bvector<BeFileName> fileVec{inputFile};
        ChangesetFileReaderBase reader(fileVec);

        // Collect DDL and schema-change flag from each file before merging.
        bool fileHasSchemaChanges = false;
        DdlChanges fileDdl;
        if (BE_SQLITE_OK != reader.MakeReader()->GetSchemaChanges(fileHasSchemaChanges, fileDdl))
            return BE_SQLITE_ERROR;
        if (fileHasSchemaChanges)
            containsSchemaChanges = true;
        for (auto const& ddl : fileDdl.GetDDLs())
            groupDdl.AddDDL(ddl.c_str());

        if (BE_SQLITE_OK != reader.AddToChangeGroup(group))
            return BE_SQLITE_ERROR;
    }

    // Materialize the merged group to measure its size.
    auto cs = std::make_unique<ChangeSet>();
    if (BE_SQLITE_OK != cs->FromChangeGroup(group))
        return BE_SQLITE_ERROR;

    if (cs->GetSize() > spillThreshold) {
        cs.reset();
        if (BE_SQLITE_OK != WriteGroupToFile(group, groupDdl, containsSchemaChanges, m_tempGroupFile))
            return BE_SQLITE_ERROR;
        group.Finalize(); // release the ChangeGroup RAM before opening the file
        return OpenChangesetFile(m_tempGroupFile.GetNameUtf8(), invert, propertyFilter);
    }

    group.Finalize(); // group is fully materialized into cs; release the ChangeGroup RAM
    return OpenInMemoryChangeset(std::move(cs), invert, propertyFilter, spillThreshold);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::WriteGroupToFile(ChangeGroup& changeGroup, DdlChanges const& ddlChanges, bool containsSchemaChanges, BeFileNameR outPath) {
    BeGuid guid(true);
    BeFileName tempPath(Utf8String(m_ecdb.GetTempFileBaseName() + "-" + guid.ToString() + "-merged.changeset").c_str());

    if(tempPath.DoesPathExist()) {
        LOG.errorv("WriteGroupToFile: temp file '%s' already exists.", tempPath.GetNameUtf8().c_str());
        return BE_SQLITE_ERROR;
    }

    // Level 1 compression with a small dictionary: this file is ephemeral so speed beats ratio.
    LzmaEncoder::LzmaParams fastParams(1 << 16, false, 1, 1);
    ChangesetFileWriter writer(tempPath, containsSchemaChanges, ddlChanges, &m_ecdb, fastParams);

    if (BE_SQLITE_OK != writer.Initialize()) {
        LOG.errorv("WriteGroupToFile: failed to initialize ChangesetFileWriter for '%s'.", tempPath.GetNameUtf8().c_str());
        return BE_SQLITE_ERROR;
    }

    // Always write via FromChangeGroup so that DDL and schema-change flags flow through correctly.
    writer.FromChangeGroup(changeGroup);

    if(!tempPath.DoesPathExist()) {
        LOG.errorv("WriteGroupToFile: failed to write temp file '%s'.", tempPath.GetNameUtf8().c_str());
        return BE_SQLITE_ERROR;
    }

    // writer destructor fires FinishOutput here, flushing and closing the file
    outPath = tempPath;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedChangesetReader::ClearFields() {
    m_fields.clear();
    m_changedPropNames.clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedChangesetReader::Close() {
    m_currentChange = Changes::Change(nullptr, false);
    m_changes = nullptr;
    m_changeStream = nullptr; // must be destroyed before we delete the temp file it may be reading
    m_invert = false;
    ClearFields();
    ClearTableFilters();
    ClearOpcodeFilters();
    ClearECClassNameFilters();
    if (m_tempGroupFile.DoesPathExist())
        m_tempGroupFile.BeDeleteFile();
    m_tempGroupFile = BeFileName{};
}


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::Step() {
    if(!IsOpen()) {
        LOG.errorv("Attempting to step a closed PreparedChangesetReader.");
        return BE_SQLITE_ERROR;
    }
    if (m_changes == nullptr) {
        ClearFields();
        m_changes = std::make_unique<Changes>(*m_changeStream, m_invert);
        m_currentChange = m_changes->begin();
    } else {
        if(m_currentChange.IsValid()) {
            ClearFields();
            ++m_currentChange;
        }
    }
    auto stat = m_currentChange.IsValid() ? BE_SQLITE_ROW : BE_SQLITE_DONE;
    bool isCurrentRowFilteredOut = false;
    if(ReFetchValues(isCurrentRowFilteredOut) != SUCCESS)
        return BE_SQLITE_ERROR;

    if(isCurrentRowFilteredOut) {
        LOG.infov("Current change is filtered out. Stepping to the next change.");
        return Step();
    }
    return stat;
}

//---------------------------------------------------------------------------------------
// @bsimethod
// In case the current row doesnot fit the filters, we will not create fields for it. When users call GetColumnValue, the number of rows returned will be 0. 
// As the current API we step row  by row. And after we step onto a row we come to know whether the row is filtered out or not.
// So if the row is filtered out, we will not create fields for it(as that is the most expensive part of the operation).
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::ReFetchValues(bool& isCurrentRowFilteredOut) {
    isCurrentRowFilteredOut = false;
    m_fields.try_emplace(Stage::New);
    m_fields.try_emplace(Stage::Old);
    if (m_currentChange.IsValid()) {
        DbOpcode opCode;
        if(GetOpcode(opCode) != SUCCESS)
            return ERROR;
        
        if(!IsOpcodeAllowedPostFilter(opCode)) { // First is opCode filter
            LOG.infov("Opcode '%s' is not allowed by filters. Skipping creating fields", DbOpcodeToString(opCode).c_str());
            isCurrentRowFilteredOut = true;
            return SUCCESS;
        }
        
        Utf8String tableName;
        if(GetTableName(tableName) != SUCCESS)
            return ERROR;
        
        if(!IsTableAllowedPostFilter(tableName)) { // second is table filter
            LOG.infov("Table '%s' is not allowed by filters. Skipping creating fields", tableName.c_str());
            isCurrentRowFilteredOut = true;
            return SUCCESS;
        }

        DbSchema const& dbSchema = m_ecdb.Schemas().Main().GetDbSchema();
        DbTable const* dbTable = dbSchema.FindTable(tableName);
        if (dbTable == nullptr) {
            LOG.errorv("Table '%s' not found in schema.", tableName.c_str());
            return ERROR;
        }

        bool isECTable = false;
        if(IsECTable(isECTable) != SUCCESS)
            return ERROR;
        if(!isECTable) {
            LOG.infov("Table '%s' is not an EC table. Skipping creating fields", tableName.c_str());
            return SUCCESS;
        }

        if(opCode != DbOpcode::Delete) {
            ColumnValueMap newValues;
            if (GetColumnValues(Stage::New, newValues) != SUCCESS)
                return ERROR;
            ECClassId classId;
            bool isClassIdFromChangeset = false;
            if (ChangesetValueFactory::ResolveClassId(m_ecdb, *dbTable, newValues, classId, isClassIdFromChangeset) != SUCCESS)
                return ERROR;
            ECClassCP ecClass = m_ecdb.Schemas().Main().GetClass(classId);
            if (ecClass == nullptr) {
                LOG.errorv("ECClass with id %" PRIu64 " not found in schema.", classId.GetValueUnchecked());
                return ERROR;
            }
            Utf8String fullClassName = ecClass->GetFullName();
            if(!IsECClassNameAllowedPostFilter(fullClassName)) { // Third is ECClassName filter for old values
                LOG.infov("ECClass '%s' is not allowed by filters. Skipping creating fields", fullClassName.c_str());
                isCurrentRowFilteredOut = true;
                return SUCCESS;
            }
            if (ChangesetValueFactory::Create(m_ecdb, *dbTable, newValues, classId, isClassIdFromChangeset, m_fields.at(Stage::New), m_propertyFilter, m_changedPropNames) != SUCCESS)
                return ERROR;
        }
        if(opCode != DbOpcode::Insert) {
            ColumnValueMap oldValues;
            if (GetColumnValues(Stage::Old, oldValues) != SUCCESS)
                return ERROR;
            ECClassId classId;
            bool isClassIdFromChangeset = false;
            if (ChangesetValueFactory::ResolveClassId(m_ecdb, *dbTable, oldValues, classId, isClassIdFromChangeset) != SUCCESS)
                return ERROR;
            ECClassCP ecClass = m_ecdb.Schemas().Main().GetClass(classId);
            if (ecClass == nullptr) {
                LOG.errorv("ECClass with id %" PRIu64 " not found in schema.", classId.GetValueUnchecked());
                return ERROR;
            }
            Utf8String classFullName = ecClass->GetFullName();
            if(!IsECClassNameAllowedPostFilter(classFullName)) { // Third is ECClassName filter for old values
                LOG.infov("ECClass '%s' is not allowed by filters. Skipping creating fields", classFullName.c_str());
                isCurrentRowFilteredOut = true;
                return SUCCESS;
            }
            std::vector<Utf8String> ignored; // For update operation we have already filled m_changedProps in the above ChangesetValueFactory::Create call
            if (ChangesetValueFactory::Create(m_ecdb, *dbTable, oldValues, classId, isClassIdFromChangeset, m_fields.at(Stage::Old), m_propertyFilter, opCode == DbOpcode::Update ? ignored : m_changedPropNames) != SUCCESS)
                return ERROR;
        }
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::GetColumnValues(Stage stage, ColumnValueMap& outMap) const {
    if (!IsOpen()) {
        LOG.errorv("Attempting to get column values from a closed PreparedChangesetReader.");
        return ERROR;
    }
    if (!IsStepped()) {
        LOG.errorv("Attempting to get column values from a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return ERROR;
    }
    Utf8String tableName;
    if (GetTableName(tableName) != SUCCESS)
        return ERROR;

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT [name] FROM PRAGMA_TABLE_INFO(?) ORDER BY [cid]");
    if(stmt == nullptr) {
        LOG.errorv("Failed to prepare statement to get column names for table '%s'.", tableName.c_str());
        return ERROR;
    }
    stmt->Reset(); // reset before each use to ensure statement is in a clean state
    stmt->ClearBindings(); // clear bindings to remove any previous parameters
    stmt->BindText(1, tableName.c_str(), Statement::MakeCopy::No);

    outMap.clear();
    int colIdx = 0;
    DbResult stat = stmt->Step();
    while (stat == BE_SQLITE_ROW) {
        Utf8CP colName = stmt->GetValueText(0);
        DbValue val = m_currentChange.GetValue(colIdx, stage);
        if (!val.IsValid() && m_currentChange.IsPrimaryKeyColumn(colIdx)) {
            // SQLite changesets store PK column values only in the Old slot, even for UPDATE.
            val = m_currentChange.GetOldValue(colIdx);
        }
        if (val.IsValid())
            outMap.emplace(colName, val);
            
        ++colIdx;
        stat = stmt->Step();
    }
    stmt->Reset(); // reset after stepping to prepare for next use
    stmt->ClearBindings(); // clear bindings after stepping to remove parameters for next use
    if(stat != BE_SQLITE_DONE) {
        LOG.errorv("Failed to step through column names for table '%s'.", tableName.c_str());
        return ERROR;
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedChangesetReader::DumpColumnValues(ColumnValueMap const& map) const {
    for (auto const& [key, val] : map)
        LOG.debugv("%s = %s", key.c_str(), val.IsNull() ? "NULL" : (val.GetValueText() ? val.GetValueText() : "(blob)"));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int PreparedChangesetReader::GetColumnCount(Stage stage) const {
    if(!IsOpen())
    {
        LOG.warningv("Attempting to get column count from a closed PreparedChangesetReader.");
        return 0;
    }
    if(!IsStepped())
    {
        LOG.warningv("Attempting to get column count from a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return 0;
    }
    return m_fields.find(stage) != m_fields.end() ? static_cast<int>(m_fields.at(stage).size()) : 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::GetTableName(Utf8StringR tableName) const {
    if (!IsOpen())
        {
        LOG.errorv("Attempting to get table name from a closed PreparedChangesetReader.");
        return ERROR;
        }
    if (!IsStepped())
        {
        LOG.errorv("Attempting to get table name from a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return ERROR;
        }
    tableName = m_currentChange.GetTableName();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::GetOpcode(DbOpcode& opcode) const {
    if (!IsOpen())
        {
        LOG.errorv("Attempting to get opcode from a closed PreparedChangesetReader.");
        return ERROR;
        }
    if (!IsStepped())
        {
        LOG.errorv("Attempting to get opcode from a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return ERROR;
        }
    opcode = m_currentChange.GetOpcode();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& PreparedChangesetReader::GetValue(Stage stage, int columnIndex) const {
    if (!IsOpen())
        {
        LOG.warningv("Attempting to get value from a closed PreparedChangesetReader.");
        return NoopECSqlValue::GetSingleton();
        }
    if (!IsStepped())
        {
        LOG.warningv("Attempting to get value from a PreparedChangesetReader that has not been stepped or is on an invalid change.");
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
BentleyStatus PreparedChangesetReader::GetInstanceKey(Stage stage, Utf8StringR key) const {
    if (!IsOpen())
        {
        LOG.errorv("Attempting to get instance key from a closed PreparedChangesetReader.");
        return ERROR;
        }
    if (!IsStepped())
        {
        LOG.errorv("Attempting to get instance key from a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return ERROR;
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
        LOG.warningv("Could not find either ECInstanceId or ECClassId or both for stage %s of current change. Instance key cannot be constructed.", stage == Stage::New ? "New" : "Old");
        key.clear();
        return ERROR;
        }
    key.Sprintf("%s-%s", instanceId.c_str(), classId.c_str());
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::IsECTable(bool& isECTable) const {
    if (!IsOpen())
        {
        LOG.errorv("Attempting to check IsECTable on a closed PreparedChangesetReader.");
        return ERROR;
        }
    if (!IsStepped())
        {
        LOG.errorv("Attempting to check IsECTable on a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return ERROR;
        }
    
    Utf8String tableName;
    if(GetTableName(tableName) != SUCCESS)
        return ERROR;
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT 1 FROM ec_Table WHERE Name=?");
    if (stmt == nullptr) {
        LOG.errorv("Failed to prepare statement to check if table '%s' is an EC table.", tableName.c_str());
        return ERROR;
    }
    stmt->Reset(); // reset before each use to ensure statement is in a clean state
    stmt->ClearBindings(); // clear bindings to remove any previous parameters
    stmt->BindText(1, tableName.c_str(), Statement::MakeCopy::No);
    DbResult rc = stmt->Step();
    stmt->Reset(); // reset after stepping to prepare for next use
    stmt->ClearBindings(); // clear bindings after stepping to remove parameters for next use
    if (rc != BE_SQLITE_ROW && rc != BE_SQLITE_DONE) {
        LOG.errorv("Failed to step statement to check if table '%s' is an EC table.", tableName.c_str());
        return ERROR;
    }
    isECTable = (rc == BE_SQLITE_ROW);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::GetChangeFetchedPropertyNames(std::vector<Utf8String>& out) const {
    if(!IsOpen())
        {
        LOG.errorv("Attempting to get changed property names from a closed PreparedChangesetReader.");
        return ERROR;
        }
    if(!IsStepped())
        {
        LOG.errorv("Attempting to get changed property names from a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return ERROR;
        }
    out.clear();
    out = m_changedPropNames;
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::IsIndirectChange(bool& isIndirect) const {
    if (!IsOpen())
        {
        LOG.errorv("Attempting to check IsIndirectChange on a closed PreparedChangesetReader.");
        return ERROR;
        }
    if (!IsStepped())
        {
        LOG.errorv("Attempting to check IsIndirectChange on a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return ERROR;
        }
    isIndirect = m_currentChange.IsIndirect();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool PreparedChangesetReader::IsTableAllowedPostFilter(Utf8StringCR tableName) const {
    if (m_tableFilters.empty())
        return true;
    return std::find(m_tableFilters.begin(), m_tableFilters.end(), tableName) != m_tableFilters.end();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool PreparedChangesetReader::IsOpcodeAllowedPostFilter(DbOpcode const& opcode) const {
    if (m_opcodeFilters.empty())
        return true;
    return std::find(m_opcodeFilters.begin(), m_opcodeFilters.end(), opcode) != m_opcodeFilters.end();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool PreparedChangesetReader::IsECClassNameAllowedPostFilter(Utf8StringCR classFullName) const {
    if (m_ecclassNameFilters.empty())
        return true;
    return std::find(m_ecclassNameFilters.begin(), m_ecclassNameFilters.end(), classFullName) != m_ecclassNameFilters.end();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String PreparedChangesetReader::DbOpcodeToString(DbOpcode const& opcode) const {
    switch (opcode) {
        case DbOpcode::Insert:
            return "Insert";
        case DbOpcode::Update:
            return "Update";
        case DbOpcode::Delete:
            return "Delete";
        default:
            return "Unknown";
    }
}

END_BENTLEY_SQLITE_EC_NAMESPACE
