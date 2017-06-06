/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbImpl.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/SchemaManager.h>
#include <BeSQLite/BeBriefcaseBasedIdSequence.h>
#include "ProfileManager.h"
#include "IssueReporter.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECCrudWriteToken final
    {};

struct SchemaImportToken final
    {};

//=======================================================================================
// Holds and issues the tokens used to restrict certain ECDb APIs
// @bsiclass                                                Krischan.Eberle      12/2016
//+===============+===============+===============+===============+===============+======
struct SettingsHolder final : NonCopyableClass
    {
private:
    ECDb::Settings m_settings;

    std::unique_ptr<ECCrudWriteToken> m_eccrudWriteToken;
    std::unique_ptr<SchemaImportToken> m_ecSchemaImportToken;

public:
    SettingsHolder() : m_eccrudWriteToken(nullptr), m_ecSchemaImportToken(nullptr) {}

    void ApplySettings(bool requireECCrudTokenValidation, bool requireECSchemaImportTokenValidation, bool allowChangesetMergingIncompatibleECSchemaImport);

    ECDb::Settings const& GetSettings() const { return m_settings; }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      02/2017
//+===============+===============+===============+===============+===============+======
struct IdSequences final : NonCopyableClass
    {
public:
    //the numbers are the indexes into the sequence vector of SequenceManager. So they
    //must match the order of the names in the name vector passed in the ctor
    enum class Key : uint32_t
        {
        InstanceId = 0,
        SchemaId = 1,
        SchemaReferenceId,
        ClassId,
        ClassHasBaseClassesId,
        PropertyId,
        PropertyPathId,
        RelationshipConstraintId,
        RelationshipConstraintClassId,
        CustomAttributeId,
        EnumId,
        KoqId,
        PropertyMapId,
        TableId,
        ColumnId,
        IndexId,
        IndexColumnId
        };

private:
    BeBriefcaseBasedIdSequenceManager m_sequenceManager;

public:
    explicit IdSequences(ECDbR ecdb) : 
        m_sequenceManager(ecdb, {"ec_instanceidsequence", "ec_schemaidsequence","ec_schemarefidsequence", "ec_classidsequence","ec_classhasbaseclassesidsequence",
                                 "ec_propertyidsequence","ec_propertypathidsequence", 
                                "ec_relconstraintidsequence", "ec_relconstraintclassidsequence",
                                "ec_customattributeidsequence", "ec_enumidsequence","ec_koqidsequence", "ec_propertymapidsequence",
                                 "ec_tableidsequence","ec_columnidsequence", "ec_indexidsequence", "ec_indexcolumnidsequence"})
        {}

    BeBriefcaseBasedIdSequence const& GetSequence(Key key) const { return m_sequenceManager.GetSequence(Enum::Convert<Key, uint32_t>(key)); }
    BeBriefcaseBasedIdSequenceManager const& GetManager() const { return m_sequenceManager; }
    };

//=======================================================================================
//! ECDb::Impl is the private implementation of ECDb hidden from the public headers
//! (PIMPL idiom)
// @bsiclass                                                Krischan.Eberle      12/2014
//+===============+===============+===============+===============+===============+======
struct ECDb::Impl final : NonCopyableClass
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

    
    mutable BeMutex m_mutex;
    ECDbR m_ecdb;
    std::unique_ptr<SchemaManager> m_schemaManager;

    SettingsHolder m_settings;

    IdSequences m_idSequences;
    mutable bmap<DbFunctionKey, DbFunction*, DbFunctionKey::Comparer> m_sqlFunctions;
    mutable bset<AppData::Key const*, std::less<AppData::Key const*>> m_appDataToDeleteOnClearCache;
    mutable ClearCacheCounter m_clearCacheCounter;
    IssueReporter m_issueReporter;

    //Mirrored ECDb methods are only called by ECDb (friend), therefore private
    explicit Impl(ECDbR ecdb) : m_ecdb(ecdb), m_idSequences(ecdb) { m_schemaManager = std::make_unique<SchemaManager>(ecdb, m_mutex); }

    DbResult CheckProfileVersion(bool& fileIsAutoUpgradable, bool openModeIsReadonly) const;

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

    BentleyStatus OpenBlobIO(BlobIO&, ECN::ECClassCR, Utf8CP propertyAccessString, BeInt64Id ecinstanceId, bool writable, ECCrudWriteToken const*) const;

    void ClearECDbCache() const;
    DbResult OnDbOpening() const;
    DbResult OnDbCreated() const;
    void OnDbClose() const;
    DbResult OnBriefcaseIdAssigned(BeBriefcaseId newBriefcaseId);
    void OnDbChangedByOtherConnection() const { ClearECDbCache(); }
    DbResult VerifyProfileVersion(Db::OpenParams const& params) const { return ProfileManager::UpgradeProfile(m_ecdb, params); }

    void RegisterBuiltinFunctions() const;
    void UnregisterBuiltinFunctions() const;

    static DbResult InitializeLib(BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors);

public:
    ~Impl() {}

    bool TryGetSqlFunction(DbFunction*& function, Utf8CP name, int argCount) const;
    ECDb::Settings const& GetSettings() const { return m_settings.GetSettings(); }

    BeBriefcaseBasedIdSequence const& GetSequence(IdSequences::Key sequenceKey) const { return m_idSequences.GetSequence(sequenceKey); }

    //! The clear cache counter is incremented with every call to ClearECDbCache. This is used
    //! by code that refers to objects held in the cache to invalidate itself.
    //! E.g. Any existing ECSqlStatement would be invalid after ClearECDbCache and would return
    //! an error from any of its methods.
    ClearCacheCounter const& GetClearCacheCounter() const { return m_clearCacheCounter; }

    IssueReporter const& GetIssueReporter() const { return m_issueReporter; }

    BeMutex& GetMutex() const { return m_mutex; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
