/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbImpl.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                               Affan.Khan               03/2018
//---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDb::Impl::s_isInitalized = false;

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnDbCreated() const
    {
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
// @bsimethod                                Krischan.Eberle                12/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnDbOpening() const
    {
    RegisterBuiltinFunctions();
    return m_idSequenceManager.InitializeSequences();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2017
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnDbAttached(Utf8CP dbFileName, Utf8CP tableSpaceName) const
    {
    DbTableSpace tableSpace(tableSpaceName, dbFileName);
    if (!DbTableSpace::IsAttachedECDbFile(m_ecdb, tableSpaceName))
        return BE_SQLITE_OK; //only need to react to attached ECDb files

    if (SUCCESS != m_schemaManager->GetDispatcher().AddManager(tableSpace))
        return BE_SQLITE_ERROR;
    
    GetChangeManager().OnDbAttached(tableSpace);
    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2017
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
// @bsimethod                                Krischan.Eberle                12/2012
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

    return stat;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2017
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
// @bsimethod                                Krischan.Eberle                10/2017
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
// @bsimethod                                Krischan.Eberle                10/2017
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
// @bsimethod                                Krischan.Eberle                07/2017
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
// @bsimethod                                Krischan.Eberle                12/2014
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::ClearECDbCache() const
    {
    BeMutexHolder lock(m_mutex);
    
    //this event allows consuming code to free anything that relies on the ECDb cache (like ECSchemas, ECSqlStatements etc)
    m_ecdb._OnBeforeClearECDbCache();

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
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("After ECDb::ClearECDbCache");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  03/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Impl::OnAddFunction(DbFunction& function) const
    {
    BeMutexHolder lock(m_mutex);
    DbFunctionKey key(function);
    m_sqlFunctions[key] = &function;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  03/2015
//+---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::OnRemoveFunction(DbFunction& function) const
    {
    BeMutexHolder lock(m_mutex);
    DbFunctionKey key(function);
    m_sqlFunctions.erase(key);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  03/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDb::Impl::TryGetSqlFunction(DbFunction*& function, Utf8CP name, int argCount) const
    {
    auto it = m_sqlFunctions.find(DbFunctionKey(name, argCount));
    if (it == m_sqlFunctions.end())
        {
        function = nullptr;
        return false;
        }

    function = it->second;
    return true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::RegisterBuiltinFunctions() const
    {
    m_changeManager.RegisterSqlFunctions();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::UnregisterBuiltinFunctions() const
    {
    if (!m_ecdb.IsDbOpen())
        return;

    m_changeManager.UnregisterSqlFunction();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2015
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
// @bsimethod                                                    Krischan.Eberle  11/2015
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
            m_issueReporter.ReportV("FileInfo owner ECClass not found for " ECDBSYS_PROP_ECClassId " %s.", ownerClassId.ToString().c_str());
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
// @bsimethod                                                    Krischan.Eberle  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::AddAppData(ECDb::AppData::Key const& key, ECDb::AppData* appData, bool deleteOnClearCache) const
    {
    if (deleteOnClearCache)
        m_appDataToDeleteOnClearCache.insert(&key);

    m_ecdb.AddAppData(key, appData);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Impl::OpenBlobIO(BlobIO& blobIO, Utf8CP tableSpaceName, ECN::ECClassCR ecClass, Utf8CP propertyAccessString, BeInt64Id ecinstanceId, bool writable, ECCrudWriteToken const* writeToken) const
    {
    if (blobIO.IsValid())
        return ERROR;

    Policy policy = PolicyManager::GetPolicy(ECCrudPermissionPolicyAssertion(m_ecdb, writable, writeToken));
    if (!policy.IsSupported())
        {
        m_issueReporter.Report(policy.GetNotSupportedMessage().c_str());
        return ERROR;
        }

    //all this method does is to determine the table and column name for the given class name and property access string.
    //the code is verbose because serious validation is done to ensure that the BlobIO is only used for ECProperties
    //that store BLOB values in a single column.
    ClassMap const* classMap = m_ecdb.Schemas().GetDispatcher().GetClassMap(ecClass, tableSpaceName);
    if (classMap == nullptr || classMap->GetType() == ClassMap::Type::NotMapped)
        {
        m_issueReporter.ReportV("Cannot open BlobIO for ECProperty '%s.%s' (Table space: %s). Cannot find class map for the ECClass.",
                   ecClass.GetFullName(), propertyAccessString, Utf8String::IsNullOrEmpty(tableSpaceName) ? "any" : tableSpaceName);
        return ERROR;
        }

    PropertyMap const* propMap = classMap->GetPropertyMaps().Find(propertyAccessString);
    if (propMap == nullptr)
        {
        m_issueReporter.ReportV("Cannot open BlobIO for ECProperty '%s.%s'. The ECProperty doesn't exist in the ECClass.",
                   ecClass.GetFullName(), propertyAccessString);
        return ERROR;
        }

    if (PropertyMap::Type::Primitive != propMap->GetType())
        {
        m_issueReporter.ReportV("Cannot open BlobIO for ECProperty '%s.%s'. The ECProperty must be primitive and of type Binary.",
                   ecClass.GetFullName(), propertyAccessString);
        return ERROR;
        }

    BeAssert(propMap->GetProperty().GetIsPrimitive());
    const PrimitiveType primType = propMap->GetProperty().GetAsPrimitiveProperty()->GetType();
    if (primType != PRIMITIVETYPE_Binary && primType != PRIMITIVETYPE_IGeometry)
        {
        m_issueReporter.ReportV("Cannot open BlobIO for ECProperty '%s.%s'. It must be either of type Binary or IGeometry.",
                   ecClass.GetFullName(), propertyAccessString);
        return ERROR;
        }

    DbColumn const& col = propMap->GetAs<PrimitivePropertyMap>().GetColumn();

    if (col.GetPersistenceType() == PersistenceType::Virtual)
        {
        m_issueReporter.ReportV("Cannot open BlobIO for ECProperty '%s.%s' as it is not mapped to a column.",
                   ecClass.GetFullName(), propertyAccessString);
        return ERROR;
        }

    return blobIO.Open(m_ecdb, col.GetTable().GetName().c_str(), col.GetName().c_str(), ecinstanceId.GetValue(), writable, tableSpaceName) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDb::Impl::InitializeLib(BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors)
    {
    if (s_isInitalized)
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

    s_isInitalized = true;
    return BE_SQLITE_OK;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
