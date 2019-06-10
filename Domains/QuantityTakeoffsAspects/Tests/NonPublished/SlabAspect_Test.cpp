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

    auto slabDirection1 = SlabDirectionType::OneWay;
    auto type1 = SlabType::Roof;

    SlabAspectPtr aspect = SlabAspect::Create(slabDirection1, type1);
    ASSERT_TRUE(aspect.IsValid());

    SlabAspect::SetAspect(*object, *aspect);

    object->Update();

    SlabAspectPtr aspect2 = SlabAspect::GetP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(slabDirection1, aspect2->GetSlabDirection());
    ASSERT_EQ(type1, aspect2->GetType());

    auto slabDirection2 = SlabDirectionType::TwoWay;
    auto type2 = SlabType::Floor;

    aspect2->SetSlabDirection(slabDirection2);
    aspect2->SetType(type2);

    object->Update();
    SlabAspectCPtr aspect3 = SlabAspect::GetCP(*object);
    ASSERT_TRUE(aspect3.IsValid());

    ASSERT_EQ(slabDirection2, aspect3->GetSlabDirection());
    ASSERT_EQ(type2, aspect3->GetType());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE