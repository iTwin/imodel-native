/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ECDbTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  07/15
//=======================================================================================    
struct SchemaItem final
    {
    std::vector<Utf8String> m_schemaXmlList;

    explicit SchemaItem(Utf8CP schemaXml) { m_schemaXmlList.push_back(Utf8String(schemaXml)); }
    explicit SchemaItem(std::vector<Utf8String> const& schemaXmlList) : m_schemaXmlList(schemaXmlList) {}

    Utf8String ToString() const;
    };

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
        static BentleyStatus ImportSchema(ECDbCR, SchemaItem const&);

        //!logs the issues if there are any
        static bool HasDataCorruptingMappingIssues(ECDbCR);

        static Utf8String RetrieveDdl(ECDbCR, Utf8CP entityName, Utf8CP entityType = "table");

        static ECSqlStatus PrepareECSql(ECDbCR ecdb, Utf8CP ecsql) { ECSqlStatement stmt;  return stmt.Prepare(ecdb, ecsql); }
        static DbResult ExecuteNonSelectECSql(ECDbCR, Utf8CP ecsql);
        static DbResult ExecuteInsertECSql(ECInstanceKey&, ECDbCR, Utf8CP ecsql);
    };

//=======================================================================================    
// @bsiclass                                                 Carole.MacDonald     09/2015
//=======================================================================================    
struct ECDbTestFixture : public ::testing::Test
    {
private:
    friend struct TestHelper;

    static bool s_isInitialized;
    static bmap<BeFileName, Utf8String> s_seedECDbs;

protected:
    ECDb m_ecdb;

    DbResult SetupECDb(Utf8CP ecdbFileName);
    BentleyStatus SetupECDb(Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    BentleyStatus SetupECDb(Utf8CP ecdbFileName, SchemaItem const& schema, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    DbResult ReopenECDb();

    static DbResult CloneECDb(ECDbR clone, Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    static BentleyStatus PopulateECDb(ECDbR, ECN::ECSchemaCR, int instanceCountPerClass);
    static BentleyStatus PopulateECDb(ECDbR, int instanceCountPerClass);

    static BentleyStatus ImportSchema(ECDbCR, SchemaItem const&);
    static BentleyStatus ImportSchema(ECDbCR, BeFileNameCR schemaXmlFilePath);


    BentleyStatus GetInstances(bvector<ECN::IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className);

    static BentleyStatus ReadECSchemaFromString(ECN::ECSchemaReadContextPtr&, ECDbCR, SchemaItem const&);
    ECN::ECSchemaPtr ReadECSchemaFromDisk(ECN::ECSchemaReadContextPtr& ctx, BeFileNameCR schemaFileName) const { return ReadECSchemaFromDisk(ctx, m_ecdb, schemaFileName); }
    static ECN::ECSchemaPtr ReadECSchemaFromDisk(ECN::ECSchemaReadContextPtr&, ECDbCR, BeFileNameCR schemaFileName);

public:
    ECDbTestFixture() : ::testing::Test() {}
    virtual ~ECDbTestFixture () {}
    void SetUp() override { Initialize(); }
    void TearDown () override {}

    //! Initializes the test environment by setting up the schema read context and search dirs etc.
    //! Gets implicitly called when calling SetupECDb, too. Tests that don't use
    //! that method can call this method statically.
    static void Initialize();

    static BeFileName BuildECDbPath(Utf8CP ecdbFileName);
    static DbResult CreateECDb(ECDbR, Utf8CP ecdbFileName = nullptr);
    };

END_ECDBUNITTESTS_NAMESPACE
