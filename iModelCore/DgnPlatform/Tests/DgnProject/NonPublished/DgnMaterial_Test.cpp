/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnMaterial_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnMaterialsTest : public ::testing::Test
{
public:
    ScopedDgnHost  m_host;
    DgnDbPtr       m_project;

    void SetupProject(WCharCP projFile, Db::OpenMode mode)
        {
        DgnDbTestDgnManager tdm(projFile, __FILE__, mode);
        m_project = tdm.GetDgnProjectP();
        ASSERT_TRUE(m_project != NULL);
        }
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnMaterialsTest, Materials)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OpenMode::ReadWrite);
    DgnMaterials& materials= m_project->Materials();

    DgnMaterials::Material material1("Material1", "Palette1", "descr of mat 1");
    DgnMaterialId materialId = materials.Insert(material1);
    EXPECT_TRUE(materialId.IsValid());
    EXPECT_TRUE(materialId == material1.GetId());

    DgnMaterials::Material material2("Material2", "Palette", "descr of mat 2");
    DgnMaterialId materialId2 = materials.Insert(material2);
    EXPECT_TRUE(materialId2.IsValid());

    DgnMaterials::Material material3("Material3", "Palette", "material material 3");
    DgnMaterialId materialId3 = materials.Insert(material3); 
    EXPECT_TRUE(materialId3.IsValid());

    DgnMaterialId dup3 = materials.Insert(material3); 
    EXPECT_TRUE(!dup3.IsValid());

    DgnMaterials::Material material4("Material3", "Palette2", "material material 3 for palette2");
    DgnMaterialId materialId4 = materials.Insert(material4); 
    EXPECT_TRUE(materialId4.IsValid());

    int i=0;
    for (auto& it : materials.MakeIterator())
        {
        if (it.GetId() == material1.GetId())
            {
            ++i;
            EXPECT_TRUE(it.GetPalette() == material1.GetPalette());
            EXPECT_TRUE(it.GetName() == material1.GetName());
            EXPECT_TRUE(it.GetValue() == material1.GetValue());
            EXPECT_TRUE(it.GetDescr() == material1.GetDescr());
            EXPECT_TRUE(it.GetParentId() == material1.GetParentId());
            }
        else if (it.GetId() == material2.GetId())
            {
            ++i;
            EXPECT_TRUE(it.GetPalette()  == material2.GetPalette());
            EXPECT_TRUE(it.GetName()     == material2.GetName());
            EXPECT_TRUE(it.GetValue()    == material2.GetValue());
            EXPECT_TRUE(it.GetDescr()    == material2.GetDescr());
            EXPECT_TRUE(it.GetParentId() == material2.GetParentId());
            }
        else if (it.GetId() == material3.GetId())
            {
            ++i;
            EXPECT_TRUE(it.GetPalette()  == material3.GetPalette());
            EXPECT_TRUE(it.GetName()     == material3.GetName());
            EXPECT_TRUE(it.GetValue()    == material3.GetValue());
            EXPECT_TRUE(it.GetDescr()    == material3.GetDescr());
            EXPECT_TRUE(it.GetParentId() == material3.GetParentId());
            }
        else if (it.GetId() == material4.GetId())
            {
            ++i;
            EXPECT_TRUE(it.GetPalette()  == material4.GetPalette());
            EXPECT_TRUE(it.GetName()     == material4.GetName());
            EXPECT_TRUE(it.GetValue()    == material4.GetValue());
            EXPECT_TRUE(it.GetDescr()    == material4.GetDescr());
            EXPECT_TRUE(it.GetParentId() == material4.GetParentId());
            }
        else
            EXPECT_TRUE(false); // too many entries in iterator
        }

    EXPECT_TRUE(4 == i);

    DgnMaterials::Material toFind = materials.Query(materialId);
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(toFind.GetId() == material1.GetId());
    EXPECT_TRUE(toFind.GetValue() == material1.GetValue());
    EXPECT_TRUE(toFind.GetName() == material1.GetName());
    EXPECT_TRUE(toFind.GetPalette() == material1.GetPalette());
    EXPECT_TRUE(toFind.GetParentId() == material1.GetParentId());

    auto idfound = materials.QueryMaterialId(material2.GetName(), material2.GetPalette());
    EXPECT_TRUE(idfound == materialId2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnMaterialsTest, Materials_Update)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OpenMode::ReadWrite);
    DgnMaterials& materials= m_project->Materials();

    DgnMaterials::Material material1("Material1", "Palette1", "descr of mat 1");
    DgnMaterialId materialId = materials.Insert(material1);
    EXPECT_TRUE(materialId.IsValid());
    EXPECT_TRUE(materialId == material1.GetId());

    DgnMaterials::Material toFind = materials.Query(materialId);
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(toFind.GetId() == material1.GetId());
    EXPECT_TRUE(toFind.GetValue() == material1.GetValue());
    EXPECT_TRUE(toFind.GetName() == material1.GetName());
    EXPECT_TRUE(toFind.GetPalette() == material1.GetPalette());
    EXPECT_TRUE(toFind.GetParentId() == material1.GetParentId());

    // Update description
    material1.SetDescr("Updated descriptoin of mat 1");
    EXPECT_TRUE(DgnDbStatus::Success == materials.Update(material1));
    toFind = materials.Query(materialId);
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(toFind.GetId() == material1.GetId());
    EXPECT_TRUE(toFind.GetValue() == material1.GetValue());
    EXPECT_TRUE(toFind.GetName() == material1.GetName());
    EXPECT_TRUE(toFind.GetPalette() == material1.GetPalette());
    EXPECT_TRUE(toFind.GetParentId() == material1.GetParentId());

    // update pallete name ( invalid case )
    material1.SetPalette("Updated pallete name of mat 1");
    EXPECT_TRUE(DgnDbStatus::Success == materials.Update(material1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnMaterialsTest, Materials_Delete)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OpenMode::ReadWrite);
    DgnMaterials& materials= m_project->Materials();

    DgnMaterials::Material material1("Material1", "Palette1", "descr of mat 1");
    DgnMaterialId materialId = materials.Insert(material1);
    EXPECT_TRUE(materialId.IsValid());
    EXPECT_TRUE(materialId == material1.GetId());

    DgnMaterials::Material toFind = materials.Query(materialId);
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(toFind.GetId() == material1.GetId());
    EXPECT_TRUE(toFind.GetValue() == material1.GetValue());
    EXPECT_TRUE(toFind.GetName() == material1.GetName());
    EXPECT_TRUE(toFind.GetPalette() == material1.GetPalette());
    EXPECT_TRUE(toFind.GetParentId() == material1.GetParentId());

    // Delete Material
    EXPECT_TRUE(BE_SQLITE_OK == materials.Delete(materialId));

    auto idfound = materials.QueryMaterialId(material1.GetName(), material1.GetPalette());
    EXPECT_TRUE(!idfound.IsValid());
    }
