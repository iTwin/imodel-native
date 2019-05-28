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
#include <QuantityTakeoffsAspects/Elements/SlabAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for SlabAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct SlabAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        SlabAspectTestFixture() {};
        ~SlabAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SlabAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    auto slabDirection = SlabDirectionType::OneWay;

    SlabAspectPtr aspect = SlabAspect::Create(slabDirection);
    ASSERT_TRUE(aspect.IsValid());

    SlabAspect::SetAspect(*object, *aspect);

    object->Update();

    SlabAspectCPtr aspect2 = SlabAspect::GetCP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(slabDirection, aspect2->GetSlabDirection());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE