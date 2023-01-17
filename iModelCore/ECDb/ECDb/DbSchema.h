/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <cstddef>
#include "ECDbInternalTypes.h"
#include <Bentley/BeId.h>
#include <Bentley/Nullable.h>
#include <ECObjects/ECObjectsAPI.h>
#include <BeSQLite/BeBriefcaseBasedIdSequence.h>
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

struct DbTableId final: BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(DbTableId, BeInt64Id)
    };

struct DbColumnId final : BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(DbColumnId, BeInt64Id)
    };

struct DbConstraintId final : BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(DbConstraintId, BeInt64Id)
    };

struct PropertyPathId final : BeInt64Id
    {
    BEINT64_ID_DECLARE_MEMBERS(PropertyPathId, BeInt64Id)
    };

//======================================================================================
// @bsiclass
//======================================================================================
struct DbSchemaNameGenerator final
    {
private:
    uint32_t m_lastId = 0;
    Utf8String m_format;

public:
    DbSchemaNameGenerator() {}
    //! @param[in] namePrefix Prefix for the names to be generated. The generator appends a number to it.
    //! Must not include a placeholder for the number appended.
    explicit DbSchemaNameGenerator(Utf8CP namePrefix) :m_format(namePrefix) 
        { 
        BeAssert(!Utf8String::IsNullOrEmpty(namePrefix));
        if (!Utf8String::IsNullOrEmpty(namePrefix))
            m_format.assign(namePrefix).append("%" PRIu32);
        }
    ~DbSchemaNameGenerator() {}

    void Initialize(uint32_t lastId) { BeAssert(IsValid()); m_lastId = lastId; }
    void Generate(Utf8StringR generatedName) { BeAssert(IsValid()); m_lastId++; generatedName.Sprintf(m_format.c_str(), m_lastId); }

    bool IsValid() const { return !m_format.empty(); }
    };

struct DbTable;
struct PrimaryKeyDbConstraint;

//======================================================================================
// @bsiclass
//======================================================================================
struct DbColumn final
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
        Default = 0, //! Not known to ECDb or data columns
        ECInstanceId = 1, //! ECInstanceId system column, i.e.the primary key of the table
        ECClassId = 2, //! ECClassId system column. Use if more then on classes is mapped to this table
        SharedData = 4, //! shared data column
        };

    struct Constraints final
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
            bool m_hasNotNullConstraint = false;
            bool m_hasUniqueConstraint = false;
            Utf8String m_checkConstraint;
            Utf8String m_defaultValueConstraint;
            Collation m_collation = Collation::Unset;

            //not copyable
            Constraints(Constraints const&) = delete;
            Constraints& operator=(Constraints const&) = delete;

        public:
            Constraints() {}

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

    struct CreateParams final
        {
        private:
            Utf8String m_columnName;
            bool m_columnNameIsFromPropertyMapCA = false;
            bool m_addNotNullConstraint = false;
            bool m_addUniqueConstraint = false;
            Constraints::Collation m_collation = DbColumn::Constraints::Collation::Unset;

            //not copyable
            CreateParams(CreateParams const&) = delete;
            CreateParams& operator=(CreateParams const&) = delete;

        public:
            CreateParams() {}
            explicit CreateParams(Utf8StringCR colName) : m_columnName(colName) {}

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
    PrimaryKeyDbConstraint const* m_pkConstraint = nullptr;
   
    //not copyable
    DbColumn(DbColumn const&) = delete;
    DbColumn& operator=(DbColumn const&) = delete;

public:
    DbColumn(DbTable& table, Utf8StringCR name, Type type, Kind kind, PersistenceType persistenceType) : DbColumn(DbColumnId(), table, name, type, kind, persistenceType) {}
    DbColumn(DbColumnId id, DbTable& table, Utf8StringCR name, Type type, Kind kind, PersistenceType persistenceType)
        : m_id(id), m_table(table), m_name(name), m_type(type), m_persistenceType(persistenceType), m_kind(kind)
        {}

    ~DbColumn() {}

    bool operator==(DbColumn const& rhs) const;
    bool operator!=(DbColumn const& rhs) const { return !(*this == rhs); }

    DbColumnId GetId() const { BeAssert(m_id.IsValid() && "DbColumn::GetId must not be called if the DbColumn is not persisted yet"); return m_id; }
    void SetId(DbColumnId id) { BeAssert(!m_id.IsValid()); BeAssert(id.IsValid()); m_id = id; }
    bool HasId() const { return m_id.IsValid();}
    PersistenceType GetPersistenceType() const { return m_persistenceType; }
    Utf8StringCR GetName() const { return m_name; }
    Type GetType() const { return m_type; }
    bool DoNotAllowDbNull() const { return m_pkConstraint != nullptr || m_constraints.HasNotNullConstraint(); }
    bool IsUnique() const;
    DbTable const& GetTable() const { return m_table; }
    Constraints const& GetConstraints() const { return m_constraints; };
    bool IsOnlyColumnOfPrimaryKeyConstraint() const;
    Kind GetKind() const { return m_kind; }
    bool IsShared() const { return m_kind ==  Kind::SharedData; }
    DbTable& GetTableR() const { return m_table; }
    Constraints& GetConstraintsR() { return m_constraints; };
    void SetIsPrimaryKeyColumn(PrimaryKeyDbConstraint const& pkConstraint) { m_pkConstraint = &pkConstraint; }
    BentleyStatus SetKind(Kind);
    bool IsVirtual() const { return GetPersistenceType() == PersistenceType::Virtual; }
    //!@return position of this column in its table (0-based index).
    int DeterminePosition() const;

    static Utf8CP TypeToSql(DbColumn::Type);
    static bool IsCompatible(Type lhs, Type rhs);
    };


