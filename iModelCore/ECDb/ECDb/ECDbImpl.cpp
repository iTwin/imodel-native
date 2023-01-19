/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

PragmaManager& ECDb::Impl::GetPragmaManager() const
    {
    if (m_pragmaProcessor == nullptr)
        {
        m_pragmaProcessor = std::make_unique<PragmaManager>(m_ecdb);
        }
    return *m_pragmaProcessor;
    }

ECDb::Impl::Impl(ECDbR ecdb) : m_ecdb(ecdb), m_profileManager(ecdb), m_changeManager(ecdb), m_sqliteStatementCache(50, &m_mutex), m_idSequenceManager(ecdb, bvector<Utf8CP>(1, "ec_instanceidsequence"))
    {
    m_schemaManager = std::make_unique<SchemaManager>(ecdb, m_mutex);
    m_viewManager = std::make_unique<ViewManager> (ecdb);
    // set default logger
    IssueDataSource::AppendLogSink(m_issueReporter, "ECDb");
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IdFactory::IdSequence> IdFactory::IdSequence::Create(ECDbCR db, Utf8CP tableName, Utf8CP idColumnName) {
    BeAssert(!Utf8String::IsNullOrEmpty(tableName));
    BeAssert(!Utf8String::IsNullOrEmpty(idColumnName));
    BeAssert(db.IsDbOpen());

    if (db.TableExists(tableName)) {
        Statement stmt;
        DbResult rc = stmt.Prepare(db, SqlPrintfString("select max([%s]) from [%s]", idColumnName, tableName));
        if (rc != BE_SQLITE_OK) {
            BeAssert(false && "Unable to prepare id statement");
            return nullptr;
        }
        rc = stmt.Step();
        if (rc != BE_SQLITE_ROW) {
            BeAssert(false && "Unable to step id statement");
            return nullptr;
        }
        uint64_t id = 0;
        if (!stmt.IsColumnNull(0)) {
            id = stmt.GetValueUInt64(0);
        }
        return std::make_unique<IdSequence>(id, true);
    }
    LOG.debugv("Unable initialize sequence for '%s' not supported in current profile.", tableName);
    return std::make_unique<IdSequence>(0, false);
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
IdFactory::IdFactory(ECDbCR ecdb): m_ecdb(ecdb) {
    Reset();
}
//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool IdFactory::Reset() const {
    if (!m_ecdb.IsDbOpen()) {
        BeAssert(false && "ECDb is closed");
        return false;
    }
    m_classIdSeq = IdSequence::Create(m_ecdb, TABLE_Class, COL_DEFAULTNAME_Id);
    m_classHasBaseClassesIdSeq = IdSequence::Create(m_ecdb, TABLE_ClassHasBaseClasses, COL_DEFAULTNAME_Id);
    m_columnIdSeq = IdSequence::Create(m_ecdb, TABLE_Column, COL_DEFAULTNAME_Id);
    m_customAttributeIdSeq = IdSequence::Create(m_ecdb, TABLE_CustomAttribute, COL_DEFAULTNAME_Id);
    m_enumerationIdSeq = IdSequence::Create(m_ecdb, TABLE_Enumeration, COL_DEFAULTNAME_Id);
    m_formatIdSeq = IdSequence::Create(m_ecdb, TABLE_Format, COL_DEFAULTNAME_Id);
    m_formatCompositeUnitIdSeq = IdSequence::Create(m_ecdb, TABLE_FormatCompositeUnit, COL_DEFAULTNAME_Id);
    m_indexIdSeq = IdSequence::Create(m_ecdb, TABLE_Index, COL_DEFAULTNAME_Id);
    m_indexColumnIdSeq = IdSequence::Create(m_ecdb, TABLE_IndexColumn, COL_DEFAULTNAME_Id);
    m_kindOfQuantityIdSeq = IdSequence::Create(m_ecdb, TABLE_KindOfQuantity, COL_DEFAULTNAME_Id);
    m_phenomenonIdSeq = IdSequence::Create(m_ecdb, TABLE_Phenomenon, COL_DEFAULTNAME_Id);
    m_propertyIdSeq = IdSequence::Create(m_ecdb, TABLE_Property, COL_DEFAULTNAME_Id);
    m_propertyCategoryIdSeq = IdSequence::Create(m_ecdb, TABLE_PropertyCategory, COL_DEFAULTNAME_Id);
    m_propertyMapSeq = IdSequence::Create(m_ecdb, TABLE_PropertyMap, COL_DEFAULTNAME_Id);
    m_propertyPathIdSeq = IdSequence::Create(m_ecdb, TABLE_PropertyPath, COL_DEFAULTNAME_Id);
    m_relationshipConstraintIdSeq = IdSequence::Create(m_ecdb, TABLE_RelationshipConstraint, COL_DEFAULTNAME_Id);
    m_relationshipConstraintClassIdSeq = IdSequence::Create(m_ecdb, TABLE_RelationshipConstraintClass, COL_DEFAULTNAME_Id);
    m_schemaIdSeq = IdSequence::Create(m_ecdb, TABLE_Schema, COL_DEFAULTNAME_Id);
    m_schemaReferenceIdSeq = IdSequence::Create(m_ecdb, TABLE_SchemaReference, COL_DEFAULTNAME_Id);
    m_tableIdSeq = IdSequence::Create(m_ecdb, TABLE_Table, COL_DEFAULTNAME_Id);
    m_unitIdSeq = IdSequence::Create(m_ecdb, TABLE_Unit, COL_DEFAULTNAME_Id);
    m_unitSystemIdSeq = IdSequence::Create(m_ecdb, TABLE_UnitSystem, COL_DEFAULTNAME_Id);
    return IsValid();
}
//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool IdFactory::IsValid() const {
    const bool valid =
        m_classIdSeq != nullptr &&
        m_classHasBaseClassesIdSeq != nullptr &&
        m_columnIdSeq != nullptr &&
        m_customAttributeIdSeq != nullptr &&
        m_enumerationIdSeq != nullptr &&
        m_formatIdSeq != nullptr &&
        m_formatCompositeUnitIdSeq != nullptr &&
        m_indexIdSeq != nullptr &&
        m_indexColumnIdSeq != nullptr &&
        m_kindOfQuantityIdSeq != nullptr &&
        m_phenomenonIdSeq != nullptr &&
        m_propertyIdSeq != nullptr &&
        m_propertyCategoryIdSeq != nullptr &&
        m_propertyMapSeq != nullptr &&
        m_propertyPathIdSeq != nullptr &&
        m_relationshipConstraintIdSeq != nullptr &&
        m_relationshipConstraintClassIdSeq != nullptr &&
        m_schemaIdSeq != nullptr &&
        m_schemaReferenceIdSeq != nullptr &&
        m_tableIdSeq != nullptr &&
        m_unitIdSeq != nullptr &&
        m_unitSystemIdSeq != nullptr;

    BeAssert(valid);
    return valid;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<IdFactory> IdFactory::Create(ECDbCR ecdb) {
    auto idseq = std::make_unique<IdFactory>(ecdb);
    if (!idseq->IsValid()) {
        return nullptr;
    }

    return idseq;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
IdFactory& ECDb::Impl::GetIdFactory() const {
    if (m_idFactory == nullptr) {
        m_idFactory = IdFactory::Create(m_ecdb);
        BeAssert(m_idFactory != nullptr);
    }
    return *m_idFactory;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDb::Impl::s_isInitialized = false;

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnDbCreated() const
    {
    m_id.Create();
    RegisterBuiltinFunctions();

    DbResult stat = m_idSequenceManager.InitializeSequences();
    if (BE_SQLITE_OK != stat)
        return stat;

    //set initial value of sequence to current briefcase id.
    stat = m_idSequenceManager.ResetSequences();
    if (BE_SQLITE_OK != stat)
        return stat;

    return m_profileManager.CreateProfile();
    }


//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnDbOpening() const
    {
    m_id.Create();
    RegisterBuiltinFunctions();
    return m_idSequenceManager.InitializeSequences();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnDbAttached(Utf8CP dbFileName, Utf8CP tableSpaceName) const
    {
    DbTableSpace tableSpace(tableSpaceName, dbFileName);
    if (!DbTableSpace::IsAttachedECDbFile(m_ecdb, tableSpaceName))
        return BE_SQLITE_OK; //only need to react to attached ECDb files

    if (SUCCESS != m_schemaManager->GetDispatcher().AddManager(tableSpace))
        return BE_SQLITE_ERROR;

    GetChangeManager().OnDbAttached(tableSpace, dbFileName);
    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnDbDetached(Utf8CP tableSpaceName) const
    {
    DbTableSpace tableSpace(tableSpaceName);
    if (SUCCESS != m_schemaManager->GetDispatcher().RemoveManager(tableSpace))
        return BE_SQLITE_ERROR;

    ClearECDbCache();

    GetChangeManager().OnDbDetached(tableSpace);
    return BE_SQLITE_OK;
    }


//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ECDb::Impl::TryGetChangeCacheFileName(BeFileNameR changeCachePath) const {
    return GetChangeManager().TryGetChangeCacheFileName(changeCachePath);
}
//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnBriefcaseIdAssigned(BeBriefcaseId newBriefcaseId)
    {
    if (m_ecdb.IsReadonly())
        return BE_SQLITE_READONLY;

    const DbResult stat = m_idSequenceManager.ResetSequences(&newBriefcaseId);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("Changing briefcase id to %" PRIu32 " in file '%s' failed because ECDb's id sequences could not be reset.",
                   newBriefcaseId.GetValue(),
                   m_ecdb.GetDbFileName());
        }

    if (BSISUCCESS != ResetInstanceIdSequence(newBriefcaseId, nullptr)) // We may be restoring an existing, populated briefcase. If it is a checkpoint, there may be no changesets to follow.
        return BE_SQLITE_ERROR;

    return stat;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Impl::ResetInstanceIdSequence(BeBriefcaseId briefcaseId, IdSet<ECN::ECClassId> const* ecClassIgnoreList)
    {
    if (!briefcaseId.IsValid() || m_ecdb.IsReadonly())
        return ERROR;

    //ECInstanceId sequence. It has to compute the current max ECInstanceId across all EC data tables
    ECInstanceId maxECInstanceId;
    if (SUCCESS != DetermineMaxInstanceIdForBriefcase(maxECInstanceId, briefcaseId, ecClassIgnoreList))
        {
        LOG.errorv("Changing BriefcaseId to %" PRIu32 " failed: The maximum for the ECInstanceId sequence for the new BriefcaseId could not be determined.",
                   briefcaseId.GetValue());
        return ERROR;
        }

    if (BE_SQLITE_OK != GetInstanceIdSequence().Reset(maxECInstanceId.GetValueUnchecked()))
        {
        LOG.errorv("Changing BriefcaseId to %" PRIu32 " failed: The ECInstanceId sequence could not be reset to the new BriefcaseId.",
                   briefcaseId.GetValue());
        return ERROR;
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Impl::DetermineMaxInstanceIdForBriefcase(ECInstanceId& maxId, BeBriefcaseId briefcaseId, IdSet<ECN::ECClassId> const* ecClassIgnoreList) const
    {
    if (!briefcaseId.IsValid())
        return ERROR;

    Statement primaryTableStmt;
    if (BE_SQLITE_OK != primaryTableStmt.Prepare(m_ecdb, "SELECT t.Id, t.Name, c.Name FROM main." TABLE_Table " t JOIN main." TABLE_Column " c ON t.Id=c.TableId WHERE t.Type=" SQLVAL_DbTable_Type_Primary " AND c.ColumnKind=" SQLVAL_DbColumn_Kind_ECInstanceId))
        return ERROR;

    Statement ignoreTableStmt;
    if (ecClassIgnoreList != nullptr)
        {
        if (BE_SQLITE_OK != ignoreTableStmt.Prepare(m_ecdb, "SELECT 1 FROM main." TABLE_Class " c JOIN main." TABLE_PropertyMap " pm ON c.Id=pm.ClassId "
                                                    "JOIN main." TABLE_Column " col ON col.Id=pm.ColumnId "
                                                    "WHERE col.TableId=? AND InVirtualSet(?,c.Id) LIMIT 1"))
            return ERROR;
        }


    BeBriefcaseBasedId maxIdTemp(briefcaseId, 0);
    while (BE_SQLITE_ROW == primaryTableStmt.Step())
        {
        DbTableId tableId = primaryTableStmt.GetValueId<DbTableId>(0);
        Utf8CP tableName = primaryTableStmt.GetValueText(1);
        Utf8CP pkColName = primaryTableStmt.GetValueText(2);

        if (ecClassIgnoreList != nullptr)
            {
            if (BE_SQLITE_OK != ignoreTableStmt.BindId(1, tableId))
                return ERROR;

            if (BE_SQLITE_OK != ignoreTableStmt.BindVirtualSet(2, *ecClassIgnoreList))
                return ERROR;

            const bool ignoreTable = ignoreTableStmt.Step() == BE_SQLITE_ROW;
            ignoreTableStmt.Reset();
            ignoreTableStmt.ClearBindings();

            if (ignoreTable)
                continue;
            }

        BeBriefcaseBasedId tableMaxId;
        if (SUCCESS != DetermineMaxIdForBriefcase(tableMaxId, briefcaseId, tableName, pkColName))
            return ERROR;

        if (tableMaxId > maxIdTemp)
            maxIdTemp = tableMaxId;
        }

    maxId = ECInstanceId(maxIdTemp);
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Impl::DetermineMaxIdForBriefcase(BeBriefcaseBasedId& maxId, BeBriefcaseId briefcaseId, Utf8CP tableName, Utf8CP idColName) const
    {
    if (!briefcaseId.IsValid())
        return ERROR;

    Utf8String sql;
    sql.Sprintf("SELECT max([%s]) FROM [%s] WHERE [%s] >= ? AND [%s] < ?", idColName, tableName, idColName, idColName);

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, sql.c_str()))
        return ERROR;

    //bind min id for briefcase id
    const BeBriefcaseBasedId minIdThreshold(briefcaseId, 0);
    //bind exclusive max id for briefcase id
    const BeBriefcaseBasedId maxIdThreshold(briefcaseId.GetNextBriefcaseId(), 0);
    if (BE_SQLITE_OK != stmt.BindUInt64(1, minIdThreshold.GetValueUnchecked()) || BE_SQLITE_OK != stmt.BindUInt64(2, maxIdThreshold.GetValueUnchecked()))
        return ERROR;

    const DbResult stat = stmt.Step();
    switch (stat)
        {
            case BE_SQLITE_ROW:
                if (stmt.IsColumnNull(0))
                    maxId = minIdThreshold;
                else
                    maxId = stmt.GetValueId<BeBriefcaseBasedId>(0);

                return SUCCESS;

            case BE_SQLITE_DONE:
                maxId = minIdThreshold;
                return SUCCESS;

            default:
                maxId.Invalidate();
                return ERROR;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
CachedStatementPtr ECDb::Impl::GetCachedSqliteStatement(Utf8CP sql) const
    {
    if (!m_ecdb.IsDbOpen())
        return nullptr;

    BeAssert(m_ecdb.GetDbFile() != nullptr);
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_sqliteStatementCache.GetPreparedStatement(stmt, *m_ecdb.GetDbFile(), sql))
        return nullptr;

    return stmt;
    }


//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::ClearECDbCache() const
    {
    BeMutexHolder lock(m_mutex);

    if (m_viewManager != nullptr) {
        m_viewManager->ClearCache();
    }

    // this event allows consuming code to free anything that relies on the ECDb cache (like ECSchemas, ECSqlStatements etc)
    for (auto listener : m_ecdbCacheClearListeners)
        listener->_OnBeforeClearECDbCache();

    for (AppData::Key const* appDataKey : m_appDataToDeleteOnClearCache)
        {
        m_ecdb.DropAppData(*appDataKey);
        }

    if (m_schemaManager != nullptr)
        m_schemaManager->ClearCache();

    const_cast<ChangeManager&>(m_changeManager).ClearCache();
    const_cast<StatementCache&>(m_sqliteStatementCache).Empty();

    //increment the counter. This allows code (e.g. ECSqlStatement) that depends on objects in the cache to invalidate itself
    //after the cache was cleared.
    m_clearCacheCounter.Increment();

    for (auto listener : m_ecdbCacheClearListeners)
        listener->_OnAfterClearECDbCache();

    STATEMENT_DIAGNOSTICS_LOGCOMMENT("After ECDb::ClearECDbCache");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Impl::OnAddFunction(DbFunction& function) const
    {
    BeMutexHolder lock(m_mutex);
    DbFunctionKey key(function);
    m_sqlFunctions[key] = &function;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::OnRemoveFunction(DbFunction& function) const
    {
    BeMutexHolder lock(m_mutex);
    DbFunctionKey key(function);
    m_sqlFunctions.erase(key);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDb::Impl::TryGetSqlFunction(DbFunction*& function, Utf8CP name, int argCount) const
    {
    BeMutexHolder lock(m_mutex);
    auto it = m_sqlFunctions.find(DbFunctionKey(name, argCount));
    if (it == m_sqlFunctions.end())
        {
        function = nullptr;
        return false;
        }

    function = it->second;
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bvector<DbFunction*> ECDb::Impl::GetSqlFunctions() const {
    bvector<DbFunction*> funcs;
    for (auto& it : m_sqlFunctions) {
        funcs.push_back(it.second);
    }
    return funcs;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::RegisterBuiltinFunctions() const
    {
    m_ecdb.AddFunction(GuidToStr::GetSingleton());
    m_ecdb.AddFunction(StrToGuid::GetSingleton());
    m_ecdb.AddFunction(IdToHex::GetSingleton());
    m_ecdb.AddFunction(HexToId::GetSingleton());
    m_changeManager.RegisterSqlFunctions();

    m_classNameFunc = ClassNameFunc::Create(m_ecdb);
    if (m_classNameFunc != nullptr)
        m_ecdb.AddFunction(*m_classNameFunc);

    m_classIdFunc = ClassIdFunc::Create(m_ecdb);
    if (m_classIdFunc != nullptr)
        m_ecdb.AddFunction(*m_classIdFunc);

    m_instanceOfFunc = InstanceOfFunc::Create(m_ecdb);
    if (m_instanceOfFunc != nullptr)
        m_ecdb.AddFunction(*m_instanceOfFunc);

    m_ecJsonFunc = ECJsonFunction::Create(m_ecdb);
    if (m_ecJsonFunc != nullptr)
        m_ecdb.AddFunction(*m_ecJsonFunc);

    m_xmlCAToJsonFunc = XmlCAToJson::Create(m_ecdb.Schemas());
    if (m_xmlCAToJsonFunc != nullptr)
        m_ecdb.AddFunction(*m_xmlCAToJsonFunc);
   }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::UnregisterBuiltinFunctions() const
    {
    if (!m_ecdb.IsDbOpen())
        return;

    m_ecdb.RemoveFunction(GuidToStr::GetSingleton());
    m_ecdb.RemoveFunction(StrToGuid::GetSingleton());
    m_ecdb.RemoveFunction(IdToHex::GetSingleton());
    m_ecdb.RemoveFunction(HexToId::GetSingleton());
    m_changeManager.UnregisterSqlFunction();

    if (m_classNameFunc != nullptr)
        {
        m_ecdb.RemoveFunction(*m_classNameFunc);
        m_classNameFunc = nullptr;
        }
    if (m_classIdFunc != nullptr)
        {
        m_ecdb.RemoveFunction(*m_classIdFunc);
        m_classIdFunc = nullptr;
        }
    if (m_instanceOfFunc != nullptr)
        {
        m_ecdb.RemoveFunction(*m_instanceOfFunc);
        m_instanceOfFunc = nullptr;
        }

    if (m_ecJsonFunc != nullptr)
        {
        m_ecdb.RemoveFunction(*m_ecJsonFunc);
        m_ecJsonFunc = nullptr;
        }

    if (m_xmlCAToJsonFunc != nullptr)
        {
        m_ecdb.RemoveFunction(*m_xmlCAToJsonFunc);
        m_xmlCAToJsonFunc = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Impl::Purge(ECDb::PurgeMode mode) const
    {
    BeMutexHolder lock(m_mutex);
    if (Enum::Contains(mode, ECDb::PurgeMode::FileInfoOwnerships))
        {
        if (SUCCESS != PurgeFileInfos())
            return ERROR;
        }

    return SUCCESS;
    }

#define ECDBF_FILEINFOOWNERSHIP_FULLCLASSNAME "ecdbf.FileInfoOwnership"

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Impl::PurgeFileInfos() const
    {
    bvector<ECClassId> ownerClassIds;
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, "SELECT DISTINCT OwnerECClassId FROM " ECDBF_FILEINFOOWNERSHIP_FULLCLASSNAME))
        return ERROR;

    while (BE_SQLITE_ROW == stmt.Step())
        {
        BeAssert(!stmt.IsValueNull(0) && "OwnerECClassId must not be null as enforced in ECSchema");
        ownerClassIds.push_back(stmt.GetValueId<ECClassId>(0));
        }
    }

    if (ownerClassIds.empty())
        return SUCCESS; // ownership class is empty -> nothing to purge

    //Step 1: Purge ownership class from records for which owner doesn't exist anymore
    Utf8String purgeOwnershipByOwnersECSql("DELETE FROM " ECDBF_FILEINFOOWNERSHIP_FULLCLASSNAME " WHERE ");
    bool isFirstOwnerClassId = true;
    for (ECClassId ownerClassId : ownerClassIds)
        {
        ECClassCP ownerClass = Schemas().GetClass(ownerClassId);
        if (ownerClass == nullptr)
            {
            m_issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "FileInfo owner ECClass not found for " ECDBSYS_PROP_ECClassId " %s.", ownerClassId.ToString().c_str());
            return ERROR;
            }

        if (!isFirstOwnerClassId)
            purgeOwnershipByOwnersECSql.append(" OR ");

        Utf8String whereSnippet;
        whereSnippet.Sprintf("(OwnerECClassId=%s AND OwnerId NOT IN (SELECT " ECDBSYS_PROP_ECInstanceId " FROM ONLY %s))", ownerClassId.ToString().c_str(), ownerClass->GetECSqlName().c_str());
        purgeOwnershipByOwnersECSql.append(whereSnippet);

        isFirstOwnerClassId = false;
        }

    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, purgeOwnershipByOwnersECSql.c_str(), m_settingsManager.GetCrudWriteToken()))
        return ERROR;

    if (BE_SQLITE_DONE != stmt.Step())
        return ERROR;

    stmt.Finalize();

    //Step 2: Purge ownership class from records for which file info doesn't exist anymore
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, "DELETE FROM " ECDBF_FILEINFOOWNERSHIP_FULLCLASSNAME " WHERE FileInfoId NOT IN (SELECT " ECDBSYS_PROP_ECInstanceId " FROM ecdbf.FileInfo)", m_settingsManager.GetCrudWriteToken()))
        return ERROR;

    return BE_SQLITE_DONE == stmt.Step() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::AddAppData(ECDb::AppData::Key const& key, ECDb::AppData* appData, bool deleteOnClearCache) const
    {
    if (deleteOnClearCache)
        m_appDataToDeleteOnClearCache.insert(&key);

    m_ecdb.AddAppData(key, appData);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Impl::OpenBlobIO(BlobIO& blobIO, Utf8CP tableSpaceName, ECN::ECClassCR ecClass, Utf8CP propertyAccessString, BeInt64Id ecinstanceId, bool writable, ECCrudWriteToken const* writeToken) const
    {
    if (blobIO.IsValid())
        return ERROR;

    Policy policy = PolicyManager::GetPolicy(ECCrudPermissionPolicyAssertion(m_ecdb, writable, writeToken));
    if (!policy.IsSupported())
        {
        m_issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, policy.GetNotSupportedMessage().c_str());
        return ERROR;
        }

    //all this method does is to determine the table and column name for the given class name and property access string.
    //the code is verbose because serious validation is done to ensure that the BlobIO is only used for ECProperties
    //that store BLOB values in a single column.
    ClassMap const* classMap = m_ecdb.Schemas().GetDispatcher().GetClassMap(ecClass, tableSpaceName);
    if (classMap == nullptr || classMap->GetType() == ClassMap::Type::NotMapped)
        {
        m_issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Cannot open BlobIO for ECProperty '%s.%s' (Table space: %s). Cannot find class map for the ECClass.",
                   ecClass.GetFullName(), propertyAccessString, Utf8String::IsNullOrEmpty(tableSpaceName) ? "any" : tableSpaceName);
        return ERROR;
        }

    PropertyMap const* propMap = classMap->GetPropertyMaps().Find(propertyAccessString);
    if (propMap == nullptr)
        {
        m_issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Cannot open BlobIO for ECProperty '%s.%s'. The ECProperty doesn't exist in the ECClass.",
                   ecClass.GetFullName(), propertyAccessString);
        return ERROR;
        }

    if (PropertyMap::Type::Primitive != propMap->GetType())
        {
        m_issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Cannot open BlobIO for ECProperty '%s.%s'. The ECProperty must be primitive and of type Binary.",
                   ecClass.GetFullName(), propertyAccessString);
        return ERROR;
        }

    BeAssert(propMap->GetProperty().GetIsPrimitive());
    const PrimitiveType primType = propMap->GetProperty().GetAsPrimitiveProperty()->GetType();
    if (primType != PRIMITIVETYPE_Binary && primType != PRIMITIVETYPE_IGeometry)
        {
        m_issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Cannot open BlobIO for ECProperty '%s.%s'. It must be either of type Binary or IGeometry.",
                   ecClass.GetFullName(), propertyAccessString);
        return ERROR;
        }

    DbColumn const& col = propMap->GetAs<PrimitivePropertyMap>().GetColumn();

    if (col.GetPersistenceType() == PersistenceType::Virtual)
        {
        m_issueReporter.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Cannot open BlobIO for ECProperty '%s.%s' as it is not mapped to a column.",
                   ecClass.GetFullName(), propertyAccessString);
        return ERROR;
        }

    return blobIO.Open(m_ecdb, col.GetTable().GetName().c_str(), col.GetName().c_str(), ecinstanceId.GetValue(), writable, tableSpaceName) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDb::Impl::InitializeLib(BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors)
    {
    if (s_isInitialized)
        return BE_SQLITE_OK;

    const DbResult stat = BeSQLiteLib::Initialize(ecdbTempDir, logSqliteErrors);
    if (stat != BE_SQLITE_OK)
        return stat;

    if (hostAssetsDir != nullptr)
        {
        if (!hostAssetsDir->DoesPathExist())
            {
            LOG.warningv("ECDb::Initialize: host assets dir '%s' does not exist.", hostAssetsDir->GetNameUtf8().c_str());
            BeAssert(false && "ECDb::Initialize: host assets dir does not exist!");
            }

        ECN::ECSchemaReadContext::Initialize(*hostAssetsDir);
        }

    s_isInitialized = true;
    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Impl::RefreshViews() const {
   return GetViewManager().RefreshViews();
}

END_BENTLEY_SQLITE_EC_NAMESPACE
