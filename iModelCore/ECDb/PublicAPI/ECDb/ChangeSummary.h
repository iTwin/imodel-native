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

struct ChangeSummary;

//=======================================================================================
//! @private internal use only
//! A custom function to test if an instance exists in the change set
//! @e Example
//!     SELECT el.ECInstanceId
//!     FROM dgn.Element el
//!     JOIN dgn.ElementGeom elg USING dgn.ElementOwnsGeom
//!     WHERE IsChangedInstance(elg.GetECClassId(), elg.ECInstanceId)
//! @bsiclass                                                 Ramanujam.Raman      08/2015
//=======================================================================================
struct IsChangedInstanceSqlFunction : ScalarFunction
{
private:
    ChangeSummary const& m_changeSummary;
    virtual void _ComputeScalar(ScalarFunction::Context& ctx, int nArgs, DbValue* args) override;
public:
    IsChangedInstanceSqlFunction(ChangeSummary const& changeSummary)
        : m_changeSummary(changeSummary), ScalarFunction("IsChangedInstance", 2, DbValueType::IntegerVal) {}
    ~IsChangedInstanceSqlFunction() {}
};

//=======================================================================================
//! A set of changes to database rows interpreted as EC instances. 
//! @see ChangeSet
//! @ingroup ECDbGroup
//! @bsiclass                                                 Ramanujam.Raman      07/2015
//=======================================================================================
struct ChangeSummary : NonCopyableClass
{
    friend struct ChangeInstance;

    struct TableMap;
    typedef TableMap const* TableMapCP;
    typedef TableMap* TableMapP;

    struct ClassMap;
    typedef ClassMap const* ClassMapCP;
    typedef ClassMap* ClassMapP;

    struct Instance;
    struct InstanceIterator;
    struct ValueIterator;

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
        bset<int> m_primaryKeyColumnIndices;

    public:
        SqlChange(Changes::Change const& change);

        Changes::Change const& GetChange() const { return m_sqlChange; }
        Utf8CP GetTableName() const { return m_tableName.c_str(); }
        DbOpcode GetDbOpcode() const { return m_dbOpcode; }
        int GetIndirect() const { return m_indirect; }
        int GetNCols() const { return m_nCols; }
        bool IsPrimaryKeyColumn(int index) const { return m_primaryKeyColumnIndices.find(index) != m_primaryKeyColumnIndices.end(); }
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
        int m_columnType;

    public:
        ColumnMap() {}
        ColumnMap(Utf8CP accessString, Utf8CP columnName, int columnIndex, int columnType) 
            : m_accessString(accessString), m_columnName(columnName), m_columnIndex(columnIndex) , m_columnType (columnType) {}

        Utf8CP GetColumnName() const { return m_columnName.c_str(); }
        Utf8CP GetAccessString() const { return m_accessString.c_str(); }
        int GetColumnIndex() const { return m_columnIndex; }
        int GetColumnType() const { return m_columnType; }
    };

    typedef bmap<Utf8String, ColumnMap> ColumnMapByAccessString;

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

        void AddColumn(Utf8CP accessString, Utf8CP columnName, int columnIndex, int columnType);
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

    //! Represents a changed instance
    struct Instance
    {
    private:
        ECDbCP m_ecdb = nullptr;
        ECN::ECClassId m_classId = -1;
        ECInstanceId m_instanceId;
        DbOpcode m_dbOpcode;
        int m_indirect;
        mutable CachedStatementPtr m_valuesTableSelect;

        void SetupValuesTableSelectStatement(Utf8CP accessString) const;

    public:
        //! Empty constructor
        Instance() {}

        //! Constructor
        Instance(ECDbCR ecdb, ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect) :
            m_ecdb(&ecdb), m_classId(classId), m_instanceId(instanceId), m_dbOpcode(dbOpcode), m_indirect(indirect)
            {}

        //! Copy constructor
        Instance(Instance const& other) {*this = other;}

        //! Assignment operator
        ECDB_EXPORT Instance& operator=(Instance const& other);

        //! Get the class id of the changed instance
        ECN::ECClassId GetClassId() const { return m_classId; }

        //! Get the instance id of the changed instance
        ECInstanceId GetInstanceId() const { return m_instanceId; }

        //! Get the DbOpcode of the changed instance that indicates that the instance was inserted, updated or deleted.
        DbOpcode GetDbOpcode() const { return m_dbOpcode; }

        //! Get the flag indicating if the change was "indirectly" caused by a database trigger or other means. 
        int GetIndirect() const { return m_indirect; }

        //! Returns true if the instance is valid. 
        bool IsValid() const { return m_instanceId.IsValid(); }
    
        //! Returns true if the value specified by the accessString exists
        ECDB_EXPORT bool HasValue(Utf8CP accessString) const;

        //! Get a specific changed value
        ECDB_EXPORT DbDupValue GetOldValue(Utf8CP accessString) const;

        //! Get a specific changed value
        ECDB_EXPORT DbDupValue GetNewValue(Utf8CP accessString) const;
                
        //! Make an iterator over the changed values in a changed instance.
        ValueIterator MakeValueIterator(ECDbR ecdb) const { return ChangeSummary::ValueIterator(ecdb, m_classId, m_instanceId); }
    };

    //! An iterator over changed instances in a ChangeSummary
    struct InstanceIterator : BeSQLite::DbTableIterator
    {
    private:
        ECDbCR m_ecdb;

    public:
        explicit InstanceIterator(ECDbCR ecdb) : DbTableIterator((BeSQLite::DbCR) ecdb), m_ecdb(ecdb) {}

        //! An entry in the table.
        struct Entry : DbTableIterator::Entry, std::iterator < std::input_iterator_tag, Entry const >
            {
            private:
                friend struct InstanceIterator;
                ECDbCR m_ecdb;
                Entry(ECDbCR ecdb, BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid), m_ecdb(ecdb) {}

            public:
                //! Get the class id of the current change
                ECN::ECClassId GetClassId() const {return (ECN::ECClassId) m_sql->GetValueInt64(0);}

                //! Get the instance id of the current change
                ECInstanceId GetInstanceId() const { return m_sql->GetValueId<ECInstanceId>(1); }

                //! Get the DbOpcode of the current change
                DbOpcode GetDbOpcode() const { return (DbOpcode) m_sql->GetValueInt(2); }

                //! Get the flag indicating if the current change was "indirectly" caused by a database trigger or other means. 
                int GetIndirect() const { m_sql->GetValueInt(3); }
                
                //! Get the entire instance representing the current change.
                ECDB_EXPORT Instance GetInstance() const;

                Entry const& operator*() const { return *this; }
            };

        typedef Entry const_iterator;
        typedef Entry iterator;
        ECDB_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(m_ecdb, m_stmt.get(), false); }
        ECDB_EXPORT int QueryCount() const;
    }; // InstanceIterator

    //! An iterator over values in a changed instance
    struct ValueIterator : BeSQLite::DbTableIterator
    {
    private:
        ECN::ECClassId m_classId;
        ECInstanceId m_instanceId;

    public:
        explicit ValueIterator(BeSQLite::DbCR db, ECN::ECClassId classId, ECInstanceId instanceId)
            : DbTableIterator(db), m_classId(classId), m_instanceId(instanceId) {}

        //! An entry in the table of values in a changed instance.
        struct Entry : DbTableIterator::Entry, std::iterator < std::input_iterator_tag, Entry const >
        {
        private:
            friend struct ValueIterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) {}

        public:
            //! Gets the access string
            Utf8CP GetAccessString() const { return m_sql->GetValueText(0); }

            //! Gets the old value
            DbDupValue GetOldValue() const { return std::move(m_sql->GetDbValue(1)); }

            //! Gets the new value
            DbDupValue GetNewValue() const { return std::move(m_sql->GetDbValue(2)); }

            Entry const& operator*() const { return *this; }
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        ECDB_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(m_stmt.get(), false); }
        ECDB_EXPORT int QueryCount() const;
    }; // ValueIterator

    //! DbOpcodes that can be bitwise combined to pass as arguments to query methods
    enum QueryDbOpcode
        {
        Insert = 1,
        Delete = 1 << 1,
        Update = 1 << 2,
        All = Insert | Delete | Update
        };

private:
    ECDbR m_ecdb;
    bool m_isValid = false;

    mutable TableMapByName m_tableMapByName;

    mutable Statement m_instancesTableInsert;
    mutable Statement m_instancesTableUpdate;
    mutable Statement m_instancesTableSelect;

    mutable Statement m_valuesTableInsert;

    IsChangedInstanceSqlFunction* m_isChangedInstanceSqlFunction = nullptr;
    
    void Initialize();
    void RegisterSqlFunctions();
    void UnregisterSqlFunctions();

    ChangeSummary::TableMapCP GetTableMap(Utf8CP tableName) const;
    void AddTableToMap(Utf8CP tableName) const;
    void FreeTableMap();

    BentleyStatus ProcessSqlChangeForTable(SqlChange const& sqlChange, TableMap const& tableMap);
    void ProcessSqlChangeForStructMap(SqlChange const& sqlChange, ChangeSummary::ClassMap const& classMap, ECInstanceId instanceId);
    void ProcessSqlChangeForForeignKeyMap(SqlChange const& sqlChange, ChangeSummary::ClassMap const& classMap, ECInstanceId instanceId);
    void ProcessSqlChangeForClassMap(SqlChange const& sqlChange, ChangeSummary::ClassMap const& classMap, ECInstanceId instanceId);

    bool SqlChangeHasUpdatesForClass(SqlChange const& sqlChange, ClassMap const& classMap) const;
    
    DbDupValue GetValueFromTable(Utf8CP tableName, Utf8CP columnName, Utf8CP instanceIdColumnName, ECInstanceId instanceId) const;
    DbValue GetValueFromChange(SqlChange const& sqlChange, int columnIndex) const;

    int64_t GetValueInt64FromTable(Utf8CP tableName, Utf8CP columnName, Utf8CP instanceIdColumnName, ECInstanceId instanceId) const;
    int64_t GetValueInt64FromChange(SqlChange const& sqlChange, int columnIndex) const;
    int64_t GetValueInt64FromChangeOrTable(SqlChange const& sqlChange, Utf8CP tableName, Utf8CP columnName, int columnIndex, Utf8CP instanceIdColumnName, ECInstanceId instanceId) const;

    ECInstanceId GetInstanceIdFromChange(SqlChange const& sqlChange, TableMap const& tableMap) const;
    ECN::ECClassId GetClassIdFromChangeOrTable(SqlChange const& sqlChange, TableMap const& tableMap, ECInstanceId instanceId) const;
    bool IsInstanceDeleted(ECInstanceId instanceId, TableMap const& tableMap) const;

    bool GetStructArrayParentFromChange(ECN::ECClassId& parentClassId, ECInstanceId& parentInstanceId, SqlChange const& sqlChange, ClassMap const& classMap, ECInstanceId instanceId);

    void CreateInstancesTable();
    void DestroyInstancesTable();
    void PrepareInstancesTableStatements();
    void FinalizeInstancesTableStatements();
    void RecordInInstancesTable(ChangeSummary::Instance const& changeInstance);

    void InsertInInstancesTable(ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect);
    void UpdateInInstancesTable(ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect);
    void BindInstancesTableStatement(Statement& statement, ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect) const;
    void BindInstancesTableStatement(Statement& statement, ECN::ECClassId classId, ECInstanceId instanceId) const;
    Instance SelectInInstancesTable(ECN::ECClassId classId, ECInstanceId instanceId) const;

    void CreateValuesTable();
    void DestroyValuesTable();
    void PrepareValuesTableStatements();
    void FinalizeValuesTableStatements();
    void RecordInValuesTable(ChangeSummary::Instance const& changeInstance, SqlChange const& sqlChange, ChangeSummary::ClassMap const& classMap);

    void InsertInValuesTable(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue);
    void BindValuesTableStatement(Statement& statement, ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue);
   
    Utf8String ConstructWhereInClause(int queryDbOpcodes) const;
public:
    //! Construct a ChangeSummary from a BeSQLite ChangeSet
    ChangeSummary(ECDbR ecdb) : m_ecdb(ecdb) {}

    //! Destructor
    ECDB_EXPORT ~ChangeSummary();

    //! Get the Db used by this change set
    ECDbR GetDb() const { return m_ecdb; }

    //! Create a ChangeSummary from the contents of a BeSQLite ChangeSet
    //! @remarks The ChangeSummary needs to be new or freed before this call. 
    //! @see MakeIterator, GetInstancesTableName
    ECDB_EXPORT BentleyStatus FromSqlChangeSet(BeSQLite::ChangeSet& changeSet);

    //! Free the data held by this ChangeSummary.
    //! @note Normally the destructor will call Free. After this call the ChangeSet is invalid.
    ECDB_EXPORT void Free();

    //! Determine whether this ChangeSet holds valid data or not.
    bool IsValid() const { return m_isValid; }

    //! Dump to stdout for debugging purposes.
    ECDB_EXPORT void Dump() const;

    //! Make an iterator over the changed instances
    //! Use @ref FromSqlChangeSet to populate the ChangeSummary
    InstanceIterator MakeInstanceIterator() const { return InstanceIterator(m_ecdb); }

    //! Check if the change summary includes a specific instance
    ECDB_EXPORT bool HasInstance(ECN::ECClassId classId, ECInstanceId instanceId) const;

    //! Get a specific changed instance
    ECDB_EXPORT Instance GetInstance(ECN::ECClassId classId, ECInstanceId instanceId) const;

    //! Get the name of the table containing summary of changed instances
    //! @remarks The table includes ClassId, InstanceId, DbOpcode and Indirect columns, and can be used as part of other 
    //! queries. Use @ref FromSqlChangeSet to populate the ChangeSummary table.
    ECDB_EXPORT Utf8String GetInstancesTableName() const;

    //! Get the name of the table containing all the changed values
    //! @remarks The table includes ClassId, InstanceId, AccessString, OldValue, NewValue columns, and can be used as part of other 
    //! queries. Use @ref FromSqlChangeSet to populate the ChangeSummary table.
    ECDB_EXPORT Utf8String GetValuesTableName() const;

    //! @private internal use only
    //! Query for all changed instances of the specified class (and it's sub classes). 
    ECDB_EXPORT void QueryByClass(bmap<ECInstanceId, ChangeSummary::Instance>& changes, ECN::ECClassId classId, bool isPolymorphic = true, int queryDbOpcodes = QueryDbOpcode::All) const;
};

END_BENTLEY_SQLITE_EC_NAMESPACE
