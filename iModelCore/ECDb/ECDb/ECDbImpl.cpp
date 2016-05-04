/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbImpl.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::Impl::Impl(ECDbR ecdb) : m_ecdb(ecdb), m_schemaManager(nullptr),
                            m_ecdbMap(std::unique_ptr<ECDbMap>(new ECDbMap(ecdb))),
                            m_ecInstanceIdSequence(ecdb, "ec_ecinstanceidsequence"),
                            m_ecSchemaIdSequence(ecdb, "ec_ecschemaidsequence"),
                            m_ecClassIdSequence(ecdb, "ec_ecclassidsequence"),
                            m_ecPropertyIdSequence(ecdb, "ec_ecpropertyidsequence"),
                            m_ecEnumIdSequence(ecdb, "ec_ecenumidsequence"),
                            m_tableIdSequence(ecdb, "ec_tableidsequence"),
                            m_columnIdSequence(ecdb, "ec_columnidsequence"),
                            m_indexIdSequence(ecdb, "ec_indexidsequence"),
                            m_classmapIdSequence(ecdb, "ec_classmapidsequence"),
                            m_propertypathIdSequence(ecdb, "ec_propertypathidsequence"),
                            m_issueReporter(ecdb)
    {
    m_schemaManager = std::unique_ptr<ECDbSchemaManager>(new ECDbSchemaManager(ecdb, *m_ecdbMap));
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
    if (m_ecdbMap != nullptr)
        m_ecdbMap->ClearCache();

    if (m_schemaManager != nullptr)
        m_schemaManager->ClearCache();

    m_statementRegistry.ReprepareStatements();

    for (AppData::Key const* appDataKey : m_appDataToDeleteOnClearCache)
        {
        m_ecdb.DropAppData(*appDataKey);
        }

    STATEMENT_DIAGNOSTICS_LOGCOMMENT("After ECDb::ClearECDbCache");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  03/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Impl::OnAddFunction(DbFunction& function) const
    {
    DbFunctionKey key(function);
    m_sqlFunctions[key] = &function;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  03/2015
//+---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::OnRemoveFunction(DbFunction& function) const
    {
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
            &m_classmapIdSequence,
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
    if (Enum::Contains(mode, ECDb::PurgeMode::FileInfoOwnerships))
        if (SUCCESS != PurgeFileInfos())
            return ERROR;

    if (Enum::Contains(mode, ECDb::PurgeMode::HoldingRelationships))
        {
        RelationshipPurger purger;
        if (SUCCESS != purger.Purge(m_ecdb))
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

//******************************************
// ECDb::Impl::ECSqlStatementRegistry
//******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::ECSqlStatementRegistry::Add(ECSqlStatement::Impl& stmt) const
    {
    BeMutexHolder lock(m_mutex);
    m_statements.insert(&stmt);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::ECSqlStatementRegistry::Remove(ECSqlStatement::Impl& stmt) const
    {
    BeMutexHolder lock(m_mutex);
    auto it = m_statements.find(&stmt);
    if (it != m_statements.end())
        m_statements.erase(it);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                01/2016
//---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECDb::Impl::ECSqlStatementRegistry::ReprepareStatements() const
    {
    BeMutexHolder lock(m_mutex);
    for (ECSqlStatement::Impl* stmt : m_statements)
        {
        if (stmt != nullptr && stmt->IsPrepared())
            stmt->Reprepare();
        }

    return ECSqlStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                01/2016
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::ECSqlStatementRegistry::FinalizeStatements() const
    {
    BeMutexHolder lock(m_mutex);
    for (ECSqlStatement::Impl* stmt : m_statements)
        {
        if (stmt != nullptr)
            stmt->DoFinalize(false); //don't unregister, as we will simply clear the registry after all statements have been finalized
        }

    m_statements.clear();
    }

//******************************************
// IssueReporter
//******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  09/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus IssueReporter::AddListener(ECDb::IIssueListener const& issueListener)
    {
    BeMutexHolder lock(m_mutex);

    if (m_issueListener != nullptr)
        return ERROR;

    m_issueListener = &issueListener;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  09/2015
//+---------------+---------------+---------------+---------------+---------------+------
void IssueReporter::RemoveListener()
    {
    BeMutexHolder lock(m_mutex);
    m_issueListener = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  09/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool IssueReporter::IsSeverityEnabled(ECDbIssueSeverity severity) const
    {
    return m_issueListener != nullptr || LOG.isSeverityEnabled(ToLogSeverity(severity));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  09/2015
//+---------------+---------------+---------------+---------------+---------------+------
void IssueReporter::Report(ECDbIssueSeverity severity, Utf8CP message, ...) const
    {
    if (Utf8String::IsNullOrEmpty(message))
        return;

    const NativeLogging::SEVERITY logSeverity = ToLogSeverity(severity);
    const bool isLogSeverityEnabled = LOG.isSeverityEnabled(logSeverity);

    BeMutexHolder lock(m_mutex);

    if (m_issueListener != nullptr || isLogSeverityEnabled)
        {
        va_list args;
        va_start(args, message);

        Utf8String formattedMessage;
        formattedMessage.VSprintf(message, args);

        if (m_issueListener != nullptr)
            m_issueListener->ReportIssue(severity, formattedMessage.c_str());

        if (isLogSeverityEnabled)
            LOG.message(logSeverity, formattedMessage.c_str());

        va_end(args);
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2014
//+---------------+---------------+---------------+---------------+---------------+------
void IssueReporter::ReportSqliteIssue(ECDbIssueSeverity sev, DbResult sqliteStat, Utf8CP messageHeader) const
    {
    if (BE_SQLITE_OK != sqliteStat && IsSeverityEnabled(sev))
        {
        if (messageHeader == nullptr)
            messageHeader = "SQLite error:";

        Utf8CP dbResultStr = ECDb::InterpretDbResult(sqliteStat);

        Utf8String lastSqliteErrorMsg = m_ecdb.GetLastError();
        //ECDb sometimes returns DbResult errors on its own. In that case there is no SQLite error to output
        if (lastSqliteErrorMsg.empty())
            Report(sev, "%s %s", messageHeader, dbResultStr);
        else
            Report(sev, "%s %s: %s", messageHeader, dbResultStr, lastSqliteErrorMsg.c_str());
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
