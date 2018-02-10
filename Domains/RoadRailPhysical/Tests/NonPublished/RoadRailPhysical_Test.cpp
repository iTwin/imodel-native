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
    DgnModelId physicalModelId = QueryFirstModelIdOfType(*projectPtr, 
        DgnClassId(projectPtr->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalModel)));
    auto physicalModelPtr = projectPtr->Models().Get<PhysicalModel>(physicalModelId);

    // Create Roadway
    auto roadwayPtr = Roadway::Create(*physicalModelPtr);
    roadwayPtr->SetMainLinearElement(alignmentPtr.get());
    auto roadwayCPtr = roadwayPtr->Insert();
    ASSERT_TRUE(roadwayCPtr.IsValid());    
    ASSERT_EQ(alignmentPtr->GetElementId(), roadwayCPtr->GetMainLinearElementId());

    alignmentPtr->SetILinearElementSource(roadwayPtr.get());
    ASSERT_TRUE(alignmentPtr->Update().IsValid());

    auto linearElements = roadwayPtr->QueryLinearElements();
    ASSERT_EQ(1, linearElements.size());
    ASSERT_EQ(alignmentPtr->GetElementId(), *linearElements.begin());
#pragma endregion
    }