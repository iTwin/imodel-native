/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ClassificationSystems/ClassificationSystemsApi.h>
#include "ClassificationSystemsTestsBase.h"

USING_NAMESPACE_CLASSIFICATIONSYSTEMS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
struct ClassificationGroupTestFixture : public ClassificationSystemsTestsBase 
    {
    ClassificationGroupTestFixture() {};
    ~ClassificationGroupTestFixture() {};
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassificationGroupTestFixture, Create_InsertsAndSetsTableSubmodelSuccessfully)
    {
    Dgn::DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    ClassificationTablePtr table = ClassificationSystemsTestsBase::CreateAndInsertTable(db);
    ASSERT_FALSE(table.IsNull()) << "Failed to create table";

    Utf8CP expectedName = "TestClassificationGroup";

    ClassificationGroupPtr group = ClassificationGroup::Create(*table, expectedName);
    ASSERT_TRUE(group.IsValid()) << "Failed to create group";

    ASSERT_TRUE(0 == strcmp(group->GetUserLabel(), expectedName)) << "Group label was set wrong";

    Dgn::DgnDbStatus stat;
    group->Insert(&stat);
    ASSERT_EQ(Dgn::DgnDbStatus::Success, stat) << "Group failed to be inserted to Db";

    Dgn::DgnModelId submodelId = table->GetSubModelId();

    ASSERT_TRUE(submodelId.IsValid()) << "No model set to the Classification Table";

    ASSERT_EQ(group->GetModelId().GetValue(), submodelId.GetValue()) << "Classification was not added as a submodel to Table";

    bvector<Dgn::DgnElementId> classificationgroups = table->MakeClassificationGroupIterator().BuildIdList<Dgn::DgnElementId>();
    ASSERT_EQ(1, classificationgroups.size()) << "Group not found in inserted Classification Table";
    db.SaveChanges();
    }