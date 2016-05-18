/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSql.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"
#include <BeXml/BeXml.h>
#include <Bentley/BeVersion.h>

#include <unordered_map>
#include <unordered_set>
#include "BeBriefcaseBasedIdSequence.h"
#include "MapStrategy.h"

#define DBSCHEMA_NULLTABLENAME "ECDbNotMapped"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

typedef int64_t ECDbTableId;
typedef int64_t ECDbColumnId;
typedef int64_t ECDbIndexId;
typedef int64_t ECDbConstraintId;
typedef int64_t ECDbPropertyPathId;
typedef int64_t ECDbClassMapId;

enum class ColumnKind
    {
    Unknown = 0, //! Not known to ECDb or user define columns
    ECInstanceId = 1, //! ECInstanceId system column also primary key of the table
    ECClassId = 2, //! ECClassId system column. Use if more then on classes is mapped to this table
    ParentECInstanceId = 4, //! ParentECInstanceId column used in struct array
    ECPropertyPathId = 8, //! ECPropertyPathId column used in struct array
    ECArrayIndex = 16, //! ECArrayIndex column used in struct array
    SourceECInstanceId = 32,
    SourceECClassId = 64,
    TargetECInstanceId = 128,
    TargetECClassId = 256,
    DataColumn = 512, //! Data column defined by none key column in ECClass
    //Following is helper group for search operation. There cannot be a column with OR'ed flags
    ConstraintECInstanceId = SourceECInstanceId | TargetECInstanceId,
    NonRelSystemColumn = ECInstanceId | ECClassId | ParentECInstanceId | ECPropertyPathId | ECArrayIndex
    };



enum class TriggerType
    {
    Create,
    Update,
    Delete
    };
enum class TriggerSubType
    {
    Before,
    After
    };
enum class PersistenceType
    {
    Persisted, //! Persisted in DB
    Virtual //! Not persisted in db rather used as a view specification
    };

