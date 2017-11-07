/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummaryImplV2.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//_BENTLEY_INTERNAL_ONLY_
#include "ECDbInternalTypes.h"
#include <ECDb/ChangeSummaryV2.h>
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2017
//=======================================================================================
struct ChangesetTable final : NonCopyableClass
    {
    private:
        ChangeSummaryV2CR m_changeSummary;
        ECDbCR m_ecdb;

    public:
        mutable ECSqlStatement m_instancesTableDelete;
    };
//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2015
//=======================================================================================
struct InstancesTableV2 final : NonCopyableClass
    {
    private:
        ChangeSummaryV2CR m_changeSummary;
        ECDbCR m_ecdb;
        mutable ECSqlStatement m_instancesTableDelete;
        mutable ECSqlStatement m_instancesTableInsert;
        mutable ECSqlStatement m_instancesTableUpdate;
        mutable ECSqlStatement m_instancesTableSelect;
        mutable ECSqlStatement m_findInstance;

        void PrepareStatements();

        void Insert(ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect, Utf8StringCR tableName);
        void Update(ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect);

    public:
        InstancesTableV2(ChangeSummaryV2CR changeSummary);
        ~InstancesTableV2() {}

        void Initialize();

        ECDbCR GetDb() const { return m_ecdb; }
        ChangeSummaryV2CR GetChangeSummary() const { return m_changeSummary; }
        ECInstanceId GetChangesetId() const { return m_changeSummary.GetId(); }
        void InsertOrUpdate(ChangeSummaryV2::InstanceCR changeInstance);
        void Delete(ECN::ECClassId changedClassId, ECInstanceId changedInstanceId);
        ChangeSummaryV2::Instance QueryChangedInstance(ECN::ECClassId changedClassId, ECInstanceId changedInstanceId) const;
        ECInstanceId FindInstanceId(ECN::ECClassId changedClassId, ECInstanceId changedInstanceId) const;
        ECN::ECClassId QueryChangedClassId(Utf8StringCR tableName, ECInstanceId changedInstanceId) const;
        bool ContainsInstance(ECN::ECClassId changedClassId, ECInstanceId changedInstanceId) const;
    };

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2015
//=======================================================================================
struct ValuesTableV2 final : NonCopyableClass
    {
    private:
        ChangeSummaryV2CR m_changeSummary;
        ECDbCR m_ecdb;
        InstancesTableV2 const& m_instancesTable;
        Utf8String m_valuesTableNameNoPrefix;
        mutable ECSqlStatement m_valuesTableInsert;

        void PrepareStatements();
        static ECSqlStatus BindDbValue(ECSqlStatement&, int, DbValue const&);
    public:
        explicit ValuesTableV2(InstancesTableV2 const&);
        ~ValuesTableV2() {}

        void Initialize();

        void Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue);
        void Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, ECN::ECClassId oldId, ECN::ECClassId newId);
        void Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, ECInstanceId oldId, ECInstanceId newId);
    };

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2015
//=======================================================================================
struct ChangeExtractorV2 final : NonCopyableClass
    {
    enum class ExtractOption
        {
        InstancesOnly = 1,
        RelationshipInstancesOnly = 2
        };

    typedef bmap<Utf8String, TableMapPtr> TableMapByName; // TODO: Remove tochange iterator

    private:
        ChangeSummaryV2CR m_changeSummary;

        ECDbCR m_ecdb;
        mutable TableMapByName m_tableMapByName; // TODO: REmove to ChangeIterator
        InstancesTableV2& m_instancesTable;
        ValuesTableV2& m_valuesTable;

        BentleyStatus FromChangeSet(IChangeSet& changeSet, ExtractOption extractOption);

        void GetRelEndInstanceKeys(ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, ChangeIterator::RowEntry const& rowEntry, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd) const;
        ECN::ECClassId GetRelEndClassId(ChangeIterator::RowEntry const& rowEntry, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd, ECInstanceId endInstanceId) const;
        ECN::ECClassId GetClassIdFromColumn(TableMap const& tableMap, DbColumn const& classIdColumn, ECInstanceId instanceId) const;
        static ECN::ECClassId GetRelEndClassIdFromRelClass(ECN::ECRelationshipClassCP relClass, ECN::ECRelationshipEnd relEnd);
        int GetFirstColumnIndex(PropertyMap const* propertyMap, ChangeIterator::RowEntry const& rowEntry) const;

        void ExtractInstance(ChangeIterator::RowEntry const& rowEntry);
        void RecordInstance(ChangeSummaryV2::InstanceCR instance, ChangeIterator::RowEntry const& rowEntry, bool recordOnlyIfUpdatedProperties);
        bool RecordValue(ChangeSummaryV2::InstanceCR instance, ChangeIterator::ColumnEntry const& columnEntry);

        void ExtractRelInstances(ChangeIterator::RowEntry const& rowEntry);
        void ExtractRelInstanceInLinkTable(ChangeIterator::RowEntry const& rowEntry, RelationshipClassLinkTableMap const& relClassMap);
        void ExtractRelInstanceInEndTable(ChangeIterator::RowEntry const& rowEntry, TableClassMap::EndTableRelationshipMap const& endTableRelMap);
        bool ClassIdMatchesConstraint(ECN::ECClassId relClassId, ECN::ECRelationshipEnd end, ECN::ECClassId candidateClassId) const;
        bool RecordRelInstance(ChangeSummaryV2::InstanceCR instance, ChangeIterator::RowEntry const& rowEntry, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey);

    public:
        ChangeExtractorV2(ChangeSummaryV2CR changeSummary, InstancesTableV2& instancesTable, ValuesTableV2& valuesTable);
        BentleyStatus FromChangeSet(IChangeSet& changeSet, bool includeRelationshipInstances);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE