/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <Grids/GridsApi.h>
#include "GridsTestFixtureBase.h"
#include <DgnClientFx/DgnClientApp.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGNCLIENTFX

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               07/2018
//=======================================================================================
struct ElevationGridTestFixture : public GridsTestFixtureBase
    {
    ElevationGridTestFixture() {};
    ~ElevationGridTestFixture() {};

    void SetUp() override;
    void TearDown() override;

    SpatialLocationModelPtr m_model;

    static DgnDbR GetDgnDb() { return *DgnClientApp::App().Project(); }
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElevationGridTestFixture::SetUp()
    {
    GridsTestFixtureBase::SetUp();
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    SpatialLocationPartitionPtr partition = SpatialLocationPartition::Create(*rootSubject, "GridSpatialPartition");
    db.Elements().Insert<SpatialLocationPartition>(*partition);
    m_model = SpatialLocationModel::CreateAndInsert(*partition);
    db.SaveChanges();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void ElevationGridTestFixture::TearDown()
    {
    m_model = nullptr;
    GridsTestFixtureBase::TearDown();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                  07/2018
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(ElevationGridTestFixture, GetFirstAboveAndBelow)
    {
    Dgn::DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();
    ElevationGridCPtr floorGrid = ElevationGrid::CreateAndInsert(ElevationGrid::CreateParams(*m_model,
                                                                                             db.Elements().GetRootSubject()->GetElementId(),
                                                                                             "Floor-Grid"));

    ASSERT_TRUE(floorGrid.IsValid());
    SpatialLocationModelPtr gridModel = floorGrid->GetSurfacesModel();
    GridAxisCPtr axis = floorGrid->GetAxis();
    ASSERT_TRUE(gridModel.IsValid());
    ASSERT_TRUE(axis.IsValid());

    ElevationGridSurface::CreateParams params1(*gridModel, *axis, nullptr, 0);
    ElevationGridSurfacePtr surface1 = ElevationGridSurface::Create(params1);
    ASSERT_TRUE(surface1->Insert().IsValid());

    ElevationGridSurface::CreateParams params2(*gridModel, *axis, nullptr, 10);
    ElevationGridSurfacePtr surface2 = ElevationGridSurface::Create(params2);
    ASSERT_TRUE(surface2->Insert().IsValid());

    ElevationGridCPtr otherFloorGrid = ElevationGrid::CreateAndInsert(ElevationGrid::CreateParams(*m_model,
                                                                                                  db.Elements().GetRootSubject()->GetElementId(),
                                                                                                  "OtherFloor-Grid"));

    ASSERT_TRUE(otherFloorGrid.IsValid());
    SpatialLocationModelPtr otherGridModel = otherFloorGrid->GetSurfacesModel();
    GridAxisCPtr otherAxis = otherFloorGrid->GetAxis();
    ASSERT_TRUE(otherGridModel.IsValid());
    ASSERT_TRUE(otherAxis.IsValid());

    ElevationGridSurface::CreateParams otherParams1(*otherGridModel, *otherAxis, nullptr, -0.5);
    ElevationGridSurfacePtr otherSurface1 = ElevationGridSurface::Create(otherParams1);
    ASSERT_TRUE(otherSurface1->Insert().IsValid());

    ElevationGridSurface::CreateParams otherParams2(*otherGridModel, *otherAxis, nullptr, 1);
    ElevationGridSurfacePtr otherSurface2 = ElevationGridSurface::Create(otherParams2);
    ASSERT_TRUE(otherSurface2->Insert().IsValid());

    ElevationGridSurface::CreateParams otherParams3(*otherGridModel, *otherAxis, nullptr, 10.5);
    ElevationGridSurfacePtr otherSurface3 = ElevationGridSurface::Create(otherParams3);
    ASSERT_TRUE(otherSurface3->Insert().IsValid());

    ElevationGridSurfaceCPtr aboveMinus1 = floorGrid->GetFirstAbove(-1);
    ASSERT_TRUE(aboveMinus1.IsValid());
    ASSERT_EQ(aboveMinus1->GetElementId(), surface1->GetElementId());

    ElevationGridSurfaceCPtr above0 = floorGrid->GetFirstAbove(0);
    ASSERT_TRUE(above0.IsValid());
    ASSERT_EQ(above0->GetElementId(), surface2->GetElementId());

    ElevationGridSurfaceCPtr above10 = floorGrid->GetFirstAbove(10);
    ASSERT_TRUE(above10.IsNull());

    ElevationGridSurfaceCPtr below11 = floorGrid->GetFirstBelow(11);
    ASSERT_TRUE(below11.IsValid());
    ASSERT_EQ(below11->GetElementId(), surface2->GetElementId());
    
    ElevationGridSurfaceCPtr below10 = floorGrid->GetFirstBelow(10);
    ASSERT_TRUE(below10.IsValid());
    ASSERT_EQ(below10->GetElementId(), surface1->GetElementId());

    ElevationGridSurfaceCPtr below0 = floorGrid->GetFirstBelow(0);
    ASSERT_TRUE(below0.IsNull());

    db.SaveChanges();
    }