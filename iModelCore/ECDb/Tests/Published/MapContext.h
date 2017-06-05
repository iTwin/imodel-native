/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/MapContext.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbPublishedTests.h"
#include <ostream>
#include "DataTable.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                      Affan.Khan                       05/17
//+===============+===============+===============+===============+===============+======
struct MapContext final
    {
    public:
        struct Table;
        struct ClassMap;

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

            friend MapContext;
            private:
                Table const& m_table;
                Utf8String m_name;
                Type m_type;
                bool m_isVirtual;
                int m_ordinal;
                bool m_notNullConstraint;
                bool m_uniqueConstraint;
                Utf8String m_checkConstraint;
                Utf8String m_defaultConstraint;
                Utf8String m_collationConstraint;
                int m_ordinalInPrimaryKey;
                Utf8String m_kind;

            public:
                Column(Table const& table, Utf8StringCR name) :m_table(table), m_name(name) {}
                Table const& GetTable() const { return m_table; }
                Utf8StringCR GetName() const { return m_name; }
                Utf8StringCR GetKind() const { return m_kind; }
                Type GetType() const { return m_type; }
                bool IsVirtual() const { return m_isVirtual; }
                int GetOrdinal() const { return m_ordinal; }
                bool GetNotNullConstraint() const { return m_notNullConstraint; }
                bool GetUniqueConstraint() const { return m_uniqueConstraint; }
                Utf8StringCR GetCheckConstraint() const { return m_checkConstraint; }
                Utf8StringCR GetDefaultConstraint() const { return m_defaultConstraint; }
                Utf8StringCR GetCollationConstraint() const { return m_collationConstraint; }
                int GetOrdinalInPrimaryKey() const { return m_ordinalInPrimaryKey; }
            };

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
                Type m_type;
                Utf8String m_parentTableName;
                Utf8String m_exclusiveRootSchemaName;
                Utf8String m_exclusiveRootClassName;
                std::map<Utf8String, std::unique_ptr<Column>> m_columns;
                std::vector<Column const*> m_columnsOrdered;

            public:
                Table(Utf8CP name, Type type, Utf8CP parentTableName, Utf8CP exclusiveRootSchemaName, Utf8CP exclusiveRootClassName) : m_name(name), m_type(type), m_parentTableName(parentTableName), m_exclusiveRootSchemaName(exclusiveRootSchemaName), m_exclusiveRootClassName(exclusiveRootClassName) {}
                Utf8StringCR GetName() const { return m_name; }
                Type GetType() const { return m_type; }
                Utf8StringCR GetParentTable() const { return m_parentTableName; }
                Utf8StringCR GetExclusiveRootSchema() const { return m_exclusiveRootSchemaName; }
                Utf8StringCR GetExclusiveRootClass() const { return m_exclusiveRootClassName; }

                void AddColumn(std::unique_ptr<Column> column) 
                    {
                    Column* columnP = column.get();
                    m_columns[columnP->GetName()] = std::move(column);
                    m_columnsOrdered.push_back(columnP);
                    }

                std::vector<Column const*> const& GetColumns() const { return m_columnsOrdered; }
                Column const* FindColumn(Utf8CP name) const { auto itor = m_columns.find(Utf8String(name)); return itor != m_columns.end() ? itor->second.get() : nullptr; }
            };

        struct PropertyMap final
            {
            private:
                ClassMap const& m_classMap;
                Utf8String m_accessString;
                std::set<Column const*> m_column;

            public:
                PropertyMap(ClassMap const& classMap, Column const& column, Utf8StringCR accessString) :m_classMap(classMap), m_accessString(accessString) { m_column.insert(&column); }
                ClassMap const& GetClassMap() const { return m_classMap; }
                std::set<Column const*> const& GetColumns() const { return m_column; }
                Utf8StringCR GetAccessString() const { return m_accessString; }
                void AddColumn(Column const& column) { m_column.insert(&column); }
            };

        struct ClassMap final
            {
            private:
                std::map <Utf8String, std::unique_ptr<PropertyMap>> m_propertyMaps;
                std::vector<PropertyMap const*> m_propertyMapsOrdered;
                Utf8String m_name;
                Utf8String m_schema;
            public:
                Utf8String GetFullName() const { return m_schema + ":" + m_name; }
                Utf8StringCR GetName() const { return m_name; }
                Utf8StringCR GetSchemaName() const { return m_schema; }
                std::vector<PropertyMap const*> const& GetPropertyMaps() const { return m_propertyMapsOrdered; }
                PropertyMap const* FindPropertyMap(Utf8CP accessString) const { auto itor = m_propertyMaps.find(Utf8String(accessString)); return itor != m_propertyMaps.end() ? itor->second.get() : nullptr; }
                void AddPropertyMap(std::unique_ptr<PropertyMap> pm)
                    {
                    m_propertyMapsOrdered.push_back(pm.get());
                    m_propertyMaps[pm->GetAccessString()] = std::move(pm);
                    }
            };
    private:
        ECDbCR m_ecdb;
        std::map<Utf8String, std::unique_ptr<ClassMap>> m_classMaps;
        std::map<Utf8String, std::unique_ptr<Table>> m_tables;

    private:
        void LoadTables();
        ClassMap const* LoadClassMap(Utf8CP schemaName, Utf8CP className);

        BentleyStatus LoadColumns(Table& table) const;

    public:
        explicit MapContext(ECDbCR ecdb):m_ecdb(ecdb){}
        PropertyMap const* FindPropertyMap(PropertyAccessString const&);
        PropertyMap const* FindPropertyMap(PropertyAccessString const&, Utf8CP table);
        PropertyMap const* FindPropertyMap(PropertyAccessString const&, Utf8CP table, Utf8CP column);
        ClassMap const* FindClassMap(Utf8CP schemaName, Utf8CP className);
        Column const* FindColumn(Utf8CP tableName, Utf8CP columnName);
        Table const* FindTable(Utf8CP tableName);
        
        void Clear();
    };

#define ASSERT_EXISTS_CLASSMAP(context, schemaName, className)    ASSERT_TRUE(nullptr != context.FindClassMap(schemaName,className))
#define ASSERT_NOTEXISTS_CLASSMAP(context, schemaName, className)   ASSERT_TRUE(nullptr == context.FindClassMap(schemaName,className))

#define ASSERT_EXISTS_PROPERTYMAP(context, accessString)    ASSERT_TRUE(nullptr != context.FindPropertyMap(accessString))
#define ASSERT_NOTEXISTS_PROPERTYMAP(context, accessString)   ASSERT_TRUE(nullptr == context.FindPropertyMap(accessString))

#define ASSERT_EXISTS_PROPERTYMAP_TABLE(context, accessString, tableName)    ASSERT_TRUE(nullptr != context.FindPropertyMap(accessString, tableName))
#define ASSERT_NOTEXISTS_PROPERTYMAP_TABLE(context, accessString, tableName)   ASSERT_TRUE(nullptr == context.FindPropertyMap(accessString, tableName))

#define ASSERT_EXISTS_PROPERTYMAP_COLUMN(context, accessString, tableName, columnName)    ASSERT_TRUE(nullptr != context.FindPropertyMap(accessString, tableName, columnName))
#define ASSERT_NOTEXISTS_PROPERTYMAP_COLUMN(context, accessString, tableName, columnName)   ASSERT_TRUE(nullptr == context.FindPropertyMap(accessString, tableName, columnName))

#define ASSERT_EXISTS_TABLE(context, tableName)    ASSERT_TRUE(nullptr != context.FindTable(tableName))
#define ASSERT_NOTEXISTS_TABLE(context, tableName)   ASSERT_TRUE(nullptr == context.FindTable(tableName))

#define ASSERT_EXISTS_COLUMN(context, tableName, columnName)    ASSERT_TRUE(nullptr != context.FindColumn(tableName, columnName))
#define ASSERT_NOTEXISTS_COLUMN(context, tableName, columnName)   ASSERT_TRUE(nullptr == context.FindColumn(tableName, columnName))


#define ASSERT_COLUMN_IS_VIRTUAL(context, tableName, columnName)    ASSERT_EXISTS_COLUMN(ctx,tableName, columnName);\
                                                            ASSERT_TRUE(ctx.FindColumn(tableName,columnName)->IsVirtual())

#define ASSERT_COLUMN_IS_NOT_VIRTUAL(context, tableName, columnName)    ASSERT_EXISTS_COLUMN(ctx,tableName, columnName);\
                                                            ASSERT_FALSE(ctx.FindColumn(tableName,columnName)->IsVirtual())

#define ASSERT_TABLE_TYPE(context, expectedType, tableName)   ASSERT_EXISTS_TABLE(ctx,tableName);\
                                                         ASSERT_EQ(expectedType, ctx.FindTable(tableName)->GetType())

#define ASSERT_COLUMN_TYPE(context, expectedType, tableName, columnName)    ASSERT_EXISTS_COLUMN(ctx,tableName, columnName);\
                                                            ASSERT_EQ(expectedType, ctx.FindColumn(tableName,columnName)->GetType())

#define ASSERT_COLUMN_COUNT(ctx, expectedCount, tableName)  ASSERT_EXISTS_TABLE(ctx,tableName);\
                                                            ASSERT_EQ(expectedCount, ctx.FindTable(tableName)->GetColumns().size())

#define ASSERT_PROPERTYMAP_COUNT(ctx, expectedCount, schemaName, className)  ASSERT_EXISTS_CLASSMAP(ctx, schemaName, className);\
                                                            ASSERT_EQ(expectedCount, ctx.FindClassMap( schemaName, className)->GetPropertyMaps().size())


// GTest Format customizations for types not handled by GTest
void PrintTo(MapContext::Table::Type, std::ostream*);
void PrintTo(MapContext::Column::Type, std::ostream*);


struct DataFacetComparer
    {
    private:
        DataTable m_baseLine;
        DataTable m_current;
        BeFileName m_dataFile;
        ECDbCR m_ecdb;
        Utf8String m_dataFolder;
        Utf8String m_test;

    private:
        virtual Utf8CP GetSQL() const = 0;
        virtual Utf8CP GetDataSetId() const = 0;
        Utf8String GetDataFilePath() const
            {
            Utf8String str;
            str.append(m_dataFolder).append("/").append(m_test).append(".").append(GetDataSetId()).append(".csv");
            return str;
            }
        BentleyStatus ReadFromDb(DataTable& table)
            {
            Statement stmt;
            if (stmt.Prepare(m_ecdb, GetSQL()) != BE_SQLITE_OK)
                return ERROR;

            SqlAdaptor::Fill(table, stmt, nullptr);
            return SUCCESS;
            }
        BentleyStatus WriteFile(Utf8StringCR content, bool overrideFile)
            {
            BeFileName file(GetDataFilePath());
            if (file.DoesPathExist())
                {
                if (!overrideFile)
                    return ERROR;

                if (file.BeDeleteFile() != BeFileNameStatus::Success)
                    return ERROR;
                }
            BeFile o;
            if (o.Create(file.GetNameUtf8(), true) != BeFileStatus::Success)
                return ERROR;

            uint32_t bytesWritten;
            if (o.Write(&bytesWritten, content.c_str(), (uint32_t) content.size()) != BeFileStatus::Success)
                return ERROR;

            o.Flush();
            return o.Close() != BeFileStatus::Success ? ERROR : SUCCESS;
            }
        BentleyStatus ReadFile(Utf8StringR content, Utf8StringCR path)
            {
            BeFileName bf(path);
            if (!bf.DoesPathExist())
                return ERROR;

            BeFile file;
            if (file.Open(bf.GetNameUtf8(), BeFileAccess::Read) != BeFileStatus::Success)
                return ERROR;

            ByteStream bs;
            if (file.ReadEntireFile(bs) != BeFileStatus::Success)
                return ERROR;

            content.clear();
            content.reserve(bs.GetSize());
            for (auto itor = bs.begin(); itor != bs.end(); ++itor)
                content.push_back((Utf8Char) (*itor));

            return SUCCESS;
            }
        BentleyStatus ReadFromFile(DataTable& table)
            {
            Utf8String content;
            if (ReadFile(content, GetDataFilePath()) != SUCCESS)
                return ERROR;

            return CSVAdaptor::Fill(table, content.c_str());
            }
        DataTable const& GetCurrent() const { return m_current; }
        DataTable const& GetBaseline() const { return m_baseLine; }

    public:
        DataFacetComparer(ECDbCR ecdb, Utf8CP test, Utf8CP directory)
            :m_ecdb(ecdb), m_dataFolder(directory), m_test(test)
            {}
        BentleyStatus Init()
            {
            m_baseLine.Reset();
            m_current.Reset();
            if (ReadFromDb(m_baseLine) != SUCCESS)
                return ERROR;

            return ReadFromFile(m_current);
            }
        BentleyStatus WriteBaselineToDisk()
            {
            m_baseLine.Reset();
            if (ReadFromDb(m_baseLine) != SUCCESS)
                return ERROR;

            Utf8String csvBuffer;
            CSVAdaptor::Fill(csvBuffer, m_baseLine);
            return WriteFile(csvBuffer, true);
            }
        void Assert()
            {
            ASSERT_EQ(SUCCESS, Init());
            ASSERT_TRUE(GetCurrent().IsDefinitionLocked()) << "DataTable/Current was not loaded correctly -> " << GetDataSetId();
            ASSERT_TRUE(GetBaseline().IsDefinitionLocked()) << "DataTable/Baseline was not loaded correctly -> " << GetDataSetId();
            ASSERT_EQ(GetBaseline().GetColumnCount(), GetCurrent().GetColumnCount()) << "Column must match -> " << GetDataSetId();
            ASSERT_EQ(GetBaseline().GetRowCount(), GetCurrent().GetRowCount()) << "Row must match -> " << GetDataSetId();
            const size_t maxColumns = MIN(GetBaseline().GetColumnCount(), GetCurrent().GetColumnCount());
            const size_t maxRows = MIN(GetBaseline().GetRowCount(), GetCurrent().GetRowCount());
            for (size_t row = 0; row < maxRows; ++row)
                {
                DataTable::Row const& currentRow = GetCurrent().GetRow(row);
                DataTable::Row const& baselineRow = GetBaseline().GetRow(row);
                ASSERT_STREQ(baselineRow.ToString().c_str(), currentRow.ToString().c_str());
                for (size_t col = 0; col < maxColumns; ++col)
                    {
                    DataTable::Value const& currentValue = currentRow.GetValue(col);
                    DataTable::Value const& baselineValue = baselineRow.GetValue(col);
                    ASSERT_EQ(baselineValue.GetType(), currentValue.GetType());
                    switch (baselineValue.GetType())
                        {
                            case DataTable::Value::Type::Text:
                            {
                            ASSERT_STREQ(baselineValue.GetText(), currentValue.GetText())
                                << "BaseLine:" << baselineRow.ToString().c_str() << ", Current: " << currentRow.ToString().c_str();
                            break;
                            }
                            case DataTable::Value::Type::Double:
                            {
                            ASSERT_EQ(baselineValue.GetDouble(), currentValue.GetDouble())
                                << "BaseLine:" << baselineRow.ToString().c_str() << ", Current: " << currentRow.ToString().c_str();
                            break;
                            }
                            case DataTable::Value::Type::Integer:
                            {
                            ASSERT_EQ(baselineValue.GetInteger(), currentValue.GetInteger())
                                << "BaseLine:" << baselineRow.ToString().c_str() << ", Current: " << currentRow.ToString().c_str();
                            break;
                            }
                            case DataTable::Value::Type::Blob:
                            {
                            ASSERT_TRUE(false);
                            }
                        }
                    }
                }
            }
    };

struct PropertyMapComparer : DataFacetComparer
    {
    private:
        virtual Utf8CP GetDataSetId()  const override { return "PropertyMap"; }
        virtual Utf8CP GetSQL() const override
            {
            return R"(
                SELECT [S].[Name] [Schema], 
                        [CL].[Name] [Class], 
                        [PP].[AccessString], 
                        [T].[Name] [Table], 
                        [C].[Name] [Column]
                FROM   [ec_ClassMap] [CM]
                        INNER JOIN [ec_PropertyMap] [PM] ON [PM].[ClassId] = [CM].[ClassId]
                        INNER JOIN [ec_PropertyPath] [PP] ON [PP].[Id] = [PM].[PropertyPathId]
                        INNER JOIN [ec_Class] [CL] ON [CL].[Id] = [PM].[ClassId]
                        INNER JOIN [ec_Schema] [S] ON [S].[Id] = [CL].[SchemaId]
                        INNER JOIN [ec_Column] [C] ON [C].[Id] = [PM].[ColumnId]
                        INNER JOIN [ec_Table] [T] ON [T].[Id] = [C].[TableId]
                ORDER  BY [PM].[Id];)";
            }
    public:
        PropertyMapComparer(ECDbCR ecdb, Utf8CP test, Utf8CP directory)
            : DataFacetComparer(ecdb, test, directory)
            {}
    };

struct TableComparer : DataFacetComparer
    {
    private:
        virtual Utf8CP GetDataSetId()  const override { return "Table"; }
        virtual Utf8CP GetSQL() const override
            {
            return R"(
                SELECT [T].[Name], 
                       CASE [T].[Type] WHEN 0 THEN 'Primary' WHEN 1 THEN 'Joined' WHEN 2 THEN 'Existing' WHEN 3 THEN 'Overflow' WHEN 4 THEN 'Virtual' ELSE '<err>' END [Type], 
                       CASE [T].[Type] WHEN 4 THEN 'True' ELSE 'False' END [IsVirtual], 
                       [TP].[Name] [ParentTable], 
                       [S].[Name] [ExclusiveRootSchema], 
                       [C].[Name] [ExclusiveRootClass]
                FROM   [ec_Table] [T]
                       LEFT JOIN [ec_Table] [TP] ON [TP].[Id] = [T].[ParentTableId]
                       LEFT JOIN [ec_Class] [C] ON [C].[Id] = [T].[ExclusiveRootClassId]
                       LEFT JOIN [ec_Schema] [S] ON [S].[Id] = [C].[SchemaId]
                ORDER  BY [T].[Name];)";
            }
    public:
        TableComparer(ECDbCR ecdb, Utf8CP test, Utf8CP directory)
            : DataFacetComparer(ecdb, test, directory)
            {}
    };

struct IndexComparer : DataFacetComparer
    {
    private:
        virtual Utf8CP GetDataSetId()  const override { return "Index"; }
        virtual Utf8CP GetSQL() const override
            {
            return R"(
                SELECT [I].[Name] [Index], 
                       [T].[Name] [Table], 
                       CASE [IsUnique] WHEN 0 THEN 'False' WHEN 1 THEN 'True' END [IsUnique], 
                       CASE [AddNotNullWhereExp] WHEN 0 THEN 'False' WHEN 1 THEN 'True' END [AddNotNullWhereExp], 
                       CASE [IsAutoGenerated] WHEN 0 THEN 'False' WHEN 1 THEN 'True' END [IsAutoGenerated], 
                       CASE [AppliesToSubclassesIfPartial] WHEN 0 THEN 'False' WHEN 1 THEN 'True' END [AppliesToSubclassesIfPartial], 
                       [S].[Name] [RootSchema], 
                       [C].[Name] [RootClass],
                       (SELECT GROUP_CONCAT(C.Name, ', ') from ec_IndexColumn IC INNER JOIN ec_Column C ON C.Id = IC.ColumnId WHERE IC.IndexId=I.Id ORDER BY C.Name) Columns                 
                FROM   [ec_Index] [I]
                       INNER JOIN [ec_Table] [T] ON [T].[Id] = [I].[TableId]
                       LEFT OUTER JOIN [ec_Class] [C] ON [C].[Id] = [I].[ClassId]
                       LEFT OUTER JOIN [ec_Schema] [S] ON [S].[Id] = [C].[SchemaId];
                ORDER BY T.Name, I.Name)";
            }
    public:
        IndexComparer(ECDbCR ecdb, Utf8CP test, Utf8CP directory)
            : DataFacetComparer(ecdb, test, directory)
            {}
    };
