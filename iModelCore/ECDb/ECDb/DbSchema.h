/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbSchema.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"
#include <Bentley/BeId.h>
#include "BeBriefcaseBasedIdSequence.h"
#include "MapStrategy.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

enum class PersistenceType
    {
    Persisted, //! Persisted in DB
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

struct ClassMapId : BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(ClassMapId, BeInt64Id)
    };

struct PropertyPathId : BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(PropertyPathId, BeInt64Id)
    };

struct DbSchemaNameGenerator
    {
private:
    int m_uniqueIdGenerator;
    Utf8String m_format;

public:
    explicit DbSchemaNameGenerator(Utf8CP format = "ecdb_%s") :m_format(format), m_uniqueIdGenerator(1) {}
    ~DbSchemaNameGenerator() {}

    void Generate(Utf8StringR generatedName)
        {
        generatedName.clear();
        generatedName.Sprintf(m_format.c_str(), m_uniqueIdGenerator);
        m_uniqueIdGenerator++;
        }
    };

struct DbTable;

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
        Unknown = 0, //! Not known to ECDb or user define columns
        ECInstanceId = 1, //! ECInstanceId system column also primary key of the table
        ECClassId = 2, //! ECClassId system column. Use if more then on classes is mapped to this table
        SourceECInstanceId = 32,
        SourceECClassId = 64,
        TargetECInstanceId = 128,
        TargetECClassId = 256,
        DataColumn = 512, //! unshared data column
        SharedDataColumn = 1024, //! shared data column
        NonRelSystemColumn = ECInstanceId | ECClassId
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
    DbColumnId m_id;
    Utf8String m_name;
    DbTable& m_table;
    Type m_type;
    Kind m_kind;
    Constraint m_constraints;
    PersistenceType m_persistenceType;

public:
    DbColumn(DbColumnId id, DbTable& table, Utf8CP name, Type type, Kind kind, PersistenceType persistenceType)
        : m_id(id), m_table(table), m_name(name), m_type(type), m_persistenceType(persistenceType), m_kind(kind)
        {}

    ~DbColumn() {}

    DbColumnId GetId() const { return m_id; }
    PersistenceType GetPersistenceType() const { return m_persistenceType; }
    Utf8StringCR GetName() const { return m_name; }
    Type GetType() const { return m_type; };
    DbTable const& GetTable() const { return m_table; }
    DbTable&  GetTableR() const { return m_table; }
    Constraint const& GetConstraint() const { return m_constraints; };
    Constraint& GetConstraintR() { return m_constraints; };

    Kind GetKind() const { return m_kind; }
    BentleyStatus SetKind(Kind);
    BentleyStatus AddKind(Kind kind) { return SetKind(Enum::Or(m_kind, kind)); }

    bool IsShared() const { return m_kind == Kind::SharedDataColumn; }

    static Utf8CP TypeToSql(DbColumn::Type);
    static Type PrimitiveTypeToColumnType(ECN::PrimitiveType);
    static bool IsCompatible(Type lhs, Type rhs);
    static Utf8CP KindToString(Kind);
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

public:
    PrimaryKeyDbConstraint(DbTable const& table) :DbConstraint(Type::PrimaryKey, table) {}
    ~PrimaryKeyDbConstraint() {}
    BentleyStatus Add(Utf8CP columnName);
    BentleyStatus InsertOrReplace(Utf8CP columnName, size_t position);

    BentleyStatus Remove(Utf8CP columnName);
    bool Contains(Utf8CP columnName) const { return std::find_if(m_columns.begin(), m_columns.end(), [columnName] (DbColumn const* column) { return column->GetName().EqualsI(columnName); }) != m_columns.end(); }
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
        DbConstraintId m_id;
        DbTable const& m_referencedTable;
        std::vector<DbColumn const*> m_fkColumns;
        std::vector<DbColumn const*> m_referencedTableColumns;
        ActionType m_onDeleteAction;
        ActionType m_onUpdateAction;

    public:
        ForeignKeyDbConstraint(DbConstraintId id, DbTable const& fkTable, DbTable const& referencedTable)
            :DbConstraint(DbConstraint::Type::ForeignKey, fkTable), m_id(id), m_referencedTable(referencedTable), m_onDeleteAction(ActionType::NotSpecified), m_onUpdateAction(ActionType::NotSpecified)
            {}

        ~ForeignKeyDbConstraint() {}

        DbConstraintId GetId() const { return m_id; }
        void SetId(DbConstraintId id) { m_id = id; }

        DbTable const& GetReferencedTable() const { return m_referencedTable; }
        DbTable const& GetForeignKeyTable() const { return GetTable(); }

        std::vector<DbColumn const*> const& GetFkColumns() const { return m_fkColumns; }
        std::vector<DbColumn const*> const& GetReferencedTableColumns() const { return m_referencedTableColumns; }

        ActionType GetOnDeleteAction() const { return m_onDeleteAction; }
        ActionType GetOnUpdateAction() const { return m_onUpdateAction; }
        void SetOnDeleteAction(ActionType action) { m_onDeleteAction = action; }
        void SetOnUpdateAction(ActionType action) { m_onUpdateAction = action; }

        bool IsValid() const;
        bool IsDuplicate() const;
        void RemoveIfDuplicate();
        bool Equals(ForeignKeyDbConstraint const& rhs) const;

        BentleyStatus Add(Utf8CP fkColumnName, Utf8CP referencedColumnName);
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

    enum class ColumnEvent
        {
        Created,
        Deleted
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
    DbSchemaNameGenerator m_columnNameGenerator;
    Type m_type;
    PersistenceType m_persistenceType;
    std::map<Utf8CP, std::shared_ptr<DbColumn>, CompareIUtf8Ascii> m_columns;
    std::map<Utf8CP, std::unique_ptr<DbTrigger>, CompareIUtf8Ascii> m_triggers;
    std::vector<DbColumn const*> m_orderedColumns;

    int m_minimumSharedColumnCount;
    mutable bool m_isClassIdColumnCached;
    mutable DbColumn const* m_classIdColumn;
    std::vector<std::unique_ptr<DbConstraint>> m_constraints;
    EditHandle m_editHandle;
    std::vector<DbTable const*> m_joinedTables;
    std::vector<std::function<void(ColumnEvent, DbColumn&)>> m_columnEvents;

    DbColumn* CreateColumn(DbColumnId, Utf8CP name, DbColumn::Type, int position, DbColumn::Kind, PersistenceType);

public:
    DbTable(DbTableId id, Utf8CP name, DbSchema& dbSchema, PersistenceType type, Type tableType, DbTable const* parentOfJoinedTable)
        : m_id(id), m_name(name), m_dbSchema(dbSchema), m_columnNameGenerator("sc%02x"), m_persistenceType(type), m_type(tableType),
        m_minimumSharedColumnCount(ECN::ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT), m_isClassIdColumnCached(false),
        m_classIdColumn(nullptr), m_parentOfJoinedTable(parentOfJoinedTable)
        {
        BeAssert((tableType == Type::Joined && parentOfJoinedTable != nullptr) ||
                 (tableType != Type::Joined && parentOfJoinedTable == nullptr) && "parentOfJoinedTable must be provided for Type::Joined and must be null for any other DbTable::Type.");

        if (tableType == Type::Joined && parentOfJoinedTable != nullptr)
            const_cast<DbTable*>(parentOfJoinedTable)->m_joinedTables.push_back(this);
        }

    ~DbTable() {}

    //!If this is a joined table the method returns the parent of the joined table, aka primary table.
    //!Otherwise the method returns nullptr
    DbTable const* GetParentOfJoinedTable() const { return m_parentOfJoinedTable; }

    DbTableId GetId() const { return m_id; }
    void SetId(DbTableId id) { m_id = id; }
    Utf8StringCR GetName() const { return m_name; }
    PersistenceType GetPersistenceType() const { return m_persistenceType; }
    Type GetType() const { return m_type; }
    bool IsOwnedByECDb() const { return m_type != Type::Existing; }
    DbColumn* CreateColumn(Utf8CP name, DbColumn::Type type, DbColumn::Kind kind, PersistenceType persistenceType) { return CreateColumn(name, type, -1, kind, persistenceType); }
    DbColumn* CreateSharedColumn() { return CreateColumn(nullptr, DbColumn::Type::Any, DbColumn::Kind::SharedDataColumn, PersistenceType::Persisted); }
    DbColumn* CreateColumn(Utf8CP name, DbColumn::Type type, int position, DbColumn::Kind kind, PersistenceType persType) { return CreateColumn(DbColumnId(), name, type, position, kind, persType); }
    DbColumn* CreateColumn(DbColumnId id, Utf8CP name, DbColumn::Type type, DbColumn::Kind kind, PersistenceType persType) { return CreateColumn(id, name, type, -1, kind, persType); }
    BentleyStatus SetMinimumSharedColumnCount(int minimumSharedColumnCount);
    BentleyStatus EnsureMinimumNumberOfSharedColumns();

    std::vector<DbTable const*> const& GetJoinedTables() const { return m_joinedTables; }

    BentleyStatus CreateTrigger(Utf8CP triggerName, DbTrigger::Type, Utf8CP condition, Utf8CP body);
    std::vector<const DbTrigger*> GetTriggers()const;
    DbColumn const* FindColumn(Utf8CP name) const;
    DbColumn* FindColumnP(Utf8CP name) const;
    std::weak_ptr<DbColumn> FindColumnWeakPtr(Utf8CP name) const;
    bool TryGetECClassIdColumn(DbColumn const*&) const;
    std::vector<DbColumn const*> const& GetColumns() const { return m_orderedColumns; }
    EditHandle& GetEditHandleR() { return m_editHandle; }
    EditHandle const& GetEditHandle() const { return m_editHandle; }
    PrimaryKeyDbConstraint& GetPrimaryKeyConstraintR();
    PrimaryKeyDbConstraint const* GetPrimaryKeyConstraint() const;
    ForeignKeyDbConstraint* CreateForeignKeyConstraint(DbTable const& referencedTable);
    BentleyStatus RemoveConstraint(DbConstraint const&);
    std::vector<DbConstraint const*> GetConstraints() const;
    BentleyStatus GetFilteredColumnList(std::vector<DbColumn const*>&, PersistenceType) const;
    BentleyStatus GetFilteredColumnList(std::vector<DbColumn const*>&, DbColumn::Kind) const;
    DbColumn const* GetFilteredColumnFirst(DbColumn::Kind) const;
    bool DeleteColumn(Utf8CP name);
    void AddColumnEventHandler(std::function<void(ColumnEvent, DbColumn&)> columnEventHandler) { m_columnEvents.push_back(columnEventHandler); }
    bool IsNullTable() const;
    bool IsValid() const { return m_columns.size() > 0; }
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

struct ClassDbMapping;

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct PropertyDbMapping : NonCopyableClass
    {
public:
    struct Path
        {
        private:
            PropertyPathId m_pathId;
            ECN::ECPropertyId m_rootPropertyId;
            Utf8String m_accessString;

        public:
            Path(PropertyPathId id, ECN::ECPropertyId rootPropertyId, Utf8CP accessString)
                :m_pathId(id), m_rootPropertyId(rootPropertyId), m_accessString(accessString)
                {}
            ~Path() {}

            PropertyPathId GetId() const { return m_pathId; }
            ECN::ECPropertyId GetRootPropertyId() const { return m_rootPropertyId; }
            Utf8StringCR GetAccessString() const { return m_accessString; }
            Utf8String GetName() const
                {
                const size_t i = m_accessString.find('.');
                if (i == Utf8String::npos)
                    return m_accessString;

                return m_accessString.substr(0, i);
                }
        };

private:
    ClassDbMapping const& m_classMapping;
    Path const& m_propertyPath;
    std::vector<DbColumn const*> m_columns;

public:
    PropertyDbMapping(ClassDbMapping const& classMapping, Path const& propertyPath) :m_classMapping(classMapping), m_propertyPath(propertyPath) {}
    std::vector<DbColumn const*>& GetColumnsR() { return m_columns; }
    std::vector<DbColumn const*> const& GetColumns() const { return m_columns; }
    Path const& GetPropertyPath() const { return m_propertyPath; }
    ClassDbMapping const& GetClassMapping() const { return m_classMapping; }
    DbColumn const* ExpectingSingleColumn() const { BeAssert(m_columns.size() == 1 && "Expecting exactly one column"); return m_columns[0]; }
    };

struct DbMappings;

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct ClassDbMapping : NonCopyableClass
    {
private:
    DbMappings& m_dbMappings;
    ClassMapId m_id;
    ECN::ECClassId m_ecClassId;
    ECDbMapStrategy m_mapStrategy;
    std::vector<std::unique_ptr<PropertyDbMapping>> m_localPropertyMaps;
    ClassMapId m_baseClassMappingId;

public:
    ClassDbMapping(DbMappings& dbMappings, ClassMapId id, ECN::ECClassId classId, ECDbMapStrategy mapStrategy, ClassMapId baseClassMappingId)
        :m_dbMappings(dbMappings), m_id(id), m_ecClassId(classId), m_mapStrategy(mapStrategy), m_baseClassMappingId(baseClassMappingId)
        {}

    ClassMapId GetId() const { return m_id; }
    ECN::ECClassId GetClassId() const { return m_ecClassId; }
    ECDbMapStrategy const& GetMapStrategy() const { return m_mapStrategy; }

    ClassMapId GetBaseClassMappingId() const { return m_baseClassMappingId; }
    ClassDbMapping const* GetBaseClassMapping() const;
    void GetPropertyMappings(std::vector<PropertyDbMapping const*>& propertyMappings, bool onlyLocal) const;

    PropertyDbMapping const* FindPropertyMapping(ECN::ECPropertyId rootPropertyId, Utf8CP accessString) const;
    PropertyDbMapping const* FindPropertyMapping(Utf8CP accessString) const;
    PropertyDbMapping* CreatePropertyMapping(PropertyDbMapping::Path const&);
    PropertyDbMapping* CreatePropertyMapping(ECN::ECPropertyId rootPropertyId, Utf8CP accessString, std::vector<DbColumn const*> const&);
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbMappings
    {
public:
    typedef std::map<PropertyPathId, std::unique_ptr<PropertyDbMapping::Path>> PropertyPathMap;
    typedef std::map<ClassMapId, std::unique_ptr<ClassDbMapping>> ClassMappingMap;

private:
    ECDbCR m_ecdb;
    PropertyPathMap m_propertyPaths;
    bmap<ECN::ECPropertyId, bmap<Utf8CP, PropertyDbMapping::Path*, CompareUtf8>> m_propertyPathsByRootPropertyId;
    ClassMappingMap m_classMappings;
    bmap<ECN::ECClassId, std::vector<ClassDbMapping const*>> m_classMappingsByClassId;

public:
    explicit DbMappings(ECDbCR ecdb) : m_ecdb(ecdb) {}
        
    void Reset();

    ClassMappingMap const& GetClassMappings() const { return m_classMappings; }
    ClassDbMapping const* FindClassMapping(ClassMapId) const;
    std::vector<ClassDbMapping const*> const* FindClassMappings(ECN::ECClassId) const;

    PropertyPathMap const& GetPropertyPaths() const { return m_propertyPaths; }
    PropertyDbMapping::Path const * FindPropertyPath(ECN::ECPropertyId rootPropertyId, Utf8CP accessString) const;
    PropertyDbMapping::Path const* FindPropertyPath(PropertyPathId) const;
        
    ClassDbMapping* AddClassMapping(ClassMapId, ECN::ECClassId, ECDbMapStrategy const& mapStrategy, ClassMapId baseClassMapId);
    ClassDbMapping* CreateClassMapping(ECN::ECClassId classId, ECDbMapStrategy const& mapStrategy, ClassMapId baseClassMapId);
    PropertyDbMapping::Path* AddPropertyPath(PropertyPathId, ECN::ECPropertyId rootPropertyId, Utf8CP accessString);
    PropertyDbMapping::Path* CreatePropertyPath(ECN::ECPropertyId rootPropertyId, Utf8CP accessString);
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct DbSchema : NonCopyableClass
    {
public:
    enum class LoadState
        {
        NotLoaded = 0,
        Core = 1,
        Indexes = 2,
        ForSchemaImport = Core | Indexes
        };

    enum class EntityType
        {
        None,
        Table,
        View,
        Index,
        Trigger
        };

    typedef std::map<Utf8CP, std::unique_ptr<DbTable>, CompareIUtf8Ascii> TableMap;

private:
    ECDbCR m_ecdb;
    DbSchemaNameGenerator m_nameGenerator;
    TableMap m_tables;
    mutable DbTable* m_nullTable;
    std::vector<std::unique_ptr<DbIndex>> m_indexes;
    mutable bset<Utf8CP, CompareIUtf8Ascii> m_usedIndexNames;
    DbMappings m_dbMappings;
    LoadState m_loadState;

public:
    explicit DbSchema(ECDbCR ecdb) : m_ecdb(ecdb), m_nameGenerator("ecdb_%03d"), m_nullTable(nullptr), m_dbMappings(ecdb), m_loadState(LoadState::NotLoaded) {}
    ~DbSchema() {}

    LoadState GetLoadState() const { return m_loadState; }
    void SetLoadState(LoadState state) { m_loadState = state; }
    //! Create a table with a given name or if name is null a name will be generated
    DbTable* CreateTable(Utf8CP name, DbTable::Type, PersistenceType type, DbTable const* primaryTable);
    DbTable* CreateTableAndColumnsForExistingTableMapStrategy(Utf8CP existingTableName);
    DbTable* CreateTable(DbTableId, Utf8CP name, DbTable::Type, PersistenceType type, DbTable const* primaryTable);
    TableMap const& GetTables() const { return m_tables; }
    DbTable const* FindTable(Utf8CP name) const;
    DbTable* FindTableP(Utf8CP name) const;
    DbSchemaNameGenerator& GetNameGenerator() { return m_nameGenerator; }
    bool IsTableNameInUse(Utf8CP tableName) const { return m_tables.find(tableName) != m_tables.end(); }

    DbTable const* GetNullTable() const;

    std::vector<std::unique_ptr<DbIndex>> const& GetIndexes() const { return m_indexes; }
    DbIndex* CreateIndex(DbIndexId, DbTable&, Utf8CP indexName, bool isUnique, std::vector<DbColumn const*> const&, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId, bool applyToSubclassesIfPartial);
    DbIndex* CreateIndex(DbTable&, Utf8CP indexName, bool isUnique, std::vector<DbColumn const*> const&, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId, bool applyToSubclassesIfPartial = true);
    DbIndex* CreateIndex(DbTable&, Utf8CP indexName, bool isUnique, std::vector<Utf8CP> const& columnNames, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId, bool applyToSubclassesIfPartial = true);

    DbMappings const& GetDbMappings() const { return m_dbMappings; }
    DbMappings& GetDbMappingsR() { return m_dbMappings; }

    ECDbCR GetECDb() const { return m_ecdb; }
    void Reset();

    static EntityType GetEntityType(ECDbCR ecdb, Utf8CP name);
    };

ENUM_IS_FLAGS(DbSchema::LoadState);

END_BENTLEY_SQLITE_EC_NAMESPACE

