/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/TestHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ECDbTestFixture.h"
#include <ostream>

BEGIN_ECDBUNITTESTS_NAMESPACE

struct IndexInfo;

//=======================================================================================    
//! Provides testing methods that can be used in the ATPs to test certain aspects of the ECDb APIs
//! using the ASSERT_ macros.
//! Their return values are compatible with the ASSERT_ macros.
// @bsiclass                                                 Krischan.Eberle     06/2017
//=======================================================================================    
struct TestHelper
    {
    private:
        TestHelper();
        ~TestHelper();

    public:
        static BentleyStatus ImportSchema(SchemaItem const&, Utf8CP fileName = nullptr);
        static BentleyStatus ImportSchema(ECDbR, SchemaItem const&);
        static BentleyStatus ImportSchemas(std::vector<SchemaItem> const&, Utf8CP fileName = nullptr);
        static BentleyStatus ImportSchemas(ECDbR, std::vector<SchemaItem> const&);

        //!logs the issues if there are any
        static bool HasDataCorruptingMappingIssues(ECDbCR);

        static Utf8String RetrieveDdl(ECDbCR, Utf8CP entityName, Utf8CP entityType = "table");
        static Utf8String RetrieveIndexDdl(ECDbCR ecdb, Utf8StringCR indexName) { return RetrieveDdl(ecdb, indexName.c_str(), "index"); }
        static bool IndexExists(ECDbCR ecdb, IndexInfo const&);
        static std::vector<Utf8String> RetrieveIndexNamesForTable(ECDbCR, Utf8StringCR tableName);
        static ECSqlStatus PrepareECSql(ECDbCR ecdb, Utf8CP ecsql) { ECSqlStatement stmt;  return stmt.Prepare(ecdb, ecsql); }
        static DbResult ExecuteNonSelectECSql(ECDbCR, Utf8CP ecsql);
        static DbResult ExecuteInsertECSql(ECInstanceKey&, ECDbCR, Utf8CP ecsql);
    };

//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  05/17
//=======================================================================================    
struct PropertyAccessString final
    {
    Utf8String m_schemaNameOrAlias;
    Utf8String m_className;
    Utf8String m_propAccessString;

    PropertyAccessString(Utf8CP schemaNameOrAlias, Utf8CP className, Utf8CP propAccessString) : m_schemaNameOrAlias(schemaNameOrAlias), m_className(className), m_propAccessString(propAccessString) {}
    Utf8String ToString() const { Utf8String str; str.Sprintf("%s:%s.%s", m_schemaNameOrAlias.c_str(), m_className.c_str(), m_propAccessString.c_str()); return str; }
    };

//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  05/17
//=======================================================================================    
struct ColumnInfo final
    {
    typedef std::vector<ColumnInfo> List;

    Utf8String m_propAccessString;
    Utf8String m_tableName;
    Utf8String m_columnName;
    bool m_isVirtual = false;

    ColumnInfo(Utf8CP propAccessString, Utf8CP tableName, Utf8CP columnName) : ColumnInfo(propAccessString, tableName, columnName, false) {}
    ColumnInfo(Utf8CP tableName, Utf8CP columnName, bool isVirtual) : ColumnInfo(nullptr, tableName, columnName, isVirtual) {}
    ColumnInfo(Utf8CP tableName, Utf8CP columnName) : ColumnInfo(tableName, columnName, false) {}
    ColumnInfo(Utf8CP propAccessString, Utf8CP tableName, Utf8CP columnName, bool isVirtual) : m_propAccessString(propAccessString), m_tableName(tableName), m_columnName(columnName), m_isVirtual(isVirtual) {}

    bool operator==(ColumnInfo const& rhs) const
        {
        //compare prop access string only if specified on both sides
        if (!m_propAccessString.empty() && !rhs.m_propAccessString.empty() && !m_propAccessString.EqualsIAscii(rhs.m_propAccessString))
            return false;

        return m_tableName.EqualsIAscii(rhs.m_tableName) && m_columnName.EqualsIAscii(rhs.m_columnName) && m_isVirtual == rhs.m_isVirtual;
        }

    bool operator!=(ColumnInfo const& rhs) const { return !(*this == rhs); }

    Utf8String ToString() const
        {
        Utf8String str;
        if (!m_propAccessString.empty())
            str.append("Property: ").append(m_propAccessString).append("|");


        str.append("Column: ").append(m_tableName).append(":").append(m_columnName);
        if (m_isVirtual)
            str.append(" (virtual)");

        return str;
        }
    };

// GTest EQ and Format customizations for types not handled by GTest
void PrintTo(ColumnInfo const&, std::ostream*);
void PrintTo(ColumnInfo::List const&, std::ostream*);
bool operator==(ColumnInfo::List const& lhs, ColumnInfo::List const& rhs);
bool operator!=(ColumnInfo::List const& lhs, ColumnInfo::List const& rhs);

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
            void AppendClassIdFilter(std::vector<ECN::ECClassId> const& classIdFilter, bool negateClassIdFilter = false);
            void AppendNotNullFilter(std::vector<Utf8String> const& indexColumns);

            bool IsDefined() const { return !m_whereClause.empty(); }
            Utf8StringCR ToDdl() const { return m_whereClause; }
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

END_ECDBUNITTESTS_NAMESPACE

