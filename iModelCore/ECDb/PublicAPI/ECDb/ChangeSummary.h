/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ChangeSummary.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECInstanceId.h>
#include <BeSQLite/ChangeSet.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! A set of changes to database rows interpreted as EC instances. 
//! @see ChangeSet
//! @ingroup ECDbGroup
//! @bsiclass                                                 Ramanujam.Raman      07/2015
//=======================================================================================
struct ChangeSummary : NonCopyableClass
{
    //=======================================================================================
    // @bsiclass                                              Ramanujam.Raman      07/2015
    //=======================================================================================
    struct SqlChange
    {
    private:
        Changes::Change const& m_sqlChange;
        Utf8String m_tableName;
        int m_indirect;
        int m_nCols;
        DbOpcode m_dbOpcode;
    public:
        SqlChange(Changes::Change const& change);

        Changes::Change const& GetChange() const { return m_sqlChange; }
        Utf8CP GetTableName() const { return m_tableName.c_str(); }
        DbOpcode GetDbOpcode() const { return m_dbOpcode; }
        int GetIndirect() const { return m_indirect; }
        int GetNCols() const { return m_nCols; }
    };

    struct ECChange
        {
        private:
            ECN::ECClassId m_classId;
            ECInstanceId m_instanceId;
            DbOpcode m_dbOpcode;
            int m_indirect;
        public:
            ECChange(ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect) :
                m_classId(classId), m_instanceId(instanceId), m_dbOpcode(dbOpcode), m_indirect(indirect)
                {}
            ECN::ECClassId GetECClassId() const { return m_classId; }
            ECInstanceId GetECInstanceId() const { return m_instanceId; }
            DbOpcode GetDbOpcode() const { return m_dbOpcode; }
            int GetIndirect() const { return m_indirect; }
        };

    //=======================================================================================
    // @bsiclass                                              Ramanujam.Raman      07/2015
    //=======================================================================================
    struct ColumnMap
    {
    private:
        int m_columnIndex = -1;         // Index of column
        Utf8String m_columnName;        // Name of column
        Utf8String m_accessString;      // Property access string - only used for debugging
    public:
        ColumnMap() {}
        ColumnMap(Utf8CP accessString, Utf8CP columnName, int columnIndex) : m_accessString(accessString), m_columnName(columnName), m_columnIndex(columnIndex) {}

        Utf8CP GetColumnName() const { return m_columnName.c_str(); }
        Utf8CP GetAccessString() const { return m_accessString.c_str(); }
        int GetColumnIndex() const { return m_columnIndex; }
    };

    typedef bmap<Utf8String, ColumnMap> ColumnMapByAccessString;

    struct TableMap;
    typedef TableMap const* TableMapCP;
    typedef TableMap* TableMapP;

    struct ClassMap;
    typedef ClassMap const* ClassMapCP;
    typedef ClassMap* ClassMapP;

    //=======================================================================================
    // @bsiclass                                              Ramanujam.Raman      07/2015
    //=======================================================================================
    struct ClassMap
    {
        friend struct TableMap;
    private:
        Utf8String m_tableName;
        ECN::ECClassId m_classId = -1;      // ECClassId - only setup if the ECClassId doesn't depend
        bool m_isRelationship = false;      // Set to true if the class is a relationship 
        bool m_isStruct = false;            // Set to true if the class is a struct
        Utf8String m_className;             // Class name 
        int m_mapStrategy;                  // Map strategy
        bvector<ClassMapCP> m_baseClassMaps; // Maps of base classes *in* the same table. 

        ColumnMapByAccessString m_columnMapByAccessString; // Mapping of relevant columns organized by the property access string. 

        void AddColumn(Utf8CP accessString, Utf8CP columnName, int columnIndex);
        //! Returns false if the class is not mapped
        bool Initialize(ECDbR ecdb, ECN::ECClassId classId, Utf8CP tableName, bmap<Utf8String, int> const& columnIndexByName);
        void SetBaseClassMaps(bvector<ClassMapCP> const& baseClassMaps) { m_baseClassMaps = baseClassMaps; }
        
    public:
        ClassMap() {}

        Utf8CP GetTableName() const { return m_tableName.c_str(); }
        ECN::ECClassId GetClassId() const { return m_classId; }
        bool GetIsStruct() const { return m_isStruct; }
        bool GetIsRelationship() const { return m_isRelationship; }
        int GetMapStrategy() const { return m_mapStrategy; }
        bool GetIsForeignKeyRelationship() const;
        bvector<ClassMapCP> const& GetBaseClassMaps() const { return m_baseClassMaps; }
        Utf8CP GetColumnName(Utf8CP accessString) const;
        int GetColumnIndex(Utf8CP accessString) const;
        ColumnMapByAccessString const& GetColumnMapByAccessString() const { return m_columnMapByAccessString; }
    };

    typedef bmap<ECN::ECClassId, ClassMapP> ClassMapById;

    //=======================================================================================
    // @bsiclass                                              Ramanujam.Raman      07/2015
    //=======================================================================================
    struct TableMap
    {
    private:
        Utf8String m_tableName;
        ClassMapById m_classMaps;
        Utf8String m_ecInstanceIdColumnName;
        int  m_ecInstanceIdColumnIndex = -1;
        Utf8String m_ecClassIdColumnName;
        int  m_ecClassIdColumnIndex = -1;

        bool QueryIdColumnFromMap(Utf8StringR idColumnName, ECDbR ecdb, Utf8CP tableName, int userData) const;
        bool QueryInstanceIdColumnFromMap(Utf8StringR idColumnName, ECDbR ecdb, Utf8CP tableName) const;
        bool QueryClassIdColumnFromMap(Utf8StringR idColumnName, ECDbR ecdb, Utf8CP tableName) const;
        void FreeClassMap();

    public:
        TableMap() {}
        ~TableMap();

        void Initialize(ECDbR ecDb, Utf8CP tableName);
        Utf8CP GetTableName() const { return m_tableName.c_str(); }
        bool GetIsMapped() const { return !m_classMaps.empty(); }
        Utf8CP GetECInstanceIdColumnName() const { return m_ecInstanceIdColumnName.c_str(); }
        int GetECInstanceIdColumnIndex() const { return m_ecInstanceIdColumnIndex; }
        Utf8CP GetECClassIdColumnName() const { return m_ecClassIdColumnName.c_str(); }
        int GetECClassIdColumnIndex() const { return m_ecClassIdColumnIndex; }

        ClassMapById const& GetClassMaps() const { return m_classMaps; }
    };

    typedef bmap<Utf8String, TableMapP> TableMapByName;
    
    //! An iterator over changes in a ChangeSummary
    struct Iterator : BeSQLite::DbTableIterator
    {
    public:
        explicit Iterator(ECDbCR db) : DbTableIterator((BeSQLite::DbCR) db) {}

        //! An entry in the table.
        struct Entry : DbTableIterator::Entry, std::iterator < std::input_iterator_tag, Entry const >
        {
        private:
            friend struct Iterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) {}
        public:
            //! Get the ECClassId of the current change
            ECN::ECClassId GetClassId() const { return (ECN::ECClassId) m_sql->GetValueInt64(0); }

            //! Get the ECInstanceId of the current change
            ECInstanceId GetInstanceId() const { return m_sql->GetValueId<ECInstanceId>(1); }

            //! Get the DbOpcode of the current change (Insert, Update or Delete)
            DbOpcode GetDbOpcode() const { return (DbOpcode) m_sql->GetValueInt(2); }

            //! Get the flag indicating that the current change was indirectly caused
            bool GetIsIndirect() const { return m_sql->GetValueInt(3) > 0; }

            Entry const& operator*() const { return *this; }
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        ECDB_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(m_stmt.get(), false); }
        ECDB_EXPORT int QueryCount() const;
    }; // Iterator

