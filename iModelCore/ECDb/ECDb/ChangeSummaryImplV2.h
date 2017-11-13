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
struct ChangedInstance
    {
    private:
        ChangeSummaryV2 const* m_changeSummary = nullptr;
        ECN::ECClassId m_classId;
        ECInstanceId m_instanceId;
        DbOpcode m_dbOpcode;
        int m_indirect;
        Utf8String m_tableName;
        mutable ECSqlStatement m_valuesTableSelect;

        void SetupValuesTableSelectStatement(Utf8CP accessString) const;

    public:
        ChangedInstance() {}

        ChangedInstance(ChangeSummaryV2 const& changeSummary, ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect, Utf8StringCR tableName) :
            m_changeSummary(&changeSummary), m_classId(classId), m_instanceId(instanceId), m_dbOpcode(dbOpcode), m_indirect(indirect), m_tableName(tableName)
            {}

        ChangedInstance(ChangedInstance const& other) { *this = other; }

        ChangedInstance& operator=(ChangedInstance const& other);

        //! Get the class id of the changed instance
        ECN::ECClassId GetClassId() const { return m_classId; }

        //! Get the instance id of the changed instance
        ECInstanceId GetInstanceId() const { return m_instanceId; }

        //! Get the DbOpcode of the changed instance that indicates that the instance was inserted, updated or deleted.
        DbOpcode GetDbOpcode() const { return m_dbOpcode; }

        //! Get the flag indicating if the change was "indirectly" caused by a database trigger or other means. 
        int GetIndirect() const { return m_indirect; }

        //! Get the name of the primary table containing the instance
        Utf8StringCR GetTableName() const { return m_tableName; }

        //! Returns true if the instance is valid. 
        bool IsValid() const { return m_instanceId.IsValid(); }

        //! Returns true if the value specified by the accessString exists
        bool ContainsValue(Utf8CP accessString) const;

        //! Get a specific changed value
        DbDupValue GetOldValue(Utf8CP accessString) const;

        //! Get a specific changed value
        DbDupValue GetNewValue(Utf8CP accessString) const;

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

    private:
        ChangeSummaryV2 const& m_changeSummary;

        InstanceChangeManager& m_instanceChangeManager;
        PropertyValueChangeManager& m_propertyValueChangeManager;

        BentleyStatus FromChangeSet(IChangeSet& changeSet, ExtractOption extractOption);

        void GetRelEndInstanceKeys(ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, ChangeIterator::RowEntry const& rowEntry, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd) const;
        ECN::ECClassId GetRelEndClassId(ChangeIterator::RowEntry const& rowEntry, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd, ECInstanceId endInstanceId) const;
        ECN::ECClassId GetClassIdFromColumn(TableMap const& tableMap, DbColumn const& classIdColumn, ECInstanceId instanceId) const;
        static ECN::ECClassId GetRelEndClassIdFromRelClass(ECN::ECRelationshipClassCP relClass, ECN::ECRelationshipEnd relEnd);
        int GetFirstColumnIndex(PropertyMap const* propertyMap, ChangeIterator::RowEntry const& rowEntry) const;

        BentleyStatus ExtractInstance(ChangeIterator::RowEntry const& rowEntry);
        void RecordInstance(ChangedInstanceCR instance, ChangeIterator::RowEntry const& rowEntry, bool recordOnlyIfUpdatedProperties);
        bool RecordValue(ChangedInstanceCR instance, ChangeIterator::ColumnEntry const& columnEntry);
        BentleyStatus RecordRelInstance(ChangeSummaryV2::Instance const& instance, ChangeIterator::RowEntry const& rowEntry, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey);

        BentleyStatus ExtractRelInstances(ChangeIterator::RowEntry const& rowEntry);
        BentleyStatus ExtractRelInstanceInLinkTable(ChangeIterator::RowEntry const& rowEntry, RelationshipClassLinkTableMap const& relClassMap);
        BentleyStatus ExtractRelInstanceInEndTable(ChangeIterator::RowEntry const& rowEntry, TableClassMap::EndTableRelationshipMap const& endTableRelMap);
        bool ClassIdMatchesConstraint(ECN::ECClassId relClassId, ECN::ECRelationshipEnd end, ECN::ECClassId candidateClassId) const;
        bool RecordRelInstance(ChangeSummaryV2::InstanceCR instance, ChangeIterator::RowEntry const& rowEntry, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey);
        static bool RawIndirectToBool(int indirect) { return indirect != 0; }

    public:
        ChangeExtractorV2(ChangeSummaryV2 const& changeSummary, InstanceChangeManager& instanceChangeManager, PropertyValueChangeManager& propertyValueChangeManager)
            : m_changeSummary(changeSummary), m_instanceChangeManager(instanceChangeManager), m_propertyValueChangeManager(propertyValueChangeManager)
            {}

        BentleyStatus FromChangeSet(IChangeSet& changeSet, bool includeRelationshipInstances);
    };

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2015
//=======================================================================================
struct InstanceChangeManager final : NonCopyableClass
    {
    private:
        ChangeSummaryV2 const& m_changeSummary;
        ECSqlStatementCache m_stmtCache;

    public:
        explicit InstanceChangeManager(ChangeSummaryV2 const& changeSummary) : m_changeSummary(changeSummary), m_stmtCache(6) {}
        ~InstanceChangeManager() {}

        ChangeSummaryV2 const& GetChangeSummary() const { return m_changeSummary; }
        DbResult InsertOrUpdate(ChangeSummaryV2::Instance const&);
        DbResult Delete(ECInstanceKey const&);
        ChangeSummaryV2::Instance QueryChangedInstance(ECInstanceKey const&) const;
        ECInstanceId FindChangeId(ECInstanceKey const&) const;
        ECN::ECClassId QueryClassIdOfChangedInstance(Utf8StringCR tableName, ECInstanceId idOfChangedInstance) const;
        bool ContainsChange(ECInstanceKey const& keyOfChangedInstance) const { return FindChangeId(keyOfChangedInstance).IsValid(); }
    };

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2015
//=======================================================================================
struct PropertyValueChangeManager final : NonCopyableClass
    {
    private:
        ChangeSummaryV2 const& m_changeSummary;
        InstanceChangeManager const& m_instancesTable;
        Utf8String m_valuesTableNameNoPrefix;
        mutable ECSqlStatement m_valuesTableInsert;

        ECSqlStatus PrepareStatements();
        static ECSqlStatus BindDbValue(ECSqlStatement&, int, DbValue const&);
    public:
        explicit PropertyValueChangeManager(InstanceChangeManager const& tab) : m_instancesTable(tab), m_changeSummary(tab.GetChangeSummary()) {}
        ~PropertyValueChangeManager() {}

        BentleyStatus Initialize() { return PrepareStatements() == ECSqlStatus::Success ? SUCCESS : ERROR; }

        DbResult Insert(ECInstanceKey const&, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue);
        DbResult Insert(ECInstanceKey const&, Utf8CP accessString, ECN::ECClassId oldId, ECN::ECClassId newId);
        DbResult Insert(ECInstanceKey const&, Utf8CP accessString, ECInstanceId oldId, ECInstanceId newId);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE