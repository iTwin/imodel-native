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
    auto roadSegment1Ptr = RoadSegment::Create(*roadRangeCPtr, 0, 50);
    auto roadSegment1CPtr = roadSegment1Ptr->Insert();
    ASSERT_TRUE(roadSegment1CPtr.IsValid());
    ASSERT_EQ(alignmentPtr->GetElementId(), roadSegment1CPtr->GetLinearElementId());

    // Create TransitionSegment
    auto transitionPtr = TransitionSegment::Create(*roadRangeCPtr, 50, 100);
    auto transitionCPtr = transitionPtr->Insert();
    ASSERT_TRUE(transitionCPtr.IsValid());
    ASSERT_EQ(alignmentPtr->GetElementId(), transitionCPtr->GetLinearElementId());

    // Create RoadSegment #2
    auto roadSegment2Ptr = RoadSegment::Create(*roadRangeCPtr, 100, 150);
    ASSERT_TRUE(roadSegment2Ptr->Insert().IsValid());
#pragma endregion

#pragma region Station-change Cascading
    roadSegment1Ptr->SetToDistanceAlong(35);
    roadSegment1Ptr->SetCascadeLocationChangesActionFlag(CascadeLocationChangesAction::OnlyIfLocationsChanged);
    ASSERT_TRUE(roadSegment1Ptr->Update().IsValid());
    ASSERT_DOUBLE_EQ(35, roadSegment1Ptr->GetToDistanceAlong());

    transitionCPtr = TransitionSegment::Get(*projectPtr, transitionPtr->GetElementId());
    ASSERT_DOUBLE_EQ(35, transitionCPtr->GetFromDistanceAlong());
    ASSERT_DOUBLE_EQ(100, transitionCPtr->GetToDistanceAlong());
#pragma endregion
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailPhysicalTests, BasicRoadRangeWithBridgeTest)
    {
#pragma region SetUp
    DgnDbPtr projectPtr = CreateProject(L"BasicRoadRangeWithBridgeTest.bim");
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
    auto roadSegment1Ptr = RoadSegment::Create(*roadRangeCPtr, 0, 10);
    auto roadSegment1CPtr = roadSegment1Ptr->Insert();
    ASSERT_TRUE(roadSegment1CPtr.IsValid());
    ASSERT_EQ(alignmentPtr->GetElementId(), roadSegment1CPtr->GetLinearElementId());

    // Create TransitionSegment #1
    auto transition1Ptr = TransitionSegment::Create(*roadRangeCPtr, 10, 20);
    auto transition1CPtr = transition1Ptr->Insert();
    ASSERT_TRUE(transition1CPtr.IsValid());
    ASSERT_EQ(alignmentPtr->GetElementId(), transition1CPtr->GetLinearElementId());

    // Create RoadSegmentOnBridge
    auto roadOnBridgePtr = RoadSegmentOnBridge::Create(*roadRangeCPtr, 20, 120);
    auto roadOnBridgeCPtr = roadOnBridgePtr->Insert();
    ASSERT_TRUE(roadOnBridgeCPtr.IsValid());

    // Create Bridge
    auto bridgePtr = Bridge::Create(*roadOnBridgeCPtr);
    auto bridgeCPtr = bridgePtr->Insert();
    ASSERT_TRUE(bridgeCPtr.IsValid());

    // Create Abutment - Pier - Abutment
    auto abutment1Ptr = BridgeAbutment::Create(*bridgeCPtr, 0);
    ASSERT_TRUE(abutment1Ptr->Insert().IsValid());

    auto pierPtr = BridgePier::Create(*bridgeCPtr, 50);
    ASSERT_TRUE(pierPtr->Insert().IsValid());

    auto abutment2Ptr = BridgeAbutment::Create(*bridgeCPtr, 100);
    ASSERT_TRUE(abutment2Ptr->Insert().IsValid());

    // Create TransitionSegment #2
    auto transition2Ptr = TransitionSegment::Create(*roadRangeCPtr, 1, 2);
    auto transition2CPtr = transition2Ptr->Insert();
    ASSERT_TRUE(transition2CPtr.IsValid());
    ASSERT_EQ(alignmentPtr->GetElementId(), transition2CPtr->GetLinearElementId());

    // Create RoadSegment #2
    auto roadSegment2Ptr = RoadSegment::Create(*roadRangeCPtr, 100, 150);
    ASSERT_TRUE(roadSegment2Ptr->Insert().IsValid());
#pragma endregion
    }
