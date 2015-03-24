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
ECDb::Impl::Impl (ECDbR ecdb)
    : m_ecdb(ecdb), m_schemaManager (nullptr),
    m_ecdbMap (std::unique_ptr<ECDbMap> (new ECDbMap (ecdb))),
    m_ecInstanceIdSequence (ecdb, ECINSTANCEIDSEQUENCE_BELOCALKEY),
    m_ecSchemaIdSequence (ecdb, ECSCHEMAIDSEQUENCE_BELOCALKEY),
    m_ecClassIdSequence (ecdb, ECCLASSIDSEQUENCE_BELOCALKEY),
    m_ecPropertyIdSequence (ecdb, ECPROPERTYIDSEQUENCE_BELOCALKEY),
    m_tableIdSequence (ecdb, TABLEIDSEQUENCE_BELOCALKEY),
    m_columnIdSequence (ecdb, COLUMNIDSEQUENCE_BELOCALKEY),
    m_indexIdSequence (ecdb, INDEXIDSEQUENCE_BELOCALKEY),
    m_constraintIdSequence (ecdb, CONSTRAINTIDSEQUENCE_BELOCALKEY),
    m_classmapIdSequence (ecdb, CLASSMAPIDSEQUENCE_BELOCALKEY),
    m_propertypathIdSequence (ecdb, PROPERTYPATHIDSEQUENCE_BELOCALKEY)
    {
    m_schemaManager = std::unique_ptr<ECDbSchemaManager> (new ECDbSchemaManager (ecdb, *m_ecdbMap));
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
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::Impl::~Impl ()
    {
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
DbResult ECDb::Impl::OnRepositoryIdChanged(BeRepositoryId newRepositoryId)
    {
    if (m_ecdb.IsReadonly ())
        return BE_SQLITE_READONLY;

    const auto stat = ResetSequences (&newRepositoryId);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv ("Changing repository id to %d in file '%s' failed because ECDb's id sequences could not be reset.",
                    newRepositoryId.GetValue (),
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
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2014
//---------------+---------------+---------------+---------------+---------------+------
std::vector<BeRepositoryBasedIdSequence const*> ECDb::Impl::GetSequences () const
    {
    return std::move (std::vector < BeRepositoryBasedIdSequence const* > 
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
ECDbSchemaManagerCR ECDb::Impl::GetSchemaManager () const
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
int ECDb::Impl::AddECSqlCustomFunction(ScalarFunction& ecsqlFunction) const
    {
    const int stat = m_ecdb.AddScalarFunction(ecsqlFunction);
    if (BE_SQLITE_OK == stat)
        m_ecsqlCustomFunctions.insert(&ecsqlFunction);

    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  03/2015
//+---------------+---------------+---------------+---------------+---------------+------
int ECDb::Impl::RemoveECSqlCustomFunction(DbFunction& function) const
    {
    m_ecsqlCustomFunctions.erase(&function);
    return m_ecdb.RemoveFunction(function);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  03/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDb::Impl::IsECSqlFunctionDefined(Utf8CP name, int nArgs) const
    {
    DbFunction key(name, nArgs);
    return m_ecsqlCustomFunctions.find(&key) != m_ecsqlCustomFunctions.end();
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
DbResult ECDb::Impl::ResetSequences (BeRepositoryId* repoId)
    {
    BeRepositoryId actualRepoId = repoId != nullptr ? *repoId : m_ecdb.GetRepositoryId ();
    for (auto sequence : GetSequences ())
        {
        auto stat = sequence->Reset (actualRepoId);
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    return BE_SQLITE_OK;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
