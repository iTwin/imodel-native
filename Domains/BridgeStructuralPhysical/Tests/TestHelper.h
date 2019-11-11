#pragma once
#include <BridgeStructuralPhysical/BridgeStructuralPhysicalApi.h>
#include <BridgeStructuralPhysical/BridgeStructuralPhysicalDomain.h>
#include <BridgeStructuralPhysical/BridgeStructuralPhysicalDomainUtilities.h>
#include <BridgeStructuralPhysical/Bridge.h>
#include <BridgeStructuralPhysical/BridgeCategory.h>
#include <BridgeStructuralPhysical/BridgeSubstructure.h>
#include <BridgeStructuralPhysical/BridgeSuperstructure.h>
//#include <ProfilesDomain/ProfilesDomainApi.h>
//#include <ProfilesDomain/Profile.h>
#include "GeometryCreation.h"
#include <StructuralDomain/StructuralPhysicalApi.h>

#include <DgnPlatform/DgnCoreAPI.h>
//---------------------------------------------------------------------------------------
// @bsiclass Static Methods to be re-used amongs Tests          Nick.Purcell    06/2018
//---------------------------------------------------------------------------------------
struct TestHelper
	{
	#pragma region Static Helper Methods
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	//---------------------------------------------------------------------------------------
	static DgnDbStatus SetDgnElementCodeAndInsert(DgnElementPtr element, Dgn::DgnCode code)
		{
		DgnDbStatus status = element->SetCode(code);
		if (status != DgnDbStatus::Success)
			return status;
		element->Insert(&status);
		return status;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Get Profile Cardinal Points
	// @note Assumes base profile lives in the xz-plane and has its aligned point at the origin (0,0)
	//---------------------------------------------------------------------------------------
	//static bvector<DPoint2d> GetProfilesCardinalPoints(Profiles::ProfilePtr profilePtr)
	//	{
	//	bvector<DPoint2d> points;
	//	uint32_t index(0);
	//	profilePtr->GetPropertyIndex(index, "CardinalPoints");
	//	ECN::ECValue value;

	//	auto prop = profilePtr->GetPropertyValue(value, index);
	//	if (!value.IsArray())
	//		return points;

	//	auto arrayInfo = value.GetArrayInfo();
	//	auto count = arrayInfo.GetCount();

	//	for (uint32_t i = 0; i < count; i++)
	//		{
	//		Dgn::PropertyArrayIndex propIndex(i);
	//		ECN::ECValue structValue;
	//		profilePtr->GetPropertyValue(structValue, index, propIndex);
	//		auto structInstance = structValue.GetStruct();
	//		ECN::ECValue coordValue;
	//		structInstance->GetValue(coordValue, "Coordinates");
	//		if (!coordValue.IsPoint2d())
	//			continue;

	//		auto pt = coordValue.GetPoint2d();
	//		points.push_back(pt);

	//		}
	//	return points;
	//}
	#pragma endregion

	#pragma region Test View Definitions
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	//---------------------------------------------------------------------------------------
	static Dgn::CategorySelectorPtr CreateCategorySelector(Dgn::DefinitionModelR definitionModel, Utf8StringCR name = "Default")
		{
		// A CategorySelector is a definition element that is normally shared by many ViewDefinitions.
		// To start off, we'll create a default selector that includes the one category that we use.
		// We have to give the selector a unique name of its own. Since we are set up up a new bim,
		// we know that we can safely choose any name.
		auto categorySelector = new Dgn::CategorySelector(definitionModel, name);
		return categorySelector;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	//---------------------------------------------------------------------------------------
	static Dgn::ModelSelectorPtr CreateModelSelector(Dgn::DefinitionModelR definitionModel, Dgn::SpatialModelCR modelToSelect,
		Utf8StringCR name = "Default")
		{
		// A ModelSelector is a definition element that is normally shared by many ViewDefinitions.
		// To start off, we'll create a default selector that includes the one model that we use.
		// We have to give the selector a unique name of its own. Since we are set up up a new
		// bim, we know that we can safely choose any name.
		auto modelSelector = new Dgn::ModelSelector(definitionModel, name);
		modelSelector->AddModel(modelToSelect.GetModelId());
		return modelSelector;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	//---------------------------------------------------------------------------------------
	static Dgn::ModelSelectorPtr CreateModelSelector(Dgn::DefinitionModelR definitionModel, bvector<DgnModelId> modelIds,
		Utf8StringCR name = "Default")
		{
		// A ModelSelector is a definition element that is normally shared by many ViewDefinitions.
		// To start off, we'll create a default selector that includes the one model that we use.
		// We have to give the selector a unique name of its own. Since we are set up up a new
		// bim, we know that we can safely choose any name.
		auto modelSelector = new Dgn::ModelSelector(definitionModel, name);
		for (auto const& id : modelIds)
			{
			modelSelector->AddModel(id);
			}
		return modelSelector;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	//---------------------------------------------------------------------------------------
	static void CreateAndVerifyViewDefinitionForBridgeModel(DgnDbPtr db,
		PhysicalModelCPtr bridgeStructuralPhysicalModelCPtr)
		{
		// Create and Verify view definition
		Dgn::DefinitionModelR dictionary = db->GetDictionaryModel();
		Dgn::CategorySelectorPtr categorySelector = CreateCategorySelector(dictionary);
		Dgn::ModelSelectorPtr modelSelector = CreateModelSelector(dictionary,
			*bridgeStructuralPhysicalModelCPtr);
		Dgn::DisplayStyle3dPtr displayStyle = CreateDisplayStyle3d(dictionary);
		DgnViewId bridgeViewId = CreateView(dictionary, "BridgeView", *categorySelector,
			*modelSelector, *displayStyle);
		ASSERT_TRUE(bridgeViewId.IsValid());
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	//---------------------------------------------------------------------------------------
	static void CreateAndVerifyViewDefinitionForAlignmentModel(DgnDbPtr db, AlignmentModelPtr alignModelPtr)
		{
		// Create and Verify view definition
		Dgn::DefinitionModelR dictionary = db->GetDictionaryModel();
		Dgn::CategorySelectorPtr categorySelector = CreateCategorySelector(dictionary);
		Dgn::ModelSelectorPtr modelSelector = CreateModelSelector(dictionary, *alignModelPtr);
		Dgn::DisplayStyle3dPtr displayStyle = CreateDisplayStyle3d(dictionary);
		DgnViewId alignmentViewId = CreateView(dictionary, "AlignmentView", *categorySelector,
			*modelSelector, *displayStyle);
		ASSERT_TRUE(alignmentViewId.IsValid());
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	//---------------------------------------------------------------------------------------
	static void CreateAndVerifyViewDefinitionsForBridgeAndAlignmentModels(DgnDbPtr db,
		PhysicalModelCPtr bridgeStructuralPhysicalModelCPtr, AlignmentModelPtr alignModelPtr)
		{
		// Create and Verify view definitions
		Dgn::DefinitionModelR dictionary = db->GetDictionaryModel();
		Dgn::CategorySelectorPtr categorySelector = CreateCategorySelector(dictionary);
		bvector<DgnModelId> modelIds;
		modelIds.push_back(bridgeStructuralPhysicalModelCPtr->GetModelId());
		modelIds.push_back(alignModelPtr->GetModelId());
		Dgn::ModelSelectorPtr modelSelector = CreateModelSelector(dictionary, modelIds);
		Dgn::DisplayStyle3dPtr displayStyle = CreateDisplayStyle3d(dictionary);
		DgnViewId bridgeViewId = CreateView(dictionary, "BridgeView", *categorySelector,
			*modelSelector, *displayStyle);
		DgnViewId alignmentViewId = CreateView(dictionary, "AlignmentView", *categorySelector,
			*modelSelector, *displayStyle);
		ASSERT_TRUE(bridgeViewId.IsValid());
		ASSERT_TRUE(alignmentViewId.IsValid());
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	//---------------------------------------------------------------------------------------
	static Dgn::DisplayStyle3dPtr CreateDisplayStyle3d(Dgn::DefinitionModelR definitionModel, Utf8StringCR name = "Default")
		{
		// DisplayStyle is a definition element that is potentially shared by many ViewDefinitions.
		// To start off, we'll create a style that can be used as a good default for 3D views.
		// We have to give the style a unique name of its own. Since we are settup up a new bim, we know that we can safely choose any name.
		auto displayStyle = new Dgn::DisplayStyle3d(definitionModel, name);
		displayStyle->SetBackgroundColor(Dgn::ColorDef::White());
		displayStyle->SetSkyBoxEnabled(false);
		displayStyle->SetGroundPlaneEnabled(false);
		Dgn::Render::ViewFlags viewFlags = displayStyle->GetViewFlags();
		viewFlags.SetRenderMode(Dgn::Render::RenderMode::SmoothShade);
		displayStyle->SetViewFlags(viewFlags);
		return displayStyle;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	//---------------------------------------------------------------------------------------
	static Dgn::DgnViewId CreateView(Dgn::DefinitionModelR model, Utf8CP name, Dgn::CategorySelectorR categorySelector, Dgn::ModelSelectorR modelSelector, Dgn::DisplayStyle3dR displayStyle)
		{
		// CategorySelector, ModelSelector, and DisplayStyle are definition elements that are normally shared by many ViewDefinitions.
		// That is why they are inputs to this function.
		Dgn::OrthographicViewDefinition view(model, name, categorySelector, displayStyle, modelSelector);

		// Define the view direction and volume.
		Dgn::DgnDbR db = model.GetDgnDb();
		view.SetStandardViewRotation(Dgn::StandardView::Iso); // Default to a rotated view
		view.LookAtVolume(db.GeoLocation().GetProjectExtents()); // A good default for a new view is to "fit" it to the contents of the bim.

																 // Write the ViewDefinition to the bim
		if (!view.Insert().IsValid())
			{
			return Dgn::DgnViewId();
			}

		// Set the DefaultView property of the bim
		Dgn::DgnViewId viewId = view.GetViewId();
		db.SaveProperty(Dgn::DgnViewProperty::DefaultView(), &viewId, (uint32_t) sizeof(viewId));
		return viewId;
		}
	#pragma endregion

	#pragma region Static Default Factory methods
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	//---------------------------------------------------------------------------------------
	static AlignmentModelPtr GetDefaultAlignmentModel(DgnDbPtr dbPtr)
		{
		auto configModelPtr = ConfigurationModel::Query(*dbPtr->Elements().GetRootSubject());
		if (!configModelPtr.IsValid())
			return nullptr;
		auto subjectCPtr = configModelPtr->GetParentSubject();
		if (!subjectCPtr.IsValid())
			return nullptr;
		if (dbPtr->Elements().GetRootSubjectId() != subjectCPtr->GetElementId())
			return nullptr;
		auto alignmentModelPtr = AlignmentModel::Query(*subjectCPtr, RoadRailAlignmentDomain::GetDesignPartitionName());
		return alignmentModelPtr;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	//---------------------------------------------------------------------------------------
	static AlignmentPtr GetDefaultAlignment(DgnDbPtr dbPtr, AlignmentModelPtr alignModelPtr, Utf8StringCR code = "ALG-1", bool loadWithArcs = false)
		{
		// Create Alignment
		auto alignmentPtr = Alignment::Create(*alignModelPtr);
		DgnDbStatus status = alignmentPtr->SetCode(RoadRailAlignmentDomain::CreateCode(*alignModelPtr, code));
		if (status != DgnDbStatus::Success)
			return nullptr;
		alignmentPtr->Insert(&status);
		if (status != DgnDbStatus::Success)
			return nullptr;
		auto horizAlignmPtr = TestHelper::GetDefaultHorizontalAlignment(alignmentPtr, loadWithArcs);
		status = horizAlignmPtr->GenerateElementGeom();
		if (status != DgnDbStatus::Success)
			return nullptr;
		if (!horizAlignmPtr->Insert().IsValid())
			return nullptr;
		if (status != DgnDbStatus::Success)
			return nullptr;
		auto verticalModelPtr = VerticalAlignmentModel::Create(VerticalAlignmentModel::CreateParams(*dbPtr, alignmentPtr->GetElementId()));
		status = verticalModelPtr->Insert();
		if (status != DgnDbStatus::Success)
			return nullptr;
		const auto verticalAlignmPtr = TestHelper::GetDefaultVerticalAlignment(verticalModelPtr);
		status =  verticalAlignmPtr->GenerateElementGeom();
		if (status != DgnDbStatus::Success)
			return nullptr;

		if (!verticalAlignmPtr->InsertAsMainVertical().IsValid())
			return nullptr;
		return alignmentPtr;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	//---------------------------------------------------------------------------------------
	static HorizontalAlignmentPtr GetDefaultHorizontalAlignment(AlignmentPtr alignmentPtr, bool loadWithArcs = false)
		{
		// Create Horizontal Alignment
		CurveVectorPtr horizAlignVecPtr = nullptr;
		if (loadWithArcs)		
			horizAlignVecPtr = GeometryCreation::GetDefaultArchedHorizontalAlignmentGeometry();
		else
			horizAlignVecPtr = GeometryCreation::GetDefaultStraightHorizontalAlignmentGeometry();

		auto horizAlignmPtr = HorizontalAlignment::Create(*alignmentPtr, *horizAlignVecPtr);
		return horizAlignmPtr;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	//---------------------------------------------------------------------------------------
	static VerticalAlignmentPtr GetDefaultVerticalAlignment(VerticalAlignmentModelPtr verticalModelPtr)
		{
		auto vertAlignVecPtr = GeometryCreation::GetDefualtStraightVerticalAlignmentGeometry();
		auto verticalAlignmPtr = VerticalAlignment::Create(*verticalModelPtr, *vertAlignVecPtr);
		return verticalAlignmPtr;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	//---------------------------------------------------------------------------------------
	static GenericSubstructureElementPtr CreateAndPositionMultiColumnPier(
		GeometricElement3d::CreateParams createParams,
		ILinearlyLocatedSingleAt::CreateAtParams createAtParams,
		PhysicalModelCPtr modelCPtr,
		Utf8StringCR codeName,
		double skew = 0)
		{
		auto substructureElementPtr = GenericSubstructureElement::Create(createParams,
			createAtParams, skew);
		auto code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *modelCPtr,
			codeName);
		Dgn::DgnDbStatus status = SetDgnElementCodeAndInsert(substructureElementPtr, code);
		if (status != DgnDbStatus::Success)
			return nullptr;
		status = CreateAndPositionMultiColumnPierMembers(substructureElementPtr, modelCPtr);
		if (status != DgnDbStatus::Success)
			return nullptr;
		return substructureElementPtr;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Create And Position Multi-Column Pier Substructure Structural Members
	//---------------------------------------------------------------------------------------
	static Dgn::DgnDbStatus CreateAndPositionMultiColumnPierMembers(
		GenericSubstructureElementPtr substructureElementPtr, PhysicalModelCPtr modelCPtr)
		{
		// Basic method for creating structural members for a basic multi-column pier (3 cols, 1 cap, 1 footing)
		// TODO: refactor this huge method
		Dgn::DgnDbStatus status;
		auto substructCode = substructureElementPtr->GetCode().GetValueUtf8();
		auto skew = substructureElementPtr->GetSkew();
		auto relClassId = substructureElementPtr->GetDgnDb().Schemas().GetClassId(
			BBP_SCHEMA_NAME, BBP_REL_SubstructureElementAssemblesStructuralMembers);
		Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY,
			*modelCPtr, substructCode + "SupportCol1");
		auto column1Ptr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
			STRUCTURAL_PHYSICAL_CLASS_Column, modelCPtr, nullptr, substructureElementPtr->GetElementId(), relClassId);

		status = SetDgnElementCodeAndInsert(column1Ptr, code);
		if (status != DgnDbStatus::Success)
			return status;

		code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *modelCPtr,
			substructCode + "SupportCol2");
		auto column2Ptr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
			STRUCTURAL_PHYSICAL_CLASS_Column, modelCPtr, nullptr, substructureElementPtr->GetElementId(), relClassId);

		status = SetDgnElementCodeAndInsert(column2Ptr, code);
		if (status != DgnDbStatus::Success)
			return status;

		code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *modelCPtr,
			substructCode + "SupportCol3");
		auto column3Ptr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
			STRUCTURAL_PHYSICAL_CLASS_Column, modelCPtr, nullptr, substructureElementPtr->GetElementId(), relClassId);

		status = SetDgnElementCodeAndInsert(column3Ptr, code);
		if (status != DgnDbStatus::Success)
			return status;

		auto distExpr = substructureElementPtr->GetSingleLinearlyReferencedAtLocation()->GetAtPosition();
		auto leCP = substructureElementPtr->GetLinearElement();
		auto spatialLinearElementCP = dynamic_cast<LinearReferencing::ISpatialLinearElementCP>(leCP);
		
		distExpr.SetVerticalOffsetFromILinearElement(-13.5);
		auto placementPt = spatialLinearElementCP->ToDPoint3d(distExpr);
		Placement3d placement;
		placement.GetOriginR() = placementPt;

		distExpr.SetLateralOffsetFromILinearElement(5.0);
		auto placementPt2 = spatialLinearElementCP->ToDPoint3d(distExpr);
		Placement3d placement2;
		placement2.GetOriginR() = placementPt2;

		auto rotationalVec = DVec3d::FromStartEnd(placementPt2, placementPt);
		auto yaw = -rotationalVec.AngleToXY(DVec3d::UnitX());

		distExpr.SetLateralOffsetFromILinearElement(-5.0);
		auto placementPt3 = spatialLinearElementCP->ToDPoint3d(distExpr);
		Placement3d placement3;
		placement3.GetOriginR() = placementPt3;

		auto cylinderPrimitive = GeometryCreation::CreateCylinderPrimitive(DPoint3d::From(0, 0, 0), DVec3d::From(0, 0, 1), 1, DVec3d::From(0, 0, 10));
		auto placementAndUnitZ = DPoint3d::From(placementPt.x, placementPt.y, placementPt.z + 1);

		auto skewTransform = Transform::FromLineAndRotationAngle(placementPt, placementAndUnitZ, skew);
		auto cylinderPrimitive2 = cylinderPrimitive->Clone();
		auto cylinderPrimitive3 = cylinderPrimitive->Clone();

		status = GeometryCreation::SetPlacementAndPrimitiveGeometry(*column2Ptr, placement, cylinderPrimitive, &skewTransform);
		if (status != DgnDbStatus::Success)
			return status;
		status = GeometryCreation::SetPlacementAndPrimitiveGeometry(*column1Ptr, placement2, cylinderPrimitive2, &skewTransform);
		if (status != DgnDbStatus::Success)
			return status;
		status = GeometryCreation::SetPlacementAndPrimitiveGeometry(*column3Ptr, placement3, cylinderPrimitive3, &skewTransform);
		if (status != DgnDbStatus::Success)
			return status;
		// TODO: Add Beam Seats?
		// TODO: Add Piles to footing?
		code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *modelCPtr,
			substructCode + "StripFooting1");
		auto stripFootingPtr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
			STRUCTURAL_PHYSICAL_CLASS_StripFooting, modelCPtr, nullptr, substructureElementPtr->GetElementId(), relClassId);

		status = SetDgnElementCodeAndInsert(stripFootingPtr, code);
		if (status != DgnDbStatus::Success)
			return status;
		distExpr.SetLateralOffsetFromILinearElement(0.0);
		distExpr.SetVerticalOffsetFromILinearElement(-13);
		placementPt = spatialLinearElementCP->ToDPoint3d(distExpr);
		placement.GetOriginR() = placementPt;
		auto footerPrimitive = GeometryCreation::CreateRectanglePrimitive(DPoint3d::From(0, 0, 0), 16, 6, 1);
		placement.GetAnglesR() = YawPitchRollAngles::FromRadians(yaw + skew, 0, 0);
		status = GeometryCreation::SetPlacementAndPrimitiveGeometry(*stripFootingPtr, placement, footerPrimitive);
		if (status != DgnDbStatus::Success)
			return status;

		auto pierCapPtr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
			STRUCTURAL_PHYSICAL_CLASS_Beam, modelCPtr, nullptr, substructureElementPtr->GetElementId(), relClassId);

		code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *modelCPtr,
			substructCode + "PierCap1");
		status = SetDgnElementCodeAndInsert(pierCapPtr, code);
		if (status != DgnDbStatus::Success)
			return status;

		bvector<CurveVectorPtr> pierCapProfiles = GeometryCreation::GetDefaultPierCapProfiles();
		IBRepEntityPtr pierCapBrep;
		BRepUtil::Create::BodyFromLoft(pierCapBrep, pierCapProfiles, nullptr, false);
		if (pierCapBrep == nullptr)
			return Dgn::DgnDbStatus::NoGeometry;
		distExpr.SetLateralOffsetFromILinearElement(0.0);
		distExpr.SetVerticalOffsetFromILinearElement(-2.5);
		placementPt = spatialLinearElementCP->ToDPoint3d(distExpr);
		placement.GetOriginR() = placementPt;
		GeometryCreation::BuildGeomForGeomElementWithBRep(pierCapPtr, pierCapBrep, placement, true);
		if (!pierCapPtr->HasGeometry())
			return DgnDbStatus::NoGeometry;
		
		return status;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	//---------------------------------------------------------------------------------------
	static GenericSubstructureElementPtr CreateAndPositionSingleColumnPierSubstructure(
		GeometricElement3d::CreateParams createParams,
		ILinearlyLocatedSingleAt::CreateAtParams createAtParams,
		PhysicalModelCPtr modelCPtr,
		Utf8StringCR codeName,
		double skew = 0)
		{
		auto substructureElementPtr = GenericSubstructureElement::Create(createParams,
			createAtParams, skew);
		auto code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY, *modelCPtr,
			codeName);
		Dgn::DgnDbStatus status = SetDgnElementCodeAndInsert(substructureElementPtr, code);
		if (status != DgnDbStatus::Success)
			return nullptr;
		status = CreateAndPositionSingleColumnPierSubstructureMembers(substructureElementPtr, modelCPtr);
		if (status != DgnDbStatus::Success)
			return nullptr;
		return substructureElementPtr;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Create And Position Single column Pier Substructure Structural Members
	//---------------------------------------------------------------------------------------
	static DgnDbStatus CreateAndPositionSingleColumnPierSubstructureMembers(
		GenericSubstructureElementPtr substructureElementPtr, PhysicalModelCPtr modelCPtr)
		{
		// Basic method for creating structural members for a basic multi-column pier (3 cols, 1 cap, 1 footing)
		Dgn::DgnDbStatus status;
		auto substructCode = substructureElementPtr->GetCode().GetValueUtf8();
		auto skew = substructureElementPtr->GetSkew();
		auto relClassId = substructureElementPtr->GetDgnDb().Schemas().GetClassId(
			BBP_SCHEMA_NAME, BBP_REL_SubstructureElementAssemblesStructuralMembers);
		Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY,
			*modelCPtr, substructCode + "SupportCol");
		auto columnPtr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
			STRUCTURAL_PHYSICAL_CLASS_Column, modelCPtr, nullptr, substructureElementPtr->GetElementId(), relClassId);

		status = SetDgnElementCodeAndInsert(columnPtr, code);
		if (status != DgnDbStatus::Success)
			return status;
		auto colProfiles = GeometryCreation::GetDefaultCurvedPierColumnProfiles();
		IBRepEntityPtr colBrep;
		BRepUtil::Create::BodyFromLoft(colBrep, colProfiles, nullptr, false);
		if (colBrep == nullptr)
			return Dgn::DgnDbStatus::NoGeometry;

		auto distExpr = substructureElementPtr->GetSingleLinearlyReferencedAtLocation()->GetAtPosition();
		distExpr.SetVerticalOffsetFromILinearElement(-5);
		auto leCP = substructureElementPtr->GetLinearElement();
		auto spatialLinearElementCP = dynamic_cast<LinearReferencing::ISpatialLinearElementCP>(leCP);
		Placement3d placement;
		auto placementPt = spatialLinearElementCP->ToDPoint3d(distExpr);
		placement.GetOriginR() = placementPt;
		distExpr.SetLateralOffsetFromILinearElement(1);
		auto normalPt = spatialLinearElementCP->ToDPoint3d(distExpr);
		auto rotationalVec = DVec3d::FromStartEnd(placementPt, normalPt);
		auto yaw = -rotationalVec.AngleToXY(DVec3d::UnitX());
		placement.GetAnglesR() = YawPitchRollAngles::FromRadians(yaw + skew, 0, 0);
		GeometryCreation::BuildGeomForGeomElementWithBRep(columnPtr, colBrep, placement, true);
		if (!columnPtr->HasGeometry())
			return DgnDbStatus::NoGeometry;
		return status;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Create And Position Slab-Girder Superstructure Structural Members
	//---------------------------------------------------------------------------------------
	static DgnDbStatus CreateAndPositionSlabGirderSuperstructureMembers(
		GenericSuperstructureElementPtr superstructureElementPtr, PhysicalModelCPtr modelCPtr)
		{
		// This method is really long
		Dgn::DgnDbStatus status;
		auto relClassId = superstructureElementPtr->GetDgnDb().Schemas().GetClassId(
			BBP_SCHEMA_NAME, BBP_REL_SuperstructureElementAssemblesStructuralMembers);
		auto slabPtr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
			STRUCTURAL_PHYSICAL_CLASS_Slab, modelCPtr, nullptr, superstructureElementPtr->GetElementId(), relClassId);
		slabPtr->Insert(&status);
		if (Dgn::DgnDbStatus::Success != status)
			return status;

		DPoint3d positionPt = DPoint3d::From(0, 0, 0);
		auto profiles = GeometryCreation::GenerateProfileVectorsAlongILinearElement(
			GeometryCreation::GetGirderDeckBaseProfilePoints(),
			superstructureElementPtr->GetLinearElement(),
			positionPt
		);
		IBRepEntityPtr deckBrep;
		BRepUtil::Create::BodyFromLoft(deckBrep, profiles, nullptr, false);
		if (deckBrep.IsNull())
			return DgnDbStatus::NoGeometry;
		Placement3d placement;
		placement.GetOriginR() = positionPt;
		GeometryCreation::BuildGeomForGeomElementWithBRep(slabPtr, deckBrep, placement);
		if (!slabPtr->HasGeometry())
			return DgnDbStatus::NoGeometry;

		auto beam1Ptr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
			STRUCTURAL_PHYSICAL_CLASS_Beam, modelCPtr, nullptr, superstructureElementPtr->GetElementId(), relClassId);
		beam1Ptr->Insert(&status);
		if (Dgn::DgnDbStatus::Success != status)
			return status;

		auto girder1Profiles = GeometryCreation::GenerateProfileVectorsAlongILinearElement(
			GeometryCreation::GetGirderBaseProfilePoints(),
			superstructureElementPtr->GetLinearElement(),
			positionPt, nullptr, nullptr, 9, -.4
		);
		IBRepEntityPtr girder1Brep;
		BRepUtil::Create::BodyFromLoft(girder1Brep, girder1Profiles, nullptr, false);
		if (girder1Brep.IsNull())
			return DgnDbStatus::NoGeometry;
		GeometryCreation::BuildGeomForGeomElementWithBRep(beam1Ptr, girder1Brep, placement);
		if (!beam1Ptr->HasGeometry())
			return DgnDbStatus::NoGeometry;

		auto beam2Ptr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
			STRUCTURAL_PHYSICAL_CLASS_Beam, modelCPtr, nullptr, superstructureElementPtr->GetElementId(), relClassId);
		beam2Ptr->Insert(&status);
		if (Dgn::DgnDbStatus::Success != status)
			return status;

		auto girder2Profiles = GeometryCreation::GenerateProfileVectorsAlongILinearElement(
			GeometryCreation::GetGirderBaseProfilePoints(),
			superstructureElementPtr->GetLinearElement(),
			positionPt, nullptr, nullptr, 4.5, -.4
		);
		IBRepEntityPtr girder2Brep;
		BRepUtil::Create::BodyFromLoft(girder2Brep, girder2Profiles, nullptr, false);
		if (girder2Brep.IsNull())
			return DgnDbStatus::NoGeometry;
		GeometryCreation::BuildGeomForGeomElementWithBRep(beam2Ptr, girder2Brep, placement);
		if (!beam2Ptr->HasGeometry())
			return DgnDbStatus::NoGeometry;

		auto beam3Ptr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
			STRUCTURAL_PHYSICAL_CLASS_Beam, modelCPtr, nullptr, superstructureElementPtr->GetElementId(), relClassId);
		beam3Ptr->Insert(&status);
		if (Dgn::DgnDbStatus::Success != status)
			return status;

		auto girder3Profiles = GeometryCreation::GenerateProfileVectorsAlongILinearElement(
			GeometryCreation::GetGirderBaseProfilePoints(),
			superstructureElementPtr->GetLinearElement(),
			positionPt, nullptr, nullptr, 0, -.4
		);
		IBRepEntityPtr girder3Brep;
		BRepUtil::Create::BodyFromLoft(girder3Brep, girder3Profiles, nullptr, false);
		if (girder3Brep.IsNull())
			return DgnDbStatus::NoGeometry;
		GeometryCreation::BuildGeomForGeomElementWithBRep(beam3Ptr, girder3Brep, placement);
		if (!beam3Ptr->HasGeometry())
			return DgnDbStatus::NoGeometry;

		auto beam4Ptr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
			STRUCTURAL_PHYSICAL_CLASS_Beam, modelCPtr, nullptr, superstructureElementPtr->GetElementId(), relClassId);
		beam4Ptr->Insert(&status);
		if (Dgn::DgnDbStatus::Success != status)
			return status;

		auto girder4Profiles = GeometryCreation::GenerateProfileVectorsAlongILinearElement(
			GeometryCreation::GetGirderBaseProfilePoints(),
			superstructureElementPtr->GetLinearElement(),
			positionPt, nullptr, nullptr, -4.5, -.4
		);
		IBRepEntityPtr girder4Brep;
		BRepUtil::Create::BodyFromLoft(girder4Brep, girder4Profiles, nullptr, false);
		if (girder4Brep.IsNull())
			return DgnDbStatus::NoGeometry;
		GeometryCreation::BuildGeomForGeomElementWithBRep(beam4Ptr, girder4Brep, placement);
		if (!beam4Ptr->HasGeometry())
			return DgnDbStatus::NoGeometry;

		auto beam5Ptr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
			STRUCTURAL_PHYSICAL_CLASS_Beam, modelCPtr, nullptr, superstructureElementPtr->GetElementId(), relClassId);
		beam5Ptr->Insert(&status);
		if (Dgn::DgnDbStatus::Success != status)
			return status;
		auto girder5Profiles = GeometryCreation::GenerateProfileVectorsAlongILinearElement(
			GeometryCreation::GetGirderBaseProfilePoints(),
			superstructureElementPtr->GetLinearElement(),
			positionPt, nullptr, nullptr, -9, -.4
		);
		IBRepEntityPtr girder5Brep;
		BRepUtil::Create::BodyFromLoft(girder5Brep, girder5Profiles, nullptr, false);
		if (girder5Brep.IsNull())
			return DgnDbStatus::NoGeometry;
		GeometryCreation::BuildGeomForGeomElementWithBRep(beam5Ptr, girder5Brep, placement);
		if (!beam5Ptr->HasGeometry())
			return DgnDbStatus::NoGeometry;
		return status;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Create And Position Box-Girder Superstructure Structural Members
	//---------------------------------------------------------------------------------------
	static DgnDbStatus CreateAndPositionBoxGirderSuperstructureMembers(
		GenericSuperstructureElementPtr superstructureElementPtr, PhysicalModelCPtr modelCPtr)
		{
		Dgn::DgnDbStatus status;
		auto relClassId = superstructureElementPtr->GetDgnDb().Schemas().GetClassId(
			BBP_SCHEMA_NAME, BBP_REL_SuperstructureElementAssemblesStructuralMembers);
		auto boxGirderPtr = BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME,
			STRUCTURAL_PHYSICAL_CLASS_StructuralMember, modelCPtr, nullptr, superstructureElementPtr->GetElementId(), relClassId);
		boxGirderPtr->Insert(&status);
		if (Dgn::DgnDbStatus::Success != status)
			return status;

		DPoint3d positionPt = DPoint3d::From(0, 0, 0);
		auto profiles = GeometryCreation::GenerateProfileVectorsAlongILinearElement(
			GeometryCreation::GetBoxGirderBaseProfilePoints(),
			superstructureElementPtr->GetLinearElement(),
			positionPt
		);
		IBRepEntityPtr boxGirderBrep;
		BentleyStatus bstatus;
		bstatus = BRepUtil::Create::BodyFromLoft(boxGirderBrep, profiles, nullptr, false);
		if (bstatus != BentleyStatus::SUCCESS)
			return DgnDbStatus::NoGeometry;
		auto holeProfiles = GeometryCreation::GenerateProfileVectorsAlongILinearElement(
			GeometryCreation::GetBoxGirderHoleBaseProfilePoints(),
			superstructureElementPtr->GetLinearElement(),
			positionPt
		);

		IBRepEntityPtr holeBrep;
		bstatus = BRepUtil::Create::BodyFromLoft(holeBrep, holeProfiles, nullptr, false);
		if (bstatus != BentleyStatus::SUCCESS)
			return DgnDbStatus::NoGeometry;
		bvector<IBRepEntityPtr> holes;
		holes.push_back(holeBrep);
		bstatus = BRepUtil::Modify::BooleanOperation(boxGirderBrep, holes, BRepUtil::Modify::BooleanMode::Subtract);
		if (bstatus != BentleyStatus::SUCCESS)
			return DgnDbStatus::NoGeometry;

		Placement3d placement;
		placement.GetOriginR() = positionPt;
		GeometryCreation::BuildGeomForGeomElementWithBRep(boxGirderPtr, boxGirderBrep, placement);
		if (!boxGirderPtr->HasGeometry())
			return DgnDbStatus::NoGeometry;
		return status;
		}
	#pragma endregion
	};


