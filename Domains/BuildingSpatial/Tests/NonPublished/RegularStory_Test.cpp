/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RegularStory_Test.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <BeSQLite/BeSQLite.h>
#include "BuildingSpatialTestFixtureBase.h"
#include <BuildingSpatial/Elements/RegularStory.h>
#include <BuildingShared/BuildingSharedApi.h>

USING_NAMESPACE_BUILDINGSPATIAL
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING_SHARED

//=======================================================================================
// Sets up environment for BuildingSpatial testing.
// @bsiclass                                    Elonas.Seviakovas               04/2019
//=======================================================================================
struct RegularStoryTestFixture : public BuildingSpatialTestFixtureBase
    {
    public:
        RegularStoryTestFixture() {};
        ~RegularStoryTestFixture() {};
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(RegularStoryTestFixture, RegularStoryIsInserted)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();
    RegularStoryPtr regularStory = BuildingSpatial::RegularStory::Create(*m_model);

    ASSERT_TRUE(regularStory.IsValid()) << "Failed to create RegularStory";

    ASSERT_TRUE(regularStory->Insert().IsValid()) << "Failed to insert RegularStory";
    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(RegularStoryTestFixture, SetFootprintShape)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    RegularStoryPtr regularStory = BuildingSpatial::RegularStory::Create(*m_model);
    ASSERT_TRUE(regularStory->Insert().IsValid());

    ASSERT_EQ(0, regularStory->GetFootprintArea());

    CurveVectorPtr rectCurve = CurveVector::CreateRectangle(0, 0, 10, 10, 0);

    regularStory->SetFootprintShape(rectCurve);

    CurveVectorPtr regularStoryShape = DgnGeometryUtils::GetBaseShape(*regularStory);

    ASSERT_TRUE(regularStoryShape.IsValid());
    ASSERT_EQ(GeometryUtils::GetCurveArea(*rectCurve), GeometryUtils::GetCurveArea(*regularStoryShape));

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }
