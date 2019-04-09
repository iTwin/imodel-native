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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassificationTestFixture, GetOrCreateBySystemTableNames_NoElementExists_CreatesWholeHierarchy)
    {
    Dgn::DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    const Utf8String classificationSystemName = "Test System";
    ClassificationSystemCPtr system = ClassificationSystem::TryGet(db, classificationSystemName.c_str());
    ASSERT_FALSE(system.IsValid()) << "System already exists";

    const Utf8String classificationTableName = "Test Table";
    const Utf8String classificationName = "Test Classification";

    ClassificationPtr classification = Classification::GetOrCreateBySystemTableNames(
        db, classificationName.c_str(), "01-00-00", "", classificationSystemName, classificationTableName);
    ASSERT_TRUE(classification.IsValid()) << "Classification did not get created";

    system = ClassificationSystem::TryGet(db, classificationSystemName.c_str());
    ASSERT_TRUE(system.IsValid()) << "System did not get created";

    ClassificationTableCPtr table = ClassificationTable::TryGet(*system, classificationTableName.c_str());
    ASSERT_TRUE(table.IsValid()) << "Table did not get created";

    ASSERT_EQ(table->GetElementId(), classification->GetClassificationTableId());

    ASSERT_EQ(system->GetElementId(), table->GetClassificationSystemId());

    db.SaveChanges();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassificationTestFixture, GetOrCreateBySystemTableNames_HierarchyAlreadyExists_ReturnsCorrectClassification)
{
    Dgn::DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    const Utf8String classificationSystemName = "Test System";
    ClassificationSystemPtr system = ClassificationSystem::Create(db, classificationSystemName.c_str());
    ASSERT_TRUE(system.IsValid()) << "Could not create System";
    system->Insert();

    const Utf8String classificationTableName = "Test Table";
    ClassificationTablePtr table = ClassificationTable::Create(*system, classificationTableName.c_str());
    ASSERT_TRUE(table.IsValid()) << "Couldnot create Table";
    table->Insert();

    const Utf8String classificationName = "Test Classification";
    ClassificationPtr classification = Classification::CreateAndInsert(*table, classificationName.c_str(), "01-01-00", "", nullptr, nullptr);
    ASSERT_TRUE(classification.IsValid()) << "Could not create Classification";

    ClassificationPtr queriedClassification = Classification::GetOrCreateBySystemTableNames(
        db, classificationName.c_str(), "01-01-00", "", classificationSystemName.c_str(), classificationTableName.c_str());

    ASSERT_EQ(classification->GetElementId(), queriedClassification->GetElementId());

    ASSERT_EQ(table->GetElementId(), queriedClassification->GetClassificationTableId());

    db.SaveChanges();
}