/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ChangeSummary.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECInstanceId.h>
#include <BeSQLite/ChangeSet.h>

ECDB_TYPEDEFS(ChangeSummary);
ECDB_TYPEDEFS(InstancesTable);
ECDB_TYPEDEFS(ValuesTable);

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ChangeExtractor;
typedef ChangeExtractor* ChangeExtractorP;

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
    virtual void _ComputeScalar(ScalarFunction::Context& ctx, int nArgs, DbValue* args) override;
public:
    IsChangedInstanceSqlFunction() : ScalarFunction("IsChangedInstance", 3, DbValueType::IntegerVal) {}
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
        UpdateDelete    = Update | Delete,
        };

    //! Options to control extraction of the change summary
    struct Options
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
        ChangeSummaryCP m_changeSummary = nullptr;
        ECN::ECClassId m_classId;
        ECInstanceId m_instanceId;
        DbOpcode m_dbOpcode;
        int m_indirect;
        Utf8String m_tableName;
        mutable CachedStatementPtr m_valuesTableSelect;

        void SetupValuesTableSelectStatement(Utf8CP accessString) const;

    public:
        //! Empty constructor
        Instance() {}

        //! Constructor
        Instance(ChangeSummaryCR changeSummary, ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect, Utf8StringCR tableName) :
            m_changeSummary(&changeSummary), m_classId(classId), m_instanceId(instanceId), m_dbOpcode(dbOpcode), m_indirect(indirect), m_tableName(tableName)
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
        ValueIterator MakeValueIterator(ChangeSummaryCR changeSummary) const { return ChangeSummary::ValueIterator(changeSummary, m_classId, m_instanceId); }
    };

    typedef Instance const& InstanceCR;

    //! An iterator over changed instances in a ChangeSummary
    struct InstanceIterator : BeSQLite::DbTableIterator
    {
    public:
        struct Options
        {
        private:
            friend struct InstanceIterator;

            ECN::ECClassId m_classId;
            bool m_polymorphic;
            QueryDbOpcode m_opcodes;

            Utf8String ToSelectStatement(Utf8CP columnsToSelect, ChangeSummaryCR summary) const;
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
        ChangeSummaryCR m_changeSummary;
        Options m_options;
        
        Utf8String MakeSelectStatement(Utf8CP columns) const;
    public:
        explicit InstanceIterator(ChangeSummaryCR changeSummary, Options const& options=Options())
            : DbTableIterator((BeSQLite::DbCR) changeSummary.GetDb()), m_changeSummary(changeSummary), m_options(options) { }

        //! An entry in the table.
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
            {
            private:
                friend struct InstanceIterator;
                ChangeSummaryCR m_changeSummary;
                Entry(ChangeSummaryCR changeSummary, BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid), m_changeSummary(changeSummary) {}

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

                ChangeSummaryCR GetChangeSummary() const { return m_changeSummary; } //!< @private
            };

        typedef Entry const_iterator;
        typedef Entry iterator;
        ECDB_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(m_changeSummary, m_stmt.get(), false); }
        ECDB_EXPORT int QueryCount() const;
    }; // InstanceIterator

    //! An iterator over values in a changed instance
    struct ValueIterator : BeSQLite::DbTableIterator
    {
    private:
        ChangeSummaryCR m_changeSummary;
        ECN::ECClassId m_classId;
        ECInstanceId m_instanceId;

    public:
        ValueIterator(ChangeSummaryCR changeSummary, ECN::ECClassId classId, ECInstanceId instanceId)
            : DbTableIterator(changeSummary.GetDb()), m_changeSummary (changeSummary), m_classId(classId), m_instanceId(instanceId)
            {}

        //! An entry in the table of values in a changed instance.
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
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

    //! @private
    //! Information on mappings to a column
    struct ColumnMapInfo
    {
    private:
        Utf8String m_columnName;
        int m_columnIndex;
    public:
        ColumnMapInfo() {}
        ColumnMapInfo(Utf8StringCR columnName, int columnIndex) : m_columnName(columnName), m_columnIndex(columnIndex) {}

        Utf8StringCR GetColumnName() const { return m_columnName; }
        int GetColumnIndex() const { return m_columnIndex; }
    };

    //! @private
    //! Information on mappings to a table
    struct TableMapInfo
    {
    friend struct ChangeSummary;
    private:
        Utf8String m_tableName;
        bmap<Utf8String, ColumnMapInfo> m_columnMapByAccessString;
    public:
        Utf8StringCR GetTableName() const { return m_tableName; }
        ColumnMapInfo const& GetColumnMap(Utf8CP propertyAccessString) const
            {
            bmap<Utf8String, ColumnMapInfo>::const_iterator iter = m_columnMapByAccessString.find(propertyAccessString);
            BeAssert(iter != m_columnMapByAccessString.end());
            return iter->second;
            }
    };

private:
    ECDbCR m_ecdb;
    bool m_isValid = false;
    InstancesTableP m_instancesTable;
    ValuesTableP m_valuesTable;
    ChangeExtractorP m_changeExtractor;

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
    ECDB_EXPORT ~ChangeSummary();

    //! Get the Db used by this change set
    ECDbCR GetDb() const { return m_ecdb; }

    //! Create a ChangeSummary from the contents of a BeSQLite ChangeSet
    //! @remarks The ChangeSummary needs to be new or freed before this call. 
    //! @see MakeIterator, GetInstancesTableName
    ECDB_EXPORT BentleyStatus FromChangeSet(BeSQLite::IChangeSet& changeSet, Options const& options = Options());

    //! Free the data held by this ChangeSummary.
    //! @note Normally the destructor will call Free. After this call the ChangeSet is invalid.
    ECDB_EXPORT void Free();

    //! Determine whether this ChangeSet holds valid data or not.
    bool IsValid() const { return m_isValid; }

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

    //! @private
    Utf8String ConstructWhereInClause(QueryDbOpcode queryDbOpcodes) const;

    //! @private internal use only
    //! Utility to get the mapping information on the primary table containing the class
    ECDB_EXPORT static BentleyStatus GetPrimaryTableMapInfo(TableMapInfo& tableMapInfo, ECN::ECClassCR cls, ECDbCR ecdb);
};

ENUM_IS_FLAGS(ChangeSummary::QueryDbOpcode);

END_BENTLEY_SQLITE_EC_NAMESPACE
