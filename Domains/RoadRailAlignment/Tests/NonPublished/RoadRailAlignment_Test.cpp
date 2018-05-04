#include "../BackDoor/PublicApi/BackDoor/RoadRailAlignment/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailAlignmentTests, BasicAlignmentTest)
    {
    DgnDbPtr projectPtr = CreateProject(L"BasicAlignmentTest.bim");
    ASSERT_TRUE(projectPtr.IsValid());

    auto configurationModelPtr = ConfigurationModel::Query(*projectPtr->Elements().GetRootSubject());
    ASSERT_TRUE(configurationModelPtr.IsValid());

    auto subjectCPtr = configurationModelPtr->GetParentSubject();
    ASSERT_TRUE(subjectCPtr.IsValid());
    ASSERT_EQ(projectPtr->Elements().GetRootSubjectId(), subjectCPtr->GetElementId());

    auto alignModelPtr = AlignmentModel::Query(*subjectCPtr, RoadRailAlignmentDomain::GetDesignPartitionName());

    // Create Alignment
    auto alignmentPtr = Alignment::Create(*alignModelPtr);
    alignmentPtr->SetCode(RoadRailAlignmentDomain::CreateCode(*alignModelPtr, "ALG-1"));
    alignmentPtr->SetStartStation(1000);
    ASSERT_TRUE(alignmentPtr->Insert().IsValid());

    ASSERT_EQ(1, alignModelPtr->QueryAlignmentIds().size());

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
    ASSERT_TRUE(alignmentPairPtr->IsValidVertical());

    // ISpatialLinearElement
    DPoint3d point = alignmentPtr->ToDPoint3d(DistanceExpression(75.0, 10.0, 5.0));
    ASSERT_DOUBLE_EQ(75.0, point.x);
    ASSERT_DOUBLE_EQ(-10.0, point.y);
    ASSERT_DOUBLE_EQ(5.0, point.z);

    DistanceExpression distanceExp = alignmentPtr->ToDistanceExpression(point);
    ASSERT_DOUBLE_EQ(75.0, distanceExp.GetDistanceAlongFromStart());
    ASSERT_DOUBLE_EQ(10.0, distanceExp.GetLateralOffsetFromILinearElement().Value());
    ASSERT_DOUBLE_EQ(5.0, distanceExp.GetVerticalOffsetFromILinearElement().Value());

    // AlignmentStations
    // DistanceAlong    0       30      70      120     150
    // Station          1000    10      10000   100     130

    auto stationPtr = AlignmentStation::Create(AlignmentStation::CreateAtParams(*alignmentPtr, 30, 10));
    auto stationCPtr = stationPtr->Insert();
    ASSERT_TRUE(stationCPtr.IsValid());
    ASSERT_DOUBLE_EQ(30.0, stationCPtr->GetAtDistanceAlongFromStart());
    ASSERT_DOUBLE_EQ(10.0, stationCPtr->GetStation());

    stationPtr = AlignmentStation::Create(AlignmentStation::CreateAtParams(*alignmentPtr, 70, 10000));
    ASSERT_TRUE(stationPtr->Insert().IsValid());

    stationPtr = AlignmentStation::Create(AlignmentStation::CreateAtParams(*alignmentPtr, 120, 100));
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

    ASSERT_TRUE(stationTranslatorPtr->ToDistanceAlongFromStart(-1).IsNull());
    ASSERT_DOUBLE_EQ(0.0, stationTranslatorPtr->ToDistanceAlongFromStart(1000.0).Value());
    ASSERT_DOUBLE_EQ(29.0, stationTranslatorPtr->ToDistanceAlongFromStart(1029.0).Value());
    ASSERT_DOUBLE_EQ(30.0, stationTranslatorPtr->ToDistanceAlongFromStart(10.0).Value());
    ASSERT_DOUBLE_EQ(71.0, stationTranslatorPtr->ToDistanceAlongFromStart(10001.0).Value());
    ASSERT_DOUBLE_EQ(129.0, stationTranslatorPtr->ToDistanceAlongFromStart(109.0).Value());
    ASSERT_DOUBLE_EQ(150.0, stationTranslatorPtr->ToDistanceAlongFromStart(130.0).Value());
    ASSERT_TRUE(stationTranslatorPtr->ToDistanceAlongFromStart(140.0).IsNull());

    // Station formatting
    auto koqCP = projectPtr->Schemas().GetKindOfQuantity(BRRA_SCHEMA_NAME, "STATION");
    auto presentationUnitList = koqCP->GetPresentationUnitList();

    auto mUnitP = UnitRegistry::Instance().LookupUnit("M");
    Quantity qty(stationTranslatorPtr->ToStation(71).Value(), *mUnitP);
    auto fusP = std::find_if(presentationUnitList.begin(), presentationUnitList.end(), 
        [](FormatUnitSet& fus) { return 0 == fus.GetUnitName().CompareTo("M"); });
    ASSERT_TRUE(fusP != presentationUnitList.end());
    ASSERT_TRUE(fusP->GetUnit() != nullptr);

    Utf8String str = fusP->FormatQuantity(qty, nullptr);
    ASSERT_STRCASEEQ("10+001.00", str.c_str());
    
    qty = Quantity(stationTranslatorPtr->ToStation(71).Value(), *mUnitP);
    fusP = std::find_if(presentationUnitList.begin(), presentationUnitList.end(),
        [](FormatUnitSet& fus) { return 0 == fus.GetUnitName().CompareTo("FT"); });
    ASSERT_TRUE(fusP != presentationUnitList.end());
    ASSERT_TRUE(fusP->GetUnit() != nullptr);

    str = fusP->FormatQuantity(qty, nullptr);
    ASSERT_STRCASEEQ("328+11.68", str.c_str());

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

    auto alignPairPtr = AlignmentPairEditor::Create(horizAlignVecPtr.get(), vertAlignVecPtr.get());

    // Create Alignment
    auto alignmentPtr = Alignment::Create(*alignModelPtr);
    alignmentPtr->SetCode(RoadRailAlignmentDomain::CreateCode(*alignModelPtr, "ALG-1"));
    ASSERT_TRUE(alignmentPtr->InsertWithMainPair(*alignPairPtr).IsValid());

    ASSERT_TRUE(alignmentPtr->QueryHorizontal()->GetElementId().IsValid());
    ASSERT_TRUE(alignmentPtr->QueryMainVertical()->GetElementId().IsValid());

    auto verticalIds = alignmentPtr->QueryVerticalAlignmentIds();
    ASSERT_EQ(1, verticalIds.size());

    auto horizAlignPtr = HorizontalAlignment::GetForEdit(*projectPtr, alignmentPtr->QueryHorizontal()->GetElementId());
    ASSERT_EQ(DgnDbStatus::Success, horizAlignPtr->GenerateElementGeom());
    ASSERT_TRUE(horizAlignPtr->Update().IsValid());

    // Get AlignmentPair
    auto alignmentPairPtr = alignmentPtr->QueryMainPair();
    ASSERT_TRUE(alignmentPairPtr != nullptr);
    ASSERT_DOUBLE_EQ(150.0, alignmentPairPtr->LengthXY());
    ASSERT_TRUE(alignmentPairPtr->IsValidVertical());
    }
