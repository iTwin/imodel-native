#include "../BackDoor/PublicApi/BackDoor/RoadRailPhysical/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailPhysicalTests, BasicRoadwayTest)
    {
#pragma region SetUp
    DgnDbPtr projectPtr = CreateProject(L"BasicRoadwayTest.bim");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId alignmentModelId = QueryFirstModelIdOfType(*projectPtr, AlignmentModel::QueryClassId(*projectPtr));
    auto alignModelPtr = AlignmentModel::Get(*projectPtr, alignmentModelId);

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
    auto verticalAlignmPtr = VerticalAlignment::Create(*verticalModelPtr, *vertAlignVecPtr);
    ASSERT_TRUE(verticalAlignmPtr->InsertAsMainVertical().IsValid());
#pragma endregion

#pragma region Create Road Elements
    DgnModelId roadwayStandardsModelId = QueryFirstModelIdOfType(*projectPtr, RoadwayStandardsModel::QueryClassId(*projectPtr));
    auto roadwayStandardsModelPtr = RoadwayStandardsModel::Get(*projectPtr, roadwayStandardsModelId);
    auto roadTravelwayDefPtr = RoadTravelwayDefinition::Create(*roadwayStandardsModelPtr, "2 lane");

    TypicalSectionPortionBreakDownModelPtr breakDownModelPtr;
    ASSERT_TRUE(roadTravelwayDefPtr->Insert(breakDownModelPtr).IsValid());

    auto overallTypicalSectionPtr = OverallTypicalSection::Create(*roadwayStandardsModelPtr, "rural 2 lanes");

    OverallTypicalSectionBreakDownModelPtr overallTypicalSectionModelPtr;
    ASSERT_TRUE(overallTypicalSectionPtr->Insert(overallTypicalSectionModelPtr).IsValid());

    DgnModelId physicalModelId = QueryFirstModelIdOfType(*projectPtr, 
        DgnClassId(projectPtr->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalModel)));
    auto physicalModelPtr = projectPtr->Models().Get<PhysicalModel>(physicalModelId);

    // Create Roadway
    auto roadwayPtr = Roadway::Create(*physicalModelPtr);
    StatusAspect::Set(*roadwayPtr, *StatusAspect::Create(StatusAspect::Status::Proposed));
    roadwayPtr->SetAlignment(alignmentPtr.get());
    auto roadwayCPtr = roadwayPtr->Insert();
    ASSERT_TRUE(roadwayCPtr.IsValid());    
    ASSERT_EQ(StatusAspect::Status::Proposed, StatusAspect::Get(*roadwayCPtr)->GetStatus());
    ASSERT_EQ(alignmentPtr->GetElementId(), roadwayCPtr->GetAlignmentId());

    alignmentPtr->SetILinearElementSource(roadwayPtr.get());
    ASSERT_TRUE(alignmentPtr->Update().IsValid());

    auto linearElements = roadwayPtr->QueryLinearElements();
    ASSERT_EQ(1, linearElements.size());
    ASSERT_EQ(alignmentPtr->GetElementId(), *linearElements.begin());

    auto designSpeedDefPtr = InsertRoadDesignSpeedDefinition(*projectPtr);
    auto designSpeedPtr = DesignSpeed::Create(*roadwayCPtr, *designSpeedDefPtr, 0, 150);
    ASSERT_TRUE(designSpeedPtr->Insert().IsValid());

    // Create RoadSegment #1
    auto roadSegment1Ptr = RegularTravelwaySegment::Create(*roadwayCPtr, 0, 50, *roadTravelwayDefPtr);
    auto roadSegment1CPtr = roadSegment1Ptr->Insert();
    ASSERT_TRUE(roadSegment1CPtr.IsValid());
    ASSERT_EQ(alignmentPtr->GetElementId(), roadSegment1CPtr->GetLinearElementId());

    // Create TransitionSegment
    auto transitionPtr = TravelwayTransition::Create(*roadwayCPtr, 50, 100);
    auto transitionCPtr = transitionPtr->Insert();
    ASSERT_TRUE(transitionCPtr.IsValid());
    ASSERT_EQ(alignmentPtr->GetElementId(), transitionCPtr->GetLinearElementId());

    // Create RoadSegment #2
    auto roadSegment2Ptr = RegularTravelwaySegment::Create(*roadwayCPtr, 100, 150, *roadTravelwayDefPtr);
    auto roadSegment2CPtr = roadSegment2Ptr->Insert();
    ASSERT_TRUE(roadSegment2CPtr.IsValid());
    ASSERT_EQ(alignmentPtr->GetElementId(), roadSegment2CPtr->GetLinearElementId());
#pragma endregion

#pragma region Station-change Cascading - First segment
    roadSegment1Ptr->SetToDistanceAlong(35);
    roadSegment1Ptr->SetCascadeLocationChangesActionFlag(CascadeLocationChangesAction::OnlyIfLocationsChanged);
    ASSERT_TRUE(roadSegment1Ptr->Update().IsValid());

    transitionCPtr = TravelwayTransition::Get(*projectPtr, transitionPtr->GetElementId());
    ASSERT_DOUBLE_EQ(35, transitionCPtr->GetFromDistanceAlong());
    ASSERT_DOUBLE_EQ(100, transitionCPtr->GetToDistanceAlong());
#pragma endregion

