/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnMaterial_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnCore/DgnMaterial.h>

USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct MaterialTest : public ::testing::Test
{
private:
    ScopedDgnHost   m_host;
    DgnDbPtr        m_db;
protected:
    void SetupProject()
        {
        m_db = CreateDb(L"materials.idgndb");
        ASSERT_TRUE(m_db != nullptr);
        }

    DgnDbPtr CreateDb(WCharCP dbname)
        {
        BeFileName filename = DgnDbTestDgnManager::GetOutputFilePath(dbname);
        BeFileName::BeDeleteFile(filename);

        CreateDgnDbParams params;
        params.SetOverwriteExisting(false);
        DbResult status;
        DgnDbPtr db = DgnDb::CreateDgnDb(&status, filename, params);
        EXPECT_EQ(BE_SQLITE_OK, status) << status;

        return db;
        }

    void ExpectParent(DgnDbR db, Utf8StringCR childName, Utf8StringCR parentName);

    DgnDbR GetDb() const { return *m_db; }

    DgnMaterial::CreateParams MakeParams(Utf8StringCR palette, Utf8StringCR name, DgnMaterialId parent=DgnMaterialId(), Utf8StringCR descr="", DgnDbP pDb=nullptr)
        {
        static int32_t s_jsonDummy = 0;
        Utf8PrintfString value("value:%d", ++s_jsonDummy);
        DgnDbR db = nullptr != pDb ? *pDb : *m_db;
        return DgnMaterial::CreateParams(db, palette, name, value, parent, descr);
        }

    template<typename T, typename U>
    void Compare(T const& a, U const& b)
        {
        EXPECT_EQ(a.GetPaletteName(), b.GetPaletteName());
        EXPECT_EQ(a.GetMaterialName(), b.GetMaterialName());
        EXPECT_EQ(a.GetParentId(), b.GetParentId());
        EXPECT_EQ(a.GetDescr(), b.GetDescr());
        EXPECT_EQ(a.GetValue(), b.GetValue());
        }

    DgnMaterialCPtr CreateMaterial(Utf8StringCR palette, Utf8StringCR name, DgnMaterialId parentId = DgnMaterialId(), DgnDbP pDb = nullptr)
        {
        DgnMaterial material(MakeParams(palette, name, parentId, "", pDb));
        return material.Insert();
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MaterialTest, CRUD)
    {
    SetupProject();
    auto params = MakeParams("Palette1", "Material1");
    DgnMaterialPtr mat = new DgnMaterial(params);
    ASSERT_TRUE(mat.IsValid());
    DgnMaterialCPtr persistent = mat->Insert();
    EXPECT_TRUE(persistent.IsValid());
    EXPECT_TRUE(persistent->GetElementId().IsValid());

    Compare(*mat, *persistent);

    mat->SetDescr("new description");
    mat->SetValue("value:1234");

    DgnMaterialCPtr updatedMat = mat->Update();
    EXPECT_TRUE(updatedMat.IsValid());
    Compare(*mat, *updatedMat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialTest::ExpectParent(DgnDbR db, Utf8StringCR childName, Utf8StringCR parentName)
    {
    DgnMaterialId childId = DgnMaterial::QueryMaterialId("Palette", childName, db),
                  parentId = DgnMaterial::QueryMaterialId("Palette", parentName, db);
    EXPECT_TRUE(childId.IsValid());
    EXPECT_TRUE(parentId.IsValid());

    DgnMaterialCPtr child = DgnMaterial::QueryMaterial(childId, db),
                    parent = DgnMaterial::QueryMaterial(parentId, db);
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
    SetupProject();
    auto params = MakeParams("Palette", "Parent");
    DgnMaterialPtr parent = new DgnMaterial(params);
    parent->Insert();

    DgnElementId parentId = parent->GetElementId();

    // SetParentId() will detect direct cycles (this == this.parent)
    parent = GetDb().Elements().GetElement(parentId)->MakeCopy<DgnMaterial>();
    EXPECT_EQ(DgnDbStatus::InvalidParent, parent->SetParentId(parentId));

    auto childParams = MakeParams("Palette", "Child");
    DgnMaterialPtr child = new DgnMaterial(childParams);
    child->Insert();

    DgnElementId childId = child->GetElementId();

    DgnDbStatus status;
    child = GetDb().Elements().GetElement(childId)->MakeCopy<DgnMaterial>();
    EXPECT_EQ(DgnDbStatus::Success, child->SetParentId(parentId));
    child->Update(&status);
    EXPECT_EQ(DgnDbStatus::Success, status);
    ExpectParent(GetDb(), "Child", "Parent");

    // Child.parent=Parent && Parent.parent=Child:
    //  Child => Parent => Child => ... - caught in Update()
    parent = GetDb().Elements().GetElement(parentId)->MakeCopy<DgnMaterial>();
    EXPECT_EQ(DgnDbStatus::Success, parent->SetParentId(childId));
    parent->Update(&status);
    EXPECT_EQ(DgnDbStatus::InvalidParent, status);

    // Grandchild => Child => Parent - OK.
    parent->SetParentId(DgnElementId());
    auto grandchildParams = MakeParams("Palette", "Grandchild", child->GetMaterialId());
    DgnMaterial grandchild(grandchildParams);
    grandchild.Insert(&status);
    EXPECT_EQ(DgnDbStatus::Success, status);
    ExpectParent(GetDb(), "Grandchild", "Child");
    ExpectParent(GetDb(), "Child", "Parent");

    // Grandchild => Child => Parent => Grandchild - caught in Update()
    EXPECT_EQ(DgnDbStatus::Success, parent->SetParentId(grandchild.GetElementId()));
    parent->Update(&status);
    EXPECT_EQ(DgnDbStatus::InvalidParent, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MaterialTest, ParentChildClone)
    {
    SetupProject();
    DgnDbPtr db2 = CreateDb(L"clonematerials.idgndb");

    Utf8String palette("Palette");
    DgnMaterialCPtr parent = CreateMaterial(palette, "Parent");
    ASSERT_TRUE(parent.IsValid());

    DgnMaterialId parentId = parent->GetMaterialId();
    DgnMaterialCPtr childA = CreateMaterial(palette, "ChildA", parentId),
                    childB = CreateMaterial(palette, "ChildB", parentId);

    ASSERT_TRUE(childA.IsValid());
    ASSERT_TRUE(childB.IsValid());
    EXPECT_EQ(parentId, childA->GetParentMaterialId());
    EXPECT_EQ(parentId, childB->GetParentMaterialId());
    EXPECT_EQ(childA->GetParentMaterial().get(), parent.get());
    EXPECT_EQ(childB->GetParentMaterial().get(), parent.get());

    DgnMaterialCPtr grandchildA = CreateMaterial(palette, "GrandchildA", childA->GetMaterialId());
    ASSERT_TRUE(grandchildA.IsValid());
    EXPECT_EQ(grandchildA->GetParentMaterialId(), childA->GetMaterialId());
    EXPECT_EQ(grandchildA->GetParentMaterial().get(), childA.get());

    DgnMaterialCPtr grandchildB = CreateMaterial(palette, "GrandchildB", childB->GetMaterialId());
    ASSERT_TRUE(grandchildB.IsValid());

    // Importing a child imports its parent. Importing a parent does not import its children.
    DgnImportContext importer(GetDb(), *db2);
    DgnElementCPtr clonedGrandchild = grandchildA->Import(nullptr, db2->GetDictionaryModel(), importer);
    ASSERT_TRUE(clonedGrandchild.IsValid());

    ExpectParent(*db2, "GrandchildA", "ChildA");
    ExpectParent(*db2, "ChildA", "Parent");
    EXPECT_FALSE(DgnMaterial::QueryMaterialId(palette, "ChildB", *db2).IsValid());
    EXPECT_FALSE(DgnMaterial::QueryMaterialId(palette, "GrandchildB", *db2).IsValid());

    // Importing a child when the parent already exists (by Code) associates the child to the existing parent.
    // The version of ChildB in the destination db does not have a parent material.
    DgnMaterialCPtr destChildA = CreateMaterial(palette, "ChildB", DgnMaterialId(), db2.get());
    ASSERT_TRUE(destChildA.IsValid());

    DgnImportContext importer2(GetDb(), *db2);
    DgnMaterialCPtr destGrandchildB = dynamic_cast<DgnMaterialCP>(grandchildB->Import(nullptr, db2->GetDictionaryModel(), importer2).get());
    ASSERT_TRUE(destGrandchildB.IsValid());

    ExpectParent(*db2, "GrandchildB", "ChildB");
    DgnMaterialCPtr destChildB = DgnMaterial::QueryMaterial(DgnMaterial::QueryMaterialId(palette, "ChildB", *db2), *db2);
    EXPECT_FALSE(destChildB->GetParentMaterialId().IsValid());
    }

