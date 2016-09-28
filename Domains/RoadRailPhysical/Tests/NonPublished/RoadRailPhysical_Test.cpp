#include "../BackDoor/PublicApi/BackDoor/RoadRailPhysical/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailPhysicalTests, BasicRoadRangeTest)
    {
#pragma region SetUp
    DgnDbPtr projectPtr = CreateProject(L"BasicRoadRangeTest.bim");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId alignmentModelId = QueryFirstAlignmentModelId(*projectPtr);
    auto alignModelPtr = AlignmentModel::Get(*projectPtr, alignmentModelId);

    // Create Alignment
    auto alignmentPtr = Alignment::Create(*alignModelPtr);
    alignmentPtr->SetCode(RoadRailAlignmentDomain::CreateCode(*projectPtr, "ALG-1"));
    ASSERT_TRUE(alignmentPtr->Insert().IsValid());

    // Create Horizontal 
    DPoint2d pntsHoriz2d[]{ { 0, 0 },{ 50, 0 },{ 100, 0 },{ 150, 0 } };
    CurveVectorPtr horizAlignVecPtr = CurveVector::CreateLinear(pntsHoriz2d, 4);
    auto horizAlignmPtr = AlignmentHorizontal::Create(*alignmentPtr, *horizAlignVecPtr);
    ASSERT_TRUE(horizAlignmPtr->Insert().IsValid());

    // Create Vertical
    DPoint2d pntsVert2d[]{ { 0, 0 },{ 150, 0 } };
    CurveVectorPtr vertAlignVecPtr = CurveVector::CreateLinear(pntsVert2d, 2);
    auto verticalAlignmPtr = AlignmentVertical::Create(*alignmentPtr, *vertAlignVecPtr);
    ASSERT_TRUE(verticalAlignmPtr->InsertAsMainVertical().IsValid());
#pragma endregion

#pragma region Create Road Elements
    DgnModelId physicalModelId = QueryFirstPhysicalModelId(*projectPtr);
    auto physicalModelPtr = projectPtr->Models().Get<PhysicalModel>(physicalModelId);

    // Create RoadRange
    auto roadRangePtr = RoadRange::Create(*physicalModelPtr);
    StatusAspect::Set(*roadRangePtr, *StatusAspect::Create(StatusAspect::Status::Proposed));
    auto roadRangeCPtr = roadRangePtr->InsertWithAlignment(*alignmentPtr);
    ASSERT_TRUE(roadRangeCPtr.IsValid());    
    ASSERT_EQ(StatusAspect::Status::Proposed, StatusAspect::Get(*roadRangeCPtr)->GetStatus());
    ASSERT_EQ(alignmentPtr->GetElementId(), roadRangeCPtr->QueryAlignmentId());

    // Create RoadSegment #1
    auto roadSegment1Ptr = RoadSegment::Create(*roadRangeCPtr, DistanceExpression(0), DistanceExpression(50));
    auto roadSegment1CPtr = roadSegment1Ptr->Insert();
    ASSERT_TRUE(roadSegment1CPtr.IsValid());
    ASSERT_EQ(alignmentPtr->GetElementId(), roadSegment1CPtr->GetLinearElementId());

    // Create TransitionSegment
    auto transitionPtr = TransitionSegment::Create(*roadRangeCPtr, DistanceExpression(50), DistanceExpression(100));
    auto transitionCPtr = transitionPtr->Insert();
    ASSERT_TRUE(transitionCPtr.IsValid());
    ASSERT_EQ(alignmentPtr->GetElementId(), transitionCPtr->GetLinearElementId());

    // Create RoadSegment #2
    auto roadSegment2Ptr = RoadSegment::Create(*roadRangeCPtr, DistanceExpression(100), DistanceExpression(150));
    ASSERT_TRUE(roadSegment2Ptr->Insert().IsValid());
#pragma endregion

#pragma region Station-change Cascading
    roadSegment1Ptr->GetFromToLocationP()->GetToPositionR().SetDistanceAlongFromStart(35);
    roadSegment1Ptr->SetCascadeLocationChangesActionFlag(CascadeLocationChangesAction::OnlyIfLocationsChanged);
    ASSERT_TRUE(roadSegment1Ptr->Update().IsValid());
    ASSERT_DOUBLE_EQ(35, roadSegment1Ptr->GetFromToLocation()->GetToPosition().GetDistanceAlongFromStart());

    transitionCPtr = TransitionSegment::Get(*projectPtr, transitionPtr->GetElementId());
    ASSERT_DOUBLE_EQ(35, transitionCPtr->GetFromToLocation()->GetFromPosition().GetDistanceAlongFromStart());
    ASSERT_DOUBLE_EQ(100, transitionCPtr->GetFromToLocation()->GetToPosition().GetDistanceAlongFromStart());
#pragma endregion
    }
