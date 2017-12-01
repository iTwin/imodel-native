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

#define FILEEXT_ChangeSummaryCache L".changesummaries"

//=======================================================================================
// @bsiclass                                            Krischan.Eberle      11/2017
//=======================================================================================
struct ChangeSummaryManager final : NonCopyableClass
    {
    private:
        ECDbCR m_ecdb;
        ChangeSummaryExtractor m_extractor;
        mutable std::unique_ptr<ChangedValueSqlFunction> m_changedValueSqlFunction;

        DbResult CreateCacheFile(BeFileNameCR cachePath) const;
        DbResult AddMetadataToChangeSummaryCacheFile(ECDb& cacheFile, ECDbCR primaryECDb) const;

    public:
        explicit ChangeSummaryManager(ECDbCR ecdb) : m_ecdb(ecdb) {}

        BentleyStatus Extract(ECInstanceKey& summaryKey, BeSQLite::IChangeSet&, ECDb::ChangeSummaryExtractOptions const&) const;

        static BeFileName DetermineCachePath(ECDbCR);
        static BeFileName DetermineCachePath(BeFileNameCR ecdbPath);

        bool IsChangeSummaryCacheAttached() const { return DbUtilities::TableSpaceExists(m_ecdb, TABLESPACE_ChangeSummaries); }
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

