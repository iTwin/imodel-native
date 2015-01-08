/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbStore.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static
Utf8CP const ECDbStore::ECINSTANCEIDSEQUENCE_BELOCALKEY = "ec_ecidsequence";
//static
Utf8CP const ECDbStore::ECSCHEMAIDSEQUENCE_BELOCALKEY = "ec_ecschemaidsequence";
//static
Utf8CP const ECDbStore::ECCLASSIDSEQUENCE_BELOCALKEY = "ec_ecclassidsequence";
//static
Utf8CP const ECDbStore::ECPROPERTYIDSEQUENCE_BELOCALKEY = "ec_ecpropertyidsequence";

//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                03/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECDbStore::ECDbStore (ECDbR ecdb) 
: m_ecDb (ecdb), m_ecDbMap (nullptr), m_ecDbSchemaManager (nullptr),
      m_ecInstanceIdSequence (ecdb, ECINSTANCEIDSEQUENCE_BELOCALKEY), 
      m_ecSchemaIdSequence (ecdb, ECSCHEMAIDSEQUENCE_BELOCALKEY), 
      m_ecClassIdSequence (ecdb, ECCLASSIDSEQUENCE_BELOCALKEY), 
      m_ecPropertyIdSequence (ecdb, ECPROPERTYIDSEQUENCE_BELOCALKEY)
    {
    m_ecDbMap = std::unique_ptr<ECDbMap> (new ECDbMap (ecdb));
    m_ecDbSchemaManager = std::unique_ptr<ECDbSchemaManager> (new ECDbSchemaManager (ecdb, *m_ecDbMap));
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                03/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECDbStore::~ECDbStore ()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                11/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECDbStorePtr ECDbStore::Create (ECDbR ecDb)
    {
    return new ECDbStore (ecDb);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Krischan.Eberle                07/2014
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbStore::OnDbOpened ()
    {
    bvector<BeRepositoryBasedIdSequenceCP> sequences;
    GetIdSequences (sequences);
    for (auto sequence : sequences)
        {
        auto stat = sequence->Initialize ();
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Krischan.Eberle                07/2014
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbStore::OnDbCreated ()
    {
    auto stat = OnDbOpened ();
    if (stat != BE_SQLITE_OK)
        return stat;

    return ECDbProfileManager::CreateECProfile (m_ecDb);
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                 Krischan.Eberle                11/2012
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbStore::OnRepositoryIdChanged (BeRepositoryId newRepositoryId)
    {
    PRECONDITION (!m_ecDb.IsReadonly (), BE_SQLITE_READONLY);

    bvector<BeRepositoryBasedIdSequenceCP> idSequences;
    GetIdSequences (idSequences);
    const auto stat = ECDbProfileManager::ResetIdSequences (GetECDbR (), idSequences, newRepositoryId);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv ("Changing repository id to %d in file '%s' failed because ECDb's id sequences could not be reset.",
            newRepositoryId.GetValue (),
            m_ecDb.GetDbFileName ());
        }

    return stat;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Krischan.Eberle                01/2015
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbStore::OnDbChangedByOtherConnection () const
    {
    ClearCache ();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Krischan.Eberle                11/2012
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbStore::Close ()
    {
    ClearCache ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                  06/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbStore::ClearCache () const
    {
    if (m_ecDbSchemaManager != nullptr)
        m_ecDbSchemaManager->ClearCache ();

    if (m_ecDbMap != nullptr)
        m_ecDbMap->ClearCache ();

    m_ecPersistenceCache.clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                06/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECPersistencePtr ECDbStore::GetECPersistence (PersistenceStatus* status, IClassMap const& classMap) const
    {
    PersistenceStatus ALLOW_NULL_OUTPUT (locStatus, status);

    // Error checks
    if (classMap.IsUnmapped () || classMap.GetTable ().IsVirtual ())
        {
        locStatus = PERSISTENCE_ClassIsNotMapped;
        return nullptr;
        }

    auto& ecClass = classMap.GetClass ();
    if (ecClass.GetIsStruct () && !ecClass.GetIsDomainClass ())
        {
        locStatus = PERSISTENCE_StructIsNotDomainClass;
        return nullptr;
        }

    locStatus = PERSISTENCE_Success;

    // Look up cache
    auto it = m_ecPersistenceCache.find (&classMap);
    if (it != m_ecPersistenceCache.end())
        return it->second;

    // Create a new persistence
    ECPersistencePtr ecPersistence = new ECPersistence (classMap, m_ecInstanceIdSequence);
    m_ecPersistenceCache[&classMap] = ecPersistence;
    return ecPersistence;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                06/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECPersistencePtr ECDbStore::GetECPersistence (PersistenceStatus* status, ECN::ECClassCR ecClass) const
    {
   auto classMap = GetECDbMap().GetClassMap (ecClass, true);
   if (classMap == nullptr)
        {
        if (status != nullptr)
            *status = PersistenceStatus::PERSISTENCE_ClassIsNotMapped;

        LOG.errorv (L"Failed to get ECPersistence for ECClass %ls. ECClass' class map could not be found.", ecClass.GetName ().c_str ());
        BeAssert (false && "GetECPersistence> Failed to get class map for ECClass");
        return nullptr;
        }

    return GetECPersistence (status, *classMap);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                     12/2012
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbStore::ReplaceEmptyTablesWithEmptyViews() const
    {
    return GetECDbMap().ReplaceEmptyTablesWithEmptyViews();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                     09/2014
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbStore::ReplaceViewsWithEmptyTables () const
    {
    return GetECDbMap ().ReplaceViewsWithEmptyTables ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    12/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECDbR ECDbStore::GetECDbR () const
    {
    return m_ecDb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen     02/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECDbMapCR ECDbStore::GetECDbMap() const
    {
    return *m_ecDbMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan       06/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECDbSchemaManagerCR ECDbStore::GetSchemaManager() const
    {
    return *m_ecDbSchemaManager;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//+---------------+---------------+---------------+---------------+---------------+------
IECSchemaLocaterR ECDbStore::GetSchemaLocater () const
    {
    return *m_ecDbSchemaManager;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//+---------------+---------------+---------------+---------------+---------------+------
IECClassLocaterR ECDbStore::GetClassLocater () const
    {
    return *m_ecDbSchemaManager;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       12/2012
//+---------------+---------------+---------------+---------------+---------------+------
BeRepositoryBasedIdSequenceR ECDbStore::GetECInstanceIdSequence ()
    {
    return m_ecInstanceIdSequence;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       02/2013
//+---------------+---------------+---------------+---------------+---------------+------
BeRepositoryBasedIdSequenceR ECDbStore::GetECSchemaIdSequence ()
    {
    return m_ecSchemaIdSequence;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       02/2013
//+---------------+---------------+---------------+---------------+---------------+------
BeRepositoryBasedIdSequenceR ECDbStore::GetECClassIdSequence ()
    {
    return m_ecClassIdSequence;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       02/2013
//+---------------+---------------+---------------+---------------+---------------+------
BeRepositoryBasedIdSequenceR ECDbStore::GetECPropertyIdSequence ()
    {
    return m_ecPropertyIdSequence;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       02/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbStore::GetIdSequences (bvector<BeRepositoryBasedIdSequenceCP>& idSequences) const
    {
    idSequences.push_back (&m_ecInstanceIdSequence);
    idSequences.push_back (&m_ecSchemaIdSequence);
    idSequences.push_back (&m_ecClassIdSequence);
    idSequences.push_back (&m_ecPropertyIdSequence);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
