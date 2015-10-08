/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSql.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"
#include <BeXml/BeXml.h>
#include <unordered_map>
#include <unordered_set>
#include "BeRepositoryBasedIdSequence.h"
#include "MapStrategy.h"
#include "ECSql/NativeSqlBuilder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECDBSQL_VERSION_MAJOR 1
#define ECDBSQL_VERSION_MINOR 0
#define ECDBSQL_NULLTABLENAME "ECDbNotMapped"
#define ECDBSQL_PROPERTYNAME "ECDbSqlSchema"
#define ECDBSQL_NAMESPACE "ECDb"
#define ECDBSQL_SCHEMA_ID "ECDbPersistence"

typedef int64_t ECDbTableId;
typedef int64_t ECDbColumnId;
typedef int64_t ECDbIndexId;
typedef int64_t ECDbConstraintId;
typedef int64_t ECDbPropertyPathId;
typedef int64_t ECDbClassMapId;

//!TODO This should replace int in UserData in column
enum class ECDbKnownColumns
    {
    Unknown = 0x0U, //! Not known to ECDb or user define columns
    ECInstanceId = 0x1U, //! ECInstanceId system column also primary key of the table
    ECClassId = 0x2U, //! ECClassId system column. Use if more then on classes is mapped to this table
    ParentECInstanceId = 0x4U, //! ParentECInstanceId column used in struct array
    ECPropertyPathId = 0x8U, //! ECPropertyPathId column used in struct array
    ECArrayIndex = 0x10U, //! ECArrayIndex column used in struct array
    SourceECInstanceId = 0x20U,
    SourceECClassId = 0x40U,
    TargetECInstanceId = 0x80U,
    TargetECClassId = 0x100U,
    DataColumn = 0x200U, //! Data column defined by none key column in ECClass
    //Following is helper group for search operation. There cannot be a column with OR'ed flags
    GroupSourceECInstanceKey = SourceECInstanceId | SourceECClassId,
    GroupTargetECInstanceKey = TargetECInstanceId | TargetECClassId,
    GroupForeignKeys = GroupSourceECInstanceKey | GroupTargetECInstanceKey,
    GroupECInstanceKey = ECInstanceId | ECClassId, //! Group key to identify ECInstance
    GroupECStructKey = GroupECInstanceKey | ParentECInstanceId | ECPropertyPathId | ECArrayIndex, //! Group key to identify ECStruct instance
    GroupSystemColumns = GroupECStructKey
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

enum class OwnerType
    {
    ECDb, //! owned by ECDb.
    ExistingTable //! existing table
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
struct ECDbSqlIndex;
struct ECDbVersion
    {
    private:
        uint32_t m_versionMajor, m_versionMinor;
    public:
        explicit ECDbVersion (uint32_t versionMajor, uint32_t versionMinor)
            : m_versionMajor (versionMajor), m_versionMinor (versionMinor)
            {
            }
        explicit ECDbVersion (uint32_t versionMajor)
            : m_versionMajor (versionMajor), m_versionMinor (0)
            {
            }
        explicit ECDbVersion ()
            : m_versionMajor (1), m_versionMinor (0)
            {
            }
        uint32_t GetVersionMajor () const { return m_versionMajor; }
        uint32_t GetVersionMinor () const{ return m_versionMinor; }
        Utf8String ToString () const 
            {
            Utf8String str;
            str.Sprintf ("%d.%d", m_versionMajor, m_versionMinor);
            return str;
            }
        static ECDbVersion Parse (Utf8CP version) 
            {
            ECDbVersion ver;
            sscanf (version, "%d.%d", &ver.m_versionMajor, &ver.m_versionMinor);
            return ver;
            }
    };
struct ECDbSQLManager;
//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlDb : NonCopyableClass
    {
private:
    ECDbVersion m_version;
    NameGenerator m_nameGenerator;
    std::map<Utf8CP, std::unique_ptr<ECDbSqlTable>, CompareIUtf8> m_tables;
    std::map<Utf8CP, std::unique_ptr<ECDbSqlIndex>, CompareIUtf8> m_indexes;
    StringPool m_stringPool;
    ECDbSQLManager& m_sqlManager;


public:
    explicit ECDbSqlDb(ECDbSQLManager& manager) : m_version(ECDbVersion(1, 0)), m_nameGenerator("ecdb_%03d"), m_sqlManager(manager) {}
    virtual ~ECDbSqlDb() {}

    bool HasObject(Utf8CP name);

    //! Create a table with a given name or if name is null a name will be generated
    ECDbSqlTable* CreateTable (Utf8CP name, PersistenceType type = PersistenceType::Persisted);
    ECDbSqlTable* CreateTableForExistingTableMapStrategy (ECDbCR, Utf8CP existingTableName);
    ECDbSqlTable* CreateTableForExistingTableMapStrategy (Utf8CP existingTableName);
    ECDbVersion const& GetVersion () const { return m_version; }
    //! Find a table with a given name
    ECDbSqlTable const* FindTable (Utf8CP name) const;
    ECDbSqlTable* FindTableP (Utf8CP name) const;
    ECDbSqlIndex const* FindIndex (Utf8CP name) const;
    ECDbSqlIndex* FindIndexP (Utf8CP name) ;

    ECDbSqlIndex* AddIndex(std::unique_ptr<ECDbSqlIndex>&);

    const std::vector<ECDbSqlIndex const*> GetIndexes () const;
    std::vector<ECDbSqlIndex*> GetIndexesR ();

    BentleyStatus CreateOrUpdateIndices() const;

    const std::vector<ECDbSqlTable const*> GetTables () const;
    const std::vector<ECDbSqlTable*> GetTablesR ();

    ECDbSQLManager& GetManager() const { return m_sqlManager; }
    ECDbSQLManager & GetManagerR() { return m_sqlManager; }

    NameGenerator& GetNameGenerator() { return m_nameGenerator; }

    StringPool const& GetStringPool () const { return m_stringPool; }
    StringPool& GetStringPoolR ()  { return m_stringPool; }
    BentleyStatus DropIndex (Utf8CP name);
    BentleyStatus DropTable (Utf8CP name);
    bool IsModified () const;
    void Reset ();        
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlIndex : NonCopyableClass
    {
public:
    struct PersistenceManager : NonCopyableClass
        {
    private:
        ECDbSqlIndex& m_index;

    public:
        explicit PersistenceManager (ECDbSqlIndex& index) :m_index(index) {}
        ~PersistenceManager (){}

        ECDbSqlIndex const& GetIndex () const { return m_index; }
        BentleyStatus Create(ECDbR) const;
        BentleyStatus Drop(ECDbR) const;
        bool Exists (ECDbR) const;
        };

private:
    ECDbIndexId m_id;
    Utf8String m_name;
    ECDbSqlTable& m_table;
    std::vector<ECDbSqlColumn const*> m_columns;
    bool m_isUnique;
    ECN::ECClassId m_classId;
    Utf8String m_additionalWhereExpression;
    PersistenceManager m_persistenceManager;

    BentleyStatus BuildCreateDdl(NativeSqlBuilder&, ECDbCR) const;

public:
    ECDbSqlIndex(ECDbIndexId id, ECDbSqlTable& table, Utf8CP name, bool isUnique, ECN::ECClassId classId)
        :m_id(id), m_name(name), m_table(table), m_isUnique(isUnique), m_classId(classId), m_persistenceManager(*this)  {}

    ECDbIndexId GetId() const { BeAssert(m_id != IIdGenerator::UNSET_ID); return m_id; }

    Utf8StringCR GetName () const { return m_name; }
    ECDbSqlTable const& GetTable () const { return m_table; }
    ECDbSqlTable & GetTableR () { return m_table; }
    bool GetIsUnique () const { return m_isUnique; }
    std::vector<ECDbSqlColumn const*> const& GetColumns () const { return m_columns; }
    bool HasClassId() const { return m_classId != ECN::ECClass::UNSET_ECCLASSID; }
    ECN::ECClassId GetClassId() const { return m_classId; }
    void SetAdditionalWhereExpression (Utf8CP expression) { m_additionalWhereExpression = expression; }
    Utf8StringCR GetAdditionalWhereExpression () const { return m_additionalWhereExpression; }
    bool Contains (Utf8CP column) const;
    BentleyStatus Add (Utf8CP column);
    BentleyStatus Remove (Utf8CP column);
    BentleyStatus Drop ();
    bool IsValid () const { return !m_columns.empty (); }
    PersistenceManager const& GetPersistenceManager () const { return m_persistenceManager; }
    PersistenceManager & GetPersistenceManagerR () { return m_persistenceManager; }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlColumn : NonCopyableClass
    {
    enum class Type
        {
        Integer, Long, Double, DateTime, Binary, Boolean, String, Any
        };
    struct Constraint : NonCopyableClass
        {

        enum class Collation
            {
            Default, // Default is really Binary in sqlite. But we will not provide collation for property to sqlite in this case and assume sqlite default.
            Binary, // Compares string data using memcmp, regardless of text encoding
            NoCase, // The same as binary, except the 26 upper case characters of ASCII are folded to their lower case equivalents before the comparison is performed. Note that only ASCII characters are case folded. SQLite does not attempt to do full UTF case folding due to the size of the tables required.
            RTrim,  // The same as binary, except that trailing space characters are ignored.
            };

        private:
            bool m_constraintNotNull : 1;
            bool m_constraintIsUnique : 2;
            Utf8String m_constraintCheck;
            Utf8String m_constraintDefaultValue;
            Collation m_collation;
        public:
            Constraint () :m_constraintNotNull (false), m_constraintIsUnique (false), m_collation (Collation::Default)
                {}
            bool IsNotNull () const { return m_constraintNotNull; }
            bool IsUnique () const { return m_constraintIsUnique; }
            Utf8StringCR GetCheckExpression () const { return m_constraintCheck; }
            Utf8StringCR GetDefaultExpression () const { return m_constraintDefaultValue; }
            void SetIsNotNull (bool isNotNull) { m_constraintNotNull = isNotNull; }
            void SetIsUnique (bool isUnique) { m_constraintIsUnique = isUnique; }
            void SetCheckExpression (Utf8CP expression) { m_constraintCheck = expression; }
            void SetDefaultExpression (Utf8CP expression) { m_constraintDefaultValue = expression; }
            Collation GetCollation ()  const { return m_collation; }
            void SetCollation(Collation collation) { m_collation = collation; }
            static Utf8CP CollationToString (Collation);
            static bool TryParseCollationString(Collation&, Utf8CP);
        };

    private:
        static const std::map<ECDbKnownColumns, Utf8CP> s_knownColumnNames;

        Type m_type;
        ECDbSqlTable& m_ownerTable;
        Constraint m_constraints;
        Utf8String m_name;
        PersistenceType m_persistenceType;
        ECDbKnownColumns m_knowColumnId;
        ECDbColumnId m_id;
        bool m_isShared;

    public:
        ECDbSqlColumn (Utf8CP name, Type type, ECDbSqlTable& owner, PersistenceType persistenceType, ECDbColumnId id)
            : m_name (name), m_ownerTable (owner), m_type (type), m_persistenceType (persistenceType), m_knowColumnId (ECDbKnownColumns::DataColumn), m_id (id), m_isShared(false) {}

        ECDbColumnId GetId () const { return m_id; }
        void SetId (ECDbColumnId id) { m_id = id; }
        PersistenceType GetPersistenceType () const { return m_persistenceType; }
        Utf8StringCR GetName () const { return m_name; }
        Type GetType () const { return m_type; };
        bool IsShared() const { return m_isShared; }
        void SetIsShared(bool isShared) { m_isShared = isShared; }
        ECDbSqlTable const& GetTable () const { return m_ownerTable; }
        ECDbSqlTable&  GetTableR ()  { return m_ownerTable; }
        Constraint const& GetConstraint () const { return m_constraints; };
        Constraint& GetConstraintR ()  { return m_constraints; };
        bool IsReusable () const { return m_type == Type::Any; }
        virtual ~ECDbSqlColumn () {}
        static Type StringToType (Utf8CP typeName);
        static Utf8CP TypeToString (Type type);
        BentleyStatus SetKnownColumnId (ECDbKnownColumns knownColumnId);
        ECDbKnownColumns GetKnownColumnId() const { return m_knowColumnId; }
        const Utf8String GetFullName () const;
        std::weak_ptr<ECDbSqlColumn> GetWeakPtr () const;
        static const Utf8String BuildFullName (Utf8CP table, Utf8CP column);
        static Utf8CP KnownColumnToString (ECDbKnownColumns columnId);
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
    enum class ActionType
        {
        NotSpecified,
        Cascade,
        NoAction,
        SetNull,
        SetDefault,
        Restrict,
        };

    private:
        ECDbSqlTable const& m_targetTable;
        std::vector<ECDbSqlColumn const*> m_sourceColumns;
        std::vector<ECDbSqlColumn const*> m_targetColumns;
        ActionType m_onDeleteAction;
        ActionType m_onUpdateAction;
        Utf8String m_name;
        ECDbConstraintId m_id;
    public:
        ECDbSqlForeignKeyConstraint (ECDbSqlTable const& sourceTable, ECDbSqlTable const& targetTable, ECDbConstraintId id)
            :ECDbSqlConstraint (ECDbSqlConstraint::Type::ForeignKey, sourceTable), m_id (id), m_targetTable (targetTable), m_onDeleteAction (ActionType::NotSpecified), m_onUpdateAction (ActionType::NotSpecified)
            {}

        ECDbConstraintId GetId () const { return m_id; }
        void SetId (ECDbConstraintId id) { m_id = id; }
        Utf8StringCR GetName () const { return m_name; }
        void SetName (Utf8CP name) { m_name = name; }
        void SetOnDeleteAction (ActionType action) { m_onDeleteAction = action; }
        void SetOnUpdateAction (ActionType action) { m_onUpdateAction = action; }

        ActionType GetOnDeleteAction () const { return m_onDeleteAction; }
        ActionType GetOnUpdateAction () const { return m_onUpdateAction; }

        BentleyStatus Add (Utf8CP sourceColumn, Utf8CP targetColumn);
        BentleyStatus Remove (size_t index);
        std::vector<ECDbSqlColumn const*> const& GetSourceColumns () const { return m_sourceColumns; }
        std::vector<ECDbSqlColumn const*> const& GetTargetColumns () const { return m_targetColumns; }
        ECDbSqlTable const& GetTargetTable () const { return m_targetTable; }
        ECDbSqlTable const& GetSourceTable () const { return GetTable (); }
        bool ContainsInSource (Utf8CP columnName) const;
        bool ContainsInTarget (Utf8CP columnName) const;
        BentleyStatus Remove (Utf8CP sourceColumn, Utf8CP targetColumn);
        size_t Count () const { return m_targetColumns.size (); }
        static ActionType ToActionType(Utf8CP str);
        static Utf8CP ToSQL(ActionType actionType);
        virtual ~ECDbSqlForeignKeyConstraint (){}
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlTrigger;
struct ECDbSqlTable : NonCopyableClass
    {
    enum class ColumnEvent
        {
        Created,
        Deleted
        };
    friend ECDbSqlTable* ECDbSqlDb::CreateTable (Utf8CP name, PersistenceType type);
    friend ECDbSqlTable* ECDbSqlDb::CreateTableForExistingTableMapStrategy (ECDbCR ecdb, Utf8CP existingTableName);
    friend ECDbSqlTable* ECDbSqlDb::CreateTableForExistingTableMapStrategy (Utf8CP existingTableName);
    friend std::weak_ptr<ECDbSqlColumn> ECDbSqlColumn::GetWeakPtr () const;
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
        BentleyStatus Drop(ECDbR ecdb) const;
        bool Exist (ECDbR ecdb) const;
        };

    private:
        ECDbSqlDb& m_dbDef;
        ECDbTableId m_id;
        Utf8String m_name;
        NameGenerator m_nameGeneratorForColumn;
        PersistenceType m_type;
        OwnerType m_ownerType;
        std::map<Utf8CP, std::shared_ptr<ECDbSqlColumn>, CompareIUtf8> m_columns;
        std::map<Utf8CP, std::unique_ptr<ECDbSqlTrigger>, CompareIUtf8> m_trigers;
        std::vector<ECDbSqlColumn const*> m_orderedColumns;
        mutable bool m_isClassIdColumnCached;
        mutable ECDbSqlColumn const* m_classIdColumn;
        std::vector<std::unique_ptr<ECDbSqlConstraint>> m_constraints;
        PersistenceManager m_persistenceManager;
        EditHandle m_editInfo;
        std::vector<std::function<void (ColumnEvent, ECDbSqlColumn&)>> m_columnEvents;
    private:
        ECDbSqlTable (Utf8CP name, ECDbSqlDb& sqlDbDef, ECDbTableId id, PersistenceType type, OwnerType ownerType)
            : m_dbDef(sqlDbDef), m_id(id), m_name(name), m_nameGeneratorForColumn("sc%02x"), m_type(type), m_ownerType(ownerType), 
            m_isClassIdColumnCached(false), m_classIdColumn(nullptr), m_persistenceManager(*this)
            {}

        std::weak_ptr<ECDbSqlColumn> GetColumnWeakPtr (Utf8CP name) const;
    public:
        virtual ~ECDbSqlTable() {}

        ECDbTableId GetId () const { return m_id; }
        void SetId (ECDbTableId id) { m_id = id; }
        Utf8StringCR GetName () const { return m_name; }
        PersistenceType GetPersistenceType () const { return m_type; }
        OwnerType GetOwnerType () const { return m_ownerType; }
        ECDbSqlDb const& GetDbDef () const{ return m_dbDef; }
        ECDbSqlDb & GetDbDefR () { return m_dbDef; }
        //! Any type will be mark as reusable column
        ECDbSqlColumn* CreateColumn (Utf8CP name, ECDbSqlColumn::Type type, ECDbKnownColumns knownColumnId = ECDbKnownColumns::DataColumn, PersistenceType persistenceType = PersistenceType::Persisted);
        ECDbSqlColumn* CreateColumn (Utf8CP name, ECDbSqlColumn::Type type, size_t position, ECDbKnownColumns knownColumnId = ECDbKnownColumns::DataColumn, PersistenceType persistenceType = PersistenceType::Persisted);

        BentleyStatus CreateTrigger(Utf8CP triggerName, ECDbSqlTable& table, Utf8CP condition, Utf8CP body, TriggerType ecsqlType,TriggerSubType triggerSubType);
        std::vector<const ECDbSqlTrigger*> GetTriggers()const;
        ECDbSqlColumn const* FindColumnCP (Utf8CP name) const;
        ECDbSqlColumn* FindColumnP (Utf8CP name) const;
        bool TryGetECClassIdColumn(ECDbSqlColumn const*&) const;
        std::vector<ECDbSqlColumn const*> const& GetColumns () const;
        EditHandle& GetEditHandleR () { return m_editInfo; }
        EditHandle const& GetEditHandle () const { return m_editInfo; }
        ECDbSqlIndex* CreateIndex(Utf8CP indexName, bool isUnique, ECN::ECClassId classId);
        ECDbSqlIndex* CreateIndex(ECDbIndexId id, Utf8CP indexName, bool isUnique, ECN::ECClassId classId);
        const std::vector<ECDbSqlIndex const*> GetIndexes() const;
        ECDbSqlPrimaryKeyConstraint* GetPrimaryKeyConstraint (bool createIfDonotExist = true);
        ECDbSqlForeignKeyConstraint* CreateForeignKeyConstraint (ECDbSqlTable const& targetTable);
        std::vector<ECDbSqlConstraint const*> GetConstraints () const;   
        BentleyStatus GetFilteredColumnList (std::vector<ECDbSqlColumn const*>& columns, PersistenceType persistenceType) const;
        BentleyStatus GetFilteredColumnList (std::vector<ECDbSqlColumn const*>& columns, ECDbKnownColumns knowColumnIds) const;
        ECDbSqlColumn const* GetFilteredColumnFirst (ECDbKnownColumns knowColumnIds) const;
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

    static Utf8String GetDropTableDDL (ECDbSqlTable const& table) { return "DROP TABLE [" + table.GetName () + "]"; }
    static Utf8String GetDropIndexDDL (ECDbSqlIndex const& index) { return "DROP INDEX [" + index.GetName () + "]"; }

    static BentleyStatus AddColumns(ECDbR, ECDbSqlTable const&, std::vector<Utf8CP> const& newColumns);
    static BentleyStatus CopyRows(ECDbR, Utf8CP sourceTable, bvector<Utf8String>& sourceColumns, Utf8CP targetTable, bvector<Utf8String>& targetColumns);
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSqlHelper
    {
private:
    ECDbSqlHelper();
    ~ECDbSqlHelper();

public:
    static ECDbSqlColumn::Type PrimitiveTypeToColumnType (ECN::PrimitiveType type);
    static bool IsCompatible (ECDbSqlColumn::Type target, ECDbSqlColumn::Type source);
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
        Utf8String GetAccessString () const { return m_accessString; }
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
        ECDbSqlColumn const& m_column;
    public:
        ECDbPropertyMapInfo (ECDbClassMapInfo const& classMap, ECDbPropertyPath const& propertyPath, ECDbSqlColumn const& column)
            :m_classMap (classMap), m_propertyPath (propertyPath), m_column (column)
            {
            }
        ECDbSqlColumn const& GetColumn () const { return m_column; }
        ECDbPropertyPath const& GetPropertyPath () const { return m_propertyPath; }
        ECDbClassMapInfo const& GetClassMap () const { return m_classMap; }
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
        ECDbMapStorage& m_map;

    protected:
        const std::map<Utf8CP, ECDbPropertyMapInfo const*, CompareIUtf8> GetPropertyMapByColumnName (bool onlyLocal) const;
        void GetPropertyMaps (std::vector<ECDbPropertyMapInfo const*>& propertyMaps, bool onlyLocal) const;

    public:
        ECDbClassMapInfo (ECDbMapStorage& map, ECDbClassMapId id, ECN::ECClassId classId, ECDbMapStrategy mapStrategy, ECDbClassMapId baseClassMap = 0LL)
            :m_map (map), m_id (id), m_ecClassId (classId), m_mapStrategy (mapStrategy), m_ecBaseClassMap (nullptr), m_ecBaseClassMapId (baseClassMap)
            {}

        ECDbMapStorage& GetMapStorageR () { return m_map; }
        ECDbMapStorage const& GetMapStorage () const{ return m_map; }
        ECDbClassMapId GetId () const { return m_id; }
        ECN::ECClassId GetClassId () const { return m_ecClassId; }
        const std::vector<ECDbPropertyMapInfo const*>  GetPropertyMaps (bool onlyLocal) const;
        ECDbClassMapInfo const*  GetBaseClassMap () const;
        ECDbMapStrategy const& GetMapStrategy () const { return m_mapStrategy; }
        std::vector<ECDbClassMapInfo*> const& GetChildren () const { return m_childClassMaps; }
        ECDbPropertyMapInfo const * FindPropertyMap (Utf8CP columnName) const;
        ECDbPropertyMapInfo const* FindPropertyMap (ECN::ECPropertyId rootPropertyId, Utf8CP accessString) const;
        ECDbPropertyMapInfo* CreatePropertyMap (ECDbPropertyPath const& propertyPath, ECDbSqlColumn const& column);
        ECDbPropertyMapInfo* CreatePropertyMap (ECN::ECPropertyId rootPropertyId, Utf8CP accessString, ECDbSqlColumn const& column);
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbMapStorage
    {
    private:

        const Utf8CP Sql_InsertPropertyPath = "INSERT OR REPLACE INTO ec_PropertyPath (Id, RootPropertyId, AccessString) VALUES (?,?,?)";
        const Utf8CP Sql_InsertClassMap = "INSERT OR REPLACE INTO ec_ClassMap(Id, ParentId, ClassId, MapStrategy, MapStrategyOptions, MapStrategyAppliesToSubclasses) VALUES (?,?,?,?,?,?)";
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
        std::map<ECDbPropertyPathId, std::unique_ptr<ECDbPropertyPath>> m_propertyPaths;
        std::map<ECN::ECPropertyId, std::map<Utf8CP, ECDbPropertyPath*, CompareUtf8>> m_propertyPathByPropertyId;
        std::map<ECDbClassMapId, std::unique_ptr<ECDbClassMapInfo>> m_classMaps;
        std::map<ECN::ECClassId, std::vector<ECDbClassMapInfo const*>> m_classMapByClassId;

        ECDbSQLManager& m_manager;
    private:
        ECDbPropertyPath* Set (std::unique_ptr<ECDbPropertyPath> propertyPath);
        ECDbClassMapInfo* Set (std::unique_ptr<ECDbClassMapInfo> classMap);
        CachedStatementPtr GetStatement (StatementType type);
        DbResult InsertOrReplace ();
        DbResult InsertPropertyMap (ECDbPropertyMapInfo const& o);
        DbResult InsertClassMap (ECDbClassMapInfo const& o);
        DbResult InsertPropertyPath (ECDbPropertyPath const& o);
        DbResult Read ();
        DbResult ReadPropertyMap (ECDbClassMapInfo& o);
        DbResult ReadClassMaps ();
        DbResult ReadPropertyPaths ();

    public:
        ECDbMapStorage (ECDbSQLManager& manager) :m_manager (manager) {}
        ECDbPropertyPath const * FindPropertyPath (ECN::ECPropertyId rootPropertyId, Utf8CP accessString) const;
        ECDbPropertyPath const* FindPropertyPath (ECDbPropertyPathId propertyPathId) const;
        ECDbClassMapInfo const* FindClassMap (ECDbClassMapId id) const;
        std::vector<ECDbClassMapInfo const*> const* FindClassMapsByClassId (ECN::ECClassId id) const;

        ECDbPropertyPath* CreatePropertyPath (ECN::ECPropertyId rootPropertyId, Utf8CP accessString);
        ECDbClassMapInfo* CreateClassMap (ECN::ECClassId classId, ECDbMapStrategy const& mapStrategy, ECDbClassMapId baseClassMapId = 0LL);

        BentleyStatus Load () { return Read () != BE_SQLITE_DONE ? BentleyStatus::ERROR : BentleyStatus::SUCCESS; }
        BentleyStatus Save () { return InsertOrReplace () != BE_SQLITE_DONE ? BentleyStatus::ERROR : BentleyStatus::SUCCESS; }
        void Reset ()
            {
            m_propertyPaths.clear ();
            m_propertyPathByPropertyId.clear ();
            m_classMaps.clear ();
            m_classMapByClassId.clear ();
            }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         01/2015
//======================================================================================
struct ECDbRepositoryBasedId : IIdGenerator
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
    explicit ECDbRepositoryBasedId (ECDbR ecdb) :IIdGenerator (), m_ecdb (ecdb) {}
    virtual ~ECDbRepositoryBasedId () {}
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         01/2015
//======================================================================================
struct ECDbSqlPersistence : NonCopyableClass
    {
    private:
        const Utf8CP Sql_InsertTable = "INSERT OR REPLACE INTO ec_Table (Id, Name, IsOwnedByECDb, IsVirtual) VALUES (?, ?, ?, ?)";
        const Utf8CP Sql_InsertColumn = "INSERT OR REPLACE INTO ec_Column (Id, TableId, Name, Type, IsVirtual, Ordinal, NotNullConstraint, UniqueConstraint, CheckConstraint, DefaultConstraint, CollationConstraint, OrdinalInPrimaryKey, KnownColumn) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        const Utf8CP Sql_InsertIndex = "INSERT OR REPLACE INTO ec_Index (Id, TableId, Name, IsUnique, ClassId, AdditionalWhereExpression) VALUES (?, ?, ?, ?, ?, ?)";
        const Utf8CP Sql_InsertIndexColumn = "INSERT OR REPLACE INTO ec_IndexColumn (IndexId, ColumnId, Ordinal) VALUES (?, ?, ?)";
        const Utf8CP Sql_InsertForeignKey = "INSERT OR REPLACE INTO ec_ForeignKey (Id, TableId, ReferencedTableId, Name, OnDelete, OnUpdate) VALUES (?, ?, ?, ?, ?, ?)";
        const Utf8CP Sql_InsertForeignKeyColumn = "INSERT OR REPLACE INTO ec_ForeignKeyColumn (ForeignKeyId, ColumnId, ReferencedColumnId, Ordinal) VALUES (?, ?, ?, ?)";
        const Utf8CP Sql_SelectTable = "SELECT Id, Name, IsOwnedByECDb, IsVirtual FROM ec_Table";
        const Utf8CP Sql_SelectColumn = "SELECT Id, Name, Type, IsVirtual, NotNullConstraint, UniqueConstraint, CheckConstraint, DefaultConstraint, CollationConstraint, OrdinalInPrimaryKey, KnownColumn FROM ec_Column WHERE TableId = ? ORDER BY Ordinal";
        const Utf8CP Sql_SelectIndex = "SELECT I.Id, T.Name, I.Name, I.IsUnique, I.ClassId, I.AdditionalWhereExpression FROM ec_Index I INNER JOIN ec_Table T ON T.Id = I.TableId";
        const Utf8CP Sql_SelectIndexColumn = "SELECT C.Name FROM ec_IndexColumn I INNER JOIN ec_Column C ON C.Id = I.ColumnId WHERE I.IndexId = ? ORDER BY I.Ordinal";
        const Utf8CP Sql_SelectForeignKey = "SELECT F.Id, R.Name, F.Name, F.OnDelete, F.OnUpdate FROM ec_ForeignKey F INNER JOIN ec_Table R ON R.Id = F.ReferencedTableId WHERE F.TableId = ?";
        const Utf8CP Sql_SelectForeignKeyColumn = "SELECT A.Name, B.Name FROM ec_ForeignKeyColumn F INNER JOIN ec_Column A ON F.ColumnId = A.Id INNER JOIN ec_Column B ON F.ReferencedColumnId = B.Id  WHERE F.ForeignKeyId = ? ORDER BY F.Ordinal";

        enum class StatementType
            {
            SqlInsertTable,
            SqlInsertColumn,
            SqlInsertIndex,
            SqlInsertIndexColumn,
            SqlInsertForeignKey,
            SqlInsertForeignKeyColumn,
            SqlSelectTable,
            SqlSelectColumn,
            SqlSelectIndex,
            SqlSelectIndexColumn,
            SqlSelectForeignKey,
            SqlSelectForeignKeyColumn
            };

    private:
        std::map <StatementType, std::unique_ptr<Statement>> m_statementCache;
        ECDb& m_ecdb;

    private:

        CachedStatementPtr GetStatement (StatementType);

        DbResult ReadTables (ECDbSqlDb&);
        DbResult ReadTable (Statement&, ECDbSqlDb&);
        DbResult ReadColumns (ECDbSqlTable&);
        DbResult ReadIndexes (ECDbSqlDb&);
        DbResult ReadForeignKeys (ECDbSqlDb&);
        DbResult ReadColumn (Statement&, ECDbSqlTable&, std::map<size_t, ECDbSqlColumn const*>& primaryKeys);
        DbResult ReadIndex (Statement&, ECDbSqlDb&);
        DbResult ReadForeignKeys (ECDbSqlTable&);
        DbResult ReadForeignKey (Statement&, ECDbSqlTable&);
        DbResult InsertTable (ECDbSqlTable const&);
        DbResult InsertIndex (ECDbSqlIndex const&);
        DbResult InsertColumn (ECDbSqlColumn const&, int primaryKeyOrdianal);
        DbResult InsertConstraint (ECDbSqlConstraint const&);
        DbResult InsertForeignKey (ECDbSqlForeignKeyConstraint const&);

    public:
        explicit ECDbSqlPersistence (ECDbR ecdb):m_ecdb (ecdb) {}
        ~ECDbSqlPersistence (){};
        DbResult Read (ECDbSqlDb&);
        DbResult Insert (ECDbSqlDb const&);
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ECDbSQLManager : public NonCopyableClass
    {
    private:
        ECDbSqlDb m_defaultDb;
        ECDbR m_ecdb;
        mutable ECDbSqlTable* m_nullTable;
        ECDbSqlPersistence m_persistence;
        ECDbRepositoryBasedId m_idGenerator;
        ECDbMapStorage m_mapStorage;
        bool m_loaded;
    private:
        void SetupNullTable ();
    public:

        ECDbSQLManager (ECDbR ecdb);
        ~ECDbSQLManager (){}
        ECDbR GetECDbR () { return m_ecdb; }
        ECDbCR GetECDb () const{ return m_ecdb; }
        ECDbSqlDb& GetDbSchemaR () { return m_defaultDb; }
        ECDbSqlDb const& GetDbSchema () const { return m_defaultDb; }
        BentleyStatus Load ();
        BentleyStatus Save ();
        ECDbSqlTable const* GetNullTable () const;
        bool IsNullTable (ECDbSqlTable const& table) const { return &table == GetNullTable (); }
        bool IsTableChanged (ECDbSqlTable const& table) const;
        ECDbMapStorage& GetMapStorageR () { return m_mapStorage; }
        ECDbMapStorage const& GetMapStorage () const { return m_mapStorage; }
        void Reset ();
        bool IsLoaded () const { return m_loaded; }
        IIdGenerator& GetIdGenerator ()
            {
            return m_idGenerator;
            }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE