/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnBaseDomainSchema_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Shaun.Sewall                    01/2016
//----------------------------------------------------------------------------------------
struct DgnBaseDomainSchemaTests : public DgnDbTestFixture
    {
    Utf8String GetDDL(Utf8CP tableName);
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2016
//----------------------------------------------------------------------------------------
Utf8String DgnBaseDomainSchemaTests::GetDDL(Utf8CP tableName)
    {
    CachedStatementPtr statement;
    m_db->GetCachedStatement(statement, "SELECT sql FROM sqlite_master WHERE name=?");
    statement->BindText(1, tableName, Statement::MakeCopy::No);
    DbResult stepStatus = statement->Step();
    BeAssert(BE_SQLITE_ROW == stepStatus);
    return statement->GetValueText(0);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnBaseDomainSchemaTests, ValidateDomainSchemaDDL)
    {
    SetupSeedProject();

    // dgn_Element
        {
        Statement statement(*m_db, "PRAGMA TABLE_INFO(" DGN_TABLE(DGN_CLASSNAME_Element) ")");
        int numColumns = 0;
        bvector<Utf8String> expectedColumnNames;
        expectedColumnNames.push_back("Id");
        expectedColumnNames.push_back("ECClassId");
        expectedColumnNames.push_back("Code_AuthorityId");
        expectedColumnNames.push_back("Code_Namespace");
        expectedColumnNames.push_back("Code_Value");
        expectedColumnNames.push_back("ModelId");
        expectedColumnNames.push_back("ParentId");
        expectedColumnNames.push_back("Label");
        expectedColumnNames.push_back("UserProperties");
        expectedColumnNames.push_back("LastMod");

        while (BE_SQLITE_ROW == statement.Step())
            {
            ++numColumns;
            Utf8String columnName = statement.GetValueText(1);
            bool found = false;

            for (Utf8String expectedColumnName : expectedColumnNames)
                {
                if (expectedColumnName.Equals(columnName))
                    {
                    found = true;
                    break;
                    }
                }

            ASSERT_TRUE(found);
            }

        ASSERT_EQ(numColumns, expectedColumnNames.size());

        Utf8String ddl = GetDDL(DGN_TABLE(DGN_CLASSNAME_Element));
        ASSERT_TRUE(ddl.Contains("[Id] INTEGER PRIMARY KEY,"));
        ASSERT_TRUE(ddl.Contains("[ECClassId] INTEGER NOT NULL,"));
        ASSERT_TRUE(ddl.Contains("[ParentId] INTEGER,"));
        ASSERT_TRUE(ddl.Contains("[ModelId] INTEGER NOT NULL,"));
        ASSERT_TRUE(ddl.Contains("[Code_AuthorityId] INTEGER NOT NULL,"));
        ASSERT_TRUE(ddl.Contains("[Code_Namespace] TEXT NOT NULL COLLATE NOCASE,"));
        ASSERT_TRUE(ddl.Contains("[Code_Value] TEXT COLLATE NOCASE,"));
        ASSERT_TRUE(ddl.Contains("[LastMod] TIMESTAMP NOT NULL DEFAULT(julianday('now')),"));
        ASSERT_FALSE(ddl.Contains("PRIMARY KEY([Id])"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([Code_AuthorityId]) REFERENCES [dgn_Authority]([Id])"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ParentId]) REFERENCES [dgn_Element]([Id])")); // Element API does the "cascade delete"
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ModelId]) REFERENCES [dgn_Model]([Id])"));
        ASSERT_FALSE(ddl.Contains("ON DELETE RESTRICT"));
        ASSERT_FALSE(ddl.Contains("ON UPDATE RESTRICT"));
        }

    // dgn_DefinitionElement
        {
        Utf8String ddl = GetDDL(DGN_TABLE(DGN_CLASSNAME_DefinitionElement));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ElementId]) REFERENCES [dgn_Element]([Id]) ON DELETE CASCADE"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([BaseModelId]) REFERENCES [dgn_Model]([Id])"));
        ASSERT_FALSE(ddl.Contains("ON DELETE RESTRICT"));
        ASSERT_FALSE(ddl.Contains("ON UPDATE RESTRICT"));
        }

    // dgn_GeometricElement2d
        {
        Utf8String ddl = GetDDL(DGN_TABLE(DGN_CLASSNAME_GeometricElement2d));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ElementId]) REFERENCES [dgn_Element]([Id]) ON DELETE CASCADE"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([CategoryId]) REFERENCES [dgn_Element]([Id])"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ViewId]) REFERENCES [dgn_Element]([Id])"));
        ASSERT_FALSE(ddl.Contains("ON DELETE RESTRICT"));
        ASSERT_FALSE(ddl.Contains("ON UPDATE RESTRICT"));
        }

    // dgn_GeometricElement3d
        {
        Utf8String ddl = GetDDL(DGN_TABLE(DGN_CLASSNAME_GeometricElement3d));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ElementId]) REFERENCES [dgn_Element]([Id]) ON DELETE CASCADE"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([CategoryId]) REFERENCES [dgn_Element]([Id])")); 
        ASSERT_FALSE(ddl.Contains("ON DELETE RESTRICT"));
        ASSERT_FALSE(ddl.Contains("ON UPDATE RESTRICT"));
        }

    // Validate unique indices
        {
        Statement statement(*m_db, "SELECT sql FROM sqlite_master WHERE type='index' AND sql LIKE 'CREATE UNIQUE INDEX%'");
        bvector<Utf8String> expectedSqlList;
        expectedSqlList.push_back("ON [" DGN_TABLE(DGN_CLASSNAME_Model)   "]([Code_AuthorityId], [Code_Namespace], [Code_Value])");
        expectedSqlList.push_back("ON [" DGN_TABLE(DGN_CLASSNAME_Element) "]([Code_AuthorityId], [Code_Namespace], [Code_Value])");

        for (Utf8String expectedSql : expectedSqlList)
            {
            bool found = false;

            while (BE_SQLITE_ROW == statement.Step())
                {
                Utf8String sql = statement.GetValueText(0);
                if (sql.EndsWith(expectedSql))
                    {
                    found = true;
                    break;
                    }
                }

            ASSERT_TRUE(found);
            statement.Reset();
            }
        }

    // Validate indices
        {
        Statement statement(*m_db, "SELECT sql FROM sqlite_master WHERE type='index' AND sql LIKE 'CREATE INDEX%'");
        bvector<Utf8String> expectedSqlList;
        expectedSqlList.push_back("ON [" DGN_TABLE(DGN_CLASSNAME_Authority)          "]([ECClassId])");
        expectedSqlList.push_back("ON [" DGN_TABLE(DGN_CLASSNAME_Model)              "]([ECClassId])");
        expectedSqlList.push_back("ON [" DGN_TABLE(DGN_CLASSNAME_Element)            "]([ECClassId])");
        expectedSqlList.push_back("ON [" DGN_TABLE(DGN_CLASSNAME_Element)            "]([ParentId]) WHERE ([ParentId] IS NOT NULL)");
        expectedSqlList.push_back("ON [" DGN_TABLE(DGN_CLASSNAME_Element)            "]([ModelId])");
        expectedSqlList.push_back("ON [" DGN_TABLE(DGN_CLASSNAME_Element)            "]([Label]) WHERE ([Label] IS NOT NULL)");
        expectedSqlList.push_back("ON [" DGN_TABLE(DGN_CLASSNAME_GeometricElement2d) "]([CategoryId])");
        expectedSqlList.push_back("ON [" DGN_TABLE(DGN_CLASSNAME_GeometricElement2d) "]([ViewId]) WHERE ([ViewId] IS NOT NULL)");
        expectedSqlList.push_back("ON [" DGN_TABLE(DGN_CLASSNAME_GeometricElement3d) "]([CategoryId])");

        for (Utf8String expectedSql : expectedSqlList)
            {
            bool found = false;

            while (BE_SQLITE_ROW == statement.Step())
                {
                Utf8String sql = statement.GetValueText(0);
                if (sql.EndsWith(expectedSql))
                    {
                    found = true;
                    break;
                    }
                }

            ASSERT_TRUE(found);
            statement.Reset();
            }
        }
    }
