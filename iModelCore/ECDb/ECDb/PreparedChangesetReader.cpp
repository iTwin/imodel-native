/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=============================================================================
// ChangesetFilter
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFilter::IsTableAllowed(Utf8StringCR tableName) const {
    if (m_tableFilters.empty())
        return true;
    return std::find(m_tableFilters.begin(), m_tableFilters.end(), tableName) != m_tableFilters.end();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFilter::IsOpcodeAllowed(DbOpcode opcode) const {
    if (m_opcodeFilters.empty())
        return true;
    return std::find(m_opcodeFilters.begin(), m_opcodeFilters.end(), opcode) != m_opcodeFilters.end();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFilter::IsECClassNameAllowed(Utf8StringCR className) const {
    if (m_ecclassNameFilters.empty())
        return true;
    return std::find(m_ecclassNameFilters.begin(), m_ecclassNameFilters.end(), className) != m_ecclassNameFilters.end();
}

//=============================================================================
// ChangesetTempFileManager
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetTempFileManager::WriteGroupToFile(ChangeGroup& changeGroup, DdlChanges const& ddlChanges, bool containsSchemaChanges) {
    BeGuid guid(true);
    BeFileName tempPath(Utf8String(m_ecdb.GetTempFileBaseName() + "-" + guid.ToString() + "-merged.changeset").c_str());

    if (tempPath.DoesPathExist()) {
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
    if (BE_SQLITE_OK != writer.FromChangeGroup(changeGroup)) {
        LOG.errorv("WriteGroupToFile: failed to write change group to temp file '%s'.", tempPath.GetNameUtf8().c_str());
        return BE_SQLITE_ERROR;
    }

    if (!tempPath.DoesPathExist()) {
        LOG.errorv("WriteGroupToFile: failed to write temp file '%s'.", tempPath.GetNameUtf8().c_str());
        return BE_SQLITE_ERROR;
    }

    // writer destructor fires FinishOutput here, flushing and closing the file
    m_tempFile = tempPath;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetTempFileManager::Cleanup() {
    if (m_tempFile.GetNameUtf8().empty())
        return SUCCESS;
    if (m_tempFile.DoesPathExist()) {
        if (m_tempFile.BeDeleteFile() != BeFileNameStatus::Success) {
            LOG.errorv("Failed to delete temporary changeset file '%s'.", m_tempFile.GetNameUtf8().c_str());
            return ERROR;
        }
    }
    Reset();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangesetTempFileManager::CleanupInfallible() {
    if (m_tempFile.GetNameUtf8().empty())
        return;
    if (m_tempFile.DoesPathExist()) {
        if (m_tempFile.BeDeleteFile() != BeFileNameStatus::Success)
            LOG.errorv("Failed to delete temporary changeset file '%s'.", m_tempFile.GetNameUtf8().c_str());
    }
    Reset();
}

//=============================================================================
// ChangesetSqliteIterator
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangesetSqliteIterator::Open(std::unique_ptr<ChangeStream> changeStream, bool invert) {
    m_invert = invert;
    m_changeStream = std::move(changeStream);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangesetSqliteIterator::Close() {
    m_currentChange = Changes::Change(nullptr, false);
    m_changes = nullptr;
    m_changeStream = nullptr; // must be destroyed before caller deletes any temp file it is reading
    m_invert = false;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetSqliteIterator::StepRaw() {
    if (m_changes == nullptr) {
        m_changes = std::make_unique<Changes>(*m_changeStream, m_invert);
        m_currentChange = m_changes->begin();
    } else {
        if (m_currentChange.IsValid())
            ++m_currentChange;
    }
    return m_currentChange.IsValid() ? BE_SQLITE_ROW : BE_SQLITE_DONE;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetSqliteIterator::GetTableName(Utf8StringR tableName) const {
    tableName = m_currentChange.GetTableName();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetSqliteIterator::GetOpcode(DbOpcode& opcode) const {
    opcode = m_currentChange.GetOpcode();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetSqliteIterator::IsECTable(bool& isECTable) const {
    Utf8String tableName;
    if (GetTableName(tableName) != SUCCESS)
        return ERROR;
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT 1 FROM ec_Table WHERE Name=?");
    if (stmt == nullptr) {
        LOG.errorv("Failed to prepare statement to check if table '%s' is an EC table.", tableName.c_str());
        return ERROR;
    }
    stmt->Reset();
    stmt->ClearBindings();
    stmt->BindText(1, tableName.c_str(), Statement::MakeCopy::No);
    DbResult rc = stmt->Step();
    stmt->Reset();
    stmt->ClearBindings();
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
BentleyStatus ChangesetSqliteIterator::IsIndirectChange(bool& isIndirect) const {
    isIndirect = m_currentChange.IsIndirect();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetSqliteIterator::GetChangeColumnCount() const {
    return m_currentChange.GetColumnCount();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetSqliteIterator::GetColumnCount(Utf8StringCR tableName, int& columnCount) const {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT COUNT(*) FROM PRAGMA_TABLE_INFO(?)");
    if (stmt == nullptr) {
        LOG.errorv("Failed to prepare statement to get column count for table '%s'.", tableName.c_str());
        return ERROR;
    }
    stmt->Reset();
    stmt->ClearBindings();
    stmt->BindText(1, tableName.c_str(), Statement::MakeCopy::No);
    DbResult stat = stmt->Step();
    if (stat == BE_SQLITE_ROW) {
        columnCount = stmt->GetValueInt(0);
    } else {
        LOG.errorv("Failed to step through column count query for table '%s'.", tableName.c_str());
        stmt->Reset();
        stmt->ClearBindings();
        return ERROR;
    }
    stmt->Reset();
    stmt->ClearBindings();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetSqliteIterator::GetColumnValues(Stage stage, Utf8StringCR tableName, ColumnValueMap& outMap) const {
    int columnCount = 0;
    if (GetColumnCount(tableName, columnCount) != SUCCESS) {
        LOG.errorv("Failed to get column count for table '%s'.", tableName.c_str());
        return ERROR;
    }
    int minimum = std::min(columnCount, m_currentChange.GetColumnCount());
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT [name] FROM PRAGMA_TABLE_INFO(?) ORDER BY [cid]");
    if (stmt == nullptr) {
        LOG.errorv("Failed to prepare statement to get column names for table '%s'.", tableName.c_str());
        return ERROR;
    }
    stmt->Reset();
    stmt->ClearBindings();
    stmt->BindText(1, tableName.c_str(), Statement::MakeCopy::No);

    outMap.clear();
    int colIdx = 0;
    DbResult stat = stmt->Step();
    while (colIdx < minimum && stat == BE_SQLITE_ROW) {
        Utf8CP colName = stmt->GetValueText(0);
        DbValue val = m_currentChange.GetValue(colIdx, stage);
        if (!val.IsValid() && m_currentChange.IsPrimaryKeyColumn(colIdx))
            val = m_currentChange.GetOldValue(colIdx);
        if (val.IsValid())
            outMap.emplace(colName, val);
        ++colIdx;
        stat = stmt->Step();
    }
    stmt->Reset();
    stmt->ClearBindings();
    if (colIdx != minimum) {
        LOG.errorv("Failed to step through required column names for table '%s'.", tableName.c_str());
        return ERROR;
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangesetSqliteIterator::DumpColumnValues(ColumnValueMap const& map) const {
    for (auto const& [key, val] : map)
        LOG.debugv("%s = %s", key.c_str(), val.IsNull() ? "NULL" : (val.GetValueText() ? val.GetValueText() : "(blob)"));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ChangesetSqliteIterator::DbOpcodeToString(DbOpcode opcode) {
    switch (opcode) {
        case DbOpcode::Insert: return "Insert";
        case DbOpcode::Update: return "Update";
        case DbOpcode::Delete: return "Delete";
        default:               return "Unknown";
    }
}

//=============================================================================
// PreparedChangesetReader
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PreparedChangesetReader::PreparedChangesetReader(ECDbCR ecdb)
    : m_ecdb(ecdb), m_tempFileManager(ecdb), m_iterator(ecdb)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::OpenChangesetFile(Utf8StringCR changesetFile, bool invert, PropertyFilter mode) {
    if (IsOpen()) {
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
    if (IsOpen()) {
        LOG.errorv("Attempting to open on an already open PreparedChangesetReader.");
        return BE_SQLITE_ERROR;
    }
    if (changeStream == nullptr)
        return BE_SQLITE_ERROR;

    m_propertyFilter = propertyFilter;
    m_iterator.Open(std::move(changeStream), invert);
    m_fields.try_emplace(Stage::New);
    m_fields.try_emplace(Stage::Old);
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
        if (BE_SQLITE_OK != m_tempFileManager.WriteGroupToFile(tempGroup, emptyDdl, tempGroup.ContainsEcSchemaChanges()))
            return BE_SQLITE_ERROR;
        tempGroup.Finalize(); // release the ChangeGroup RAM before opening the file
        return OpenChangesetFile(m_tempFileManager.GetTempFilePath().GetNameUtf8(), invert, propertyFilter);
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
        if (BE_SQLITE_OK != m_tempFileManager.WriteGroupToFile(group, groupDdl, containsSchemaChanges))
            return BE_SQLITE_ERROR;
        group.Finalize(); // release the ChangeGroup RAM before opening the file
        return OpenChangesetFile(m_tempFileManager.GetTempFilePath().GetNameUtf8(), invert, propertyFilter);
    }

    group.Finalize(); // group is fully materialized into cs; release the ChangeGroup RAM
    return OpenInMemoryChangeset(std::move(cs), invert, propertyFilter, spillThreshold);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedChangesetReader::ClearFields() {
    // Only clear the vectors — the Stage::New / Stage::Old keys must stay intact for the lifetime of an open reader.
    m_fields[Stage::New].clear();
    m_fields[Stage::Old].clear();
    m_changedPropNames.clear();
    m_columnValuesScratch.clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::Close() {
    ClearMembers();
    return m_tempFileManager.Cleanup();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedChangesetReader::CloseInfallible() {
    ClearMembers();
    m_tempFileManager.CleanupInfallible();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedChangesetReader::ClearMembers() {
    m_iterator.Close();          // stream destroyed here — safe to delete any temp file afterwards
    m_filter.Reset();
    m_fields.clear();
    m_changedPropNames.clear();
    m_columnValuesScratch.clear();
    m_propertyFilter = PropertyFilter::All;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::Step() {
    if (!IsOpen()) {
        LOG.errorv("Attempting to step a closed PreparedChangesetReader.");
        return BE_SQLITE_ERROR;
    }
    ClearFields();
    DbResult stat = m_iterator.StepRaw();
    bool isCurrentRowFilteredOut = false;
    if (ReFetchValues(isCurrentRowFilteredOut) != SUCCESS) {
        ClearFields(); // ensure fields are cleared on error as well in case of error
        return BE_SQLITE_ERROR;
    }
    if (isCurrentRowFilteredOut) {
        LOG.infov("Current change is filtered out. Stepping to the next change.");
        return Step();
    }
    return stat;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PreparedChangesetReader::StageProcessResult PreparedChangesetReader::ProcessStageValues(Stage stage, DbTable const& dbTable, std::vector<Utf8String>& changedPropNames) {
    if (m_iterator.GetColumnValues(stage, dbTable.GetName(), m_columnValuesScratch) != SUCCESS)
        return StageProcessResult::Error;
    ECClassId classId;
    bool isClassIdFromChangeset = false;
    if (ChangesetValueFactory::ResolveClassId(m_ecdb, dbTable, m_columnValuesScratch, classId, isClassIdFromChangeset) != SUCCESS)
        return StageProcessResult::Error;
    ECClassCP ecClass = m_ecdb.Schemas().Main().GetClass(classId);
    if (ecClass == nullptr) {
        LOG.errorv("ECClass with id %" PRIu64 " not found in schema.", classId.GetValueUnchecked());
        return StageProcessResult::Error;
    }
    Utf8String className = ecClass->GetFullName();
    if (!m_filter.IsECClassNameAllowed(className)) {
        LOG.infov("ECClass '%s' is not allowed by filters. Skipping creating fields", className.c_str());
        return StageProcessResult::Filtered;
    }
    if (m_filter.IsStrictMode()) {
        int columnCount = 0;
        if (m_iterator.GetColumnCount(dbTable.GetName(), columnCount) != SUCCESS)
            return StageProcessResult::Error;
        if (columnCount != m_iterator.GetChangeColumnCount()) {
            LOG.errorv("Column count mismatch for table '%s': expected %d columns based on PRAGMA_TABLE_INFO, but changeset has %d columns. Disable strict mode to not treat this as an error.", dbTable.GetName().c_str(), columnCount, m_iterator.GetChangeColumnCount());
            return StageProcessResult::Error;
        }
    }
    if (ChangesetValueFactory::Create(m_ecdb, dbTable, m_columnValuesScratch, classId, isClassIdFromChangeset, m_fields.at(stage), m_propertyFilter, changedPropNames) != SUCCESS)
        return StageProcessResult::Error;
    return StageProcessResult::Success;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::ReFetchValues(bool& isCurrentRowFilteredOut) {
    isCurrentRowFilteredOut = false;
    if (!IsStepped())
        return SUCCESS;

    DbOpcode opCode;
    if (GetOpcode(opCode) != SUCCESS)
        return ERROR;

    if (!m_filter.IsOpcodeAllowed(opCode)) {
        LOG.infov("Opcode '%s' is not allowed by filters. Skipping creating fields", ChangesetSqliteIterator::DbOpcodeToString(opCode).c_str());
        isCurrentRowFilteredOut = true;
        return SUCCESS;
    }

    Utf8String tableName;
    if (GetTableName(tableName) != SUCCESS)
        return ERROR;

    if (!m_filter.IsTableAllowed(tableName)) {
        LOG.infov("Table '%s' is not allowed by filters. Skipping creating fields", tableName.c_str());
        isCurrentRowFilteredOut = true;
        return SUCCESS;
    }

    bool isECTable = false;
    if (IsECTable(isECTable) != SUCCESS)
        return ERROR;
    if (!isECTable) {
        LOG.infov("Table '%s' is not an EC table. Skipping creating fields", tableName.c_str());
        return SUCCESS;
    }

    DbTable const* dbTable = m_ecdb.Schemas().Main().GetDbSchema().FindTable(tableName);
    if (dbTable == nullptr) {
        LOG.errorv("Table '%s' not found in schema.", tableName.c_str());
        return ERROR;
    }

    if (opCode != DbOpcode::Delete) {
        auto result = ProcessStageValues(Stage::New, *dbTable, m_changedPropNames);
        m_columnValuesScratch.clear(); // clear scratch map after processing New stage to free up memory
        if (result == StageProcessResult::Error) return ERROR;
        if (result == StageProcessResult::Filtered) { isCurrentRowFilteredOut = true; return SUCCESS; }
    }
    if (opCode != DbOpcode::Insert) {
        std::vector<Utf8String> ignored; // For update operation we have already filled m_changedProps in the above ChangesetValueFactory::Create call
        auto result = ProcessStageValues(Stage::Old, *dbTable, (opCode == DbOpcode::Update) ? ignored : m_changedPropNames);
        m_columnValuesScratch.clear(); // clear scratch map after processing Old stage to free up memory
        if (result == StageProcessResult::Error) return ERROR;
        if (result == StageProcessResult::Filtered) { isCurrentRowFilteredOut = true; return SUCCESS; }
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int PreparedChangesetReader::GetColumnCount(Stage stage) const {
    if (!IsOpen()) {
        LOG.errorv("Attempting to get column count from a closed PreparedChangesetReader.");
        return 0;
    }
    if (!IsStepped()) {
        LOG.errorv("Attempting to get column count from a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return 0;
    }
    return m_fields.find(stage) != m_fields.end() ? static_cast<int>(m_fields.at(stage).size()) : 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::GetTableName(Utf8StringR tableName) const {
    if (!IsOpen()) {
        LOG.errorv("Attempting to get table name from a closed PreparedChangesetReader.");
        return ERROR;
    }
    if (!IsStepped()) {
        LOG.errorv("Attempting to get table name from a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return ERROR;
    }
    return m_iterator.GetTableName(tableName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::GetOpcode(DbOpcode& opcode) const {
    if (!IsOpen()) {
        LOG.errorv("Attempting to get opcode from a closed PreparedChangesetReader.");
        return ERROR;
    }
    if (!IsStepped()) {
        LOG.errorv("Attempting to get opcode from a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return ERROR;
    }
    return m_iterator.GetOpcode(opcode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& PreparedChangesetReader::GetValue(Stage stage, int columnIndex) const {
    if (!IsOpen()) {
        LOG.errorv("Attempting to get value from a closed PreparedChangesetReader.");
        return NoopECSqlValue::GetSingleton();
    }
    if (!IsStepped()) {
        LOG.errorv("Attempting to get value from a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return NoopECSqlValue::GetSingleton();
    }
    int size = GetColumnCount(stage);
    if (columnIndex < 0 || columnIndex >= size) {
        LOG.errorv("Column index %d is out of range for current row of change record for stage %s.", columnIndex, stage == Stage::New ? "New" : "Old");
        return NoopECSqlValue::GetSingleton();
    }
    return *m_fields.at(stage).at(columnIndex);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::GetInstanceKey(Stage stage, Utf8StringR key) const {
    if (!IsOpen()) {
        LOG.errorv("Attempting to get instance key from a closed PreparedChangesetReader.");
        return ERROR;
    }
    if (!IsStepped()) {
        LOG.errorv("Attempting to get instance key from a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return ERROR;
    }
    const int count = GetColumnCount(stage);
    Utf8String instanceId;
    Utf8String classId;
    for (int i = 0; i < count; ++i) {
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
    if (instanceId.empty() || classId.empty()) {
        LOG.errorv("Could not find either ECInstanceId or ECClassId or both for stage %s of current change. Instance key cannot be constructed.", stage == Stage::New ? "New" : "Old");
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
    if (!IsOpen()) {
        LOG.errorv("Attempting to check IsECTable on a closed PreparedChangesetReader.");
        return ERROR;
    }
    if (!IsStepped()) {
        LOG.errorv("Attempting to check IsECTable on a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return ERROR;
    }
    return m_iterator.IsECTable(isECTable);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<Utf8String> const* PreparedChangesetReader::GetChangeFetchedPropertyNames() const {
    if (!IsOpen()) {
        LOG.errorv("Attempting to get changed property names from a closed PreparedChangesetReader.");
        return nullptr;
    }
    if (!IsStepped()) {
        LOG.errorv("Attempting to get changed property names from a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return nullptr;
    }
    return &m_changedPropNames;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::IsIndirectChange(bool& isIndirect) const {
    if (!IsOpen()) {
        LOG.errorv("Attempting to check IsIndirectChange on a closed PreparedChangesetReader.");
        return ERROR;
    }
    if (!IsStepped()) {
        LOG.errorv("Attempting to check IsIndirectChange on a PreparedChangesetReader that has not been stepped or is on an invalid change.");
        return ERROR;
    }
    return m_iterator.IsIndirectChange(isIndirect);
}

END_BENTLEY_SQLITE_EC_NAMESPACE
