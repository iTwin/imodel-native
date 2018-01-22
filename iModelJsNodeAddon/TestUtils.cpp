/*--------------------------------------------------------------------------------------+
|
|     $Source: TestUtils.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestUtils.h"
#include "AddonUtils.h"

#include <json/value.h>
#include <rapidjson/rapidjson.h>


// ===========================================================================================================
// Exposed test function implementations
// ===========================================================================================================

Json::Value TestUtils::ViewStateCreate(Utf8String params) {
	Json::Value props(Json::Value::From(params));
	if (!props.isMember("path") || !props.isMember("id") || !props.isMember("dsId") || !props.isMember("csId") || !props.isMember("msId"))
		return Json::Value();

	DgnDbPtr db;
	BeFileName path(props["path"].asCString());
	AddonUtils::OpenDgnDb(db, path, Db::OpenMode::ReadWrite);
	if (!db.IsValid())
		return Json::Value();

	// Grab the view element and member-related elements
	DgnElementId vId(props["id"].asUInt64());
	auto oldView = (*db).Elements().Get<Dgn::SpatialViewDefinition>(vId);

	// Grab the original form in json to use for extra properties later
	Json::Value originalJson = (*oldView).ToJson(props["id"]);

	// Grab the view element's other member elements
	DgnElementId dId(props["dsId"].asUInt64());
	auto displayStyle = (*db).Elements().Get<Dgn::DisplayStyle3d>(dId);
	DgnElementId cId(props["csId"].asUInt64());
	auto catSelector = (*db).Elements().Get<Dgn::CategorySelector>(cId);
	DgnElementId mId(props["msId"].asUInt64());
	auto modelSelector = (*db).Elements().Get<Dgn::ModelSelector>(mId);

	if (!oldView.IsValid() || !displayStyle.IsValid() || !catSelector.IsValid() || !modelSelector.IsValid()) {
		return Json::Value();
	}

	// Create the new view
	SpatialViewDefinition::CreateParams newViewParams(
		*db,
		oldView->GetModelId(),
		oldView->GetElementClassId(),
		oldView->GetCode(),
		(CategorySelectorR)*catSelector,
		(DisplayStyle3dR)*displayStyle,
		(ModelSelectorR)*modelSelector,
		&(oldView->GetCamera()));

	newViewParams.m_id = vId;
	newViewParams.m_origin.Init(-87.73958171815832, -108.96514044887601, -0.0853709702222105);
	newViewParams.m_extents.Init(429.6229727570776, 232.24786876266097, 0.1017680889917761);
	SpatialViewDefinition view(newViewParams);
	view.SetJsonProperties("viewDetails", originalJson["jsonProperties"]["viewDetails"]);
	view.TurnCameraOff();

	Json::Value retVal = view.ToJson(props["id"]);
	retVal["id"] = props["id"];		// Id was not being set
	return retVal;
}

Json::Value TestUtils::ViewStateVolumeAdjustments(Utf8String params) {
	Json::Value props(Json::Value::From(params));
	if (!props.isMember("path") || !props.isMember("id") || !props.isMember("dsId") || !props.isMember("csId") || !props.isMember("msId") || !props.isMember("testMode"))
		return Json::Value();

	DgnDbPtr db;
	BeFileName path(props["path"].asCString());
	AddonUtils::OpenDgnDb(db, path, Db::OpenMode::ReadWrite);
	if (!db.IsValid())
		return Json::Value();

	// Grab the view element and member-related elements
	DgnElementId vId(props["id"].asUInt64());
	auto oldView = (*db).Elements().Get<Dgn::SpatialViewDefinition>(vId);

	// Grab the original form in json to use for extra properties later
	Json::Value originalJson = (*oldView).ToJson(props["id"]);

	// Grab the view element's other member elements
	DgnElementId dId(props["dsId"].asUInt64());
	auto displayStyle = (*db).Elements().Get<Dgn::DisplayStyle3d>(dId);
	DgnElementId cId(props["csId"].asUInt64());
	auto catSelector = (*db).Elements().Get<Dgn::CategorySelector>(cId);
	DgnElementId mId(props["msId"].asUInt64());
	auto modelSelector = (*db).Elements().Get<Dgn::ModelSelector>(mId);

	if (!oldView.IsValid() || !displayStyle.IsValid() || !catSelector.IsValid() || !modelSelector.IsValid()) {
		return Json::Value();
	}

	// Create the new view
	SpatialViewDefinition::CreateParams newViewParams(
		*db,
		oldView->GetModelId(),
		oldView->GetElementClassId(),
		oldView->GetCode(),
		(CategorySelectorR)*catSelector,
		(DisplayStyle3dR)*displayStyle,
		(ModelSelectorR)*modelSelector,
		&(oldView->GetCamera()));

	newViewParams.m_id = vId;
	SpatialViewDefinition view(newViewParams);
	view.SetJsonProperties("viewDetails", originalJson["jsonProperties"]["viewDetails"]);
	int testMode = props["testMode"].asUInt64();		// This test will test a variety of cases based on the property "testMode"
	ViewDefinition::MarginPercent margin(1, 1, 2, 2);
	const double aspect0 = 1.8;
	const double aspect1 = 1.2;

	switch (testMode) {
	case 0:		 // Flat view test #1
	{
		view.SetOrigin(DPoint3d::From(-5, -5, 0));
		view.SetExtents(DVec3d::From(10, 10, 1));
		view.SetRotation(RotMatrix::FromRowValues(1, 0, 0, 0, 1, 0, 0, 0, 1));
		view.SetLensAngle(Angle::FromDegrees(50));
		view.SetFocusDistance(49);
		view.SetEyePoint(DPoint3d::From(5, 5, 50));
		view.TurnCameraOff();
		view.LookAtVolume(DRange3d::From(10, 20, 0.5, 35, 21, 2));
		break;
	}
	case 1:		// Flat view test #2
	{
		view.SetOrigin(DPoint3d::From(100, 1000, -2));
		view.SetExtents(DVec3d::From(314, 1, -.00001));
		view.SetRotation(RotMatrix::FromRowValues(1, 4, -1, 2, 3, 6, -9, 4, 3));
		view.SetLensAngle(Angle::FromDegrees(12));
		view.SetFocusDistance(1000);
		view.SetEyePoint(DPoint3d::From(1, 1000, 2));
		view.TurnCameraOff();
		view.LookAtVolume(DRange3d::From(-1000, -10, 6, -5, 0, 0), &aspect0, &margin);
		break;
	}
	case 2:		// Camera view test #1
	{
		view.SetOrigin(DPoint3d::From(5, 5, 5));
		view.SetExtents(DVec3d::From(2, 5, 2));
		view.SetRotation(RotMatrix::FromRowValues(1, 0, 0, 0, 1, 0, 0, 0, 1));
		view.SetLensAngle(Angle::FromDegrees(30));
		view.SetFocusDistance(50);
		view.SetEyePoint(DPoint3d::From(5, 5, 75));
		view.LookAtVolume(DRange3d::From(0.1, 0.1, 0.1, 10, 20, 30));
		break;
	}
	case 3:		// Camera view test #2
	{
		view.SetOrigin(DPoint3d::From(100, 1000, -2));
		view.SetExtents(DVec3d::From(314, 1, -.00001));
		view.SetRotation(YawPitchRollAngles::FromDegrees(25, 25, 0.1).ToRotMatrix());
		view.SetLensAngle(Angle::FromDegrees(108));
		view.SetFocusDistance(89);
		view.SetEyePoint(DPoint3d::From(1, 1000, 2));
		margin.Init(1, 2, 3, 4);
		view.LookAtVolume(DRange3d::From(-1000, -10, 6, -5, 0, 0), &aspect1, &margin);
		break;
	}
	default:
		return Json::Value();
	}

	Json::Value retVal = view.ToJson(props["id"]);
	retVal["id"] = props["id"];		// Id was not being set
	return retVal;
}

Json::Value TestUtils::ViewStateLookAt(Utf8String params) {
	Json::Value props(Json::Value::From(params));
	if (!props.isMember("path") || !props.isMember("id") || !props.isMember("dsId") || !props.isMember("csId") || !props.isMember("msId") || !props.isMember("testMode"))
		return Json::Value();

	DgnDbPtr db;
	BeFileName path(props["path"].asCString());
	AddonUtils::OpenDgnDb(db, path, Db::OpenMode::ReadWrite);
	if (!db.IsValid())
		return Json::Value();

	// Grab the view element and member-related elements
	DgnElementId vId(props["id"].asUInt64());
	auto oldView = (*db).Elements().Get<Dgn::SpatialViewDefinition>(vId);

	// Grab the original form in json to use for extra properties later
	Json::Value originalJson = (*oldView).ToJson(props["id"]);

	// Grab the view element's other member elements
	DgnElementId dId(props["dsId"].asUInt64());
	auto displayStyle = (*db).Elements().Get<Dgn::DisplayStyle3d>(dId);
	DgnElementId cId(props["csId"].asUInt64());
	auto catSelector = (*db).Elements().Get<Dgn::CategorySelector>(cId);
	DgnElementId mId(props["msId"].asUInt64());
	auto modelSelector = (*db).Elements().Get<Dgn::ModelSelector>(mId);

	if (!oldView.IsValid() || !displayStyle.IsValid() || !catSelector.IsValid() || !modelSelector.IsValid()) {
		return Json::Value();
	}

	// Create the new view
	SpatialViewDefinition::CreateParams newViewParams(
		*db,
		oldView->GetModelId(),
		oldView->GetElementClassId(),
		oldView->GetCode(),
		(CategorySelectorR)*catSelector,
		(DisplayStyle3dR)*displayStyle,
		(ModelSelectorR)*modelSelector,
		&(oldView->GetCamera()));

	newViewParams.m_id = vId;
	SpatialViewDefinition view(newViewParams);
	view.SetJsonProperties("viewDetails", originalJson["jsonProperties"]["viewDetails"]);
	int testMode = props["testMode"].asUInt64();		// This test will test a variety of cases based on the property "testMode"

	switch (testMode) {
	case 0:			// Flat view test #1
	{
		view.SetOrigin(DPoint3d::From(-5, -5, 0));
		view.SetExtents(DVec3d::From(10, 10, 1));
		view.SetRotation(RotMatrix::FromRowValues(1, 0, 0, 0, 1, 0, 0, 0, 1));
		view.SetLensAngle(Angle::FromDegrees(50));
		view.SetFocusDistance(49);
		view.SetEyePoint(DPoint3d::From(5, 5, 50));
		view.TurnCameraOff();
		view.RotateCameraLocal(1.28, DVec3d::From(2, 5, 7), nullptr);
		break;
	}
	case 1:			// Camera view test #1
	{
		view.SetOrigin(DPoint3d::From(100, 23, -18));
		view.SetExtents(DVec3d::From(55, 0.01, 23));
		view.SetRotation(YawPitchRollAngles::FromDegrees(23, 65, 2).ToRotMatrix());
		view.SetLensAngle(Angle::FromDegrees(11));
		view.SetFocusDistance(191);
		view.SetEyePoint(DPoint3d::From(-64, 120, 500));
		DPoint3d aroundPoint = DPoint3d::From(1, 2, 3);
		view.RotateCameraLocal(1.6788888, DVec3d::From(-1, 6, 3), &aroundPoint);
		break;
	}
	case 2:			// Camera view test #2 (usingLensAngle)
	{
		view.SetOrigin(DPoint3d::From(100, 23, -18));
		view.SetExtents(DVec3d::From(55, 0.01, 23));
		view.SetRotation(YawPitchRollAngles::FromDegrees(23, 65, 2).ToRotMatrix());
		view.SetLensAngle(Angle::FromDegrees(11));
		view.SetFocusDistance(191);
		view.SetEyePoint(DPoint3d::From(-64, 120, 500));
		const double frontDist = 100.89;
		const double backDist = 101.23;
		view.LookAtUsingLensAngle(DPoint3d::From(8, 6, 7), DPoint3d::From(100, -67, 5), DVec3d::From(1.001, 2.200, -3.999), Angle::FromDegrees(27.897), &frontDist, &backDist);
		break;
	}
	default:
		return Json::Value();
	}

	Json::Value retVal = view.ToJson(props["id"]);
	retVal["id"] = props["id"];		// Id was not being set
	return retVal;
}

Json::Value TestUtils::DeserializeGeometryStream(Utf8String params) {
	Json::Value props(Json::Value::From(params));
	if (!props.isMember("geom") || \
		!props.isMember("bsurfacePts") || !props.isMember("numSurfacePts") || !props["bsurfacePts"].isArray() || \
		!props.isMember("polyPts") || !props.isMember("numPolyPts") || !props["polyPts"].isArray())
		return Json::Value();

	// Set up the original geometry to test against de-serialized geometry
	DEllipse3d origCurve;
	origCurve.InitFromVectors(DPoint3d::From(1, 2, 3), DVec3d::From(0, 0, 2), DVec3d::From(0, 3, 0), 0, Angle::TwoPi());
	
	CurveVectorPtr origCurveVect = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_ParityRegion);
	CurveVectorPtr loop1 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc(DEllipse3d::FromScaledRotMatrix(DPoint3d::From(-5, 0, 0), RotMatrix::FromRowValues(2, 0, 0, 0, 2, 0, 0, 0, 1), 1, 1, 0, Angle::TwoPi())));
	CurveVectorPtr loop2 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc(DEllipse3d::FromScaledRotMatrix(DPoint3d::From(-5, 0, 0), RotMatrix::FromRowValues(1, 0, 0, 0, 1, 0, 0, 0, 1), 1, 1, 0, Angle::TwoPi())));
	origCurveVect->Add(loop1);
	origCurveVect->Add(loop2);
	
	ISolidPrimitivePtr origSolid = ISolidPrimitive::CreateDgnCone(DgnConeDetail::DgnConeDetail(DPoint3d::From(0, 0.34, 0), DPoint3d::From(0, 0, 1030.0), DVec3d::From(-1, 0, 0), DVec3d::From(-0, -0.9999999455179609, -0.00033009706939427836), 1.5, 1.5, true));
	
	bvector<DPoint3d> bSurfacePts;
	int numSurfacePts = props["numSurfacePts"].asInt();
	Json::Value surfacePts = props["bsurfacePts"];
	for (int i = 0; i < numSurfacePts; i++)
		bSurfacePts.push_back(DPoint3d::From(surfacePts[i][0].asDouble(), surfacePts[i][1].asDouble(), surfacePts[i][2].asDouble()));
	MSBsplineSurfacePtr origSurface = MSBsplineSurface::CreatePtr();
	origSurface->InitFromPointsAndOrder(3, 4, 4, 6, &bSurfacePts[0]);
	
	int numPolyPts = props["numPolyPts"].asInt();
	Json::Value polyPts = props["polyPts"];
	PolyfaceHeaderPtr origPolyface = PolyfaceHeader::CreateVariableSizeIndexed();
	for (int i = 0; i < numPolyPts; i++)
		origPolyface->Point().push_back(DPoint3d::From(polyPts[i][0].asDouble(), polyPts[i][1].asDouble(), polyPts[i][2].asDouble()));
	for (int i = 1; i < numPolyPts - 1; i++) {
		origPolyface->PointIndex().push_back(i);
		origPolyface->PointIndex().push_back(i + 1);
		origPolyface->PointIndex().push_back(i + 2);
		origPolyface->PointIndex().push_back(0);
	}


	// Get bytebuffer
	GeometryStream arrayBuff;
	arrayBuff.FromBase64(props["geom"].asString());

	// Set up collection iterator
	BeSQLite::DbResult status;
	BeFileName dbName("myDb");
	CreateDgnDbParams dgndbParams("DeserializeGeometryStream");
	DgnDbPtr db = DgnDb::CreateDgnDb(&status, dbName, dgndbParams);
	GeometryCollection collection(arrayBuff, *db);

	// Iterate through the buffer making comparisons
	for (auto iter : collection) {
		GeometricPrimitivePtr geom = iter.GetGeometryPtr();
		if (!geom.IsValid())
			return Json::Value();

		GeometricPrimitive::GeometryType geomType = geom->GetGeometryType();
		switch ((int)geomType) {
		case 1:		// CurvePrimitive
		{
			ICurvePrimitivePtr curve = geom->GetAsICurvePrimitive();
			if (!curve->IsSameStructureAndGeometry(*ICurvePrimitive::CreateArc(origCurve)))
				return Json::Value();
			break;
		}
		case 2:		// CurveVector
		{
			CurveVectorPtr curveVect = geom->GetAsCurveVector();
			if (!curveVect->IsSameStructureAndGeometry(*origCurveVect))
				return Json::Value();
			break;
		}
		case 3:		// SolidPrimitive
		{
			ISolidPrimitivePtr solid = geom->GetAsISolidPrimitive();
			if (!solid->IsSameStructureAndGeometry(*origSolid))
				return Json::Value();
			break;
		}
		case 4:		// BsplineSurface
		{
			MSBsplineSurfacePtr surface = geom->GetAsMSBsplineSurface();
			if (!surface->IsSameStructureAndGeometry(*origSurface, 0))
				return Json::Value();
			break;
		}
		case 5:		// Polyface
		{
			PolyfaceHeaderPtr polyface = geom->GetAsPolyfaceHeader();
			DPoint3dCP nativePoints = origPolyface->GetPointCP();
			DPoint3dCP jsPoints = polyface->GetPointCP();
			for (size_t i = 0; i < numPolyPts; i++)
				if (!nativePoints[i].IsEqual(jsPoints[i]))
					return Json::Value();
			size_t numIndexes = origPolyface->GetPointIndexCount();
			int32_t const* nativeIndexes = origPolyface->GetPointIndexCP();
			int32_t const* jsIndexes = polyface->GetPointIndexCP();
			for (size_t i = 0; i < numIndexes; i++)
				if (nativeIndexes[i] != jsIndexes[i])
					return Json::Value();
			break;
		}
		default:
		{
			return Json::Value();
		}
		}
	}

	// All geometry de-serialized matched the originals
	Json::Value retVal;
	retVal["returnValue"] = true;
	return retVal;
}

Json::Value TestUtils::BuildKnownGeometryStream(Utf8String params) {
	Json::Value props(Json::Value::From(params));
	if (!props.isMember("bsurfacePts") || !props.isMember("numSurfacePts") || !props["bsurfacePts"].isArray() || \
		!props.isMember("polyPts") || !props.isMember("numPolyPts") || !props["polyPts"].isArray())
		return Json::Value();

	// Set up the geometry to insert into the geometry stream
	DEllipse3d origCurve;
	origCurve.InitFromVectors(DPoint3d::From(1, 2, 3), DVec3d::From(0, 0, 2), DVec3d::From(0, 3, 0), 0, Angle::TwoPi());
	CurveVectorPtr origCurveVect = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_ParityRegion);
	CurveVectorPtr loop1 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc(DEllipse3d::FromScaledRotMatrix(DPoint3d::From(-5, 0, 0), RotMatrix::FromRowValues(2, 0, 0, 0, 2, 0, 0, 0, 1), 1, 1, 0, Angle::TwoPi())));
	CurveVectorPtr loop2 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc(DEllipse3d::FromScaledRotMatrix(DPoint3d::From(-5, 0, 0), RotMatrix::FromRowValues(1, 0, 0, 0, 1, 0, 0, 0, 1), 1, 1, 0, Angle::TwoPi())));
	origCurveVect->Add(loop1);
	origCurveVect->Add(loop2);
	ISolidPrimitivePtr origSolid = ISolidPrimitive::CreateDgnCone(DgnConeDetail::DgnConeDetail(DPoint3d::From(0, 0.34, 0), DPoint3d::From(0, 0, 1030.0), DVec3d::From(-1, 0, 0), DVec3d::From(-0, -0.9999999455179609, -0.00033009706939427836), 1.5, 1.5, true));
	bvector<DPoint3d> pointArr;
	int numSurfacePts = props["numSurfacePts"].asInt();
	Json::Value surfacePts = props["bsurfacePts"];
	for (int i = 0; i < numSurfacePts; i++) {
		pointArr.push_back(DPoint3d::From(surfacePts[i][0].asInt(), surfacePts[i][1].asInt(), surfacePts[i][2].asInt()));
	}
	MSBsplineSurfacePtr origSurface = MSBsplineSurface::CreatePtr();
	origSurface->InitFromPointsAndOrder(3, 4, 4, 6, &pointArr[0]);
	
	int numPolyPts = props["numPolyPts"].asInt();
	Json::Value polyPts = props["polyPts"];
	PolyfaceHeaderPtr origPolyface = PolyfaceHeader::CreateVariableSizeIndexed();
	for (int i = 0; i < numPolyPts; i++)
		origPolyface->Point().push_back(DPoint3d::From(polyPts[i][0].asDouble(), polyPts[i][1].asDouble(), polyPts[i][2].asDouble()));
	for (int i = 1; i < numPolyPts - 1; i++) {
		origPolyface->PointIndex().push_back(i);
		origPolyface->PointIndex().push_back(i + 1);
		origPolyface->PointIndex().push_back(i + 2);
		origPolyface->PointIndex().push_back(0);
	}

	// Set up the GeometryBuilder
	BeSQLite::DbResult status;
	BeFileName dbName("testDb");
	CreateDgnDbParams dgndbParams;
	DgnDbPtr db = DgnDb::CreateDgnDb(&status, dbName, dgndbParams);
	GeometryBuilder builder = *GeometryBuilder::CreateGeometryPart(*db, true);

	// Append the geometry
	builder.Append(*GeometricPrimitive::Create(origCurve));
	builder.Append(*GeometricPrimitive::Create(origCurveVect));
	builder.Append(*GeometricPrimitive::Create(origSolid));
	builder.Append(*GeometricPrimitive::Create(origSurface));
	builder.Append(*GeometricPrimitive::Create(origPolyface));

	// Output the GeometryStream
	GeometryStream gs = GeometryStream();
	builder.GetGeometryStream(gs);
	Json::Value retVal;
	retVal["geom"] = gs.ToBase64();
	return retVal;
}