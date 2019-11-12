/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnMaterial.h>

USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct MaterialTest : public DgnDbTestFixture
{
    void ExpectParent(DefinitionModelR model, Utf8StringCR paletteName, Utf8StringCR childName, Utf8StringCR parentName);

    template<typename T, typename U>
    void Compare(T const& a, U const& b)
        {
        EXPECT_EQ(a.GetPaletteName(), b.GetPaletteName());
        EXPECT_EQ(a.GetMaterialName(), b.GetMaterialName());
        EXPECT_EQ(a.GetParentId(), b.GetParentId());
        }

    RenderMaterialCPtr InsertMaterial(DefinitionModelR model, Utf8StringCR palette, Utf8StringCR name, RenderMaterialId parentId = RenderMaterialId())
        {
        RenderMaterial material(model, palette, name, parentId);
        return material.Insert();
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MaterialTest, CRUD)
    {
    SetupSeedProject();
    DefinitionModelR dictionary = m_db->GetDictionaryModel();

    RenderMaterialPtr mat = new RenderMaterial(dictionary, "Palette1", "Material1");
    ASSERT_TRUE(mat.IsValid());
    RenderMaterialCPtr persistent = mat->Insert();
    EXPECT_TRUE(persistent.IsValid());
    EXPECT_TRUE(persistent->GetElementId().IsValid());

    Compare(*mat, *persistent);

    RenderMaterialCPtr updatedMat = mat->Update();
    EXPECT_TRUE(updatedMat.IsValid());
    Compare(*mat, *updatedMat);

    EXPECT_TRUE(DgnDbStatus::DeletionProhibited == updatedMat->Delete());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialTest::ExpectParent(DefinitionModelR model, Utf8StringCR paletteName, Utf8StringCR childName, Utf8StringCR parentName)
    {
    RenderMaterialId childId = RenderMaterial::QueryMaterialId(model, paletteName, childName),
                  parentId = RenderMaterial::QueryMaterialId(model, paletteName, parentName);
    EXPECT_TRUE(childId.IsValid());
    EXPECT_TRUE(parentId.IsValid());

    DgnDbR db = model.GetDgnDb();
    RenderMaterialCPtr child = RenderMaterial::Get(db, childId),
                    parent = RenderMaterial::Get(db, parentId);
    EXPECT_TRUE(child.IsValid());
    EXPECT_TRUE(parent.IsValid());
    if (child.IsValid() && parent.IsValid())
        EXPECT_EQ(child->GetParentMaterialId(), parent->GetMaterialId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MaterialTest, ParentChildCycles)
    {
    SetupSeedProject();
    DefinitionModelR dictionary = m_db->GetDictionaryModel();

    RenderMaterialPtr parent = new RenderMaterial(dictionary, "Palette", "Parent");
    parent->Insert();

    DgnElementId parentId = parent->GetElementId();
    DgnClassId parentRelClassId = GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);

    // SetParentId() will detect direct cycles (this == this.parent)
    parent = GetDgnDb().Elements().GetElement(parentId)->MakeCopy<RenderMaterial>();
    EXPECT_EQ(DgnDbStatus::InvalidParent, parent->SetParentId(parentId, parentRelClassId));

    RenderMaterialPtr child = new RenderMaterial(dictionary, "Palette", "Child");
    child->Insert();

    DgnElementId childId = child->GetElementId();

    DgnDbStatus status;
    child = GetDgnDb().Elements().GetElement(childId)->MakeCopy<RenderMaterial>();
    EXPECT_EQ(DgnDbStatus::Success, child->SetParentId(parentId, parentRelClassId));
    child->Update(&status);
    EXPECT_EQ(DgnDbStatus::Success, status);
    ExpectParent(dictionary, "Palette", "Child", "Parent");

    // Child.parent=Parent && Parent.parent=Child:
    //  Child => Parent => Child => ... - caught in Update()
    parent = GetDgnDb().Elements().GetElement(parentId)->MakeCopy<RenderMaterial>();
    EXPECT_EQ(DgnDbStatus::Success, parent->SetParentId(childId, parentRelClassId));
    parent->Update(&status);
    EXPECT_EQ(DgnDbStatus::InvalidParent, status);

    // Grandchild => Child => Parent - OK.
    parent->SetParentId(DgnElementId(), DgnClassId());
    RenderMaterial grandchild(dictionary, "Palette", "Grandchild", child->GetMaterialId());
    grandchild.Insert(&status);
    EXPECT_EQ(DgnDbStatus::Success, status);
    ExpectParent(dictionary, "Palette", "Grandchild", "Child");
    ExpectParent(dictionary, "Palette", "Child", "Parent");

    // Grandchild => Child => Parent => Grandchild - caught in Update()
    EXPECT_EQ(DgnDbStatus::Success, parent->SetParentId(grandchild.GetElementId(), parentRelClassId));
    parent->Update(&status);
    EXPECT_EQ(DgnDbStatus::InvalidParent, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MaterialTest, ParentChildClone)
    {
    SetupSeedProject();
    DgnDbPtr db2 = DgnPlatformSeedManager::OpenSeedDbCopy(DgnDbTestFixture::s_seedFileInfo.fileName, L"clonematerials.bim");
    DefinitionModelR dictionary = m_db->GetDictionaryModel();
    DefinitionModelR dictionary2 = db2->GetDictionaryModel();

    Utf8String palette("Palette");
    RenderMaterialCPtr parent = InsertMaterial(dictionary, palette, "Parent");
    ASSERT_TRUE(parent.IsValid());

    RenderMaterialId parentId = parent->GetMaterialId();
    RenderMaterialCPtr childA = InsertMaterial(dictionary, palette, "ChildA", parentId),
                    childB = InsertMaterial(dictionary, palette, "ChildB", parentId);

    ASSERT_TRUE(childA.IsValid());
    ASSERT_TRUE(childB.IsValid());
    EXPECT_EQ(parentId, childA->GetParentMaterialId());
    EXPECT_EQ(parentId, childB->GetParentMaterialId());
    EXPECT_EQ(childA->GetParentMaterial().get(), parent.get());
    EXPECT_EQ(childB->GetParentMaterial().get(), parent.get());

    RenderMaterialCPtr grandchildA = InsertMaterial(dictionary, palette, "GrandchildA", childA->GetMaterialId());
    ASSERT_TRUE(grandchildA.IsValid());
    EXPECT_EQ(grandchildA->GetParentMaterialId(), childA->GetMaterialId());
    EXPECT_EQ(grandchildA->GetParentMaterial().get(), childA.get());

    RenderMaterialCPtr grandchildB = InsertMaterial(dictionary, palette, "GrandchildB", childB->GetMaterialId());
    ASSERT_TRUE(grandchildB.IsValid());

    // Importing a child imports its parent. Importing a parent does not import its children.
    DgnImportContext importer(GetDgnDb(), *db2);
    DgnElementCPtr clonedGrandchild = grandchildA->Import(nullptr, dictionary2, importer);
    ASSERT_TRUE(clonedGrandchild.IsValid());

    ExpectParent(dictionary2, palette, "GrandchildA", "ChildA");
    ExpectParent(dictionary2, palette, "ChildA", "Parent");
    EXPECT_FALSE(RenderMaterial::QueryMaterialId(dictionary2, palette, "ChildB").IsValid());
    EXPECT_FALSE(RenderMaterial::QueryMaterialId(dictionary2, palette, "GrandchildB").IsValid());

    // Importing a child when the parent already exists (by Code) associates the child to the existing parent.
    // The version of ChildB in the destination db does not have a parent material.
    RenderMaterialCPtr destChildA = InsertMaterial(dictionary2, palette, "ChildB");
    ASSERT_TRUE(destChildA.IsValid());

    DgnImportContext importer2(GetDgnDb(), *db2);
    RenderMaterialCPtr destGrandchildB = dynamic_cast<RenderMaterialCP>(grandchildB->Import(nullptr, dictionary2, importer2).get());
    ASSERT_TRUE(destGrandchildB.IsValid());

    ExpectParent(dictionary2, palette, "GrandchildB", "ChildB");
    RenderMaterialCPtr destChildB = RenderMaterial::Get(*db2, RenderMaterial::QueryMaterialId(dictionary2, palette, "ChildB"));
    EXPECT_FALSE(destChildB->GetParentMaterialId().IsValid());

    db2->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MaterialTest, Iterate)
    {
    SetupSeedProject();
    DefinitionModelR dictionary = m_db->GetDictionaryModel();

    RenderMaterialCPtr mat1 = InsertMaterial(dictionary, "Palette1", "Material1");
    ASSERT_TRUE(mat1.IsValid());
    RenderMaterialCPtr mat2 = InsertMaterial(dictionary, "Palette1", "Material2");
    ASSERT_TRUE(mat2.IsValid());
    RenderMaterialCPtr mat3 = InsertMaterial(dictionary, "Palette2", "Material3");
    ASSERT_TRUE(mat3.IsValid());
    RenderMaterialCPtr mat4 = InsertMaterial(dictionary, "Palette3", "Material4");
    ASSERT_TRUE(mat4.IsValid());
    RenderMaterialCPtr mat5 = InsertMaterial(dictionary, "Palette4", "Material5");
    ASSERT_TRUE(mat5.IsValid());
    
    int count = 0;

    for (RenderMaterial::Entry entry : RenderMaterial::MakeIterator(*m_db))
        {
        if (entry.GetId() == mat1->GetMaterialId())
            {
            EXPECT_STREQ(mat1->GetMaterialName().c_str(), entry.GetName());
            EXPECT_STREQ(mat1->GetPaletteName().c_str(), entry.GetPalette());
            }
        else if (entry.GetId() == mat2->GetMaterialId())
            {
            EXPECT_STREQ(mat2->GetMaterialName().c_str(), entry.GetName());
            EXPECT_STREQ(mat2->GetPaletteName().c_str(), entry.GetPalette());
            }
        else if (entry.GetId() == mat3->GetMaterialId())
            {
            EXPECT_STREQ(mat3->GetMaterialName().c_str(), entry.GetName());
            EXPECT_STREQ(mat3->GetPaletteName().c_str(), entry.GetPalette());
            }
        else if (entry.GetId() == mat4->GetMaterialId())
            {
            EXPECT_STREQ(mat4->GetMaterialName().c_str(), entry.GetName());
            EXPECT_STREQ(mat4->GetPaletteName().c_str(), entry.GetPalette());
            }
        else if (entry.GetId() == mat5->GetMaterialId())
        {
            EXPECT_STREQ(mat5->GetMaterialName().c_str(), entry.GetName());
            EXPECT_STREQ(mat5->GetPaletteName().c_str(), entry.GetPalette());
        }
        else
            FAIL() << "This material should not exist";

            count++;
        }
    ASSERT_EQ(5, count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MaterialTest, Iterate_WithFilter)
    {
    SetupSeedProject();
    DefinitionModelR dictionary = m_db->GetDictionaryModel();

    RenderMaterialCPtr mat1 = InsertMaterial(dictionary, "Palette1", "Material1");
    ASSERT_TRUE(mat1.IsValid());
    RenderMaterialCPtr mat2 = InsertMaterial(dictionary, "Palette1", "Material2");
    ASSERT_TRUE(mat2.IsValid());
    RenderMaterialCPtr mat3 = InsertMaterial(dictionary, "Palette2", "Material3", mat2->GetMaterialId());
    ASSERT_TRUE(mat3.IsValid());

    int count = 0;
    for (RenderMaterial::Entry entry : RenderMaterial::MakeIterator(*m_db, RenderMaterial::Iterator::Options::ByPalette("Palette1")))
        {
        if (entry.GetId() == mat1->GetMaterialId())
            EXPECT_STREQ(mat1->GetMaterialName().c_str(), entry.GetName());
        else if (entry.GetId() == mat2->GetMaterialId())
            EXPECT_STREQ(mat2->GetMaterialName().c_str(), entry.GetName());
        else
            FAIL() << "This material should not exisit";

            count++;
        }
    ASSERT_EQ(2, count);

    count = 0;
    for (RenderMaterial::Entry entry : RenderMaterial::MakeIterator(*m_db, RenderMaterial::Iterator::Options::ByParentId(mat2->GetMaterialId())))
        {
        if (entry.GetId() == mat3->GetMaterialId())
            EXPECT_STREQ(mat3->GetMaterialName().c_str(), entry.GetName());
        else
            FAIL() << "This material should not exist";

        count++;
        }
    ASSERT_EQ(1, count);
    }
