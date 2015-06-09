/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/ECDb/ECDbSql_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/NonPublished/ECDb/ECDbTestProject.h>
#include <initializer_list>
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Majd.Uddin                         03/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSql, PartialIndex)
    {
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create ("Testindex.ecdb", L"ECDbSqlIndexTest.01.00.ecschema.xml", false);

    //Verify that one Partial index was created
    BeSQLite::Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT * FROM SQLITE_MASTER WHERE type='index' AND tbl_name='ecdbST_IndexClass' AND name=?"));
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

    ASSERT_EQ(BE_SQLITE_OK, stmt.Reset());
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_InvalidPartial", Statement::MakeCopy::No));
    //IDX_InvalidPartial will be skipped as it has incorrect WHERE clause
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();
    db.CloseDb();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Majd.Uddin                         03/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSql, UniqueIndex)
    {
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("Testindex2.ecdb", L"ECDbSqlIndexTest.01.00.ecschema.xml", false);

    //Verify that one Unique index was created
    BeSQLite::Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT * FROM SQLITE_MASTER WHERE type='index' AND tbl_name='ecdbST_IndexClass2' AND name=?"));
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
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("Testindex3.ecdb", L"ECDbSqlIndexTest.01.00.ecschema.xml", false);

    BeSQLite::Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT * FROM SQLITE_MASTER WHERE type='index' AND tbl_name='ecdbST_IndexClass3' AND name=?"));
    EXPECT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_NoProperty", Statement::MakeCopy::No));
    // Index with No Property specified will not be created
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    
    EXPECT_EQ(BE_SQLITE_OK, stmt.Reset());
    EXPECT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_WrongProperty", Statement::MakeCopy::No));
    // Index with Wrong Property specified will not be created
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());

    EXPECT_EQ(BE_SQLITE_OK, stmt.Reset());
    EXPECT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_Full", Statement::MakeCopy::No));
    // Index with Name of an existing name will not be created
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());

    EXPECT_EQ(BE_SQLITE_OK, stmt.Reset());
    EXPECT_EQ(BE_SQLITE_OK, stmt.BindText(1, "IDX_WrongPropertyArray", Statement::MakeCopy::No));
    // Index with Property packaged in <int> doesn't get created.
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());

    EXPECT_EQ(BE_SQLITE_OK, stmt.Reset());
    EXPECT_EQ(BE_SQLITE_OK, stmt.BindText(1, "ECDbObj_001", Statement::MakeCopy::No)); // There is one with no name, so it will get default name
    // Index with No Name will be created with a default name
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());

    EXPECT_EQ(BE_SQLITE_OK, stmt.Reset());
    EXPECT_EQ(BE_SQLITE_OK, stmt.BindText(1, "SELECT", Statement::MakeCopy::No));
    // Index with reserved word will be created with [] around it e.g. [SELECT]
    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());

    stmt.Finalize();
    db.CloseDb();
    }

END_ECDBUNITTESTS_NAMESPACE
