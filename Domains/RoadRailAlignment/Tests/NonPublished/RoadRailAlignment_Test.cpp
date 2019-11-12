/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicApi/BackDoor/RoadRailAlignment/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailAlignmentTests, BasicAlignmentTest)
    {
    DgnDbPtr projectPtr = CreateProject(L"BasicAlignmentTest.bim");
    ASSERT_TRUE(projectPtr.IsValid());

    auto subjectCPtr = projectPtr->Elements().GetRootSubject();
    auto configurationModelPtr = RoadRailAlignmentDomain::QueryConfigurationModel(*subjectCPtr);
    ASSERT_TRUE(configurationModelPtr.IsValid());

    auto physicalModelPtr = SetUpPhysicalPartition(*subjectCPtr);
    auto alignmentsCPtr = DesignAlignments::Query(*physicalModelPtr, "Design Alignments");
    auto alignModelPtr = alignmentsCPtr->GetAlignmentModel();

    // Create Alignment
    auto alignmentPtr = Alignment::Create(*alignModelPtr);
    alignmentPtr->getP()->SetCode(RoadRailAlignmentDomain::CreateCode(*alignModelPtr, "ALG-1"));
    alignmentPtr->SetStartStation(1000);
    ASSERT_TRUE(alignmentPtr->Insert().IsValid());

    ASSERT_EQ(1, AlignmentModelUtilities::QueryAlignmentIds(*alignModelPtr).size());

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
    auto verticalAlignmPtr = VerticalAlignment::Create(*alignmentPtr, *vertAlignVecPtr);
    ASSERT_TRUE(verticalAlignmPtr->InsertAsMainVertical().IsValid());

    auto alignmentCPtr = Alignment::Get(*projectPtr, alignmentPtr->GetElementId());
    ASSERT_EQ(horizAlignmPtr->GetElementId(), alignmentCPtr->GetHorizontal()->GetElementId());
    ASSERT_EQ(verticalAlignmPtr->GetElementId(), alignmentCPtr->GetMainVertical()->GetElementId());

    auto verticalIds = alignmentCPtr->QueryVerticalAlignmentIds();
    ASSERT_EQ(1, verticalIds.size());
    ASSERT_EQ(verticalAlignmPtr->GetElementId(), *verticalIds.begin());

    // Get AlignmentPair
    auto alignmentPairPtr = alignmentCPtr->QueryMainPair();
    ASSERT_TRUE(alignmentPairPtr != nullptr);
    ASSERT_DOUBLE_EQ(150.0, alignmentPairPtr->LengthXY());
    ASSERT_TRUE(alignmentPairPtr->IsValidVertical());

    // ISpatialLinearElement
    DPoint3d point = alignmentCPtr->ToDPoint3d(DistanceExpression(75.0, 10.0, 5.0));
    ASSERT_DOUBLE_EQ(75.0, point.x);
    ASSERT_DOUBLE_EQ(-10.0, point.y);
    ASSERT_DOUBLE_EQ(5.0, point.z);

    DistanceExpression distanceExp = alignmentCPtr->ToDistanceExpression(point);
    ASSERT_DOUBLE_EQ(75.0, distanceExp.GetDistanceAlongFromStart());
    ASSERT_DOUBLE_EQ(10.0, distanceExp.GetLateralOffsetFromILinearElement().Value());
    ASSERT_DOUBLE_EQ(5.0, distanceExp.GetVerticalOffsetFromILinearElement().Value());

    // AlignmentStations
    // DistanceAlong    0       30      70      120     150
    // Station          1000    10      10000   100     130

    auto stationPtr = AlignmentStation::Create(AlignmentStation::CreateAtParams(*alignmentCPtr, 30, 10));
    auto stationCPtr = stationPtr->Insert();
    ASSERT_TRUE(stationCPtr.IsValid());
    ASSERT_DOUBLE_EQ(30.0, stationCPtr->GetAtDistanceAlongFromStart());
    ASSERT_DOUBLE_EQ(10.0, stationCPtr->GetStation());

    stationPtr = AlignmentStation::Create(AlignmentStation::CreateAtParams(*alignmentCPtr, 70, 10000));
    ASSERT_TRUE(stationPtr->Insert().IsValid());

    stationPtr = AlignmentStation::Create(AlignmentStation::CreateAtParams(*alignmentCPtr, 120, 100));
    ASSERT_TRUE(stationPtr->Insert().IsValid());

    auto stationTranslatorPtr = AlignmentStationingTranslator::Create(*alignmentCPtr);
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
    auto koqCP = projectPtr->Schemas().GetKindOfQuantity("RoadRailUnits", "STATION");
    auto presentationFormats = koqCP->GetPresentationFormats();

    auto mUnitP = projectPtr->Schemas().GetUnit("Units", "M");
    Quantity qty(stationTranslatorPtr->ToStation(71).Value(), *mUnitP);
    auto formatP = std::find_if(presentationFormats.begin(), presentationFormats.end(), 
        [](NamedFormat& format) { return format.HasCompositeMajorUnit() && 0 == format.GetCompositeMajorUnit()->GetName().CompareTo("M"); });
    ASSERT_TRUE(formatP != presentationFormats.end());
    ASSERT_TRUE(formatP->GetCompositeMajorUnit() != nullptr);

    Utf8String str = formatP->FormatQuantity(qty, nullptr);
    ASSERT_STRCASEEQ("10+001.00", str.c_str());
    
    qty = Quantity(stationTranslatorPtr->ToStation(71).Value(), *mUnitP);
    formatP = std::find_if(presentationFormats.begin(), presentationFormats.end(), 
        [](NamedFormat& format) { return format.HasCompositeMajorUnit() && 0 == format.GetCompositeMajorUnit()->GetName().CompareTo("FT"); });
    ASSERT_TRUE(formatP != presentationFormats.end());
    ASSERT_TRUE(formatP->GetCompositeMajorUnit() != nullptr);

    str = formatP->FormatQuantity(qty, nullptr);
    ASSERT_STRCASEEQ("328+11.68", str.c_str());

    // Delete-cascade
    /*ASSERT_EQ(DgnDbStatus::Success, alignmentPtr->Delete());
    
    ASSERT_TRUE(projectPtr->Elements().GetElement(alignmentPtr->GetElementId()).IsNull());
    ASSERT_TRUE(projectPtr->Elements().GetElement(horizAlignmPtr->GetElementId()).IsNull());
    ASSERT_TRUE(projectPtr->Elements().GetElement(verticalAlignmPtr->GetElementId()).IsNull());
    ASSERT_TRUE(projectPtr->Models().GetModel(verticalModelPtr->GetModelId()).IsNull());*/
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailAlignmentTests, AlignmentPairEditorTest)
    {
    DgnDbPtr projectPtr = CreateProject(L"AlignmentPairEditorTest.bim");
    ASSERT_TRUE(projectPtr.IsValid());

    auto subjectCPtr = projectPtr->Elements().GetRootSubject();
    auto physicalModelPtr = SetUpPhysicalPartition(*subjectCPtr);
    auto alignmentsCPtr = DesignAlignments::Query(*physicalModelPtr, "Design Alignments");
    auto alignModelPtr = alignmentsCPtr->GetAlignmentModel();

    // Create Horizontal 
    DPoint2d pntsHoriz2d[]{ { 0, 0 },{ 50, 0 },{ 100, 0 },{ 150, 0 } };
    CurveVectorPtr horizAlignVecPtr = CurveVector::CreateLinear(pntsHoriz2d, 4);

    // Create Vertical
    DPoint2d pntsVert2d[]{ { 0, 0 },{ 150, 0 } };
    CurveVectorPtr vertAlignVecPtr = CurveVector::CreateLinear(pntsVert2d, 2);

    auto alignPairPtr = AlignmentPairEditor::Create(horizAlignVecPtr.get(), vertAlignVecPtr.get());

    // Create Alignment
    auto alignmentPtr = Alignment::Create(*alignModelPtr);
    alignmentPtr->getP()->SetCode(RoadRailAlignmentDomain::CreateCode(*alignModelPtr, "ALG-1"));
    ASSERT_TRUE(alignmentPtr->InsertWithMainPair(*alignPairPtr).IsValid());

    ASSERT_TRUE(alignmentPtr->GetHorizontal()->GetElementId().IsValid());
    ASSERT_TRUE(alignmentPtr->GetMainVertical()->GetElementId().IsValid());

    auto verticalIds = alignmentPtr->QueryVerticalAlignmentIds();
    ASSERT_EQ(1, verticalIds.size());

    auto horizAlignPtr = HorizontalAlignment::GetForEdit(*projectPtr, alignmentPtr->GetHorizontal()->GetElementId());
    ASSERT_EQ(DgnDbStatus::Success, horizAlignPtr->GenerateElementGeom());
    ASSERT_TRUE(horizAlignPtr->Update().IsValid());

    // Get AlignmentPair
    auto alignmentPairPtr = alignmentPtr->QueryMainPair();
    ASSERT_TRUE(alignmentPairPtr != nullptr);
    ASSERT_DOUBLE_EQ(150.0, alignmentPairPtr->LengthXY());
    ASSERT_TRUE(alignmentPairPtr->IsValidVertical());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailAlignmentTests, AlignmentSegmentationTest)
    {
    DgnDbPtr projectPtr = CreateProject(L"AlignmentSegmentationTest.bim");
    ASSERT_TRUE(projectPtr.IsValid());

    auto subjectCPtr = projectPtr->Elements().GetRootSubject();
    auto physicalModelPtr = SetUpPhysicalPartition(*subjectCPtr);
    auto alignmentsCPtr = DesignAlignments::Query(*physicalModelPtr, "Design Alignments");
    auto alignModelPtr = alignmentsCPtr->GetAlignmentModel();

    // Create Alignment
    auto alignmentPtr = Alignment::Create(*alignModelPtr);
    alignmentPtr->getP()->SetCode(RoadRailAlignmentDomain::CreateCode(*alignModelPtr, "ALG-1"));
    ASSERT_TRUE(alignmentPtr->Insert().IsValid());

    // Create Stations
    auto station1Ptr = AlignmentStation::Create(AlignmentStation::CreateAtParams(*alignmentPtr, 50.0, 100.0));
    ASSERT_TRUE(station1Ptr->Insert().IsValid());
    ASSERT_DOUBLE_EQ(50.0, station1Ptr->GetAtDistanceAlongFromStart());
    ASSERT_DOUBLE_EQ(100.0, station1Ptr->GetStation());
    
    auto station2Ptr = AlignmentStation::Create(AlignmentStation::CreateAtParams(*alignmentPtr, 100.0, 200.0));
    ASSERT_TRUE(station2Ptr->Insert().IsValid());

    // Segmentation
    bset<DgnClassId> classIds;
    classIds.insert(AlignmentStation::QueryClassId(*projectPtr));

    auto locations = alignmentPtr->QueryLinearLocations(ILinearElement::QueryParams(classIds));
    ASSERT_EQ(2, locations.size());

    locations = alignmentPtr->QueryLinearLocations(ILinearElement::QueryParams(classIds, 75.0, 125.0));
    ASSERT_EQ(1, locations.size());

    locations = alignmentPtr->QueryLinearLocations(ILinearElement::QueryParams(classIds, NullableDouble(), 75.0));
    ASSERT_EQ(1, locations.size());

    locations = alignmentPtr->QueryLinearLocations(ILinearElement::QueryParams(classIds, 125.0, NullableDouble()));
    ASSERT_EQ(0, locations.size());

    locations = alignmentPtr->QueryLinearLocations(ILinearElement::QueryParams(50.0, 100.0));
    ASSERT_EQ(2, locations.size());

    locations = alignmentPtr->QueryLinearLocations(ILinearElement::QueryParams(50.0, ILinearElement::QueryParams::ComparisonOption::Exclusive, 100.0, ILinearElement::QueryParams::ComparisonOption::Inclusive));
    ASSERT_EQ(1, locations.size());

    locations = alignmentPtr->QueryLinearLocations(ILinearElement::QueryParams(50.0, ILinearElement::QueryParams::ComparisonOption::Exclusive, 100.0, ILinearElement::QueryParams::ComparisonOption::Exclusive));
    ASSERT_EQ(0, locations.size());
    }