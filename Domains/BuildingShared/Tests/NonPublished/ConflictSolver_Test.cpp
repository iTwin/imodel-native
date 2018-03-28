/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/NonPublished/ConflictSolver_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING_SHARED

#define AREA_TOLERANCE 0.004645 //0.05ft.
#define DISTANCE_TOLERANCE 0.1 // 10cm

struct ConflictSolverTestFixture : public ::testing::Test
    {
    };

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, NoConflicts)
    {
    ConflictSolver sut;

    sut.AddContainerBoundary(*CurveVector::CreateRectangle(0, 0, 10, 10, 0), BeInt64Id(1));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(1, 1, 9, 9, 0), BeInt64Id(2));

    ASSERT_TRUE(sut.Solve().empty());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, NoBoundaries)
    {
    ConflictSolver sut;

    ASSERT_TRUE(sut.Solve().empty());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, HasContainerBoundaryButNoInnerBoundaries)
    {
    ConflictSolver sut;
    
    sut.AddContainerBoundary(*CurveVector::CreateRectangle(0, 0, 10, 10, 0), BeInt64Id(1));

    ASSERT_TRUE(sut.Solve().empty());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, HasContainerBoundaryWithAHoleButNoInnerBoundaries)
    {
    ConflictSolver sut;

    CurveVectorPtr containerBoundary = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
    containerBoundary->Add(CurveVector::CreateRectangle(0, 0, 10, 10, 0, CurveVector::BOUNDARY_TYPE_Outer));
    containerBoundary->Add(CurveVector::CreateLinear({{4,1,0},{4,9,0},{6,9,0},{6,1,0}}, CurveVector::BOUNDARY_TYPE_Inner));
    sut.AddContainerBoundary(*containerBoundary, BeInt64Id(1));

    ASSERT_TRUE(sut.Solve().empty());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, InnerBoundaryWithoutContainerBoundary)
    {
    ConflictSolver sut;

    sut.AddInnerBoundary(*CurveVector::CreateRectangle(0, 0, 10, 10, 0), BeInt64Id(1));

    ASSERT_TRUE(sut.Solve().empty());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, CrossingInnerBoundaries)
    {
    ConflictSolver sut;

    CurveVectorPtr expectedConflict = CurveVector::CreateRectangle(-1, -1, 1, 1, 0);
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(-1, -5, 1, 5, 0), BeInt64Id(1));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(-5, -1, 5, 1, 0), BeInt64Id(2));

    bvector<Conflict> conflicts = sut.Solve();
    ASSERT_EQ(1, conflicts.size());
    bset<BeInt64Id> conflictingBoundaries = conflicts.front().m_conflictingBoundaries;
    ASSERT_EQ(2, conflictingBoundaries.size());
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(1)));
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(2)));
    CurveVectorPtr actualConflict = conflicts.front().m_geometry;
    ASSERT_TRUE(actualConflict.IsValid());
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*expectedConflict, *actualConflict));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, InnerBoundaryOutsideOfOuterBoundary)
    {
    ConflictSolver sut;

    CurveVectorPtr conflictGeometry = CurveVector::CreateRectangle(20, 0, 30, 10, 0);
    sut.AddContainerBoundary(*CurveVector::CreateRectangle(0, 0, 10, 10, 0), BeInt64Id(1));
    sut.AddInnerBoundary(*conflictGeometry, BeInt64Id(2));

    bvector<Conflict> conflicts = sut.Solve();
    ASSERT_EQ(1, conflicts.size());
    bset<BeInt64Id> conflictingBoundaries = conflicts.front().m_conflictingBoundaries;
    ASSERT_EQ(1, conflictingBoundaries.size());
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(2)));
    ASSERT_TRUE(conflicts.front().m_geometry.IsValid());
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*conflictGeometry, *conflicts.front().m_geometry));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, PartOfInnerBoundaryOutsideOfOuterBoundary)
    {
    ConflictSolver sut;

    CurveVectorPtr expectedConflict = CurveVector::CreateRectangle(10, 0, 15, 10, 0);
    sut.AddContainerBoundary(*CurveVector::CreateRectangle(0, 0, 10, 10, 0), BeInt64Id(1));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(5, 0, 15, 10, 0), BeInt64Id(2));

    bvector<Conflict> conflicts = sut.Solve();
    ASSERT_EQ(1, conflicts.size());
    bset<BeInt64Id> conflictingBoundaries = conflicts.front().m_conflictingBoundaries;
    ASSERT_EQ(2, conflictingBoundaries.size());
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(1)));
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(2)));
    CurveVectorPtr actualConflict = conflicts.front().m_geometry;
    ASSERT_TRUE(actualConflict.IsValid());
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*expectedConflict, *actualConflict));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, InnerBoundaryOnTopOfAnotherInnerBoundary)
    {
    ConflictSolver sut;

    CurveVectorPtr expectedConflict = CurveVector::CreateRectangle(2, 2, 8, 8, 0);
    sut.AddContainerBoundary(*CurveVector::CreateRectangle(0, 0, 10, 10, 0), BeInt64Id(1));
    sut.AddInnerBoundary(*expectedConflict, BeInt64Id(2));
    sut.AddInnerBoundary(*expectedConflict, BeInt64Id(3));

    bvector<Conflict> conflicts = sut.Solve();
    ASSERT_EQ(1, conflicts.size());
    bset<BeInt64Id> conflictingBoundaries = conflicts.front().m_conflictingBoundaries;
    ASSERT_EQ(2, conflictingBoundaries.size());
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(2)));
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(3)));
    CurveVectorPtr actualConflict = conflicts.front().m_geometry;
    ASSERT_TRUE(actualConflict.IsValid());
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*expectedConflict, *actualConflict));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, InnerBoundaryPartiallyOverlapsAnotherInnerBoundary)
    {
    ConflictSolver sut;

    CurveVectorPtr expectedConflict = CurveVector::CreateRectangle(4, 4, 6, 6, 0);
    sut.AddContainerBoundary(*CurveVector::CreateRectangle(0, 0, 10, 10, 0), BeInt64Id(1));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(0, 0, 6, 6, 0), BeInt64Id(2));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(4, 4, 10, 10, 0), BeInt64Id(3));

    bvector<Conflict> conflicts = sut.Solve();
    ASSERT_EQ(1, conflicts.size());
    bset<BeInt64Id> conflictingBoundaries = conflicts.front().m_conflictingBoundaries;
    ASSERT_EQ(2, conflictingBoundaries.size());
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(2)));
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(3)));
    CurveVectorPtr actualConflict = conflicts.front().m_geometry;
    ASSERT_TRUE(actualConflict.IsValid());
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*expectedConflict, *actualConflict));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, 3OverlappingInnerBoundaries)
    {
    ConflictSolver sut;

    CurveVectorPtr expectedConflict = CurveVector::CreateRectangle(4, 4, 6, 6, 0);
    sut.AddContainerBoundary(*CurveVector::CreateRectangle(0, 0, 10, 10, 0), BeInt64Id(1));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(0, 4, 6, 6, 0), BeInt64Id(2));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(4, 4, 10, 6, 0), BeInt64Id(3));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(4, 0, 6, 6, 0), BeInt64Id(4));

    bvector<Conflict> conflicts = sut.Solve();
    ASSERT_EQ(1, conflicts.size());
    bset<BeInt64Id> conflictingBoundaries = conflicts.front().m_conflictingBoundaries;
    ASSERT_EQ(3, conflictingBoundaries.size());
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(2)));
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(3)));
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(4)));
    CurveVectorPtr actualConflict = conflicts.front().m_geometry;
    ASSERT_TRUE(actualConflict.IsValid());
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*expectedConflict, *actualConflict));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, ContainerWithAHoleHasInnerBoundaryAcrossThatHole)
    {
    ConflictSolver sut;

    CurveVectorPtr expectedConflict = CurveVector::CreateRectangle(4, 4, 6, 6, 0);
    CurveVectorPtr containerBoundary = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
    containerBoundary->Add(CurveVector::CreateRectangle(0, 0, 10, 10, 0, CurveVector::BOUNDARY_TYPE_Outer));
    containerBoundary->Add(CurveVector::CreateLinear({{4,1,0},{4,9,0},{6,9,0},{6,1,0}}, CurveVector::BOUNDARY_TYPE_Inner));
    sut.AddContainerBoundary(*containerBoundary, BeInt64Id(1));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(0, 4, 10, 6, 0), BeInt64Id(2));

    bvector<Conflict> conflicts = sut.Solve();
    ASSERT_EQ(1, conflicts.size());
    bset<BeInt64Id> conflictingBoundaries = conflicts.front().m_conflictingBoundaries;
    ASSERT_EQ(2, conflictingBoundaries.size());
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(1)));
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(2)));
    CurveVectorPtr actualConflict = conflicts.front().m_geometry;
    ASSERT_TRUE(actualConflict.IsValid());
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*expectedConflict, *actualConflict));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, AddInWorldCoordinates_ReturnedGeometryIsInSameWorldCoordinates)
    {
    ConflictSolver sut(Transform::From(DPoint3d::From(10,0,0)));

    CurveVectorPtr expectedConflict = CurveVector::CreateRectangle(-1, -1, 1, 1, 0);
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(-1, -5, 1, 5, 0), BeInt64Id(1));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(-5, -1, 5, 1, 0), BeInt64Id(2));

    bvector<Conflict> conflicts = sut.Solve();
    ASSERT_EQ(1, conflicts.size());
    bset<BeInt64Id> conflictingBoundaries = conflicts.front().m_conflictingBoundaries;
    ASSERT_EQ(2, conflictingBoundaries.size());
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(1)));
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(2)));
    CurveVectorPtr actualConflict = conflicts.front().m_geometry;
    ASSERT_TRUE(actualConflict.IsValid());
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*expectedConflict, *actualConflict));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, OverlappingInnerBoundariesProtrudeContainerBoundary)
    {
    ConflictSolver sut;

    sut.AddContainerBoundary(*CurveVector::CreateRectangle(0, 0, 10, 10, 0), BeInt64Id(1));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(-1, 0, 1, 6, 0), BeInt64Id(2));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(-1, 4, 1, 10, 0), BeInt64Id(3));

    bvector<Conflict> conflicts = sut.Solve();
    ASSERT_EQ(4, conflicts.size());

    // conflict0 - overlapping inner boundaries outside of container boundary
    auto conflict0 = std::find_if(conflicts.begin(), conflicts.end(), [] (Conflict const& c)
        {
        return c.m_conflictingBoundaries.size() == 3 &&
            c.m_conflictingBoundaries.end() != std::find(c.m_conflictingBoundaries.begin(), c.m_conflictingBoundaries.end(), BeInt64Id(1)) &&
            c.m_conflictingBoundaries.end() != std::find(c.m_conflictingBoundaries.begin(), c.m_conflictingBoundaries.end(), BeInt64Id(2)) &&
            c.m_conflictingBoundaries.end() != std::find(c.m_conflictingBoundaries.begin(), c.m_conflictingBoundaries.end(), BeInt64Id(3));
        });
    CurveVectorPtr expectedConflict0Geometry = CurveVector::CreateRectangle(-1, 4, 0, 6, 0);

    // conflict1 - overlapping inner boundaries inside the container boundary
    auto conflict1 = std::find_if(conflicts.begin(), conflicts.end(), [] (Conflict const& c)
        {
        return c.m_conflictingBoundaries.size() == 2 &&
            c.m_conflictingBoundaries.end() != std::find(c.m_conflictingBoundaries.begin(), c.m_conflictingBoundaries.end(), BeInt64Id(2)) &&
            c.m_conflictingBoundaries.end() != std::find(c.m_conflictingBoundaries.begin(), c.m_conflictingBoundaries.end(), BeInt64Id(3));
        });
    CurveVectorPtr expectedConflict1Geometry = CurveVector::CreateRectangle(0, 4, 1, 6, 0);

    // conflict2 and conflict3 - inner boundaries outisde of container boundary
    auto conflict2 = std::find_if(conflicts.begin(), conflicts.end(), [] (Conflict const& c)
        {
        return c.m_conflictingBoundaries.size() == 2 &&
            c.m_conflictingBoundaries.end() != std::find(c.m_conflictingBoundaries.begin(), c.m_conflictingBoundaries.end(), BeInt64Id(1)) &&
            c.m_conflictingBoundaries.end() != std::find(c.m_conflictingBoundaries.begin(), c.m_conflictingBoundaries.end(), BeInt64Id(2));
        });
    CurveVectorPtr expectedConflict2Geometry = CurveVector::CreateRectangle(-1, 0, 0, 4, 0);
    auto conflict3 = std::find_if(conflicts.begin(), conflicts.end(), [] (Conflict const& c)
        {
        return c.m_conflictingBoundaries.size() == 2 &&
            c.m_conflictingBoundaries.end() != std::find(c.m_conflictingBoundaries.begin(), c.m_conflictingBoundaries.end(), BeInt64Id(1)) &&
            c.m_conflictingBoundaries.end() != std::find(c.m_conflictingBoundaries.begin(), c.m_conflictingBoundaries.end(), BeInt64Id(3));
        });
    CurveVectorPtr expectedConflict3Geometry = CurveVector::CreateRectangle(-1, 6, 0, 10, 0);

    ASSERT_TRUE(conflicts.end() != conflict0);
    ASSERT_TRUE(conflicts.end() != conflict1);
    ASSERT_TRUE(conflicts.end() != conflict2);
    ASSERT_TRUE(conflicts.end() != conflict3);
    ASSERT_TRUE(conflict0->m_geometry.IsValid());
    ASSERT_TRUE(conflict1->m_geometry.IsValid());
    ASSERT_TRUE(conflict2->m_geometry.IsValid());
    ASSERT_TRUE(conflict3->m_geometry.IsValid());
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*expectedConflict0Geometry, *conflict0->m_geometry));
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*expectedConflict1Geometry, *conflict1->m_geometry));
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*expectedConflict2Geometry, *conflict2->m_geometry));
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*expectedConflict3Geometry, *conflict3->m_geometry));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, InnerBoundaryOverlapsOtherInnerBoundaryThatHasAHole)
    {
    ConflictSolver sut;

    CurveVectorPtr expectedConflict = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
    CurveVectorPtr expectedConflictOuter = CurveVector::CreateRectangle(0, 0, 10, 10, 0, CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr expectedConflictInner = CurveVector::CreateLinear({{2,2,0}, {2,8,0}, {8,8,0}, {8,2,0}}, CurveVector::BOUNDARY_TYPE_Inner);
    expectedConflict->Add(expectedConflictOuter);
    expectedConflict->Add(expectedConflictInner);
    sut.AddInnerBoundary(*expectedConflict, BeInt64Id(1));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(0, 0, 10, 10, 0), BeInt64Id(2));

    bvector<Conflict> conflicts = sut.Solve();
    ASSERT_EQ(1, conflicts.size());
    bset<BeInt64Id> conflictingBoundaries = conflicts.front().m_conflictingBoundaries;
    ASSERT_EQ(2, conflictingBoundaries.size());
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(1)));
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(2)));
    CurveVectorPtr actualConflict = conflicts.front().m_geometry;
    ASSERT_TRUE(actualConflict.IsValid());
    ASSERT_EQ(CurveVector::BOUNDARY_TYPE_ParityRegion, actualConflict->GetBoundaryType());
    ASSERT_EQ(2, actualConflict->size());

    auto actualConflictOuter = std::find_if(actualConflict->begin(), actualConflict->end(), [&] (ICurvePrimitivePtr const& actual)
        {
        return actual->GetCurvePrimitiveType() == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector &&
            nullptr != actual->GetChildCurveVectorCP() &&
            GeometryUtils::IsSameSingleLoopGeometry(*actual->GetChildCurveVectorCP(), *expectedConflictOuter);
        });
    auto actualConflictInner = std::find_if(actualConflict->begin(), actualConflict->end(), [&] (ICurvePrimitivePtr const& actual)
        {
        return actual->GetCurvePrimitiveType() == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector &&
            nullptr != actual->GetChildCurveVectorCP() && 
            GeometryUtils::IsSameSingleLoopGeometry(*actual->GetChildCurveVectorCP(), *expectedConflictInner);
        });

    ASSERT_TRUE(actualConflict->end() != actualConflictOuter);
    ASSERT_TRUE(actualConflict->end() != actualConflictInner);
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, 2OverlappingInnerBoundariesAboveXYPlane)
    {
    Transform localToWorld = Transform::From(DPoint3d::From(0,0,3));
    Transform worldToLocal = localToWorld.ValidatedInverse();

    ConflictSolver sut(localToWorld);
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(0, 0, 10, 10, 3), BeInt64Id(1));
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(5, 0, 15, 10, 3), BeInt64Id(2));

    bvector<Conflict> conflicts = sut.Solve();
    ASSERT_EQ(1, conflicts.size());
    bset<BeInt64Id> conflictingBoundaries = conflicts.front().m_conflictingBoundaries;
    ASSERT_EQ(2, conflictingBoundaries.size());
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(1)));
    ASSERT_TRUE(conflictingBoundaries.end() != std::find(conflictingBoundaries.begin(), conflictingBoundaries.end(), BeInt64Id(2)));

    CurveVectorPtr expectedConflict = CurveVector::CreateRectangle(5, 0, 10, 10, 3);
    CurveVectorPtr actualConflict = conflicts.front().m_geometry;
    ASSERT_TRUE(actualConflict.IsValid());
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*actualConflict, *expectedConflict));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, InnerBoundaryWithNoArea)
    {
    ConflictSolver sut;
    sut.AddContainerBoundary(*CurveVector::CreateRectangle(0, 0, 10, 10, 0), BeInt64Id(1));
    sut.AddInnerBoundary(*CurveVector::CreateLinear({{1,1,0},{3,1,0}}, CurveVector::BOUNDARY_TYPE_Outer), BeInt64Id(2));

    ASSERT_TRUE(sut.Solve().empty());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConflictSolverTestFixture, InnerBoundaryWithNoArea_OverlapsOtherInnerBoundaryThatHasArea)
    {
    ConflictSolver sut;
    sut.AddInnerBoundary(*CurveVector::CreateRectangle(0, 0, 10, 10, 0), BeInt64Id(1));
    sut.AddInnerBoundary(*CurveVector::CreateLinear({{-1,1,0},{3,1,0}}, CurveVector::BOUNDARY_TYPE_Outer), BeInt64Id(2));

    ASSERT_TRUE(sut.Solve().empty());
    }