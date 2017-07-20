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
// @bsimethod                                Krischan.Eberle                12/2014
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::ClearECDbCache() const
    {
    BeMutexHolder lock(m_mutex);

    if (m_schemaManager != nullptr)
        m_schemaManager->ClearCache();

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
