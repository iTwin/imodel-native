/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ChangeIterator.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECDb.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ChangeSet.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Utility to iterate over change records and interpret it as ECInstances. 
//!
//! @remarks The utility provides a forward only iterator over the rows of a change set/stream, 
//! and has methods to interpret the raw SQLite changes as ECInstances, taking into account
//! the mapping between the ECSchema and the persisted SQLite Db. 
//!
//! The changes to ECInstances and ECRelationshipInstances may be spread over multiple 
//! tables, and the may be streamed in no particular order. This utility does not attempt 
//! to consolidate the EC information, and doesn't provide information on the relationships. 
//! It must be used only for cases of mining specific EC information from the ChangeSet-s 
//! without incurring the performance penalty of creating change summaries. 
//!
//! See @ref ECDbChange to get a consolidated view of the changed ECInstances and 
//! ECRelationshipInstances
//!
//! @see @ref ECDbChange, ChangeSet
//! @ingroup ECDbGroup
// @bsiclass                                               12/2016
//=======================================================================================
struct ChangeIterator final
    {
    struct ColumnIterator;
    struct TableMap;
    struct TableMapCollection;
    struct TableClassMap;
    struct ColumnMap;
    struct SqlChange;

    struct RowEntry;

    //! An entry in the ColumnInterator
    struct ColumnEntry final
        {
        friend ColumnIterator;

        private:
            ECDbCR m_ecdb;
            SqlChange const* m_sqlChange;
            ColumnIterator const& m_columnIterator;
            bmap<Utf8String, ColumnMap*>::const_iterator m_columnMapIterator;
            bool m_isValid = false;

            explicit ColumnEntry(ColumnIterator const&);
            ColumnEntry(ColumnIterator const&, bmap<Utf8String, ColumnMap*>::const_iterator columnMapIterator);

        public:
            //! Get the access string for the EC property corresponding to this column
            ECDB_EXPORT Utf8StringCR GetPropertyAccessString() const;

            //! Get the old or new value from the change
            ECDB_EXPORT DbDupValue GetValue(Changes::Change::Stage stage) const;

            //! Query the current value from the Db
            ECDB_EXPORT DbDupValue QueryValueFromDb() const;

            //! Returns true if the column stores the primary key
            ECDB_EXPORT bool IsPrimaryKeyColumn() const;

            ECDB_EXPORT ColumnEntry& operator++();

            ColumnEntry const& operator*() const { return *this; }
            ECDB_EXPORT bool operator!=(ColumnEntry const& rhs) const;
            ECDB_EXPORT bool operator==(ColumnEntry const& rhs) const;
        };

    //! Iterator to go over changed columns
    struct ColumnIterator final
        {
        friend ColumnEntry;
        private:
            RowEntry const& m_rowEntry;
            ECDbCR m_ecdb;
            SqlChange const* m_sqlChange = nullptr;
            TableClassMap const* m_tableClassMap = nullptr;


        public:
            //! @private
            ColumnIterator(RowEntry const&, ECN::ECClassCP);

            //! Get the column for the specified property access string
            ECDB_EXPORT ColumnEntry GetColumn(Utf8CP propertyAccessString) const;

            typedef ColumnEntry const_iterator;
            typedef ColumnEntry iterator;
            ECDB_EXPORT const_iterator begin() const;
            ECDB_EXPORT const_iterator end() const;

            //! @private
            RowEntry const& GetRowEntry() const { return m_rowEntry; }
        };

    //! An entry in the ChangeIterator.
    struct RowEntry final
        {
        friend ChangeIterator;

        private:
            ECDbCR m_ecdb;
            ChangeIterator const& m_iterator;
            Changes::Change m_change;

            SqlChange const* m_sqlChange = nullptr;
            TableMap const* m_tableMap = nullptr;
            ECInstanceId m_primaryInstanceId;
            ECN::ECClassCP m_primaryClass = nullptr;

            RowEntry(ChangeIterator const& iterator, Changes::Change const& change);

            void Initialize();
            void InitPrimaryInstance();
            void FreeSqlChange();
            void Reset();

        public:
            ECDB_EXPORT ~RowEntry();
            ECDB_EXPORT RowEntry(RowEntry const& other);
            ECDB_EXPORT RowEntry& operator=(RowEntry const& other);

            bool IsValid() const { return m_tableMap != nullptr; }

            //! Returns true if the entry points to a row that's mapped to a ECClass
            bool IsMapped() const { return m_primaryClass != nullptr; }

            //! Get the table name of the current change
            ECDB_EXPORT Utf8StringCR GetTableName() const;

            //! Return true if the current change is in the primary table (and not a joined or overflow table)
            ECDB_EXPORT bool IsPrimaryTable() const;

            //! Get the DbOpcode of the current change
            ECDB_EXPORT DbOpcode GetDbOpcode() const;

            //! Get the flag indicating if the current change was "indirectly" caused by a database trigger or other means. 
            ECDB_EXPORT int GetIndirect() const;

            //! Get the primary class of the current change
            ECN::ECClassCP GetPrimaryClass() const { return m_primaryClass; }

            //! Get the (primary) instance id of the current change
            ECInstanceId GetPrimaryInstanceId() const { return m_primaryInstanceId; }

            //! Make an iterator over the changed columns of the specified class
            ECDB_EXPORT ColumnIterator MakeColumnIterator(ECN::ECClassCR ecClass) const;
            //! Make an iterator over the changed columns of the primary class mapped to the table
            ECDB_EXPORT ColumnIterator MakePrimaryColumnIterator() const;

            ECDB_EXPORT RowEntry& operator++();

            RowEntry const& operator*() const { return *this; }
            bool operator==(RowEntry const& rhs) const { return m_change == rhs.m_change; }
            bool operator!=(RowEntry const& rhs) const { return !(*this == rhs); }

            ECDB_EXPORT Utf8String ToString() const;

            ECDbCR GetDb() const { return m_ecdb; } //!< @private
            SqlChange const* GetSqlChange() const { return m_sqlChange; } //!< @private
            Changes::Change const& GetChange() const { return m_change; } //!< @private
            TableMap const* GetTableMap() const { return m_tableMap; } //!< @private
            ChangeIterator const& GetChangeIterator() const { return m_iterator; }  //!< @private
            ECN::ECClassId GetClassIdFromChangeOrTable(Utf8CP classIdColumnName, ECInstanceId whereInstanceIdIs) const; //!< @private
        };

  
    private:
        ECDbCR m_ecdb;
        Changes m_changes;
        mutable std::unique_ptr<TableMapCollection> m_tableMaps;

        //not copyable
        ChangeIterator(ChangeIterator const&) = delete;
        ChangeIterator& operator=(ChangeIterator const&) = delete;

    public:
        ECDB_EXPORT ChangeIterator(ECDbCR ecdb, Changes const& changes);
        ChangeIterator(ECDbCR ecdb, IChangeSet& changeSet) : ChangeIterator(ecdb, changeSet.GetChanges()) {}
        ECDB_EXPORT ~ChangeIterator();

        typedef RowEntry const_iterator;
        typedef RowEntry iterator;
        ECDB_EXPORT const_iterator begin() const;
        ECDB_EXPORT const_iterator end() const;

        //! @private
        TableMap const* GetTableMap(Utf8StringCR tableName) const; 
    };

END_BENTLEY_SQLITE_EC_NAMESPACE