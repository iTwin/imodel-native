/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummaryManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//_BENTLEY_INTERNAL_ONLY_
#include "ChangeSummaryExtractor.h"
#include "ECDbSqlFunctions.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define TABLESPACE_ChangeSummaries "changesummaries"

#define ECSCHEMA_ECDbChangeSummaries "ECDbChangeSummaries"
#define ECSCHEMA_ALIAS_ECDbChangeSummaries "change"

#define ECDBCHANGE_CLASS_ChangeSummary "ChangeSummary"
#define ECDBCHANGE_CLASS_InstanceChange "InstanceChange"
#define ECDBCHANGE_CLASS_PropertyValueChange "PropertyValueChange"

#define FILEEXT_ChangeSummaryCache L".changesummaries"

//=======================================================================================
// @bsiclass                                            Krischan.Eberle      11/2017
//=======================================================================================
struct ChangeSummaryManager final : NonCopyableClass
    {
    private:
        struct SeedFile final
            {
        private:
            BeFileName m_ecdbPath;
            BeFileName m_cachePath;

            BeFileName m_seedFolder;
            BeFileName m_seedECDbPath;
            mutable BeFileName m_seedCachePath;


        public:
            SeedFile(Utf8CP ecdbPath, BeFileName const& cachePath);
            //! Deletes any seed file artefacts on disk
            ~SeedFile();

            bool IsValid() const { return !m_seedFolder.empty(); }

            DbResult CreateCacheFromSeed(BeFileNameCR targetCachePath) const;
            };

        ECDbCR m_ecdb;
        ChangeSummaryExtractor m_extractor;
        mutable std::unique_ptr<ChangedValueSqlFunction> m_changedValueSqlFunction;

        DbResult CreateChangeSummaryCacheFile(BeFileName const& cachePath) const;

        //If the cache file does not exist, the method returns without error.
        //The method does not recreate the cache file
        DbResult DoAttachChangeSummaryCacheFile(BeFileNameCR cachePath) const;

        DbResult AddMetadataToChangeSummaryCacheFile(Db& cacheFile, ProfileVersion const& ecdbProfileVersion) const;

    public:
        explicit ChangeSummaryManager(ECDbCR ecdb) : m_ecdb(ecdb), m_extractor(ecdb) {}

        BentleyStatus Extract(ECInstanceKey& summaryKey, BeSQLite::IChangeSet&, ECDb::ChangeSummaryExtractOptions const&) const;

        static BeFileName DetermineCachePath(ECDbCR);
        static BeFileName DetermineCachePath(BeFileNameCR ecdbPath);

        DbResult OnCreatingECDb() const;

        DbResult AttachChangeSummaryCacheFile(bool createIfNotExists) const;

        ChangeSummaryExtractor const& GetExtractor() const { return m_extractor; }

        void RegisterSqlFunctions() const;
        void UnregisterSqlFunction() const;

        void ClearCache();

        static Nullable<ChangeOpCode> ToChangeOpCode(DbOpcode);
        static Nullable<ChangeOpCode> ToChangeOpCode(int val);
        static Nullable<DbOpcode> ToDbOpCode(ChangeOpCode);
        static Nullable<ChangedValueState> ToChangedValueState(int val);
        static Nullable<ChangedValueState> ToChangedValueState(Utf8CP strVal);
        static Nullable<ChangeOpCode> DetermineOpCodeFromChangedValueState(ChangedValueState);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

