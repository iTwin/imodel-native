#pragma once
struct GeometryCreation
    {
	#pragma region Static Default Geometries
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Get Default Horizontal Alignment Geometry
	//---------------------------------------------------------------------------------------
	static CurveVectorPtr GetDefaultStraightHorizontalAlignmentGeometry() 
		{
		CurveVectorPtr horizAlignVecPtr;
		DPoint2d pntsHoriz2d[]{ { 0, 0 },{ 50, 0 },{ 100, 0 },{ 150, 0 } };
		horizAlignVecPtr = CurveVector::CreateLinear(pntsHoriz2d, 4);
		return horizAlignVecPtr;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Get Default Arched Horizontal Alignment Geometry
	//---------------------------------------------------------------------------------------
	static CurveVectorPtr GetDefaultArchedHorizontalAlignmentGeometry()
		{
		CurveVectorPtr horizAlignVecPtr;
		DPoint2d line1Points2d[]{ { 0, 0 },{ 0,  50 } };
		auto baseCurveVector = CurveVector::CreateLinear(line1Points2d, 2);
		auto arcEllipse =
			DEllipse3d::FromArcCenterStartEnd(DPoint3d::From(-50, 50), DPoint3d::From(0, 50), DPoint3d::From(-50, 100));
		auto arcPrimitive = ICurvePrimitive::CreateArc(arcEllipse);
		auto arcEllipseCurveVector = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, arcPrimitive);
		baseCurveVector->AddPrimitives(*arcEllipseCurveVector);

		DPoint2d line2Points2d[]{ { -50, 100 },{ -100,  100 } };
		auto secondLineCurveVector = CurveVector::CreateLinear(line2Points2d, 2);
		baseCurveVector->AddPrimitives(*secondLineCurveVector);

		horizAlignVecPtr = baseCurveVector;
		return horizAlignVecPtr;
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Get Default Vertical Alignment Geometry
	//---------------------------------------------------------------------------------------
	static CurveVectorPtr GetDefualtStraightVerticalAlignmentGeometry()
		{
		DPoint3d pntsVert3d[]{ { 0, 0, 0 },{ 150, 0, 0 } };
		CurveVectorPtr vertAlignVecPtr = CurveVector::CreateLinear(pntsVert3d, 2);
		return vertAlignVecPtr;
		}
	
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Get Default Pier Cap Profiles (Curve Vectors)
	//---------------------------------------------------------------------------------------
	static bvector<CurveVectorPtr> GetDefaultPierCapProfiles()
		{		
		bvector<CurveVectorPtr> pierCapProfiles;
		bvector<DPoint3d> bottomPoints;
		bottomPoints.push_back(DPoint3d::From(6, 3, -1));
		bottomPoints.push_back(DPoint3d::From(-6, 3, -1));
		bottomPoints.push_back(DPoint3d::From(-6, -3, -1));
		bottomPoints.push_back(DPoint3d::From(6, -3, -1));
		bottomPoints.push_back(DPoint3d::From(6, 3, -1));

		bvector<DPoint3d> topPoints;
		topPoints.push_back(DPoint3d::From(9, 3, 1));
		topPoints.push_back(DPoint3d::From(-9, 3, 1));
		topPoints.push_back(DPoint3d::From(-9, -3, 1));
		topPoints.push_back(DPoint3d::From(9, -3, 1));
		topPoints.push_back(DPoint3d::From(9, 3, 1));
		auto bottomProfile = CurveVector::CreateLinear(bottomPoints, CurveVector::BOUNDARY_TYPE_Outer);
		auto topProfile = CurveVector::CreateLinear(topPoints, CurveVector::BOUNDARY_TYPE_Outer);
		pierCapProfiles.push_back(bottomProfile);
		pierCapProfiles.push_back(topProfile);
		return pierCapProfiles;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Get Curved Pier Column Profiles (Curve Vectors)
	//---------------------------------------------------------------------------------------
	static bvector<CurveVectorPtr> GetDefaultCurvedPierColumnProfiles()
		{
		bvector<CurveVectorPtr> colProfiles;
		colProfiles.push_back(GetRectangleProfile(14, 4, 0));
		colProfiles.push_back(GetRectangleProfile(13.6, 4, -.064));
		colProfiles.push_back(GetRectangleProfile(13.2, 4, -.512));
		colProfiles.push_back(GetRectangleProfile(12.8, 4, -1.728));
		colProfiles.push_back(GetRectangleProfile(12.4, 4, -4.096));
		colProfiles.push_back(GetRectangleProfile(12, 4, -8));
		colProfiles.push_back(GetRectangleProfile(11.6, 4, -13.824));
		colProfiles.push_back(GetRectangleProfile(11.2, 4, -21.952));
		colProfiles.push_back(GetRectangleProfile(10.8, 4, -32.768));
		colProfiles.push_back(GetRectangleProfile(10.4, 4, -46.656));
		colProfiles.push_back(GetRectangleProfile(10, 4, -64));
		return colProfiles;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	//---------------------------------------------------------------------------------------
	static CurveVectorPtr GetRectangleProfile(double length, double width, double z)
		{
		bvector<DPoint3d> rectPoints;
		rectPoints.push_back(DPoint3d::From(length / 2, width/2, z));
		rectPoints.push_back(DPoint3d::From(length / 2, -width/2, z));
		rectPoints.push_back(DPoint3d::From(-length / 2, -width/2, z));
		rectPoints.push_back(DPoint3d::From(-length / 2, width/2, z));
		auto retVal = CurveVector::CreateLinear(rectPoints, CurveVector::BOUNDARY_TYPE_Outer);
		return retVal;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	// Get Default GeomParams
	//---------------------------------------------------------------------------------------
	static Dgn::Render::GeometryParams GetDefaultGeomParams(Dgn::DgnCategoryId categoryId, ColorDef colorDef = ColorDef::Red())
		{
		Dgn::Render::GeometryParams params;
		params.SetCategoryId(categoryId);
		params.SetLineColor(colorDef);
		params.SetWeight(2);
		return params;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Get Default Girder Base Profile Points
	// @note Assumes base profile lives in the xz-plane and has its aligned point at the origin (0,0)
	//---------------------------------------------------------------------------------------
	static bvector<DPoint2d> GetGirderBaseProfilePoints()
		{
		bvector<DPoint2d> points;
		points.push_back(DPoint2d::From(0, 0));
		points.push_back(DPoint2d::From(1, 0));
		points.push_back(DPoint2d::From(1, -.1));
		points.push_back(DPoint2d::From(.33, -.2));
		points.push_back(DPoint2d::From(.11, -.3));
		points.push_back(DPoint2d::From(.11, -.6));
		points.push_back(DPoint2d::From(.33, -1));
		points.push_back(DPoint2d::From(.33, -1.1));
		points.push_back(DPoint2d::From(-.33, -1.1));
		points.push_back(DPoint2d::From(-.33, -1));
		points.push_back(DPoint2d::From(-.11, -.6));
		points.push_back(DPoint2d::From(-.11, -.3));
		points.push_back(DPoint2d::From(-.33, -.2));
		points.push_back(DPoint2d::From(-1, -.1));
		points.push_back(DPoint2d::From(-1, 0));
		points.push_back(DPoint2d::From(0, 0));
		return points;
		}
	

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Get Default Girder-Deck Profile Points
	// @note Assumes base profile lives in the xz-plane and has its aligned point at the origin (0,0)
	//---------------------------------------------------------------------------------------
	static bvector<DPoint2d> GetGirderDeckBaseProfilePoints()
		{
		bvector<DPoint2d> points;
		points.push_back(DPoint2d::From(0, 0));
		points.push_back(DPoint2d::From(10, -.2));
		points.push_back(DPoint2d::From(10, -.4));
		points.push_back(DPoint2d::From(8, -.4));
		points.push_back(DPoint2d::From(8, -.36));
		points.push_back(DPoint2d::From(5.5, -.31));
		points.push_back(DPoint2d::From(5.5, -.4));
		points.push_back(DPoint2d::From(3.5, -.4));
		points.push_back(DPoint2d::From(3.5, -.27));
		points.push_back(DPoint2d::From(1, -.22));
		points.push_back(DPoint2d::From(1, -.4));
		points.push_back(DPoint2d::From(-1, -.4));
		points.push_back(DPoint2d::From(-1, -.22));
		points.push_back(DPoint2d::From(-3.5, -.27));
		points.push_back(DPoint2d::From(-3.5, -.4));
		points.push_back(DPoint2d::From(-5.5, -.4));
		points.push_back(DPoint2d::From(-5.5, -.31));
		points.push_back(DPoint2d::From(-8, -.36));
		points.push_back(DPoint2d::From(-8, -.4));
		points.push_back(DPoint2d::From(-10, -.4));
		points.push_back(DPoint2d::From(-10, -.2));
		points.push_back(DPoint2d::From(0, 0));
		return points;
		}
	
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Get Default Box Girder Base Profile Points
	// @note Assumes base profile lives in the xz-plane and has its aligned point at the origin (0,0)
	//---------------------------------------------------------------------------------------
	static bvector<DPoint2d> GetBoxGirderBaseProfilePoints()
		{
		bvector<DPoint2d> points;
		points.push_back(DPoint2d::From(0, 0));
		points.push_back(DPoint2d::From(16, 0));
		points.push_back(DPoint2d::From(16, -.33));
		points.push_back(DPoint2d::From(9, -.66));
		points.push_back(DPoint2d::From(9, -1));
		points.push_back(DPoint2d::From(7, -5));
		points.push_back(DPoint2d::From(-7, -5));
		points.push_back(DPoint2d::From(-9, -1));
		points.push_back(DPoint2d::From(-9, -.66));
		points.push_back(DPoint2d::From(-16, -.33));
		points.push_back(DPoint2d::From(-16, 0));
		points.push_back(DPoint2d::From(0, 0));
		return points;
		}
	
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Get Default Box Girder Hole Profile Points
	// @note Assumes base profile lives in the xz-plane and has its aligned point at the origin (0,0)
	//---------------------------------------------------------------------------------------
	static bvector<DPoint2d> GetBoxGirderHoleBaseProfilePoints()
	{
		bvector<DPoint2d> points;
		points.push_back(DPoint2d::From(0, -.33));
		points.push_back(DPoint2d::From(8, -.66));
		points.push_back(DPoint2d::From(6, -4.66));
		points.push_back(DPoint2d::From(-6, -4.66));
		points.push_back(DPoint2d::From(-8, -.66));
		points.push_back(DPoint2d::From(0, -.33));
		return points;
	}
	#pragma endregion

	#pragma region Geometry Creation
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Generate Generate start and end Profiles for constant profile/profiles along spatial 
	// linear element lofted extrusion
	//---------------------------------------------------------------------------------------
	static bvector<CurveVectorPtr> GenerateStartAndEndProfilesForConstantProfilesAlongSLE(bvector<DPoint2d> baseProfilePoints,
		ILinearElementCP iLinearElementCP, const bvector<DPoint2d>& endBaseProfilePoints = bvector<DPoint2d>(),
		NullableDouble startDist = nullptr, NullableDouble endDist = nullptr,
		double lateralOffset = 0.0, double verticalOffset = 0.0)
		{
		bvector<CurveVectorPtr> profiles;
		auto spatialLinearElementCP = dynamic_cast<LinearReferencing::ISpatialLinearElementCP>(iLinearElementCP);
		auto length = endDist == nullptr ? spatialLinearElementCP->GetLength() : endDist.Value();

		auto distanceExpression = DistanceExpression(startDist.Value());
		auto startProfile = Get3dProfileAlongSpatialLinearElement(
			spatialLinearElementCP, baseProfilePoints, distanceExpression,
			lateralOffset, verticalOffset);
		bvector<DPoint3d> endProfile;
		if (endBaseProfilePoints.size() == 0)
			{
			// Use first profile points
			endProfile= Get3dProfileAlongSpatialLinearElement(
				spatialLinearElementCP, baseProfilePoints, distanceExpression,
				lateralOffset, verticalOffset);
			}
		else 
			{
			// Use second profile points
			endProfile = Get3dProfileAlongSpatialLinearElement(
				spatialLinearElementCP, endBaseProfilePoints, distanceExpression,
				lateralOffset, verticalOffset);
			}
		profiles.push_back(CurveVector::CreateLinear(startProfile, CurveVector::BOUNDARY_TYPE_Outer));
		profiles.push_back(CurveVector::CreateLinear(endProfile, CurveVector::BOUNDARY_TYPE_Outer));
		return profiles;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Generate Guide Curves for constant profile along spatial linear element lofted extrusion
	//---------------------------------------------------------------------------------------
	static bvector<CurveVectorPtr> GenerateGuideCurvesAlongILinearElementForConstantProfile(
		bvector<DPoint2d> baseProfilePoints, ILinearElementCP iLinearElementCP,
		NullableDouble startDist = nullptr, NullableDouble endDist = nullptr,
		double lateralOffset = 0.0, double verticalOffset = 0.0)
		{
		bvector<CurveVectorPtr> guideCurves;
		bvector<DPoint3d>* guidePointsArray = new bvector<DPoint3d>[baseProfilePoints.size()];

		auto placementPt = DPoint3d::From(0, 0, 0);
		auto profiles = GenerateProfilesAlongILinearElement(baseProfilePoints, iLinearElementCP,
			placementPt, startDist, endDist,
			lateralOffset, verticalOffset);
		for (auto const& profile: profiles)
			{
			int i = 0;
			for (auto const& point : profile)
				{
				guidePointsArray[i].push_back(point);
				i++;
				}
			}
		for (int i = 0; i < baseProfilePoints.size(); i++)
			{
			MSInterpolationCurve fitCurve;
			DPoint3d tangents[2];
			tangents[0] = DPoint3d::From(0, 0, 0);
			tangents[1] = DPoint3d::From(0, 0, 0);
			fitCurve.InitFromPointsAndEndTangents(guidePointsArray[i], false, 0.0, tangents, false, 
				false, false, false);
			auto curvePrim = ICurvePrimitive::CreateInterpolationCurveSwapFromSource(fitCurve);
			guideCurves.push_back(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curvePrim));
			}
		delete[] guidePointsArray;
		guidePointsArray = nullptr;

		return guideCurves;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Generate Guide Curves for constant profile along spatial linear element lofted extrusion
	//---------------------------------------------------------------------------------------
	static void AddPointsAlongAlignmentToGuideCurvePointsArray(ISpatialLinearElementCP spatialLinearElementCP,
		bvector<DPoint2d> baseProfilePoints, DistanceExpression distanceExpression,
		double lateralOffset, double verticalOffset, bvector<DPoint3d> guidePointsArray[])
		{
		int i = 0;
		for (auto const& baseProfilePoint : baseProfilePoints)
			{
			distanceExpression.SetLateralOffsetFromILinearElement(lateralOffset + baseProfilePoint.x);
			distanceExpression.SetVerticalOffsetFromILinearElement(verticalOffset + baseProfilePoint.y);
			auto newPoint = spatialLinearElementCP->ToDPoint3d(distanceExpression);
			guidePointsArray[i].push_back(newPoint);
			i++;
			}
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	// basic method used to generate profiles along spatial linear elements
	// @note Will not work if change in curvature of alignment is too large
	// A more dynamic approach should be implemented when it is necessary
	// @note Assumes base base profiles live in the xy-plane and has its aligned point at the origin (0,0)
	//---------------------------------------------------------------------------------------
	static bvector<CurveVectorPtr> GenerateProfileVectorsAlongILinearElement(bvector<DPoint2d> baseProfilePoints, ILinearElementCP iLinearElementCP,
		DPoint3dR positionPt, NullableDouble startDist = nullptr, NullableDouble endDist = nullptr,
		double lateralOffset = 0.0, double verticalOffset = 0.0)
		{
		bvector<CurveVectorPtr> profileVectors;
		auto profiles = GenerateProfilesAlongILinearElement(baseProfilePoints, iLinearElementCP,
			positionPt, startDist, endDist, lateralOffset, verticalOffset);
		for (int i = 0; i < profiles.size(); i ++)
			{
			profileVectors.push_back(CurveVector::CreateLinear(profiles.at(i)));
			}
		return profileVectors;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	// basic method used to generate profiles along spatial linear elements
	// @note Will not work if change in curvature of alignment is too large
	// A more dynamic approach should be implemented when it is necessary
	// @note Assumes base base profiles live in the xy-plane and has its aligned point at the origin (0,0)
	//---------------------------------------------------------------------------------------
	static bvector<bvector<DPoint3d>> GenerateProfilesAlongILinearElement(bvector<DPoint2d> baseProfilePoints, ILinearElementCP iLinearElementCP,
		DPoint3dR positionPt, NullableDouble startDist = nullptr, NullableDouble endDist = nullptr,
		double lateralOffset = 0.0, double verticalOffset = 0.0)
		{
		bvector<bvector<DPoint3d>> profiles;
		auto spatialLinearElementCP = dynamic_cast<LinearReferencing::ISpatialLinearElementCP>(iLinearElementCP);
		auto length = endDist == nullptr ? spatialLinearElementCP->GetLength() : endDist.Value();
		auto strokeLength = 1;
		if (startDist == nullptr)
			startDist = 0.0;
		auto distanceExpression = DistanceExpression(startDist.Value());
		positionPt = spatialLinearElementCP->ToDPoint3d(distanceExpression);
		double stationDistance = startDist.Value();
		while (stationDistance < length)
			{
			distanceExpression.SetDistanceAlongFromStart(stationDistance);
			auto nextProfile = Get3dProfileAlongSpatialLinearElement(
				spatialLinearElementCP, baseProfilePoints, distanceExpression,
				lateralOffset, verticalOffset);
			profiles.push_back(nextProfile);
			stationDistance += strokeLength;
			}
		if (stationDistance >= length)
			{
			distanceExpression.SetDistanceAlongFromStart(length);
			auto nextProfile = Get3dProfileAlongSpatialLinearElement(
				spatialLinearElementCP, baseProfilePoints, distanceExpression,
				lateralOffset, verticalOffset);
			profiles.push_back(nextProfile);
			}
		return profiles;
		}
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Get 3D Profile Instance along ISpatialLinearElement
	// @note Assumes base profile lives in the xz-plane and has its aligned point at the origin (0,0)
	//---------------------------------------------------------------------------------------
	static bvector<DPoint3d> Get3dProfileAlongSpatialLinearElement(ISpatialLinearElementCP spatialLinearElementCP,
		bvector<DPoint2d> baseProfilePoints, DistanceExpression distanceExpression,
		double lateralOffset, double verticalOffset)
		{

		bvector<DPoint3d> profilePoints;
		for (auto const& baseProfilePoint : baseProfilePoints)
			{
			distanceExpression.SetLateralOffsetFromILinearElement(lateralOffset + baseProfilePoint.x);
			distanceExpression.SetVerticalOffsetFromILinearElement(verticalOffset + baseProfilePoint.y);
			auto newPoint = spatialLinearElementCP->ToDPoint3d(distanceExpression);
			profilePoints.push_back(newPoint);
			}

		return profilePoints;
		}
	
	
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Create Cylindrical Solid Primitive
	//---------------------------------------------------------------------------------------
	static ISolidPrimitivePtr CreateCylinderPrimitive(DPoint3d center, DVec3d normal, double radius, DVec3d extrusionVector)
		{
		auto ellipse = DEllipse3d::FromCenterNormalRadius(center, normal, radius);
		CurveVectorPtr circleProfile = CurveVector::CreateDisk(ellipse);
		DgnExtrusionDetail dgnExtrusionDetail(circleProfile, extrusionVector, true);
		ISolidPrimitivePtr cylinderGeom = ISolidPrimitive::CreateDgnExtrusion(dgnExtrusionDetail);
		return cylinderGeom;
		}
	
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Create Rectangle Solid Primitive
	//---------------------------------------------------------------------------------------
	static ISolidPrimitivePtr CreateRectanglePrimitive(DPoint3d center, double width, double base, double height)
		{
		DgnBoxDetail boxDetail = DgnBoxDetail::InitFromCenterAndSize(center, DPoint3d::From(width, base, height), true);
		return ISolidPrimitive::CreateDgnBox(boxDetail);
		}
	#pragma endregion

	#pragma region Element Placement and Geometry Setters
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  07/2018
	// Set Placement and Primitive Geometry of a GeometricElement3d
	//---------------------------------------------------------------------------------------
	static DgnDbStatus SetPlacementAndPrimitiveGeometry(Dgn::GeometricElement3dR geomElem,
		Placement3d placement, ISolidPrimitivePtr primitiveGeom,
		TransformCP secondTransform = nullptr)
		{
		auto positionTransform = placement.GetTransform();
		primitiveGeom->TransformInPlace(positionTransform);
		if (secondTransform != nullptr)
			primitiveGeom->TransformInPlace(*secondTransform);
		auto categoryId = geomElem.GetCategoryId();
		auto geomSrc = geomElem.ToGeometrySourceP();
		Dgn::GeometryBuilderPtr geomBuilderPtr = Dgn::GeometryBuilder::Create(*geomSrc);
		geomSrc->SetCategoryId(categoryId);
		geomBuilderPtr->Append(categoryId, GeometryBuilder::CoordSystem::World);
		geomBuilderPtr->Append(GeometryCreation::GetDefaultGeomParams(categoryId));
		if (!geomBuilderPtr->Append(*primitiveGeom, GeometryBuilder::CoordSystem::World))
			return DgnDbStatus::NoGeometry;
		if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(*geomSrc))
			return DgnDbStatus::NoGeometry;
		geomElem.Update();
		return DgnDbStatus::Success;
		}
	
	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Nick.Purcell                  06/2018
	// Set Placement and BRep Geometry of a GeometricElement3d
	//---------------------------------------------------------------------------------------
	static void BuildGeomForGeomElementWithBRep(GeometricElement3dPtr elementPtr, IBRepEntityPtr brep,
		Placement3d placement, bool transformToPlacement = false)
		{
		// Place member
		elementPtr->SetPlacement(placement);
		if (transformToPlacement)
			brep->ApplyTransform(placement.GetTransform());
		auto categoryId = elementPtr->GetCategoryId();
		GeometrySourceP geomSrc = elementPtr->ToGeometrySourceP();
		// Create Geometry Builder
		Dgn::GeometryBuilderPtr geomBuilderPtr = Dgn::GeometryBuilder::Create(*geomSrc);
		geomSrc->SetCategoryId(categoryId);
		geomBuilderPtr->Append(categoryId, GeometryBuilder::CoordSystem::World);
		geomBuilderPtr->Append(GeometryCreation::GetDefaultGeomParams(categoryId), GeometryBuilder::CoordSystem::World);
		geomBuilderPtr->Append(*brep, GeometryBuilder::CoordSystem::World);
		geomBuilderPtr->Finish(*geomSrc);
		elementPtr->Update();
		}
	#pragma endregion
	};