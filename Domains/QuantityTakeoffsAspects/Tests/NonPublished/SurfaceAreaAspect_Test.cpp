/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "QuantityTakeoffsAspectsTestFixtureBase.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/GenericDomain.h>
#include <BeSQLite/BeSQLite.h>
#include <QuantityTakeoffsAspects/Elements/SurfaceAreaAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for SurfaceAreaAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct SurfaceAreaAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        SurfaceAreaAspectTestFixture() {};
        ~SurfaceAreaAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SurfaceAreaAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    double grossA1 = 15.0f;
    double netA1 = 5.0f;

    SurfaceAreaAspectPtr aspect = SurfaceAreaAspect::Create(grossA1, netA1);
    ASSERT_TRUE(aspect.IsValid());

    SurfaceAreaAspect::SetAspect(*object, *aspect);

    object->Update();

    SurfaceAreaAspectPtr aspect2 = SurfaceAreaAspect::GetP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(grossA1, aspect2->GetGrossSurfaceArea());
    ASSERT_EQ(netA1, aspect2->GetNetSurfaceArea());

    double grossA2 = 15.0f;
    double netA2 = 5.0f;

    aspect2->SetGrossSurfaceArea(grossA2);
    aspect2->SetNetSurfaceArea(netA2);

    object->Update();
    SurfaceAreaAspectCPtr aspect3 = SurfaceAreaAspect::GetCP(*object);
    ASSERT_TRUE(aspect3.IsValid());

    ASSERT_EQ(grossA2, aspect3->GetGrossSurfaceArea());
    ASSERT_EQ(netA2, aspect3->GetNetSurfaceArea());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE