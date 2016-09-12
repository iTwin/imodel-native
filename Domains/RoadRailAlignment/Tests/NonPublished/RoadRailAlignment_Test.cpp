#include "../BackDoor/PublicApi/BackDoor/RoadRailAlignment/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailAlignmentTests, BasicAlignmentTest)
    {
    DgnModelId modelId;
    DgnDbPtr projectPtr = CreateProject(L"BasicAlignmentTest.bim", modelId);
    ASSERT_TRUE(projectPtr.IsValid());

    auto alignModelPtr = AlignmentModel::Get(*projectPtr, modelId);
    auto alignmentPtr = Alignment::Create(*alignModelPtr);
    alignmentPtr->SetCode(RoadRailAlignmentDomain::CreateCode(*projectPtr, "ALG-1"));
    ASSERT_TRUE(alignmentPtr->Insert().IsValid());

    // Create Alignment
    DPoint2d pntsHoriz2d[]{ { 0, 0 },{ 50, 0 },{ 100, 0 },{ 150, 0 } };
    CurveVectorPtr horizAlignVecPtr = CurveVector::CreateLinear(pntsHoriz2d, 4);
    auto horizAlignmPtr = AlignmentHorizontal::Create(*alignmentPtr, *horizAlignVecPtr);
    ASSERT_TRUE(horizAlignmPtr->Insert().IsValid());

    DPoint2d pntsVert2d[]{ { 0, 0 },{ 150, 0 } };
    CurveVectorPtr vertAlignVecPtr = CurveVector::CreateLinear(pntsVert2d, 2);
    auto verticalAlignmPtr = AlignmentVertical::Create(*alignmentPtr, *vertAlignVecPtr);
    ASSERT_TRUE(verticalAlignmPtr->Insert().IsValid());

    ASSERT_EQ(DgnDbStatus::Success, alignmentPtr->SetMainVertical(*verticalAlignmPtr));
    ASSERT_TRUE(alignmentPtr->Update().IsValid());

    ASSERT_EQ(horizAlignmPtr->GetElementId(), alignmentPtr->QueryHorizontal()->GetElementId());
    ASSERT_EQ(verticalAlignmPtr->GetElementId(), alignmentPtr->QueryMainVertical()->GetElementId());

    auto verticalIds = alignmentPtr->QueryAlignmentVerticalIds();
    ASSERT_EQ(1, verticalIds.size());
    ASSERT_EQ(verticalAlignmPtr->GetElementId(), *verticalIds.begin());

    // Create Stations
    auto station1Ptr = AlignmentStation::Create(*alignmentPtr, DistanceExpression(50.0), 100.0);
    ASSERT_TRUE(station1Ptr->Insert().IsValid());

    auto station2Ptr = AlignmentStation::Create(*alignmentPtr, DistanceExpression(100.0), 200.0);
    ASSERT_TRUE(station2Ptr->Insert().IsValid());
    }