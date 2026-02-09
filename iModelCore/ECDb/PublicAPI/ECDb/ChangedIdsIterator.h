/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDb.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ChangeSet.h>



BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct DbTable;
struct ClassMap;

//=======================================================================================
//! Utility to iterate over change records and interpret it as ECInstance Ids.
//! this is a pared down version of @see @ref ChangeIterator
//! @ingroup ECDbGroup
// @bsiclass
//=======================================================================================
struct ChangedIdsIterator
    {
    struct TableClassMap;
    struct SqlChange;

    //=======================================================================================
    //! Information on mappings to a column
    // @bsiclass
    //=======================================================================================
    struct ColumnMap
        {
        Utf8String columnName;
        int columnIndex = -1;
        bool HasBeenInitialized() const { return columnIndex >= 0; }
        bool IsValid() const { return HasBeenInitialized(); }
        };
    
    //=======================================================================================
    //! Information on mappings to a table
    // @bsiclass
    //=======================================================================================
    struct TableMap
        {
            bool isMapped = false;
            DbTable const* m_dbTable = nullptr;
            Utf8String tableName;
            bmap<Utf8String, int> m_columnIndexByName;

            ColumnMap classIdColumnMap;
            ColumnMap instanceIdColumnMap;

        protected:
            void Initialize(ECDbCR ecdb, Utf8StringCR tableName);
            void InitSystemColumnMaps();

        public:
            TableMap(ECDbCR ecdb, Utf8StringCR tableName) { Initialize(ecdb, tableName); }
            ~TableMap() = default;

            //! Returns true if the table contains a column storing ECClassId (if the table stores multiple classes)
            bool ContainsECClassIdColumn() const { return classIdColumnMap.IsValid(); }

            int GetColumnIndexByName(Utf8StringCR columnName) const;
        };

    //! An entry in the ChangedIdsIterator.
    struct iterator
        {
        friend ChangedIdsIterator;

        protected:
            ChangedIdsIterator const& m_iterator;
            Changes::Change m_change;

            SqlChange const* m_sqlChange = nullptr;
            TableMap const* m_tableMap = nullptr;
            ECInstanceId m_primaryInstanceId;

            iterator(ChangedIdsIterator const& iterator, Changes::Change const& change);

            void ReInitialize();
            void Initialize();
            virtual void InitPrimaryInstance();
            void FreeSqlChange();
            void Reset();

        public:
            ECDB_EXPORT virtual ~iterator();
            ECDB_EXPORT iterator(iterator const& other);
            ECDB_EXPORT iterator& operator=(iterator const& other);

            bool IsValid() const { return m_tableMap != nullptr; }

            bool IsMapped() const { return m_tableMap->isMapped; }

            //! Get the table name of the current change
            ECDB_EXPORT Utf8StringCR GetTableName() const;

            //! Return true if the current change is in the primary table (and not a joined or overflow table)
            ECDB_EXPORT bool IsPrimaryTable() const;

            //! Get the DbOpcode of the current change
            ECDB_EXPORT DbOpcode GetDbOpcode() const;

            //! Get the flag indicating if the current change was "indirectly" caused by a database trigger or other means.
            ECDB_EXPORT int GetIndirect() const;

            //! Get the (primary) instance id of the current change
            ECInstanceId GetPrimaryInstanceId() const { return m_primaryInstanceId; }

            ECDB_EXPORT iterator& operator++();

            iterator const& operator*() const { return *this; }
            bool operator==(iterator const& rhs) const { return m_change == rhs.m_change; }
            bool operator!=(iterator const& rhs) const { return !(*this == rhs); }

            SqlChange const* GetSqlChange() const { return m_sqlChange; } //!< @private
            Changes::Change const& GetChange() const { return m_change; } //!< @private
            TableMap const* GetTableMap() const { return m_tableMap; } //!< @private
            ChangedIdsIterator const& GetChangedIdsIterator() const { return m_iterator; }  //!< @private
        };
    
    using const_iterator = iterator;

    protected:
        ECDbCR m_ecdb;
        Changes m_changes;
        mutable std::map<Utf8String, TableMap> m_tableMaps;

        //not copyable
        ChangedIdsIterator(ChangedIdsIterator const&) = delete;
        ChangedIdsIterator& operator=(ChangedIdsIterator const&) = delete;

        const TableMap& GetTableMap(Utf8StringCR tableName) const;

    public:
        ECDB_EXPORT ChangedIdsIterator(ECDbCR ecdb, Changes const& changes);
        ChangedIdsIterator(ECDbCR ecdb, ChangeStream& changeSet) : ChangedIdsIterator(ecdb, changeSet.GetChanges()) {}

        ECDB_EXPORT const_iterator begin() const;
        ECDB_EXPORT const_iterator end() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE