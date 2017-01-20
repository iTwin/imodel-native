/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbImpl.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/ECDbSchemaManager.h>
#include "BeBriefcaseBasedIdSequence.h"
#include "ECDbProfileManager.h"
#include "IssueReporter.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECCrudWriteToken final
    {};

struct DbSchemaModificationToken final
    {};

//=======================================================================================
// Holds and issues the tokens used to restrict certain ECDb APIs
// @bsiclass                                                Krischan.Eberle      12/2016
//+===============+===============+===============+===============+===============+======
struct TokenManager final : NonCopyableClass
    {
private:
    std::unique_ptr<ECCrudWriteToken> m_eccrudWriteToken;
    std::unique_ptr<DbSchemaModificationToken> m_dbSchemaModificationToken;

public:
    TokenManager() : m_eccrudWriteToken(nullptr), m_dbSchemaModificationToken(nullptr) {}

    ECCrudWriteToken const& EnableECCrudWriteTokenValidation() { m_eccrudWriteToken = std::unique_ptr<ECCrudWriteToken>(new ECCrudWriteToken()); return *m_eccrudWriteToken; }
    ECCrudWriteToken const* GetECCrudWriteToken() const { return m_eccrudWriteToken.get(); }

    DbSchemaModificationToken const& EnableDbSchemaModificationTokenValidation() { m_dbSchemaModificationToken = std::unique_ptr<DbSchemaModificationToken>(new DbSchemaModificationToken()); return *m_dbSchemaModificationToken; }
    DbSchemaModificationToken const* GetDbSchemaModificationToken() const { return m_dbSchemaModificationToken.get(); }
    };

//=======================================================================================
//! ECDb::Impl is the private implementation of ECDb hidden from the public headers
//! (PIMPL idiom)
// @bsiclass                                                Krischan.Eberle      12/2014
//+===============+===============+===============+===============+===============+======
struct ECDb::Impl : NonCopyableClass
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
    struct ClearCacheCounter
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

    
    mutable BeMutex m_mutex;
    ECDbR m_ecdb;
    std::unique_ptr<ECDbSchemaManager> m_schemaManager;
    TokenManager m_tokenManager;

    mutable BeBriefcaseBasedIdSequence m_ecInstanceIdSequence;
    BeBriefcaseBasedIdSequence m_ecSchemaIdSequence;
    BeBriefcaseBasedIdSequence m_ecClassIdSequence;
    BeBriefcaseBasedIdSequence m_ecPropertyIdSequence;
    BeBriefcaseBasedIdSequence m_ecEnumIdSequence;
    BeBriefcaseBasedIdSequence m_koqIdSequence;
    BeBriefcaseBasedIdSequence m_tableIdSequence;
    BeBriefcaseBasedIdSequence m_columnIdSequence;
    BeBriefcaseBasedIdSequence m_indexIdSequence;
    BeBriefcaseBasedIdSequence m_propertypathIdSequence;
    mutable bmap<DbFunctionKey, DbFunction*, DbFunctionKey::Comparer> m_sqlFunctions;
    mutable bset<AppData::Key const*, std::less<AppData::Key const*>> m_appDataToDeleteOnClearCache;
    mutable ClearCacheCounter m_clearCacheCounter;
    IssueReporter m_issueReporter;

    //Mirrored ECDb methods are only called by ECDb (friend), therefore private
    explicit Impl(ECDbR ecdb);

    DbResult CheckProfileVersion(bool& fileIsAutoUpgradable, bool openModeIsReadonly) const { SchemaVersion unused(0, 0, 0, 0); return ECDbProfileManager::CheckProfileVersion(fileIsAutoUpgradable, unused, m_ecdb, openModeIsReadonly); }

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

    BentleyStatus OpenBlobIO(BlobIO&, ECN::ECClassCR, Utf8CP propertyAccessString, BeInt64Id ecinstanceId, bool writable, ECCrudWriteToken const*) const;

    void ClearECDbCache() const;
    DbResult OnDbOpening() const;
    DbResult OnDbCreated() const;
    void OnDbClose() const;
    DbResult OnBriefcaseIdChanged(BeBriefcaseId newBriefcaseId);
    void OnDbChangedByOtherConnection() const { ClearECDbCache(); }
    DbResult VerifySchemaVersion(Db::OpenParams const& params) const { return ECDbProfileManager::UpgradeProfile(m_ecdb, params); }

    DbResult InitializeSequences() const;
    DbResult ResetSequences(BeBriefcaseId* repoId = nullptr) const;
    std::vector<BeBriefcaseBasedIdSequence const*> GetSequences() const;

    void RegisterBuiltinFunctions() const;
    void UnregisterBuiltinFunctions() const;

    static DbResult Initialize(BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors);

public:
    ~Impl() {}

    BeBriefcaseBasedIdSequence& GetECInstanceIdSequence() { return m_ecInstanceIdSequence; }
    BeBriefcaseBasedIdSequence& GetECSchemaIdSequence() { return m_ecSchemaIdSequence; }
    BeBriefcaseBasedIdSequence& GetECClassIdSequence() { return m_ecClassIdSequence; }
    BeBriefcaseBasedIdSequence& GetECPropertyIdSequence() { return m_ecPropertyIdSequence; }
    BeBriefcaseBasedIdSequence& GetECEnumIdSequence() { return m_ecEnumIdSequence; }
    BeBriefcaseBasedIdSequence& GetKindOfQuantityIdSequence() { return m_koqIdSequence; }
    BeBriefcaseBasedIdSequence& GetTableIdSequence() { return m_tableIdSequence; }
    BeBriefcaseBasedIdSequence& GetColumnIdSequence() { return m_columnIdSequence; }
    BeBriefcaseBasedIdSequence& GetIndexIdSequence() { return m_indexIdSequence; }
    BeBriefcaseBasedIdSequence& GetPropertyPathIdSequence() { return m_propertypathIdSequence; }

    bool TryGetSqlFunction(DbFunction*& function, Utf8CP name, int argCount) const;

    TokenManager const& GetTokenManager() const { return m_tokenManager; }

    //! The clear cache counter is incremented with every call to ClearECDbCache. This is used
    //! by code that refers to objects held in the cache to invalidate itself.
    //! E.g. Any existing ECSqlStatement would be invalid after ClearECDbCache and would return
    //! an error from any of its methods.
    ClearCacheCounter const& GetClearCacheCounter() const { return m_clearCacheCounter; }

    IssueReporter const& GetIssueReporter() const { return m_issueReporter; }

    BeMutex& GetMutex() const { return m_mutex; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
