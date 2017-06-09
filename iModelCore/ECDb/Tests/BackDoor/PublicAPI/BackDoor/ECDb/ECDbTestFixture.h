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
// @bsiclass                                                 Carole.MacDonald     09/2015
//=======================================================================================    
struct ECDbTestFixture : public ::testing::Test
    {
public:
    //---------------------------------------------------------------------------------------
    // @bsiclass                                   Krischan.Eberle                  07/15
    //+---------------+---------------+---------------+---------------+---------------+------
    struct SchemaItem
        {
        std::vector<Utf8String> m_schemaXmlList;

        explicit SchemaItem(Utf8CP schemaXml) { m_schemaXmlList.push_back(Utf8String(schemaXml)); }
        explicit SchemaItem(std::vector<Utf8String> const& schemaXmlList) : m_schemaXmlList(schemaXmlList) {}

        Utf8String ToString() const
            {
            Utf8String schemaXmlList;
            for (Utf8StringCR schemaXml : m_schemaXmlList)
                {
                schemaXmlList.append(schemaXml).append("\r\n");
                }

            return schemaXmlList;
            }
        };

private:
    static bmap<BeFileName, Utf8String> s_seedECDbs;

    static bool s_isInitialized;

protected:
    ECDb m_ecdb;

    DbResult SetupECDb(Utf8CP ecdbFileName);
    BentleyStatus SetupECDb(Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    BentleyStatus SetupECDb(Utf8CP ecdbFileName, SchemaItem const& schema, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    DbResult ReopenECDb();

    static DbResult CloneECDb(ECDbR clone, Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));


    static BentleyStatus CreateECDbAndImportSchema(SchemaItem const& schema, Utf8CP fileName = nullptr) { ECDb ecdb;  return CreateECDbAndImportSchema(ecdb, schema, fileName); }
    static BentleyStatus CreateECDbAndImportSchema(ECDbR, SchemaItem const&, Utf8CP fileName = nullptr);
    static BentleyStatus ImportSchema(ECDbCR, SchemaItem const&);
    static BentleyStatus ImportSchema(ECDbCR, BeFileNameCR schemaXmlFilePath);

    static BentleyStatus Populate(ECDbCR, ECN::ECSchemaCR, int instanceCountPerClass);
    static BentleyStatus Populate(ECDbCR, int instanceCountPerClass);

    //!logs the issues if there are any
    static bool HasDataCorruptingMappingIssues(ECDbCR);

    BentleyStatus GetInstances (bvector<ECN::IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className);

    static Utf8String RetrieveDdl(ECDbCR ecdb, Utf8CP entityName, Utf8CP entityType = "table");

    ECN::ECSchemaPtr ReadECSchemaFromDisk(ECN::ECSchemaReadContextPtr& ctx, BeFileNameCR schemaFileName) const { return ReadECSchemaFromDisk(ctx, m_ecdb, schemaFileName); }

    static DbResult ExecuteNonSelectECSql(ECDbCR, Utf8CP ecsql);
    static DbResult ExecuteInsertECSql(ECInstanceKey&, ECDbCR, Utf8CP ecsql);

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
    static ECN::ECSchemaPtr ReadECSchemaFromDisk(ECN::ECSchemaReadContextPtr&, ECDbCR, BeFileNameCR schemaFileName);
    static BentleyStatus ReadECSchemaFromString(ECN::ECSchemaReadContextPtr&, ECDbCR, SchemaItem const&);
    };

END_ECDBUNITTESTS_NAMESPACE
