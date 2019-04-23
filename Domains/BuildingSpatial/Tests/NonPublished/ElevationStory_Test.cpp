/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/ElevationStory_Test.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <BeSQLite/BeSQLite.h>
#include "BuildingSpatialTestFixtureBase.h"
#include <BuildingSpatial/Elements/ElevationStory.h>

USING_NAMESPACE_BUILDINGSPATIAL
USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
// Sets up environment for BuildingSpatial testing.
// @bsiclass                                    Elonas.Seviakovas               04/2019
//=======================================================================================
struct ElevationStoryTestFixture : public BuildingSpatialTestFixtureBase
    {
    public:
        ElevationStoryTestFixture() {};
        ~ElevationStoryTestFixture() {};
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ElevationStoryTestFixture, ElevationStoryIsInserted)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();
    ElevationStoryPtr elevationStory = BuildingSpatial::ElevationStory::Create(*m_model);

    ASSERT_TRUE(elevationStory.IsValid()) << "Failed to create ElevationStory";

    ASSERT_TRUE(elevationStory->Insert().IsValid()) << "Failed to insert ElevationStory";
    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ElevationStoryTestFixture, SetFootprintShape)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    ElevationStoryPtr elevationStory = BuildingSpatial::ElevationStory::Create(*m_model);
    ASSERT_TRUE(elevationStory->Insert().IsValid());

    double area = elevationStory->GetFootprintArea();
    ASSERT_EQ(0, area);

    CurveVectorPtr rectCurve = CurveVector::CreateRectangle(0, 0, 10, 10, 0);

    DPoint3d curveCentroid;
    double curveArea;
    rectCurve->CentroidAreaXY(curveCentroid, curveArea);

    elevationStory->SetFootprintShape(rectCurve);

    area = elevationStory->GetFootprintArea();

    ASSERT_EQ(curveArea, area);

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }
