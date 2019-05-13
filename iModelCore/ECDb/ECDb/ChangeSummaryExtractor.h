/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//_BENTLEY_INTERNAL_ONLY_
#include <BeSQLite/ChangeSet.h>
#include <ECDb/ChangeIterator.h>
#include "ChangeIteratorImpl.h"
#include "RelationshipClassMap.h"
#include "DbSchema.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct MainSchemaManager;

//=======================================================================================
// @bsiclass                                             Krischan.Eberle      11/2017
//=======================================================================================
struct InstanceChange final
    {
    private:
        ECInstanceId m_summaryId;
        ECInstanceKey m_keyOfChangedInstance;
        DbOpcode m_dbOpcode;
        bool m_isIndirect;

    public:
        InstanceChange() {}

        InstanceChange(ECInstanceId summaryId, ECInstanceKey const& keyOfChangedInstance, DbOpcode dbOpcode, bool isIndirect) :
            m_summaryId(summaryId), m_keyOfChangedInstance(keyOfChangedInstance), m_dbOpcode(dbOpcode), m_isIndirect(isIndirect)
            {}

        bool IsValid() const { return m_keyOfChangedInstance.GetInstanceId().IsValid(); }

        ECInstanceId GetSummaryId() const { return m_summaryId; }
        ECInstanceKey GetKeyOfChangedInstance() const { return m_keyOfChangedInstance; }

        //! Get the DbOpcode of the changed instance that indicates that the instance was inserted, updated or deleted.
        DbOpcode GetDbOpcode() const { return m_dbOpcode; }
        bool IsIndirect() const { return m_isIndirect; }
    };

struct ChangeManager;

