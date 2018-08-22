#include <Bentley/BeTest.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <ClassificationSystems/ClassificationSystemsApi.h>
#include <DgnPlatform/DgnPlatformAPI.h>
#include "ClassificationSystemsTestsBase.h"
#include <DgnClientFx/DgnClientApp.h>
#include <BuildingShared/BuildingSharedApi.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_CLASSIFICATIONSYSTEMS
USING_NAMESPACE_BUILDING_SHARED


//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                05/2018
//---------------+---------------+---------------+---------------+---------------+------
struct ClassificationSystemsTestFixture : public ClassificationSystemsTestsBase {
        ClassificationSystemsTestFixture() {};
        ~ClassificationSystemsTestFixture() {};

        
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                05/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassificationSystemsTestFixture, StandardInsertion)
    {
        DgnDbR db = *DgnClientApp::App().Project();
        db.BriefcaseManager().StartBulkOperation();
        Utf8CP expectedSystemName = "TestSys";
        Utf8CP expectedGroupName = "TestGroup";
        Utf8CP expectedClassificationName = "TestClassification";
        Utf8CP expectedClassificationCode = "TestClassificationCode";
        DgnDbStatus stat;
        DgnCode code;

        ClassificationSystemPtr system = ClassificationSystem::Create(db, expectedSystemName);
        ASSERT_TRUE(system.IsValid()) << "Failed to create system";

        
        code = system->GetCode();
        ASSERT_TRUE(code.GetValue().Equals(expectedSystemName)) << "Code Value was set wrong";

        Utf8CP name = system->GetName();
        ASSERT_TRUE(0 == strcmp(name, expectedSystemName)) << "Failed to get name from Classification System";

        system->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat) << "System failed to be inserted to Db";

        ClassificationSystemCPtr getSystem = ClassificationSystem::TryGet(db, expectedSystemName);
        ASSERT_TRUE(getSystem.IsValid()) << "System was not found in Db";

        ClassificationGroupPtr group = ClassificationGroup::Create(*system, expectedGroupName);
        ASSERT_TRUE(group.IsValid()) << "Failed to create group";

        ASSERT_TRUE(0 == strcmp(group->GetUserLabel(), expectedGroupName)) << "Group label was set wrong";

        group->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat) << "Group failed to be inserted to Db";
        
        ClassificationPtr classification = Classification::CreateAndInsert(*system, expectedClassificationName, expectedClassificationCode, "", group.get(), nullptr);
        ASSERT_TRUE(classification.IsValid()) << "Failed to create classification";
        
        code = classification->GetCode();
        ASSERT_TRUE(code.GetValue().Equals(expectedClassificationCode)) << "Code Value was set wrong";
        
        db.SaveChanges();
    }

    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassificationSystemsTestFixture, PropertiesAreSetProperly)
    {
        DgnDbR db = *DgnClientApp::App().Project();
        db.BriefcaseManager().StartBulkOperation();
        Utf8CP expectedSystemName = "ClassificationSystem1";
        Utf8CP expectedGroupName = "ClassificationGroup1";
        Utf8CP expectedClassification1Name = "Classification1";
        Utf8CP expectedClassification1Id = "01-00-00";
        Utf8CP expectedClassification1Description = "Classification1 description";
        Utf8CP expectedClassification2Name = "Classification2";
        Utf8CP expectedClassification2Id = "01-01-00";
        Utf8CP expectedClassification2Description = "a specialized classification of Classification1";
        Utf8CP expectedClassification3Name = "Classification3";
        Utf8CP expectedClassification3Id = "01-02-00";
        Utf8CP expectedClassification3Description = "another specialized classification of Classification1";
        DgnDbStatus stat;
        DgnCode code;

        ClassificationSystemPtr system = ClassificationSystem::Create(db, expectedSystemName);
        ASSERT_TRUE(system.IsValid()) << "Failed to create system";

        
        code = system->GetCode();
        ASSERT_TRUE(code.GetValue().Equals(expectedSystemName)) << "Code Value was set wrong";

        Utf8CP name = system->GetName();
        ASSERT_TRUE(0 == strcmp(name, expectedSystemName)) << "Failed to get name from Classification System";

        system->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat) << "System failed to be inserted to Db";

        ClassificationSystemCPtr getSystem = ClassificationSystem::TryGet(db, expectedSystemName);
        ASSERT_TRUE(getSystem.IsValid()) << "System was not found in Db";

        ClassificationGroupPtr group = ClassificationGroup::Create(*system, expectedGroupName);
        ASSERT_TRUE(group.IsValid()) << "Failed to create group";

        ASSERT_TRUE(0 == strcmp(group->GetUserLabel(), expectedGroupName)) << "Group label was set wrong";
        ASSERT_TRUE(group->GetClassificationSystemId() == system->GetElementId()) << "Group label was set wrong";

        group->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat) << "Group failed to be inserted to Db";
        bvector<Dgn::DgnElementId> classificationgroups = system->MakeClassificationGroupIterator().BuildIdList<Dgn::DgnElementId>();
        ASSERT_EQ(1, classificationgroups.size()) << "Group not found in inserted classificationsystem";
        
        ClassificationPtr classification1 = Classification::CreateAndInsert(*system, expectedClassification1Name, expectedClassification1Id, expectedClassification1Description, group.get(), nullptr);
        ASSERT_TRUE(classification1.IsValid()) << "Failed to create classification";
        
        code = classification1->GetCode();
        ASSERT_TRUE(code.GetValue().Equals(expectedClassification1Id)) << "Code Value was set wrong";
        ASSERT_TRUE(strcmp(classification1->GetName(), expectedClassification1Name) == 0) << "incorrect name";
        ASSERT_TRUE(strcmp(classification1->GetUserLabel(), expectedClassification1Name) == 0) << "userlabel not used for name property";
        ASSERT_TRUE(strcmp(classification1->GetDescription(), expectedClassification1Description) == 0) << "description property is not correct";

        
        ASSERT_TRUE(classification1->GetGroupId() == group->GetElementId()) << "group was not set properly";
        ASSERT_TRUE(classification1->GetSpecializationId() == Dgn::DgnElementId()) << "specialization was not set properly";
        
        ClassificationPtr classification2 = Classification::CreateAndInsert(*system, expectedClassification2Name, expectedClassification2Id, expectedClassification2Description, nullptr, classification1.get());
        ASSERT_TRUE(classification2.IsValid()) << "Failed to create classification";
        
        code = classification2->GetCode();
        ASSERT_TRUE(code.GetValue().Equals(expectedClassification2Id)) << "Code Value was set wrong";
        ASSERT_TRUE(strcmp(classification2->GetName(), expectedClassification2Name) == 0) << "incorrect name";
        ASSERT_TRUE(strcmp(classification2->GetUserLabel(), expectedClassification2Name) == 0) << "userlabel not used for name property";
        ASSERT_TRUE(strcmp(classification2->GetDescription(), expectedClassification2Description) == 0) << "description property is not correct";

        
        ASSERT_TRUE(classification2->GetGroupId() == Dgn::DgnElementId()) << "group was not set properly";
        ASSERT_TRUE(classification2->GetSpecializationId() == classification1->GetElementId()) << "specialization was not set properly";

        ClassificationPtr classification3 = Classification::CreateAndInsert(*system, expectedClassification3Name, expectedClassification3Id, expectedClassification3Description, group.get(), classification1.get());
        ASSERT_TRUE(classification3.IsValid()) << "Failed to create classification";

        
        ASSERT_TRUE(classification3->GetGroupId() == group->GetElementId()) << "group was not set properly";
        ASSERT_TRUE(classification3->GetSpecializationId() == classification1->GetElementId()) << "specialization was not set properly";
        db.SaveChanges();
    }