/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbTests.h"
#include <Bentley/Nullable.h>
#include <ostream>

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     10/2017
//=======================================================================================    
struct JsonValue final
    {
    public:
        Json::Value m_value = Json::Value(Json::nullValue);

        JsonValue() {}
        explicit JsonValue(Json::ValueType type) : m_value(type) {}
        explicit JsonValue(JsonValueCR json) : m_value(json) {}
        explicit JsonValue(Utf8CP json);
        explicit JsonValue(Utf8StringCR json) : JsonValue(json.c_str()) {}

        bool operator==(JsonValue const& rhs) const;
        bool operator!=(JsonValue const& rhs) const { return !(*this == rhs); }

        Utf8String ToString() const { return m_value.ToString(); }
    };

void PrintTo(JsonValue const&, std::ostream*);

//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  07/15
//=======================================================================================    
struct SchemaItem final
    {
    public:
        enum class Type
            {
            String,
            File
            };

    private:
        Type m_type;
        Utf8String m_xmlStringOrFileName;

        SchemaItem(Type type, Utf8StringCR xmlStringOrFileName) : m_type(type), m_xmlStringOrFileName(xmlStringOrFileName) {}

    public:
        explicit SchemaItem(Utf8StringCR xmlString) : SchemaItem(Type::String, xmlString) {}
        static SchemaItem CreateForFile(Utf8StringCR schemaFileName) { return SchemaItem(Type::File, schemaFileName); }

        Type GetType() const { return m_type; }
        Utf8StringCR GetXmlString() const { BeAssert(m_type == Type::String);  return m_xmlStringOrFileName; }
        BeFileName GetFileName() const { BeAssert(m_type == Type::File); return BeFileName(m_xmlStringOrFileName.c_str(), true); }
        Utf8StringCR ToString() const { return m_xmlStringOrFileName; }
    };


//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  05/17
//=======================================================================================    
struct AccessString final
    {
    Utf8String m_schemaNameOrAlias;
    Utf8String m_className;
    Utf8String m_propAccessString;

    AccessString(Utf8CP schemaNameOrAlias, Utf8CP className, Utf8CP propAccessString) : m_schemaNameOrAlias(schemaNameOrAlias), m_className(className), m_propAccessString(propAccessString) {}
    Utf8String ToString() const { Utf8String str; str.Sprintf("%s:%s.%s", m_schemaNameOrAlias.c_str(), m_className.c_str(), m_propAccessString.c_str()); return str; }
    };


struct Column;

struct Table final
    {
    enum class Type
        {
        Primary = 0,
        Joined = 1,
        Existing = 2,
        Overflow = 3,
        Virtual = 4
        };

    private:
        Utf8String m_name;
        Nullable<Type> m_type;
        Utf8String m_parentTableName;
        ECN::ECClassId m_exclusiveRootClassId;
        std::vector<Column> m_columns;
        std::map<Utf8String, size_t> m_columnLookupMap;

        //avoid triggering destructor for static non-POD members -> hold as pointer and never free it
        static Column const* s_nullColumn;

    public:
        Table() {}
        Table(Utf8StringCR name, Type type, Utf8StringCR parentTableName, ECN::ECClassId exclusiveRootClassId) : m_name(name), m_type(type), m_parentTableName(parentTableName), m_exclusiveRootClassId(exclusiveRootClassId) {}
        Table(Table&& rhs) : m_name(std::move(rhs.m_name)), m_type(std::move(rhs.m_type)), m_parentTableName(std::move(rhs.m_parentTableName)), m_exclusiveRootClassId(std::move(rhs.m_exclusiveRootClassId)),
            m_columns(std::move(rhs.m_columns)), m_columnLookupMap(std::move(rhs.m_columnLookupMap)) {}

        Table& operator=(Table&& rhs)
            {
            if (this != &rhs)
                {
                m_name = std::move(rhs.m_name);
                m_type = std::move(rhs.m_type);
                m_parentTableName = std::move(rhs.m_parentTableName);
                m_exclusiveRootClassId = std::move(rhs.m_exclusiveRootClassId);
                m_columns = std::move(rhs.m_columns);
                m_columnLookupMap = std::move(rhs.m_columnLookupMap);
                }

            return *this;
            }

        void AddColumn(Column&&);

        bool Exists() const { return !m_name.empty(); }

        Utf8StringCR GetName() const { return m_name; }
        Nullable<Type> GetType() const { return m_type; }
        Utf8StringCR GetParentTable() const { return m_parentTableName; }
        ECN::ECClassId GetExclusiveRootClassId() const { return m_exclusiveRootClassId; }

        std::vector<Column> const& GetColumns() const { return m_columns; }
        Column const& GetColumn(Utf8StringCR name) const;
    };

// GTest Format customizations for types not handled by GTest

bool operator==(Table::Type lhs, Nullable<Table::Type> rhs);

void PrintTo(Nullable<Table::Type>, std::ostream*);
void PrintTo(Table::Type, std::ostream*);



enum class Virtual { Yes, No };

// GTest Format customizations for types not handled by GTest
void PrintTo(Virtual, std::ostream*);

//=======================================================================================
// @bsiclass                                      Affan.Khan                       05/17
//+===============+===============+===============+===============+===============+======
struct Column final
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

    enum class Collation
        {
        Unset = 0,
        Binary = 1, // Compares string data using memcmp, regardless of text encoding
        NoCase = 2, // The same as binary, except the 26 upper case characters of ASCII are folded to their lower case equivalents before the comparison is performed. Note that only ASCII characters are case folded. SQLite does not attempt to do full UTF case folding due to the size of the tables required.
        RTrim = 3  // The same as binary, except that trailing space characters are ignored.
        };

    enum class Kind
        {
        Default = 0, //! Not known to ECDb or user define columns
        ECInstanceId = 1, //! ECInstanceId system column, i.e.the primary key of the table
        ECClassId = 2, //! ECClassId system column. Use if more then on classes is mapped to this table
        SharedData = 4, //! shared data column
        };

    private:
        Utf8String m_tableName;
        Utf8String m_name;
        Type m_type = Type::Any;
        Virtual m_virtual = Virtual::No;
        bool m_notNullConstraint = false;
        bool m_uniqueConstraint = false;
        Utf8String m_checkConstraint;
        Utf8String m_defaultConstraint;
        Collation m_collationConstraint = Collation::Unset;
        Kind m_kind = Kind::Default;
        Nullable<uint32_t> m_ordinalInPrimaryKey;

    public:
        Column() {}
        Column(Utf8StringCR tableName, Utf8StringCR name, Type type, Virtual isVirtual, bool notNull, bool unique, Utf8StringCR checkConstraint, Utf8StringCR defaultConstraint, Collation collation, Kind kind, Nullable<uint32_t> ordinalInPk)
            :m_tableName(tableName), m_name(name), m_type(type), m_virtual(isVirtual), m_notNullConstraint(notNull), m_uniqueConstraint(unique), m_checkConstraint(checkConstraint), m_defaultConstraint(defaultConstraint), m_collationConstraint(collation), m_kind(kind), m_ordinalInPrimaryKey(ordinalInPk) {}

        Column(Column const& rhs) : m_tableName(rhs.m_tableName), m_name(rhs.m_name), m_type(rhs.m_type), m_virtual(rhs.m_virtual),
            m_notNullConstraint(rhs.m_notNullConstraint), m_uniqueConstraint(rhs.m_uniqueConstraint), m_checkConstraint(rhs.m_checkConstraint), m_defaultConstraint(rhs.m_defaultConstraint),
            m_collationConstraint(rhs.m_collationConstraint), m_kind(rhs.m_kind), m_ordinalInPrimaryKey(rhs.m_ordinalInPrimaryKey)
            {}

        Column(Column&& rhs) : m_tableName(std::move(rhs.m_tableName)), m_name(std::move(rhs.m_name)), m_type(std::move(rhs.m_type)), m_virtual(std::move(rhs.m_virtual)),
            m_notNullConstraint(std::move(rhs.m_notNullConstraint)), m_uniqueConstraint(std::move(rhs.m_uniqueConstraint)), m_checkConstraint(std::move(rhs.m_checkConstraint)), m_defaultConstraint(std::move(rhs.m_defaultConstraint)),
            m_collationConstraint(std::move(rhs.m_collationConstraint)), m_kind(std::move(rhs.m_kind)), m_ordinalInPrimaryKey(std::move(rhs.m_ordinalInPrimaryKey)) {}

        Column& operator=(Column const& rhs)
            {
            if (this != &rhs)
                {
                m_tableName = rhs.m_tableName;
                m_name = rhs.m_name;
                m_type = rhs.m_type;
                m_virtual = rhs.m_virtual;
                m_notNullConstraint = rhs.m_notNullConstraint;
                m_uniqueConstraint = rhs.m_uniqueConstraint;
                m_checkConstraint = rhs.m_checkConstraint;
                m_defaultConstraint = rhs.m_defaultConstraint;
                m_collationConstraint = rhs.m_collationConstraint;
                m_kind = rhs.m_kind;
                m_ordinalInPrimaryKey = rhs.m_ordinalInPrimaryKey;
                }

            return *this;
            }

        Column& operator=(Column&& rhs)
            {
            if (this != &rhs)
                {
                m_tableName = std::move(rhs.m_tableName);
                m_name = std::move(rhs.m_name);
                m_type = std::move(rhs.m_type);
                m_virtual = std::move(rhs.m_virtual);
                m_notNullConstraint = std::move(rhs.m_notNullConstraint);
                m_uniqueConstraint = std::move(rhs.m_uniqueConstraint);
                m_checkConstraint = std::move(rhs.m_checkConstraint);
                m_defaultConstraint = std::move(rhs.m_defaultConstraint);
                m_collationConstraint = std::move(rhs.m_collationConstraint);
                m_kind = std::move(rhs.m_kind);
                m_ordinalInPrimaryKey = std::move(rhs.m_ordinalInPrimaryKey);
                }

            return *this;
            }

        bool Exists() const { return !m_name.empty(); }
        Utf8StringCR GetTableName() const { return m_tableName; }
        Utf8StringCR GetName() const { return m_name; }
        Type GetType() const { return m_type; }
        Virtual GetVirtual() const { return m_virtual; }
        bool GetNotNullConstraint() const { return m_notNullConstraint; }
        bool GetUniqueConstraint() const { return m_uniqueConstraint; }
        Utf8StringCR GetCheckConstraint() const { return m_checkConstraint; }
        Utf8StringCR GetDefaultConstraint() const { return m_defaultConstraint; }
        Collation GetCollationConstraint() const { return m_collationConstraint; }
        Kind GetKind() const { return m_kind; }
        Nullable<uint32_t> GetOrdinalInPrimaryKey() const { return m_ordinalInPrimaryKey; }
    };

// GTest Format customizations for types not handled by GTest
void PrintTo(Column const&, std::ostream*);
void PrintTo(Column::Type, std::ostream*);
void PrintTo(Column::Kind, std::ostream*);
void PrintTo(Column::Collation, std::ostream*);
void PrintTo(std::vector<Column const*> const&, std::ostream*);

//=======================================================================================
// Use to compare a projection of a Column only, e.g. if a test only wants to assert
// on column name and virtuality.
// @bsiclass                                      Krischan.Eberle                   06/17
//+===============+===============+===============+===============+===============+======
struct ExpectedColumn final
    {
    Utf8String m_name;
    Nullable<Utf8String> m_tableName;
    Nullable<Virtual> m_virtual;
    Nullable<Column::Kind> m_kind;

    Nullable<Column::Type> m_type;
    Nullable<bool> m_notNullConstraint;
    Nullable<bool> m_uniqueConstraint;
    Nullable<Utf8String> m_checkConstraint;
    Nullable<Utf8String> m_defaultConstraint;
    Nullable<Column::Collation> m_collationConstraint;
    Nullable<uint32_t> m_ordinalInPrimaryKey;

    ExpectedColumn() {}
    ExpectedColumn(Utf8StringCR tableName, Utf8StringCR columnName, Virtual isVirtual = Virtual::No) : m_tableName(tableName), m_name(columnName), m_virtual(isVirtual) {}

    bool operator==(Column const& actual) const;
    bool operator!=(Column const& actual) const { return !(*this == actual); }
    };

typedef std::vector<ExpectedColumn> ExpectedColumns;
bool operator==(ExpectedColumns const& lhs, std::vector<Column> const& rhs);
    
// GTest Format customizations for types not handled by GTest
void PrintTo(ExpectedColumn const&, std::ostream*);


//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  06/17
//=======================================================================================    
struct IndexInfo final
    {
    struct WhereClause final
        {
        private:
            Utf8String m_whereClause;

        public:
            WhereClause() {}
            WhereClause(ECN::ECClassId classIdFilter, bool negateClassIdFilter = false) { AppendClassIdFilter(std::vector<ECN::ECClassId>{classIdFilter}, negateClassIdFilter); }
            WhereClause(std::vector<ECN::ECClassId> const& classIdFilter, bool negateClassIdFilter = false) { AppendClassIdFilter(classIdFilter, negateClassIdFilter); }
            WhereClause(bool addNotNullFilter, std::vector<Utf8String> const& indexColumns)
                {
                if (addNotNullFilter)
                    AppendNotNullFilter(indexColumns);
                }

            //! If negateClassIdFilter is false creates an expression like this: ECClassId=classid1 OR ECClassId=classid2 ...
            //! If negateClassIdFilter is true creates an expression like this: ECClassId<>classid1 AND ECClassId<>classid2 ...
            WhereClause& AppendClassIdFilter(std::vector<ECN::ECClassId> const& classIdFilter, bool negateClassIdFilter = false);
            WhereClause& AppendNotNullFilter(std::vector<Utf8String> const& indexColumns);

            bool IsDefined() const { return !m_whereClause.empty(); }
            Utf8StringCR ToDdl() const { return m_whereClause; }

            void Clear() { m_whereClause.clear(); }
        };

    private:
        Utf8String m_name;
        bool m_isUnique;
        Utf8String m_table;
        std::vector<Utf8String> m_indexColumns;
        WhereClause m_whereClause;

    public:
        IndexInfo(Utf8StringCR name, bool isUnique, Utf8StringCR table, Utf8StringCR indexColumn, WhereClause const& whereClause = WhereClause())
            : m_name(name), m_isUnique(isUnique), m_table(table), m_indexColumns(std::vector<Utf8String>{indexColumn}), m_whereClause(whereClause)
            {}

        IndexInfo(Utf8StringCR name, bool isUnique, Utf8StringCR table, std::vector<Utf8String> const& indexColumns, WhereClause const& whereClause = WhereClause())
            : m_name(name), m_isUnique(isUnique), m_table(table), m_indexColumns(indexColumns), m_whereClause(whereClause)
            {}

        Utf8StringCR GetName() const { return m_name; }
        Utf8String ToDdl() const;
    };



//=======================================================================================    
//This is a mirror of the internal MapStrategy used by ECDb and persisted in the DB.
//The values can change, so in that case this enum needs to be updated accordingly.
// @bsiclass                                   Krischan.Eberle                  05/17
//=======================================================================================    
enum class MapStrategy
    {
    NotMapped,
    OwnTable,
    TablePerHierarchy,
    ExistingTable,
    ForeignKeyRelationshipInTargetTable = 10,
    ForeignKeyRelationshipInSourceTable = 11
    };

//=======================================================================================    
//This is a mirror of the internal MapStrategy information used by ECDb and persisted in the DB.
//The values can change, so in that case this struct needs to be updated accordingly.
// @bsiclass                                   Krischan.Eberle                  05/17
//=======================================================================================    
struct MapStrategyInfo
    {
    enum class JoinedTableInfo
        {
        None = 0,
        JoinedTable = 1,
        ParentOfJoinedTable = 2
        };

    struct TablePerHierarchyInfo
        {
        enum class ShareColumnsMode
            {
            No = 0,
            Yes = 1,
            ApplyToSubclassesOnly = 2
            };

        ShareColumnsMode m_sharedColumnsMode = ShareColumnsMode::No;
        int m_maxSharedColumnsBeforeOverflow = -1;
        JoinedTableInfo m_joinedTableInfo = JoinedTableInfo::None;

        TablePerHierarchyInfo() {}
        explicit TablePerHierarchyInfo(JoinedTableInfo jti) : m_joinedTableInfo(jti) {}
        TablePerHierarchyInfo(ShareColumnsMode sharedColumnsMode, int maxSharedColumnsBeforeOverflow, JoinedTableInfo jti) : m_sharedColumnsMode(sharedColumnsMode), m_maxSharedColumnsBeforeOverflow(maxSharedColumnsBeforeOverflow), m_joinedTableInfo(jti) {}

        bool IsUnset() const { return m_sharedColumnsMode == ShareColumnsMode::No && m_joinedTableInfo == JoinedTableInfo::None; }
        bool operator==(TablePerHierarchyInfo const& rhs) const { return m_sharedColumnsMode == rhs.m_sharedColumnsMode && m_maxSharedColumnsBeforeOverflow == rhs.m_maxSharedColumnsBeforeOverflow && m_joinedTableInfo == rhs.m_joinedTableInfo; }
        bool operator!=(TablePerHierarchyInfo const& rhs) const { return !(*this == rhs); }
        };

    private:
        MapStrategy m_strategy = MapStrategy::NotMapped;
        TablePerHierarchyInfo m_tphInfo;
        bool m_exists = false;
    public:

        MapStrategyInfo() {}
        explicit MapStrategyInfo(MapStrategy strat) : m_exists(true), m_strategy(strat) {}
        MapStrategyInfo(MapStrategy strat, TablePerHierarchyInfo const& tphInfo) : m_exists(true), m_strategy(strat), m_tphInfo(tphInfo) {}

        bool operator==(MapStrategyInfo const& rhs) const { return m_exists == rhs.m_exists && m_strategy == rhs.m_strategy && m_tphInfo == rhs.m_tphInfo; }
        bool operator!=(MapStrategyInfo const& rhs) const { return !(*this == rhs); }

        bool Exists() const { return m_exists; }
        MapStrategy GetStrategy() const { return m_strategy; }
        TablePerHierarchyInfo const& GetTphInfo() const { return m_tphInfo; }

        static Utf8CP MapStrategyToString(MapStrategy strat);
    };

void PrintTo(MapStrategy, std::ostream*);
void PrintTo(MapStrategyInfo const&, std::ostream*);
void PrintTo(MapStrategyInfo::TablePerHierarchyInfo const&, std::ostream*);


//=======================================================================================
// @bsiclass                                      Affan.Khan                       05/17
//+===============+===============+===============+===============+===============+======
struct PropertyMap final
    {
    private:
        ECN::ECClassId m_classId;
        Utf8String m_accessString;
        std::vector<Column> m_columns;

    public:
        PropertyMap() {}
        PropertyMap(ECN::ECClassId classId, Utf8StringCR accessString) :m_classId(classId), m_accessString(accessString) {}

        PropertyMap(PropertyMap const& rhs) : m_classId(rhs.m_classId), m_accessString(rhs.m_accessString), m_columns(rhs.m_columns) {}
        PropertyMap(PropertyMap&& rhs) : m_classId(std::move(rhs.m_classId)), m_accessString(std::move(rhs.m_accessString)), m_columns(std::move(rhs.m_columns)) {}
        
        PropertyMap& operator=(PropertyMap const& rhs)
            {
            if (this != &rhs)
                {
                m_classId = rhs.m_classId;
                m_accessString = rhs.m_accessString;
                m_columns = rhs.m_columns;
                }

            return *this;
            }

        PropertyMap& operator=(PropertyMap&& rhs)
            {
            if (this != &rhs)
                {
                m_classId = std::move(rhs.m_classId);
                m_accessString = std::move(rhs.m_accessString);
                m_columns = std::move(rhs.m_columns);
                }

            return *this;
            }

        void AddColumn(Column&& column) { m_columns.push_back(column); }

        bool IsValid() const { return m_classId.IsValid(); }

        ECN::ECClassId GetClassId() const { return m_classId; }
        Utf8StringCR GetAccessString() const { return m_accessString; }
        std::vector<Column> const& GetColumns() const { return m_columns; }
    };

END_ECDBUNITTESTS_NAMESPACE