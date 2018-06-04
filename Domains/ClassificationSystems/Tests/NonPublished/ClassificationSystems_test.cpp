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

        BuildingLocks_LockElementForOperation(*system.get(), BeSQLite::DbOpcode::Insert, "Insert for ClassificationSystems_test");
        system->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat) << "System failed to be inserted to Db";

        ClassificationSystemCPtr getSystem = ClassificationSystem::TryGet(db, expectedSystemName);
        ASSERT_TRUE(getSystem.IsValid()) << "System was not found in Db";

        ClassificationGroupPtr group = ClassificationGroup::Create(*system, expectedGroupName);
        ASSERT_TRUE(group.IsValid()) << "Failed to create group";

        ASSERT_TRUE(0 == strcmp(group->GetUserLabel(), expectedGroupName)) << "Group label was set wrong";

        group->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat) << "Group failed to be inserted to Db";
        
        ClassificationPtr classification = Classification::Create(*system, expectedClassificationName, expectedClassificationCode, "", group.get(), nullptr);
        ASSERT_TRUE(classification.IsValid()) << "Failed to create classification";
        
        code = classification->GetCode();
        ASSERT_TRUE(code.GetValue().Equals(expectedClassificationCode)) << "Code Value was set wrong";

        BuildingLocks_LockElementForOperation(*classification.get(), BeSQLite::DbOpcode::Insert, "Insert for ClassificationSystems_test");
        classification->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat) << "Group failed to be inserted to Db";
        

        db.SaveChanges();
    }