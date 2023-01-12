/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/SchemaManager.h>
#include <BeSQLite/BeBriefcaseBasedIdSequence.h>
#include "ChangeManager.h"
#include "ProfileManager.h"
#include "IssueReporter.h"
#include <atomic>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct IdFactory final: NonCopyableClass {
    struct IdSequence final: NonCopyableClass {
        private:
            mutable std::atomic<uint64_t> m_id;
            bool m_isIntializedFromTable;
        public:
            explicit IdSequence(uint64_t id, bool isIntializedFromTable) :m_id(id), m_isIntializedFromTable(isIntializedFromTable){}
            BeInt64Id NextId() const { BeAssert(m_isIntializedFromTable); return BeInt64Id(++m_id); }
            bool IsIntializedFromTable() const { return m_isIntializedFromTable; }
            static std::unique_ptr<IdSequence> Create(ECDbCR db, Utf8CP tableName, Utf8CP idColumnName);
    };

    private:
        mutable std::unique_ptr<IdSequence> m_classIdSeq;
        mutable std::unique_ptr<IdSequence> m_classHasBaseClassesIdSeq;
        mutable std::unique_ptr<IdSequence> m_columnIdSeq;
        mutable std::unique_ptr<IdSequence> m_customAttributeIdSeq;
        mutable std::unique_ptr<IdSequence> m_enumerationIdSeq;
        mutable std::unique_ptr<IdSequence> m_formatIdSeq;
        mutable std::unique_ptr<IdSequence> m_formatCompositeUnitIdSeq;
        mutable std::unique_ptr<IdSequence> m_indexIdSeq;
        mutable std::unique_ptr<IdSequence> m_indexColumnIdSeq;
        mutable std::unique_ptr<IdSequence> m_kindOfQuantityIdSeq;
        mutable std::unique_ptr<IdSequence> m_phenomenonIdSeq;
        mutable std::unique_ptr<IdSequence> m_propertyIdSeq;
        mutable std::unique_ptr<IdSequence> m_propertyCategoryIdSeq;
        mutable std::unique_ptr<IdSequence> m_propertyMapSeq;
        mutable std::unique_ptr<IdSequence> m_propertyPathIdSeq;
        mutable std::unique_ptr<IdSequence> m_relationshipConstraintIdSeq;
        mutable std::unique_ptr<IdSequence> m_relationshipConstraintClassIdSeq;
        mutable std::unique_ptr<IdSequence> m_schemaIdSeq;
        mutable std::unique_ptr<IdSequence> m_schemaReferencIdSeq;
        mutable std::unique_ptr<IdSequence> m_tableIdSeq;
        mutable std::unique_ptr<IdSequence> m_unitIdSeq;
        mutable std::unique_ptr<IdSequence> m_unitSystemIdSeq;
        ECDbCR m_ecdb;
    public:
        explicit IdFactory(ECDbCR);
        IdSequence& Class() const { return *m_classIdSeq; }
        IdSequence& ClassHasBaseClasses() const { return *m_classHasBaseClassesIdSeq; }
        IdSequence& Column() const { return *m_columnIdSeq; }
        IdSequence& CustomAttribute() const { return *m_customAttributeIdSeq; }
        IdSequence& Enumeration() const { return *m_enumerationIdSeq; }
        IdSequence& Format() const { return *m_formatIdSeq; }
        IdSequence& FormatCompositeUnit() const { return *m_formatCompositeUnitIdSeq; }
        IdSequence& Index() const { return *m_indexIdSeq; }
        IdSequence& IndexColumn() const { return *m_indexColumnIdSeq; }
        IdSequence& KindOfQuantity() const { return *m_kindOfQuantityIdSeq; }
        IdSequence& Phenomenon() const { return *m_phenomenonIdSeq; }
        IdSequence& Property() const { return *m_propertyIdSeq; }
        IdSequence& PropertyCategory() const { return *m_propertyCategoryIdSeq; }
        IdSequence& PropertyMap() const { return *m_propertyMapSeq; }
        IdSequence& PropertyPath() const { return *m_propertyPathIdSeq; }
        IdSequence& RelationshipConstraint() const { return *m_relationshipConstraintIdSeq; }
        IdSequence& RelationshipConstraintClass() const { return *m_relationshipConstraintClassIdSeq; }
        IdSequence& Schema() const { return *m_schemaIdSeq; }
        IdSequence& SchemaReference() const { return *m_schemaReferencIdSeq; }
        IdSequence& Table() const { return *m_tableIdSeq; }
        IdSequence& Unit() const { return *m_unitIdSeq; }
        IdSequence& UnitSystem() const { return *m_unitSystemIdSeq; }
        bool IsValid() const;
        bool Reset() const;
        static std::unique_ptr<IdFactory> Create(ECDbCR ecdb);
};

