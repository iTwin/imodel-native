/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/SystemModel_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DPTEST

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    12/2015
//---------------------------------------------------------------------------------------
struct SystemModelTests : public DgnDbTestFixture
{
    DgnModel::Code GetSystemModelCode() const {return DgnModel::CreateModelCode("TestSystemModel");}
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    12/2015
//---------------------------------------------------------------------------------------
TEST_F(SystemModelTests, CRUD)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"SystemModelTests_CRUD.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    SystemModelPtr systemModel = SystemModel::Create(*m_db, GetSystemModelCode());
    ASSERT_TRUE(systemModel.IsValid());

    DgnDbStatus modelInsertStatus = systemModel->Insert();
    ASSERT_EQ(modelInsertStatus, DgnDbStatus::Success);

    TestRequirementPtr requirement = TestRequirement::Create(*m_db, systemModel->GetModelId());
    ASSERT_TRUE(requirement.IsValid());
    requirement->Insert();
    ASSERT_TRUE(requirement->GetElementId().IsValid()) << "Should be able to insert a TestRequirement into a SystemModel";

    TestGroupPtr group = TestGroup::Create(*m_db, systemModel->GetModelId(), m_defaultCategoryId);
    ASSERT_TRUE(group.IsValid());
    BeTest::SetFailOnAssert(false);
    group->Insert();
    BeTest::SetFailOnAssert(true);
    ASSERT_FALSE(group->GetElementId().IsValid()) << "Should NOT be able to insert a TestGroup into a SystemModel";
    }
