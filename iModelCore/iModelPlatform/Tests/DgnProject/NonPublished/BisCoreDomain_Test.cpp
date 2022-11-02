/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

//========================================================================================
// @bsiclass
//========================================================================================
struct BisCoreDomainTests : public DgnDbTestFixture
{
    Utf8String GetDDL(Utf8CP tableName);
};

//----------------------------------------------------------------------------------------
// @bsimethod
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
// @betest
//---------------------------------------------------------------------------------------
TEST_F(BisCoreDomainTests, ValidateDomainSchemaDDL)
    {
    SetupSeedProject();

    // bis_Element
        {
        Statement statement(*m_db, "PRAGMA TABLE_INFO(" BIS_TABLE(BIS_CLASS_Element) ")");
        int numColumns = 0;
        bvector<Utf8String> expectedColumnNames;
        expectedColumnNames.push_back("Id");
        expectedColumnNames.push_back("ECClassId");
        expectedColumnNames.push_back("FederationGuid");
        expectedColumnNames.push_back("CodeSpecId");
        expectedColumnNames.push_back("CodeScopeId");
        expectedColumnNames.push_back("CodeValue");
        expectedColumnNames.push_back("ModelId");
        expectedColumnNames.push_back("ParentId");
        expectedColumnNames.push_back("ParentRelECClassId");
        expectedColumnNames.push_back("UserLabel");
        expectedColumnNames.push_back("JsonProperties");
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
        ASSERT_TRUE(ddl.Contains("[FederationGuid] BLOB UNIQUE,"));
        ASSERT_TRUE(ddl.Contains("[CodeSpecId] INTEGER NOT NULL,"));
        ASSERT_TRUE(ddl.Contains("[CodeScopeId] INTEGER NOT NULL,"));
        ASSERT_TRUE(ddl.Contains("[CodeValue] TEXT COLLATE NOCASE,"));
        ASSERT_TRUE(ddl.Contains("[LastMod] TIMESTAMP NOT NULL DEFAULT(julianday('now')),"));
        ASSERT_FALSE(ddl.Contains("PRIMARY KEY([Id])"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([CodeSpecId]) REFERENCES [" BIS_TABLE(BIS_CLASS_CodeSpec) "]([Id])"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([CodeScopeId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id])"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ParentId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id])")); // Element API does the "cascade delete"
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ModelId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Model) "]([Id])"));
        ASSERT_FALSE(ddl.Contains("ON DELETE RESTRICT"));
        ASSERT_FALSE(ddl.Contains("ON UPDATE RESTRICT"));
        }

    // bis_DefinitionElement
        {
        Utf8String ddl = GetDDL(BIS_TABLE(BIS_CLASS_DefinitionElement));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ElementId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id]) ON DELETE CASCADE"));
        ASSERT_FALSE(ddl.Contains("ON DELETE RESTRICT"));
        ASSERT_FALSE(ddl.Contains("ON UPDATE RESTRICT"));
        }

    // bis_GeometricElement2d
        {
        Utf8String ddl = GetDDL(BIS_TABLE(BIS_CLASS_GeometricElement2d));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ElementId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id]) ON DELETE CASCADE"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([CategoryId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id])"));
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
        ASSERT_TRUE(ddl.Contains("[Id] INTEGER PRIMARY KEY"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ElementId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id]) ON DELETE CASCADE"));
        }

    // bis_ElementMultiAspect
        {
        Utf8String ddl = GetDDL(BIS_TABLE(BIS_CLASS_ElementMultiAspect));
        ASSERT_TRUE(ddl.Contains("[Id] INTEGER PRIMARY KEY"));
        ASSERT_TRUE(ddl.Contains("FOREIGN KEY([ElementId]) REFERENCES [" BIS_TABLE(BIS_CLASS_Element) "]([Id]) ON DELETE CASCADE"));
        }

    // Validate unique indices
        {
        Statement statement(*m_db, "SELECT sql FROM sqlite_master WHERE type='index' AND sql LIKE 'CREATE UNIQUE INDEX%'");
        bvector<Utf8String> expectedSqlList;
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Element) "]([CodeSpecId], [CodeScopeId], [CodeValue])");

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
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Model)              "]([ECClassId])");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Element)            "]([ECClassId])");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Element)            "]([ParentId]) WHERE ([ParentId] IS NOT NULL)");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Element)            "]([ModelId])");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_Element)            "]([UserLabel]) WHERE ([UserLabel] IS NOT NULL)");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_GeometricElement2d) "]([CategoryId])");
        expectedSqlList.push_back("ON [" BIS_TABLE(BIS_CLASS_GeometricElement3d) "]([CategoryId])");

        for (Utf8StringCR expectedSql : expectedSqlList)
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
// @betest
//---------------------------------------------------------------------------------------
TEST_F(BisCoreDomainTests, ValidateAutoCreatedModels)
    {
    SetupSeedProject();

    DgnModelPtr repositoryModel = m_db->Models().GetModel(DgnModel::RepositoryModelId());
    DgnModelPtr dictionaryModel = m_db->Models().GetModel(DgnModel::DictionaryId());
    ASSERT_TRUE(repositoryModel.IsValid());
    ASSERT_TRUE(dictionaryModel.IsValid());

    ASSERT_TRUE(m_db->GetRepositoryModel().IsValid());
    ASSERT_TRUE(m_db->GetRealityDataSourcesModel().IsValid());

    // make sure that Delete against the root Subject fails
        {
        SubjectCPtr subject = m_db->Elements().GetRootSubject();
        ASSERT_TRUE(subject.IsValid());
        BeTest::SetFailOnAssert(false);
        DgnDbStatus status = subject->Delete();
        BeTest::SetFailOnAssert(true);
        ASSERT_NE(DgnDbStatus::Success, status);
        }

    // make sure that Delete against the RepositoryModel fails
        {
        DgnModelPtr model = m_db->Models().GetModel(DgnModel::RepositoryModelId());
        ASSERT_TRUE(model.IsValid());
        ASSERT_FALSE(model->IsTemplate());
        BeTest::SetFailOnAssert(false);
        DgnDbStatus status = model->Delete();
        BeTest::SetFailOnAssert(true);
        ASSERT_NE(DgnDbStatus::Success, status);
        }

    // make sure that Delete against the DictionaryModel fails
        {
        DgnModelPtr model = m_db->Models().GetModel(DgnModel::DictionaryId());
        ASSERT_TRUE(model.IsValid());
        ASSERT_FALSE(model->IsTemplate());
        BeTest::SetFailOnAssert(false);
        DgnDbStatus status = model->Delete();
        BeTest::SetFailOnAssert(true);
        ASSERT_NE(DgnDbStatus::Success, status);
        }

    // ensure the the root Subject still exists
        {
        m_db->Elements().ClearCache(); 
        SubjectCPtr subject = m_db->Elements().GetRootSubject();
        ASSERT_TRUE(subject.IsValid());
        }
    }

//---------------------------------------------------------------------------------------
// Validate that CodeSpecs were properly inserted by DgnDb::CreateCodeSpecs
// @betest
//---------------------------------------------------------------------------------------
TEST_F(BisCoreDomainTests, ValidateAutoCreatedCodeSpecs)
    {
    SetupSeedProject();

    // Validate CodeSpecs of RepositoryScope
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_NullCodeSpec)->IsRepositoryScope());

    // Validate CodeSpecs of ModelScope
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_AnnotationFrameStyle)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_AnnotationLeaderStyle)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_AnnotationTextStyle)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_AuxCoordSystem2d)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_AuxCoordSystem3d)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_AuxCoordSystemSpatial)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_CategorySelector)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_ColorBook)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_DisplayStyle)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_Drawing)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_DrawingCategory)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_GeometryPart)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_GraphicalType2d)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_LineStyle)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_LinkElement)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_ModelSelector)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_PhysicalMaterial)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_PhysicalType)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_RenderMaterial)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_Sheet)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_SpatialCategory)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_SpatialLocationType)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_TemplateRecipe2d)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_TemplateRecipe3d)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_TextAnnotationSeed)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_Texture)->IsModelScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_ViewDefinition)->IsModelScope());

    // Validate CodeSpecs of ParentElementScope
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_InformationPartitionElement)->IsParentElementScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_SubCategory)->IsParentElementScope());
    ASSERT_TRUE(m_db->CodeSpecs().GetCodeSpec(BIS_CODESPEC_Subject)->IsParentElementScope());
    }
