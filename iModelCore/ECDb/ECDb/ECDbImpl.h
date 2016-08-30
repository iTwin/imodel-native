/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbImpl.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/ECDbSchemaManager.h>
#include "ECDbMap.h"
#include "BeBriefcaseBasedIdSequence.h"
#include "ECDbProfileManager.h"
#include "IssueReporter.h"

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
                int nameCompareResult = BeStringUtilities::StricmpAscii(lhs.m_functionName, rhs.m_functionName);
                return nameCompareResult < 0 || nameCompareResult == 0 && lhs.m_argCount < rhs.m_argCount;
                }
            };
        };

    struct ECSqlStatementRegistry : NonCopyableClass
        {
        private:
            mutable bset<ECSqlStatement::Impl*> m_statements;
            mutable BeMutex m_mutex;

        public:
            ECSqlStatementRegistry() {}

            void Add(ECSqlStatement::Impl&) const;
            void Remove(ECSqlStatement::Impl&) const;

            ECSqlStatus ReprepareStatements() const;
            void FinalizeStatements() const;
        };

    ECDbR m_ecdb;
    std::unique_ptr<ECDbSchemaManager> m_schemaManager;
    std::unique_ptr<ECDbMap> m_ecdbMap;

    mutable BeBriefcaseBasedIdSequence m_ecInstanceIdSequence;
    BeBriefcaseBasedIdSequence m_ecSchemaIdSequence;
    BeBriefcaseBasedIdSequence m_ecClassIdSequence;
    BeBriefcaseBasedIdSequence m_ecPropertyIdSequence;
    BeBriefcaseBasedIdSequence m_ecEnumIdSequence;
    BeBriefcaseBasedIdSequence m_koqIdSequence;
    BeBriefcaseBasedIdSequence m_tableIdSequence;
    BeBriefcaseBasedIdSequence m_columnIdSequence;
    BeBriefcaseBasedIdSequence m_indexIdSequence;
    BeBriefcaseBasedIdSequence m_classmapIdSequence;
    BeBriefcaseBasedIdSequence m_propertypathIdSequence;
    mutable bmap<DbFunctionKey, DbFunction*, DbFunctionKey::Comparer> m_sqlFunctions;
    ECSqlStatementRegistry m_statementRegistry;
    mutable bset<AppData::Key const*, std::less<AppData::Key const*>> m_appDataToDeleteOnClearCache;

    IssueReporter m_issueReporter;

    //Mirrored ECDb methods are only called by ECDb (friend), therefore private
    explicit Impl(ECDbR ecdb);
    static DbResult Initialize(BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors);

    ECDbSchemaManager const& Schemas() const { return *m_schemaManager; }
    ECN::IECSchemaLocaterR GetSchemaLocater() const { return *m_schemaManager; }
    ECN::IECClassLocaterR GetClassLocater() const { return *m_schemaManager; }

    BentleyStatus OnAddFunction(DbFunction&) const;
    void OnRemoveFunction(DbFunction&) const;

    BentleyStatus Purge(ECDb::PurgeMode) const;
    BentleyStatus PurgeFileInfos() const;

    BentleyStatus AddIssueListener(IIssueListener const& issueListener) { return m_issueReporter.AddListener(issueListener); }
    void RemoveIssueListener() { m_issueReporter.RemoveListener(); }

    void AddAppData(ECDb::AppData::Key const& key, ECDb::AppData* appData, bool deleteOnClearCache) const;

    void ClearECDbCache() const;
    DbResult OnDbOpening() const { return InitializeSequences(); }
    DbResult OnDbCreated() const;
    DbResult OnBriefcaseIdChanged(BeBriefcaseId newBriefcaseId);
    void OnDbChangedByOtherConnection() const { ClearECDbCache(); }
    DbResult VerifySchemaVersion(Db::OpenParams const& params) const { return ECDbProfileManager::UpgradeECProfile(m_ecdb, params); }

    DbResult InitializeSequences() const;
    DbResult ResetSequences(BeBriefcaseId* repoId = nullptr) const;
    std::vector<BeBriefcaseBasedIdSequence const*> GetSequences() const;
public:
    ~Impl() {}

    ECDbMap const& GetECDbMap() const;

    BeBriefcaseBasedIdSequence& GetECInstanceIdSequence() { return m_ecInstanceIdSequence; }
    BeBriefcaseBasedIdSequence& GetECSchemaIdSequence() { return m_ecSchemaIdSequence; }
    BeBriefcaseBasedIdSequence& GetECClassIdSequence() { return m_ecClassIdSequence; }
    BeBriefcaseBasedIdSequence& GetECPropertyIdSequence() { return m_ecPropertyIdSequence; }
    BeBriefcaseBasedIdSequence& GetECEnumIdSequence() { return m_ecEnumIdSequence; }
    BeBriefcaseBasedIdSequence& GetKindOfQuantityIdSequence() { return m_koqIdSequence; }
    BeBriefcaseBasedIdSequence& GetTableIdSequence() { return m_tableIdSequence; }
    BeBriefcaseBasedIdSequence& GetColumnIdSequence() { return m_columnIdSequence; }
    BeBriefcaseBasedIdSequence& GetIndexIdSequence() { return m_indexIdSequence; }
    BeBriefcaseBasedIdSequence& GetClassMapIdSequence() { return m_classmapIdSequence; }
    BeBriefcaseBasedIdSequence& GetPropertyPathIdSequence() { return m_propertypathIdSequence; }

    bool TryGetSqlFunction(DbFunction*& function, Utf8CP name, int argCount) const;

    ECSqlStatementRegistry const& GetStatementRegistry() const { return m_statementRegistry; }
    IssueReporter const& GetIssueReporter() const { return m_issueReporter; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
