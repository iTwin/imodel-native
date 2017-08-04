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
    alignmentPtr->SetStartStation(1000);
    ASSERT_TRUE(alignmentPtr->Insert().IsValid());

    // Create Horizontal 
    DPoint2d pntsHoriz2d[]{ { 0, 0 },{ 50, 0 },{ 100, 0 },{ 150, 0 } };
    CurveVectorPtr horizAlignVecPtr = CurveVector::CreateLinear(pntsHoriz2d, 4);
    auto horizAlignmPtr = HorizontalAlignment::Create(*alignmentPtr, *horizAlignVecPtr);
    ASSERT_EQ(DgnDbStatus::Success, horizAlignmPtr->GenerateElementGeom());
    ASSERT_TRUE(horizAlignmPtr->Insert().IsValid());
    ASSERT_TRUE(horizAlignmPtr->GetGeometry().IsOpenPath());
    ASSERT_EQ(alignmentPtr->GetElementId(), horizAlignmPtr->QueryAlignment()->GetElementId());

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

    // ISpatialLinearElement
    DPoint3d point = alignmentPtr->ToDPoint3d(DistanceExpression(75.0, 10.0, 5.0));
    ASSERT_DOUBLE_EQ(75.0, point.x);
    ASSERT_DOUBLE_EQ(-10.0, point.y);
    ASSERT_DOUBLE_EQ(5.0, point.z);

    ASSERT_EQ(DgnDbStatus::Success, alignmentPtr->GenerateAprox3dGeom());
    ASSERT_TRUE(alignmentPtr->Update().IsValid());

    DistanceExpression distanceExp = alignmentPtr->ToDistanceExpression(point);
    ASSERT_DOUBLE_EQ(75.0, distanceExp.GetDistanceAlongFromStart());
    ASSERT_DOUBLE_EQ(10.0, distanceExp.GetLateralOffsetFromILinearElement().Value());
    ASSERT_DOUBLE_EQ(5.0, distanceExp.GetVerticalOffsetFromILinearElement().Value());

    // AlignmentStations
    // DistanceAlong    0       30      70      120     150
    // Station          1000    10      10000   100     130

    auto stationPtr = AlignmentStation::Create(*alignmentPtr, 30, 10);
    auto stationCPtr = stationPtr->Insert();
    ASSERT_TRUE(stationCPtr.IsValid());
    ASSERT_DOUBLE_EQ(30.0, stationCPtr->GetAtDistanceAlong());
    ASSERT_DOUBLE_EQ(10.0, stationCPtr->GetStation());

    stationPtr = AlignmentStation::Create(*alignmentPtr, 70, 10000);
    ASSERT_TRUE(stationPtr->Insert().IsValid());

    stationPtr = AlignmentStation::Create(*alignmentPtr, 120, 100);
    ASSERT_TRUE(stationPtr->Insert().IsValid());

    auto stationTranslatorPtr = AlignmentStationingTranslator::Create(*alignmentPtr);
    ASSERT_TRUE(stationTranslatorPtr->ToStation(-1).IsNull());
    ASSERT_DOUBLE_EQ(1000.0, stationTranslatorPtr->ToStation(0).Value());
    ASSERT_DOUBLE_EQ(1029.0, stationTranslatorPtr->ToStation(29).Value());
    ASSERT_DOUBLE_EQ(10.0, stationTranslatorPtr->ToStation(30).Value());
    ASSERT_DOUBLE_EQ(10001.0, stationTranslatorPtr->ToStation(71).Value());
    ASSERT_DOUBLE_EQ(109.0, stationTranslatorPtr->ToStation(129).Value());
    ASSERT_DOUBLE_EQ(130.0, stationTranslatorPtr->ToStation(150).Value());
    ASSERT_TRUE(stationTranslatorPtr->ToStation(160).IsNull());

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

    ASSERT_EQ(DgnDbStatus::Success, alignmentPtr->GenerateAprox3dGeom());
    ASSERT_TRUE(alignmentPtr->Update().IsValid());

    auto horizAlignPtr = HorizontalAlignment::GetForEdit(*projectPtr, alignmentPtr->QueryHorizontal()->GetElementId());
    ASSERT_EQ(DgnDbStatus::Success, horizAlignPtr->GenerateElementGeom());
    ASSERT_TRUE(horizAlignPtr->Update().IsValid());

    // Get AlignmentPair
    auto alignmentPairPtr = alignmentPtr->QueryMainPair();
    ASSERT_TRUE(alignmentPairPtr != nullptr);
    ASSERT_DOUBLE_EQ(150.0, alignmentPairPtr->LengthXY());
    ASSERT_TRUE(alignmentPairPtr->VerticalCurveVector().IsValid());  

    // Setting up default views
    auto displayStyle2dPtr = CreateDisplayStyle2d(projectPtr->GetDictionaryModel());
    auto drawingCategorySelectorPtr = CreateDrawingCategorySelector(projectPtr->GetDictionaryModel());
    ASSERT_EQ(BentleyStatus::SUCCESS, Create2dView(projectPtr->GetDictionaryModel(), "Test-2d", 
        *drawingCategorySelectorPtr, alignmentPtr->QueryHorizontal()->GetModelId(), *displayStyle2dPtr));

    auto displayStyle3dPtr = CreateDisplayStyle3d(projectPtr->GetDictionaryModel());
    auto spatialCategorySelectorPtr = CreateSpatialCategorySelector(projectPtr->GetDictionaryModel());
    auto model3dSelectorPtr = CreateModelSelector(projectPtr->GetDictionaryModel(), "Default-3d");
    model3dSelectorPtr->AddModel(alignModelPtr->GetModelId());

    ASSERT_EQ(BentleyStatus::SUCCESS, Create3dView(projectPtr->GetDictionaryModel(), "Test-3d", 
        *spatialCategorySelectorPtr, *model3dSelectorPtr, *displayStyle3dPtr));
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
    auto station1Ptr = AlignmentStation::Create(*alignmentPtr, 50.0, 100.0);
    ASSERT_TRUE(station1Ptr->Insert().IsValid());
    ASSERT_DOUBLE_EQ(50.0, station1Ptr->GetAtDistanceAlong());
    ASSERT_DOUBLE_EQ(100.0, station1Ptr->GetStation());
    
    auto station2Ptr = AlignmentStation::Create(*alignmentPtr, 100.0, 200.0);
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