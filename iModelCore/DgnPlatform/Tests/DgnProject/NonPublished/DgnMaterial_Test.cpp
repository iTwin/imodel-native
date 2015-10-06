/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnMaterial_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnCore/MaterialElement.h>

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
        BeFileName filename = DgnDbTestDgnManager::GetOutputFilePath(L"materials.idgndb");
        BeFileName::BeDeleteFile(filename);

        CreateDgnDbParams params;
        params.SetOverwriteExisting(false);
        DbResult status;
        m_db = DgnDb::CreateDgnDb(&status, filename, params);
        ASSERT_TRUE(m_db != nullptr);
        ASSERT_EQ(BE_SQLITE_OK, status) << status;
        }

    DgnDbR GetDb() const { return *m_db; }

    DgnModelId CreateModel(Utf8CP name)
        {
        auto model = ResourceModel::Create(ResourceModel::CreateParams(*m_db, DgnClassId(m_db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ResourceModel)), name));
        EXPECT_EQ(DgnDbStatus::Success, model->Insert());
        return model->GetModelId();
        }

    DgnMaterial::CreateParams MakeParams(DgnModelId modelId, Utf8StringCR palette, Utf8StringCR name, DgnElementId parent=DgnElementId(), Utf8StringCR descr="")
        {
        static int32_t s_jsonDummy = 0;
        Utf8PrintfString value("value:%d", ++s_jsonDummy);
        return DgnMaterial::CreateParams(*m_db, modelId, palette, name, value, parent, descr);
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
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MaterialTest, CRUD)
    {
    SetupProject();
    DgnModelId mid = CreateModel("MaterialModel");
    auto params = MakeParams(mid, "Palette1", "Material1");
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

#ifdef CHECK_FOR_PARENT_CHILD_CYCLES
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MaterialTest, ParentChildCycles)
    {
    SetupProject();
    DgnModelId mid = CreateModel("MaterialModel");
    auto params = MakeParams(mid, "Palette", "Parent");
    DgnMaterialPtr parent = new DgnMaterial(params);
    parent->Insert();

    DgnElementId parentId = parent->GetElementId();

    parent = GetDb().Elements().GetElement(parentId)->MakeCopy<DgnMaterial>();
    EXPECT_EQ(DgnDbStatus::Success, parent->SetParentId(parentId));

    DgnDbStatus status;
    parent->Update(&status);
    EXPECT_EQ(DgnDbStatus::InvalidParent, status);

    auto childParams = MakeParams(mid, "Palette", "Child");
    DgnMaterialPtr child = new DgnMaterial(childParams);
    child->Insert();

    DgnElementId childId = child->GetElementId();

    child = GetDb().Elements().GetElement(childId)->MakeCopy<DgnMaterial>();
    EXPECT_EQ(DgnDbStatus::Success, child->SetParentId(parentId));
    child->Update(&status);
    EXPECT_EQ(DgnDbStatus::Success, status);

    parent = GetDb().Elements().GetElement(parentId)->MakeCopy<DgnMaterial>();
    EXPECT_EQ(DgnDbStatus::Success, parent->SetParentId(childId));
    parent->Update(&status);
    EXPECT_EQ(DgnDbStatus::InvalidParent, status);
    }
#endif

