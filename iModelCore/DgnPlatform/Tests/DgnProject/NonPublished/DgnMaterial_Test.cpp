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
TEST_F(DgnMaterialsTest, TrueColors)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OpenMode::ReadWrite);
    DgnColors& colors = m_project->Colors();

    DgnColors::Color color1(ColorDef(255, 254, 253), "TestName1", "TestBook1");
    DgnTrueColorId colorId = colors.Insert(color1);
    EXPECT_TRUE(colorId.IsValid());
    EXPECT_TRUE(colorId == color1.GetId());

    DgnColors::Color color2(ColorDef(2,3,33), "Color2");
    DgnTrueColorId colorId2 = colors.Insert(color2);
    EXPECT_TRUE(colorId2.IsValid());

    DgnColors::Color color3(ColorDef(2,3,33), "Color3"); // it is legal to have two colors with the same value
    DgnTrueColorId colorId3 = colors.Insert(color3); 
    EXPECT_TRUE(colorId3.IsValid());

    DgnTrueColorId dup3 = colors.Insert(color3); 
    EXPECT_TRUE(!dup3.IsValid());

    DgnColors::Color color4(ColorDef(4,3,33), "Color4");
    DgnTrueColorId colorId4 = colors.Insert(color4); 
    EXPECT_TRUE(colorId4.IsValid());

    DgnColors::Color dupColor(ColorDef(5,54,3), "TestName1", "TestBook1");
    DgnTrueColorId dupColorId = colors.Insert(dupColor);
    EXPECT_TRUE(!dupColorId.IsValid());

    EXPECT_TRUE(4 == colors.MakeIterator().QueryCount());

    int i=0;
    for (auto& it : colors.MakeIterator())
        {
        if (it.GetId() == color1.GetId())
            {
            ++i;
            EXPECT_TRUE(it.GetColor() == color1.GetColor());
            EXPECT_TRUE(it.GetName() == color1.GetName());
            EXPECT_TRUE(it.GetBook() == color1.GetBook());
            }
        else if (it.GetId() == color2.GetId())
            {
            ++i;
            EXPECT_TRUE(it.GetColor() == color2.GetColor());
            EXPECT_TRUE(it.GetName() == color2.GetName());
            EXPECT_TRUE(it.GetBook() == color2.GetBook());
            }
        else if (it.GetId() == color3.GetId())
            {
            ++i;
            EXPECT_TRUE(it.GetColor() == color3.GetColor());
            EXPECT_TRUE(it.GetName() == color3.GetName());
            EXPECT_TRUE(it.GetBook() == color3.GetBook());
            }
        else if (it.GetId() == color4.GetId())
            {
            ++i;
            EXPECT_TRUE(it.GetColor() == color4.GetColor());
            EXPECT_TRUE(it.GetName() == color4.GetName());
            EXPECT_TRUE(it.GetBook() == color4.GetBook());
            }
        else
            EXPECT_TRUE(false); // too many entries in iterator
        }

    EXPECT_TRUE(4 == i);

    EXPECT_TRUE(colorId == colors.FindMatchingColor(color1.GetColor()));

    DgnColors::Color toFind = colors.QueryColor(colorId);
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(toFind.GetId() == color1.GetId());
    EXPECT_TRUE(toFind.GetColor() == color1.GetColor());
    EXPECT_TRUE(toFind.GetName() == color1.GetName());
    EXPECT_TRUE(toFind.GetBook() == color1.GetBook());

    toFind = colors.QueryColorByName("TestName1", "TestBook1");
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(toFind.GetId() == color1.GetId());
    EXPECT_TRUE(toFind.GetColor() == color1.GetColor());
    EXPECT_TRUE(toFind.GetName() == color1.GetName());
    EXPECT_TRUE(toFind.GetBook() == color1.GetBook());
    }

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
