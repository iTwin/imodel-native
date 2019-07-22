/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//_BENTLEY_INTERNAL_ONLY_
#include "ECDbSqlFunctions.h"
#include "DbUtilities.h"
#include "SqlNames.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECSCHEMA_ECDbChange "ECDbChange"
#define ECSCHEMA_ALIAS_ECDbChange "change"

#define ECDBCHANGE_CLASS_ChangeSummary "ChangeSummary"
#define ECDBCHANGE_CLASS_InstanceChange "InstanceChange"
#define ECDBCHANGE_CLASS_PropertyValueChange "PropertyValueChange"

#define FILEEXT_ChangeCache L".ecchanges"

//=======================================================================================
// @bsiclass                                            Krischan.Eberle      11/2017
//=======================================================================================
struct ChangeManager final
    {
    private:
        ECDbCR m_ecdb;
        mutable std::unique_ptr<ChangedValueSqlFunction> m_changedValueSqlFunction;
        mutable bool m_isCacheAttached = false;

        //static non-POD must not be deleted (Bentley guideline)
        static ProfileVersion const* s_expectedCacheVersion;

        //not copyable
        ChangeManager(ChangeManager const&) = delete;
        ChangeManager& operator=(ChangeManager const&) = delete;

        DbResult AddMetadataToChangeCacheFile(ECDb& cacheFile, ECDbCR primaryECDb) const;

    public:
        explicit ChangeManager(ECDbCR ecdb) : m_ecdb(ecdb) {}

        //! Expected version of the Change Cache file (which always matches the version of the ECDbChange ECSchema)
        static ProfileVersion const& GetExpectedCacheVersion() { return *s_expectedCacheVersion; }
        static BeFileName DetermineDefaultCachePath(Utf8CP ecdbPath);

        //! This only checks whether a file with the change summary alias was attached. It could be any file though.
        //! Use IsChangeCacheAttached to find out whether the attached file is a valid change summary cache file
        bool ChangeTableSpaceExists() const { return DbUtilities::TableSpaceExists(m_ecdb, TABLESPACE_ECChange); }
        bool IsChangeCacheAttached() const { return m_isCacheAttached; }
        DbResult AttachChangeCacheFile(BeFileNameCR cacheFilePath, bool createIfNotExists) const;
        void OnDbAttached(DbTableSpace const&) const;
        void OnDbDetached(DbTableSpace const&) const;
        DbResult DetachChangeCacheFile() const;
        DbResult CreateChangeCacheFile(ECDbR, BeFileNameCR cacheFilePath) const;

        static BentleyStatus ValidateChangeCache(ECDbCR changeCache, IIssueReporter const&);

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

