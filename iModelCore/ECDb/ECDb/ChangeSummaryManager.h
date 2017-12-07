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
#include "DbUtilities.h"
#include "SqlNames.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


#define ECSCHEMA_ECDbChangeSummaries "ECDbChangeSummaries"
#define ECSCHEMA_ALIAS_ECDbChangeSummaries "change"

#define ECDBCHANGE_CLASS_ChangeSummary "ChangeSummary"
#define ECDBCHANGE_CLASS_InstanceChange "InstanceChange"
#define ECDBCHANGE_CLASS_PropertyValueChange "PropertyValueChange"

#define TABLE_ChangeSummary "change_" ECDBCHANGE_CLASS_ChangeSummary

#define FILEEXT_ChangeSummaryCache L".changesummaries"

//=======================================================================================
// @bsiclass                                            Krischan.Eberle      11/2017
//=======================================================================================
struct ChangeSummaryManager final
    {
    private:
        ECDbCR m_ecdb;
        ChangeSummaryExtractor m_extractor;
        mutable std::unique_ptr<ChangedValueSqlFunction> m_changedValueSqlFunction;
        //static non-POD must not be deleted (Bentley guideline)
        static ProfileVersion const* s_expectedCacheVersion;

        //not copyable
        ChangeSummaryManager(ChangeSummaryManager const&) = delete;
        ChangeSummaryManager& operator=(ChangeSummaryManager const&) = delete;

        DbResult CreateCacheFile(BeFileNameCR cachePath) const;
        DbResult AddMetadataToChangeSummaryCacheFile(ECDb& cacheFile, ECDbCR primaryECDb) const;

    public:
        explicit ChangeSummaryManager(ECDbCR ecdb) : m_ecdb(ecdb) {}

        BentleyStatus Extract(ECInstanceKey& summaryKey, BeSQLite::IChangeSet&, ECDb::ChangeSummaryExtractOptions const&) const;

        //! Expected version of the ChangeSummary Cache file (which always matches the version of the ECDbChangeSummaries ECSchema)
        static ProfileVersion const& GetExpectedCacheVersion() { return *s_expectedCacheVersion; }
        static BeFileName DetermineCachePath(ECDbCR);
        static BeFileName DetermineCachePath(BeFileNameCR ecdbPath);

        //! This only checks whether a file with the change summary alias was attached. It could be any file though.
        //! Use IsChangeSummaryCacheAttached to find out whether the attached file is a valid change summary cache file
        bool ChangeSummaryTableSpaceExists() const { return DbUtilities::TableSpaceExists(m_ecdb, TABLESPACE_ChangeSummaries); }
        bool IsChangeSummaryCacheAttachedAndValid(bool logError = false) const;
        static bool IsChangeSummaryCacheValid(ECDbCR cacheFile, bool logError = false);

        DbResult AttachChangeSummaryCacheFile(bool createIfNotExists) const;

        ChangeSummaryExtractor const& GetExtractor() const { return m_extractor; }

        void RegisterSqlFunctions() const;
        void UnregisterSqlFunction() const;

        ECDbCR GetECDb() const { return m_ecdb; }
        void ClearCache();

        static Nullable<ChangeOpCode> ToChangeOpCode(DbOpcode);
        static Nullable<ChangeOpCode> ToChangeOpCode(int val);
        static Nullable<DbOpcode> ToDbOpCode(ChangeOpCode);
        static Nullable<ChangedValueState> ToChangedValueState(int val);
        static Nullable<ChangedValueState> ToChangedValueState(Utf8CP strVal);
        static Nullable<ChangeOpCode> DetermineOpCodeFromChangedValueState(ChangedValueState);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

