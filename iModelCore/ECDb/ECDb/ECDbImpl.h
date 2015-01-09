/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbImpl.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <BeSQLite/ECDb/ECDb.h>
#include <BeSQLite/ECDb/ECDbSchemaManager.h>
#include "ECDbMap.h"
#include "BeRepositoryBasedIdSequence.h"
#include "ECPersistence.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! ECDb::Impl is the private implementation of ECDb hidden from the public headers
//! (PIMPL idiom)
// @bsiclass                                                Krischan.Eberle      12/2014
//+===============+===============+===============+===============+===============+======
struct ECDb::Impl : NonCopyableClass
    {
    friend struct ECDb;
        
private:
    static Utf8CP const ECINSTANCEIDSEQUENCE_BELOCALKEY;
    static Utf8CP const ECSCHEMAIDSEQUENCE_BELOCALKEY;
    static Utf8CP const ECCLASSIDSEQUENCE_BELOCALKEY;
    static Utf8CP const ECPROPERTYIDSEQUENCE_BELOCALKEY;

    std::unique_ptr<ECDbSchemaManager> m_schemaManager;
    std::unique_ptr<ECDbMap> m_ecdbMap;

    mutable BeRepositoryBasedIdSequence m_ecInstanceIdSequence;
    BeRepositoryBasedIdSequence m_ecSchemaIdSequence;
    BeRepositoryBasedIdSequence m_ecClassIdSequence;
    BeRepositoryBasedIdSequence m_ecPropertyIdSequence;

    mutable bmap<IClassMap const*, ECPersistencePtr> m_ecPersistenceCache;


    //Mirrored ECDb methods are only called by ECDb (friend), therefore private
    explicit Impl (ECDbR ecdb);
    static DbResult Initialize (BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors);

    void Close ();
    void ClearCache () const;

    ECDbSchemaManager const& GetSchemaManager () const;
    ECN::IECSchemaLocaterR GetSchemaLocater () const;
    ECN::IECClassLocaterR GetClassLocater () const;


    DbResult OnDbOpened () const;
    DbResult OnDbCreated (ECDbR ecdb) const;
    DbResult OnRepositoryIdChanged (ECDbR ecdb, BeRepositoryId newRepositoryId);
    void OnDbChangedByOtherConnection () const;
    DbResult VerifySchemaVersion (ECDbR ecdb, Db::OpenParams const& params, Savepoint& defaultTransaction) const;

    //other private methods
    std::vector<BeRepositoryBasedIdSequence const*> GetSequences () const;

public:
    ~Impl ();

    ECDbMap const& GetECDbMap () const;

    DbResult ResetSequences (ECDbR ecdb, BeRepositoryId* repoId = nullptr);
    BeRepositoryBasedIdSequence& GetECInstanceIdSequence () { return m_ecInstanceIdSequence; }
    BeRepositoryBasedIdSequence& GetECSchemaIdSequence () {return m_ecSchemaIdSequence; }
    BeRepositoryBasedIdSequence& GetECClassIdSequence () { return m_ecClassIdSequence; }
    BeRepositoryBasedIdSequence& GetECPropertyIdSequence () { return m_ecPropertyIdSequence; }

    ECPersistencePtr GetECPersistence (ECN::ECClassCR ecClass) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE