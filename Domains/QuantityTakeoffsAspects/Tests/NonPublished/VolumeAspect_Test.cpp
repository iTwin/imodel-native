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
#include <QuantityTakeoffsAspects/Elements/VolumeAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for VolumeAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct VolumeAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        VolumeAspectTestFixture() {};
        ~VolumeAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VolumeAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    double grossV1 = 15.0f;
    double netV1 = 5.0f;

    VolumeAspectPtr aspect = VolumeAspect::Create(grossV1, netV1);
    ASSERT_TRUE(aspect.IsValid());

    VolumeAspect::SetAspect(*object, *aspect);

    object->Update();

    VolumeAspectPtr aspect2 = VolumeAspect::GetP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(grossV1, aspect2->GetGrossVolume());
    ASSERT_EQ(netV1, aspect2->GetNetVolume());

    double grossV2 = 15.0f;
    double netV2 = 5.0f;

    aspect2->SetGrossVolume(grossV2);
    aspect2->SetNetVolume(netV2);

    object->Update();
    VolumeAspectCPtr aspect3 = VolumeAspect::GetCP(*object);
    ASSERT_TRUE(aspect3.IsValid());

    ASSERT_EQ(grossV2, aspect3->GetGrossVolume());
    ASSERT_EQ(netV2, aspect3->GetNetVolume());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE