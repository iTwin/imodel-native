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

    RoadwayStandardsModelCPtr standardsModel = RoadwayStandardsModel::Query(*projectPtr->Elements().GetRootSubject());
    ASSERT_TRUE(standardsModel.IsValid());

    auto clPointDefPtr = GenericTypicalSectionPointDefinition::CreateAndInsert(*standardsModel, "CL", "Center-line");
    ASSERT_TRUE(clPointDefPtr.IsValid());

    auto alignModelPtr = AlignmentModel::Query(*projectPtr->Elements().GetRootSubject(), RoadRailAlignmentDomain::GetDesignPartitionName());

    // Create Alignment
    auto alignmentPtr = Alignment::Create(*alignModelPtr);
    alignmentPtr->SetCode(RoadRailAlignmentDomain::CreateCode(*alignModelPtr, "ALG-1"));
    auto associatedFacetPtr = AssociatedFacetAspect::Create(AssociatedFacetAspect::AssociatedFacetEnum::Top);
    AssociatedFacetAspect::Set(*alignmentPtr, *associatedFacetPtr);
    ASSERT_TRUE(alignmentPtr->Insert().IsValid());

    auto associatedFacetCPtr = AssociatedFacetAspect::Get(*alignmentPtr);
    ASSERT_EQ(AssociatedFacetAspect::AssociatedFacetEnum::Top, associatedFacetCPtr->GetAssociatedFacet());

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
    auto physicalModelPtr = PhysicalModelUtilities::QueryPhysicalNetworkModel(*projectPtr->Elements().GetRootSubject(),
        RoadRailPhysicalDomain::GetDefaultPhysicalPartitionName(), RoadRailPhysicalDomain::GetDefaultPhysicalNetworkName());

    // Create Corridor
    auto corridorPtr = Corridor::Create(*physicalModelPtr);
    corridorPtr->SetMainLinearElement(alignmentPtr.get());
    auto corridorCPtr = corridorPtr->Insert();
    ASSERT_TRUE(corridorCPtr.IsValid());    
    ASSERT_EQ(alignmentPtr->GetElementId(), corridorCPtr->GetMainLinearElementId());

    alignmentPtr->SetILinearElementSource(corridorPtr.get());
    ASSERT_TRUE(alignmentPtr->Update().IsValid());

    auto linearElements = corridorPtr->QueryLinearElements();
    ASSERT_EQ(1, linearElements.size());
    ASSERT_EQ(alignmentPtr->GetElementId(), *linearElements.begin());

    auto leftRoadwayPtr = Roadway::Create(*corridorPtr);
    leftRoadwayPtr->SetMainLinearElement(alignmentPtr.get());
    ASSERT_TRUE(leftRoadwayPtr->Insert(PathwayElement::Order::LeftMost).IsValid());

    ASSERT_EQ(DgnDbStatus::Success, ILinearElementUtilities::SetRelatedCorridorPortion(*alignmentPtr, *leftRoadwayPtr, *clPointDefPtr));

    DgnElementId typicalSectionPointDefId;
    auto portionCPtr = ILinearElementUtilities::QueryRelatedCorridorPortion(*alignmentPtr, typicalSectionPointDefId);
    ASSERT_EQ(portionCPtr->GetElementId(), leftRoadwayPtr->GetElementId());
    ASSERT_EQ(typicalSectionPointDefId, clPointDefPtr->GetElementId());

    auto designSpeedDefPtr = DesignSpeedDefinition::Create(*standardsModel, 50.0, DesignSpeedDefinition::UnitSystem::SI);
    ASSERT_TRUE(designSpeedDefPtr->Insert().IsValid());

    auto designSpeedPtr = DesignSpeed::Create(DesignSpeed::CreateFromToParams(*leftRoadwayPtr, *designSpeedDefPtr, 0, 150));
    auto designSpeedCPtr = designSpeedPtr->Insert();
    ASSERT_TRUE(designSpeedCPtr.IsValid());

    auto rightRoadwayPtr = Roadway::Create(*corridorPtr);
    rightRoadwayPtr->SetMainLinearElement(alignmentPtr.get());
    ASSERT_TRUE(rightRoadwayPtr->Insert((int32_t)PathwayElement::Order::RightMost).IsValid());

    ASSERT_EQ(DgnDbStatus::Success, ILinearElementUtilities::SetRelatedCorridorPortion(*alignmentPtr, *rightRoadwayPtr, *clPointDefPtr));

    auto pathwayIds = corridorCPtr->QueryOrderedPathwayIds();
    ASSERT_EQ(2, pathwayIds.size());
    ASSERT_EQ(leftRoadwayPtr->GetElementId(), pathwayIds[0]);
    ASSERT_EQ(rightRoadwayPtr->GetElementId(), pathwayIds[1]);
#pragma endregion
    }