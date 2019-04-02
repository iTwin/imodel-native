#include <Bentley/BeTest.h>
#include <ClassificationSystems/ClassificationSystemsApi.h>
#include "ClassificationSystemsTestsBase.h"

USING_NAMESPACE_CLASSIFICATIONSYSTEMS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
struct ClassificationTestFixture : public ClassificationSystemsTestsBase 
    {
    ClassificationTestFixture() {};
    ~ClassificationTestFixture() {};
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassificationTestFixture, CreateAndInsert_InsertsAndSetsTableSubmodelSuccessfully)
    {
    Dgn::DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    ClassificationTablePtr table = ClassificationSystemsTestsBase::CreateAndInsertTable(db);
    ASSERT_FALSE(table.IsNull()) << "Failed to create table";

    Utf8CP expectedName = "TestClassification";
    Utf8CP expectedCode = "TestClassificationCode";
    Utf8CP expectedDescription = "TestClassificationDescription";

    ClassificationPtr classification = Classification::CreateAndInsert(*table, expectedName, expectedCode, expectedDescription, nullptr, nullptr);
    ASSERT_TRUE(classification.IsValid()) << "Failed to create classification";

    Dgn::DgnCode code = classification->GetCode();
    ASSERT_TRUE(code.GetValue().Equals(expectedCode)) << "Code Value was set wrong";

    Utf8CP name = classification->GetName();
    ASSERT_TRUE(0 == strcmp(name, expectedName)) << "Code Name was set wrong";

    Utf8CP description = classification->GetDescription();
    ASSERT_TRUE(0 == strcmp(description, expectedDescription)) << "Code Name was set wrong";

    Dgn::DgnModelId submodelId = table->GetSubModelId();

    ASSERT_TRUE(submodelId.IsValid()) << "No model set as a submodel to the Classification Table";
    
    ASSERT_EQ(classification->GetModelId().GetValue(), submodelId.GetValue()) << "Classification was not added as a submodel to Table";

    bvector<Dgn::DgnElementId> classifications = table->MakeClassificationIterator().BuildIdList<Dgn::DgnElementId>();
    ASSERT_EQ(1, classifications.size()) << "Classification not found in inserted Classification Table";
    db.SaveChanges();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassificationTestFixture, CreateAndInsert_InsertsAndSetsGroupAndSpecializationSuccessfully)
{
    Dgn::DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    ClassificationTablePtr table = ClassificationSystemsTestsBase::CreateAndInsertTable(db);
    ASSERT_FALSE(table.IsNull()) << "Failed to create table";

    ClassificationGroupPtr group = ClassificationGroup::Create(*table, "TestGroup");
    ASSERT_TRUE(group.IsValid()) << "Failed to create group";

    Dgn::DgnDbStatus stat;
    group->Insert(&stat);
    ASSERT_EQ(Dgn::DgnDbStatus::Success, stat) << "Group failed to be inserted to Db";

    // First classification
    ClassificationPtr classification = Classification::CreateAndInsert(*table, "TestClassification", "TestClassificationCode", "", group.get(), nullptr);
    ASSERT_TRUE(classification.IsValid()) << "Failed to create classification";

    ASSERT_TRUE(classification->GetGroupId() == group->GetElementId()) << "Group was not set properly";
    ASSERT_TRUE(classification->GetSpecializationId() == Dgn::DgnElementId()) << "Specialization was not set properly";

    //Second classification
    ClassificationPtr classification2 = Classification::CreateAndInsert(*table, "Classification", "Classification2", "", nullptr, classification.get());
    ASSERT_TRUE(classification2.IsValid()) << "Failed to create second classification";

    ASSERT_TRUE(classification2->GetGroupId() == Dgn::DgnElementId()) << "Group was not set properly";
    ASSERT_TRUE(classification2->GetSpecializationId() == classification->GetElementId()) << "Specialization was not set properly";

    Dgn::DgnElementIdSet classificationChildren = classification->QueryChildren();
    ASSERT_FALSE(classificationChildren.empty()) << "No children were added to the Classification";
    ASSERT_EQ(classification2->GetElementId().GetValue(), classificationChildren.begin()->GetValue()) << "Second Classification was not added as child to Classification";

    db.SaveChanges();
}