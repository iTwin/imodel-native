/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "serializationPCH.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include "BeCGWriter.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// static Utf8CP s_jasonValueWriterRootName = "_IModelJsonValueWriter_root";

//=======================================================================================
//
// method names:
// <ul>
// <li> appendXXX (data, jsonArray) -- create XXX as json value, append it to jsonArray

// <li> createXXX (data, jsonObj) -- return (!!) Json::Value ()
// </ul>
// @bsiclass                                                    Earlin.Lutz 3/18
//=======================================================================================
struct BeCGIModelJsonValueWriter
    {
    bool m_packIsolatedPoints;
    bool m_packArrays;
    Json::Value singleton (const char *name, Json::Value const &value)
        {
        Json::Value result;
        result[name] = value;
        return result;
        }
    void appendIfNotNull (Json::Value &jsonArray, Json::Value const &candidate)
        {
        if (!candidate.isNull ())
            jsonArray.append (candidate);
        }

    Json::Value toJson (DPoint3dCR point, bool compact)
        {
        if (compact)
            {
            auto result = Json::Value();
            result.append (Json::Value(point.x));
            result.append (Json::Value(point.y));
            result.append (Json::Value(point.z));
            return result;
            }
        auto result = Json::Value ();
        result["x"] = point.x;
        result["y"] = point.y;
        result["z"] = point.z;
        return result;
        }

    Json::Value toJson (DPoint2dCR point, bool compact)
        {
        if (compact)
            {
            auto result = Json::Value();
            result.append (Json::Value(point.x));
            result.append (Json::Value(point.y));
            return result;
            }
        auto result = Json::Value ();
        result["x"] = point.x;
        result["y"] = point.y;
        return result;
        }

    Json::Value toJson (DPoint3dCR point, double weight, bool compact)
        {
        if (compact)
            {
            auto result = Json::Value();
            result.append (Json::Value(point.x));
            result.append (Json::Value(point.y));
            result.append (Json::Value(point.z));
            result.append (Json::Value(weight));
            return result;
            }
        auto result = Json::Value ();
        result["x"] = point.x;
        result["y"] = point.y;
        result["z"] = point.z;
        result["w"] = weight;
        return result;
        }

    Json::Value toJson(bvector<DPoint3d> const &points)
        {
        Json::Value jsonArray;
        for (auto &xyz : points)
            jsonArray.append (toJson (xyz, m_packArrays));
        return jsonArray;
        }

    Json::Value toJson(DPoint3dCP points, size_t n)
        {
        Json::Value jsonArray;
        for (auto i = 0; i < n; i++)
            jsonArray.append (toJson (points[i], m_packArrays));
        return jsonArray;
        }

    Json::Value toJson(DPoint2dCP points, size_t n)
        {
        Json::Value jsonArray;
        for (auto i = 0; i < n; i++)
            jsonArray.append (toJson (points[i], m_packArrays));
        return jsonArray;
        }
    Json::Value toJson(bvector<int> const &data)
        {
        Json::Value jsonArray;
        for (auto &a : data)
            jsonArray.append (a);
        return jsonArray;
        }

    Json::Value toJson(uint32_t const *data,  size_t n)
        {
        Json::Value jsonArray;
        for (size_t i = 0; i < n; i++)
            jsonArray.append (data[i]);
        return jsonArray;
        }

    Json::Value toJson(int32_t const *data,  size_t n)
        {
        Json::Value jsonArray;
        for (size_t i = 0; i < n; i++)
            jsonArray.append (data[i]);
        return jsonArray;
        }


    Json::Value toJson(bvector<double> const &data)
        {
        Json::Value jsonArray;
        for (auto &a : data)
            jsonArray.append (Json::Value (a));
        return jsonArray;
        }

    Json::Value createSweepStartSweepRadiansToStartEndDegrees (double radians0, double sweepRadians)
        {
        auto result = Json::Value();
        result.append (Angle::RadiansToDegrees (radians0));
        result.append (Angle::RadiansToDegrees (radians0 + sweepRadians));
        return result;
        }

    Json::Value BsurfToJson (MSBsplineSurfaceCR bsurf)
        {
        Json::Value value;
        bool rational = bsurf.rational != 0;
        bvector<double> knots;
        value["orderU"] = static_cast<uint32_t>(bsurf.GetUOrder ());
        value["orderV"] = static_cast<uint32_t>(bsurf.GetVOrder ());

        bsurf.GetUKnots (knots);
        value["uKnots"] = toJson (knots);
        bsurf.GetVKnots (knots);
        value["vKnots"] = toJson (knots);
        // build this grid array directly to avoid copying all the poles
        Json::Value pointGrid;
        auto numU = bsurf.GetNumUPoles ();
        auto numV = bsurf.GetNumVPoles ();
        for (auto j = 0; j < numV; j++)
            {
            Json::Value row;
            for (auto i = 0; i < numU; i++)
                {
                if (rational)
                    row.append (toJson (bsurf.GetPole (i, j), bsurf.GetWeight(i,j), this->m_packArrays));
                else
                    row.append (toJson (bsurf.GetPole (i, j), this->m_packArrays));
                }
            pointGrid.append (row);
            }
        value["points"] = pointGrid;
        return singleton ("bsurf", value);

        }
#ifdef RawImjs
{"torusPipe":
 {"center":[1,50,3],
  "majorRadius":10,
  "minorRadius":1,
  "xyVectors":[[0.6989117773223881,-0.3136225207688945,0.6427777547384671],
  [0.10329524078492897,0.9335692701842065,0.34318873961555296]],
  "sweepAngle":45,
  "capped":true
 }
}]
// ** omit sweep if full circle.
// ** omit cap if false
#endif
Json::Value TorusPipeToJson (ISolidPrimitiveCR sp)
    {
    DgnTorusPipeDetail detail;
    Transform localToWorld, worldToLocal;
    if (sp.TryGetDgnTorusPipeDetail (detail)
        && detail.TryGetConstructiveFrame (localToWorld, worldToLocal))
        {
        Json::Value value;
        value["center"] = toJson (detail.m_center, m_packIsolatedPoints);
        value["majorRadius"] = detail.m_majorRadius;
        value["minorRadius"] = detail.m_minorRadius;
        if (!Angle::IsFullCircle (detail.m_sweepAngle))
            value["sweepAngle"] = Angle::RadiansToDegrees (detail.m_sweepAngle);
        if (detail.m_capped)
            value["capped"] = detail.m_capped;
        Json::Value xyVectors;
        xyVectors.append (toJson (localToWorld.GetMatrixColumn (0), m_packIsolatedPoints));
        xyVectors.append (toJson (localToWorld.GetMatrixColumn (1), m_packIsolatedPoints));
        value["xyVectors"] = xyVectors;
        return singleton ("torusPipe", value);
        }
    return Json::Value ();
    }
#ifdef RawImjs
{"box":
 {"baseOrigin":[1,2,3],
  "baseX":3,
  "baseY":2,
  "capped":true,
  "topOrigin":[1,2,8],
  "xyVectors":[[0.984807753012208,0.17364817766693033,0],
  [-0.17364817766693033,0.984807753012208,0]],
  "topX":1.5,
  "topY":1
 },
 "xyVectors":[[0.984807753012208,0.17364817766693033,0],
 [-0.17364817766693033,0.984807753012208,0]]
},
#endif
Json::Value BoxToJson (ISolidPrimitiveCR sp)
    {
    DgnBoxDetail detail;
    Transform localToWorld, worldToLocal;
    if (sp.TryGetDgnBoxDetail (detail)
        && detail.TryGetConstructiveFrame (localToWorld, worldToLocal))
        {
        Json::Value value;
        value["baseOrigin"] = toJson (detail.m_baseOrigin, m_packIsolatedPoints);
        value["topOrigin"] = toJson (detail.m_topOrigin, m_packIsolatedPoints);
        value["baseX"] = detail.m_baseX;
        value["baseY"] = detail.m_baseY;
        if (!DoubleOps::AlmostEqual (detail.m_topX, detail.m_baseX))
            value["topX"] = detail.m_topX;
        if (!DoubleOps::AlmostEqual (detail.m_topY, detail.m_baseY))
            value["topY"] = detail.m_topY;
        value["capped"] = detail.m_capped;
        if (!localToWorld.Matrix ().IsIdentity ())
            {
            Json::Value xyVectors;
            xyVectors.append (toJson (localToWorld.GetMatrixColumn (0), m_packIsolatedPoints));
            xyVectors.append (toJson (localToWorld.GetMatrixColumn (1), m_packIsolatedPoints));
            value["xyVectors"] = xyVectors;
            }
        return singleton ("box", value);
        }
    return Json::Value ();
    }
#ifdef RawImjs
{"sphere":
 {"center":[1,2,3],
  "zxVectors":[[0,1,0],[0,0,1]],
  "capped":true,
  "radius":4,
  "latitudeStartEnd":[-45,45]
 }
}]
#endif
Json::Value SphereToJson (ISolidPrimitiveCR sp)
    {
    DgnSphereDetail detail;
    DPoint3d center;
    DVec3d unitX, unitY, unitZ;
    double rxy, rz;
    if (sp.TryGetDgnSphereDetail (detail)
        && detail.IsTrueRotationAroundZ (center, unitX, unitY, unitZ, rxy, rz))
        {
        Json::Value value;
        value["center"] = toJson (center, m_packIsolatedPoints);
        auto axes = RotMatrix::FromColumnVectors (unitX, unitY, unitZ);
        if (detail.m_capped)
            value["capped"] = detail.m_capped;
        if (axes.IsIdentity ())
            {
            // omit orientation of unrotated !!
            }
        else
            {
            Json::Value zxVectors;
            zxVectors.append (toJson (unitZ, m_packIsolatedPoints));
            zxVectors.append (toJson (unitX, m_packIsolatedPoints));
            value["zxVectors"] = zxVectors;
            }
        if (DoubleOps::AlmostEqual (rxy, rz))
            {
            value["radius"] = rz;
            }
        else
            {
            value["radiusX"] = rxy;   // radiusY will pick this up by default.
            value["radiusZ"] = rz;
            }
        if (Angle::NearlyEqualAllowPeriodShift (detail.m_startLatitude, -Angle::PiOver2 ())
            && Angle::NearlyEqualAllowPeriodShift (detail.m_latitudeSweep, Angle::Pi ())
            )
            {
            // full sweep -- omit sweep data.
            }
        else
            {
            Json::Value latitudeStartEnd;
            latitudeStartEnd.append (Angle::RadiansToDegrees (detail.m_startLatitude));
            latitudeStartEnd.append (Angle::RadiansToDegrees (detail.m_startLatitude + detail.m_latitudeSweep));
            value["latitudeStartEnd"] = latitudeStartEnd;
            }

        return singleton ("sphere", value);
        }
    return Json::Value ();
    }

#ifdef RawImjs
{"cylinder":
 {"capped":false,
  "start":[1,2,1],
  "end":[2,3,8],
  "radius":0.5
 }
}

{"cone":
 {"capped":true,
  "start":[0,0,0],
  "end":[0,0,5],
  "startRadius":1,
  "endRadius":0.2,
  "xyVectors":[[1,0,0], [0,1,0]]
 }
},
#endif
Json::Value ConeToJson (ISolidPrimitiveCR sp)
    {
    static bool s_alwaysOutputConeXY = true;
    DgnConeDetail detail;
    DPoint3d centerA, centerB;
    double radiusA, radiusB;
    RotMatrix axes;
    bool capped;
    if (sp.TryGetDgnConeDetail (detail))
        {
        if (detail.IsCylinder (centerA, centerB, radiusA, capped))
            {
            Json::Value value;
            value["start"] = toJson (centerA, m_packIsolatedPoints);
            value["end"] = toJson (centerB, m_packIsolatedPoints);
            value["capped"] = capped;
            value["radius"] = radiusA;
            return singleton ("cylinder", value);
            }
        else if (detail.IsCircular (centerA, centerB, axes, radiusA, radiusB, capped))
            {
            DVec3d unitX, unitY, unitZ;
            axes.GetColumns (unitX, unitY, unitZ);
            double dAB = centerA.Distance (centerB);
            double hB = centerB.DotDifference (centerA, unitZ);
            Json::Value value;
            value["start"] = toJson (centerA, m_packIsolatedPoints);
            value["end"] = toJson (centerB, m_packIsolatedPoints);
            value["capped"] = capped;
            // optional compress to single radius ..
            if (DoubleOps::AlmostEqual (radiusA, radiusB))
                {
                value["radius"] = radiusA;
                }
            else
                {
                value["startRadius"] = radiusA;
                value["endRadius"] = radiusB;
                }
            if (s_alwaysOutputConeXY || !DoubleOps::AlmostEqual (hB, dAB))
                {
                // skewed cone needs explicit orientation (the perpendicular to the circle plane is not the cone axis)
                Json::Value xyVectors;
                xyVectors.append (toJson (unitX, m_packIsolatedPoints));
                xyVectors.append (toJson (unitY, m_packIsolatedPoints));
                value["xyVectors"] = xyVectors;
                }
            return singleton ("cone", value);
            }
        else
            {
            // ?? elliptic section at base ??
            }
        }
    return Json::Value ();
    }

#ifdef RawImjs
{"ruledSweep":
 {"contour":[
  {"path":<pathContents>}
  {"path":<pathContents>}
  ],
  "capped":false
 }
}
#endif
Json::Value RuledSweepToJson (ISolidPrimitiveCR sp)
    {
    DgnRuledSweepDetail detail;
    Json::Value value;
    if (sp.TryGetDgnRuledSweepDetail (detail))
        {
        Json::Value children;
        for (size_t i = 0; i < detail.m_sectionCurves.size (); i++)
            {
            auto child = CurveVectorToJson (*detail.m_sectionCurves[i]);
            children.append (child);
            }
        value["contour"] = children;
        value["capped"] = detail.m_capped;
        return singleton ("ruledSweep", value);
        }
    return Json::Value ();
    }

#ifdef RawImjs
{"linearSweep":{
 "contour":{<CurveVector>},
 "capped":false,
 "vector":[0,0,1.234]
 }}]
#endif
Json::Value LinearSweepToJson (ISolidPrimitiveCR sp)
    {
    DgnExtrusionDetail detail;
    Json::Value value;
    if (sp.TryGetDgnExtrusionDetail (detail))
        {
        value["contour"] = CurveVectorToJson (*detail.m_baseCurve);
        value["capped"] = detail.m_capped;
        value["vector"] = toJson (detail.m_extrusionVector, m_packIsolatedPoints);
        return singleton ("linearSweep", value);
        }
    return Json::Value ();
    }

#ifdef RawImjs
{"rotationalSweep":
 {"axis":[0,1,0],
  "contour":{<CurveVector>},
  "capped":false,
  "center":[0,0,0],
  "sweepAngle":119.99999999999999
 }
},
#endif
Json::Value RotationalSweepToJson (ISolidPrimitiveCR sp)
    {
    DgnRotationalSweepDetail detail;
    Json::Value value;
    if (sp.TryGetDgnRotationalSweepDetail (detail))
        {
        value["contour"] = CurveVectorToJson (*detail.m_baseCurve);
        value["capped"] = detail.m_capped;
        value["center"] = toJson (detail.m_axisOfRotation.origin, m_packIsolatedPoints);
        value["axis"] = toJson (detail.m_axisOfRotation.direction, m_packIsolatedPoints);
        value["sweepAngle"] = Angle::RadiansToDegrees (detail.m_sweepAngle);
        return singleton ("rotationalSweep", value);
        }
    return Json::Value ();
    }

#ifdef RawImjs
{"indexedMesh":
 {"color":[10,11,12,13,....,83,84,85,86,87,88,89],
  "colorIndex":[1,1,1,0,2,2,2,0,3,3,3,0,......80,80,80,0],
  "point":[[0,0,0], [1,0,0],[2,0,0],  [3,0,0],  [4,0,0],   . . . .   [6,5,0],  [7,5,0],  [8,5,0]],
  "pointIndex":[  1,2,-11,0,11,10,-1,0,2,3,-12,0,   . . .32,33,-42,0,42,41,-32,0],
  "paramIndex":{..],
  "param":[[0,0],[1,0],...],
  "normalIndex":{..],
  "normal":[[1,0,0],[0,1,0],..],
 }
}
#endif

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      04/2018
+--------------------------------------------------------------------------------------*/
static Json::Value ToJson(PolyfaceAuxChannel::DataCR in)
    {
    Json::Value     value(Json::objectValue), dataValues(Json::arrayValue);
    
    value["input"] = in.GetInput();

    for (auto& dataValue : in.GetValues())
        dataValues.append(dataValue);

    value["values"] = std::move(dataValues);

    return value;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      04/2018
+--------------------------------------------------------------------------------------*/
static Json::Value ToJson(PolyfaceAuxChannelCR in)  
    {
    Json::Value     value(Json::objectValue), dataArray(Json::arrayValue);

    value["dataType"] = (int32_t) in.GetDataType();
    value["name"] = in.GetName();
    value["inputName"] = in.GetInputName();

    for (auto data : in.GetData())
        dataArray.append(ToJson(*data));

    value["data"] = std::move(dataArray);

    return value;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      04/2018
+--------------------------------------------------------------------------------------*/
static Json::Value ToJson(PolyfaceAuxData::ChannelsCR in) 
    {
    Json::Value     value(Json::arrayValue);

    for (auto& channel : in)
        value.append(ToJson(*channel));

    return value;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      04/2018
+--------------------------------------------------------------------------------------*/
static Json::Value ToJson(PolyfaceAuxDataCR in) 
    {
    Json::Value value(Json::objectValue), indices(Json::arrayValue);

    for (auto& index : in.GetIndices())
        indices.append(index);
    
    value["indices"] = std::move(indices);
    value["channels"] = ToJson(in.GetChannels());

    return value;
    }

Json::Value IndexedPolyfaceToJson (PolyfaceHeaderCR mesh)
    {
    if (mesh.GetMeshStyle () != MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
        return Json::Value();
    Json::Value allData;
    auto indexCount = mesh.GetPointIndexCount ();
    static bool s_outputNumPerFace = false;
    static bool s_outputTwoSided = false;
    if (s_outputNumPerFace)
        allData["numPerFace"] = mesh.GetNumPerFace ();
    if (s_outputTwoSided)
        allData["twoSided"] = mesh.GetTwoSided ();

    allData["point"] = toJson (mesh.GetPointCP (), mesh.GetPointCount ());
    allData["pointIndex"] = toJson (mesh.GetPointIndexCP (), indexCount);

    if (mesh.GetColorCount() > 0)
        allData["color"] = toJson (mesh.GetIntColorCP (), mesh.GetColorCount ());
    if (mesh.GetColorIndexCP() != nullptr)
        allData["colorIndex"] = toJson (mesh.GetColorIndexCP (), indexCount);

    if (mesh.GetParamCount() > 0)
        allData["param"] = toJson (mesh.GetParamCP (), mesh.GetParamCount ());
    if (mesh.GetParamIndexCP() != nullptr)
        allData["paramIndex"] = toJson (mesh.GetParamIndexCP (), indexCount);

    if (mesh.GetNormalCount() > 0)
        allData["normal"] = toJson (mesh.GetNormalCP (), mesh.GetNormalCount ());
    if (mesh.GetNormalIndexCP() != nullptr)
        allData["normalIndex"] = toJson (mesh.GetNormalIndexCP (), indexCount);

    if(mesh.GetAuxDataCP().IsValid())
        allData["auxData"] = ToJson(*mesh.GetAuxDataCP());

    return singleton ("indexedMesh", allData);
    }

Json::Value CurvePrimitiveToJson (ICurvePrimitiveCR cp)
    {
    DSegment3d segment;
    DEllipse3d arc;
    if (cp.TryGetLine (segment))
        {
        // RawIMJS {"lineSegment":[[0,0,0], [3,3,0]]}]
        Json::Value value;
        value.append (toJson (segment.point[0], m_packIsolatedPoints));
        value.append (toJson (segment.point[1], m_packIsolatedPoints));
        return singleton ("lineSegment", value);
        }
    // RawIJMS
    // {"arc":
    // {"center":[0,0,0],
    //  "vectorX":[3,0,0],
    //  "vectorY":[0,3,0],
    //  "sweepStartEnd":[-40,270]
    // }
    if (cp.TryGetArc (arc))
        {
        Json::Value value;
        value["center"] = toJson (arc.center, m_packIsolatedPoints);
        value["vectorX"] = toJson (arc.vector0, m_packIsolatedPoints);
        value["vectorY"] = toJson (arc.vector90, m_packIsolatedPoints);
        value["sweepStartEnd"] = createSweepStartSweepRadiansToStartEndDegrees (arc.start, arc.sweep);
        return singleton ("arc", value);
        }
    bvector<DPoint3d> const *points = cp.GetLineStringCP ();
    // RawIMJS {"lineString":[[0,0,0], [1,0,0],  [1,1,0]]},
    if (points != nullptr)
        return singleton ("lineString", toJson (*points));

    points = cp.GetPointStringCP ();
    // RawIMJS {"pointString":[[0,0,0], [1,0,0],  [1,1,0]]},
    if (points != nullptr)
        return singleton ("pointString", toJson (*points));

    auto child = cp.GetChildCurveVectorCP ();
    if (child != nullptr)
        return CurveVectorToJson (*child);

    auto bcurve = cp.GetBsplineCurveCP ();
    if (bcurve != nullptr)
        {
        Json::Value value;  
        bvector<DPoint3d> poles;
        bvector<double>knots;
        bcurve->GetPoles (poles);
        bcurve->GetKnots (knots);   
        value["points"] = toJson (poles);
        value["order"] = Json::Value (bcurve->GetOrder ());
        value["knots"] = toJson (knots);
        value["closed"] = Json::Value (bcurve->IsClosed ());
        return singleton ("bcurve", value);
        }
    return Json::Value ();
    }

Json::Value CurveVectorToJson (CurveVectorCR cv)
    {
    auto type = cv.GetBoundaryType ();
    Json::Value children;
    for (auto &cp : cv)
        appendIfNotNull (children, CurvePrimitiveToJson (*cp));
    if (children.size () > 0)
        {
        if (type == CurveVector::BOUNDARY_TYPE_Open)
            return singleton ("path", children);
        if (type == CurveVector::BOUNDARY_TYPE_Outer)
            return singleton ("loop", children);
        if (type == CurveVector::BOUNDARY_TYPE_Inner)
            return singleton ("loop", children);
        if (type == CurveVector::BOUNDARY_TYPE_ParityRegion)
            return singleton ("parityRegion", children);
        if (type == CurveVector::BOUNDARY_TYPE_UnionRegion)
            return singleton ("unionRegion", children);
        if (type == CurveVector::BOUNDARY_TYPE_None)
            return singleton ("bagOfCurves", children);
        }
    return Json::Value ();
    }

Json::Value SolidPrimitiveToJson (ISolidPrimitiveCR solid)
        {
        auto type = solid.GetSolidPrimitiveType ();
        if (type == SolidPrimitiveType_DgnTorusPipe)
            return TorusPipeToJson (solid);
        if (type == SolidPrimitiveType_DgnBox)
            return BoxToJson (solid);
        if (type == SolidPrimitiveType_DgnSphere)
            return SphereToJson (solid);
        if (type == SolidPrimitiveType_DgnCone)
            return ConeToJson (solid);
        if (type == SolidPrimitiveType_DgnExtrusion)
            return LinearSweepToJson (solid);
        if (type == SolidPrimitiveType_DgnRotationalSweep)
            return RotationalSweepToJson (solid);
        if (type == SolidPrimitiveType_DgnRuledSweep)
            return RuledSweepToJson (solid);
        return Json::Value ();
        }

Json::Value PolyfaceToJson (PolyfaceHeaderCR mesh)
        {
        auto style = mesh.GetMeshStyle ();
        if (style == MESH_ELM_STYLE_INDEXED_FACE_LOOPS && mesh.GetNumPerFace () < 2)
            return IndexedPolyfaceToJson (mesh);

        auto indexedMesh = mesh.CloneAsVariableSizeIndexed(mesh);
        return IndexedPolyfaceToJson(*indexedMesh);
        }


    Json::Value GeometryToJson (IGeometryCR g)
        {
        ICurvePrimitivePtr cp = g.GetAsICurvePrimitive ();
        if (cp.IsValid ())
            return CurvePrimitiveToJson (*cp);
        CurveVectorPtr cv = g.GetAsCurveVector ();
        if (cv.IsValid ())
            return CurveVectorToJson (*cv);
        MSBsplineSurfacePtr bsurf = g.GetAsMSBsplineSurface ();
        if (bsurf.IsValid ())
            return BsurfToJson (*bsurf);
        ISolidPrimitivePtr sp = g.GetAsISolidPrimitive();
        if (sp.IsValid ())
            return SolidPrimitiveToJson (*sp);

        PolyfaceHeaderPtr pf = g.GetAsPolyfaceHeader();
        if (pf.IsValid ())
            {
            return PolyfaceToJson(*pf);
            }
        return Json::Value ();
        }

    public: BeCGIModelJsonValueWriter ()
        {
        m_packArrays = true;
        m_packIsolatedPoints = true;
        }

};

bool IModelJson::TryGeometryToIModelJsonValue(Json::Value &jsonArray, bvector<IGeometryPtr> const &data)
    {
    BeCGIModelJsonValueWriter builder;
    jsonArray = Json::Value ();
    for (auto &g : data)
        builder.appendIfNotNull (jsonArray, builder.GeometryToJson (*g));
    return !jsonArray.isNull ();
    }

bool IModelJson::TryGeometryToIModelJsonValue (Json::Value &value, IGeometryCR data)
    {
    BeCGIModelJsonValueWriter builder;
    value = builder.GeometryToJson (data);
    return !value.isNull ();
    }

bool IModelJson::TryGeometryToIModelJsonString (Utf8StringR string, IGeometryCR data)
    {
    Json::Value value;
    if (TryGeometryToIModelJsonValue(value, data))
        {
        Json::FastWriter fastWriter;
        string = fastWriter.write(value);
        return true;
        }
    return false;
    }

bool IModelJson::TryGeometryToIModelJsonString(Utf8StringR string, bvector<IGeometryPtr> const &data)
    {
    Json::Value value;
    if (TryGeometryToIModelJsonValue(value, data))
        {
        Json::FastWriter fastWriter;
        string = fastWriter.write(value);
        return true;
        }
    return false;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
