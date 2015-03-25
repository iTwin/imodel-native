/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbImpl.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/ECDbSchemaManager.h>
#include "ECDbMap.h"
#include "BeRepositoryBasedIdSequence.h"

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
    struct DbFunctionKey
        {
        Utf8CP m_functionName;
        int m_argCount;

        DbFunctionKey() : m_functionName(nullptr), m_argCount(-1) {}
        DbFunctionKey(Utf8CP functionName, int argCount) : m_functionName(functionName), m_argCount(argCount) {}
        explicit DbFunctionKey(DbFunction& function) : m_functionName(function.GetName()), m_argCount(function.GetNumArgs()) {}

        struct Comparer
            {
            bool operator() (DbFunctionKey const& lhs, DbFunctionKey const& rhs) const
                {
                return BeStringUtilities::Stricmp(lhs.m_functionName, rhs.m_functionName) < 0 || 
                    lhs.m_argCount < rhs.m_argCount;
                }
            };
        };

    static Utf8CP const ECINSTANCEIDSEQUENCE_BELOCALKEY;
    static Utf8CP const ECSCHEMAIDSEQUENCE_BELOCALKEY;
    static Utf8CP const ECCLASSIDSEQUENCE_BELOCALKEY;
    static Utf8CP const ECPROPERTYIDSEQUENCE_BELOCALKEY;

    static Utf8CP const TABLEIDSEQUENCE_BELOCALKEY;
    static Utf8CP const COLUMNIDSEQUENCE_BELOCALKEY;
    static Utf8CP const INDEXIDSEQUENCE_BELOCALKEY;
    static Utf8CP const CONSTRAINTIDSEQUENCE_BELOCALKEY;
    static Utf8CP const CLASSMAPIDSEQUENCE_BELOCALKEY;
    static Utf8CP const PROPERTYPATHIDSEQUENCE_BELOCALKEY;

    ECDbR m_ecdb;
    std::unique_ptr<ECDbSchemaManager> m_schemaManager;
    std::unique_ptr<ECDbMap> m_ecdbMap;

    mutable BeRepositoryBasedIdSequence m_ecInstanceIdSequence;
    BeRepositoryBasedIdSequence m_ecSchemaIdSequence;
    BeRepositoryBasedIdSequence m_ecClassIdSequence;
    BeRepositoryBasedIdSequence m_ecPropertyIdSequence;
    BeRepositoryBasedIdSequence m_tableIdSequence;
    BeRepositoryBasedIdSequence m_columnIdSequence;
    BeRepositoryBasedIdSequence m_indexIdSequence;
    BeRepositoryBasedIdSequence m_constraintIdSequence;
    BeRepositoryBasedIdSequence m_classmapIdSequence;
    BeRepositoryBasedIdSequence m_propertypathIdSequence;

    mutable bmap<DbFunctionKey, ECSqlScalarFunction*, DbFunctionKey::Comparer> m_ecsqlFunctions;

    //Mirrored ECDb methods are only called by ECDb (friend), therefore private
    explicit Impl (ECDbR ecdb);
    static DbResult Initialize (BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors);

    ECDbSchemaManager const& GetSchemaManager () const;
    ECN::IECSchemaLocaterR GetSchemaLocater () const;
    ECN::IECClassLocaterR GetClassLocater () const;

    BentleyStatus OnAddScalarFunction(ScalarFunction&) const;
    void OnRemoveFunction(DbFunction&) const;

    void ClearECDbCache() const;

    DbResult OnDbOpened () const;
    DbResult OnDbCreated () const;
    DbResult OnRepositoryIdChanged (BeRepositoryId newRepositoryId);
    void OnDbChangedByOtherConnection () const;
    DbResult VerifySchemaVersion (Db::OpenParams const& params) const;

    //other private methods
    std::vector<BeRepositoryBasedIdSequence const*> GetSequences () const;

public:
    ~Impl ();

    ECDbMap const& GetECDbMap () const;

    DbResult ResetSequences (BeRepositoryId* repoId = nullptr);
    BeRepositoryBasedIdSequence& GetECInstanceIdSequence () { return m_ecInstanceIdSequence; }
    BeRepositoryBasedIdSequence& GetECSchemaIdSequence () {return m_ecSchemaIdSequence; }
    BeRepositoryBasedIdSequence& GetECClassIdSequence () { return m_ecClassIdSequence; }
    BeRepositoryBasedIdSequence& GetECPropertyIdSequence () { return m_ecPropertyIdSequence; }
    BeRepositoryBasedIdSequence& GetTableIdSequence () { return m_tableIdSequence; }
    BeRepositoryBasedIdSequence& GetColumnIdSequence () { return m_columnIdSequence; }
    BeRepositoryBasedIdSequence& GetIndexIdSequence () { return m_indexIdSequence; }
    BeRepositoryBasedIdSequence& GetConstraintIdSequence () { return m_constraintIdSequence; }
    BeRepositoryBasedIdSequence& GetClassMapIdSequence () { return m_classmapIdSequence; }
    BeRepositoryBasedIdSequence& GetPropertyMapIdSequence () { return m_propertypathIdSequence; }

    bool TryGetECSqlFunction(ECSqlScalarFunction*& ecsqlFunction, Utf8CP name, int argCount) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE