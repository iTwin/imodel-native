/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbImpl.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::Impl::Impl(ECDbR ecdb) : m_ecdb(ecdb), m_expectedECSqlWriteToken(nullptr),
                            m_ecInstanceIdSequence(ecdb, "ec_ecinstanceidsequence"),
                            m_ecSchemaIdSequence(ecdb, "ec_ecschemaidsequence"),
                            m_ecClassIdSequence(ecdb, "ec_ecclassidsequence"),
                            m_ecPropertyIdSequence(ecdb, "ec_ecpropertyidsequence"),
                            m_ecEnumIdSequence(ecdb, "ec_ecenumidsequence"),
                            m_koqIdSequence(ecdb, "ec_kindofquantityidsequence"),
                            m_tableIdSequence(ecdb, "ec_tableidsequence"),
                            m_columnIdSequence(ecdb, "ec_columnidsequence"),
                            m_indexIdSequence(ecdb, "ec_indexidsequence"),
                            m_propertypathIdSequence(ecdb, "ec_propertypathidsequence"),
                            m_issueReporter(ecdb)
    {
    m_schemaManager = std::unique_ptr<ECDbSchemaManager>(new ECDbSchemaManager(ecdb, m_mutex));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDb::Impl::Initialize(BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors)
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
// @bsimethod                                Krischan.Eberle                12/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnDbCreated() const
    {
    DbResult stat = InitializeSequences();
    if (BE_SQLITE_OK != stat)
        return stat;

    //set initial value of sequence to current briefcase id.
    stat = ResetSequences();
    if (BE_SQLITE_OK != stat)
        return stat;

    return ECDbProfileManager::CreateECProfile(m_ecdb);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2016
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::OnDbClose() const
    {
    ClearECDbCache();
    m_sqlFunctions.clear();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnBriefcaseIdChanged(BeBriefcaseId newBriefcaseId)
    {
    if (m_ecdb.IsReadonly())
        return BE_SQLITE_READONLY;

    const DbResult stat = ResetSequences(&newBriefcaseId);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("Changing briefcase id to %" PRIu32 " in file '%s' failed because ECDb's id sequences could not be reset.",
                   newBriefcaseId.GetValue(),
                   m_ecdb.GetDbFileName());
        }

    return stat;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2014
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::ClearECDbCache() const
    {
    //Note: no mutex lock required as long as this method is not exported.
    //BeMutexHolder lock(m_mutex);

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
// @bsimethod                                                    Krischan.Eberle  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::InitializeSequences() const
    {
    for (BeBriefcaseBasedIdSequence const* sequence : GetSequences())
        {
        const DbResult stat = sequence->Initialize();
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2014
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::ResetSequences(BeBriefcaseId* repoId) const
    {
    BeBriefcaseId actualRepoId = repoId != nullptr ? *repoId : m_ecdb.GetBriefcaseId();
    for (BeBriefcaseBasedIdSequence const* sequence : GetSequences())
        {
        const DbResult stat = sequence->Reset(actualRepoId);
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2014
//---------------+---------------+---------------+---------------+---------------+------
std::vector<BeBriefcaseBasedIdSequence const*> ECDb::Impl::GetSequences() const
    {
    return std::vector<BeBriefcaseBasedIdSequence const*>  {
        &m_ecInstanceIdSequence,
            &m_ecSchemaIdSequence,
            &m_ecClassIdSequence,
            &m_ecPropertyIdSequence,
            &m_ecEnumIdSequence,
            &m_koqIdSequence,
            &m_columnIdSequence,
            &m_tableIdSequence,
            &m_propertypathIdSequence,
            &m_indexIdSequence};
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
        ownerClassIds.push_back(stmt.GetValueId<ECClassId>(0));
        }
    }

    //Step 1: Purge ownership class from records for which owner doesn't exist anymore
    Utf8String purgeOwnershipByOwnersECSql("DELETE FROM " ECDBF_FILEINFOOWNERSHIP_FULLCLASSNAME " WHERE ");
    bool isFirstOwnerClassId = true;
    for (ECClassId ownerClassId : ownerClassIds)
        {
        ECClassCP ownerClass = Schemas().GetECClass(ownerClassId);
        if (ownerClass == nullptr)
            {
            GetIssueReporter().Report(ECDbIssueSeverity::Error, "FileInfo owner ECClass not found for ECClassId %s.", ownerClassId.ToString().c_str());
            return ERROR;
            }

        if (!isFirstOwnerClassId)
            purgeOwnershipByOwnersECSql.append(" OR ");

        Utf8String whereSnippet;
        whereSnippet.Sprintf("(OwnerECClassId=%s AND OwnerId NOT IN (SELECT ECInstanceId FROM ONLY %s))", ownerClassId.ToString().c_str(), ownerClass->GetECSqlName().c_str());
        purgeOwnershipByOwnersECSql.append(whereSnippet);

        isFirstOwnerClassId = false;
        }

    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, purgeOwnershipByOwnersECSql.c_str()))
        return ERROR;

    if (BE_SQLITE_DONE != stmt.Step())
        return ERROR;
    
    stmt.Finalize();

    //Step 2: Purge ownership class from records for which file info doesn't exist anymore
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, "DELETE FROM " ECDBF_FILEINFOOWNERSHIP_FULLCLASSNAME " WHERE FileInfoId NOT IN (SELECT ECInstanceId FROM ecdbf.FileInfo)"))
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


END_BENTLEY_SQLITE_EC_NAMESPACE
