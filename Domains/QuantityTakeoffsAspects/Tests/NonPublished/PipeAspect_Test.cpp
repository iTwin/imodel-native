/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "QuantityTakeoffsAspectsTestFixtureBase.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/GenericDomain.h>
#include <BeSQLite/BeSQLite.h>
#include <QuantityTakeoffsAspects/Elements/PipeAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for PipeAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct PipeAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        PipeAspectTestFixture() {};
        ~PipeAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PipeAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    Utf8String schedule1 = "#01;#02";
    Point3d dimensions1{1, 2, 3};

    PipeAspectPtr aspect = PipeAspect::Create(schedule1, dimensions1.x, dimensions1.y, dimensions1.z);
    ASSERT_TRUE(aspect.IsValid());

    PipeAspect::SetAspect(*object, *aspect);

    object->Update();

    PipeAspectPtr aspect2 = PipeAspect::GetP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(schedule1, *aspect2->GetSchedule());
    ASSERT_EQ(dimensions1.x, aspect2->GetThickness());
    ASSERT_EQ(dimensions1.y, aspect2->GetLength());
    ASSERT_EQ(dimensions1.z, aspect2->GetDiameter());

    Utf8String schedule2 = "#03;#04";
    Point3d dimensions2{ 7, 8, 9 };

    aspect2->SetSchedule(schedule2);
    aspect2->SetThickness(dimensions2.x);
    aspect2->SetLength(dimensions2.y);
    aspect2->SetDiameter(dimensions2.z);

    object->Update();
    PipeAspectCPtr aspect3 = PipeAspect::GetCP(*object);
    ASSERT_TRUE(aspect3.IsValid());

    ASSERT_EQ(schedule2, *aspect3->GetSchedule());
    ASSERT_EQ(dimensions2.x, aspect3->GetThickness());
    ASSERT_EQ(dimensions2.y, aspect3->GetLength());
    ASSERT_EQ(dimensions2.z, aspect3->GetDiameter());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE