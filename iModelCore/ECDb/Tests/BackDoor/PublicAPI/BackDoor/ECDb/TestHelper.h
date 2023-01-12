/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECDbTests.h"
#include "TestInfoHolders.h"
#include <Bentley/BeNumerical.h>
#include <Bentley/BeVersion.h>
#include <Bentley/Nullable.h>
#include <json/json.h>
#include <BeRapidJson/BeRapidJson.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
//! Provides testing methods that can be used in the ATPs to test certain aspects of the ECDb APIs
//! using the ASSERT_ macros.
//! Their return values are compatible with the ASSERT_ macros. This allows you to write single-line
//! asserts like
//! TestHelper testHelper(myecdb);
//! ASSERT_EQ(ECSqlStatus::InvalidECSql, testHelper.PrepareECSql("SELECT * FROM")) << "Incomplete FROM clause";
//! ASSERT_EQ(ECSqlStatus::InvalidECSql, testHelper.PrepareECSql("SELECT * FROM Foo WHERE")) << "Incomplete WHERE clause";
//!
//! In order to avoid the ECDb argument for every call to TestHelper, you construct a TestHelper with an ECDb
//! which is then used by each method of TestHelper.
// @bsiclass
//=======================================================================================
struct TestHelper final
    {
    private:
        ECDbCR m_ecdb;

        Column GetColumnFromCurrentRow(Utf8StringCR tableName, Statement&, int columnFieldsStartIndex) const;

    public:
        explicit TestHelper(ECDbCR ecdb) : m_ecdb(ecdb) {}

        //! Runs an isolated schema import. It first creates an ECDb and then imports the specified schema.
        //! @param[in] fileName ECDb file name. If omitted, a file name will be generated
        static BentleyStatus RunSchemaImport(SchemaItem const&, Utf8CP fileName = nullptr);
        //! Runs an isolated schema import. It first creates an ECDb and then imports the specified schemas.
        //! @param[in] fileName ECDb file name. If omitted, a file name will be generated
        static BentleyStatus RunSchemaImport(std::vector<SchemaItem> const&, Utf8CP fileName = nullptr);
        static BentleyStatus RunSchemaImportOneAtATime(std::vector<SchemaItem> const& schemas, Utf8CP fileName = nullptr);
        //! Imports the specified schema into this Test's ECDb
        //! Changes are committed in case of success, and rolled back in case of error
        BentleyStatus ImportSchema(SchemaItem const&, SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::None) const;
        BentleyStatus ImportSchema(ECN::ECSchemaCP, SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::None) const;
        BentleyStatus ImportSchema(ECN::ECSchemaPtr, SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::None) const;
        //! Imports the specified schemas into this Test's ECDb
        //! Changes are committed in case of success, and rolled back in case of error
        BentleyStatus ImportSchemas(std::vector<SchemaItem> const&, SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::None) const;

        ECSqlStatus PrepareECSql(Utf8CP ecsql) const { ECSqlStatement stmt;  return stmt.Prepare(m_ecdb, ecsql); }
        Utf8String ECSqlToSql(Utf8CP ecsql) const;
        DbResult ExecuteECSql(Utf8CP ecsql) const;
        JsonValue ExecuteSelectECSql(Utf8CP ecsql) const;
        DbResult ExecuteInsertECSql(ECInstanceKey&, Utf8CP ecsql) const;
        JsonValue ExecutePreparedECSql(ECSqlStatement& stmt) const;

        BeVersion GetOriginalECXmlVersion(Utf8CP schemaName) const;

        MapStrategyInfo GetMapStrategy(ECN::ECClassId) const;

        PropertyMap GetPropertyMap(AccessString const&) const;

        //! Gets the column for a single column property map.
        //! @return Column for this single column property map. Returns nullptr
        //! if property map couldn't be found or if it maps to more than one column
        Column GetPropertyMapColumn(AccessString const&) const;
        std::vector<Column> GetPropertyMapColumns(AccessString const& propAccessString) const { return GetPropertyMap(propAccessString).GetColumns(); }

        //! Retrieves a mapped table. This is not equivalent to the physical tables in the ECDb file.
        Table GetMappedTable(Utf8StringCR tableName) const;

        bool TableSpaceExists(Utf8CP dbTableSpace) const;

        //! Checks whether a physical table with the specified name exists in the ECDb file
        bool TableExists(Utf8CP dbTableName) const { return m_ecdb.TableExists(dbTableName); }
        //! Checks whether a physical column with the specified name exists in the ECDb file
        bool ColumnExists(Utf8CP dbTableName, Utf8CP dbColumnName) const { return m_ecdb.ColumnExists(dbTableName, dbColumnName); }
        //! Retrieves the physical columns for the specified table from the ECDb file
        std::vector<Utf8String> GetColumnNames(Utf8CP dbTableName) const { bvector<Utf8String> cols; m_ecdb.GetColumns(cols, dbTableName); return std::vector<Utf8String>(cols.begin(), cols.end()); }
        //! Retrieves the physical column count for the specified table from the ECDb file
        int GetColumnCount(Utf8CP dbTableName) const { bvector<Utf8String> cols; m_ecdb.GetColumns(cols, dbTableName); return (int) cols.size(); }

        //! Checks whether the specified physical column is part of a foreign key constraint in the specified physical table
        bool IsForeignKeyColumn(Utf8CP dbTableName, Utf8CP foreignKeyColumn, Utf8CP dbSchemaName = nullptr) const;

        //! Checks whether the specified physical column is part of a foreign key constraint in the specified physical table
        //! onDeleteAction and onUpdateAction must also match
        bool IsForeignKeyColumn(Utf8CP dbTableName, Utf8CP foreignKeyColumn, Utf8CP onDeleteAction, Utf8CP onUpdateAction, Utf8CP dbSchemaName = nullptr) const;

        //! Checks whether the specified physical column is part of a foreign key constraint in the specified physical table
        Utf8String GetForeignKeyConstraintDdl(Utf8CP dbTableName, Utf8CP foreignKeyColumn, Utf8CP dbSchemaName = nullptr) const;

        //!logs the issues if there are any
        Utf8String GetDdl(Utf8CP entityName, Utf8CP dbSchemaName = nullptr, Utf8CP entityType = "table") const;
        Utf8String GetIndexDdl(Utf8StringCR indexName, Utf8CP dbSchemaName = nullptr) const { return GetDdl(indexName.c_str(), dbSchemaName, "index"); }
        bool IndexExists(Utf8StringCR indexName, Utf8CP dbSchemaName = nullptr) const { return !GetDdl(indexName.c_str(), dbSchemaName, "index").empty(); }
        std::vector<Utf8String> GetIndexNamesForTable(Utf8StringCR dbTableName, Utf8CP dbSchemaName = nullptr) const;

        //! Returns the number of non-overlapping occurances of target string in source string
        int GetFrequencyCount(Utf8StringCR source, Utf8StringCR target) const;
    };

//=======================================================================================
//! Misc test utilities
// @bsiclass
//=======================================================================================
struct TestUtilities final
    {
    private:
        TestUtilities() = delete;
        ~TestUtilities() = delete;

    public:
        static BentleyStatus ReadFile(Utf8StringR, BeFileNameCR);
        static BentleyStatus ReadFile(Json::Value&, BeFileNameCR);
        static BentleyStatus ReadFile(rapidjson::Document&, BeFileNameCR);

        static BentleyStatus ParseJson(Json::Value& json, Utf8StringCR jsonStr) { return Json::Reader::Parse(jsonStr, json) ? SUCCESS : ERROR; }
        static BentleyStatus ParseJson(rapidjson::Document& json, Utf8StringCR jsonStr) { return json.Parse<0>(jsonStr.c_str()).HasParseError() ? ERROR : SUCCESS; }
        static Utf8String ToString(rapidjson::Value const& json)
            {
            rapidjson::StringBuffer jsonStrBuf;
            rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStrBuf);
            json.Accept(writer);
            return jsonStrBuf.GetString();
            }

        static Json::Value DbValueToJson(DbValue const& v)
            {
            switch (v.GetValueType())
                {
                    case DbValueType::BlobVal:
                    {
                    Utf8String base64Str;
                    Base64Utilities::Encode(base64Str, (Byte const*) v.GetValueBlob(), (size_t) v.GetValueBytes());
                    return Json::Value(base64Str);
                    }

                    case DbValueType::FloatVal:
                        return Json::Value(v.GetValueDouble());
                    case DbValueType::IntegerVal:
                        return Json::Value(v.GetValueInt64());
                    case DbValueType::TextVal:
                        return Json::Value(v.GetValueText());
                    case DbValueType::NullVal:
                        return Json::Value(Json::ValueType::nullValue);

                    default:
                        BeAssert(false && "Unhandled DbValueType value");
                        return Json::Value(Json::ValueType::nullValue);
                }
            };
        //! Use this method to compare to double values in tests as comparing them directly often fails due to floating point inaccuracies
        static bool Equals(double lhs, double rhs) { return fabs(lhs - rhs) <= BeNumerical::ComputeComparisonTolerance(lhs, rhs); }
    };


//=======================================================================================
//! Utility to populate an ECInstance with random values
// @bsiclass
//=======================================================================================
struct ECInstancePopulator final
    {
    private:
        static void PopulateStructValue(ECN::ECValueR value, ECN::ECClassCR structType);
        static void PopulatePrimitiveValue(ECN::ECValueR ecValue, ECN::PrimitiveType primitiveType, ECN::ECPropertyCP ecProperty);

        static ECN::ECObjectsStatus CopyStruct(ECN::IECInstanceR source, ECN::ECValuesCollectionCR collection, Utf8CP baseAccessPath);
        static ECN::ECObjectsStatus CopyStruct(ECN::IECInstanceR target, ECN::IECInstanceCR structValue, Utf8CP propertyName) { return CopyStruct(target, *ECN::ECValuesCollection::Create(structValue), propertyName); }

    public:
        //! Populates the ECInstance with random values
        static void Populate(ECN::IECInstanceR, bool skipStructs = false, bool skipArrays = false, bool skipReadOnlyProps = false);
    };

END_ECDBUNITTESTS_NAMESPACE

