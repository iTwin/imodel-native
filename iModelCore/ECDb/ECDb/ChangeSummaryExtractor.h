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

//=======================================================================================
// @bsiclass                                            Krischan.Eberle      11/2017
//=======================================================================================
struct ChangeSummaryExtractor final : NonCopyableClass
    {
    private:
        enum class ExtractMode { InstancesOnly, RelationshipInstancesOnly };

        struct FkRelChangeExtractor final
            {
            private:
                ChangeSummaryExtractor const& m_extractor;

            public:
                explicit FkRelChangeExtractor(ChangeSummaryExtractor const& extractor) : m_extractor(extractor) {}
                BentleyStatus Extract(ECInstanceId summaryId, ChangeIterator::RowEntry const&, ChangeIterator::TableClassMap::EndTableRelationshipMap const&) const;
            };

        struct LinkTableRelChangeExtractor final
            {
            private:
                ChangeSummaryExtractor const& m_extractor;

                BentleyStatus GetRelEndInstanceKeys(ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, ECInstanceId summaryId, ChangeIterator::RowEntry const&, RelationshipClassLinkTableMap const&, ECInstanceId relInstanceId, ECN::ECRelationshipEnd) const;
                ECN::ECClassId GetRelEndClassId(ECInstanceId summaryId, ChangeIterator::RowEntry const&, RelationshipClassLinkTableMap const&, ECInstanceId relInstanceId, ECN::ECRelationshipEnd, ECInstanceId endInstanceId) const;

            public:
                explicit LinkTableRelChangeExtractor(ChangeSummaryExtractor const& extractor) : m_extractor(extractor) {}
                BentleyStatus Extract(ECInstanceId summaryId, ChangeIterator::RowEntry const&, RelationshipClassLinkTableMap const&) const;
            };

        ECDbCR m_ecdb;
        ECSqlStatementCache m_stmtCache;

        BentleyStatus Extract(ECInstanceId summaryId, IChangeSet& changeSet, ExtractMode) const;
        BentleyStatus ExtractInstance(ECInstanceId summaryId, ChangeIterator::RowEntry const&) const;
        BentleyStatus ExtractRelInstance(ECInstanceId summaryId, ChangeIterator::RowEntry const&) const;

        BentleyStatus RecordInstance(InstanceChange const&, ChangeIterator::RowEntry const& rowEntry, bool recordOnlyIfUpdatedProperties) const;
        BentleyStatus RecordRelInstance(InstanceChange const& instance, ChangeIterator::RowEntry const& rowEntry, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey) const;
        BentleyStatus RecordValue(bool& neededToRecord, InstanceChange const&, ChangeIterator::ColumnEntry const& columnEntry) const;       

        InstanceChange QueryInstanceChange(ECInstanceId summaryId, ECInstanceKey const&) const;
        ECInstanceId FindChangeId(ECInstanceId summaryId, ECInstanceKey const&) const;
        bool ContainsChange(ECInstanceId summaryId, ECInstanceKey const& keyOfChangedInstance) const { return FindChangeId(summaryId, keyOfChangedInstance).IsValid(); }


        BentleyStatus InsertSummary(ECInstanceId& summaryId) const;
        DbResult InsertOrUpdate(InstanceChange const&) const;
        DbResult Delete(ECInstanceId summaryId, ECInstanceKey const&) const;

        DbResult InsertPropertyValueChange(ECInstanceId summaryId, ECInstanceKey const&, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue) const;
        DbResult InsertPropertyValueChange(ECInstanceId summaryId, ECInstanceKey const&, Utf8CP accessString, ECN::ECClassId oldId, ECN::ECClassId newId) const;
        DbResult InsertPropertyValueChange(ECInstanceId summaryId, ECInstanceKey const&, Utf8CP accessString, ECInstanceId oldId, ECInstanceId newId) const;

        static ECSqlStatus BindDbValue(ECSqlStatement&, int, DbValue const&);
         static bool RawIndirectToBool(int indirect) { return indirect != 0; }
        
    public:
        explicit ChangeSummaryExtractor(ECDbCR ecdb) : m_ecdb(ecdb), m_stmtCache(15) {}

        BentleyStatus Extract(ECInstanceId& changeSummaryId, IChangeSet& changeSet, ECDb::ChangeSummaryExtractOptions const&) const;

        void ClearCache() { m_stmtCache.Empty(); }
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
