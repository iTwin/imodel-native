#include "../BackDoor/PublicApi/BackDoor/RoadRailAlignment/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailAlignmentTests, BasicAlignmentTest)
    {
    DgnDbPtr projectPtr = CreateProject(L"BasicAlignmentTest.bim");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId modelId = QueryFirstAlignmentModelId(*projectPtr);
    auto alignModelPtr = AlignmentModel::Get(*projectPtr, modelId);

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

    ASSERT_EQ(horizAlignmPtr->GetElementId(), alignmentPtr->QueryHorizontal()->GetElementId());
    ASSERT_EQ(verticalAlignmPtr->GetElementId(), alignmentPtr->QueryMainVertical()->GetElementId());

    auto verticalIds = alignmentPtr->QueryAlignmentVerticalIds();
    ASSERT_EQ(1, verticalIds.size());
    ASSERT_EQ(verticalAlignmPtr->GetElementId(), *verticalIds.begin());

    // Create Referents
    auto referent1Ptr = AlignmentReferent::Create(*alignmentPtr, DistanceExpression(50.0), 100.0);
    ASSERT_TRUE(referent1Ptr->Insert().IsValid());

    auto referent2Ptr = AlignmentReferent::Create(*alignmentPtr, DistanceExpression(100.0), 200.0);
    ASSERT_TRUE(referent2Ptr->Insert().IsValid());

    // Segmentation
    auto segments = alignmentPtr->QuerySegments();
    ASSERT_EQ(2, segments.size());
    }