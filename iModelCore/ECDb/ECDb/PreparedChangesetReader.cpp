/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=============================================================================
// TableColumnCache
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<Utf8String> const* TableColumnCache::GetOrderedColumnNames(ECDbCR ecdb, Utf8StringCR tableName) {
    auto it = m_index.find(tableName);
    if (it != m_index.end()) {
        // cache hit: promote to front
        m_lruList.splice(m_lruList.begin(), m_lruList, it->second);
        return &it->second->columnNames;
    }

    // cache miss: query DB
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT [name] FROM PRAGMA_TABLE_INFO(?) ORDER BY [cid]");
    if (stmt == nullptr) {
        LOG.errorv("TableColumnCache: failed to prepare PRAGMA_TABLE_INFO statement for table '%s'.", tableName.c_str());
        return nullptr;
    }
    stmt->Reset();
    stmt->ClearBindings();
    stmt->BindText(1, tableName.c_str(), Statement::MakeCopy::No);

    std::vector<Utf8String> names;
    DbResult rc;
    while ((rc = stmt->Step()) == BE_SQLITE_ROW)
        names.emplace_back(stmt->GetValueText(0));

    stmt->Reset();
    stmt->ClearBindings();

    if (rc != BE_SQLITE_DONE) {
        LOG.errorv("TableColumnCache: failed to step PRAGMA_TABLE_INFO for table '%s'.", tableName.c_str());
        return nullptr;
    }

    // evict LRU entry if at capacity
    if (m_lruList.size() >= MAX_ENTRIES) {
        m_index.erase(m_lruList.back().tableName);
        m_lruList.pop_back();
    }

    // insert new entry at front
    m_lruList.push_front(Entry{tableName, std::move(names)});
    m_index.emplace(m_lruList.front().tableName, m_lruList.begin());
    return &m_lruList.front().columnNames;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void TableColumnCache::Clear() {
    m_lruList.clear();
    m_index.clear();
}

//=============================================================================
// ChangesetFilterContext
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFilterContext::IsTableAllowed(Utf8StringCR tableName) const {
    if (m_tableFilters.empty())
        return true;
    return std::find(m_tableFilters.begin(), m_tableFilters.end(), tableName) != m_tableFilters.end();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFilterContext::IsOpcodeAllowed(DbOpcode const& opcode) const {
    if (m_opcodeFilters.empty())
        return true;
    return std::find(m_opcodeFilters.begin(), m_opcodeFilters.end(), opcode) != m_opcodeFilters.end();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetFilterContext::IsECClassNameAllowed(Utf8StringCR className) const {
    if (m_ecclassNameFilters.empty())
        return true;
    return std::find(m_ecclassNameFilters.begin(), m_ecclassNameFilters.end(), className) != m_ecclassNameFilters.end();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetFilterContext::CheckColumnCount(int changeColumnCount, int dbColumnCount, Utf8StringCR tableName) const {
    if (!m_strictMode)
        return SUCCESS;
    if (dbColumnCount != changeColumnCount) {
        LOG.errorv("Column count mismatch for table '%s': expected %d columns based on PRAGMA_TABLE_INFO, but changeset has %d columns. Disable strict mode to not treat this as an error.", tableName.c_str(), dbColumnCount, changeColumnCount);
        return ERROR;
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ChangesetFilterContext::OpcodeToString(DbOpcode const& opcode) {
    switch (opcode) {
        case DbOpcode::Insert: return "Insert";
        case DbOpcode::Update: return "Update";
        case DbOpcode::Delete: return "Delete";
        default:               return "Unknown";
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangesetFilterContext::Reset() {
    m_propertyFilter = PropertyFilter::All;
    m_strictMode     = false;
    m_tableFilters.clear();
    m_opcodeFilters.clear();
    m_ecclassNameFilters.clear();
}

//=============================================================================
// InternalChangeIterator
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void InternalChangeIterator::Open(std::unique_ptr<ChangeStream> stream, bool invert) {
    m_changeStream = std::move(stream);
    m_invert       = invert;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void InternalChangeIterator::Advance() {
    if (m_changes == nullptr) {
        m_changes       = std::make_unique<Changes>(*m_changeStream, m_invert);
        m_currentChange = m_changes->begin();
    } else if (m_currentChange.IsValid()) {
        ++m_currentChange;
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void InternalChangeIterator::Reset() {
    m_currentChange = Changes::Change(nullptr, false);
    m_changes       = nullptr;
    m_changeStream  = nullptr; // must be destroyed before we delete any temp file it may be reading
    m_invert        = false;
}

//=============================================================================
// PreparedChangesetReader
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PreparedChangesetReader::PreparedChangesetReader()
    : m_ecdb(nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::OpenChangesetFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert, PropertyFilter mode) {
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

    return Open(ecdb, std::move(reader), invert, mode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::Open(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert, PropertyFilter propertyFilter) {
    if(IsOpen()) {
        LOG.errorv("Attempting to open on an already open PreparedChangesetReader.");
        return BE_SQLITE_ERROR;
    }

    if (changeStream == nullptr)
        return BE_SQLITE_ERROR;

    m_filters.m_propertyFilter = propertyFilter;
    m_iterator.Open(std::move(changeStream), invert);
    m_ecdb = &ecdb;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::OpenInMemoryChangeset(ECDbCR ecdb, std::unique_ptr<ChangeSet> changeSet, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
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
        if (BE_SQLITE_OK != WriteGroupToFile(ecdb, tempGroup, emptyDdl, tempGroup.ContainsEcSchemaChanges(), m_tempGroupFile))
            return BE_SQLITE_ERROR;
        tempGroup.Finalize(); // release the ChangeGroup RAM before opening the file
        return OpenChangesetFile(ecdb, m_tempGroupFile.GetNameUtf8(), invert, propertyFilter);
    }

    return Open(ecdb, std::move(changeSet), invert, propertyFilter);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::OpenChangeGroup(ECDbCR ecdb, T_Utf8StringVector const& files, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    if (IsOpen()) {
        LOG.errorv("Attempting to open a group on an already open PreparedChangesetReader.");
        return BE_SQLITE_ERROR;
    }

    ChangeGroup group(ecdb);
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
        if (BE_SQLITE_OK != WriteGroupToFile(ecdb, group, groupDdl, containsSchemaChanges, m_tempGroupFile))
            return BE_SQLITE_ERROR;
        group.Finalize(); // release the ChangeGroup RAM before opening the file
        return OpenChangesetFile(ecdb, m_tempGroupFile.GetNameUtf8(), invert, propertyFilter);
    }

    group.Finalize(); // group is fully materialized into cs; release the ChangeGroup RAM
    return OpenInMemoryChangeset(ecdb, std::move(cs), invert, propertyFilter, spillThreshold);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::WriteGroupToFile(ECDbCR ecdb, ChangeGroup& changeGroup, DdlChanges const& ddlChanges, bool containsSchemaChanges, BeFileNameR outPath) {
    BeGuid guid(true);
    BeFileName tempPath(Utf8String(ecdb.GetTempFileBaseName() + "-" + guid.ToString() + "-merged.changeset").c_str());

    if(tempPath.DoesPathExist()) {
        LOG.errorv("WriteGroupToFile: temp file '%s' already exists.", tempPath.GetNameUtf8().c_str());
        return BE_SQLITE_ERROR;
    }

    // Level 1 compression with a small dictionary: this file is ephemeral so speed beats ratio.
    LzmaEncoder::LzmaParams fastParams(1 << 16, false, 1, 1);
    ChangesetFileWriter writer(tempPath, containsSchemaChanges, ddlChanges, &ecdb, fastParams);

    if (BE_SQLITE_OK != writer.Initialize()) {
        LOG.errorv("WriteGroupToFile: failed to initialize ChangesetFileWriter for '%s'.", tempPath.GetNameUtf8().c_str());
        return BE_SQLITE_ERROR;
    }

    // Always write via FromChangeGroup so that DDL and schema-change flags flow through correctly.
    if(BE_SQLITE_OK != writer.FromChangeGroup(changeGroup)) {
        LOG.errorv("WriteGroupToFile: failed to write change group to temp file '%s'.", tempPath.GetNameUtf8().c_str());
        return BE_SQLITE_ERROR;
    }

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
    m_oldFields.clear();
    m_newFields.clear();
    m_columnValues.clear();
    m_changedPropNames.clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::Close() {
    ClearMembers();

    if (!m_tempGroupFile.IsEmpty() && m_tempGroupFile.DoesPathExist()) {
        if(m_tempGroupFile.BeDeleteFile() != BeFileNameStatus::Success) {
            LOG.errorv("Failed to delete temporary changeset file '%s'.", m_tempGroupFile.GetNameUtf8().c_str());
            return ERROR;
        }
    }
    m_tempGroupFile = BeFileName{};
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedChangesetReader::CloseInfallible() {
    ClearMembers();

    if (!m_tempGroupFile.IsEmpty() && m_tempGroupFile.DoesPathExist()) {
        if(m_tempGroupFile.BeDeleteFile() != BeFileNameStatus::Success) {
            LOG.errorv("Failed to delete temporary changeset file '%s'.", m_tempGroupFile.GetNameUtf8().c_str());
        }
    }
    m_tempGroupFile = BeFileName{};
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedChangesetReader::ClearMembers() {
    ClearFields();
    m_filters.Reset();
    m_iterator.Reset();
    m_columnCache.Clear();
    m_ecdb = nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECDb const* PreparedChangesetReader::GetECDb() const {
    if(!IsOpen()) {
        LOG.errorv("Attempting to get ECDb from a closed ChangesetReader.");
        return nullptr;
    }
    return m_ecdb;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedChangesetReader::Step() {
    if(!IsOpen()) {
        LOG.errorv("Attempting to step a closed ChangesetReader.");
        return BE_SQLITE_ERROR;
    }
    bool isCurrentRowFilteredOut = false;
    DbResult stat = BE_SQLITE_OK;
    do {
        isCurrentRowFilteredOut = false;
        ClearFields();
        m_iterator.Advance();
        stat = m_iterator.GetCurrentChange().IsValid() ? BE_SQLITE_ROW : BE_SQLITE_DONE;
        if(stat == BE_SQLITE_DONE) return stat;
        if(ReFetchValues(isCurrentRowFilteredOut) != SUCCESS) {
            ClearFields();
            return BE_SQLITE_ERROR;
        }

        if(isCurrentRowFilteredOut)
            LOG.infov("Current change is filtered out. Stepping to the next change.");
    } while(isCurrentRowFilteredOut);

    return stat;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PreparedChangesetReader::StageProcessResult PreparedChangesetReader::ProcessStageValues(Stage stage, DbTable const& dbTable, std::vector<Utf8String>* changedPropNames) {
    if (GetColumnValues(stage) != SUCCESS)
        return StageProcessResult::Error;

    ECClassId classId;
    bool isClassIdFromChangeset = false;
    if (ChangesetValueFactory::ResolveClassId(*m_ecdb, dbTable, m_columnValues, classId, isClassIdFromChangeset) != SUCCESS)
        return StageProcessResult::Error;
    ECClassCP ecClass = m_ecdb->Schemas().Main().GetClass(classId);
    if (ecClass == nullptr) {
        LOG.errorv("ECClass with id %" PRIu64 " not found in schema.", classId.GetValueUnchecked());
        return StageProcessResult::Error;
    }
    Utf8String className = ecClass->GetFullName();
    if (!m_filters.IsECClassNameAllowed(className)) {
        LOG.infov("ECClass '%s' is not allowed by filters. Skipping creating fields", className.c_str());
        return StageProcessResult::Filtered;
    }

    Utf8String tableName;
    if (GetTableName(tableName) != SUCCESS)
        return StageProcessResult::Error;
    int dbColumnCount = 0;
    if (GetColumnCountForCurrentChangedTable(dbColumnCount, tableName) != SUCCESS)
        return StageProcessResult::Error;
    if (m_filters.CheckColumnCount(m_iterator.GetCurrentChange().GetColumnCount(), dbColumnCount, tableName) != SUCCESS)
        return StageProcessResult::Error;

    if (ChangesetValueFactory::Create(*m_ecdb, dbTable, m_columnValues, classId, isClassIdFromChangeset, (stage == Stage::New) ? m_newFields : m_oldFields, m_filters.m_propertyFilter, changedPropNames) != SUCCESS)
        return StageProcessResult::Error;
    return StageProcessResult::Success;
}

//---------------------------------------------------------------------------------------
// @bsimethod
// In case the current row does not fit the filters, we will not create fields for it. When users
// call GetColumnValue, the number of rows returned will be 0. As the current API steps row by row,
// and after stepping onto a row we come to know whether the row is filtered out or not, we skip
// the expensive field creation step when the row is filtered.
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::ReFetchValues(bool& isCurrentRowFilteredOut) {
    isCurrentRowFilteredOut = false;
    if (!m_iterator.GetCurrentChange().IsValid()) {
        LOG.errorv("Attempting to re-fetch values for an invalid change.");
        return ERROR;
    }
    DbOpcode opCode;
    if(GetOpcode(opCode) != SUCCESS)
        return ERROR;

    if(!m_filters.IsOpcodeAllowed(opCode)) { // First is opCode filter
        LOG.infov("Opcode '%s' is not allowed by filters. Skipping creating fields", ChangesetFilterContext::OpcodeToString(opCode).c_str());
        isCurrentRowFilteredOut = true;
        return SUCCESS;
    }

    Utf8String tableName;
    if(GetTableName(tableName) != SUCCESS)
        return ERROR;

    if(!m_filters.IsTableAllowed(tableName)) { // second is table filter
        LOG.infov("Table '%s' is not allowed by filters. Skipping creating fields", tableName.c_str());
        isCurrentRowFilteredOut = true;
        return SUCCESS;
    }

    bool isECTable = false;
    if(IsECTable(isECTable) != SUCCESS)
        return ERROR;
    if(!isECTable) {
        LOG.infov("Table '%s' is not an EC table. Skipping creating fields", tableName.c_str());
        return SUCCESS;
    }

    DbTable const* dbTable = m_ecdb->Schemas().Main().GetDbSchema().FindTable(tableName);
    if (dbTable == nullptr) {
        LOG.errorv("Table '%s' not found in schema.", tableName.c_str());
        return ERROR;
    }

    if(opCode != DbOpcode::Delete) {
        auto result = ProcessStageValues(Stage::New, *dbTable, &m_changedPropNames);
        m_columnValues.clear(); // Clear column values after processing the New stage
        if (result == StageProcessResult::Error) return ERROR;
        if (result == StageProcessResult::Filtered) { isCurrentRowFilteredOut = true; return SUCCESS; }
    }
    if(opCode != DbOpcode::Insert) {
        auto result = ProcessStageValues(Stage::Old, *dbTable, (opCode == DbOpcode::Update) ? nullptr : &m_changedPropNames);
        m_columnValues.clear(); // Clear column values after processing the Old stage
        if (result == StageProcessResult::Error) return ERROR;
        if (result == StageProcessResult::Filtered) { isCurrentRowFilteredOut = true; return SUCCESS; }
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::GetColumnValues(Stage stage) {
    if(!IsStepped()) {
        LOG.errorv("Attempting to get column values from a ChangesetReader that is either not open or not stepped or has finished stepping and has reached the end.");
        return ERROR;
    }
    Utf8String tableName;
    if (GetTableName(tableName) != SUCCESS)
        return ERROR;

    auto const* orderedNames = m_columnCache.GetOrderedColumnNames(*m_ecdb, tableName);
    if (orderedNames == nullptr) {
        LOG.errorv("Failed to get column names for table '%s'.", tableName.c_str());
        return ERROR;
    }

    int minimum = std::min((int)orderedNames->size(), m_iterator.GetCurrentChange().GetColumnCount());
    m_columnValues.clear();
    for (int colIdx = 0; colIdx < minimum; ++colIdx) {
        Utf8CP colName = orderedNames->at(colIdx).c_str();
        DbValue val = m_iterator.GetCurrentChange().GetValue(colIdx, stage);
        if (!val.IsValid() && m_iterator.GetCurrentChange().IsPrimaryKeyColumn(colIdx)) {
            // SQLite changesets store PK column values only in the Old slot, even for UPDATE.
            val = m_iterator.GetCurrentChange().GetOldValue(colIdx);
        }
        if (val.IsValid())
            m_columnValues.emplace(colName, val);
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::GetColumnCountForCurrentChangedTable(int& columnCount, Utf8StringCR tableName) {
    auto const* orderedNames = m_columnCache.GetOrderedColumnNames(*m_ecdb, tableName);
    if (orderedNames == nullptr) {
        LOG.errorv("Failed to get column names for table '%s'.", tableName.c_str());
        return ERROR;
    }
    columnCount = (int)orderedNames->size();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedChangesetReader::DumpColumnValues(std::unordered_map<Utf8String, DbValue> const& map) const {
    for (auto const& [key, val] : map)
        LOG.debugv("%s = %s", key.c_str(), val.IsNull() ? "NULL" : (val.GetValueText() ? val.GetValueText() : "(blob)"));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int PreparedChangesetReader::GetColumnCount(Stage stage) const {
    if(!IsStepped()) {
        LOG.errorv("Attempting to get column count from a ChangesetReader that is either not open or not stepped or has finished stepping and has reached the end.");
        return 0;
    }
    return stage == Stage::New ? static_cast<int>(m_newFields.size()) : static_cast<int>(m_oldFields.size());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::GetTableName(Utf8StringR tableName) const {
    if(!IsStepped()) {
        LOG.errorv("Attempting to get table name from a PreparedChangesetReader that is either not open or not stepped or has finished stepping and has reached the end.");
        return ERROR;
    }
    tableName = m_iterator.GetCurrentChange().GetTableName();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::GetOpcode(DbOpcode& opcode) const {
    if(!IsStepped()) {
        LOG.errorv("Attempting to get opcode from a ChangesetReader that is either not open or not stepped or has finished stepping and has reached the end.");
        return ERROR;
    }
    opcode = m_iterator.GetCurrentChange().GetOpcode();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& PreparedChangesetReader::GetValue(Stage stage, int columnIndex) const {
    if(!IsStepped()) {
        LOG.errorv("Attempting to get value from a ChangesetReader that is either not open or not stepped or has finished stepping and has reached the end.");
        return NoopECSqlValue::GetSingleton();
    }
    int size = GetColumnCount(stage);
    if (columnIndex < 0 || columnIndex >= size) {
        LOG.errorv("Column index %d is out of range for table.", columnIndex);
        return NoopECSqlValue::GetSingleton();
    }
    return stage == Stage::New ? *m_newFields.at(columnIndex) : *m_oldFields.at(columnIndex);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::GetInstanceKey(Stage stage, Utf8StringR key) const {
    if(!IsStepped()) {
        LOG.errorv("Attempting to get instance key from a ChangesetReader that is either not open or not stepped or has finished stepping and has reached the end.");
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
    if(!IsStepped()) {
        LOG.errorv("Attempting to check IsECTable on a ChangesetReader that is either not open or not stepped or has finished stepping and has reached the end.");
        return ERROR;
    }
    Utf8String tableName;
    if(GetTableName(tableName) != SUCCESS)
        return ERROR;
    CachedStatementPtr stmt = m_ecdb->GetCachedStatement("SELECT 1 FROM ec_Table WHERE Name=?");
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
std::vector<Utf8String> const* PreparedChangesetReader::GetChangeFetchedPropertyNames() const {
    if(!IsStepped()) {
        LOG.errorv("Attempting to get changed property names from a ChangesetReader that is either not open or not stepped or has finished stepping and has reached the end.");
        return nullptr;
    }
    return &m_changedPropNames;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::IsIndirectChange(bool& isIndirect) const {
    if(!IsStepped()) {
        LOG.errorv("Attempting to check IsIndirectChange on a ChangesetReader that is either not open or not stepped or has finished stepping and has reached the end.");
        return ERROR;
    }
    isIndirect = m_iterator.GetCurrentChange().IsIndirect();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::SetTableFilters(std::vector<Utf8String> const& tableFilters) {
    if(!IsOpen()) {
        LOG.errorv("Attempting to set table filters on a ChangesetReader that is not open.");
        return ERROR;
    }
    m_filters.m_tableFilters = tableFilters;
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters) {
    if(!IsOpen()) {
        LOG.errorv("Attempting to set opcode filters on a ChangesetReader that is not open.");
        return ERROR;
    }
    m_filters.m_opcodeFilters = opcodeFilters;
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::SetECClassNameFilters(std::vector<Utf8String> const& ecclassNameFilters) {
    if(!IsOpen()) {
        LOG.errorv("Attempting to set EC class name filters on a ChangesetReader that is not open.");
        return ERROR;
    }
    m_filters.m_ecclassNameFilters = ecclassNameFilters;
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::ClearTableFilters() {
    if(!IsOpen()) {
        LOG.errorv("Attempting to clear table filters on a ChangesetReader that is not open.");
        return ERROR;
    }
    m_filters.m_tableFilters.clear();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::ClearOpcodeFilters() {
    if(!IsOpen()) {
        LOG.errorv("Attempting to clear opcode filters on a ChangesetReader that is not open.");
        return ERROR;
    }
    m_filters.m_opcodeFilters.clear();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::ClearECClassNameFilters() {
    if(!IsOpen()) {
        LOG.errorv("Attempting to clear EC class name filters on a ChangesetReader that is not open.");
        return ERROR;
    }
    m_filters.m_ecclassNameFilters.clear();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::EnableStrictMode() {
    if(!IsOpen()) {
        LOG.errorv("Attempting to enable strict mode on a ChangesetReader that is not open.");
        return ERROR;
    }
    m_filters.m_strictMode = true;
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PreparedChangesetReader::DisableStrictMode() {
    if(!IsOpen()) {
        LOG.errorv("Attempting to disable strict mode on a ChangesetReader that is not open.");
        return ERROR;
    }
    m_filters.m_strictMode = false;
    return SUCCESS;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
