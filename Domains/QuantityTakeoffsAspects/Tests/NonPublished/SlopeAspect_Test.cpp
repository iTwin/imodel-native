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
#include <QuantityTakeoffsAspects/Elements/SlopeAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for SlopeAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct SlopeAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        SlopeAspectTestFixture() {};
        ~SlopeAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SlopeAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    double slope = 0.5f;

    SlopeAspectPtr aspect = SlopeAspect::Create(slope);
    ASSERT_TRUE(aspect.IsValid());

    SlopeAspect::SetAspect(*object, *aspect);

    object->Update();

    SlopeAspectCPtr aspect2 = SlopeAspect::GetCP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(slope, aspect2->GetSlope());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE