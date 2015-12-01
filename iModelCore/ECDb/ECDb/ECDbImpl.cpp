/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbImpl.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbImpl.h"
#include "ECDbProfileManager.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static
Utf8CP const ECDb::Impl::ECINSTANCEIDSEQUENCE_BELOCALKEY = "ec_ecinstanceidsequence";
//static
Utf8CP const ECDb::Impl::ECSCHEMAIDSEQUENCE_BELOCALKEY = "ec_ecschemaidsequence";
//static
Utf8CP const ECDb::Impl::ECCLASSIDSEQUENCE_BELOCALKEY = "ec_ecclassidsequence";
//static
Utf8CP const ECDb::Impl::ECPROPERTYIDSEQUENCE_BELOCALKEY = "ec_ecpropertyidsequence";
//static
Utf8CP const ECDb::Impl::TABLEIDSEQUENCE_BELOCALKEY = "ec_tableidsequence";
//static
Utf8CP const ECDb::Impl::COLUMNIDSEQUENCE_BELOCALKEY = "ec_columnidsequence";
//static
Utf8CP const ECDb::Impl::INDEXIDSEQUENCE_BELOCALKEY = "ec_indexidsequence";
//static
Utf8CP const ECDb::Impl::CONSTRAINTIDSEQUENCE_BELOCALKEY = "ec_constraintidsequence";
//static
Utf8CP const ECDb::Impl::CLASSMAPIDSEQUENCE_BELOCALKEY = "ec_classmapidsequence";
//static
Utf8CP const ECDb::Impl::PROPERTYPATHIDSEQUENCE_BELOCALKEY = "ec_propertypathidsequence";

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::Impl::Impl(ECDbR ecdb)
    : m_ecdb(ecdb), m_schemaManager(nullptr),
    m_ecdbMap(std::unique_ptr<ECDbMap>(new ECDbMap(ecdb))),
    m_ecInstanceIdSequence(ecdb, ECINSTANCEIDSEQUENCE_BELOCALKEY),
    m_ecSchemaIdSequence(ecdb, ECSCHEMAIDSEQUENCE_BELOCALKEY),
    m_ecClassIdSequence(ecdb, ECCLASSIDSEQUENCE_BELOCALKEY),
    m_ecPropertyIdSequence(ecdb, ECPROPERTYIDSEQUENCE_BELOCALKEY),
    m_tableIdSequence(ecdb, TABLEIDSEQUENCE_BELOCALKEY),
    m_columnIdSequence(ecdb, COLUMNIDSEQUENCE_BELOCALKEY),
    m_indexIdSequence(ecdb, INDEXIDSEQUENCE_BELOCALKEY),
    m_constraintIdSequence(ecdb, CONSTRAINTIDSEQUENCE_BELOCALKEY),
    m_classmapIdSequence(ecdb, CLASSMAPIDSEQUENCE_BELOCALKEY),
    m_propertypathIdSequence(ecdb, PROPERTYPATHIDSEQUENCE_BELOCALKEY),
    m_issueReporter(ecdb)
    {
    m_schemaManager = std::unique_ptr<ECDbSchemaManager>(new ECDbSchemaManager(ecdb, *m_ecdbMap));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDb::Impl::Initialize (BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors)
    {
    const auto stat = BeSQLiteLib::Initialize (ecdbTempDir, logSqliteErrors);
    if (stat != BE_SQLITE_OK)
        return stat;

    if (hostAssetsDir != nullptr)
        {
        if (!hostAssetsDir->DoesPathExist ())
            {
            LOG.warningv ("ECDb::Initialize: host assets dir '%s' does not exist.", hostAssetsDir->GetNameUtf8 ().c_str ());
            BeAssert (false && "ECDb::Initialize: host assets dir does not exist!");
            }

        ECN::ECSchemaReadContext::Initialize (*hostAssetsDir);
        }

    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2014
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnDbOpened () const
    {
    for (auto sequence : GetSequences ())
        {
        auto stat = sequence->Initialize ();
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnDbCreated () const
    {
    auto stat = OnDbOpened ();
    if (stat != BE_SQLITE_OK)
        return stat;

    return ECDbProfileManager::CreateECProfile (m_ecdb);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::OnBriefcaseIdChanged(BeBriefcaseId newBriefcaseId)
    {
    if (m_ecdb.IsReadonly ())
        return BE_SQLITE_READONLY;

    const auto stat = ResetSequences (&newBriefcaseId);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv ("Changing briefcase id to %d in file '%s' failed because ECDb's id sequences could not be reset.",
                    newBriefcaseId.GetValue (),
                    m_ecdb.GetDbFileName());
        }

    return stat;
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                01/2015
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::OnDbChangedByOtherConnection () const
    {
    ClearECDbCache ();
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::VerifySchemaVersion (Db::OpenParams const& params) const
    {
    return ECDbProfileManager::UpgradeECProfile (m_ecdb, params);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2014
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::Impl::ClearECDbCache () const
    {
    if (m_ecdbMap != nullptr)
        m_ecdbMap->ClearCache ();

    if (m_schemaManager != nullptr)
        m_schemaManager->ClearCache ();

    LOG.debug("Cleared ECDb cache.");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2014
//---------------+---------------+---------------+---------------+---------------+------
std::vector<BeBriefcaseBasedIdSequence const*> ECDb::Impl::GetSequences () const
    {
    return std::move (std::vector < BeBriefcaseBasedIdSequence const* > 
        {
        &m_ecInstanceIdSequence, 
        &m_ecSchemaIdSequence, 
        &m_ecClassIdSequence, 
        &m_ecPropertyIdSequence, 
        &m_classmapIdSequence, 
        &m_columnIdSequence, 
        &m_constraintIdSequence, 
        &m_tableIdSequence,
        &m_propertypathIdSequence,
        &m_indexIdSequence
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan       06/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECDbSchemaManagerCR ECDb::Impl::Schemas () const
    {
    return *m_schemaManager;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::IECSchemaLocaterR ECDb::Impl::GetSchemaLocater () const
    {
    return *m_schemaManager;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::IECClassLocaterR ECDb::Impl::GetClassLocater () const
    {
    return *m_schemaManager;
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
// @bsimethod                                                    Affan.Khan       06/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECDbMap const& ECDb::Impl::GetECDbMap () const
    {
    return *m_ecdbMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2014
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::Impl::ResetSequences (BeBriefcaseId* repoId)
    {
    BeBriefcaseId actualRepoId = repoId != nullptr ? *repoId : m_ecdb.GetBriefcaseId ();
    for (auto sequence : GetSequences ())
        {
        auto stat = sequence->Reset (actualRepoId);
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Impl::Purge(ECDb::PurgeMode mode) const
    {
    //All purge modes will be tried even if one fails. If one fails, the method returns ERROR
    BentleyStatus stat = SUCCESS;
    if (Enum::Contains(mode, ECDb::PurgeMode::OrphanedFileInfos))
        stat = PurgeFileInfos();

    return stat;
    }

#define ECDBF_FILEINFOOWNERSHIP_FULLCLASSNAME "ecdbf.FileInfoOwnership"
#define ECDBF_FILEINFO_FULLCLASSNAME "ecdbf.FileInfo"

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
        ownerClassIds.push_back(stmt.GetValueInt64(0));
        }
    }

    //Step 1: Purge ownership class from records for which owner doesn't exist anymore
    {
    Utf8String purgeOwnershipByOwnersECSql("DELETE FROM ONLY " ECDBF_FILEINFOOWNERSHIP_FULLCLASSNAME " WHERE ");
    bool isFirstOwnerClassId = true;
    for (ECClassId ownerClassId : ownerClassIds)
        {
        ECClassCP ownerClass = Schemas().GetECClass(ownerClassId);
        if (ownerClass == nullptr)
            {
            GetIssueReporter().Report(ECDbIssueSeverity::Error, "FileInfo owner ECClass not found for ECClassId %lld.", ownerClassId);
            return ERROR;
            }

        if (!isFirstOwnerClassId)
            purgeOwnershipByOwnersECSql.append(" OR ");

        Utf8String whereSnippet;
        whereSnippet.Sprintf("(OwnerECClassId=%lld AND OwnerId NOT IN (SELECT ECInstanceId FROM ONLY %s))", ownerClassId, ECSqlBuilder::ToECSqlSnippet(*ownerClass).c_str());
        purgeOwnershipByOwnersECSql.append(whereSnippet);

        isFirstOwnerClassId = false;
        }

    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, purgeOwnershipByOwnersECSql.c_str()))
        return ERROR;

    if (BE_SQLITE_DONE != stmt.Step())
        return ERROR;
    }

    //Step 2: Purge ownership class from records for which file info doesn't exist anymore
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, "DELETE FROM ONLY " ECDBF_FILEINFOOWNERSHIP_FULLCLASSNAME " WHERE FileInfoId NOT IN (SELECT ECInstanceId FROM " ECDBF_FILEINFO_FULLCLASSNAME ")"))
        return ERROR;

    if (BE_SQLITE_DONE != stmt.Step())
        return ERROR;
    }

    //Step 3: Purge file info class from records for which no ownership exists anymore
    {
    //TODO: Once polymorphic DELETE works again, we only need a single DELETE. Uncomment the below, and remove
    //the two statements below
/*    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, "DELETE FROM " ECDBF_FILEINFO_FULLCLASSNAME " WHERE ECInstanceId NOT IN (SELECT FileInfoId FROM ONLY " ECDBF_FILEINFOOWNERSHIP_CLASSNAME ")"))
        return ERROR;

    if (BE_SQLITE_DONE != stmt.Step())
        return ERROR;
        */

    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, "DELETE FROM ONLY ecdbf.EmbeddedFileInfo WHERE ECInstanceId NOT IN (SELECT FileInfoId FROM ONLY " ECDBF_FILEINFOOWNERSHIP_FULLCLASSNAME ")"))
        return ERROR;

    if (BE_SQLITE_DONE != stmt.Step())
        return ERROR;

    stmt.Finalize();

    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, "DELETE FROM ecdbf.ExternalFileInfo WHERE ECInstanceId NOT IN (SELECT FileInfoId FROM ONLY " ECDBF_FILEINFOOWNERSHIP_FULLCLASSNAME ")"))
        return ERROR;

    if (BE_SQLITE_DONE != stmt.Step())
        return ERROR;
    }

    return SUCCESS;
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