#pragma region Station-change Cascading - Middle segment
    transitionPtr = dynamic_cast<TravelwayTransitionP>(transitionCPtr->CopyForEdit().get());
    transitionPtr->SetFromDistanceAlong(45);
    transitionPtr->SetToDistanceAlong(120);
    transitionPtr->SetCascadeLocationChangesActionFlag(CascadeLocationChangesAction::OnlyIfLocationsChanged);
    ASSERT_TRUE(transitionPtr->Update().IsValid());

    roadSegment1CPtr = RegularTravelwaySegment::Get(*projectPtr, roadSegment1Ptr->GetElementId());
    ASSERT_DOUBLE_EQ(0, roadSegment1CPtr->GetFromDistanceAlong());
    ASSERT_DOUBLE_EQ(45, roadSegment1CPtr->GetToDistanceAlong());

    roadSegment2CPtr = RegularTravelwaySegment::Get(*projectPtr, roadSegment2Ptr->GetElementId());
    ASSERT_DOUBLE_EQ(120, roadSegment2CPtr->GetFromDistanceAlong());
    ASSERT_DOUBLE_EQ(150, roadSegment2CPtr->GetToDistanceAlong());
#pragma endregion

#pragma region Station-change Cascading - Third segment
    roadSegment2Ptr = dynamic_cast<RegularTravelwaySegmentP>(roadSegment2CPtr->CopyForEdit().get());
    roadSegment2Ptr->SetFromDistanceAlong(90);
    roadSegment2Ptr->SetCascadeLocationChangesActionFlag(CascadeLocationChangesAction::OnlyIfLocationsChanged);
    ASSERT_TRUE(roadSegment2Ptr->Update().IsValid());

    transitionCPtr = TravelwayTransition::Get(*projectPtr, transitionPtr->GetElementId());
    ASSERT_DOUBLE_EQ(45, transitionCPtr->GetFromDistanceAlong());
    ASSERT_DOUBLE_EQ(90, transitionCPtr->GetToDistanceAlong());
#pragma endregion
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailPhysicalTests, BasicTypicalSectionTest)
    {
    DgnDbPtr projectPtr = CreateProject(L"BasicRoadwayTest.bim");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId roadwayStandardsModelId = QueryFirstModelIdOfType(*projectPtr, RoadwayStandardsModel::QueryClassId(*projectPtr));
    auto roadwayStandardsModelPtr = RoadwayStandardsModel::Get(*projectPtr, roadwayStandardsModelId);
    auto roadTravelwayDefPtr = RoadTravelwayDefinition::Create(*roadwayStandardsModelPtr, "4 lanes");
    auto travelwayRightSideDefPtr = TravelwaySideDefinition::Create(*roadwayStandardsModelPtr, "Paved+Unpaved Shoulder right-side");
    auto sideSlopeRightSideDefPtr = TravelwaySideDefinition::Create(*roadwayStandardsModelPtr, "Side-Slope Conditions right-side");

    TypicalSectionPortionBreakDownModelPtr twBreakDownModelPtr;
    ASSERT_TRUE(roadTravelwayDefPtr->Insert(twBreakDownModelPtr).IsValid());

    TypicalSectionPortionBreakDownModelPtr shoulderRBreakDownModelPtr;
    ASSERT_TRUE(travelwayRightSideDefPtr->Insert(shoulderRBreakDownModelPtr).IsValid());

    TypicalSectionPortionBreakDownModelPtr sideSlopeRBreakDownModelPtr;
    ASSERT_TRUE(sideSlopeRightSideDefPtr->Insert(sideSlopeRBreakDownModelPtr).IsValid());

    InsertTestPointNames(*roadwayStandardsModelPtr);
    InsertFourLanes(*twBreakDownModelPtr);
    InsertShouldersRightSide(*shoulderRBreakDownModelPtr);
    InsertSideSlopeRightSide(*sideSlopeRBreakDownModelPtr);

    auto overallTypicalSectionPtr = OverallTypicalSection::Create(*roadwayStandardsModelPtr, "4 lanes with shoulders");

    OverallTypicalSectionBreakDownModelPtr overallTypicalSectionModelPtr;
    ASSERT_TRUE(overallTypicalSectionPtr->Insert(overallTypicalSectionModelPtr).IsValid());

    auto alignmentPlaceHolderCPtr = OverallTypicalSectionAlignment::CreateAndInsert(*overallTypicalSectionModelPtr, { 0, 0 });
    ASSERT_EQ(DgnDbStatus::Success, OverallTypicalSection::SetMainAlignment(*overallTypicalSectionPtr, *alignmentPlaceHolderCPtr));

    auto travelwayDefPortionPtr = OverallTypicalSectionPortion::Create(*overallTypicalSectionModelPtr, *roadTravelwayDefPtr, *alignmentPlaceHolderCPtr);
    ASSERT_TRUE(travelwayDefPortionPtr->Insert().IsValid());

    auto rightSideDefPortionPtr = OverallTypicalSectionPortion::Create(*overallTypicalSectionModelPtr, *travelwayRightSideDefPtr, *alignmentPlaceHolderCPtr);
    ASSERT_TRUE(rightSideDefPortionPtr->Insert().IsValid());

    auto rightSideSlopeDefPortionPtr = OverallTypicalSectionPortion::Create(*overallTypicalSectionModelPtr, *sideSlopeRightSideDefPtr, *alignmentPlaceHolderCPtr);
    ASSERT_TRUE(rightSideSlopeDefPortionPtr->Insert().IsValid());
    }