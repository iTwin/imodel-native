/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <BeSQLite/ChangeSet.h>
#include <ECDb/ChangeIterator.h>
#include "RelationshipClassMap.h"
#include "DbSchema.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Information on mappings to a column
// @bsiclass
//=======================================================================================
struct ChangeIterator::ColumnMap final
    {
    private:
        Utf8String m_columnName;
        int m_columnIndex = -1;
    public:
        ColumnMap() {}
        ColumnMap(Utf8StringCR columnName, int columnIndex) : m_columnName(columnName), m_columnIndex(columnIndex) {}

        //! Gets the name of the column
        Utf8StringCR GetName() const { return m_columnName; }

        //! Gets the index of the column in the table
        int GetIndex() const { return m_columnIndex; }

        //! Returns true if the column has been initialized
        bool IsValid() const { return m_columnIndex >= 0; }
    };

//=======================================================================================
//! Information on mappings to a table
// @bsiclass
//=======================================================================================
struct ChangeIterator::TableMap final
    {
    private:
        ECDbCR m_ecdb;
        bool m_isMapped = false;
        DbTable const* m_dbTable = nullptr;
        Utf8String m_tableName;
        bmap<Utf8String, int> m_columnIndexByName;
        ECN::ECClassId m_primaryClassId;
        mutable std::map<ECN::ECClassId, std::unique_ptr<ChangeIterator::TableClassMap>> m_tableClassMaps;

        ColumnMap m_classIdColumnMap;
        ColumnMap m_instanceIdColumnMap;


        void Initialize(Utf8StringCR tableName);
        void InitColumnIndexByName();
        void InitSystemColumnMaps();

        ECN::ECClassId QueryClassId() const;

    public:
        TableMap(ECDbCR ecdb, Utf8StringCR tableName) : m_ecdb(ecdb) { Initialize(tableName); }
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
        ECN::ECClassId GetClassId() const;

        //! Gets the primary Id column
        ColumnMap const& GetIdColumn() const { return m_instanceIdColumnMap; }

        ChangeIterator::TableClassMap const* GetTableClassMap(ECN::ECClassCR ecClass) const;

        int GetColumnIndexByName(Utf8StringCR columnName) const;
    };

//=======================================================================================
//! Information on mappings to a table
// @bsiclass
//=======================================================================================
struct ChangeIterator::TableMapCollection final
    {
private:
    std::map<Utf8String, std::unique_ptr<ChangeIterator::TableMap>> m_maps;

public:
    TableMapCollection() {}

    ChangeIterator::TableMap const* Get(Utf8StringCR tableName) const
        {
        auto it = m_maps.find(tableName);
        if (it != m_maps.end())
            return it->second.get();

        return nullptr;
        }

    void Add(std::unique_ptr<ChangeIterator::TableMap> map) { m_maps[map->GetTableName()] = std::move(map); }
    };

//=======================================================================================
//! Information on mappings to a specific class within a table
// @bsiclass
//=======================================================================================
struct ChangeIterator::TableClassMap final
    {
    struct EndTableRelationshipMap final
        {
        ECN::ECClassId m_relationshipClassId;
        ColumnMap m_relationshipClassIdColumnMap;
        ColumnMap m_relatedInstanceIdColumnMap;
        mutable bmap<ECN::ECRelationshipClassCP, DbColumn const*> m_foreignEndClassIdColumnMap;

        DbColumn const* GetForeignEndClassIdColumn(ECDbCR ecdb, ECN::ECRelationshipClassCR relationshipClass) const;
        };

    private:
        ECDbCR m_ecdb;
        TableMap const& m_tableMap;
        ECN::ECClassCR m_class;
        ClassMap const* m_classMap = nullptr;

        bmap<Utf8String, ColumnMap*> m_columnMapByAccessString;
        bvector<EndTableRelationshipMap*> m_endTableRelMaps;

        void Initialize();
        void InitPropertyColumnMaps();
        void FreeColumnMaps();
        void InitEndTableRelationshipMaps();
        void FreeEndTableRelationshipMaps();

    public:
        TableClassMap(ECDbCR ecdb, TableMap const& tableMap, ECN::ECClassCR ecClass) : m_ecdb(ecdb), m_tableMap(tableMap), m_class(ecClass) { Initialize(); }
        ~TableClassMap();

        //! Returns true if the class is really mapped
        bool IsMapped() const { return m_classMap != nullptr; }

        //! Returns true if the table contains a column for the specified property (access string)
        bool ContainsColumn(Utf8CP propertyAccessString) const;

        ECN::ECClassCR GetClass() const { return m_class; }

        TableMap const& GetTableMap() const { return m_tableMap; }

        ClassMap const* GetClassMap() const { return m_classMap; }

        bvector<EndTableRelationshipMap*> const& GetEndTableRelationshipMaps() const { return m_endTableRelMaps; }

        bmap<Utf8String, ColumnMap*> const& GetColumnMapByAccessString() const { return m_columnMapByAccessString; }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChangeIterator::SqlChange
    {
    private:
        Changes::Change const& m_sqlChange;
        Utf8String m_tableName;
        int m_indirect;
        int m_nCols;
        DbOpcode m_dbOpcode;
        bset<int> m_primaryKeyColumnIndices;

        SqlChange(SqlChange const&) = delete;
        SqlChange& operator=(SqlChange const&) = delete;

        void GetValues(DbValue& oldValue, DbValue& newValue, int columnIndex) const;
        DbValue GetValue(int columnIndex) const;

    public:
        explicit SqlChange(Changes::Change const& change);

        Changes::Change const& GetChange() const { return m_sqlChange; }
        Utf8StringCR GetTableName() const { return m_tableName; }
        DbOpcode GetDbOpcode() const { return m_dbOpcode; }
        int GetIndirect() const { return m_indirect; }
        int GetNCols() const { return m_nCols; }
        bool IsPrimaryKeyColumn(int index) const { return m_primaryKeyColumnIndices.find(index) != m_primaryKeyColumnIndices.end(); }

        template <class T_Id> T_Id GetValueId(int columnIndex) const
            {
            DbValue value = GetValue(columnIndex);
            if (!value.IsValid() || value.IsNull())
                return T_Id();
            return value.GetValueId<T_Id>();
            }

        template <class T_Id> void GetValueIds(T_Id& oldId, T_Id& newId, int idColumnIndex) const
            {
            oldId = newId = T_Id();

            DbValue oldValue(nullptr), newValue(nullptr);
            GetValues(oldValue, newValue, idColumnIndex);

            if (oldValue.IsValid() && !oldValue.IsNull())
                oldId = oldValue.GetValueId<T_Id>();

            if (newValue.IsValid() && !newValue.IsNull())
                newId = newValue.GetValueId<T_Id>();
            }

    };
END_BENTLEY_SQLITE_EC_NAMESPACE

