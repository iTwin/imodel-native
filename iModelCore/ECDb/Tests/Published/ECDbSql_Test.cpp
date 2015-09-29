/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbSql_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Majd.Uddin                         03/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ImportSchema(ECDbR ecdb, Utf8CP schemaXml)
    {
    auto schemaCache = ECDbTestUtility::ReadECSchemaFromString(schemaXml);
    if (schemaCache == nullptr)
        return ERROR;

    return ecdb.Schemas().ImportECSchemas(*schemaCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Majd.Uddin                         03/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSql, PartialIndex)
    {
    ECDbTestProject::Initialize();
    ECDb db;
    ASSERT_EQ(BE_SQLITE_OK, ECDbTestUtility::CreateECDb(db, nullptr, L"ecdbmapindextest.ecdb"));

    Utf8CP testSchemaXml = "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "   <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix ='bsca' />"
        "   <ECSchemaReference name='ECDbMap' version='01.00' prefix ='ecdbmap' />"
        "   <ECClass typeName = 'IndexClass' isDomainClass = 'True'>"
        "       <ECCustomAttributes>"
        "       <ClassMap xmlns = 'ECDbMap.01.00'>"
        "           <Indexes>"
        "               <DbIndex>"
        "                   <Name>IDX_Partial</Name>"
        "                   <IsUnique>False</IsUnique>"
        "                   <Properties>"
        "                       <string>PropertyPartialIndex</string>"
        "                   </Properties>"
        "                   <Where>ECDB_NOTNULL</Where>"
        "               </DbIndex>"
        "               <DbIndex>"
        "                   <Name>IDX_Full</Name>"
        "                   <IsUnique>False</IsUnique>"
        "                   <Properties>"
        "                       <string>PropertyFullIndex</string>"
        "                   </Properties>"
        "               </DbIndex>"
        "               <DbIndex>"
        "                   <Name>IDX_PartialMissing</Name>"
        "                   <IsUnique>False</IsUnique>"
        "                   <Properties>"
        "                       <string>PropertyPartialIndex</string>"
        "                   </Properties>"
        "                   <Where></Where>"
        "               </DbIndex>"
        "           </Indexes>"
        "       </ClassMap>"
        "   </ECCustomAttributes>"
        "   <ECProperty propertyName ='PropertyFullIndex' typeName = 'string' />"
        "   <ECProperty propertyName ='PropertyPartialIndex' typeName = 'string' />"
        "   </ECClass>"
        "</ECSchema>";

    ASSERT_EQ(SUCCESS, ImportSchema(db, testSchemaXml));

    //Verify that one Partial index was created
    BeSQLite::Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT * FROM SQLITE_MASTER WHERE type='index' AND tbl_name='ts_IndexClass' AND name=?"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_Partial", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    Utf8String sqlCmd = stmt.GetValueText(4);
    ASSERT_FALSE(sqlCmd.find("WHERE") == std::string::npos) << "IDX_Partial is a partial index and will have WHERE clause";
    //Verify that other index is not Partial as Where was not specified
    ASSERT_EQ(BE_SQLITE_OK, stmt.Reset());
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_Full", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    sqlCmd = stmt.GetValueText(4);
    ASSERT_TRUE(sqlCmd.find("WHERE") == std::string::npos);

    //Verify that index with empty Where clause is treated as not-partial index
    ASSERT_EQ(BE_SQLITE_OK, stmt.Reset());
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_PartialMissing", Statement::MakeCopy::No));
    //IDX_PartialMissing will be skipped as it has empty WHERE clause
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    stmt.Finalize();
    db.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Majd.Uddin                         03/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSql, UniqueIndex)
    {
    ECDbTestProject::Initialize();
    ECDb db;
    ASSERT_EQ(BE_SQLITE_OK, ECDbTestUtility::CreateECDb(db, nullptr, L"ecdbmapindextest.ecdb"));

    Utf8CP testSchemaXml =
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "   <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix ='bsca' />"
        "   <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "<ECClass typeName='IndexClass2' isDomainClass='True'>"
        "   <ECCustomAttributes>"
        "       <ClassMap xmlns = 'ECDbMap.01.00'>"
        "           <Indexes>"
        "               <DbIndex>"
        "                   <Name>IDX_Unique</Name>"
        "                   <IsUnique>True</IsUnique>"
        "                   <Properties>"
        "                       <string>Property2</string>"
        "                   </Properties>"
        "               </DbIndex>"
        "               <DbIndex>"
        "                   <Name>IDX_NotUnique</Name>"
        "                   <IsUnique>False</IsUnique>"
        "                   <Properties>"
        "                       <string>Property2</string>"
        "                   </Properties>"
        "               </DbIndex>"
        "           </Indexes>"
        "       </ClassMap>"
        "   </ECCustomAttributes>"
        "   <ECProperty propertyName='Property1' typeName='string' />"
        "   <ECProperty propertyName='Property2' typeName='string' />"
        "</ECClass>"
        "</ECSchema>";

    ASSERT_EQ(SUCCESS, ImportSchema(db, testSchemaXml));

    //Verify that one Unique index was created
    BeSQLite::Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT * FROM SQLITE_MASTER WHERE type='index' AND tbl_name='ts_IndexClass2' AND name=?"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_Unique", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    Utf8String sqlCmd = stmt.GetValueText(4);
    ASSERT_FALSE(sqlCmd.find("UNIQUE") == std::string::npos) << "IDX_Unique will have UNIQUE clause";

        //Verify that other indexes are not Unique
    ASSERT_EQ(BE_SQLITE_OK, stmt.Reset());
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_NotUnique", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    sqlCmd = stmt.GetValueText(4);
    ASSERT_TRUE(sqlCmd.find("UNIQUE") == std::string::npos);

    stmt.Finalize();
    db.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Majd.Uddin                         03/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSql, IndexErrors)
    {
    ECDbTestProject::Initialize();

    bvector<bpair<Utf8String,BentleyStatus>> testSchemaXmls;
    testSchemaXmls.push_back(bpair<Utf8String, BentleyStatus>(
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "   <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "   <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "<ECClass typeName='IndexClass3' isDomainClass='True'>"
        "   <ECCustomAttributes>"
        "       <ClassMap xmlns='ECDbMap.01.00'>"
        "           <Indexes>"
        "               <DbIndex>"
        "                   <Name>IDX_NoProperty</Name>"
        "                   <IsUnique>False</IsUnique>"
        "               </DbIndex>"
        "           </Indexes>"
        "       </ClassMap>"
        "   </ECCustomAttributes>"
        "   <ECProperty propertyName='PropertyString' typeName='string' />"
        "   <ECProperty propertyName='PropertyInt' typeName='int' />"
        "   <ECProperty propertyName='PropertyDouble' typeName='double' />"
        "</ECClass>"
        "</ECSchema>",
        ERROR));

    testSchemaXmls.push_back(bpair<Utf8String, BentleyStatus>(
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "   <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix ='bsca' />"
        "   <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "<ECClass typeName='IndexClass3' isDomainClass='True'>"
        "   <ECCustomAttributes>"
        "       <ClassMap xmlns='ECDbMap.01.00'>"
        "           <Indexes>"
        "               <DbIndex>"
        "                   <Name>IDX_WrongProperty</Name>"
        "                   <IsUnique>False</IsUnique>"
        "                   <Properties>"
        "                       <string>Property1</string>"
        "                   </Properties>"
        "               </DbIndex>"
        "           </Indexes>"
        "       </ClassMap>"
        "   </ECCustomAttributes>"
        "   <ECProperty propertyName ='PropertyString' typeName = 'string' />"
        "   <ECProperty propertyName ='PropertyInt' typeName = 'int' />"
        "   <ECProperty propertyName ='PropertyDouble' typeName = 'double' />"
        "</ECClass>"
        "</ECSchema>",
        ERROR));

    testSchemaXmls.push_back(bpair<Utf8String, BentleyStatus>(
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "   <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix ='bsca' />"
        "   <ECSchemaReference name='ECDbMap' version='01.00' prefix ='ecdbmap' />"
        "<ECClass typeName = 'IndexClass3' isDomainClass = 'True'>"
        "   <ECCustomAttributes>"
        "       <ClassMap xmlns = 'ECDbMap.01.00'>"
        "           <Indexes>"
        "               <DbIndex>"
        "                   <Name>IDX_WrongPropertyArray</Name>"
        "                   <IsUnique>False</IsUnique>"
        "                   <Properties>"
        "                       <int>PropertyInt</int>"
        "                   </Properties>"
        "               </DbIndex>"
        "           </Indexes>"
        "       </ClassMap>"
        "   </ECCustomAttributes>"
        "   <ECProperty propertyName ='PropertyString' typeName = 'string' />"
        "   <ECProperty propertyName ='PropertyInt' typeName = 'int' />"
        "   <ECProperty propertyName ='PropertyDouble' typeName = 'double' />"
        "</ECClass>"
        "</ECSchema>",
        ERROR));

    testSchemaXmls.push_back(bpair<Utf8String, BentleyStatus>(
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "   <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix ='bsca' />"
        "   <ECSchemaReference name='ECDbMap' version='01.00' prefix ='ecdbmap' />"
        "<ECClass typeName = 'IndexClass3' isDomainClass = 'True'>"
        "   <ECCustomAttributes>"
        "       <ClassMap xmlns = 'ECDbMap.01.00'>"
        "           <Indexes>"
        "               <DbIndex>"
        "                   <Name>SELECT</Name>"
        "                   <IsUnique>False</IsUnique>"
        "                   <Properties>"
        "                       <string>PropertyInt</string>"
        "                   </Properties>"
        "               </DbIndex>"
        "           </Indexes>"
        "       </ClassMap>"
        "   </ECCustomAttributes>"
        "   <ECProperty propertyName ='PropertyString' typeName = 'string' />"
        "   <ECProperty propertyName ='PropertyInt' typeName = 'int' />"
        "   <ECProperty propertyName ='PropertyDouble' typeName = 'double' />"
        "</ECClass>"
        "</ECSchema>",
        SUCCESS));

    int i = 0;
    for (bpair<Utf8String, BentleyStatus> const& pair : testSchemaXmls)
        {
        ECDb db;
        ASSERT_EQ(BE_SQLITE_OK, ECDbTestUtility::CreateECDb(db, nullptr, L"ecdbmapindextest.ecdb"));

        Utf8CP schemaXml = pair.first.c_str();
        BentleyStatus expectedStat = pair.second;
        ASSERT_EQ(expectedStat, ImportSchema(db, schemaXml)) << "Unexpected result for schema import for test schema #" << (i + 1);
        i++;
        }
    }

END_ECDBUNITTESTS_NAMESPACE
