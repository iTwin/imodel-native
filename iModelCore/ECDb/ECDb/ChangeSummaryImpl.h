/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummaryImpl.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//_BENTLEY_INTERNAL_ONLY_
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

typedef RefCountedPtr<TableClassMap> TableClassMapPtr;

//=======================================================================================
//! Information on mappings to a table
// @bsiclass                                              Ramanujam.Raman      07/2015
//=======================================================================================
struct TableMap : RefCounted<NonCopyableClass>
    {
    friend struct ChangeSummary;
    private:
        ECDbCR m_ecdb;
        bool m_isMapped = false;
        DbTable const* m_dbTable = nullptr;
        Utf8String m_tableName;
        bmap<Utf8String, int> m_columnIndexByName;
        bvector<ECN::ECClassId> m_fkeyRelClassIds;
        ECN::ECClassId m_primaryClassId;
        mutable bmap<ECN::ECClassId, TableClassMapPtr> m_tableClassMapsById;

        ColumnMap m_emptyColumnMap;
        ColumnMap m_classIdColumnMap;
        ColumnMap m_instanceIdColumnMap;

        TableMap(ECDbCR ecdb, Utf8StringCR tableName);

        void Initialize(Utf8StringCR tableName);
        void InitColumnIndexByName();
        void InitSystemColumnMaps();
        void InitForeignKeyRelClassMaps();

        ECN::ECClassId QueryClassId() const;

    public:
        //! @private
        //! Create the table map for a table with the specified name
        static TableMapPtr Create(ECDbCR ecdb, Utf8StringCR tableName);

        //! Destructor
        ~TableMap() {}

        //! Returns true if the table is mapped to a ECClass. false otherwise. 
         bool IsMapped() const { return m_isMapped; }

         DbTable const* GetDbTable() const { return m_dbTable; }

        //! Gets the name of the table
         Utf8StringCR GetTableName() const { return m_tableName; }

        //! Returns true if the table contains a column storing ECClassId (if the table stores multiple classes)
         bool ContainsECClassIdColumn() const { return m_classIdColumnMap.IsValid(); }

        //! Gets the primary ECClassId column if the table stores multiple classes
        //! @see ContainsECClassIdColumn()
         ColumnMap const& GetECClassIdColumn() const { return m_classIdColumnMap; }

        //! Gets the primary ECClassId if the table stores a single class
        ECN::ECClassId GetECClassId() const;

        //! Gets the primary ECInstanceId column
        ColumnMap const& GetECInstanceIdColumn() const { return m_instanceIdColumnMap; }

        //! @private
        //! Queries the value stored in the table at the specified column, for the specified instanceId
        DbDupValue QueryValueFromDb(Utf8StringCR physicalColumnName, ECInstanceId instanceId) const;

        //! @private
        //! Queries the id value stored in the table at the specified column, for the specified instanceId
        template <class T_Id> T_Id QueryValueId(Utf8StringCR physicalColumnName, ECInstanceId instanceId) const
            {
            DbDupValue value = QueryValueFromDb(physicalColumnName, instanceId);
            if (!value.IsValid() || value.IsNull())
                return T_Id();
            return value.GetValueId<T_Id>();
            }

        int GetColumnIndexByName(Utf8StringCR columnName) const;

        bvector<ECN::ECClassId> const& GetMappedForeignKeyRelationshipClasses() const { return m_fkeyRelClassIds; }

        bool QueryInstance(ECInstanceId instanceId) const;

        TableClassMapCP GetTableClassMap(ECN::ECClassCR ecClass) const;
    };

