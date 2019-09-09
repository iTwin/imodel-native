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
#include <QuantityTakeoffsAspects/Elements/EnergyPerformanceAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for EnergyPerformanceAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct EnergyPerformanceAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        EnergyPerformanceAspectTestFixture() {};
        ~EnergyPerformanceAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(EnergyPerformanceAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    double rating1 = 99.99f;

    EnergyPerformanceAspectPtr aspect = EnergyPerformanceAspect::Create(rating1);
    ASSERT_TRUE(aspect.IsValid());

    EnergyPerformanceAspect::SetAspect(*object, *aspect);

    object->Update();

    EnergyPerformanceAspectPtr aspect2 = EnergyPerformanceAspect::GetP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(rating1, aspect2->GetRating());

    double rating2 = 98.99f;
    aspect2->SetRating(rating2);

    object->Update();
    EnergyPerformanceAspectCPtr aspect3 = EnergyPerformanceAspect::GetCP(*object);
    ASSERT_TRUE(aspect3.IsValid());

    ASSERT_EQ(rating2, aspect3->GetRating());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE