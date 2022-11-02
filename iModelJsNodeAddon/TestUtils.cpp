/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"

BEGIN_UNNAMED_NAMESPACE

static Json::Value lookAtVolume(DgnDbR db, Utf8String params)
	{
	auto props = Json::Value::From(params);
	auto viewProps = props["view"];
	auto idJsonVal = viewProps[DgnElement::json_id()];
	DgnElementId vId(BeInt64Id::FromString(idJsonVal.asCString()).GetValue());
	auto viewOrig = db.Elements().Get<SpatialViewDefinition>(vId);
	auto view = viewOrig->MakeCopy<SpatialViewDefinition>();
	view->FromJson(viewProps);

	auto marginProp = props["margin"];
	ViewDefinition::MarginPercent margin(marginProp["left"].asDouble(), marginProp["top"].asDouble(), marginProp["right"].asDouble(), marginProp["bottom"].asDouble());
	const double aspect = props["aspectRatio"].asDouble();

	view->LookAtVolume(JsonUtils::ToDRange3d(props["volume"]), &aspect, &margin);
	Json::Value val;
	view->ToJson(val);
	return val;
	}

static Json::Value lookAtUsingLensAngle(DgnDbR db, Utf8String params)
	{
	auto props = Json::Value::From(params);
	auto viewProps = props["view"];
	auto idJsonVal = viewProps[DgnElement::json_id()];
	DgnElementId vId(BeInt64Id::FromString(idJsonVal.asCString()).GetValue());
	auto viewOrig = db.Elements().Get<SpatialViewDefinition>(vId);
	auto view = viewOrig->MakeCopy<SpatialViewDefinition>();
	view->FromJson(viewProps);

	auto eye = JsonUtils::ToDPoint3d(props["eye"]);
	auto target = JsonUtils::ToDPoint3d(props["target"]);
	auto up = JsonUtils::ToDVec3d(props["up"]);
	auto lens = JsonUtils::ToAngle(props["lens"]);
	double front = props["front"].asDouble();
	double back = props["back"].asDouble();

	view->LookAtUsingLensAngle(eye, target, up, lens, &front, &back);
	Json::Value val;
	view->ToJson(val);
	return val;
	}

	static Json::Value rotateCameraLocal(DgnDbR db, Utf8String params)
	{
	auto props = Json::Value::From(params);
	auto viewProps = props["view"];
	auto idJsonVal = viewProps[DgnElement::json_id()];
	DgnElementId vId(BeInt64Id::FromString(idJsonVal.asCString()).GetValue());
	auto viewOrig = db.Elements().Get<SpatialViewDefinition>(vId);
	auto view = viewOrig->MakeCopy<SpatialViewDefinition>();
	view->FromJson(viewProps);

	auto angle = props["angle"].asDouble();
	auto axis = JsonUtils::ToDVec3d(props["axis"]);
	DPoint3d about;
	DPoint3dP aboutP = nullptr;
	if (props.isMember("about"))
		{
		about = JsonUtils::ToDPoint3d(props["about"]);
		aboutP = &about;
		}

	view->RotateCameraLocal(angle, axis, aboutP);
	Json::Value val;
	view->ToJson(val);
	return val;
	}

	Json::Value deserializeGeometryStream(DgnDbR dbin, Utf8String params)
	{
	Json::Value props(Json::Value::From(params));
	if (!props.isMember("geom") ||
		!props.isMember("bsurfacePts") || !props.isMember("numSurfacePts") || !props["bsurfacePts"].isArray() ||
		!props.isMember("polyPts") || !props.isMember("numPolyPts") || !props["polyPts"].isArray() ||
		!props.isMember("outFileName"))
		return Json::Value();

	// Set up the original geometry to test against de-serialized geometry
	DEllipse3d origCurve;
	origCurve.InitFromVectors(DPoint3d::From(1, 2, 3), DVec3d::From(0, 0, 2), DVec3d::From(0, 3, 0), 0, Angle::TwoPi());

	CurveVectorPtr origCurveVect = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_ParityRegion);
	CurveVectorPtr loop1 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc(DEllipse3d::FromScaledRotMatrix(DPoint3d::From(-5, 0, 0), RotMatrix::FromRowValues(2, 0, 0, 0, 2, 0, 0, 0, 1), 1, 1, 0, Angle::TwoPi())));
	CurveVectorPtr loop2 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc(DEllipse3d::FromScaledRotMatrix(DPoint3d::From(-5, 0, 0), RotMatrix::FromRowValues(1, 0, 0, 0, 1, 0, 0, 0, 1), 1, 1, 0, Angle::TwoPi())));
	origCurveVect->Add(loop1);
	origCurveVect->Add(loop2);

	ISolidPrimitivePtr origSolid = ISolidPrimitive::CreateDgnCone(DgnConeDetail(DPoint3d::From(0, 0.34, 0), DPoint3d::From(0, 0, 1030.0), DVec3d::From(-1, 0, 0), DVec3d::From(-0, -0.9999999455179609, -0.00033009706939427836), 1.5, 1.5, true));

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
	for (int i = 1; i < numPolyPts - 1; i++)
	{
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
	BeFileName dbName(props["outFileName"].asCString(), true);
	CreateDgnDbParams dgndbParams("DeserializeGeometryStream");
	DgnDbPtr db = DgnDb::CreateDgnDb(&status, dbName, dgndbParams);
	GeometryCollection collection(arrayBuff, *db);

	// Iterate through the buffer making comparisons
	for (auto iter : collection)
	{
		GeometricPrimitivePtr geom = iter.GetGeometryPtr();
		if (!geom.IsValid())
			return Json::Value();

		GeometricPrimitive::GeometryType geomType = geom->GetGeometryType();
		switch ((int)geomType)
		{
		case 1: // CurvePrimitive
		{
			ICurvePrimitivePtr curve = geom->GetAsICurvePrimitive();
			if (!curve->IsSameStructureAndGeometry(*ICurvePrimitive::CreateArc(origCurve)))
				return Json::Value();
			break;
		}
		case 2: // CurveVector
		{
			CurveVectorPtr curveVect = geom->GetAsCurveVector();
			if (!curveVect->IsSameStructureAndGeometry(*origCurveVect))
				return Json::Value();
			break;
		}
		case 3: // SolidPrimitive
		{
			ISolidPrimitivePtr solid = geom->GetAsISolidPrimitive();
			if (!solid->IsSameStructureAndGeometry(*origSolid))
				return Json::Value();
			break;
		}
		case 4: // BsplineSurface
		{
			MSBsplineSurfacePtr surface = geom->GetAsMSBsplineSurface();
			if (!surface->IsSameStructureAndGeometry(*origSurface, 0))
				return Json::Value();
			break;
		}
		case 5: // Polyface
		{
			PolyfaceHeaderPtr polyface = geom->GetAsPolyfaceHeader();
			DPoint3dCP nativePoints = origPolyface->GetPointCP();
			DPoint3dCP jsPoints = polyface->GetPointCP();
			for (size_t i = 0; i < numPolyPts; i++)
				if (!nativePoints[i].IsEqual(jsPoints[i]))
					return Json::Value();
			size_t numIndexes = origPolyface->GetPointIndexCount();
			int32_t const *nativeIndexes = origPolyface->GetPointIndexCP();
			int32_t const *jsIndexes = polyface->GetPointIndexCP();
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

	static Json::Value buildKnownGeometryStream(DgnDbR dbin, Utf8String params)
	{
	Json::Value props(Json::Value::From(params));
	if (!props.isMember("bsurfacePts") || !props.isMember("numSurfacePts") || !props["bsurfacePts"].isArray() ||
		!props.isMember("polyPts") || !props.isMember("numPolyPts") || !props["polyPts"].isArray() ||
		!props.isMember("outFileName"))
		return Json::Value();

	// Set up the geometry to insert into the geometry stream
	DEllipse3d origCurve;
	origCurve.InitFromVectors(DPoint3d::From(1, 2, 3), DVec3d::From(0, 0, 2), DVec3d::From(0, 3, 0), 0, Angle::TwoPi());
	CurveVectorPtr origCurveVect = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_ParityRegion);
	CurveVectorPtr loop1 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc(DEllipse3d::FromScaledRotMatrix(DPoint3d::From(-5, 0, 0), RotMatrix::FromRowValues(2, 0, 0, 0, 2, 0, 0, 0, 1), 1, 1, 0, Angle::TwoPi())));
	CurveVectorPtr loop2 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc(DEllipse3d::FromScaledRotMatrix(DPoint3d::From(-5, 0, 0), RotMatrix::FromRowValues(1, 0, 0, 0, 1, 0, 0, 0, 1), 1, 1, 0, Angle::TwoPi())));
	origCurveVect->Add(loop1);
	origCurveVect->Add(loop2);
	ISolidPrimitivePtr origSolid = ISolidPrimitive::CreateDgnCone(DgnConeDetail(DPoint3d::From(0, 0.34, 0), DPoint3d::From(0, 0, 1030.0), DVec3d::From(-1, 0, 0), DVec3d::From(-0, -0.9999999455179609, -0.00033009706939427836), 1.5, 1.5, true));
	bvector<DPoint3d> pointArr;
	int numSurfacePts = props["numSurfacePts"].asInt();
	Json::Value surfacePts = props["bsurfacePts"];
	for (int i = 0; i < numSurfacePts; i++)
	{
		pointArr.push_back(DPoint3d::From(surfacePts[i][0].asInt(), surfacePts[i][1].asInt(), surfacePts[i][2].asInt()));
	}
	MSBsplineSurfacePtr origSurface = MSBsplineSurface::CreatePtr();
	origSurface->InitFromPointsAndOrder(3, 4, 4, 6, &pointArr[0]);

	int numPolyPts = props["numPolyPts"].asInt();
	Json::Value polyPts = props["polyPts"];
	PolyfaceHeaderPtr origPolyface = PolyfaceHeader::CreateVariableSizeIndexed();
	for (int i = 0; i < numPolyPts; i++)
		origPolyface->Point().push_back(DPoint3d::From(polyPts[i][0].asDouble(), polyPts[i][1].asDouble(), polyPts[i][2].asDouble()));
	for (int i = 1; i < numPolyPts - 1; i++)
	{
		origPolyface->PointIndex().push_back(i);
		origPolyface->PointIndex().push_back(i + 1);
		origPolyface->PointIndex().push_back(i + 2);
		origPolyface->PointIndex().push_back(0);
	}

	// Set up the GeometryBuilder
	BeSQLite::DbResult status;
	BeFileName dbName(props["outFileName"].asCString(), true);
	CreateDgnDbParams dgndbParams("BuildKnownGeometryStream");
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

END_UNNAMED_NAMESPACE

Json::Value IModelJsNative::JsInterop::ExecuteTest(DgnDbR db, Utf8StringCR testName, Utf8StringCR params)
    {
	if (testName.Equals("lookAtVolume")) return lookAtVolume(db, params);
	if (testName.Equals("lookAtUsingLensAngle")) return lookAtUsingLensAngle(db, params);
	if (testName.Equals("rotateCameraLocal")) return rotateCameraLocal(db, params);
	if (testName.Equals("buildKnownGeometryStream")) return buildKnownGeometryStream(db, params);
	if (testName.Equals("deserializeGeometryStream")) return deserializeGeometryStream(db, params);
	return Json::Value();
    }