//=======================================================================================
//! Information on mappings to a specific class within a table
// @bsiclass                                              Ramanujam.Raman      07/2015
//=======================================================================================
struct TableClassMap : RefCounted<NonCopyableClass>
    {
    private:
        ECDbCR m_ecdb;
        TableMapCR m_tableMap;
        ECN::ECClassCR m_class;
        ClassMapCP m_classMap = nullptr;

        ColumnMapByAccessString m_columnMapByAccessString;
 
        TableClassMap(ECDbCR ecdb, TableMapCR tableMap, ECN::ECClassCR ecClass);
        void Initialize();
        void InitPropertyColumnMaps();
        void AddColumnMapsForProperty(SingleColumnDataPropertyMap const&);
    public:
        //! @private
        //! Create the table map for the primary table of the specified class
        static TableClassMapPtr Create(ECDbCR ecdb, TableMapCR tableMap, ECN::ECClassCR ecClass);

        //! Returns true if the class is really mapped
        bool IsMapped() const { return m_classMap != nullptr; }

        //! Destructor
        ~TableClassMap() {}

        //! Returns true if the table contains a column for the specified property (access string)
        bool ContainsColumn(Utf8CP propertyAccessString) const;

        //! Gets column map for the specified property (access string)
        ColumnMap const& GetColumn(Utf8CP propertyAccessString) const;

        ECN::ECClassCR GetClass() const { return m_class; }

        TableMapCR GetTableMap() const { return m_tableMap; }

        ClassMapCP GetClassMap() const { return m_classMap; }

        ColumnMapByAccessString const& GetColumnMapByAccessString() const { return m_columnMapByAccessString; }
    };

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      07/2015
//=======================================================================================
struct SqlChange : NonCopyableClass
    {
    private:
        Changes::Change const& m_sqlChange;
        Utf8String m_tableName;
        int m_indirect;
        int m_nCols;
        DbOpcode m_dbOpcode;
        bset<int> m_primaryKeyColumnIndices;

    public:
        SqlChange(Changes::Change const& change);

        Changes::Change const& GetChange() const { return m_sqlChange; }
        Utf8StringCR GetTableName() const { return m_tableName; }
        DbOpcode GetDbOpcode() const { return m_dbOpcode; }
        int GetIndirect() const { return m_indirect; }
        int GetNCols() const { return m_nCols; }
        bool IsPrimaryKeyColumn(int index) const { return m_primaryKeyColumnIndices.find(index) != m_primaryKeyColumnIndices.end(); }

        void GetValues(DbValue& oldValue, DbValue& newValue, int columnIndex) const; // TODO: Check if this can be deprecated
        void GetValueIds(ECInstanceId& oldInstanceId, ECInstanceId& newInstanceId, int idColumnIndex) const; // TODO: Check if this can be deprecated
        DbValue GetValue(int columnIndex) const; // TODO: Check if this can be deprecated

        template <class T_Id> T_Id GetValueId(int columnIndex) const
            {
            DbValue value = GetValue(columnIndex);
            if (!value.IsValid() || value.IsNull())
                return T_Id();
            return value.GetValueId<T_Id>();
            }
    };

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2015
//=======================================================================================
struct InstancesTable : NonCopyableClass
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
struct ValuesTable : NonCopyableClass
    {
    private:
        ChangeSummaryCR m_changeSummary;
        ECDbCR m_ecdb;
        InstancesTable const& m_instancesTable;
        Utf8String m_valuesTableNameNoPrefix;
        mutable Statement m_valuesTableInsert;

        void CreateTable();
        void ClearTable();

        void PrepareStatements();
        void FinalizeStatements();

    public:
        ValuesTable(InstancesTableCR instancesTable);
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
struct ChangeExtractor : NonCopyableClass
    {
    enum class ExtractOption
        {
        InstancesOnly = 1,
        RelationshipInstancesOnly = 2
        };

    typedef bmap<Utf8String, TableMapPtr> TableMapByName; // TODO: Remove tochange iterator

    private:
        ChangeSummaryCR m_changeSummary;
        ExtractOption m_extractOption;

        ECDbCR m_ecdb;
        mutable TableMapByName m_tableMapByName; // TODO: REmove to ChangeIterator
        InstancesTable& m_instancesTable;
        ValuesTable& m_valuesTable;

        SqlChange const* m_sqlChange;
        TableMap const* m_tableMap;

        static ClassMapCP GetClassMap(ECDbCR, ECN::ECClassId);

        ECN::ECClassId GetClassIdFromChangeOrTable(Utf8CP classIdColumnName, ECInstanceId instanceId) const;
        bool ChangeUpdatesClass(TableClassMapCR classMap) const;
        int GetFirstColumnIndex(PropertyMap const*) const;

        BentleyStatus ExtractFromSqlChanges(Changes& sqlChanges, ExtractOption extractOption);
        BentleyStatus ExtractFromSqlChange(SqlChange const& sqlChange, ExtractOption extractOption);

        void ExtractInstance(TableClassMapCR tableClassMap, ECInstanceId instanceId);

        void ExtractRelInstance(TableClassMapCR tableClassMap, ECInstanceId relInstanceId);
        void ExtractRelInstanceInEndTable(TableClassMapCR tableClassMap, ECInstanceId relInstanceId, ECN::ECClassId foreignEndClassId);
        void ExtractRelInstanceInLinkTable(TableClassMapCR tableClassMap, ECInstanceId relInstanceId);
        void GetRelEndInstanceKeys(ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd) const;
        ECN::ECClassId GetRelEndClassId(RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd, ECInstanceId endInstanceId) const;
        static ECN::ECClassId GetRelEndClassIdFromRelClass(ECN::ECRelationshipClassCP relClass, ECN::ECRelationshipEnd relEnd);

        bool ClassIdMatchesConstraint(ECN::ECClassId relClassId, ECN::ECRelationshipEnd end, ECN::ECClassId candidateClassId) const;

        bool RecordInstance(TableClassMapCR tableClassMap, ECInstanceId instanceId, DbOpcode dbOpcode);
        bool RecordColumValue(ChangeSummary::InstanceCR instance, ColumnMap const& columnMap, Utf8StringCR accessString);

        bool RecordRelInstance(TableClassMapCR classMap, ECInstanceId instanceId, DbOpcode dbOpcode, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey);

        TableMap const* GetTableMap(Utf8StringCR tableName) const; // TODO: Remove
        void AddTableToMap(Utf8StringCR tableName) const; // TODO: Remove
        void FreeTableMap(); // TODO: Remove
        DbDupValue ExtractOverflowValue(DbValue const& columnValue, ColumnMap const& columnMap); // TODO: Remove

    public:
        ChangeExtractor(ChangeSummaryCR changeSummary, InstancesTableR instancesTable, ValuesTableR valuesTable);
        ~ChangeExtractor();

        BentleyStatus ExtractFromSqlChanges(Changes& changes, bool includeRelationshipInstances);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE