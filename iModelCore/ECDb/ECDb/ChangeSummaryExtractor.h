/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummaryExtractor.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

struct ChangeSummaryManager;

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
                ChangeSummaryManager& m_manager;
                ECDb m_changeSummaryECDb;
                ECSqlStatementCache m_changeSummaryStmtCache;
                bool m_wasChangeSummaryFileAttached = false;
                bool m_extractCompletedSuccessfully = false;
            public:
                explicit Context(ChangeSummaryManager& manager);
                //Performs clean-up: 
                //*saves changes to change summary ECDb and closes it
                //*reattaches the change summary ECDb to the primary ECDb if it was attached before extraction
                ~Context();

                DbResult OpenChangeSummaryECDb();
                void ExtractCompletedSuccessfully() { m_extractCompletedSuccessfully = true; }
                ECDbCR GetChangeSummaryECDb() const { return m_changeSummaryECDb; }
                CachedECSqlStatementPtr GetChangeSummaryStatement(Utf8CP ecsql) const { return m_changeSummaryStmtCache.GetPreparedStatement(m_changeSummaryECDb, ecsql); }
                ECDbCR GetPrimaryECDb() const;
                MainSchemaManager const& GetSchemaManager() const;
                IssueReporter const& Issues() const;
            };

        struct FkRelChangeExtractor final
            {
            private:
                ChangeSummaryExtractor const& m_extractor;

            public:
                explicit FkRelChangeExtractor(ChangeSummaryExtractor const& extractor) : m_extractor(extractor) {}
                BentleyStatus Extract(Context&, ECInstanceId summaryId, ChangeIterator::RowEntry const&, ChangeIterator::TableClassMap::EndTableRelationshipMap const&) const;
            };

        struct LinkTableRelChangeExtractor final
            {
            private:
                ChangeSummaryExtractor const& m_extractor;

                BentleyStatus GetRelEndInstanceKeys(Context&, ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, ECInstanceId summaryId, ChangeIterator::RowEntry const&, RelationshipClassLinkTableMap const&, ECInstanceId relInstanceId, ECN::ECRelationshipEnd) const;
                ECN::ECClassId GetRelEndClassId(Context&, ECInstanceId summaryId, ChangeIterator::RowEntry const&, RelationshipClassLinkTableMap const&, ECInstanceId relInstanceId, ECN::ECRelationshipEnd, ECInstanceId endInstanceId) const;

            public:
                explicit LinkTableRelChangeExtractor(ChangeSummaryExtractor const& extractor) : m_extractor(extractor) {}
                BentleyStatus Extract(Context&, ECInstanceId summaryId, ChangeIterator::RowEntry const&, RelationshipClassLinkTableMap const&) const;
            };

        //not copyable
        ChangeSummaryExtractor(ChangeSummaryExtractor const&) = delete;
        ChangeSummaryExtractor& operator=(ChangeSummaryExtractor const&) = delete;

        BentleyStatus Extract(Context&, ECInstanceId summaryId, IChangeSet& changeSet, ExtractMode) const;
        BentleyStatus ExtractInstance(Context&, ECInstanceId summaryId, ChangeIterator::RowEntry const&) const;
        BentleyStatus ExtractRelInstance(Context&, ECInstanceId summaryId, ChangeIterator::RowEntry const&) const;

        BentleyStatus RecordInstance(Context&, InstanceChange const&, ChangeIterator::RowEntry const& rowEntry, bool recordOnlyIfUpdatedProperties) const;
        BentleyStatus RecordRelInstance(Context&, InstanceChange const& instance, ChangeIterator::RowEntry const& rowEntry, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey) const;
        BentleyStatus RecordValue(bool& neededToRecord, Context&, InstanceChange const&, ChangeIterator::ColumnEntry const& columnEntry) const;

        InstanceChange QueryInstanceChange(Context&, ECInstanceId summaryId, ECInstanceKey const&) const;
        ECInstanceId FindChangeId(Context&, ECInstanceId summaryId, ECInstanceKey const&) const;
        bool ContainsChange(Context& ctx, ECInstanceId summaryId, ECInstanceKey const& keyOfChangedInstance) const { return FindChangeId(ctx, summaryId, keyOfChangedInstance).IsValid(); }


        BentleyStatus InsertSummary(ECInstanceKey& summaryKey, Context&) const;
        DbResult InsertOrUpdate(Context&, InstanceChange const&) const;
        DbResult Delete(Context&, ECInstanceId summaryId, ECInstanceKey const&) const;

        DbResult InsertPropertyValueChange(Context&, ECInstanceId summaryId, ECInstanceKey const&, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue) const;
        DbResult InsertPropertyValueChange(Context&, ECInstanceId summaryId, ECInstanceKey const&, Utf8CP accessString, ECN::ECClassId oldId, ECN::ECClassId newId) const;
        DbResult InsertPropertyValueChange(Context&, ECInstanceId summaryId, ECInstanceKey const&, Utf8CP accessString, ECInstanceId oldId, ECInstanceId newId) const;

        static ECSqlStatus BindDbValue(ECSqlStatement&, int, DbValue const&);
         static bool RawIndirectToBool(int indirect) { return indirect != 0; }
        
    public:
        ChangeSummaryExtractor()  {}

        BentleyStatus Extract(ECInstanceKey& changeSummaryKey, ChangeSummaryManager&, IChangeSet& changeSet, ECDb::ChangeSummaryExtractOptions const&) const;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
