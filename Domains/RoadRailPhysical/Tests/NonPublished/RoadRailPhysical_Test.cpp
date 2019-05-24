/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicApi/BackDoor/RoadRailPhysical/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailPhysicalTests, BasicCorridorTest)
    {
#pragma region SetUp
    DgnDbPtr projectPtr = CreateProject(L"BasicCorridorTest.bim");
    ASSERT_TRUE(projectPtr.IsValid());

    auto subjectCPtr = projectPtr->Elements().GetRootSubject();
    auto standardsModel = RoadwayStandardsModelUtilities::Query(*subjectCPtr);
    ASSERT_TRUE(standardsModel.IsValid());
    
    auto physicalPartitionCPtr = projectPtr->Elements().Get<PhysicalPartition>(*PhysicalModelUtilities::QueryPhysicalPartitions(*subjectCPtr).begin());
    auto networkCPtr = RoadRailNetwork::Query(*projectPtr, RoadRailNetwork::CreateCode(*physicalPartitionCPtr->GetSubModel()->ToPhysicalModel(), "Road Network"));
    auto alignmentsCPtr = DesignAlignments::Query(*networkCPtr->GetNetworkModel(), "Design Alignments");
    auto alignModelPtr = alignmentsCPtr->GetAlignmentModel();

    // Create Alignment
    auto alignmentPtr = Alignment::Create(*alignModelPtr);
    alignmentPtr->SetCode(RoadRailAlignmentDomain::CreateCode(*alignModelPtr, "ALG-1"));
    ASSERT_TRUE(alignmentPtr->Insert().IsValid());

    // Create Horizontal 
    DPoint2d pntsHoriz2d[]{ { 0, 0 },{ 50, 0 },{ 100, 0 },{ 150, 0 } };
    CurveVectorPtr horizAlignVecPtr = CurveVector::CreateLinear(pntsHoriz2d, 4);
    auto horizAlignmPtr = HorizontalAlignment::Create(*alignmentPtr, *horizAlignVecPtr);
    ASSERT_TRUE(horizAlignmPtr->Insert().IsValid());

    // Create Vertical
    auto verticalModelPtr = VerticalAlignmentModel::Create(DgnModel::CreateParams(*projectPtr, VerticalAlignmentModel::QueryClassId(*projectPtr),
        alignmentPtr->GetElementId()));
    ASSERT_EQ(DgnDbStatus::Success, verticalModelPtr->Insert());

    DPoint2d pntsVert2d[]{ { 0, 0 },{ 150, 0 } };
    CurveVectorPtr vertAlignVecPtr = CurveVector::CreateLinear(pntsVert2d, 2);
    auto verticalAlignmPtr = VerticalAlignment::Create(*alignmentPtr, *vertAlignVecPtr);
    ASSERT_TRUE(verticalAlignmPtr->InsertAsMainVertical().IsValid());
#pragma endregion

#pragma region Create Road Elements
    // Create Corridor
    auto corridorPtr = Corridor::Create(*networkCPtr);
    corridorPtr->SetDesignAlignment(alignmentPtr.get());
    auto corridorCPtr = corridorPtr->Insert();
    ASSERT_TRUE(corridorCPtr.IsValid());    
    ASSERT_EQ(alignmentPtr->GetElementId(), corridorCPtr->GetDesignAlignmentId());

    alignmentPtr->SetSource(corridorPtr.get());
    ASSERT_TRUE(alignmentPtr->Update().IsValid());

    auto linearElements = corridorPtr->QueryLinearElements();
    ASSERT_EQ(1, linearElements.size());
    ASSERT_EQ(alignmentPtr->GetElementId(), *linearElements.begin());

    auto corridorRangeCPtr = CorridorRange::Insert(*corridorPtr, "Overall Range");

    auto leftRoadwayPtr = Roadway::Create(*corridorRangeCPtr, PathwayElement::Order::LeftMost);
    leftRoadwayPtr->SetDesignAlignment(alignmentPtr.get());
    ASSERT_TRUE(leftRoadwayPtr->Insert().IsValid());

    auto pathwayDesignCriteriaCPtr = PathwayDesignCriteria::Insert(*leftRoadwayPtr);
    ASSERT_TRUE(pathwayDesignCriteriaCPtr.IsValid());

    auto designSpeedDefPtr = DesignSpeedDefinition::Create(*standardsModel, 50.0, DesignSpeedDefinition::UnitSystem::SI);
    ASSERT_TRUE(designSpeedDefPtr->Insert().IsValid());

    auto designSpeedPtr = DesignSpeed::Create(DesignSpeed::CreateFromToParams(*pathwayDesignCriteriaCPtr, *alignmentPtr, *designSpeedDefPtr, *designSpeedDefPtr, 0, 150));
    auto designSpeedCPtr = designSpeedPtr->Insert();
    ASSERT_TRUE(designSpeedCPtr.IsValid());

    auto rightRoadwayPtr = Roadway::Create(*corridorRangeCPtr, PathwayElement::Order::RightMost);
    rightRoadwayPtr->SetDesignAlignment(alignmentPtr.get());
    ASSERT_TRUE(rightRoadwayPtr->Insert().IsValid());

    auto pathwayIds = corridorRangeCPtr->QueryOrderedPathwayIds();
    ASSERT_EQ(2, pathwayIds.size());
    ASSERT_EQ(leftRoadwayPtr->GetElementId(), pathwayIds[0]);
    ASSERT_EQ(rightRoadwayPtr->GetElementId(), pathwayIds[1]);
#pragma endregion
    }