//=======================================================================================
// @bsiclass                                            Krischan.Eberle      11/2017
//=======================================================================================
struct ChangeSummaryExtractor final
    {
    private:
        enum class ExtractMode { InstancesOnly, RelationshipInstancesOnly };

        struct Context final
            {
            private:
                ECDbCR m_primaryECDb;
                ECDb* m_userChangeCacheECDb = nullptr;
                ECDb m_ownedChangeCacheECDb;
                ECSqlStatementCache m_changeSummaryStmtCache;
                Utf8String m_attachedChangeCachePath;
                bool m_extractCompletedSuccessfully = false;

            public:
                Context(ECDbCR primaryFile, ECDbR changeCacheFile) : m_primaryECDb(primaryFile), m_userChangeCacheECDb(&changeCacheFile), m_changeSummaryStmtCache(15) {}
                Context(ECDbCR primaryFile) : m_primaryECDb(primaryFile), m_changeSummaryStmtCache(15) {}
                //Performs clean-up: 
                //*saves changes to change summary ECDb
                //*reattaches the change summary ECDb to the primary ECDb if it was attached before extraction
                ~Context();

                DbResult Initialize();
                void ExtractCompletedSuccessfully() { m_extractCompletedSuccessfully = true; }
                CachedECSqlStatementPtr GetChangeSummaryStatement(Utf8CP ecsql) const 
                    { 
                    ECDbCR changeCache = m_userChangeCacheECDb != nullptr ? *m_userChangeCacheECDb : m_ownedChangeCacheECDb;
                    return m_changeSummaryStmtCache.GetPreparedStatement(changeCache, ecsql);
                    }
                ECDbCR GetPrimaryECDb() const { return m_primaryECDb; }
                MainSchemaManager const& GetPrimaryFileSchemaManager() const;
                IssueReporter const& Issues() const;
            };

        struct FkRelChangeExtractor final
            {
            private:
                FkRelChangeExtractor() = delete;
                ~FkRelChangeExtractor() = delete;

            public:
                static BentleyStatus Extract(Context&, ECInstanceId summaryId, ChangeIterator::RowEntry const&, ChangeIterator::TableClassMap::EndTableRelationshipMap const&);
            };

        struct LinkTableRelChangeExtractor final
            {
            private:
                LinkTableRelChangeExtractor() = delete;
                ~LinkTableRelChangeExtractor() = delete;

                static BentleyStatus GetRelEndInstanceKeys(Context&, ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, ECInstanceId summaryId, ChangeIterator::RowEntry const&, RelationshipClassLinkTableMap const&, ECInstanceId relInstanceId, ECN::ECRelationshipEnd);
                static BentleyStatus GetRelEndClassId(ECN::ECClassId&, Context&, ECInstanceId summaryId, ChangeIterator::RowEntry const&, RelationshipClassLinkTableMap const&, ECInstanceId relInstanceId, ECN::ECRelationshipEnd, ECInstanceId endInstanceId, bool isBeforeChange);

            public:
                static BentleyStatus Extract(Context&, ECInstanceId summaryId, ChangeIterator::RowEntry const&, RelationshipClassLinkTableMap const&);
            };

        ChangeSummaryExtractor() = delete;
        ~ChangeSummaryExtractor() = delete;

        static BentleyStatus Extract(ECInstanceKey& changeSummaryKey, Context&, ChangeSetArg const& changeSetInfo, ECDb::ChangeSummaryExtractOptions const&);
        static BentleyStatus Extract(Context&, ECInstanceId summaryId, BeSQLite::IChangeSet&, ExtractMode);
        static BentleyStatus ExtractInstance(Context&, ECInstanceId summaryId, ChangeIterator::RowEntry const&);
        static BentleyStatus ExtractRelInstance(Context&, ECInstanceId summaryId, ChangeIterator::RowEntry const&);

        static BentleyStatus RecordInstance(Context&, InstanceChange const&, ChangeIterator::RowEntry const& rowEntry, bool recordOnlyIfUpdatedProperties);
        static BentleyStatus RecordRelInstance(Context&, InstanceChange const& instance, ChangeIterator::RowEntry const& rowEntry, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey);
        static BentleyStatus RecordValue(bool& neededToRecord, Context&, InstanceChange const&, ChangeIterator::ColumnEntry const& columnEntry);

        static InstanceChange QueryInstanceChange(Context&, ECInstanceId summaryId, ECInstanceKey const&);
        static ECInstanceId FindChangeId(Context&, ECInstanceId summaryId, ECInstanceKey const&);
        static bool ContainsChange(Context& ctx, ECInstanceId summaryId, ECInstanceKey const& keyOfChangedInstance) { return FindChangeId(ctx, summaryId, keyOfChangedInstance).IsValid(); }


        static BentleyStatus InsertSummary(ECInstanceKey& summaryKey, Context&, ChangeSetArg const&);
        static DbResult InsertOrUpdate(Context&, InstanceChange const&);
        static DbResult Delete(Context&, ECInstanceId summaryId, ECInstanceKey const&);

        static DbResult InsertPropertyValueChange(Context&, ECInstanceId summaryId, ECInstanceKey const&, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue);
        static DbResult InsertPropertyValueChange(Context&, ECInstanceId summaryId, ECInstanceKey const&, Utf8CP accessString, ECN::ECClassId oldId, ECN::ECClassId newId);
        static DbResult InsertPropertyValueChange(Context&, ECInstanceId summaryId, ECInstanceKey const&, Utf8CP accessString, ECInstanceId oldId, ECInstanceId newId);

        static ECSqlStatus BindDbValue(ECSqlStatement&, int, DbValue const&);
        static bool RawIndirectToBool(int indirect) { return indirect != 0; }
        
    public:
        static BentleyStatus Extract(ECInstanceKey& changeSummaryKey, ECDbR changeCacheECDb, ECDbCR primaryECDb, ChangeSetArg const& changeSetInfo, ECDb::ChangeSummaryExtractOptions const&);
        static BentleyStatus Extract(ECInstanceKey& changeSummaryKey, ECDbCR primaryECDb, ChangeSetArg const& changeSetInfo, ECDb::ChangeSummaryExtractOptions const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
