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
#include <ostream>

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
struct TestHelper
    {
    private:
        struct DbSchemaCache final
            {
            private:
                ECDbCR m_ecdb;
                mutable std::map<ECN::ECClassId, std::unique_ptr<ClassMap>> m_classMaps;
                mutable std::map<Utf8String, std::unique_ptr<Table>> m_tables;

            public:
                explicit DbSchemaCache(ECDbCR ecdb) : m_ecdb(ecdb) {}

                ClassMap const* GetClassMap(ECN::ECClassId) const;
                Table const* GetTable(Utf8StringCR tableName) const;

                void Clear() const { m_classMaps.clear(); m_tables.clear(); }
            };

        ECDbCR m_ecdb;
        DbSchemaCache m_dbSchemaCache;

    public:
        explicit TestHelper(ECDbCR ecdb) : m_ecdb(ecdb), m_dbSchemaCache(ecdb) {}

        void ClearCache()const { m_dbSchemaCache.Clear(); }

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
        DbResult ExecuteNonSelectECSql(Utf8CP ecsql) const;
        DbResult ExecuteInsertECSql(ECInstanceKey&, Utf8CP ecsql) const;

        MapStrategyInfo GetMapStrategy(ECN::ECClassId) const;

        ClassMap const* GetClassMap(Utf8CP schemaNameOrAlias, Utf8CP className) const;
        ClassMap const* GetClassMap(ECN::ECClassId classId) const { return m_dbSchemaCache.GetClassMap(classId); }
        PropertyMap const* GetPropertyMap(AccessString const&) const;

        //! Gets the column for a single column property map.
        //! @return Column for this single column property map. Returns nullptr
        //! if property map couldn't be found or if it maps to more than one column
        Column const* GetPropertyMapColumn(AccessString const&) const;
        std::vector<Column const*> const& GetPropertyMapColumns(AccessString const& propAccessString) const;

        Table const* GetTable(Utf8CP tableName) const { return m_dbSchemaCache.GetTable(tableName); }
        Column const* GetColumn(Utf8CP tableName, Utf8CP columnName) const;

        int GetColumnCount(Utf8CP tableName) const { bvector<Utf8String> cols; m_ecdb.GetColumns(cols, tableName); return (int) cols.size(); }
        bool IsForeignKeyColumn(Utf8CP tableName, Utf8CP foreignKeyColumn) const;
        
        //!logs the issues if there are any
        Utf8String GetDdl(Utf8CP entityName, Utf8CP entityType = "table") const;
        Utf8String GetIndexDdl(Utf8StringCR indexName) const { return GetDdl(indexName.c_str(), "index"); }
        bool IndexExists(Utf8StringCR indexName) const { return !GetDdl(indexName.c_str(), "index").empty(); }
        std::vector<Utf8String> GetIndexNamesForTable(Utf8StringCR tableName) const;
    };

END_ECDBUNITTESTS_NAMESPACE

