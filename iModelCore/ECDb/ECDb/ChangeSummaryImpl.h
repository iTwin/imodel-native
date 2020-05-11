/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//_BENTLEY_INTERNAL_ONLY_
#include <ECDb/ECDb.h>
#include <ECDb/ChangeSummary.h>
#include <ECDb/ChangeIterator.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2015
//=======================================================================================
struct InstancesTable final
    {
    private:
        ChangeSummaryCR m_changeSummary;
        ECDbCR m_ecdb;
        Utf8String m_instancesTableNameNoPrefix;
        mutable Statement m_instancesTableDelete;
        mutable Statement m_instancesTableInsert;
        mutable Statement m_instancesTableUpdate;
        mutable Statement m_instancesTableSelect;

        const int m_nameSuffix;

        //not copyable
        InstancesTable(InstancesTable const&) = delete;
        InstancesTable& operator=(InstancesTable const&) = delete;

        void CreateTable();
        void PrepareStatements();
        void FinalizeStatements();
        void ClearTable();

        void Insert(ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect, Utf8StringCR tableName);
        void Update(ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect);

    public:
        InstancesTable(ChangeSummaryCR changeSummary, int nameSuffix);
        ~InstancesTable() { Free(); }

        void Initialize();
        void Free();

        ECDbCR GetDb() const { return m_ecdb; }
        ChangeSummaryCR GetChangeSummary() const { return m_changeSummary; }

        Utf8String GetName() const;
        Utf8StringCR GetNameNoPrefix() const { return m_instancesTableNameNoPrefix; }
        int GetNameSuffix() const { return m_nameSuffix; }

        void InsertOrUpdate(ChangeSummary::InstanceCR changeInstance);
        void Delete(ECN::ECClassId classId, ECInstanceId instanceId);
        ChangeSummary::Instance QueryInstance(ECN::ECClassId classId, ECInstanceId instanceId) const;
        ECN::ECClassId QueryClassId(Utf8StringCR tableName, ECInstanceId instanceId) const;
        bool ContainsInstance(ECN::ECClassId classId, ECInstanceId instanceId) const;
    };

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2015
//=======================================================================================
struct ValuesTable final
    {
    private:
        ChangeSummaryCR m_changeSummary;
        ECDbCR m_ecdb;
        InstancesTable const& m_instancesTable;
        Utf8String m_valuesTableNameNoPrefix;
        mutable Statement m_valuesTableInsert;

        //not copyable
        ValuesTable(ValuesTable const&) = delete;
        ValuesTable& operator=(ValuesTable const&) = delete;

        void CreateTable();
        void ClearTable();

        void PrepareStatements();
        void FinalizeStatements();

    public:
        explicit ValuesTable(InstancesTable const&);
        ~ValuesTable() { Free(); }

        void Initialize();
        void Free();

        Utf8String GetName() const;
        Utf8StringCR GetNameNoPrefix() const { return m_valuesTableNameNoPrefix; }

        void Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue);
        void Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, ECN::ECClassId oldId, ECN::ECClassId newId);
        void Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, ECInstanceId oldId, ECInstanceId newId);
    };

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2015
//=======================================================================================
struct ChangeExtractor final
    {
    enum class ExtractOption
        {
        InstancesOnly = 1,
        RelationshipInstancesOnly = 2
        };

    private:
        ChangeSummaryCR m_changeSummary;

        ECDbCR m_ecdb;
        InstancesTable& m_instancesTable;
        ValuesTable& m_valuesTable;

        //not copyable
        ChangeExtractor(ChangeExtractor const&) = delete;
        ChangeExtractor& operator=(ChangeExtractor const&) = delete;

        BentleyStatus FromChangeSet(ChangeStream& changeSet, ExtractOption extractOption);

        void GetRelEndInstanceKeys(ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, ChangeIterator::RowEntry const& rowEntry, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd) const;
        ECN::ECClassId GetRelEndClassId(ChangeIterator::RowEntry const& rowEntry, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd, ECInstanceId endInstanceId) const;
        ECN::ECClassId GetClassIdFromColumn(ChangeIterator::TableMap const& tableMap, DbColumn const& classIdColumn, ECInstanceId instanceId) const;
        static ECN::ECClassId GetRelEndClassIdFromRelClass(ECN::ECRelationshipClassCP relClass, ECN::ECRelationshipEnd relEnd);
        int GetFirstColumnIndex(PropertyMap const* propertyMap, ChangeIterator::RowEntry const& rowEntry) const;

        void ExtractInstance(ChangeIterator::RowEntry const& rowEntry);
        void RecordInstance(ChangeSummary::InstanceCR instance, ChangeIterator::RowEntry const& rowEntry, bool recordOnlyIfUpdatedProperties);
        bool RecordValue(ChangeSummary::InstanceCR instance, ChangeIterator::ColumnEntry const& columnEntry);

        void ExtractRelInstances(ChangeIterator::RowEntry const& rowEntry);
        void ExtractRelInstanceInLinkTable(ChangeIterator::RowEntry const& rowEntry, RelationshipClassLinkTableMap const& relClassMap);
        void ExtractRelInstanceInEndTable(ChangeIterator::RowEntry const& rowEntry, ChangeIterator::TableClassMap::EndTableRelationshipMap const& endTableRelMap);
        bool ClassIdMatchesConstraint(ECN::ECClassId relClassId, ECN::ECRelationshipEnd end, ECN::ECClassId candidateClassId) const;
        bool RecordRelInstance(ChangeSummary::InstanceCR instance, ChangeIterator::RowEntry const& rowEntry, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey);

    public:
        ChangeExtractor(ChangeSummaryCR changeSummary, InstancesTable& instancesTable, ValuesTable& valuesTable);
        BentleyStatus FromChangeSet(ChangeStream& changeSet, bool includeRelationshipInstances);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE