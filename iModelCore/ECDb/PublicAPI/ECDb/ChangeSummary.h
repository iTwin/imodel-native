/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ChangeSummary.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECInstanceId.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ChangeSet.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @private internal use only
//! A custom function to test if an instance exists in the change set
//! @e Example
//!     SELECT el.ECInstanceId
//!     FROM dgn.Element el
//!     JOIN dgn.ElementGeom elg USING dgn.ElementOwnsGeom
//!     WHERE IsChangedInstance(:changeSummary, elg.ECClassId, elg.ECInstanceId)
//! @bsiclass                                                 Ramanujam.Raman      08/2015
//=======================================================================================
struct IsChangedInstanceSqlFunction final : ScalarFunction
{
private:
    void _ComputeScalar(ScalarFunction::Context& ctx, int nArgs, DbValue* args) override;

public:
    IsChangedInstanceSqlFunction() : ScalarFunction("IsChangedInstance", 3, DbValueType::IntegerVal) {}
    ~IsChangedInstanceSqlFunction() {}
};

struct TableMap;
struct TableClassMap;
struct ColumnMap;
struct SqlChange;

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
//! Use ChangeSummary instead to get a consolidated view of the changed ECInstances and 
//! ECRelationshipInstance-s - extracting and consolidating this information involves two 
//! passes with this iterator.
//!
//! @see ChangeSet, ChangeSummary
//! @ingroup ECDbGroup
//! @bsiclass                                               Ramanujam.Raman      12/2016
//=======================================================================================
struct ChangeIterator final : NonCopyableClass
{
    struct ColumnIterator;

    typedef bmap<Utf8String, RefCountedPtr<TableMap>> TableMapByName;

    //! An entry in the ChangeIterator.
    struct RowEntry
    {
    friend ChangeIterator;

    private:
        ECDbCR m_ecdb;
        ChangeIterator const& m_iterator;
        Changes::Change m_change;

        SqlChange const* m_sqlChange;
        TableMap const* m_tableMap;
        ECInstanceId m_primaryInstanceId;
        ECN::ECClassCP m_primaryClass;
        static const Utf8String s_emptyString;

        RowEntry(ChangeIterator const& iterator, Changes::Change const& change);

        void Initialize();
        void InitPrimaryInstance();
        void InitSqlChange();
        void FreeSqlChange();
        void Reset();

    public:
        ECDB_EXPORT RowEntry(RowEntry const& other);

        ECDB_EXPORT RowEntry& operator=(RowEntry const& other);

        ECDB_EXPORT ~RowEntry();

        //! Returns true if the entry points to a row that's mapped to a ECClass
        ECDB_EXPORT bool IsMapped() const;

        //! Get the table name of the current change
        ECDB_EXPORT Utf8StringCR GetTableName() const;

        //! Return true if the current change is in the primary table (and not a joined or overflow table)
        ECDB_EXPORT bool IsPrimaryTable() const;

        //! Get the DbOpcode of the current change
        ECDB_EXPORT DbOpcode GetDbOpcode() const;

        //! Get the primary class of the current change
        ECN::ECClassCP GetPrimaryClass() const { return m_primaryClass; }

        //! Get the (primary) instance id of the current change
        ECInstanceId GetPrimaryInstanceId() const { return m_primaryInstanceId; }

        //! Make an iterator over the changed columns of the specified class
        ECDB_EXPORT ColumnIterator MakeColumnIterator(ECN::ECClassCR ecClass) const;

        //! Make an iterator over the changed columns of the primary class mapped to the table
        ECDB_EXPORT ColumnIterator MakePrimaryColumnIterator() const;

        //! Get the flag indicating if the current change was "indirectly" caused by a database trigger or other means. 
        ECDB_EXPORT int GetIndirect() const;

        ECDB_EXPORT RowEntry& operator++();

        RowEntry const& operator* () const { return *this; }
        bool operator!=(RowEntry const& rhs) const { return m_change != rhs.m_change; }
        bool operator==(RowEntry const& rhs) const { return m_change == rhs.m_change; }

        ECDbCR GetDb() const { return m_ecdb; } //!< @private
        SqlChange const* GetSqlChange() const { return m_sqlChange; } //!< @private
        Changes::Change const& GetChange() const {return m_change;} //!< @private
        TableMap const* GetTableMap() const { return m_tableMap; } //!< @private
        ChangeIterator const& GetChangeIterator() const {return m_iterator;}  //!< @private
        ECN::ECClassId GetClassIdFromChangeOrTable(Utf8CP classIdColumnName, ECInstanceId whereInstanceIdIs) const; //!< @private
    };

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
        static const Utf8String s_emptyString;

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
    friend RowEntry;

    private:
        RowEntry const& m_rowEntry;
        ECDbCR m_ecdb;
        SqlChange const* m_sqlChange;
        TableClassMap const* m_tableClassMap;

        ColumnIterator(RowEntry const&, ECN::ECClassCP);

    public:
        //! Get the column for the specified property access string
        ECDB_EXPORT ColumnEntry GetColumn(Utf8CP propertyAccessString) const;

        typedef ColumnEntry const_iterator;
        typedef ColumnEntry iterator;
        ECDB_EXPORT ColumnEntry begin() const;
        ECDB_EXPORT ColumnEntry end() const;

        ECDbCR GetDb() const { return m_ecdb; } //!< @private
        RowEntry const& GetRowEntry() const { return m_rowEntry; } //!< @private
        SqlChange const* GetSqlChange() const { return m_sqlChange; } //!< @private
    };

    private:
        ECDbCR m_ecdb;
        Changes m_changes;
        mutable TableMapByName* m_tableMapByName = nullptr;

        void InitTableMap();
        void AddTableToMap(Utf8StringCR tableName) const;
        void FreeTableMap();

    public:
        //! Construct a ChangeSummary from a BeSQLite ChangeSet
        ECDB_EXPORT explicit ChangeIterator(ECDbCR ecdb, IChangeSet& changeSet);
        ECDB_EXPORT explicit ChangeIterator(ECDbCR ecdb, Changes const& changes);

        //! Destructor
        ECDB_EXPORT ~ChangeIterator();

        typedef RowEntry const_iterator;
        typedef RowEntry iterator;
        ECDB_EXPORT RowEntry begin() const;
        ECDB_EXPORT RowEntry end() const;

        ECDbCR GetDb() const { return m_ecdb; } //!< @private
        TableMap const* GetTableMap(Utf8StringCR tableName) const;  //!< @private
};

typedef ChangeIterator const& ChangeIteratorCR;
typedef ChangeIterator const* ChangeIteratorCP;

struct InstancesTable;
struct ValuesTable;
struct ChangeExtractor;

//=======================================================================================
//! Utility to interpret a set of changes to the database as EC instances. 
//! 
//! @remarks The utility iterates over the raw Sqlite changes to consolidate and extract 
//! information on the contained ECInstances and ECRelationshipInstances. 
//! 
//! Internally two passes are made over the changes with the @ref ChangeIterator. The
//! second pass allows consolidation of instances and relationship instances that may be 
//! spread across multiple tables. The results are stored in temporary tables, and this 
//! allows iteration and queries of the changes as EC instances. 
//! 
//! @see ChangeSet, ChangeIterator
//! @ingroup ECDbGroup
//! @bsiclass                                                 Ramanujam.Raman      07/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ChangeSummary : NonCopyableClass
{
    //! DbOpcodes that can be bitwise combined to pass as arguments to query methods
    enum class QueryDbOpcode
    {
        None            = 0,
        Insert          = 1,
        Delete          = 1 << 1,
        Update          = 1 << 2,
        All             = Insert | Delete | Update,
        InsertUpdate    = Insert | Update,
        InsertDelete    = Insert | Delete,
        UpdateDelete    = Update | Delete
    };

    //! Options to control extraction of the change summary
    struct Options final
    {
    private:
        bool m_includeRelationshipInstances;
    public:
        Options() : m_includeRelationshipInstances(true) {}
        void SetIncludeRelationshipInstances(bool value) { m_includeRelationshipInstances = value; }
        bool GetIncludeRelationshipInstances() const { return m_includeRelationshipInstances; }
    };

    struct Instance;
    struct InstanceIterator;
    struct ValueIterator;

    //! Represents a changed instance
    struct Instance
    {
    private:
        ChangeSummary const* m_changeSummary = nullptr;
        ECN::ECClassId m_classId;
        ECInstanceId m_instanceId;
        DbOpcode m_dbOpcode;
        int m_indirect;
        Utf8String m_tableName;
        mutable CachedStatementPtr m_valuesTableSelect;

        void SetupValuesTableSelectStatement(Utf8CP accessString) const;

    public:
        Instance() {}

        Instance(ChangeSummary const& changeSummary, ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect, Utf8StringCR tableName) :
            m_changeSummary(&changeSummary), m_classId(classId), m_instanceId(instanceId), m_dbOpcode(dbOpcode), m_indirect(indirect), m_tableName(tableName)
            {}

        Instance(Instance const& other) {*this = other;}

        ECDB_EXPORT Instance& operator=(Instance const& other);

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
        ECDB_EXPORT bool ContainsValue(Utf8CP accessString) const;

        //! Get a specific changed value
        ECDB_EXPORT DbDupValue GetOldValue(Utf8CP accessString) const;

        //! Get a specific changed value
        ECDB_EXPORT DbDupValue GetNewValue(Utf8CP accessString) const;
                
        //! Make an iterator over the changed values in a changed instance.
        ValueIterator MakeValueIterator() const { return ChangeSummary::ValueIterator(*m_changeSummary, m_classId, m_instanceId); }
    };

    typedef Instance const& InstanceCR;

    //! An iterator over changed instances in a ChangeSummary
    struct InstanceIterator final : BeSQLite::DbTableIterator
    {
    public:
        struct Options
        {
        private:
            friend struct InstanceIterator;

            ECN::ECClassId m_classId;
            bool m_polymorphic;
            QueryDbOpcode m_opcodes;

            Utf8String ToSelectStatement(Utf8CP columnsToSelect, ChangeSummary const& summary) const;
            void Bind(BeSQLite::Statement& stmt) const;
        public:
            explicit Options(ECN::ECClassId classId=ECN::ECClassId(), bool polymorphic=true, QueryDbOpcode queryDbOpcodes=QueryDbOpcode::All)
                : m_classId(classId), m_polymorphic(polymorphic), m_opcodes(queryDbOpcodes) { }

            ECN::ECClassId GetClassId() const { return m_classId; }
            bool IsPolymorphic() const { return m_polymorphic; }
            QueryDbOpcode GetOpcodes() const { return m_opcodes; }

            bool IsEmpty() const { return !m_classId.IsValid(); }
        };
    private:
        ChangeSummary const& m_changeSummary;
        Options m_options;
        
        Utf8String MakeSelectStatement(Utf8CP columns) const;
    public:
        explicit InstanceIterator(ChangeSummary const& changeSummary, Options const& options=Options())
            : DbTableIterator((BeSQLite::DbCR) changeSummary.GetDb()), m_changeSummary(changeSummary), m_options(options) { }

        //! An entry in the table.
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct InstanceIterator;
            ChangeSummary const& m_changeSummary;
            Entry(ChangeSummary const& changeSummary, BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid), m_changeSummary(changeSummary) {}

        public:
            //! Get the class id of the current change
            ECN::ECClassId GetClassId() const {return (ECN::ECClassId) m_sql->GetValueUInt64(0);}

            //! Get the instance id of the current change
            ECInstanceId GetInstanceId() const { return m_sql->GetValueId<ECInstanceId>(1); }

            //! Get the DbOpcode of the current change
            DbOpcode GetDbOpcode() const { return (DbOpcode) m_sql->GetValueInt(2); }

            //! Get the flag indicating if the current change was "indirectly" caused by a database trigger or other means. 
            int GetIndirect() const { return m_sql->GetValueInt(3); }
                
            //! Get the entire instance representing the current change.
            ECDB_EXPORT Instance GetInstance() const;

            Entry const& operator*() const { return *this; }

            ChangeSummary const& GetChangeSummary() const { return m_changeSummary; } //!< @private
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        ECDB_EXPORT const_iterator begin() const;
        ECDB_EXPORT const_iterator end() const;
        ECDB_EXPORT int QueryCount() const;
    }; // InstanceIterator

    //! An iterator over values in a changed instance
    struct ValueIterator final : BeSQLite::DbTableIterator
    {
    private:
        ChangeSummary const& m_changeSummary;
        ECN::ECClassId m_classId;
        ECInstanceId m_instanceId;

    public:
        ValueIterator(ChangeSummary const& changeSummary, ECN::ECClassId classId, ECInstanceId instanceId)
            : DbTableIterator(changeSummary.GetDb()), m_changeSummary (changeSummary), m_classId(classId), m_instanceId(instanceId)
            {}

        //! An entry in the table of values in a changed instance.
        struct Entry final : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct ValueIterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) {}

        public:
            //! Gets the access string
            Utf8CP GetAccessString() const { return m_sql->GetValueText(0); }

            //! Gets the old value
            DbDupValue GetOldValue() const { return m_sql->GetDbValue(1); }

            //! Gets the new value
            DbDupValue GetNewValue() const { return m_sql->GetDbValue(2); }

            Entry const& operator*() const { return *this; }
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        ECDB_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(m_stmt.get(), false); }
        ECDB_EXPORT int QueryCount() const;
    }; // ValueIterator

private:
    ECDbCR m_ecdb;
    bool m_isValid = false;
    InstancesTable* m_instancesTable;
    ValuesTable* m_valuesTable;
    ChangeExtractor* m_changeExtractor;

    static int s_count;
    static IsChangedInstanceSqlFunction* s_isChangedInstanceSqlFunction;
    
    void Initialize();
    static void RegisterSqlFunctions(ECDbCR);
    static void UnregisterSqlFunctions(ECDbCR);
    Utf8String FormatInstanceIdStr(ECInstanceId) const;
    Utf8String FormatClassIdStr(ECN::ECClassId) const;

public:
    //! Construct a ChangeSummary from a BeSQLite ChangeSet
    ECDB_EXPORT explicit ChangeSummary(ECDbCR);

    //! Destructor
    ECDB_EXPORT virtual ~ChangeSummary();

    //! Populate the ChangeSummary from the contents of a BeSQLite ChangeSet
    //! @remarks The ChangeSummary needs to be new or freed before this call. 
    //! @see MakeIterator, GetInstancesTableName
    ECDB_EXPORT BentleyStatus FromChangeSet(BeSQLite::IChangeSet& changeSet, Options const& options = Options());

    //! Free the data held by this ChangeSummary.
    //! @note After this call the ChangeSet becomes invalid. Need not be called if used only once - 
    //! the destructor will automatically call Free. 
    ECDB_EXPORT void Free();

    //! Determine whether this ChangeSet holds extracted data or not.
    bool IsValid() const { return m_isValid; }

    //! Get the Db used by this change set
    ECDbCR GetDb() const { return m_ecdb; }

    //! Dump to stdout for debugging purposes.
    ECDB_EXPORT void Dump() const;

    //! Make an iterator over the changed instances
    //! Use @ref FromChangeSet to populate the ChangeSummary
    InstanceIterator MakeInstanceIterator(InstanceIterator::Options const& options=InstanceIterator::Options()) const { return InstanceIterator(*this, options); }

    //! Check if the change summary contains a specific instance
    ECDB_EXPORT bool ContainsInstance(ECN::ECClassId classId, ECInstanceId instanceId) const;

    //! Get a specific changed instance
    ECDB_EXPORT Instance GetInstance(ECN::ECClassId classId, ECInstanceId instanceId) const;

    //! Get the name of the table containing summary of changed instances
    //! @remarks The table includes ClassId, InstanceId, DbOpcode and Indirect columns, and can be used as part of other 
    //! queries. Use @ref FromChangeSet to populate the ChangeSummary table.
    ECDB_EXPORT Utf8String GetInstancesTableName() const;

    //! Get the name of the table containing all the changed values
    //! @remarks The table includes ClassId, InstanceId, AccessString, OldValue, NewValue columns, and can be used as part of other 
    //! queries. Use @ref FromChangeSet to populate the ChangeSummary table.
    ECDB_EXPORT Utf8String GetValuesTableName() const;

    //! @private internal use only
    //! Query for all changed instances of the specified class (and it's sub classes). 
    ECDB_EXPORT void QueryByClass(bmap<ECInstanceId, ChangeSummary::Instance>& changes, ECN::ECClassId classId, bool isPolymorphic = true, QueryDbOpcode queryDbOpcodes = QueryDbOpcode::All) const;

    Utf8String ConstructWhereInClause(QueryDbOpcode queryDbOpcodes) const; //! @private
    ECDB_EXPORT static BentleyStatus GetMappedPrimaryTable(Utf8StringR tableName, bool& isTablePerHierarcy, ECN::ECClassCR ecClass, ECDbCR ecdb); //!< @private
};

typedef ChangeSummary const& ChangeSummaryCR;
typedef ChangeSummary const* ChangeSummaryCP;

ENUM_IS_FLAGS(ChangeSummary::QueryDbOpcode);

END_BENTLEY_SQLITE_EC_NAMESPACE
