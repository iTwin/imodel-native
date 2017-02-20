/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbSchema.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/BeId.h>
#include "BeBriefcaseBasedIdSequence.h"
#include "MapStrategy.h"
#include <unordered_map>
#include <bitset>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define DBSCHEMA_NULLTABLENAME "ECDbNotMapped"

enum class PersistenceType
    {
    Physical, //! Persisted in DB
    Virtual //! Not persisted in db rather used as a view specification
    };

struct DbTableId : BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(DbTableId, BeInt64Id)
    };

struct DbColumnId : BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(DbColumnId, BeInt64Id)
    };

struct DbIndexId : BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(DbIndexId, BeInt64Id)
    };

struct DbConstraintId : BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(DbConstraintId, BeInt64Id)
    };

struct PropertyPathId : BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(PropertyPathId, BeInt64Id)
    };

struct DbSchemaNameGenerator
    {
private:
    int m_lastId;
    Utf8String m_format;

public:
    explicit DbSchemaNameGenerator(Utf8CP format) :m_format(format), m_lastId(0) {}
    ~DbSchemaNameGenerator() {}

    void Initialize(int lastId) { m_lastId = lastId; }

    void Generate(Utf8StringR generatedName)
        {
        m_lastId++;
        generatedName.clear();
        generatedName.Sprintf(m_format.c_str(), m_lastId);
        }
    };

struct DbTable;
struct PrimaryKeyDbConstraint;

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbColumn : NonCopyableClass
    {
public:
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

    enum class Kind
        {
        //NOTE: do not assign other ints to the values as they get persisted as is in the ECDb file
        Unknown = 0, //! Not known to ECDb or user define columns
        ECInstanceId = 1, //! ECInstanceId system column also primary key of the table
        ECClassId = 2, //! ECClassId system column. Use if more then on classes is mapped to this table
        SourceECInstanceId = 4,
        SourceECClassId = 8,
        TargetECInstanceId = 16,
        TargetECClassId = 32,
        DataColumn = 64, //! unshared data column
        SharedDataColumn = 128, //! shared data column
        RelECClassId = 256
        };

    struct Constraints : NonCopyableClass
        {
        public:
            enum class Collation
                {
                Unset = 0,
                Binary = 1, // Compares string data using memcmp, regardless of text encoding
                NoCase = 2, // The same as binary, except the 26 upper case characters of ASCII are folded to their lower case equivalents before the comparison is performed. Note that only ASCII characters are case folded. SQLite does not attempt to do full UTF case folding due to the size of the tables required.
                RTrim = 3  // The same as binary, except that trailing space characters are ignored.
                };

        private:
            bool m_hasNotNullConstraint;
            bool m_hasUniqueConstraint;
            Utf8String m_checkConstraint;
            Utf8String m_defaultValueConstraint;
            Collation m_collation;

        public:
            Constraints() : m_hasNotNullConstraint(false), m_hasUniqueConstraint(false), m_collation(Collation::Unset) {}

            bool HasNotNullConstraint() const { return m_hasNotNullConstraint; }
            void SetNotNullConstraint() { m_hasNotNullConstraint = true; }
            bool HasUniqueConstraint() const { return m_hasUniqueConstraint; }
            void SetUniqueConstraint() { m_hasUniqueConstraint = true; }
            Utf8StringCR GetCheckConstraint() const { return m_checkConstraint; }
            void SetCheckConstraint(Utf8CP expression) { m_checkConstraint = expression; }
            Utf8StringCR GetDefaultValueConstraint() const { return m_defaultValueConstraint; }
            void SetDefaultValueExpression(Utf8CP expression) { m_defaultValueConstraint = expression; }
            Collation GetCollation()  const { return m_collation; }
            void SetCollation(Collation collation) { m_collation = collation; }

            static Utf8CP CollationToSql(Collation);
            static bool TryParseCollationString(Collation&, Utf8StringCR);
        };

    struct CreateParams final : NonCopyableClass
        {
        private:
            Utf8String m_columnName;
            bool m_columnNameIsFromPropertyMapCA = false;
            bool m_addNotNullConstraint = false;
            bool m_addUniqueConstraint = false;
            Constraints::Collation m_collation = DbColumn::Constraints::Collation::Unset;

        public:
            CreateParams() {}
            explicit CreateParams(Utf8StringCR colName) : m_columnName(colName), m_columnNameIsFromPropertyMapCA(false) {}

            void Assign(Utf8StringCR colName, bool colNameIsFromPropertyMapCA, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation);
            bool IsValid() const { return !m_columnName.empty(); }
            Utf8StringCR GetColumnName() const { return m_columnName; }
            bool IsColumnNameFromPropertyMapCA() const { return m_columnNameIsFromPropertyMapCA; }
            bool AddNotNullConstraint() const { return m_addNotNullConstraint; }
            bool AddUniqueConstraint() const { return m_addUniqueConstraint; }
            Constraints::Collation GetCollation() const { return m_collation; }

            static Utf8String ColumnNameFromAccessString(Utf8StringCR accessString) { Utf8String colName(accessString); colName.ReplaceAll(".", "_"); return colName; }
        };

private:
    DbColumnId m_id;
    Utf8String m_name;
    DbTable& m_table;
    Type m_type;
    Kind m_kind;
    Constraints m_constraints;
    PersistenceType m_persistenceType;
    PrimaryKeyDbConstraint const* m_pkConstraint;
   
public:
    DbColumn(DbColumnId id, DbTable& table, Utf8StringCR name, Type type, Kind kind, PersistenceType persistenceType)
        : m_id(id), m_table(table), m_name(name), m_type(type), m_persistenceType(persistenceType), m_kind(kind), m_pkConstraint(nullptr)
        {}

    ~DbColumn() {}
    DbColumnId GetId() const { return m_id; }
    PersistenceType GetPersistenceType() const { return m_persistenceType; }
    Utf8StringCR GetName() const { return m_name; }
    Type GetType() const { return m_type; }
    bool DoNotAllowDbNull() const { return m_pkConstraint != nullptr || m_constraints.HasNotNullConstraint(); }
    bool IsUnique() const;
    DbTable const& GetTable() const { return m_table; }
    Constraints const& GetConstraints() const { return m_constraints; };
    bool IsOnlyColumnOfPrimaryKeyConstraint() const;
    Kind GetKind() const { return m_kind; }
    bool IsShared() const { return Enum::Intersects( m_kind, Kind::SharedDataColumn); }
    DbTable& GetTableR() const { return m_table; }
    Constraints& GetConstraintsR() { return m_constraints; };
    void SetIsPrimaryKeyColumn(PrimaryKeyDbConstraint const& pkConstraint) { m_pkConstraint = &pkConstraint; }
    BentleyStatus MakeNonVirtual();
    BentleyStatus SetKind(Kind);
    BentleyStatus AddKind(Kind kind) { return SetKind(Enum::Or(m_kind, kind)); }

    //!@return position of this column in its table (0-based index).
    int DeterminePosition() const;

    static Utf8CP TypeToSql(DbColumn::Type);
    static bool IsCompatible(Type lhs, Type rhs);
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbConstraint : NonCopyableClass
    {
public:
    enum class Type
        {
        PrimaryKey,
        ForeignKey
        };

private:
    Type m_type;
    DbTable const& m_table;

protected:
    DbConstraint(Type type, DbTable const& table) :m_type(type), m_table(table) {}

public:
    virtual ~DbConstraint() {}

    Type GetType() const { return m_type; }
    DbTable const& GetTable() const { return m_table; }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct PrimaryKeyDbConstraint : DbConstraint
    {
private:
    std::vector<DbColumn const*> m_columns;

    explicit PrimaryKeyDbConstraint(DbTable const& table) : DbConstraint(Type::PrimaryKey, table) {}

public:
    ~PrimaryKeyDbConstraint() {}
    static std::unique_ptr<PrimaryKeyDbConstraint> Create(DbTable const&, std::vector<DbColumn*> const&);

    bool Contains(DbColumn const& column) const { return std::find(m_columns.begin(), m_columns.end(), &column) != m_columns.end(); }
    std::vector<DbColumn const*> const& GetColumns() const { return m_columns; }
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ForeignKeyDbConstraint : DbConstraint
    {
public:
    //=======================================================================================
    //! @bsiclass                                                Affan.Khan      03/2015
    //+===============+===============+===============+===============+===============+======
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
        std::vector<DbColumn const*> m_fkColumns;
        std::vector<DbColumn const*> m_referencedTableColumns;
        ActionType m_onDeleteAction;
        ActionType m_onUpdateAction;

    public:
        ForeignKeyDbConstraint(DbTable const& fkTable, DbColumn const& fkColumn, DbColumn const& referencedColumn, ForeignKeyDbConstraint::ActionType onDeleteAction, ForeignKeyDbConstraint::ActionType onUpdateAction);
        ~ForeignKeyDbConstraint() {}

        DbTable const& GetReferencedTable() const { BeAssert(!m_referencedTableColumns.empty()); return m_referencedTableColumns[0]->GetTable(); }
        DbTable const& GetForeignKeyTable() const { return GetTable(); }

        std::vector<DbColumn const*> const& GetFkColumns() const { return m_fkColumns; }
        std::vector<DbColumn const*> const& GetReferencedTableColumns() const { return m_referencedTableColumns; }

        ActionType GetOnDeleteAction() const { return m_onDeleteAction; }
        ActionType GetOnUpdateAction() const { return m_onUpdateAction; }

        bool IsValid() const;
        bool IsDuplicate() const;
        void RemoveIfDuplicate();
        bool Equals(ForeignKeyDbConstraint const& rhs) const;

        BentleyStatus Remove(Utf8CP fkColumnName, Utf8CP referencedColumnName);
        BentleyStatus Remove(size_t index);

        static ActionType ToActionType(Utf8CP str);
        static Utf8CP ActionTypeToSql(ActionType actionType);
    };

//======================================================================================
// @bsiclass                                        muhammad.zaighum        01/2015
//======================================================================================
struct DbTrigger : NonCopyableClass
    {
public:
    enum class Type
        {
        Before,
        After
        };

private:
    Utf8String m_triggerName;
    DbTable const& m_table;
    Type m_type;
    Utf8String m_condition;
    Utf8String m_body;

    explicit DbTrigger(DbTable const& table) : m_table(table) {}

public:
    DbTrigger(Utf8CP triggerName, DbTable const& table, Type type, Utf8CP condition, Utf8CP body) : m_triggerName(triggerName), m_table(table), m_type(type), m_condition(condition), m_body(body) {}

    Utf8CP GetName()const { return m_triggerName.c_str(); }
    DbTable const& GetTable()const { return m_table; }
    Type GetType()const { return m_type; }
    Utf8CP GetCondition()const { return m_condition.c_str(); }
    Utf8CP GetBody()const { return m_body.c_str(); }
    };

struct DbSchema;

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbTable : NonCopyableClass
    {
public:
    enum class Type
        {
        Primary = 0,
        Joined = 1,
        Existing = 2
        };

    struct EditHandle : NonCopyableClass
        {
    private:
        bool m_canEdit;

    public:
        EditHandle() :m_canEdit(true) {}
        ~EditHandle() {}

        bool BeginEdit();
        bool EndEdit();
        bool CanEdit() const { return m_canEdit; }
        bool AssertNotInEditMode();
        };

private:
    DbTableId m_id;
    Utf8String m_name;
    DbTable const* m_parentOfJoinedTable;
    DbSchema& m_dbSchema;
    DbSchemaNameGenerator m_sharedColumnNameGenerator;
    Type m_type;
    PersistenceType m_persistenceType;
    ECN::ECClassId m_exclusiveRootECClassId;
    std::map<Utf8CP, std::shared_ptr<DbColumn>, CompareIUtf8Ascii> m_columns;
    bvector<DbColumn const*> m_orderedColumns;
    DbColumn const* m_classIdColumn;

    std::unique_ptr<PrimaryKeyDbConstraint> m_pkConstraint;
    std::vector<std::unique_ptr<DbConstraint>> m_constraints;
    std::map<Utf8CP, std::unique_ptr<DbTrigger>, CompareIUtf8Ascii> m_triggers;

    EditHandle m_editHandle;
    std::vector<DbTable const*> m_joinedTables;

    DbColumn* CreateColumn(DbColumnId, Utf8StringCR name, DbColumn::Type, int position, DbColumn::Kind, PersistenceType);

public:
    DbTable(DbTableId id, Utf8StringCR name, DbSchema& dbSchema, PersistenceType type, Type tableType, ECN::ECClassId const& exclusiveRootClass, DbTable const* parentOfJoinedTable);

    ~DbTable() {}

    void InitializeSharedColumnNameGenerator(int existingSharedColumnCount) { m_sharedColumnNameGenerator.Initialize(existingSharedColumnCount); }
    //!If this is a joined table the method returns the parent of the joined table, aka primary table.
    //!Otherwise the method returns nullptr
    DbTable const* GetParentOfJoinedTable() const { return m_parentOfJoinedTable; }

    DbTableId GetId() const { return m_id; }
    void SetId(DbTableId id) { m_id = id; }
    Utf8StringCR GetName() const { return m_name; }
    PersistenceType GetPersistenceType() const { return m_persistenceType; }
    Type GetType() const { return m_type; }
    bool IsOwnedByECDb() const { return m_type != Type::Existing; }
    //!See ClassMap::DetermineIsExclusiveRootClassOfTable for the rules when a table has an exclusive root class
    bool HasExclusiveRootECClass() const { return m_exclusiveRootECClassId.IsValid(); }
    ECN::ECClassId const& GetExclusiveRootECClassId() const { BeAssert(HasExclusiveRootECClass()); return m_exclusiveRootECClassId; }

    DbColumn* CreateColumn(Utf8StringCR name, DbColumn::Type type, DbColumn::Kind kind, PersistenceType persistenceType) { return CreateColumn(name, type, -1, kind, persistenceType); }
    DbColumn* CreateSharedColumn(DbColumn::Type);
    DbColumn* CreateColumn(Utf8StringCR name, DbColumn::Type type, int position, DbColumn::Kind kind, PersistenceType persType) { return CreateColumn(DbColumnId(), name, type, position, kind, persType); }
    DbColumn* CreateColumn(DbColumnId id, Utf8StringCR name, DbColumn::Type type, DbColumn::Kind kind, PersistenceType persType) { return CreateColumn(id, name, type, -1, kind, persType); }
    std::vector<DbTable const*> const& GetJoinedTables() const { return m_joinedTables; }

    BentleyStatus CreateTrigger(Utf8CP triggerName, DbTrigger::Type, Utf8CP condition, Utf8CP body);
    std::vector<const DbTrigger*> GetTriggers()const;
    DbColumn const* FindColumn(Utf8CP name) const;
    DbColumn* FindColumnP(Utf8CP name) const;
    DbColumn const& GetECClassIdColumn() const { BeAssert(m_classIdColumn != nullptr); return *m_classIdColumn; }
    bvector<DbColumn const*> const& GetColumns() const { return m_orderedColumns; }
    BentleyStatus GetFilteredColumnList(std::vector<DbColumn const*>&, PersistenceType) const;
    BentleyStatus GetFilteredColumnList(std::vector<DbColumn const*>&, DbColumn::Kind) const;
    DbColumn const* GetFilteredColumnFirst(DbColumn::Kind) const;
    BentleyStatus DeleteColumn(DbColumn&);

    EditHandle& GetEditHandleR() { return m_editHandle; }
    EditHandle const& GetEditHandle() const { return m_editHandle; }
    BentleyStatus CreatePrimaryKeyConstraint(std::vector<DbColumn*> const& pkColumns, std::vector<size_t> const* pkOrdinals = nullptr);
    PrimaryKeyDbConstraint const* GetPrimaryKeyConstraint() const { return m_pkConstraint.get(); }
    ForeignKeyDbConstraint const* CreateForeignKeyConstraint(DbColumn const& fkColumn, DbColumn const& referencedColumn, ForeignKeyDbConstraint::ActionType onDeleteAction, ForeignKeyDbConstraint::ActionType onUpdateAction);
    std::vector<DbConstraint const*> GetConstraints() const;
    BentleyStatus RemoveConstraint(DbConstraint const&);
    //! Only changing to persistence type is supported in limited conditions
    bool IsNullTable() const;
    bool IsValid() const { return m_columns.size() > 0 && m_classIdColumn != nullptr; }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbIndex
    {
private:
    DbIndexId m_id;
    Utf8String m_name;
    DbTable* m_table;
    std::vector<DbColumn const*> m_columns;
    bool m_isUnique;
    bool m_addColsAreNotNullWhereExp;
    bool m_isAutoGenerated;
    ECN::ECClassId m_classId;
    bool m_appliesToSubclassesIfPartial;

public:
    DbIndex(DbIndexId id, DbTable& table, Utf8CP name, bool isUnique, std::vector<DbColumn const*> const& columns, bool addColsAreNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool appliesToSubclassesIfPartial)
        :m_id(id), m_name(name), m_table(&table), m_isUnique(isUnique), m_columns(columns), m_addColsAreNotNullWhereExp(addColsAreNotNullWhereExp), m_isAutoGenerated(isAutoGenerated), m_classId(classId), m_appliesToSubclassesIfPartial(appliesToSubclassesIfPartial)
        {
        BeAssert(!Utf8String::IsNullOrEmpty(name) && !m_columns.empty());
        }

    DbIndexId GetId() const { BeAssert(m_id.IsValid()); return m_id; }
    Utf8StringCR GetName() const { return m_name; }
    DbTable const& GetTable() const { BeAssert(m_table != nullptr); return *m_table; }
    bool GetIsUnique() const { return m_isUnique; }
    std::vector<DbColumn const*> const& GetColumns() const { return m_columns; }
    bool IsAddColumnsAreNotNullWhereExp() const { return m_addColsAreNotNullWhereExp; }
    bool IsAutoGenerated() const { return m_isAutoGenerated; }
    //! If false, the index will not be partial
    bool HasClassId() const { return m_classId.IsValid(); }
    ECN::ECClassId GetClassId() const { return m_classId; }
    //! If true, the partial index will include subclasses in that table. If false, the partial index
    //! will not include subclasses. This is only relevant if the index is partial at all,
    //! i.e. if HasClassId() is true
    bool AppliesToSubclassesIfPartial() const { return m_appliesToSubclassesIfPartial; }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbSchema : NonCopyableClass
    {
public:
    enum class EntityType
        {
        None,
        Table,
        View,
        Index,
        Trigger
        };

private:
    ECDbCR m_ecdb;
    DbSchemaNameGenerator m_nameGenerator;
    mutable std::map<Utf8String, std::unique_ptr<DbTable>, CompareIUtf8Ascii> m_tableMapByName;
    mutable bmap<DbTableId, Utf8String> m_tableMapById;

    mutable DbTable* m_nullTable;
    mutable std::vector<std::unique_ptr<DbIndex>> m_indexes;
    mutable bset<Utf8CP, CompareIUtf8Ascii> m_usedIndexNames;
    mutable bool m_indexesLoaded;
    mutable bool m_syncTableCacheNames;

    BentleyStatus LoadTable(Utf8StringCR name, DbTable*& tableP) const;
    BentleyStatus LoadColumns(DbTable& table) const;
    BentleyStatus InsertTable(DbTable const& table) const;
    BentleyStatus InsertColumn(DbColumn const& column, int columnOrdinal, int primaryKeyOrdinal) const;
    BentleyStatus LoadIndexes() const;
    BentleyStatus InsertIndex(DbIndex const& index) const;
    BentleyStatus UpdateTable(DbTable const& table) const;
    BentleyStatus UpdateColumn(DbColumn const& column, int columnOrdinal, int primaryKeyOrdina) const;

    bool IsTableNameInUse(Utf8StringCR tableName) const;

    bmap<Utf8String, DbTableId, CompareIUtf8Ascii> GetPersistedTableMap() const;
    bmap<Utf8String, DbTableId, CompareIUtf8Ascii> GetExistingTableMap() const;
    bmap<Utf8String, DbColumnId, CompareIUtf8Ascii> GetPersistedColumnMap(DbTableId) const;

public:
    explicit DbSchema(ECDbCR ecdb) : m_ecdb(ecdb), m_nameGenerator("ecdb_%d"), m_nullTable(nullptr), m_indexesLoaded(false), m_syncTableCacheNames(false) { }
    ~DbSchema() {}
    //! Create a table with a given name or if name is null a name will be generated
    DbTable* CreateTable(Utf8StringCR name, DbTable::Type, PersistenceType type, ECN::ECClassId const& exclusiveRootClassId, DbTable const* primaryTable);
    DbTable* CreateTable(DbTableId, Utf8StringCR name, DbTable::Type, PersistenceType type, ECN::ECClassId const& exclusiveRootClassId, DbTable const* primaryTable);
    std::vector<DbTable const*> GetCachedTables() const;
    DbTable const* FindTable(Utf8CP name) const;
    DbTable const* FindTable(DbTableId id) const;
    DbTable* FindTableP(Utf8CP name) const;
    DbTable const* GetNullTable() const;
    std::vector<std::unique_ptr<DbIndex>> const& GetIndexes() const;
    DbIndex* CreateIndex(DbIndexId, DbTable&, Utf8CP indexName, bool isUnique, std::vector<DbColumn const*> const&, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId, bool applyToSubclassesIfPartial);
    DbIndex* CreateIndex(DbTable&, Utf8CP indexName, bool isUnique, std::vector<DbColumn const*> const&, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId, bool applyToSubclassesIfPartial = true);
    DbIndex* CreateIndex(DbTable&, Utf8CP indexName, bool isUnique, std::vector<Utf8CP> const& columnNames, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId, bool applyToSubclassesIfPartial = true);
    BentleyStatus SynchronizeExistingTables();
    void SyncTableCache() const;
    ECDbCR GetECDb() const { return m_ecdb; }
    void Reset() const;

    //!Update existing table in db so any new columns added would be save to disk.
    BentleyStatus UpdateTableOnDisk(DbTable const& table) const { return UpdateTable(table); }
    //!This function save or update table as required. It skip if a table is not loaded
    BentleyStatus SaveOrUpdateTables() const;
    BentleyStatus CreateOrUpdateIndexes() const;
    static EntityType GetEntityType(ECDbCR ecdb, Utf8CP name);
    };

//======================================================================================
// @bsiclass                                              Krischan.Eberle        22/2016
//======================================================================================
struct TableMapper : NonCopyableClass
    {
private:
    TableMapper();
    ~TableMapper();

    static DbTable* CreateTableForExistingTableStrategy(DbSchema&, Utf8StringCR existingTableName, Utf8StringCR primaryKeyColName);
    static DbTable* CreateTableForOtherStrategies(DbSchema&, Utf8StringCR tableName, DbTable::Type, bool isVirtual, Utf8StringCR primaryKeyColumnName, ECN::ECClassId const& exclusiveRootClassId, DbTable const* primaryTable);
public:
    static DbTable* FindOrCreateTable(DbSchema&, Utf8StringCR tableName, DbTable::Type, bool isVirtual, Utf8StringCR primaryKeyColumnName, ECN::ECClassId const& exclusiveRootClassId, DbTable const* primaryTable);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE

