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
        Utf8String m_primaryTableName;

    public:
        InstanceChange() {}

        InstanceChange(ECInstanceId summaryId, ECInstanceKey const& keyOfChangedInstance, DbOpcode dbOpcode, bool isIndirect, Utf8StringCR tableName) :
            m_summaryId(summaryId), m_keyOfChangedInstance(keyOfChangedInstance), m_dbOpcode(dbOpcode), m_isIndirect(isIndirect), m_primaryTableName(tableName)
            {}

        bool IsValid() const { return m_keyOfChangedInstance.GetInstanceId().IsValid(); }

        ECInstanceId GetSummaryId() const { return m_summaryId; }
        ECInstanceKey GetKeyOfChangedInstance() const { return m_keyOfChangedInstance; }

        //! Get the DbOpcode of the changed instance that indicates that the instance was inserted, updated or deleted.
        DbOpcode GetDbOpcode() const { return m_dbOpcode; }
        bool IsIndirect() const { return m_isIndirect; }

        //! Get the name of the primary table containing the instance
        Utf8StringCR GetPrimaryTableName() const { return m_primaryTableName; }
    };

//=======================================================================================
// @bsiclass                                            Krischan.Eberle      11/2017
//=======================================================================================
struct ChangeSummaryExtractor final : NonCopyableClass
    {
    private:
        enum class ExtractMode { InstancesOnly, RelationshipInstancesOnly };

        //! Matches the ECEnumeration Operation defined in the ECDbChangeSummaries ECSchema
        enum class Operation
            {
            Insert = 1,
            Update = 2,
            Delete = 3
            };

        ECDbCR m_ecdb;
        ECSqlStatementCache m_stmtCache;

        static std::map<int, DbOpcode> s_toOpCodeMap;
        static std::map<DbOpcode, int> s_fromOpCodeMap;

        BentleyStatus Extract(ECInstanceId summaryId, IChangeSet& changeSet, ExtractMode) const;
        BentleyStatus ExtractInstance(ECInstanceId summaryId, ChangeIterator::RowEntry const&) const;
        BentleyStatus ExtractRelInstance(ECInstanceId summaryId, ChangeIterator::RowEntry const& rowEntry) const;
        BentleyStatus ExtractRelInstanceInLinkTable(ECInstanceId summaryId, ChangeIterator::RowEntry const&, RelationshipClassLinkTableMap const&) const;
        BentleyStatus ExtractRelInstanceInEndTable(ECInstanceId summaryId, ChangeIterator::RowEntry const&, ChangeIterator::TableClassMap::EndTableRelationshipMap const&) const;

        BentleyStatus RecordInstance(InstanceChange const&, ChangeIterator::RowEntry const& rowEntry, bool recordOnlyIfUpdatedProperties) const;
        BentleyStatus RecordRelInstance(InstanceChange const& instance, ChangeIterator::RowEntry const& rowEntry, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey) const;
        BentleyStatus RecordValue(bool& neededToRecord, InstanceChange const&, ChangeIterator::ColumnEntry const& columnEntry) const;

        void GetRelEndInstanceKeys(ECInstanceId summaryId, ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, ChangeIterator::RowEntry const& rowEntry, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd) const;
        ECN::ECClassId GetRelEndClassId(ECInstanceId summaryId, ChangeIterator::RowEntry const& rowEntry, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd, ECInstanceId endInstanceId) const;
        ECN::ECClassId GetClassIdFromColumn(ECInstanceId summaryId, ChangeIterator::TableMap const& tableMap, DbColumn const& classIdColumn, ECInstanceId instanceId) const;
        static ECN::ECClassId GetRelEndClassIdFromRelClass(ECN::ECRelationshipClassCP relClass, ECN::ECRelationshipEnd relEnd);
        int GetFirstColumnIndex(PropertyMap const* propertyMap, ChangeIterator::RowEntry const& rowEntry) const;
        

        InstanceChange QueryInstanceChange(ECInstanceId summaryId, ECInstanceKey const&) const;
        ECInstanceId FindChangeId(ECInstanceId summaryId, ECInstanceKey const&) const;
        ECN::ECClassId QueryClassIdOfChangedInstance(ECInstanceId summaryId, Utf8StringCR tableName, ECInstanceId idOfChangedInstance) const;
        bool ContainsChange(ECInstanceId summaryId, ECInstanceKey const& keyOfChangedInstance) const { return FindChangeId(summaryId, keyOfChangedInstance).IsValid(); }


        BentleyStatus InsertSummary(ECInstanceId& summaryId) const;
        DbResult InsertOrUpdate(InstanceChange const&) const;
        DbResult Delete(ECInstanceId summaryId, ECInstanceKey const&) const;

        DbResult InsertPropertyValueChange(ECInstanceId summaryId, ECInstanceKey const&, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue) const;
        DbResult InsertPropertyValueChange(ECInstanceId summaryId, ECInstanceKey const&, Utf8CP accessString, ECN::ECClassId oldId, ECN::ECClassId newId) const;
        DbResult InsertPropertyValueChange(ECInstanceId summaryId, ECInstanceKey const&, Utf8CP accessString, ECInstanceId oldId, ECInstanceId newId) const;

        static ECSqlStatus BindDbValue(ECSqlStatement&, int, DbValue const&);

        static bool RawIndirectToBool(int indirect) { return indirect != 0; }

        static Nullable<Operation> DbOpCodeToOperation(DbOpcode opCode)
            {
            switch (opCode)
                {
                    case DbOpcode::Delete:
                        return Operation::Delete;
                    case DbOpcode::Insert:
                        return Operation::Insert;
                    case DbOpcode::Update:
                        return Operation::Update;
                    default:
                        BeAssert(false && "DbCode enum was changed. This code has to be adjusted.");
                        return Nullable<Operation>();
                }
            }

        static Nullable<DbOpcode> OperationToDbOpCode(Operation op)
            {
            switch (op)
                {
                    case Operation::Delete:
                        return DbOpcode::Delete;
                    case Operation::Insert:
                        return DbOpcode::Insert;
                    case Operation::Update:
                        return DbOpcode::Update;
                    default:
                        BeAssert(false && "Operation enum was changed. This code has to be adjusted.");
                        return Nullable<DbOpcode>();
                }
            }

    public:
        explicit ChangeSummaryExtractor(ECDbCR ecdb) : m_ecdb(ecdb), m_stmtCache(15) {}

        BentleyStatus Extract(ECInstanceId& changeSummaryId, IChangeSet& changeSet, ECDb::ChangeSummaryExtractOptions const&) const;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
