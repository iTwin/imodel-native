#include "../BackDoor/PublicApi/BackDoor/RoadRailAlignment/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailAlignmentTests, BasicAlignmentTest)
    {
    DgnDbPtr projectPtr = CreateProject(L"BasicAlignmentTest.bim");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId modelId = QueryFirstModelIdOfType(*projectPtr, AlignmentModel::QueryClassId(*projectPtr));
    auto alignModelPtr = AlignmentModel::Get(*projectPtr, modelId);

    // Create Alignment
    auto alignmentPtr = Alignment::Create(*alignModelPtr);
    alignmentPtr->SetCode(RoadRailAlignmentDomain::CreateCode(*alignModelPtr, "ALG-1"));
    ASSERT_TRUE(alignmentPtr->Insert().IsValid());

    // Create Horizontal 
    DPoint2d pntsHoriz2d[]{ { 0, 0 },{ 50, 0 },{ 100, 0 },{ 150, 0 } };
    CurveVectorPtr horizAlignVecPtr = CurveVector::CreateLinear(pntsHoriz2d, 4);
    auto horizAlignmPtr = HorizontalAlignment::Create(*alignmentPtr, *horizAlignVecPtr);
    ASSERT_EQ(DgnDbStatus::Success, horizAlignmPtr->GenerateElementGeom());
    ASSERT_TRUE(horizAlignmPtr->Insert().IsValid());
    ASSERT_TRUE(horizAlignmPtr->GetGeometry().IsOpenPath());

    // Create Vertical
    auto verticalModelPtr = VerticalAlignmentModel::Create(VerticalAlignmentModel::CreateParams(*projectPtr, alignmentPtr->GetElementId()));
    ASSERT_EQ(DgnDbStatus::Success, verticalModelPtr->Insert());

    DPoint2d pntsVert2d[]{ { 0, 0 },{ 150, 0 } };
    CurveVectorPtr vertAlignVecPtr = CurveVector::CreateLinear(pntsVert2d, 2);
    auto verticalAlignmPtr = VerticalAlignment::Create(*verticalModelPtr, *vertAlignVecPtr);
    ASSERT_TRUE(verticalAlignmPtr->InsertAsMainVertical().IsValid());

    ASSERT_EQ(horizAlignmPtr->GetElementId(), alignmentPtr->QueryHorizontal()->GetElementId());
    ASSERT_EQ(verticalAlignmPtr->GetElementId(), alignmentPtr->QueryMainVertical()->GetElementId());

    auto verticalIds = alignmentPtr->QueryVerticalAlignmentIds();
    ASSERT_EQ(1, verticalIds.size());
    ASSERT_EQ(verticalAlignmPtr->GetElementId(), *verticalIds.begin());

    // Get AlignmentPair
    auto alignmentPairPtr = alignmentPtr->QueryMainPair();
    ASSERT_TRUE(alignmentPairPtr != nullptr);
    ASSERT_DOUBLE_EQ(150.0, alignmentPairPtr->LengthXY());
    ASSERT_TRUE(alignmentPairPtr->VerticalCurveVector().IsValid());

    // Delete-cascade
    ASSERT_EQ(DgnDbStatus::Success, alignmentPtr->Delete());
    
    ASSERT_TRUE(projectPtr->Elements().GetElement(alignmentPtr->GetElementId()).IsNull());
    ASSERT_TRUE(projectPtr->Elements().GetElement(horizAlignmPtr->GetElementId()).IsNull());
    ASSERT_TRUE(projectPtr->Elements().GetElement(verticalAlignmPtr->GetElementId()).IsNull());
    ASSERT_TRUE(projectPtr->Models().GetModel(verticalModelPtr->GetModelId()).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailAlignmentTests, AlignmentPairEditorTest)
    {
    DgnDbPtr projectPtr = CreateProject(L"AlignmentPairEditorTest.bim");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId modelId = QueryFirstModelIdOfType(*projectPtr, AlignmentModel::QueryClassId(*projectPtr));
    auto alignModelPtr = AlignmentModel::Get(*projectPtr, modelId);

    // Create Horizontal 
    DPoint2d pntsHoriz2d[]{ { 0, 0 },{ 50, 0 },{ 100, 0 },{ 150, 0 } };
    CurveVectorPtr horizAlignVecPtr = CurveVector::CreateLinear(pntsHoriz2d, 4);

    // Create Vertical
    DPoint2d pntsVert2d[]{ { 0, 0 },{ 150, 0 } };
    CurveVectorPtr vertAlignVecPtr = CurveVector::CreateLinear(pntsVert2d, 2);

    auto alignPairPtr = AlignmentPairEditor::Create(*horizAlignVecPtr, vertAlignVecPtr.get());

    // Create Alignment
    auto alignmentPtr = Alignment::Create(*alignModelPtr);
    alignmentPtr->SetCode(RoadRailAlignmentDomain::CreateCode(*alignModelPtr, "ALG-1"));
    ASSERT_TRUE(alignmentPtr->InsertWithMainPair(*alignPairPtr).IsValid());

    ASSERT_TRUE(alignmentPtr->QueryHorizontal()->GetElementId().IsValid());
    ASSERT_TRUE(alignmentPtr->QueryMainVertical()->GetElementId().IsValid());

    auto verticalIds = alignmentPtr->QueryVerticalAlignmentIds();
    ASSERT_EQ(1, verticalIds.size());

    // Get AlignmentPair
    auto alignmentPairPtr = alignmentPtr->QueryMainPair();
    ASSERT_TRUE(alignmentPairPtr != nullptr);
    ASSERT_DOUBLE_EQ(150.0, alignmentPairPtr->LengthXY());
    ASSERT_TRUE(alignmentPairPtr->VerticalCurveVector().IsValid());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailAlignmentTests, AlignmentSegmationTest)
    {
    DgnDbPtr projectPtr = CreateProject(L"AlignmentSegmationTest.bim");
    ASSERT_TRUE(projectPtr.IsValid());

    DgnModelId modelId = QueryFirstModelIdOfType(*projectPtr, AlignmentModel::QueryClassId(*projectPtr));
    auto alignModelPtr = AlignmentModel::Get(*projectPtr, modelId);

    // Create Alignment
    auto alignmentPtr = Alignment::Create(*alignModelPtr);
    alignmentPtr->SetCode(RoadRailAlignmentDomain::CreateCode(*alignModelPtr, "ALG-1"));
    ASSERT_TRUE(alignmentPtr->Insert().IsValid());

    // Create Stations
    auto station1Ptr = AlignmentStation::Create(*alignmentPtr, DistanceExpression(50.0), 100.0);
    ASSERT_TRUE(station1Ptr->Insert().IsValid());
    ASSERT_DOUBLE_EQ(50.0, station1Ptr->GetAtPosition().GetDistanceAlongFromStart());

    auto station2Ptr = AlignmentStation::Create(*alignmentPtr, DistanceExpression(100.0), 200.0);
    ASSERT_TRUE(station2Ptr->Insert().IsValid());

    // Segmentation
    bset<DgnClassId> classIds;
    classIds.insert(AlignmentStation::QueryClassId(*projectPtr));

    auto locations = alignmentPtr->QueryLinearLocations(classIds);
    ASSERT_EQ(2, locations.size());

    locations = alignmentPtr->QueryLinearLocations(classIds, 75.0, 125.0);
    ASSERT_EQ(1, locations.size());

    locations = alignmentPtr->QueryLinearLocations(classIds, NullableDouble(), 75.0);
    ASSERT_EQ(1, locations.size());

    locations = alignmentPtr->QueryLinearLocations(classIds, 125.0);
    ASSERT_EQ(0, locations.size());
    }