struct ColumnComparer : DataFacetComparer
    {
    private:
        virtual Utf8CP GetDataSetId()  const override { return "Column"; }
        virtual Utf8CP GetSQL() const override
            {
            return  R"(
                SELECT [T].[Name] [Table], 
                        [C].[Name] [Column], 
                        CASE [C].[Type] WHEN 0 THEN 'Any' WHEN 1 THEN 'Boolean' WHEN 2 THEN 'Blob' WHEN 3 THEN 'TimeStamp' WHEN 4 THEN 'Real' WHEN 5 THEN 'Integer' WHEN 6 THEN 'Text' ELSE '<err>' END [Type], 
                        CASE [C].[IsVirtual] WHEN 0 THEN 'False' WHEN 1 THEN 'True' ELSE '<err>' END [IsVirtual], 
                        [Ordinal], 
                        CASE [C].[NotNullConstraint] WHEN 1 THEN 'True' WHEN 0 THEN 'False' ELSE '<err>' END [NotNullConstraint], 
                        CASE [C].[UniqueConstraint] WHEN 1 THEN 'True' WHEN 0 THEN 'False' ELSE '<err>' END [UniqueConstraint], 
                        [CheckConstraint], 
                        [DefaultConstraint], 
                        CASE [CollationConstraint] WHEN 0 THEN 'Unset' WHEN 1 THEN 'Binary' WHEN 2 THEN 'NoCase' WHEN 3 THEN 'RTrim' ELSE '<err>' END CollationConstraint, 
                        [OrdinalInPrimaryKey], 
                        (SELECT GROUP_CONCAT ([Name], ' | ')
                FROM   (SELECT [] [Value], 
                                [:1] [Name]
                        FROM   (VALUES (1, 'ECInstanceId')
                                UNION
                                VALUES (2, 'ECClassId')
                                UNION
                                VALUES (4, 'SourceECInstanceId')
                                UNION
                                VALUES (8, 'SourceECClassId')
                                UNION
                                VALUES (16, 'TargetECInstanceId')
                                UNION
                                VALUES (32, 'TargetECClassId')
                                UNION
                                VALUES (64, 'DataColumn')
                                UNION
                                VALUES (128, 'SharedDataColumn')
                                UNION
                                VALUES (256, 'RelECClassId')))
                WHERE  [Value] | [ColumnKind] = [ColumnKind]) [ColumnKind]
                FROM   [ec_Column] [C]
                        INNER JOIN [ec_Table] [T] ON [T].[Id] = [C].[TableId]
                ORDER  BY [C].[TableId],
                            [C].[Id];)";
            }
    public:
        ColumnComparer(ECDbCR ecdb, Utf8CP test, Utf8CP directory)
            : DataFacetComparer(ecdb, test, directory)
            {}

    };
struct ComparerContext
    {
    private:
        static Utf8String GetBaselineFolder()
            {
            return "C:/TEMP/BaseLines/";
            //BeFileName ecdbPath;
            //BeTest::GetHost().GetDocumentsRoot(ecdbPath);
            //ecdbPath.AppendToPath(WString("Baselines", BentleyCharEncoding::Utf8).c_str());
            //return ecdbPath.GetNameUtf8();
            }

        std::vector< std::unique_ptr<DataFacetComparer>> m_comparers;
    public:
        ComparerContext(ECDbCR ecdb, Utf8CP test)
            {
            m_comparers.push_back(std::unique_ptr<DataFacetComparer>(new ColumnComparer(ecdb, test, GetBaselineFolder().c_str())));
            m_comparers.push_back(std::unique_ptr<DataFacetComparer>(new IndexComparer(ecdb, test, GetBaselineFolder().c_str())));
            m_comparers.push_back(std::unique_ptr<DataFacetComparer>(new TableComparer(ecdb, test, GetBaselineFolder().c_str())));
            m_comparers.push_back(std::unique_ptr<DataFacetComparer>(new PropertyMapComparer(ecdb, test, GetBaselineFolder().c_str())));
            }
        BentleyStatus CreateBaseline()
            {
            for (auto& v : m_comparers)
                if (v->WriteBaselineToDisk() != SUCCESS)
                    return ERROR;

            return SUCCESS;
            }

        void Assert()
            {
            for (auto& v : m_comparers)
                v->Assert();
            }
    };
END_ECDBUNITTESTS_NAMESPACE