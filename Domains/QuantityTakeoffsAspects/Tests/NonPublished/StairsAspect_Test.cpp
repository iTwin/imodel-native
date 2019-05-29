/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "QuantityTakeoffsAspectsTestFixtureBase.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/GenericDomain.h>
#include <BeSQLite/BeSQLite.h>
#include <QuantityTakeoffsAspects/Elements/StairsAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for StairsAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct StairsAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        StairsAspectTestFixture() {};
        ~StairsAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StairsAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    uint32_t numberOfRisers1 = 5;
    double riserHeight1 = 10.0f;

    StairsAspectPtr aspect = StairsAspect::Create(numberOfRisers1, riserHeight1);
    ASSERT_TRUE(aspect.IsValid());

    StairsAspect::SetAspect(*object, *aspect);

    object->Update();

    StairsAspectPtr aspect2 = StairsAspect::GetP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(numberOfRisers1, aspect2->GetNumberOfRisers());
    ASSERT_EQ(riserHeight1, aspect2->GetRiserHeight());

    uint32_t numberOfRisers2 = 6;
    double riserHeight2 = 9.0f;

    aspect2->SetNumberOfRisers(numberOfRisers2);
    aspect2->SetRiserHeight(riserHeight2);

    object->Update();
    StairsAspectCPtr aspect3 = StairsAspect::GetCP(*object);
    ASSERT_TRUE(aspect3.IsValid());

    ASSERT_EQ(numberOfRisers2, aspect3->GetNumberOfRisers());
    ASSERT_EQ(riserHeight2, aspect3->GetRiserHeight());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE