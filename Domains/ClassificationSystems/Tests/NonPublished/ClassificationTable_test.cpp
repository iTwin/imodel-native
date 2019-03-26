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
TEST_F(ClassificationTableTestFixture, CreateAndInsert_InsertsSuccessfully)
    {
    Dgn::DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    ClassificationTablePtr table = ClassificationTable::Create(db);
    ASSERT_TRUE(table.IsValid()) << "Failed to create table";


    Dgn::DgnDbStatus status;
    table->Insert(&status);
    ASSERT_EQ(Dgn::DgnDbStatus::Success, status) << "Table failed to be inserted to Db";

    db.SaveChanges();
    }