enum class TableType
    {
    Primary = 0,
    Joined = 1,
    Existing = 2,
    StructArray = 3
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct EditHandle : NonCopyableClass
    {
    private:
        bool m_canEdit : 1;
        bool m_isModified : 2;

    public:
        EditHandle ();
        bool BeginEdit ();
        bool IsModified () const;
        bool SetModified ();
        bool EndEdit ();
        bool CanEdit () const;
        bool AssertNotInEditMode ();
        bool AssertInEditMode () const;
        EditHandle (EditHandle const &&  eh);
        ~EditHandle ();
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
enum class ECDbSqlTypeAffinity
    {
    Integer, Real, Numeric, None, Text
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct IIdGenerator
    {
public:
    static const int64_t UNSET_ID = 0ULL;

private: 
    bool m_enabled;

    virtual ECDbTableId _NextTableId() { return UNSET_ID; }
    virtual ECDbColumnId _NextColumnId() { return UNSET_ID; }
    virtual ECDbIndexId _NextIndexId() { return UNSET_ID; }
    virtual ECDbConstraintId _NextConstraintId() { return UNSET_ID; }
    virtual ECDbClassMapId _NextClassMapId() { return UNSET_ID; }
    virtual ECDbPropertyPathId _NextPropertyPathId() { return UNSET_ID; }

protected:
    IIdGenerator() : m_enabled(true) {}

public:
    virtual ~IIdGenerator (){}
    ECDbTableId NextTableId() { return  m_enabled ? _NextTableId() : UNSET_ID; }
    ECDbColumnId NextColumnId() { return  m_enabled ? _NextColumnId() : UNSET_ID; }
    ECDbIndexId NextIndexId() { return  m_enabled ? _NextIndexId() : UNSET_ID; }
    ECDbConstraintId NextConstraintId() { return  m_enabled ? _NextConstraintId() : UNSET_ID; }
    ECDbClassMapId NextClassMapId() { return  m_enabled ? _NextClassMapId() : UNSET_ID; }
    ECDbPropertyPathId NextPropertyPathId() { return  m_enabled ? _NextPropertyPathId() : UNSET_ID; }

    void SetEnabled (bool enabled) { m_enabled = enabled; }

    struct DisableGeneratorScope
        {
        private:
            IIdGenerator& m_ig;
        public:
            DisableGeneratorScope (IIdGenerator& ig)
                :m_ig (ig)
                {
                m_ig.SetEnabled (false);
                }
            ~DisableGeneratorScope (){ m_ig.SetEnabled (true); }
        };
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct NameGenerator
    {
    private:
        int m_uniqueIdGenerator;
        Utf8String m_format;

    public:
        explicit NameGenerator (Utf8CP format = "ecdb_%s")
            :m_format (format), m_uniqueIdGenerator (1) {}

        ~NameGenerator() {}

        void Generate (Utf8StringR generatedName)
            {
            generatedName.clear ();
            generatedName.Sprintf (m_format.c_str (), m_uniqueIdGenerator);
            m_uniqueIdGenerator++;
            }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct StringPool : NonCopyableClass
    {
    private:
        std::set<Utf8CP, CompareUtf8> m_lookUp;

    public:
        StringPool ()
            {
            //Reserve (1024);
            }
        ~StringPool ()
            {
            for (auto str : m_lookUp)
                {
                free (const_cast<Utf8P>(str));
                }

            m_lookUp.clear ();
            }
        Utf8CP Set (Utf8CP str);
        bool Exists (Utf8CP accessString) const;
        Utf8CP Get (Utf8CP str) const;
    };

struct ECDbSqlColumn;
struct ECDbSqlTable;
struct ECDbSQLManager;

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbMapDb : NonCopyableClass
    {
private:
    BeVersion m_version;
    NameGenerator m_nameGenerator;
    std::map<Utf8CP, std::unique_ptr<ECDbSqlTable>, CompareIUtf8> m_tables;
    StringPool m_stringPool;
    ECDbSQLManager& m_sqlManager;

public:
    explicit ECDbMapDb(ECDbSQLManager& manager) : m_version(1, 0), m_nameGenerator("ecdb_%03d"), m_sqlManager(manager) {}
    virtual ~ECDbMapDb() {}

    //! Create a table with a given name or if name is null a name will be generated
    ECDbSqlTable* CreateTable (Utf8CP name, TableType, PersistenceType type = PersistenceType::Persisted , ECDbSqlTable const* baseTable = nullptr);

    ECDbSqlTable* CreateTableForExistingTableMapStrategy (ECDbCR, Utf8CP existingTableName);
    ECDbSqlTable* CreateTableForExistingTableMapStrategy (Utf8CP existingTableName);
    BeVersion const& GetVersion () const { return m_version; }
    //! Find a table with a given name
    ECDbSqlTable const* FindTable (Utf8CP name) const;
    ECDbSqlTable* FindTableP (Utf8CP name) const;
    const std::vector<ECDbSqlTable const*> GetTables () const;
    const std::vector<ECDbSqlTable*> GetTablesR ();

    ECDbSQLManager& GetManager() const { return m_sqlManager; }
    ECDbSQLManager & GetManagerR() { return m_sqlManager; }

    NameGenerator& GetNameGenerator() { return m_nameGenerator; }

    bool IsNameInUse(Utf8CP name) const;

    StringPool const& GetStringPool () const { return m_stringPool; }
    StringPool& GetStringPoolR () { return m_stringPool; }
    BentleyStatus DropTable (Utf8CP name);
    bool IsModified () const;
    void Reset ();
    };



//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlColumn : NonCopyableClass
    {
    enum class Type
        {
        Any = 0,
        Boolean = 1,
        Blob = 2, 
        TimeStamp = 3,
        Real = 4, 
        Integer = 5, 
        Text = 6
        };

    struct Constraint : NonCopyableClass
        {
    public:
        enum class Collation
            {
            Default = 0, // Default is really Binary in sqlite. But we will not provide collation for property to sqlite in this case and assume sqlite default.
            Binary = 1, // Compares string data using memcmp, regardless of text encoding
            NoCase = 2, // The same as binary, except the 26 upper case characters of ASCII are folded to their lower case equivalents before the comparison is performed. Note that only ASCII characters are case folded. SQLite does not attempt to do full UTF case folding due to the size of the tables required.
            RTrim = 3  // The same as binary, except that trailing space characters are ignored.
            };

    private:
        bool m_constraintNotNull : 1;
        bool m_constraintIsUnique : 2;
        Utf8String m_constraintCheck;
        Utf8String m_constraintDefaultValue;
        Collation m_collation;

    public:
        Constraint() : m_constraintNotNull(false), m_constraintIsUnique(false), m_collation(Collation::Default) {}

        bool IsNotNull() const { return m_constraintNotNull; }
        void SetIsNotNull(bool isNotNull) { m_constraintNotNull = isNotNull; }
        bool IsUnique() const { return m_constraintIsUnique; }
        void SetIsUnique(bool isUnique) { m_constraintIsUnique = isUnique; }
        Utf8StringCR GetCheckExpression() const { return m_constraintCheck; }
        void SetCheckExpression(Utf8CP expression) { m_constraintCheck = expression; }
        Utf8StringCR GetDefaultExpression() const { return m_constraintDefaultValue; }
        void SetDefaultExpression(Utf8CP expression) { m_constraintDefaultValue = expression; }           
        Collation GetCollation()  const { return m_collation; }
        void SetCollation(Collation collation) { m_collation = collation; }
        static Utf8CP CollationToString(Collation);
        static bool TryParseCollationString(Collation&, Utf8CP);
        };

    private:
        Type m_type;
        ECDbSqlTable& m_ownerTable;
        Constraint m_constraints;
        Utf8String m_name;
        PersistenceType m_persistenceType;
        ColumnKind m_kind;
        ECDbColumnId m_id;

    public:
        ECDbSqlColumn(Utf8CP name, Type type, ECDbSqlTable& owner, PersistenceType persistenceType, ECDbColumnId id)
            : m_name(name), m_ownerTable(owner), m_type(type), m_persistenceType(persistenceType), m_kind(ColumnKind::DataColumn), m_id(id)
            {}

        ~ECDbSqlColumn() {}

        ECDbColumnId GetId() const { return m_id; }
        void SetId(ECDbColumnId id) { m_id = id; }
        PersistenceType GetPersistenceType() const { return m_persistenceType; }
        Utf8StringCR GetName() const { return m_name; }
        Type GetType() const { return m_type; };
        ECDbSqlTable const& GetTable() const { return m_ownerTable; }
        ECDbSqlTable&  GetTableR() const { return m_ownerTable; }
        Constraint const& GetConstraint() const { return m_constraints; };
        Constraint& GetConstraintR() { return m_constraints; };

        ColumnKind GetKind() const { return m_kind; }
        BentleyStatus SetKind(ColumnKind);
        BentleyStatus AddKind(ColumnKind);

        bool IsReusable() const { return m_type == Type::Any; }
        Utf8String GetFullName() const;
        std::weak_ptr<ECDbSqlColumn> GetWeakPtr() const;

        static Type PrimitiveTypeToColumnType(ECN::PrimitiveType type);
        static bool IsCompatible(Type lhs, Type rhs);
        static Utf8String BuildFullName(Utf8CP table, Utf8CP column);
        static Utf8CP KindToString(ColumnKind);
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlConstraint : NonCopyableClass
    {
    enum class Type
        {
        PrimaryKey,
        ForeignKey
        };

    private:
        Type m_type;
        ECDbSqlTable const& m_table;

    public:
        ECDbSqlConstraint (Type type, ECDbSqlTable const& table) :m_type (type), m_table (table) {}

        virtual ~ECDbSqlConstraint() {}

        Type GetType () const { return m_type; }
        ECDbSqlTable const& GetTable () const { return m_table; }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlPrimaryKeyConstraint : ECDbSqlConstraint
    {
    private:
        std::vector<ECDbSqlColumn const*> m_columns;

    public:
        ECDbSqlPrimaryKeyConstraint (ECDbSqlTable const& table)
            :ECDbSqlConstraint (ECDbSqlConstraint::Type::PrimaryKey, table)
            {}
        virtual ~ECDbSqlPrimaryKeyConstraint (){}
        BentleyStatus Add (Utf8CP columnName);
        BentleyStatus InsertOrReplace (Utf8CP columnName, size_t position);

        BentleyStatus Remove (Utf8CP columnName);
        bool Contains (Utf8CP columnName) const
            {
            return std::find_if (m_columns.begin (), m_columns.end (), [columnName] (ECDbSqlColumn const* column){ return column->GetName ().EqualsI (columnName); }) != m_columns.end ();
            }       
        std::vector<ECDbSqlColumn const*> const& GetColumns () const { return m_columns; }        
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlForeignKeyConstraint : ECDbSqlConstraint
    {
    private:
        ECDbSqlTable const& m_referencedTable;
        std::vector<ECDbSqlColumn const*> m_fkColumns;
        std::vector<ECDbSqlColumn const*> m_referencedTableColumns;
        ForeignKeyActionType m_onDeleteAction;
        ForeignKeyActionType m_onUpdateAction;
        Utf8String m_name;
        ECDbConstraintId m_id;

    public:
        ECDbSqlForeignKeyConstraint(ECDbSqlTable const& fkTable, ECDbSqlTable const& referencedTable, ECDbConstraintId id)
            :ECDbSqlConstraint(ECDbSqlConstraint::Type::ForeignKey, fkTable), m_id(id), m_referencedTable(referencedTable), m_onDeleteAction(ForeignKeyActionType::NotSpecified), m_onUpdateAction(ForeignKeyActionType::NotSpecified)
            {}

        virtual ~ECDbSqlForeignKeyConstraint() {}

        ECDbConstraintId GetId() const { return m_id; }
        void SetId(ECDbConstraintId id) { m_id = id; }
        Utf8StringCR GetName() const { return m_name; }
        void SetName(Utf8CP name) { m_name = name; }

        ForeignKeyActionType GetOnDeleteAction() const { return m_onDeleteAction; }
        ForeignKeyActionType GetOnUpdateAction() const { return m_onUpdateAction; }
        void SetOnDeleteAction(ForeignKeyActionType action) { m_onDeleteAction = action; }
        void SetOnUpdateAction(ForeignKeyActionType action) { m_onUpdateAction = action; }

        std::vector<ECDbSqlColumn const*> const& GetFkColumns() const { return m_fkColumns; }
        std::vector<ECDbSqlColumn const*> const& GetReferencedTableColumns() const { return m_referencedTableColumns; }
        ECDbSqlTable const& GetReferencedTable() const { return m_referencedTable; }
        ECDbSqlTable const& GetForeignKeyTable() const { return GetTable(); }
        bool ContainsInFkColumns(Utf8CP columnName) const;
        bool ContainsInReferencedTableColumns(Utf8CP columnName) const;
        size_t Count() const { return m_fkColumns.size(); }

        bool IsDuplicate() const;
        void RemoveIfDuplicate();
        bool Equals(ECDbSqlForeignKeyConstraint const& rhs) const;

        BentleyStatus Add(Utf8CP fkColumnName, Utf8CP referencedColumnName);
        BentleyStatus Remove(Utf8CP fkColumnName, Utf8CP referencedColumnName);
        BentleyStatus Remove(size_t index);

        static ForeignKeyActionType ToActionType(Utf8CP str);
        static Utf8CP ToSQL(ForeignKeyActionType actionType);
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlTrigger;

struct ECDbSqlTable : NonCopyableClass
    {
public:
    friend ECDbSqlTable* ECDbMapDb::CreateTable (Utf8CP name, TableType, PersistenceType, ECDbSqlTable const*);
    friend ECDbSqlTable* ECDbMapDb::CreateTableForExistingTableMapStrategy (ECDbCR, Utf8CP existingTableName);
    friend ECDbSqlTable* ECDbMapDb::CreateTableForExistingTableMapStrategy (Utf8CP existingTableName);
    friend std::weak_ptr<ECDbSqlColumn> ECDbSqlColumn::GetWeakPtr () const;

    enum class ColumnEvent
        {
        Created,
        Deleted
        };

    struct PersistenceManager : NonCopyableClass
        {
    private:
        ECDbSqlTable& m_table;

        BentleyStatus CreateTriggers(ECDbR, bool failIfExists) const;

    public:
        explicit PersistenceManager (ECDbSqlTable& table) :m_table (table) {}
        ~PersistenceManager (){}

        BentleyStatus Create(ECDbR ecdb) const;
        BentleyStatus CreateOrUpdate(ECDbR ecdb) const;
        bool Exist (ECDbR ecdb) const;
        };

    private:
        ECDbSqlTable const* m_parentOfJoinedTable;
        ECDbMapDb& m_dbDef;
        ECDbTableId m_id;
        Utf8String m_name;
        NameGenerator m_nameGeneratorForColumn;
        TableType m_tableType;
        PersistenceType m_persistenceType;
        std::map<Utf8CP, std::shared_ptr<ECDbSqlColumn>, CompareIUtf8> m_columns;
        std::map<Utf8CP, std::unique_ptr<ECDbSqlTrigger>, CompareIUtf8> m_triggers;
        std::vector<ECDbSqlColumn const*> m_orderedColumns;
        mutable bool m_isClassIdColumnCached;
        mutable ECDbSqlColumn const* m_classIdColumn;
        std::vector<std::unique_ptr<ECDbSqlConstraint>> m_constraints;
        PersistenceManager m_persistenceManager;
        EditHandle m_editInfo;
        std::vector<ECDbSqlTable const*> m_childTables;
        std::vector<std::function<void (ColumnEvent, ECDbSqlColumn&)>> m_columnEvents;
    
        ECDbSqlTable(Utf8CP name, ECDbMapDb& sqlDbDef, ECDbTableId id, PersistenceType type, TableType tableType, ECDbSqlTable const* parentOfJoinedTable)
            : m_dbDef(sqlDbDef), m_id(id), m_name(name), m_nameGeneratorForColumn("sc%02x"), m_persistenceType(type), m_tableType(tableType),
            m_isClassIdColumnCached(false), m_classIdColumn(nullptr), m_persistenceManager(*this), m_parentOfJoinedTable(parentOfJoinedTable)
            {
            BeAssert((tableType == TableType::Joined && parentOfJoinedTable != nullptr) ||
                     (tableType != TableType::Joined && parentOfJoinedTable == nullptr) && "parentOfJoinedTable must be provided for TableType::Joined and must be null for any other TableType.");

            if (tableType == TableType::Joined && parentOfJoinedTable != nullptr)
                const_cast<ECDbSqlTable*>(parentOfJoinedTable)->m_childTables.push_back(this);
            }

        std::weak_ptr<ECDbSqlColumn> GetColumnWeakPtr (Utf8CP name) const;

    public:
        virtual ~ECDbSqlTable() {}

        //!If this is a joined table the method returns the parent of the joined table, aka primary table.
        //!Otherwise the method returns nullptr
        ECDbSqlTable const* GetParentOfJoinedTable() const { return m_parentOfJoinedTable; }

        ECDbTableId GetId () const { return m_id; }
        void SetId (ECDbTableId id) { m_id = id; }
        Utf8StringCR GetName () const { return m_name; }
        PersistenceType GetPersistenceType () const { return m_persistenceType; }
        TableType GetTableType () const { return m_tableType; }
        bool IsOwnedByECDb() const { return m_tableType != TableType::Existing; }
        ECDbMapDb const& GetDbDef () const{ return m_dbDef; }
        ECDbMapDb & GetDbDefR () { return m_dbDef; }
        //! Any type will be mark as reusable column
        ECDbSqlColumn* CreateColumn (Utf8CP name, ECDbSqlColumn::Type type, ColumnKind kind = ColumnKind::DataColumn, PersistenceType persistenceType = PersistenceType::Persisted);
        ECDbSqlColumn* CreateColumn (Utf8CP name, ECDbSqlColumn::Type type, size_t position, ColumnKind kind = ColumnKind::DataColumn, PersistenceType persistenceType = PersistenceType::Persisted);
        std::vector<ECDbSqlTable const*> const& GetChildTables() const {return m_childTables;}
            
        BentleyStatus CreateTrigger(Utf8CP triggerName, ECDbSqlTable& table, Utf8CP condition, Utf8CP body, TriggerType ecsqlType,TriggerSubType triggerSubType);
        std::vector<const ECDbSqlTrigger*> GetTriggers()const;
        ECDbSqlColumn const* FindColumnCP (Utf8CP name) const;
        ECDbSqlColumn* FindColumnP (Utf8CP name) const;
        bool TryGetECClassIdColumn(ECDbSqlColumn const*&) const;
        std::vector<ECDbSqlColumn const*> const& GetColumns () const;
        EditHandle& GetEditHandleR () { return m_editInfo; }
        EditHandle const& GetEditHandle () const { return m_editInfo; }
        ECDbSqlPrimaryKeyConstraint* GetPrimaryKeyConstraint (bool createIfDonotExist = true);
        ECDbSqlForeignKeyConstraint* CreateForeignKeyConstraint (ECDbSqlTable const& referencedTable);
        BentleyStatus RemoveConstraint(ECDbSqlConstraint const& constraint);
        std::vector<ECDbSqlConstraint const*> GetConstraints () const;   
        BentleyStatus GetFilteredColumnList (std::vector<ECDbSqlColumn const*>& columns, PersistenceType persistenceType) const;
        BentleyStatus GetFilteredColumnList (std::vector<ECDbSqlColumn const*>& columns, ColumnKind knowColumnIds) const;
        ECDbSqlColumn const* GetFilteredColumnFirst (ColumnKind knowColumnIds) const;
        bool DeleteColumn (Utf8CP name);
        BentleyStatus FinishEditing ();
        //! temp method
        void AddColumnEventHandler (std::function<void (ColumnEvent, ECDbSqlColumn&)> columnEventHandler){ m_columnEvents.push_back(columnEventHandler); }
        PersistenceManager const& GetPersistenceManager () const { return m_persistenceManager; }
        bool IsValid () const { return m_columns.size () > 0; }
        size_t IndexOf (ECDbSqlColumn const& column) const;
    };

//======================================================================================
// @bsiclass                                        muhammad.zaighum        01/2015
//======================================================================================
struct ECDbSqlTrigger : NonCopyableClass
    {
    public:
        
        friend BentleyStatus ECDbSqlTable::CreateTrigger(Utf8CP triggerName, ECDbSqlTable& table, Utf8CP condition, Utf8CP body, TriggerType ecsqlType,TriggerSubType triggerSubType);
    private:
        Utf8String m_triggerName;
        ECDbSqlTable& m_table;
        Utf8String m_condition;
        Utf8String m_body;
        TriggerType m_triggerType;
        TriggerSubType m_triggerSubType;
        ECDbSqlTrigger(Utf8CP triggerName, ECDbSqlTable& table, Utf8CP condition, Utf8CP body, TriggerType ecsqlType, TriggerSubType triggerSubType) : m_triggerName(triggerName), m_table(table), m_condition(condition), m_body(body), m_triggerType(ecsqlType), m_triggerSubType(triggerSubType)
            {}
        explicit ECDbSqlTrigger(ECDbSqlTable& table) : m_table(table) {}

    public:
        Utf8String GetName()const { return m_triggerName; }
        Utf8String GetCondition()const { return m_condition; }
        ECDbSqlTable& GetTable()const { return m_table; }
        Utf8String GetBody()const { return m_body; }
        TriggerType GetType()const{ return m_triggerType; }
        TriggerSubType GetSubType()const{ return m_triggerSubType; }

        BentleyStatus CreateInDb(ECDbR) const;
        bool ExistsInDb(ECDbR) const;
    };
//======================================================================================
// @bsiclass                                                 Affan.Khan         10/2014
//======================================================================================
struct DDLGenerator
    {
    enum class CreateOption
        {
        Create,
        CreateOrReplace
        };

private:
    private:
        DDLGenerator();
        ~DDLGenerator();

    static Utf8String GetColumnList (std::vector<ECDbSqlColumn const*> const&);
    static Utf8String GetForeignKeyConstraintDDL (ECDbSqlForeignKeyConstraint const&);
    static Utf8String GetPrimarykeyConstraintDDL (ECDbSqlPrimaryKeyConstraint const&);
    static Utf8String GetConstraintsDDL (std::vector<ECDbSqlConstraint const*> const&);
    static Utf8String GetColumnsDDL (std::vector<ECDbSqlColumn const*>);
    static Utf8String GetColumnDDL (ECDbSqlColumn const&);
public:
    static Utf8String GetCreateTableDDL(ECDbSqlTable const&, CreateOption);
    static Utf8String GetCreateTriggerDDL(ECDbSqlTrigger const&);

    static BentleyStatus AddColumns(ECDbR, ECDbSqlTable const&, std::vector<Utf8CP> const& newColumns);
    static BentleyStatus CopyRows(ECDbR, Utf8CP sourceTable, bvector<Utf8String>& sourceColumns, Utf8CP targetTable, bvector<Utf8String>& targetColumns);

    static Utf8CP ColumnTypeToSql(ECDbSqlColumn::Type);
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbPropertyPath : NonCopyableClass
    {
    private:
        ECDbPropertyPathId m_pathId;
        ECN::ECPropertyId m_rootPropertyId;
        Utf8String m_accessString;
        int Compare (ECDbPropertyPath const& rhs) const
            {
            if (m_pathId < rhs.m_pathId)
                return -1;
            else if (m_pathId > rhs.m_pathId)
                return 1;
            else if (m_rootPropertyId < rhs.m_rootPropertyId)
                return -1;
            else if (m_rootPropertyId > rhs.m_rootPropertyId)
                return 1;
            else if (m_accessString < rhs.m_accessString)
                return -1;
            else if (m_accessString > rhs.m_accessString)
                return 1;

            return 0;
            }
    public:
        ECDbPropertyPath (ECDbPropertyPathId id, ECN::ECPropertyId rootPropertyId, Utf8CP accessString)
            :m_pathId (id), m_rootPropertyId (rootPropertyId), m_accessString (accessString)
            {}
        ~ECDbPropertyPath (){}
        ECDbPropertyPathId GetId () const { return m_pathId; }
        ECN::ECPropertyId GetRootPropertyId () const { return m_rootPropertyId; }
        Utf8String const& GetAccessString () const { return m_accessString; }
        const Utf8String  GetName() const
            {
            auto i = m_accessString.find('.');
            if (i == Utf8String::npos)
                return m_accessString;

            return m_accessString.substr(0, i);
            }
        bool operator == (ECDbPropertyPath const& rhs) const
            {
            return Compare (rhs) == 0;
            }
        bool operator != (ECDbPropertyPath const& rhs) const
            {
            return Compare (rhs) != 0;
            }

        bool operator < (ECDbPropertyPath const& rhs) const
            {
            return Compare (rhs) < 0;
            }
        bool operator > (ECDbPropertyPath const& rhs) const
            {
            return Compare (rhs) > 0;
            }
    };

struct ECDbClassMapInfo;
struct ECDbMapStorage;
//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbPropertyMapInfo : NonCopyableClass
    {
private:
    ECDbClassMapInfo const&  m_classMap;
    ECDbPropertyPath const& m_propertyPath;
    std::vector<ECDbSqlColumn const*> m_columns;
public:
    ECDbPropertyMapInfo (ECDbClassMapInfo const& classMap, ECDbPropertyPath const& propertyPath) :m_classMap (classMap), m_propertyPath (propertyPath) { }
    std::vector<ECDbSqlColumn const*>& GetColumnsR(){return m_columns;}
    std::vector<ECDbSqlColumn const*> const& GetColumns() const {return m_columns;}
    ECDbPropertyPath const& GetPropertyPath () const { return m_propertyPath; }
    ECDbClassMapInfo const& GetClassMap () const { return m_classMap; }
    ECDbSqlColumn const* ExpectingSingleColumn() const { BeAssert(m_columns.size() == 1 && "Expecting exactly one column"); return m_columns.front(); }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbClassMapInfo : NonCopyableClass
    {
    private:
        ECDbClassMapId m_id;
        ECN::ECClassId m_ecClassId;
        mutable ECDbClassMapInfo const* m_ecBaseClassMap;
        ECDbClassMapId m_ecBaseClassMapId;
        ECDbMapStrategy m_mapStrategy;
        std::vector<std::unique_ptr<ECDbPropertyMapInfo>> m_localPropertyMaps;
        std::vector<ECDbClassMapInfo*> m_childClassMaps;
        ECDbMapStorage const& m_map;

    protected:
        const std::map<Utf8CP, ECDbPropertyMapInfo const*, CompareIUtf8> GetPropertyMapByColumnName (bool onlyLocal) const;
        void GetPropertyMaps (std::vector<ECDbPropertyMapInfo const*>& propertyMaps, bool onlyLocal) const;

    public:
        ECDbClassMapInfo (ECDbMapStorage const& map, ECDbClassMapId id, ECN::ECClassId classId, ECDbMapStrategy mapStrategy, ECDbClassMapId baseClassMap = 0LL)
            :m_map (map), m_id (id), m_ecClassId (classId), m_mapStrategy (mapStrategy), m_ecBaseClassMap (nullptr), m_ecBaseClassMapId (baseClassMap)
            {}

        ECDbMapStorage const& GetMapStorage () const{ return m_map; }
        ECDbClassMapId GetId () const { return m_id; }
        ECN::ECClassId GetClassId () const { return m_ecClassId; }
        const std::vector<ECDbPropertyMapInfo const*>  GetPropertyMaps (bool onlyLocal) const;
        ECDbClassMapInfo const*  GetBaseClassMap () const;
        ECDbMapStrategy const& GetMapStrategy () const { return m_mapStrategy; }
        std::vector<ECDbClassMapInfo*> const& GetChildren () const { return m_childClassMaps; }
        ECDbPropertyMapInfo const * FindPropertyMap (Utf8CP columnName) const;
        ECDbPropertyMapInfo const* FindPropertyMap (ECN::ECPropertyId rootPropertyId, Utf8CP accessString) const;
        ECDbPropertyMapInfo const* FindPropertyMapByAccessString(Utf8CP accessString) const;
        ECDbPropertyMapInfo* CreatePropertyMap (ECDbPropertyPath const& propertyPath);
        ECDbPropertyMapInfo* CreatePropertyMap (ECN::ECPropertyId rootPropertyId, Utf8CP accessString, std::vector<ECDbSqlColumn const*> const& columns);
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbMapStorage
    {
    private:

        const Utf8CP Sql_InsertPropertyPath = "INSERT OR REPLACE INTO ec_PropertyPath (Id, RootPropertyId, AccessString) VALUES (?,?,?)";
        const Utf8CP Sql_InsertClassMap = "INSERT INTO ec_ClassMap(Id, ParentId, ClassId, MapStrategy, MapStrategyOptions, MapStrategyAppliesToSubclasses) VALUES (?,?,?,?,?,?)";
        const Utf8CP Sql_DeleteClassMap = "DELETE FROM ec_ClassMap WHERE Id = ?";

        const Utf8CP Sql_InsertPropertyMap = "INSERT OR REPLACE INTO ec_PropertyMap (ClassMapId, PropertyPathId, ColumnId) VALUES (?,?,?)";
        const Utf8CP Sql_SelectPropertyPath = "SELECT Id, RootPropertyId, AccessString FROM ec_PropertyPath";
        const Utf8CP Sql_SelectClassMap = "SELECT Id, ParentId, ClassId, MapStrategy, MapStrategyOptions, MapStrategyAppliesToSubclasses FROM ec_ClassMap ORDER BY Id, ParentId";
        const Utf8CP Sql_SelectPropertyMap = "SELECT PropertyPathId, T.Name TableName, C.Name ColumnName FROM ec_PropertyMap P INNER JOIN ec_Column C ON C.Id = P.ColumnId INNER JOIN ec_Table T ON T.Id = C.TableId WHERE P.ClassMapId = ?";
        enum class StatementType
            {
            SqlInsertPropertyPath,
            SqlInsertClassMap,
            SqlInsertPropertyMap,
            SqlSelectPropertyPath,
            SqlSelectClassMap,
            SqlSelectPropertyMap,
            SqlDeleteClassMap,
            };


        std::map<StatementType, std::unique_ptr<Statement>>  m_statementCache;
        mutable std::map<ECDbPropertyPathId, std::unique_ptr<ECDbPropertyPath>> m_propertyPaths;
        mutable std::map<ECN::ECPropertyId, std::map<Utf8CP, ECDbPropertyPath*, CompareUtf8>> m_propertyPathByPropertyId;
        mutable std::map<ECDbClassMapId, std::unique_ptr<ECDbClassMapInfo>> m_classMaps;
        mutable std::map<ECN::ECClassId, std::vector<ECDbClassMapInfo const*>> m_classMapByClassId;

        ECDbSQLManager& m_manager;
    private:
        ECDbPropertyPath* Set(std::unique_ptr<ECDbPropertyPath> propertyPath) const;
        ECDbClassMapInfo* Set(std::unique_ptr<ECDbClassMapInfo> classMap) const;
        CachedStatementPtr GetStatement(StatementType type) const;
        DbResult InsertOrReplace() const;
        DbResult InsertPropertyMap(ECDbPropertyMapInfo const& o) const;
        DbResult InsertClassMap(ECDbClassMapInfo const& o) const;
        DbResult InsertPropertyPath(ECDbPropertyPath const& o) const;
        DbResult Read() const;
        DbResult ReadPropertyMap(ECDbClassMapInfo&) const;
        DbResult ReadClassMaps() const;
        DbResult ReadPropertyPaths() const;

    public:
        explicit ECDbMapStorage(ECDbSQLManager& manager) :m_manager(manager) {}
        ECDbPropertyPath const * FindPropertyPath(ECN::ECPropertyId rootPropertyId, Utf8CP accessString) const;
        ECDbPropertyPath const* FindPropertyPath(ECDbPropertyPathId propertyPathId) const;
        ECDbClassMapInfo const* FindClassMap(ECDbClassMapId id) const;
        std::vector<ECDbClassMapInfo const*> const* FindClassMapsByClassId(ECN::ECClassId id) const;

        ECDbPropertyPath* CreatePropertyPath(ECN::ECPropertyId rootPropertyId, Utf8CP accessString) const;
        ECDbClassMapInfo* CreateClassMap(ECN::ECClassId classId, ECDbMapStrategy const& mapStrategy, ECDbClassMapId baseClassMapId = 0LL) const;

        BentleyStatus Load() const { return Read() != BE_SQLITE_OK ? ERROR : SUCCESS; }
        BentleyStatus Save() const { return InsertOrReplace() != BE_SQLITE_OK ? ERROR : SUCCESS; }
        void Reset() const;
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         01/2015
//======================================================================================
struct ECDbBriefcaseBasedId : IIdGenerator
    {
private:
    ECDbR m_ecdb;

    virtual ECDbTableId _NextTableId () override;
    virtual ECDbColumnId _NextColumnId () override;
    virtual ECDbIndexId _NextIndexId () override;
    virtual ECDbConstraintId _NextConstraintId () override;
    virtual ECDbClassMapId _NextClassMapId () override;
    virtual ECDbPropertyPathId _NextPropertyPathId () override;

public:
    explicit ECDbBriefcaseBasedId (ECDbR ecdb) :IIdGenerator (), m_ecdb (ecdb) {}
    virtual ~ECDbBriefcaseBasedId () {}
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         01/2015
//======================================================================================
struct ECDbSqlPersistence : NonCopyableClass
    {
    private:
        const Utf8CP Sql_InsertTable = "INSERT OR REPLACE INTO ec_Table (Id, Name, Type, IsVirtual, BaseTableId) VALUES (?, ?, ?, ?, ?)";
        const Utf8CP Sql_InsertColumn = "INSERT OR REPLACE INTO ec_Column (Id, TableId, Name, Type, IsVirtual, Ordinal, NotNullConstraint, UniqueConstraint, CheckConstraint, DefaultConstraint, CollationConstraint, OrdinalInPrimaryKey, ColumnKind) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        const Utf8CP Sql_InsertForeignKey = "INSERT OR REPLACE INTO ec_ForeignKey (Id, TableId, ReferencedTableId, Name, OnDelete, OnUpdate) VALUES (?, ?, ?, ?, ?, ?)";
        const Utf8CP Sql_InsertForeignKeyColumn = "INSERT OR REPLACE INTO ec_ForeignKeyColumn (ForeignKeyId, ColumnId, ReferencedColumnId, Ordinal) VALUES (?, ?, ?, ?)";
        const Utf8CP Sql_SelectTable = "SELECT A.Id, A.Name, A.Type, A.IsVirtual,  B.Name BaseTableName FROM ec_Table A LEFT JOIN ec_Table B ON A.BaseTableId = B.Id ORDER BY A.BaseTableId";
        const Utf8CP Sql_SelectColumn = "SELECT Id, Name, Type, IsVirtual, NotNullConstraint, UniqueConstraint, CheckConstraint, DefaultConstraint, CollationConstraint, OrdinalInPrimaryKey, ColumnKind FROM ec_Column WHERE TableId = ? ORDER BY Ordinal";
        const Utf8CP Sql_SelectForeignKey = "SELECT F.Id, R.Name, F.Name, F.OnDelete, F.OnUpdate FROM ec_ForeignKey F INNER JOIN ec_Table R ON R.Id = F.ReferencedTableId WHERE F.TableId = ?";
        const Utf8CP Sql_SelectForeignKeyColumn = "SELECT A.Name, B.Name FROM ec_ForeignKeyColumn F INNER JOIN ec_Column A ON F.ColumnId = A.Id INNER JOIN ec_Column B ON F.ReferencedColumnId = B.Id  WHERE F.ForeignKeyId = ? ORDER BY F.Ordinal";

        enum class StatementType
            {
            SqlInsertTable,
            SqlInsertColumn,
            SqlInsertForeignKey,
            SqlInsertForeignKeyColumn,
            SqlSelectTable,
            SqlSelectColumn,
            SqlSelectForeignKey,
            SqlSelectForeignKeyColumn
            };

    private:
        ECDb& m_ecdb;

    private:

        DbResult ReadTables (ECDbMapDb&) const;
        DbResult ReadTable (Statement&, ECDbMapDb&) const;
        DbResult ReadColumns (ECDbSqlTable&) const;
        DbResult ReadForeignKeys (ECDbMapDb&) const;
        DbResult ReadColumn (Statement&, ECDbSqlTable&, std::map<size_t, ECDbSqlColumn const*>& primaryKeys) const;
        DbResult ReadForeignKeys (ECDbSqlTable&) const;
        DbResult ReadForeignKey (Statement&, ECDbSqlTable&) const;
        DbResult InsertTable (ECDbSqlTable const&) const;
        DbResult InsertColumn (ECDbSqlColumn const&, int primaryKeyOrdianal) const;
        DbResult InsertConstraint (ECDbSqlConstraint const&) const;
        DbResult InsertForeignKey (ECDbSqlForeignKeyConstraint const&) const;

    public:
        explicit ECDbSqlPersistence (ECDbR ecdb):m_ecdb (ecdb) {}
        ~ECDbSqlPersistence (){};
        DbResult Read (ECDbMapDb&) const;
        DbResult Insert (ECDbMapDb const&) const;

        CachedStatementPtr GetStatement(StatementType) const;
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSQLManager : public NonCopyableClass
    {
    private:
        mutable ECDbMapDb m_defaultDb;
        ECDbR m_ecdb;
        mutable ECDbSqlTable* m_nullTable;
        ECDbSqlPersistence m_persistence;
        ECDbBriefcaseBasedId m_idGenerator;
        ECDbMapStorage m_mapStorage;
        mutable bool m_loaded;

        void SetupNullTable ();
    public:
        explicit ECDbSQLManager (ECDbR ecdb);
        ~ECDbSQLManager (){}
        ECDbR GetECDbR () { return m_ecdb; }
        ECDbCR GetECDb () const{ return m_ecdb; }
        ECDbMapDb const& GetDbSchema () const { return m_defaultDb; }
        ECDbMapDb& GetDbSchemaR() const { return m_defaultDb; }
        BentleyStatus Load () const;
        BentleyStatus Save () const;
        ECDbSqlTable const* GetNullTable () const;
        bool IsNullTable (ECDbSqlTable const& table) const { return &table == GetNullTable (); }
        bool IsTableChanged (ECDbSqlTable const& table) const;
        ECDbSqlPersistence const& GetPersistence() const { return m_persistence; }
        ECDbMapStorage const& GetMapStorage () const { return m_mapStorage; }
        void Reset () const;
        bool IsLoaded () const { return m_loaded; }
        IIdGenerator& GetIdGenerator () {return m_idGenerator;}
    };

//=================================================================================
// @bsiclass                                                     Affan.Khan      11/2011
//+===============+===============+===============+===============+===============+======
struct DbMetaDataHelper
    {
    enum class ObjectType
        {
        None,
        Table,
        View,
        Index,
        };
    static ObjectType GetObjectType(Db& db, Utf8CP name)
        {
        BeSQLite::CachedStatementPtr stmt;
        db.GetCachedStatement(stmt, "SELECT type FROM sqlite_master WHERE name =?");
        stmt->BindText(1, name, BeSQLite::Statement::MakeCopy::No);
        if (stmt->Step() == BE_SQLITE_ROW)
            {
            if (BeStringUtilities::Stricmp(stmt->GetValueText(0), "table") == 0)
                return ObjectType::Table;
            if (BeStringUtilities::Stricmp(stmt->GetValueText(0), "view") == 0)
                return ObjectType::View;
            if (BeStringUtilities::Stricmp(stmt->GetValueText(0), "index") == 0)
                return ObjectType::Index;
            }
        return ObjectType::None;
        }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
