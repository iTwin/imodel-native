/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/TestHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ECDbTests.h"
#include "TestInfoHolders.h"
#include <Bentley/BeNumerical.h>
#include <json/json.h>
#include <rapidjson/BeRapidJson.h>

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
// @bsiclass                                                 Krischan.Eberle     06/2017
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

        //! Imports the specified schema into this Test's ECDb
        //! Changes are committed in case of success, and rolled back in case of error
        BentleyStatus ImportSchema(SchemaItem const&) const;
        //! Imports the specified schemas into this Test's ECDb
        //! Changes are committed in case of success, and rolled back in case of error
        BentleyStatus ImportSchemas(std::vector<SchemaItem> const&) const;

        ECSqlStatus PrepareECSql(Utf8CP ecsql) const { ECSqlStatement stmt;  return stmt.Prepare(m_ecdb, ecsql); }
        DbResult ExecuteECSql(Utf8CP ecsql) const;
        DbResult ExecuteInsertECSql(ECInstanceKey&, Utf8CP ecsql) const;

        MapStrategyInfo GetMapStrategy(ECN::ECClassId) const;

        PropertyMap GetPropertyMap(AccessString const&) const;

        //! Gets the column for a single column property map.
        //! @return Column for this single column property map. Returns nullptr
        //! if property map couldn't be found or if it maps to more than one column
        Column GetPropertyMapColumn(AccessString const&) const;
        std::vector<Column> GetPropertyMapColumns(AccessString const& propAccessString) const { return GetPropertyMap(propAccessString).GetColumns(); }

        //! Retrieves a mapped table. This is not equivalent to the physical tables in the ECDb file.
        Table GetMappedTable(Utf8StringCR tableName) const;

        //! Checks whether a physical table with the specified name exists in the ECDb file
        bool TableExists(Utf8CP dbTableName) const { return m_ecdb.TableExists(dbTableName); }
        //! Checks whether a physical column with the specified name exists in the ECDb file
        bool ColumnExists(Utf8CP dbTableName, Utf8CP dbColumnName) const { return m_ecdb.ColumnExists(dbTableName, dbColumnName); }
        //! Retrieves the physical columns for the specified table from the ECDb file
        std::vector<Utf8String> GetColumnNames(Utf8CP dbTableName) const { bvector<Utf8String> cols; m_ecdb.GetColumns(cols, dbTableName); return std::vector<Utf8String>(cols.begin(), cols.end()); }
        //! Retrieves the physical column count for the specified table from the ECDb file
        int GetColumnCount(Utf8CP dbTableName) const { bvector<Utf8String> cols; m_ecdb.GetColumns(cols, dbTableName); return (int) cols.size(); }
        //! Checks whether the specified physical column is part of a foreign key constraint in the specified physical table
        bool IsForeignKeyColumn(Utf8CP dbTableName, Utf8CP foreignKeyColumn) const;
        
        //!logs the issues if there are any
        Utf8String GetDdl(Utf8CP entityName, Utf8CP entityType = "table") const;
        Utf8String GetIndexDdl(Utf8StringCR indexName) const { return GetDdl(indexName.c_str(), "index"); }
        bool IndexExists(Utf8StringCR indexName) const { return !GetDdl(indexName.c_str(), "index").empty(); }
        std::vector<Utf8String> GetIndexNamesForTable(Utf8StringCR dbTableName) const;
    };

//=======================================================================================    
//! Misc test utilities
// @bsiclass                                                 Krischan.Eberle     10/2017
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

        //! Use this method to compare to double values in tests as comparing them directly often fails due to floating point inaccuracies
        static bool Equals(double lhs, double rhs) { return fabs(lhs - rhs) <= BeNumerical::ComputeComparisonTolerance(lhs, rhs); }
    };

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     10/2017
//=======================================================================================    
struct ComparableJsonCppValue final
    {
public:
    JsonValueCR m_value;

    explicit ComparableJsonCppValue(JsonValueCR json) : m_value(json) {}

    bool operator==(ComparableJsonCppValue const& rhs) const;
    bool operator!=(ComparableJsonCppValue const& rhs) const { return !(*this == rhs); }
    };

void PrintTo(ComparableJsonCppValue const&, std::ostream*);

//=======================================================================================    
//! Utility to populate an ECInstance with random values
// @bsiclass                                                 Affan.Khan     03/12
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