struct PragmaManager;
//=======================================================================================
//! ECDb::Impl is the private implementation of ECDb hidden from the public headers
//! (PIMPL idiom)
// @bsiclass
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
    // @bsiclass
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
        Utf8String m_functionName;
        int m_argCount;

        DbFunctionKey(): m_argCount(-1) {}
        DbFunctionKey(Utf8CP functionName, int argCount) : m_functionName(functionName), m_argCount(argCount) {}
        explicit DbFunctionKey(DbFunction& function) : m_functionName(function.GetName()), m_argCount(function.GetNumArgs()) {}

        struct Comparer
            {
            bool operator() (DbFunctionKey const& lhs, DbFunctionKey const& rhs) const
                {
                int nameCompareResult = BeStringUtilities::StricmpAscii(lhs.m_functionName.c_str(), rhs.m_functionName.c_str());
                return nameCompareResult < 0 || nameCompareResult == 0 && lhs.m_argCount < rhs.m_argCount;
                }
            };
        };


    static bool s_isInitialized;
    mutable BeMutex m_mutex;
    ECDbR m_ecdb;
    mutable BeGuid m_id;
    ProfileManager m_profileManager;
    std::unique_ptr<SchemaManager> m_schemaManager;
    ChangeManager m_changeManager;
    SettingsManager m_settingsManager;
    StatementCache m_sqliteStatementCache;
    mutable std::unique_ptr<InstanceReader> m_instanceReader;
    BeBriefcaseBasedIdSequenceManager m_idSequenceManager;
    static const uint32_t s_instanceIdSequenceKey = 0;
    mutable bmap<DbFunctionKey, DbFunction*, DbFunctionKey::Comparer> m_sqlFunctions;
    mutable bset<AppData::Key const*, std::less<AppData::Key const*>> m_appDataToDeleteOnClearCache;
    mutable ClearCacheCounter m_clearCacheCounter;
    bvector<IECDbCacheClearListener*> m_ecdbCacheClearListeners;
    IssueDataSource m_issueReporter;
    mutable std::unique_ptr<ClassNameFunc> m_classNameFunc;
    mutable std::unique_ptr<ClassIdFunc> m_classIdFunc;
    mutable std::unique_ptr<InstanceOfFunc> m_instanceOfFunc;
    mutable std::unique_ptr<IdFactory> m_idFactory;
    mutable std::unique_ptr<ExtractInstFunc> m_extractInstFunc;
    mutable std::unique_ptr<ExtractPropFunc> m_extractPropFunc;
    mutable std::unique_ptr<PropExistsFunc> m_propExistsFunc;
    mutable EC::ECSqlConfig m_ecSqlConfig;
    mutable std::unique_ptr<PragmaManager> m_pragmaProcessor;
    //Mirrored ECDb methods are only called by ECDb (friend), therefore private
    explicit Impl(ECDbR ecdb);

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

    BentleyStatus AddIssueListener(ECN::IIssueListener const& issueListener) { m_issueReporter.SetOldStyleListener(&issueListener); return SUCCESS; }
    void RemoveIssueListener() { m_issueReporter.SetOldStyleListener(nullptr); }

    void AddECDbCacheClearListener(IECDbCacheClearListener& listener) { m_ecdbCacheClearListeners.push_back(&listener); }
    void RemoveECDbCacheClearListener(IECDbCacheClearListener& listener) { m_ecdbCacheClearListeners.erase(std::find(m_ecdbCacheClearListeners.begin(), m_ecdbCacheClearListeners.end(), &listener), m_ecdbCacheClearListeners.end()); }

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
    static bool IsInitialized() { return s_isInitialized; }
public:
    ~Impl() { m_sqliteStatementCache.Empty(); }
    EC::ECSqlConfig& GetECSqlConfig() const { return m_ecSqlConfig; }
    bvector<DbFunction*> GetSqlFunctions() const;
    bool TryGetSqlFunction(DbFunction*& function, Utf8CP name, int argCount) const;
    ECDb::SettingsManager const& GetSettingsManager() const { return m_settingsManager; }
    bool TryGetChangeCacheFileName(BeFileNameR changeCachePath) const;
    CachedStatementPtr GetCachedSqliteStatement(Utf8CP sql) const;
    BeBriefcaseBasedIdSequence const& GetInstanceIdSequence() const { return m_idSequenceManager.GetSequence(s_instanceIdSequenceKey); }
    ChangeManager const& GetChangeManager() const { return m_changeManager; }
    BeGuid GetId() const  {return m_id; }
    IdFactory& GetIdFactory() const;
    PragmaManager& GetPragmaManager() const;
    //! The clear cache counter is incremented with every call to ClearECDbCache. This is used
    //! by code that refers to objects held in the cache to invalidate itself.
    //! E.g. Any existing ECSqlStatement would be invalid after ClearECDbCache and would return
    //! an error from any of its methods.
    ClearCacheCounter const& GetClearCacheCounter() const { return m_clearCacheCounter; }
    InstanceReader& GetInstanceReader() const { 
        if (m_instanceReader == nullptr) {
            BeMutexHolder holder(m_mutex);
            if (m_instanceReader == nullptr) {
                m_instanceReader = std::make_unique<InstanceReader>(m_ecdb);
            }
        }
        return *m_instanceReader; 
    }
    IssueDataSource const& Issues() const { return m_issueReporter; }

    BeMutex& GetMutex() const { return m_mutex; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
