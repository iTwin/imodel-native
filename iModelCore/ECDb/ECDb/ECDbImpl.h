/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/SchemaManager.h>
#include <BeSQLite/BeBriefcaseBasedIdSequence.h>
#include "ChangeManager.h"
#include "ProfileManager.h"
#include "IssueReporter.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECDb::Impl is the private implementation of ECDb hidden from the public headers
//! (PIMPL idiom)
// @bsiclass                                                Krischan.Eberle      12/2014
//+===============+===============+===============+===============+===============+======
struct ECDb::Impl final
    {
friend struct ECDb;

public:
    //=======================================================================================
    //! The clear cache counter is incremented with every call to ClearECDbCache. This is used
    //! by code that refers to objects held in the cache to invalidate itself.
    //! E.g. Any existing ECSqlStatement would be invalid after ClearECDbCache and would return
    //! an error from any of its methods.
    // @bsiclass                                                Krischan.Eberle       10/2016
    //+===============+===============+===============+===============+===============+======
    struct ClearCacheCounter final
        {
        private:
            uint32_t m_value;

        public:
            ClearCacheCounter() : m_value(0) {}

            bool operator==(ClearCacheCounter const& rhs) const { return m_value == rhs.m_value; }
            bool operator!=(ClearCacheCounter const& rhs) const { return m_value != rhs.m_value; }

            void Increment() { m_value++; }
            uint32_t GetValue() const { return m_value; }
        };

private:
    struct DbFunctionKey final
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

    
    static bool s_isInitalized;
    mutable BeMutex m_mutex;
    ECDbR m_ecdb;
    ProfileManager m_profileManager;
    std::unique_ptr<SchemaManager> m_schemaManager;
    ChangeManager m_changeManager;
    SettingsManager m_settingsManager;

    StatementCache m_sqliteStatementCache;
    BeBriefcaseBasedIdSequenceManager m_idSequenceManager;
    static const uint32_t s_instanceIdSequenceKey = 0;
    mutable bmap<DbFunctionKey, DbFunction*, DbFunctionKey::Comparer> m_sqlFunctions;
    mutable bset<AppData::Key const*, std::less<AppData::Key const*>> m_appDataToDeleteOnClearCache;
    mutable ClearCacheCounter m_clearCacheCounter;
    IssueReporter m_issueReporter;

    //Mirrored ECDb methods are only called by ECDb (friend), therefore private
    explicit Impl(ECDbR ecdb) : m_ecdb(ecdb), m_profileManager(ecdb), m_changeManager(ecdb), m_sqliteStatementCache(50, &m_mutex), m_idSequenceManager(ecdb, bvector<Utf8CP>(1, "ec_instanceidsequence"))
        {
        m_schemaManager = std::make_unique<SchemaManager>(ecdb, m_mutex);
        }

    //not copyable
    Impl(Impl const&) = delete;
    Impl& operator=(Impl const&) = delete;

    ProfileManager const& GetProfileManager() const { return m_profileManager; }

    SchemaManager const& Schemas() const { return *m_schemaManager; }
    ECN::IECSchemaLocaterR GetSchemaLocater() const { return *m_schemaManager; }
    ECN::IECClassLocaterR GetClassLocater() const { return *m_schemaManager; }

    BentleyStatus OnAddFunction(DbFunction&) const;
    void OnRemoveFunction(DbFunction&) const;

    BentleyStatus Purge(ECDb::PurgeMode) const;
    BentleyStatus PurgeFileInfos() const;

    BentleyStatus AddIssueListener(IIssueListener const& issueListener) { return m_issueReporter.AddListener(issueListener); }
    void RemoveIssueListener() { m_issueReporter.RemoveListener(); }

    void AddAppData(ECDb::AppData::Key const& key, ECDb::AppData* appData, bool deleteOnClearCache) const;

    BentleyStatus OpenBlobIO(BlobIO&, Utf8CP tableSpace, ECN::ECClassCR, Utf8CP propertyAccessString, BeInt64Id ecinstanceId, bool writable, ECCrudWriteToken const*) const;
    bool IsChangeCacheAttached() const { return m_changeManager.IsChangeCacheAttached(); }
    void ClearECDbCache() const;
    DbResult OnDbOpening() const;
    DbResult OnDbCreated() const;
    DbResult OnDbAttached(Utf8CP dbFileName, Utf8CP tableSpaceName) const;
    DbResult OnDbDetached(Utf8CP tableSpaceName) const;

    DbResult OnBriefcaseIdAssigned(BeBriefcaseId newBriefcaseId);
    void OnDbChangedByOtherConnection() const { ClearECDbCache(); }
    BentleyStatus ResetInstanceIdSequence(BeBriefcaseId, IdSet<ECN::ECClassId> const* ecClassIgnoreList);

    BentleyStatus DetermineMaxInstanceIdForBriefcase(ECInstanceId& maxId, BeBriefcaseId, IdSet<ECN::ECClassId> const* ecClassIgnoreList) const;
    BentleyStatus DetermineMaxIdForBriefcase(BeBriefcaseBasedId& maxId, BeBriefcaseId, Utf8CP tableName, Utf8CP idColName) const;

    void RegisterBuiltinFunctions() const;
    void UnregisterBuiltinFunctions() const;

    static DbResult InitializeLib(BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors);
    static bool IsInitialized() { return s_isInitalized; }
public:
    ~Impl() { m_sqliteStatementCache.Empty(); }
   
    bool TryGetSqlFunction(DbFunction*& function, Utf8CP name, int argCount) const;
    ECDb::SettingsManager const& GetSettingsManager() const { return m_settingsManager; }

    CachedStatementPtr GetCachedSqliteStatement(Utf8CP sql) const;
    BeBriefcaseBasedIdSequence const& GetInstanceIdSequence() const { return m_idSequenceManager.GetSequence(s_instanceIdSequenceKey); }

    ChangeManager const& GetChangeManager() const { return m_changeManager; }

    //! The clear cache counter is incremented with every call to ClearECDbCache. This is used
    //! by code that refers to objects held in the cache to invalidate itself.
    //! E.g. Any existing ECSqlStatement would be invalid after ClearECDbCache and would return
    //! an error from any of its methods.
    ClearCacheCounter const& GetClearCacheCounter() const { return m_clearCacheCounter; }

    IssueReporter const& Issues() const { return m_issueReporter; }

    BeMutex& GetMutex() const { return m_mutex; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