//======================================================================================
// @bsiclass
//======================================================================================
struct DbConstraint
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

    //not copyable
    DbConstraint(DbConstraint const&) = delete;
    DbConstraint& operator=(DbConstraint const&) = delete;

protected:
    DbConstraint(Type type, DbTable const& table) :m_type(type), m_table(table) {}

public:
    virtual ~DbConstraint() {}

    Type GetType() const { return m_type; }
    DbTable const& GetTable() const { return m_table; }
    };

//======================================================================================
// @bsiclass
//======================================================================================
struct PrimaryKeyDbConstraint final : DbConstraint
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
// @bsiclass
//======================================================================================
struct ForeignKeyDbConstraint final : DbConstraint
    {
public:
    //=======================================================================================
    //! @bsiclass
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

        static BentleyStatus TryParseActionType(ActionType&, Nullable<Utf8String> const&);
        static Utf8CP ActionTypeToSql(ActionType actionType);
    };

struct DbTable;

//======================================================================================
// @bsiclass
//======================================================================================
struct DbIndex final
    {
    private:
        Utf8String m_name;
        DbTable const& m_table;
        std::vector<DbColumn const*> m_columns;
        bool m_isUnique;
        bool m_addColsAreNotNullWhereExp;
        bool m_isAutoGenerated;
        ECN::ECClassId m_classId;
        bool m_appliesToSubclassesIfPartial;

    public:
        DbIndex(DbTable const& table, Utf8StringCR name, bool isUnique, std::vector<DbColumn const*> const& columns, bool addColsAreNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool appliesToSubclassesIfPartial)
            : m_name(name), m_table(table), m_isUnique(isUnique), m_columns(columns), m_addColsAreNotNullWhereExp(addColsAreNotNullWhereExp), m_isAutoGenerated(isAutoGenerated), m_classId(classId), m_appliesToSubclassesIfPartial(appliesToSubclassesIfPartial) { BeAssert(!name.empty() && !m_columns.empty()); }

        Utf8StringCR GetName() const { return m_name; }
        DbTable const& GetTable() const { return m_table; }
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
// @bsiclass
//======================================================================================
struct DbTrigger final
    {
public:
    enum class Type
        {
        Before,
        After
        };

private:
    DbTable const& m_table;
    Utf8String m_name;
    Type m_type;
    Utf8String m_condition;
    Utf8String m_body;

    explicit DbTrigger(DbTable const& table) : m_table(table) {}
    //not copyable
    DbTrigger(DbTrigger const&) = delete;
    DbTrigger& operator=(DbTrigger const&) = delete;

public:
    DbTrigger(Utf8StringCR triggerName, DbTable const& table, Type type, Utf8StringCR condition, Utf8StringCR body) : m_table(table), m_name(triggerName), m_type(type), m_condition(condition), m_body(body) {}

    DbTable const& GetTable()const { return m_table; }
    Utf8StringCR GetName()const { return m_name; }
    Type GetType()const { return m_type; }
    Utf8StringCR GetCondition()const { return m_condition; }
    Utf8StringCR GetBody()const { return m_body; }
    };



struct DbSchema;

//======================================================================================
// @bsiclass
//======================================================================================
struct DbTable final
    {
public:
    enum class Type
        {
        Primary = 0,
        Joined = 1, //! Joined Table cannot exist without a primary table
        Existing = 2, 
        Overflow = 3, //! Overflow table cannot exist without a primary or joined table
        Virtual = 4, //for abstract classes not using TPH and mixins
        Temp = 6, // Not supported yet. Tell ECDb to make sure table available at run time
        };

    struct LinkNode final
        {
        private:
            DbTable const& m_table;
            LinkNode const* m_parent = nullptr;
            std::vector<LinkNode const*> m_children;

            //not copyable
            LinkNode(LinkNode const&) = delete;
            LinkNode& operator=(LinkNode const&) = delete;

        public:
            LinkNode(DbTable const& thisTable, DbTable const* parent);

            DbTable const& GetTable() const { return m_table; }
            DbTable& GetTableR() const { return const_cast<DbTable&>(m_table); }
            bool IsChildTable() const { return m_table.GetType() == Type::Joined || m_table.GetType() == Type::Overflow; }

            LinkNode const* GetParent() const { return m_parent; }
            std::vector<LinkNode const*> const& GetChildren() const { return m_children; }
            LinkNode const* FindOverflowTable() const;

            BentleyStatus Validate() const;
        };

    struct EditHandle final
        {
    private:
        bool m_canEdit = true;

        //not copyable
        EditHandle(EditHandle const&) = delete;
        EditHandle& operator=(EditHandle const&) = delete;

    public:
        EditHandle() {}
        ~EditHandle() {}

        bool BeginEdit();
        bool EndEdit();
        bool CanEdit() const { return m_canEdit; }
        bool AssertNotInEditMode();
        };

private:
    DbTableId m_id;
    Utf8String m_name;
    DbTableSpace const& m_tableSpace;
    Type m_type;
    ECN::ECClassId m_exclusiveRootECClassId;
    std::map<Utf8CP, std::shared_ptr<DbColumn>, CompareIUtf8Ascii> m_columns;
    bvector<DbColumn const*> m_orderedColumns;
    DbColumn const* m_classIdColumn = nullptr;

    std::unique_ptr<PrimaryKeyDbConstraint> m_pkConstraint;
    std::vector<std::unique_ptr<DbConstraint>> m_constraints;
    std::vector<std::unique_ptr<DbIndex>> m_indexes;
    std::map<Utf8CP, std::unique_ptr<DbTrigger>, CompareIUtf8Ascii> m_triggers;

    DbSchemaNameGenerator m_sharedColumnNameGenerator;
    LinkNode m_linkNode;

    EditHandle m_editHandle;

    //not copyable
    DbTable(DbTable const&) = delete;
    DbTable& operator=(DbTable const&) = delete;
    DbColumn* AddColumn(DbColumnId, Utf8StringCR name, DbColumn::Type type, int position, DbColumn::Kind kind, PersistenceType persType);
    static Utf8CP GetSharedColumnNamePrefix(Type);

public:
    DbTable(DbTableId id, Utf8StringCR name, DbTableSpace const&, Type, ECN::ECClassId exclusiveRootClass, DbTable const* parentTable);
    ~DbTable() {}

    bool operator==(DbTable const& rhs) const;
    bool operator!=(DbTable const& rhs) const { return !(*this == rhs); }

    void InitializeSharedColumnNameGenerator(uint32_t existingSharedColumnCount) { m_sharedColumnNameGenerator.Initialize(existingSharedColumnCount); }
    DbTableId GetId() const { BeAssert(m_id.IsValid() && "Must not call DbTable::GetId on unpersisted DbTable"); return m_id; }
    void SetId(DbTableId id) { BeAssert(!m_id.IsValid()); BeAssert(id.IsValid()); m_id = id; }
    bool HasId() const { return m_id.IsValid();}
    Utf8StringCR GetName() const { return m_name; }
    DbTableSpace const& GetTableSpace() const { return m_tableSpace; }
    Type GetType() const { return m_type; }
    //!See ClassMap::DetermineIsExclusiveRootClassOfTable for the rules when a table has an exclusive root class
    bool HasExclusiveRootECClass() const { return m_exclusiveRootECClassId.IsValid(); }
    ECN::ECClassId GetExclusiveRootECClassId() const { BeAssert(HasExclusiveRootECClass()); return m_exclusiveRootECClassId; }

    DbColumn const* FindColumn(Utf8CP name) const;
    DbColumn* FindColumnP(Utf8CP name) const;
    DbColumn const& GetECClassIdColumn() const { BeAssert(m_classIdColumn != nullptr); return *m_classIdColumn; }
    bvector<DbColumn const*> const& GetColumns() const { return m_orderedColumns; }
    std::vector<DbColumn const*> FindAll(PersistenceType) const;
    std::vector<DbColumn const*> FindAll(DbColumn::Kind) const;
    DbColumn const* FindFirst(DbColumn::Kind) const;

    DbColumn* AddColumn(Utf8StringCR name, DbColumn::Type type, DbColumn::Kind kind, PersistenceType persistenceType) { return AddColumn(DbColumnId(), name, type, kind, persistenceType); }
    DbColumn* AddColumn(DbColumnId id, Utf8StringCR name, DbColumn::Type type, DbColumn::Kind kind, PersistenceType persistenceType) { return AddColumn(id, name, type, -1, kind, persistenceType); }
    DbColumn* AddColumn(Utf8StringCR name, DbColumn::Type type, int position, DbColumn::Kind kind, PersistenceType persType) { return AddColumn(DbColumnId(), name, type, position, kind, persType); }
    DbColumn* AddSharedColumn();

    LinkNode const& GetLinkNode() const { return m_linkNode; }

    std::vector<std::unique_ptr<DbIndex>> const& GetIndexes() const { return m_indexes; }
    void AddIndexDef(std::unique_ptr<DbIndex>&& indexDef) { m_indexes.push_back(std::move(indexDef)); }
    bool RemoveIndexDef(Utf8StringCR indexName);

    BentleyStatus AddTrigger(Utf8StringCR triggerName, DbTrigger::Type, Utf8StringCR condition, Utf8StringCR body);
    std::vector<const DbTrigger*> GetTriggers() const;

    EditHandle& GetEditHandleR() { return m_editHandle; }
    EditHandle const& GetEditHandle() const { return m_editHandle; }
    BentleyStatus AddPrimaryKeyConstraint(std::vector<DbColumn*> const& pkColumns, std::vector<size_t> const* pkOrdinals = nullptr);
    PrimaryKeyDbConstraint const* GetPrimaryKeyConstraint() const { return m_pkConstraint.get(); }
    ForeignKeyDbConstraint const* AddForeignKeyConstraint(DbColumn const& fkColumn, DbColumn const& referencedColumn, ForeignKeyDbConstraint::ActionType onDeleteAction, ForeignKeyDbConstraint::ActionType onUpdateAction);
    std::vector<DbConstraint const*> GetConstraints() const;
    BentleyStatus RemoveConstraint(DbConstraint const&);
    bool IsValid() const { return m_columns.size() > 0 && m_classIdColumn != nullptr; }
    };


 struct TableSpaceSchemaManager;

//======================================================================================
// @bsiclass
//======================================================================================
struct DbSchema final
    {
public:
   struct TableCollection final
        {
        public:
            struct const_iterator final
                {
                using iterator_category=std::forward_iterator_tag;
                using value_type=DbTable const*;
                using difference_type=std::ptrdiff_t;
                using pointer=DbTable const**;
                using reference=DbTable const*&;

                private:
                    std::map<Utf8String, std::unique_ptr<DbTable>, CompareIUtf8Ascii>::const_iterator m_it;

                public:
                    const_iterator(std::map<Utf8String, std::unique_ptr<DbTable>, CompareIUtf8Ascii>::const_iterator const& innerIt) : m_it(innerIt) {}
                    ~const_iterator() {}
                    const_iterator(const_iterator const& rhs) : m_it(rhs.m_it) {}
                    const_iterator& operator=(const_iterator const& rhs)
                        {
                        if (this != &rhs)
                            m_it = rhs.m_it;

                        return *this;
                        }
                    const_iterator(const_iterator&& rhs) : m_it(std::move(rhs.m_it)) {}
                    const_iterator& operator=(const_iterator&& rhs)
                        {
                        if (this != &rhs)
                            m_it = std::move(rhs.m_it);

                        return *this;
                        }

                    bool operator==(const_iterator const& rhs) const { return m_it == rhs.m_it; }
                    bool operator!=(const_iterator const& rhs) const { return !(*this == rhs); }

                    DbTable const* operator*() const { return m_it->second.get(); }
                    const_iterator& operator++() { m_it++; return *this; }
                };
        private:
            DbTableSpace const& m_tableSpace;
            mutable std::map<Utf8String, std::unique_ptr<DbTable>, CompareIUtf8Ascii> m_tableMapByName;
            mutable bmap<DbTableId, DbTable const*> m_cacheById;

        public:
            explicit TableCollection(DbTableSpace const& tableSpace) : m_tableSpace(tableSpace) {}

            DbTable* Add(DbTableId, Utf8StringCR name, DbTable::Type, ECN::ECClassId exclusiveRootClassId, DbTable const* parentTable);
            void Remove(Utf8StringCR tableName) const;

            DbTable const* Get(Utf8StringCR tableName) const;
            DbTable const* Get(DbTableId) const;
            bool Contains(Utf8StringCR tableName) const { return m_tableMapByName.find(tableName) != m_tableMapByName.end(); }
            const_iterator begin() const { return const_iterator(m_tableMapByName.begin()); }
            const_iterator end() const { return const_iterator(m_tableMapByName.end()); }

            std::vector<DbTable const*> GetTablesInDependencyOrder() const;

            void ClearCache() const { m_tableMapByName.clear(); m_cacheById.clear(); }
        };

private:
    TableSpaceSchemaManager const& m_schemaManager;

    TableCollection m_tables;
    mutable bool m_indexDefsAreLoaded = false;
    mutable DbTable* m_nullTable = nullptr;

    //not copyable
    DbSchema(DbSchema const&) = delete;
    DbSchema& operator=(DbSchema const&) = delete;

    DbTable* LoadTable(Utf8StringCR name) const;
    DbTable* LoadTable(DbTableId) const;
    BentleyStatus LoadColumns(DbTable&) const;
    BentleyStatus InsertTable(DbTable const&) const;
    BentleyStatus InsertColumn(DbColumn const&, int columnOrdinal, int primaryKeyOrdinal) const;
    BentleyStatus UpdateColumn(DbColumn const&, int columnOrdinal, int primaryKeyOrdinal) const;

    BentleyStatus LoadIndexDefs(std::vector<std::pair<DbTable*, std::unique_ptr<DbIndex>>>&, Utf8CP sqlWhereOrJoinClause) const;

    CachedStatementPtr GetCachedStatement(Utf8CP sql) const;

public:
    explicit DbSchema(TableSpaceSchemaManager const&);
    ~DbSchema() {}

    //! Create a table with a given name or if name is null a name will be generated
    DbTable* AddTable(Utf8StringCR name, DbTable::Type, ECN::ECClassId exclusiveRootClassId);
    DbTable* AddTable(Utf8StringCR name, DbTable::Type, ECN::ECClassId exclusiveRootClassId, DbTable const& parentTable);
    TableCollection const& Tables() const { return m_tables; }
    DbTable const* FindTable(Utf8StringCR name) const;
    DbTable* FindTableP(Utf8StringCR name) const;
    DbTable const* FindTable(DbTableId) const;

    DbTable const* GetNullTable() const;
    bool IsNullTable(DbTable const& table) const { return &table == GetNullTable(); }
    BentleyStatus SynchronizeExistingTables();

    //!Update existing table in db so any new columns added would be save to disk.
    BentleyStatus UpdateTable(DbTable const&) const;

    //!This function save or update table as required. It skip if a table is not loaded
    BentleyStatus SaveOrUpdateTables() const;
    BentleyStatus LoadIndexDefs() const;
    BentleyStatus PersistIndexDef(DbIndex const&) const;

    void ClearCache() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

