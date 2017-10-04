/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbImpl.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2017
//---------------+---------------+---------------+---------------+---------------+------
//static
//no need to release a static non-POD variable (Bentley C++ coding standards)
bvector<Utf8CP> const* IdSequences::s_sequenceNames = new bvector<Utf8CP> {"ec_instanceidsequence", "ec_schemaidsequence","ec_schemarefidsequence", "ec_classidsequence","ec_classhasbaseclassesidsequence",
"ec_propertyidsequence","ec_propertypathidsequence",
"ec_relconstraintidsequence", "ec_relconstraintclassidsequence",
"ec_customattributeidsequence", "ec_enumidsequence","ec_koqidsequence", "ec_propertycategoryidsequence", "ec_propertymapidsequence",
"ec_tableidsequence","ec_columnidsequence", "ec_indexidsequence", "ec_indexcolumnidsequence"};

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnDbCreated() const
    {
    RegisterBuiltinFunctions();

    DbResult stat = m_idSequences.GetManager().InitializeSequences();
    if (BE_SQLITE_OK != stat)
        return stat;

    //set initial value of sequence to current briefcase id.
    stat = m_idSequences.GetManager().ResetSequences();
    if (BE_SQLITE_OK != stat)
        return stat;

    return ProfileManager::CreateProfile(m_ecdb);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnDbOpening() const
    {
    RegisterBuiltinFunctions();
    return m_idSequences.GetManager().InitializeSequences();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2016
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::OnDbClose() const
    {
    ClearECDbCache();
    m_sqlFunctions.clear();
    UnregisterBuiltinFunctions();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnBriefcaseIdAssigned(BeBriefcaseId newBriefcaseId)
    {
    if (m_ecdb.IsReadonly())
        return BE_SQLITE_READONLY;

    const DbResult stat = m_idSequences.GetManager().ResetSequences(&newBriefcaseId);
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
BentleyStatus ECDb::Impl::ResetIdSequences(BeBriefcaseId briefcaseId, IdSet<ECN::ECClassId> const* ecClassIgnoreList)
    {
    if (!briefcaseId.IsValid() || m_ecdb.IsReadonly())
        return ERROR;

    //ECInstanceId sequence. It has to compute the current max ECInstanceId across all EC data tables
    ECInstanceId maxECInstanceId;
    if (SUCCESS != DetermineMaxECInstanceIdForBriefcase(maxECInstanceId, briefcaseId, ecClassIgnoreList))
        {
        LOG.errorv("Changing BriefcaseId to %" PRIu32 " failed: The maximum for the ECInstanceId sequence for the new BriefcaseId could not be determined.",
                   briefcaseId.GetValue());
        return ERROR;
        }

    if (BE_SQLITE_OK != m_idSequences.GetSequence(IdSequences::Key::InstanceId).Reset(maxECInstanceId.GetValueUnchecked()))
        {
        LOG.errorv("Changing BriefcaseId to %" PRIu32 " failed: The ECInstanceId sequence could not be reset to the new BriefcaseId.",
                   briefcaseId.GetValue());
        return ERROR;
        }

    //Profile table sequences
    const std::vector<std::tuple<IdSequences::Key, Utf8CP, Utf8CP>> profileTableSequences
        {{IdSequences::Key::SchemaId, TABLE_Schema, COL_PROFILETABLE_Id},
        {IdSequences::Key::SchemaReferenceId, TABLE_SchemaReference, COL_PROFILETABLE_Id},
        {IdSequences::Key::ClassId, TABLE_Class, COL_PROFILETABLE_Id},
        {IdSequences::Key::ClassHasBaseClassesId, TABLE_ClassHasBaseClasses, COL_PROFILETABLE_Id},
        {IdSequences::Key::PropertyId, TABLE_Property, COL_PROFILETABLE_Id},
        {IdSequences::Key::PropertyPathId, TABLE_PropertyPath, COL_PROFILETABLE_Id},
        {IdSequences::Key::RelationshipConstraintId, TABLE_RelationshipConstraint, COL_PROFILETABLE_Id},
        {IdSequences::Key::RelationshipConstraintClassId, TABLE_RelationshipConstraintClass, COL_PROFILETABLE_Id},
        {IdSequences::Key::CustomAttributeId, TABLE_CustomAttribute, COL_PROFILETABLE_Id},
        {IdSequences::Key::EnumId, TABLE_Enumeration, COL_PROFILETABLE_Id},
        {IdSequences::Key::KoqId, TABLE_KindOfQuantity, COL_PROFILETABLE_Id},
        {IdSequences::Key::PropertyCategoryId, TABLE_PropertyCategory, COL_PROFILETABLE_Id},
        {IdSequences::Key::PropertyMapId, TABLE_PropertyMap, COL_PROFILETABLE_Id},
        {IdSequences::Key::TableId, TABLE_Table, COL_PROFILETABLE_Id},
        {IdSequences::Key::ColumnId, TABLE_Column, COL_PROFILETABLE_Id},
        {IdSequences::Key::IndexId, TABLE_Index, COL_PROFILETABLE_Id},
        {IdSequences::Key::IndexColumnId, TABLE_IndexColumn, COL_PROFILETABLE_Id}
        };


    for (std::tuple<IdSequences::Key, Utf8CP, Utf8CP> const& profileTableSequence : profileTableSequences)
        {
        IdSequences::Key sequenceKey = std::get<0>(profileTableSequence);
        Utf8CP tableName = std::get<1>(profileTableSequence);
        Utf8CP idColName = std::get<2>(profileTableSequence);

        BeBriefcaseBasedId newSequenceValue;
        if (SUCCESS != DetermineMaxIdForBriefcase(newSequenceValue, briefcaseId, tableName, idColName))
            {
            LOG.errorv("Changing BriefcaseId to %" PRIu32 " failed: The maximum id for sequence '%s' for the new BriefcaseId could not be determined.",
                       briefcaseId.GetValue(), m_idSequences.GetSequence(sequenceKey).GetName());
            return ERROR;
            }

        if (BE_SQLITE_OK != m_idSequences.GetSequence(sequenceKey).Reset(newSequenceValue.GetValueUnchecked()))
            {
            LOG.errorv("Changing BriefcaseId to %" PRIu32 " failed: The sequence '%s' could not be reset to the new BriefcaseId.",
                       briefcaseId.GetValue(), m_idSequences.GetSequence(sequenceKey).GetName());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2017
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Impl::DetermineMaxECInstanceIdForBriefcase(ECInstanceId& maxId, BeBriefcaseId briefcaseId, IdSet<ECN::ECClassId> const* ecClassIgnoreList) const
    {
    if (!briefcaseId.IsValid())
        return ERROR;

    Statement primaryTableStmt;
    if (BE_SQLITE_OK != primaryTableStmt.Prepare(m_ecdb, "SELECT t.Id, t.Name, c.Name FROM " TABLE_Table " t JOIN " TABLE_Column " c ON t.Id=c.TableId WHERE t.Type=" SQLVAL_DbTable_Type_Primary " AND c.ColumnKind=" SQLVAL_DbColumn_Kind_ECInstanceId))
        return ERROR;

    Statement ignoreTableStmt;
    if (ecClassIgnoreList != nullptr)
        {
        if (BE_SQLITE_OK != ignoreTableStmt.Prepare(m_ecdb, "SELECT 1 FROM " TABLE_Class " c JOIN " TABLE_PropertyMap " pm ON c.Id=pm.ClassId "
                                                    "JOIN " TABLE_Column " col ON col.Id=pm.ColumnId "
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
    sql.Sprintf("SELECT max(%s) FROM %s WHERE %s >= ? AND %s < ?", idColName, tableName, idColName, idColName);

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
// @bsimethod                                Krischan.Eberle                12/2016
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::CheckProfileVersion(bool& fileIsAutoUpgradable, bool openModeIsReadonly) const
    {
    if (!m_ecdb.GetDefaultTransaction()->IsActive())
        return BE_SQLITE_ERROR_NoTxnActive;

    ProfileVersion unused(0, 0, 0, 0);
    return ProfileManager::CheckProfileVersion(fileIsAutoUpgradable, unused, m_ecdb, openModeIsReadonly);
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

    if (m_schemaManager != nullptr)
        m_schemaManager->ClearCache();

    const_cast<StatementCache&>(m_sqliteStatementCache).Empty();

    for (AppData::Key const* appDataKey : m_appDataToDeleteOnClearCache)
        {
        m_ecdb.DropAppData(*appDataKey);
        }

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
    m_ecdb.AddFunction(Base64ToBlobSqlFunction::GetSingleton());
    m_ecdb.AddFunction(BlobToBase64SqlFunction::GetSingleton());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::UnregisterBuiltinFunctions() const
    {
    if (!m_ecdb.IsDbOpen())
        return;

    m_ecdb.RemoveFunction(Base64ToBlobSqlFunction::GetSingleton());
    m_ecdb.RemoveFunction(BlobToBase64SqlFunction::GetSingleton());
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
            m_issueReporter.Report("FileInfo owner ECClass not found for " ECDBSYS_PROP_ECClassId " %s.", ownerClassId.ToString().c_str());
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
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, purgeOwnershipByOwnersECSql.c_str(), GetSettings().GetCrudWriteToken()))
        return ERROR;

    if (BE_SQLITE_DONE != stmt.Step())
        return ERROR;

    stmt.Finalize();

    //Step 2: Purge ownership class from records for which file info doesn't exist anymore
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, "DELETE FROM " ECDBF_FILEINFOOWNERSHIP_FULLCLASSNAME " WHERE FileInfoId NOT IN (SELECT " ECDBSYS_PROP_ECInstanceId " FROM ecdbf.FileInfo)", GetSettings().GetCrudWriteToken()))
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
BentleyStatus ECDb::Impl::OpenBlobIO(BlobIO& blobIO, ECN::ECClassCR ecClass, Utf8CP propertyAccessString, BeInt64Id ecinstanceId, bool writable, ECCrudWriteToken const* writeToken) const
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
    ClassMapCP classMap = m_ecdb.Schemas().GetDbMap().GetClassMap(ecClass);
    if (classMap == nullptr || classMap->GetType() == ClassMap::Type::NotMapped)
        {
        LOG.errorv("Cannot open BlobIO for ECProperty '%s.%s'. Its ECClass is not mapped to a table.",
                   ecClass.GetFullName(), propertyAccessString);
        return ERROR;
        }

    PropertyMap const* propMap = classMap->GetPropertyMaps().Find(propertyAccessString);
    if (propMap == nullptr)
        {
        LOG.errorv("Cannot open BlobIO for ECProperty '%s.%s'. The ECProperty doesn't exist in the ECClass.",
                   ecClass.GetFullName(), propertyAccessString);
        return ERROR;
        }

    if (PropertyMap::Type::Primitive != propMap->GetType())
        {
        LOG.errorv("Cannot open BlobIO for ECProperty '%s.%s'. The ECProperty must be primitive and of type Binary.",
                   ecClass.GetFullName(), propertyAccessString);
        return ERROR;
        }

    BeAssert(propMap->GetProperty().GetIsPrimitive());
    const PrimitiveType primType = propMap->GetProperty().GetAsPrimitiveProperty()->GetType();
    if (primType != PRIMITIVETYPE_Binary && primType != PRIMITIVETYPE_IGeometry)
        {
        LOG.errorv("Cannot open BlobIO for ECProperty '%s.%s'. It must be either of type Binary or IGeometry.",
                   ecClass.GetFullName(), propertyAccessString);
        return ERROR;
        }

    DbColumn const& col = propMap->GetAs<PrimitivePropertyMap>().GetColumn();

    if (col.GetPersistenceType() == PersistenceType::Virtual)
        {
        LOG.errorv("Cannot open BlobIO for ECProperty '%s.%s' as it is not mapped to a column.",
                   ecClass.GetFullName(), propertyAccessString);
        return ERROR;
        }

    return blobIO.Open(m_ecdb, col.GetTable().GetName().c_str(), col.GetName().c_str(), ecinstanceId.GetValue(), writable) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDb::Impl::InitializeLib(BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors)
    {
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

    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                02/2017
//---------------+---------------+---------------+---------------+---------------+------
void SettingsHolder::ApplySettings(bool requireECCrudTokenValidation, bool requireECSchemaImportTokenValidation, bool allowChangesetMergingIncompatibleECSchemaImport)
    {
    if (requireECCrudTokenValidation)
        m_eccrudWriteToken = std::unique_ptr<ECCrudWriteToken>(new ECCrudWriteToken());

    if (requireECSchemaImportTokenValidation)
        m_ecSchemaImportToken = std::unique_ptr<SchemaImportToken>(new SchemaImportToken());

    m_settings = ECDb::Settings(m_eccrudWriteToken.get(), m_ecSchemaImportToken.get(), allowChangesetMergingIncompatibleECSchemaImport);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
