#include <Bentley/BeTest.h>
#include <ClassificationSystems/ClassificationSystemsApi.h>
#include "ClassificationSystemsTestsBase.h"

USING_NAMESPACE_CLASSIFICATIONSYSTEMS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               03/2019
//---------------+---------------+---------------+---------------+---------------+------
struct ClassificationTableTestFixture : public ClassificationSystemsTestsBase 
    {
    ClassificationTableTestFixture() {};
    ~ClassificationTableTestFixture() {};
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               03/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassificationTableTestFixture, Create_InsertsAndSetsSubmodelSuccessfully)
    {
    Dgn::DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    Utf8CP systemName = "TestSystem";
    ClassificationSystemPtr system = ClassificationSystem::Create(db, systemName);

    Dgn::DgnDbStatus status;
    system->Insert(&status);
    ASSERT_EQ(Dgn::DgnDbStatus::Success, status) << "System failed to be inserted to Db";

    Utf8CP tableName = "TestTable";
    ClassificationTablePtr table = ClassificationTable::Create(*system, tableName);
    ASSERT_TRUE(table.IsValid()) << "Failed to create table";

    table->Insert(&status);
    ASSERT_EQ(Dgn::DgnDbStatus::Success, status) << "Table failed to be inserted to Db";

    Dgn::DgnElementIdSet children = system->QueryChildren();

    ASSERT_FALSE(children.empty()) << "No children were added to the Classification System";

    ASSERT_EQ(table->GetElementId().GetValue(), children.begin()->GetValue()) << "Table was not added as child to System";
    db.SaveChanges();
    }