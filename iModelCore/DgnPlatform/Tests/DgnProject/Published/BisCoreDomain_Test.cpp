/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/BisCoreDomain_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Shaun.Sewall                    01/2016
//----------------------------------------------------------------------------------------
struct BisCoreDomainTests : public DgnDbTestFixture
    {
    Utf8String GetDDL(Utf8CP tableName);
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2016
//----------------------------------------------------------------------------------------
Utf8String BisCoreDomainTests::GetDDL(Utf8CP tableName)
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
TEST_F(BisCoreDomainTests, ValidateDomainSchemaDDL)
    {
    SetupSeedProject();

    // dgn_Element
        {
        Statement statement(*m_db, "PRAGMA TABLE_INFO(" BIS_TABLE(BIS_CLASS_Element) ")");
        int numColumns = 0;
        bvector<Utf8String> expectedColumnNames;
        expectedColumnNames.push_back("Id");
        expectedColumnNames.push_back("ECClassId");
        expectedColumnNames.push_back("CodeAuthorityId");
        expectedColumnNames.push_back("CodeNamespace");
        expectedColumnNames.push_back("CodeValue");
        expectedColumnNames.push_back("ModelId");
        expectedColumnNames.push_back("ParentId");
        expectedColumnNames.push_back("UserLabel");
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

        Utf8String ddl = GetDDL(BIS_TABLE(BIS_CLASS_Element));
        ASSERT_TRUE(ddl.Contains("[Id] INTEGER PRIMARY KEY,"));
        ASSERT_TRUE(ddl.Contains("[ECClassId] INTEGER NOT NULL,"));
        ASSERT_TRUE(ddl.Contains("[ParentId] INTEGER,"));
        ASSERT_TRUE(ddl.Contains("[ModelId] INTEGER NOT NULL,"));
        ASSERT_TRUE(ddl.Contains("[CodeAuthorityId] INTEGER NOT NULL,"));
        ASSERT_TRUE(ddl.Contains("[CodeNamespace] TEXT NOT NULL COLLATE NOCASE,"));
        ASSERT_TRUE(ddl.Contains("[CodeValue] TEXT COLLATE NOCASE,"));
        ASSERT_TRUE(ddl.Contains("[LastMod] TIMESTAMP NOT NULL DEFAULT(julianday('now')),"));
        ASSERT_FALSE(ddl.Contains("PRIMARY KEY([Id])"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([CodeAuthorityId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Authority) "]([Id])"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ParentId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id])")); // Element API does the "cascade delete"
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ModelId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Model) "]([Id])"));
        ASSERT_FALSE(ddl.Contains("ON DELETE RESTRICT"));
        ASSERT_FALSE(ddl.Contains("ON UPDATE RESTRICT"));
        }

    // bis_DefinitionElement
        {
        Utf8String ddl = GetDDL(BIS_TABLE(BIS_CLASS_DefinitionElement));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ElementId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id]) ON DELETE CASCADE"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([BaseModel]) REFERENCES [" BIS_TABLE(BIS_CLASS_Model) "]([Id])"));
        ASSERT_FALSE(ddl.Contains("ON DELETE RESTRICT"));
        ASSERT_FALSE(ddl.Contains("ON UPDATE RESTRICT"));
        }

    // bis_GeometricElement2d
        {
        Utf8String ddl = GetDDL(BIS_TABLE(BIS_CLASS_GeometricElement2d));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ElementId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id]) ON DELETE CASCADE"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([CategoryId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id])"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ViewId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id])"));
        ASSERT_FALSE(ddl.Contains("ON DELETE RESTRICT"));
        ASSERT_FALSE(ddl.Contains("ON UPDATE RESTRICT"));
        }

    // bis_GeometricElement3d
        {
        Utf8String ddl = GetDDL(BIS_TABLE(BIS_CLASS_GeometricElement3d));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ElementId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id]) ON DELETE CASCADE"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([CategoryId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id])")); 
        ASSERT_FALSE(ddl.Contains("ON DELETE RESTRICT"));
        ASSERT_FALSE(ddl.Contains("ON UPDATE RESTRICT"));
        }

    // bis_ElementUniqueAspect
        {
        Utf8String ddl = GetDDL(BIS_TABLE(BIS_CLASS_ElementUniqueAspect));
        ASSERT_TRUE(ddl.Contains("[ECInstanceId] INTEGER PRIMARY KEY"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ElementId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id]) ON DELETE CASCADE"));
        }

    // bis_ElementMultiAspect
        {
        Utf8String ddl = GetDDL(BIS_TABLE(BIS_CLASS_ElementMultiAspect));
        ASSERT_TRUE(ddl.Contains("[ECInstanceId] INTEGER PRIMARY KEY"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ElementId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id]) ON DELETE CASCADE"));
        }

    // Validate unique indices
        {
        Statement statement(*m_db, "SELECT sql FROM sqlite_master WHERE type='index' AND sql LIKE 'CREATE UNIQUE INDEX%'");
        bvector<Utf8String> expectedSqlList;
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Model)   "]([CodeAuthorityId], [CodeNamespace], [CodeValue])");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Element) "]([CodeAuthorityId], [CodeNamespace], [CodeValue])");

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
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Authority)          "]([ECClassId])");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Model)              "]([ECClassId])");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Element)            "]([ECClassId])");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Element)            "]([ParentId]) WHERE ([ParentId] IS NOT NULL)");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Element)            "]([ModelId])");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Element)            "]([UserLabel]) WHERE ([UserLabel] IS NOT NULL)");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_GeometricElement2d) "]([CategoryId])");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_GeometricElement2d) "]([ViewId]) WHERE ([ViewId] IS NOT NULL)");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_GeometricElement3d) "]([CategoryId])");

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

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    03/2016
//---------------------------------------------------------------------------------------
TEST_F(BisCoreDomainTests, ValidateAutoCreatedModels)
    {
    SetupSeedProject();

    DgnModelPtr repositoryModel = m_db->Models().GetModel(DgnModel::RepositoryModelId());
    DgnModelPtr dictionaryModel = m_db->Models().GetModel(DgnModel::DictionaryId());
    DgnModelPtr groupInformationModel = m_db->Models().GetModel(DgnModel::GroupInformationId());
    
    ASSERT_TRUE(repositoryModel.IsValid());
    ASSERT_TRUE(dictionaryModel.IsValid());
    ASSERT_TRUE(groupInformationModel.IsValid());

    ASSERT_STREQ(BIS_ECSCHEMA_NAME, repositoryModel->GetCode().GetNamespace().c_str());
    ASSERT_STREQ(BIS_ECSCHEMA_NAME, dictionaryModel->GetCode().GetNamespace().c_str());
    ASSERT_STREQ(BIS_ECSCHEMA_NAME, groupInformationModel->GetCode().GetNamespace().c_str());
    }