private:
    ECDbR m_ecdb;
    bool m_isValid = false;

    mutable TableMapByName m_tableMapByName;

    Statement m_changeSummaryTableInsert;
    Statement m_changeSummaryTableUpdate;
    Statement m_changeSummaryTableSelect;
    Statement m_changeSummaryTableDelete;

    void Initialize();

    ChangeSummary::TableMapCP GetTableMap(Utf8CP tableName) const;
    void AddTableToMap(Utf8CP tableName) const;
    void FreeTableMap();

    BentleyStatus ProcessSqlChangeForTable(SqlChange const& sqlChange, TableMap const& tableMap);
    void ProcessSqlChangeForStructMap(SqlChange const& sqlChange, ChangeSummary::ClassMap const& classMap, ECInstanceId instanceId);
    void ProcessSqlChangeForForeignKeyMap(SqlChange const& sqlChange, ChangeSummary::ClassMap const& classMap, ECInstanceId instanceId);
    void ProcessSqlChangeForClassMap(SqlChange const& sqlChange, ChangeSummary::ClassMap const& classMap, ECInstanceId instanceId);

    ECInstanceId GetInstanceIdFromChange(SqlChange const& sqlChange, TableMap const& tableMap) const;
    bool SqlChangeHasUpdatesForClass(SqlChange const& sqlChange, ClassMap const& classMap) const;
    bool IsInstanceDeleted(ECInstanceId instanceId, TableMap const& tableMap) const;
    bool QueryColumnValueFromTable(int64_t& value, Utf8CP tableName, Utf8CP columnName, Utf8CP instanceIdColumnName, ECInstanceId instanceId) const;
    bool GetColumnValueFromChange(int64_t& value, SqlChange const& sqlChange, int columnIndex) const;
    bool GetColumnValueFromChangeOrTable(int64_t& value, SqlChange const& sqlChange, Utf8CP tableName, Utf8CP columnName, int columnIndex, Utf8CP instanceIdColumnName, ECInstanceId instanceId) const;
    ECN::ECClassId GetClassIdFromChangeOrTable(SqlChange const& sqlChange, TableMap const& tableMap, ECInstanceId instanceId) const;
    bool GetStructArrayParentFromChange(ECN::ECClassId& parentClassId, ECInstanceId& parentInstanceId, SqlChange const& sqlChange, ClassMap const& classMap, ECInstanceId instanceId);

    void CreateChangeSummaryTable();
    void DestroyChangeSummaryTable();
    void PrepareChangeSummaryTableStatements();
    void FinalizeChangeSummaryTableStatements();
    void RecordInChangeSummaryTable(ECChange const& ecChange);
    bool SelectInChangeSummaryTable(DbOpcode& dbOpcode, ECN::ECClassId classId, ECInstanceId instanceId);
    void InsertInChangeSummaryTable(ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect);
    void DeleteInChangeSummaryTable(ECN::ECClassId classId, ECInstanceId instanceId);
    void UpdateInChangeSummaryTable(ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect);
    void BindChangeSummaryTableStatement(Statement& statement, ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect);
    void BindChangeSummaryTableStatement(Statement& statement, ECN::ECClassId classId, ECInstanceId instanceId);


public:
    //! Construct a ChangeSummary from a BeSQLite ChangeSet
    ChangeSummary(ECDbR ecdb) : m_ecdb(ecdb) {}

    //! Destructor
    ECDB_EXPORT ~ChangeSummary();

    //! Get the Db used by this change set
    ECDbR GetDb() const { return m_ecdb; }

    //! Create a ChangeSummary from the contents of a BeSQLite ChangeSet
    //! @remarks The ChangeSummary needs to be new or freed before this call. 
    //! @see MakeIterator, GetChangeSummaryTableName
    ECDB_EXPORT BentleyStatus FromSqlChangeSet(BeSQLite::ChangeSet& changeSet);

    //! Get an iterator over the changes in this ChangeSummary
    //! Use @ref FromSqlChangeSet to populate the ChangeSummary table.
    Iterator MakeIterator() const { return Iterator(m_ecdb); }

    //! Get the name of the change set table
    //! @remarks The table includes ClassId, InstanceId and DbOpcode and IsIndirect columns, and can be used as part of other 
    //! queries. Use @ref FromSqlChangeSet to populate the ChangeSummary table.
    ECDB_EXPORT Utf8String GetChangeSummaryTableName() const;
        
    //! Free the data held by this ChangeSummary.
    //! @note Normally the destructor will call Free. After this call the ChangeSet is invalid.
    ECDB_EXPORT void Free();

    //! Determine whether this ChangeSet holds valid data or not.
    bool IsValid() const { return m_isValid; }

    //! Dump to stdout for debugging purposes.
    ECDB_EXPORT void Dump() const;
};

END_BENTLEY_SQLITE_EC_NAMESPACE
