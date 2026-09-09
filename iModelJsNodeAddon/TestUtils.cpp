/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"

BEGIN_UNNAMED_NAMESPACE

static BeJsDocument lookAtVolume(DgnDbR db, Utf8String params)
	{
	BeJsDocument props(params);
	auto viewProps = props["view"];
	auto idJsonVal = viewProps[DgnElement::json_id()];
	DgnElementId vId(BeInt64Id::FromString(idJsonVal.asCString()).GetValue());
	auto viewOrig = db.Elements().Get<SpatialViewDefinition>(vId);
	auto view = viewOrig->MakeCopy<SpatialViewDefinition>();
	view->FromJson(viewProps);

	auto marginProp = props["margin"];
	ViewDefinition::MarginPercent margin(marginProp["left"].asDouble(), marginProp["top"].asDouble(), marginProp["right"].asDouble(), marginProp["bottom"].asDouble());
	const double aspect = props["aspectRatio"].asDouble();

	view->LookAtVolume(BeJsGeomUtils::ToDRange3d(props["volume"]), &aspect, &margin);
	BeJsDocument val;
	view->ToJson(val);
	return val;
	}

static BeJsDocument lookAtUsingLensAngle(DgnDbR db, Utf8String params)
	{
	BeJsDocument props(params);
	auto viewProps = props["view"];
	auto idJsonVal = viewProps[DgnElement::json_id()];
	DgnElementId vId(BeInt64Id::FromString(idJsonVal.asCString()).GetValue());
	auto viewOrig = db.Elements().Get<SpatialViewDefinition>(vId);
	auto view = viewOrig->MakeCopy<SpatialViewDefinition>();
	view->FromJson(viewProps);

	auto eye = BeJsGeomUtils::ToDPoint3d(props["eye"]);
	auto target = BeJsGeomUtils::ToDPoint3d(props["target"]);
	auto up = BeJsGeomUtils::ToDVec3d(props["up"]);
	auto lens = BeJsGeomUtils::ToAngle(props["lens"]);
	double front = props["front"].asDouble();
	double back = props["back"].asDouble();

	view->LookAtUsingLensAngle(eye, target, up, lens, &front, &back);
	BeJsDocument val;
	view->ToJson(val);
	return val;
	}

	static BeJsDocument rotateCameraLocal(DgnDbR db, Utf8String params)
	{
	BeJsDocument props(params);
	auto viewProps = props["view"];
	auto idJsonVal = viewProps[DgnElement::json_id()];
	DgnElementId vId(BeInt64Id::FromString(idJsonVal.asCString()).GetValue());
	auto viewOrig = db.Elements().Get<SpatialViewDefinition>(vId);
	auto view = viewOrig->MakeCopy<SpatialViewDefinition>();
	view->FromJson(viewProps);

	auto angle = props["angle"].asDouble();
	auto axis = BeJsGeomUtils::ToDVec3d(props["axis"]);
	DPoint3d about;
	DPoint3dP aboutP = nullptr;
	if (props.isMember("about"))
		{
		about = BeJsGeomUtils::ToDPoint3d(props["about"]);
		aboutP = &about;
		}

	view->RotateCameraLocal(angle, axis, aboutP);
	BeJsDocument val;
	view->ToJson(val);
	return val;
	}

	BeJsDocument deserializeGeometryStream(DgnDbR dbin, Utf8String params)
	{
	BeJsDocument props(params);
	if (!props.isMember("geom") ||
		!props.isMember("bsurfacePts") || !props.isMember("numSurfacePts") || !props["bsurfacePts"].isArray() ||
		!props.isMember("polyPts") || !props.isMember("numPolyPts") || !props["polyPts"].isArray() ||
		!props.isMember("outFileName"))
		return BeJsDocument();

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
	auto surfacePts = props["bsurfacePts"];
	for (int i = 0; i < numSurfacePts; i++)
		bSurfacePts.push_back(DPoint3d::From(surfacePts[i][0].asDouble(), surfacePts[i][1].asDouble(), surfacePts[i][2].asDouble()));
	MSBsplineSurfacePtr origSurface = MSBsplineSurface::CreatePtr();
	origSurface->InitFromPointsAndOrder(3, 4, 4, 6, &bSurfacePts[0]);

	int numPolyPts = props["numPolyPts"].asInt();
	auto polyPts = props["polyPts"];
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
	DgnDbPtr db = DgnDb::CreateIModel(&status, dbName, dgndbParams);
	GeometryCollection collection(arrayBuff, *db);

	// Iterate through the buffer making comparisons
	for (auto iter : collection)
	{
		GeometricPrimitivePtr geom = iter.GetGeometryPtr();
		if (!geom.IsValid())
			return BeJsDocument();

		GeometricPrimitive::GeometryType geomType = geom->GetGeometryType();
		switch ((int)geomType)
		{
		case 1: // CurvePrimitive
		{
			ICurvePrimitivePtr curve = geom->GetAsICurvePrimitive();
			if (!curve->IsSameStructureAndGeometry(*ICurvePrimitive::CreateArc(origCurve)))
				return BeJsDocument();
			break;
		}
		case 2: // CurveVector
		{
			CurveVectorPtr curveVect = geom->GetAsCurveVector();
			if (!curveVect->IsSameStructureAndGeometry(*origCurveVect))
				return BeJsDocument();
			break;
		}
		case 3: // SolidPrimitive
		{
			ISolidPrimitivePtr solid = geom->GetAsISolidPrimitive();
			if (!solid->IsSameStructureAndGeometry(*origSolid))
				return BeJsDocument();
			break;
		}
		case 4: // BsplineSurface
		{
			MSBsplineSurfacePtr surface = geom->GetAsMSBsplineSurface();
			if (!surface->IsSameStructureAndGeometry(*origSurface, 0))
				return BeJsDocument();
			break;
		}
		case 5: // Polyface
		{
			PolyfaceHeaderPtr polyface = geom->GetAsPolyfaceHeader();
			DPoint3dCP nativePoints = origPolyface->GetPointCP();
			DPoint3dCP jsPoints = polyface->GetPointCP();
			for (size_t i = 0; i < numPolyPts; i++)
				if (!nativePoints[i].IsEqual(jsPoints[i]))
					return BeJsDocument();
			size_t numIndexes = origPolyface->GetPointIndexCount();
			int32_t const *nativeIndexes = origPolyface->GetPointIndexCP();
			int32_t const *jsIndexes = polyface->GetPointIndexCP();
			for (size_t i = 0; i < numIndexes; i++)
				if (nativeIndexes[i] != jsIndexes[i])
					return BeJsDocument();
			break;
		}
		default:
		{
			return BeJsDocument();
		}
		}
	}

	// All geometry de-serialized matched the originals
	BeJsDocument retVal;
	retVal["returnValue"] = true;
	return retVal;
	}

	static BeJsDocument buildKnownGeometryStream(DgnDbR dbin, Utf8String params)
	{
	BeJsDocument props(params);
	if (!props.isMember("bsurfacePts") || !props.isMember("numSurfacePts") || !props["bsurfacePts"].isArray() ||
		!props.isMember("polyPts") || !props.isMember("numPolyPts") || !props["polyPts"].isArray() ||
		!props.isMember("outFileName"))
		return BeJsDocument();

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
	auto surfacePts = props["bsurfacePts"];
	for (int i = 0; i < numSurfacePts; i++)
	{
		pointArr.push_back(DPoint3d::From(surfacePts[i][0].asInt(), surfacePts[i][1].asInt(), surfacePts[i][2].asInt()));
	}
	MSBsplineSurfacePtr origSurface = MSBsplineSurface::CreatePtr();
	origSurface->InitFromPointsAndOrder(3, 4, 4, 6, &pointArr[0]);

	int numPolyPts = props["numPolyPts"].asInt();
	auto polyPts = props["polyPts"];
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
	DgnDbPtr db = DgnDb::CreateIModel(&status, dbName, dgndbParams);
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
	BeJsDocument retVal;
	retVal["geom"] = gs.ToBase64();
	return retVal;
	}

END_UNNAMED_NAMESPACE

BeJsDocument IModelJsNative::JsInterop::ExecuteTest(DgnDbR db, Utf8StringCR testName, Utf8StringCR params)
    {
	if (testName.Equals("lookAtVolume")) return lookAtVolume(db, params);
	if (testName.Equals("lookAtUsingLensAngle")) return lookAtUsingLensAngle(db, params);
	if (testName.Equals("rotateCameraLocal")) return rotateCameraLocal(db, params);
	if (testName.Equals("buildKnownGeometryStream")) return buildKnownGeometryStream(db, params);
	if (testName.Equals("deserializeGeometryStream")) return deserializeGeometryStream(db, params);
	return BeJsDocument();
    }
