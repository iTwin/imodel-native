/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "serializationPCH.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include "BeCGWriter.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static GeometryValidatorPtr s_writeValidator = GeometryValidator::Create();

// static Utf8CP s_jasonValueWriterRootName = "_IModelJsonValueWriter_root";
static double CurvatureToRadius(double curvature)
    {
    double a;
    DoubleOps::SafeDivide(a, 1.0, curvature, 0.0);
    return a;
    }
//=======================================================================================
//
// method names:
// <ul>
// <li> appendXXX (data, jsonArray) -- create XXX as json value, append it to jsonArray

// <li> createXXX (data, jsonObj) -- return (!!) Json::Value ()
// </ul>
// @bsiclass
//=======================================================================================
struct BeCGIModelJsonValueWriter
    {
    bool m_packIsolatedPoints;
    bool m_packArrays;
    void appendIfNotNull (BeJsValue jsonArray, BeJsConst candidate)
        {
        if (!candidate.isNull())
            jsonArray.appendValue().From(candidate);
        }

    void toJson (BeJsValue result, DPoint3dCR point, bool compact)
        {
        if (compact)
            {
            result.appendValue() = point.x;
            result.appendValue() = point.y;
            result.appendValue() = point.z;
            return;
            }
        result["x"] = point.x;
        result["y"] = point.y;
        result["z"] = point.z;
        }

    void toJson(BeJsValue result, DPoint4dCR point, bool compact)
        {
        if (compact)
            {
            result.appendValue() = point.x;
            result.appendValue() = point.y;
            result.appendValue() = point.z;
            result.appendValue() = point.w;
            return ;
            }
        result["x"] = point.x;
        result["y"] = point.y;
        result["z"] = point.z;
        result["w"] = point.w;
        }

    void toJson(BeJsValue result, DPoint2dCR point, bool compact)
        {
        if (compact)
            {
            result.appendValue() = point.x;
            result.appendValue() = point.y;
            return;
            }
        result["x"] = point.x;
        result["y"] = point.y;
        }

    void toJson(BeJsValue result, DPoint3dCR point, double weight, bool compact)
        {
        if (compact)
            {
            result.appendValue() = point.x;
            result.appendValue() = point.y;
            result.appendValue() = point.z;
            result.appendValue() = weight;
            return;
            }
        result["x"] = point.x;
        result["y"] = point.y;
        result["z"] = point.z;
        result["w"] = weight;
        }

    void toJson(BeJsValue jsonArray, bvector<DPoint3d> const &points)
        {
        for (auto &xyz : points)
            toJson (jsonArray.appendValue(), xyz, m_packArrays);
        }


    void toJson(BeJsValue jsonArray, bvector<DPoint4d> const &points)
        {
        for (auto &xyz : points)
            toJson (jsonArray.appendValue(), xyz, m_packArrays);
        }

    void toJson(BeJsValue jsonArray, DPoint3dCP points, size_t n)
        {
        for (auto i = 0; i < n; i++)
             toJson (jsonArray.appendValue(), points[i], m_packArrays);
        }

    void FaceDataToJson(BeJsValue jsonArray, FacetFaceDataCP data, size_t n)
        {
        double doubles[8];
        for (size_t k = 0; k < n; k++)
            {
            FacetFaceDataCP ffd = data + k;
            uint32_t i = 0;
            doubles[i++] = ffd->m_paramDistanceRange.low.x;
            doubles[i++] = ffd->m_paramDistanceRange.low.y;
            doubles[i++] = ffd->m_paramDistanceRange.high.x;
            doubles[i++] = ffd->m_paramDistanceRange.high.y;

            doubles[i++] = ffd->m_paramRange.low.x;
            doubles[i++] = ffd->m_paramRange.low.y;
            doubles[i++] = ffd->m_paramRange.high.x;
            doubles[i++] = ffd->m_paramRange.high.y;
            toJson(jsonArray.appendValue(), doubles, i);
            }
        }

    void toJson(BeJsValue jsonArray, DPoint2dCP points, size_t n)
        {
        for (auto i = 0; i < n; i++)
            toJson (jsonArray.appendValue(), points[i], m_packArrays);
        }
    void toJson(BeJsValue jsonArray, bvector<int> const &data)
        {
        for (auto &a : data)
            jsonArray.appendValue() = a;
        }

    void toJson(BeJsValue jsonArray, uint32_t const *data,  size_t n)
        {
        for (size_t i = 0; i < n; i++)
            jsonArray.appendValue() = data[i];
        }

    void toJson(BeJsValue jsonArray, int32_t const *data,  size_t n)
        {
        for (size_t i = 0; i < n; i++)
            jsonArray.appendValue()= data[i];
        }

    void toJson(BeJsValue jsonArray, double const *data, size_t n)
        {
        for (size_t i = 0; i < n; i++)
            jsonArray.appendValue() = data[i];
        }

    void toJson(BeJsValue jsonArray, bvector<double> const &data)
        {
        for (auto &a : data)
            jsonArray.appendValue() = a;
        }

    void createSweepStartSweepRadiansToStartEndDegrees (BeJsValue result, double radians0, double sweepRadians)
        {
        result.appendValue() = Angle::RadiansToDegrees (radians0);
        result.appendValue() = Angle::RadiansToDegrees (radians0 + sweepRadians);
        }

    void BsurfToJson (BeJsValue in, MSBsplineSurfaceCR bsurf)
        {

        auto value = in["bsurf"];
        bool rational = bsurf.rational != 0;
        bvector<double> knots;
        value["orderU"] = static_cast<uint32_t>(bsurf.GetUOrder ());
        value["orderV"] = static_cast<uint32_t>(bsurf.GetVOrder ());
        bsurf.GetUKnots (knots);
        toJson (value["uKnots"], knots);
        bsurf.GetVKnots (knots);
        toJson (value["vKnots"], knots);
        if (bsurf.GetIsUClosed())
            value["closedU"] = true;
        if (bsurf.GetIsVClosed())
            value["closedV"] = true;
        // build this grid array directly to avoid copying all the poles
        auto numU = bsurf.GetNumUPoles ();
        auto numV = bsurf.GetNumVPoles ();
        auto pointGrid =value["points"];
        for (auto j = 0; j < numV; j++)
            {
            auto row = pointGrid.appendValue();
            for (auto i = 0; i < numU; i++)
                {
                if (rational)
                    toJson (row.appendValue(), bsurf.GetPole (i, j), bsurf.GetWeight(i,j), this->m_packArrays);
                else
                    toJson (row.appendValue(), bsurf.GetPole (i, j), this->m_packArrays);
                }
            }
        CurveVectorPtr boundaries = bsurf.GetUVBoundaryCurves(false, false);
        if (boundaries.IsValid () && boundaries->size () > 0)
            {
            CurveVectorToJson(value["uvBoundaries"], *boundaries);
            if (!bsurf.IsOuterBoundaryActive())
                value["outerBoundaryActive"] = false;
            }
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

void TorusPipeToJson (BeJsValue in, ISolidPrimitiveCR sp)
    {
    auto value = in["torusPipe"];
    DgnTorusPipeDetail detail;
    Transform localToWorld, worldToLocal;
    if (sp.TryGetDgnTorusPipeDetail (detail)
        && detail.TryGetConstructiveFrame (localToWorld, worldToLocal))
        {
        value["majorRadius"] = detail.m_majorRadius;
        value["minorRadius"] = detail.m_minorRadius;
        if (!Angle::IsFullCircle (detail.m_sweepAngle))
            value["sweepAngle"] = Angle::RadiansToDegrees (detail.m_sweepAngle);
        if (detail.m_capped)
            value["capped"] = detail.m_capped;
        toJson(value["center"], detail.m_center, m_packIsolatedPoints);
        auto xyVectors = value["xyVectors"];
        toJson(xyVectors.appendValue(), localToWorld.GetMatrixColumn (0), m_packIsolatedPoints);
        toJson(xyVectors.appendValue(), localToWorld.GetMatrixColumn (1), m_packIsolatedPoints);
        }
    }

#ifdef RawImjs
{"box":
 {"baseOrigin":[1,2,3],
  "origin":[1,2,3],
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
void BoxToJson (BeJsValue in, ISolidPrimitiveCR sp)
    {
    auto value = in["box"];
    DgnBoxDetail detail;
    Transform localToWorld, worldToLocal;
    if (sp.TryGetDgnBoxDetail (detail)
        && detail.TryGetConstructiveFrame (localToWorld, worldToLocal))
        {
        toJson (value["baseOrigin"], detail.m_baseOrigin, m_packIsolatedPoints);
        toJson (value["origin"], detail.m_baseOrigin, m_packIsolatedPoints);
        toJson (value["topOrigin"], detail.m_topOrigin, m_packIsolatedPoints);
        value["baseX"] = detail.m_baseX;
        value["baseY"] = detail.m_baseY;
        if (!DoubleOps::AlmostEqual (detail.m_topX, detail.m_baseX))
            value["topX"] = detail.m_topX;
        if (!DoubleOps::AlmostEqual (detail.m_topY, detail.m_baseY))
            value["topY"] = detail.m_topY;
        value["capped"] = detail.m_capped;
        if (!localToWorld.Matrix ().IsIdentity ())
            {
            auto xyVectors = value["xyVectors"];
            toJson (xyVectors.appendValue(), localToWorld.GetMatrixColumn (0), m_packIsolatedPoints);
            toJson (xyVectors.appendValue(), localToWorld.GetMatrixColumn (1), m_packIsolatedPoints);
            }
        }
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
void SphereToJson (BeJsValue in, ISolidPrimitiveCR sp)
    {
    auto value = in["sphere"];
    DgnSphereDetail detail;
    DPoint3d center;
    DVec3d unitX, unitY, unitZ;
    double rxy, rz;
    if (sp.TryGetDgnSphereDetail (detail)
        && detail.IsTrueRotationAroundZ (center, unitX, unitY, unitZ, rxy, rz))
        {
        toJson (value["center"], center, m_packIsolatedPoints);
        auto axes = RotMatrix::FromColumnVectors (unitX, unitY, unitZ);
        if (detail.m_capped)
            value["capped"] = detail.m_capped;
        if (axes.IsIdentity ())
            {
            // omit orientation of unrotated !!
            }
        else
            {
            auto zxVectors = value["zxVectors"];
            toJson (zxVectors.appendValue(),unitZ, m_packIsolatedPoints);
            toJson (zxVectors.appendValue(),unitX, m_packIsolatedPoints);
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
            auto latitudeStartEnd = value["latitudeStartEnd"];
            latitudeStartEnd.appendValue() = Angle::RadiansToDegrees (detail.m_startLatitude);
            latitudeStartEnd.appendValue() = Angle::RadiansToDegrees (detail.m_startLatitude + detail.m_latitudeSweep);
            }

        }
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
void ConeToJson (BeJsValue in, ISolidPrimitiveCR sp)
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
            auto value = in["cylinder"];
            toJson (value["start"], centerA, m_packIsolatedPoints);
            toJson (value["end"], centerB, m_packIsolatedPoints);
            value["capped"] = capped;
            value["radius"] = radiusA;
            return;
            }
        else if (detail.IsCircular (centerA, centerB, axes, radiusA, radiusB, capped))
            {
            auto value = in["cone"];
            DVec3d unitX, unitY, unitZ;
            axes.GetColumns (unitX, unitY, unitZ);
            double dAB = centerA.Distance (centerB);
            double hB = centerB.DotDifference (centerA, unitZ);
            toJson (value["start"], centerA, m_packIsolatedPoints);
            toJson (value["end"], centerB, m_packIsolatedPoints);
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
                auto xyVectors = value["xyVectors"];
                toJson (xyVectors.appendValue(), unitX, m_packIsolatedPoints);
                toJson (xyVectors.appendValue(), unitY, m_packIsolatedPoints);
                }
            }
        else
            {
            // ?? elliptic section at base ??
            }
        }
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
void RuledSweepToJson (BeJsValue in, ISolidPrimitiveCR sp)
    {
    DgnRuledSweepDetail detail;
    if (sp.TryGetDgnRuledSweepDetail (detail))
        {
        auto value = in["ruledSweep"];
        auto children = value["contour"];
        for (size_t i = 0; i < detail.m_sectionCurves.size (); i++)
             CurveVectorToJson (children.appendValue(), *detail.m_sectionCurves[i]);

        value["capped"] = detail.m_capped;
        }
    }

#ifdef RawImjs
{"linearSweep":{
 "contour":{<CurveVector>},
 "capped":false,
 "vector":[0,0,1.234]
 }}]
#endif
void LinearSweepToJson (BeJsValue in, ISolidPrimitiveCR sp)
    {
    DgnExtrusionDetail detail;
    if (sp.TryGetDgnExtrusionDetail (detail))
        {
        auto value = in["linearSweep"];
        value["capped"] = detail.m_capped;
        CurveVectorToJson (value["contour"], *detail.m_baseCurve);
        toJson (value["vector"], detail.m_extrusionVector, m_packIsolatedPoints);
        }
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
void RotationalSweepToJson (BeJsValue in, ISolidPrimitiveCR sp)
    {
    DgnRotationalSweepDetail detail;
    if (sp.TryGetDgnRotationalSweepDetail (detail))
        {
        auto value = in["rotationalSweep"];
        CurveVectorToJson (value["contour"], *detail.m_baseCurve);
        value["capped"] = detail.m_capped;
        toJson (value["center"], detail.m_axisOfRotation.origin, m_packIsolatedPoints);
        toJson (value["axis"], detail.m_axisOfRotation.direction, m_packIsolatedPoints);
        value["sweepAngle"] = Angle::RadiansToDegrees (detail.m_sweepAngle);
        }
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
// AUXDATA -- DO NOT MERGE TO CONNECT
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static void ToJson(BeJsValue value, PolyfaceAuxChannel::DataCR in)
    {
    value["input"] = in.GetInput();

    auto dataValues = value["values"];
    for (auto& dataValue : in.GetValues())
        dataValues.appendValue() = dataValue;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static void  ToJson(BeJsValue value, PolyfaceAuxChannelCR in)
    {
    value["dataType"] = (int32_t) in.GetDataType();
    value["name"] = in.GetName();
    value["inputName"] = in.GetInputName();

    auto dataArray = value["data"];
    for (auto data : in.GetData())
        ToJson(dataArray.appendValue(), *data);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static void ToJson(BeJsValue value, PolyfaceAuxData::ChannelsCR in)
    {
    for (auto& channel : in)
        ToJson(value.appendValue(), *channel);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static void ToJson(BeJsValue value, PolyfaceAuxDataCR in)
    {
    auto indices = value["indices"];
    for (auto& index : in.GetIndices())
        indices.appendValue() = index;

    ToJson(value["channels"], in.GetChannels());
    }

void IndexedPolyfaceToJson(BeJsValue in, PolyfaceHeaderCR mesh)
    {
    if (mesh.GetMeshStyle () != MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
        return;

    auto allData = in["indexedMesh"];
    auto indexCount = mesh.GetPointIndexCount ();
    static bool s_outputNumPerFace = true;
    static bool s_outputTwoSided = false;
    static uint32_t s_outputExpectedClosure = true;
    if (s_outputNumPerFace)
        allData["numPerFace"] = mesh.GetNumPerFace ();
    if (s_outputTwoSided)
        allData["twoSided"] = mesh.GetTwoSided ();
    if (s_outputExpectedClosure)
        allData["expectedClosure"] = mesh.GetExpectedClosure();

    toJson (allData["point"], mesh.GetPointCP (), mesh.GetPointCount ());
    toJson (allData["pointIndex"], mesh.GetPointIndexCP (), indexCount);

    if (mesh.GetColorCount() > 0)
        toJson (allData["color"], mesh.GetIntColorCP (), mesh.GetColorCount ());
    if (mesh.GetColorIndexCP() != nullptr)
        toJson (allData["colorIndex"], mesh.GetColorIndexCP (), indexCount);

    if (mesh.GetParamCount() > 0)
        toJson (allData["param"], mesh.GetParamCP (), mesh.GetParamCount ());
    if (mesh.GetParamIndexCP() != nullptr)
        toJson (allData["paramIndex"], mesh.GetParamIndexCP (), indexCount);

    if (mesh.GetNormalCount() > 0)
        toJson (allData["normal"], mesh.GetNormalCP (), mesh.GetNormalCount ());
    if (mesh.GetNormalIndexCP() != nullptr)
        toJson (allData["normalIndex"], mesh.GetNormalIndexCP (), indexCount);
    if (mesh.GetFaceDataCP() != nullptr && mesh.GetFaceCount () > 0)
        FaceDataToJson (allData["faceData"], mesh.GetFaceDataCP (), mesh.GetFaceCount ());
    if (mesh.GetFaceIndexCount () > 0)  // There is a separate GetFaceIndexCount, but it has to match GetPointIndexCount.
        toJson (allData["faceIndex"], mesh.GetFaceIndexCP (), mesh.GetPointIndexCount ());
    TaggedNumericData const *taggedData = mesh.GetNumericTagsCP();
    if (taggedData != nullptr)
        {
        ToJson (allData["tags"], *taggedData);
        }

    if(mesh.GetAuxDataCP().IsValid())
        ToJson(allData["auxData"], *mesh.GetAuxDataCP());
    }

void CurvePrimitiveToJson(BeJsValue in, ICurvePrimitiveCR cp)
    {
    DSegment3d segment;
    DEllipse3d arc;
    if (cp.TryGetLine (segment))
        {
        // RawIMJS {"lineSegment":[[0,0,0], [3,3,0]]}]
        auto value = in["lineSegment"];
        toJson (value.appendValue(), segment.point[0], m_packIsolatedPoints);
        toJson (value.appendValue(), segment.point[1], m_packIsolatedPoints);
        return;
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
        auto  value = in["arc"];
        toJson (value["center"], arc.center, m_packIsolatedPoints);
        toJson (value["vectorX"], arc.vector0, m_packIsolatedPoints);
        toJson (value["vectorY"], arc.vector90, m_packIsolatedPoints);
        createSweepStartSweepRadiansToStartEndDegrees (value["sweepStartEnd"], arc.start, arc.sweep);
        return;
        }
    bvector<DPoint3d> const *points = cp.GetLineStringCP ();
    // RawIMJS {"lineString":[[0,0,0], [1,0,0],  [1,1,0]]},
    if (points != nullptr)
        return toJson(in["lineString"], *points);

    points = cp.GetPointStringCP ();
    // RawIMJS {"pointString":[[0,0,0], [1,0,0],  [1,1,0]]},
    if (points != nullptr)
        return toJson (in["pointString"], *points);

    auto child = cp.GetChildCurveVectorCP ();
    if (child != nullptr)
        return CurveVectorToJson(in, *child);

    auto bcurve = cp.GetBsplineCurveCP ();
    if (bcurve != nullptr)
        {
        auto value = in["bcurve"];
        bvector<double>knots;

        bcurve->GetKnots (knots);
        if (bcurve->HasWeights ())
            {
            bvector<DPoint4d> poles4d;
            bcurve->GetPoles4d (poles4d);
            toJson (value["points"], poles4d);
            }
        else
            {
            bvector<DPoint3d> poles;
            bcurve->GetPoles (poles);
            toJson (value["points"], poles);
            }
        value["order"] = (int) bcurve->GetOrder();
        toJson (value["knots"], knots);
        value["closed"] = bcurve->IsClosed();
        return;
        }
    auto spiralPlacement = cp.GetSpiralPlacementCP();
/*
{"transitionSpiral":{
"activeFractionInterval":[0.0,1.0059395557393587],
 "origin":[0,0,0],
 "type":"clothoid",
 "startRadius":0,
 "endRadius":1000,
 "startBearing":0,
 "endBearing":2.8647889756541165}}
*/
    if (spiralPlacement != nullptr)
        {
        DSpiral2dDirectEvaluation const * nominalLengthSpiral = dynamic_cast <DSpiral2dDirectEvaluation const*> (spiralPlacement->spiral);

        auto value = in["transitionSpiral"];
        Transform frame = spiralPlacement->frame;
        DVec3d unitX, unitY, unitZ;
        DPoint3d origin;
        frame.GetOriginAndVectors(origin, unitX, unitY, unitZ);
        toJson(value["origin"], origin, m_packIsolatedPoints);

        Utf8String typeName;
        if (DSpiral2dBase::TransitionTypeToString (spiralPlacement->spiral->GetTransitionTypeCode (), typeName))
            value["type"] = typeName;
        else
            value["type"] = "unknown";
        if (!frame.Matrix().IsIdentity())
            {
            auto xyVectors = value["xyVectors"];
            toJson(xyVectors.appendValue(), unitX, m_packIsolatedPoints);
            toJson(xyVectors.appendValue(), unitY, m_packIsolatedPoints);
            }
        value["startRadius"] = CurvatureToRadius(spiralPlacement->spiral->mCurvature0);
        value["endRadius"] = CurvatureToRadius(spiralPlacement->spiral->mCurvature1);

        value["startBearing"] = Angle::RadiansToDegrees(spiralPlacement->spiral->mTheta0);
        if (nominalLengthSpiral != nullptr)
            value["length"] = nominalLengthSpiral->m_nominalLength;
        else
            value["endBearing"] = Angle::RadiansToDegrees(spiralPlacement->spiral->mTheta1);
        if (!DoubleOps::IsExact01(spiralPlacement->fractionA, spiralPlacement->fractionB))
            {
            auto interval = value["activeFractionInterval"];
            interval.appendValue() = spiralPlacement->fractionA;
            interval.appendValue() = spiralPlacement->fractionB;
            }
        return;
        }
    auto interpolationCurve = cp.GetInterpolationCurveCP ();
    if (interpolationCurve != nullptr)
        {
        // RawIMJS {"lineString":[[0,0,0], [1,0,0],  [1,1,0]]},
        auto value = in["interpolationCurve"];
        toJson(value["fitPoints"], interpolationCurve->fitPoints, interpolationCurve->params.numPoints);
        if (interpolationCurve->knots != nullptr && interpolationCurve->params.numKnots > 0)
            {
            toJson(value["knots"], interpolationCurve->knots, interpolationCurve->params.numKnots);
            }
        value["order"] = interpolationCurve->params.order;
        if (interpolationCurve->params.isPeriodic != 0)
            value["closed"] = true;
        if (interpolationCurve->params.isChordLenKnots != 0)
            value["isChordLenKnots"] = interpolationCurve->params.isChordLenKnots;
        if (interpolationCurve->params.isColinearTangents != 0)
            value["isColinearTangents"] = interpolationCurve->params.isColinearTangents;
        if (interpolationCurve->params.isChordLenTangents != 0)
            value["isChordLenTangents"] = interpolationCurve->params.isChordLenTangents;
        if (interpolationCurve->params.isNaturalTangents != 0)
            value["isNaturalTangents"] = interpolationCurve->params.isNaturalTangents;
        if (interpolationCurve->startTangent.Magnitude () != 0)
            toJson(value["startTangent"], interpolationCurve->startTangent, m_packIsolatedPoints);
        if (interpolationCurve->endTangent.Magnitude() != 0)
            toJson(value["endTangent"], interpolationCurve->endTangent, m_packIsolatedPoints);

/*
        table InterpolationCurve
            {
            order:int;
            closed:bool;
            isChordLenKnots:int;
            isColinearTangents:int;
            isChordLenTangents:int;
            isNaturalTangents:int;
            startTangent:DPoint3d;
            endTangent:DVector3d;
            fitPoints:[double]; // xyz flattened
            knots:[double];
*/
        }
    }

void CurveVectorToJson(BeJsValue in, CurveVectorCR cv) {
    auto type = cv.GetBoundaryType();

    Utf8CP name = nullptr;
    if (type == CurveVector::BOUNDARY_TYPE_Open)
        name = "path";
    else if (type == CurveVector::BOUNDARY_TYPE_Outer)
        name = "loop";
    else if (type == CurveVector::BOUNDARY_TYPE_Inner)
        name = "loop";
    else if (type == CurveVector::BOUNDARY_TYPE_ParityRegion)
        name = "parityRegion";
    else if (type == CurveVector::BOUNDARY_TYPE_UnionRegion)
        name = "unionRegion";
    else if (type == CurveVector::BOUNDARY_TYPE_None)
        name = "bagOfCurves";
    if (nullptr == name)
        return;

    auto head = in[name];
    for (auto& cp : cv)
        CurvePrimitiveToJson(head.appendValue(), *cp);
}

void SolidPrimitiveToJson (BeJsValue in, ISolidPrimitiveCR solid)
        {
        auto type = solid.GetSolidPrimitiveType ();
        if (type == SolidPrimitiveType_DgnTorusPipe)
            return TorusPipeToJson (in, solid);
        if (type == SolidPrimitiveType_DgnBox)
            return BoxToJson (in, solid);
        if (type == SolidPrimitiveType_DgnSphere)
            return SphereToJson (in, solid);
        if (type == SolidPrimitiveType_DgnCone)
            return ConeToJson (in, solid);
        if (type == SolidPrimitiveType_DgnExtrusion)
            return LinearSweepToJson (in, solid);
        if (type == SolidPrimitiveType_DgnRotationalSweep)
            return RotationalSweepToJson (in, solid);
        if (type == SolidPrimitiveType_DgnRuledSweep)
            return RuledSweepToJson (in, solid);
        }

void PolyfaceToJson (BeJsValue in, PolyfaceHeaderCR mesh)
        {
        auto style = mesh.GetMeshStyle ();
        if (style == MESH_ELM_STYLE_INDEXED_FACE_LOOPS)// && mesh.GetNumPerFace () < 2)
                    // typescript side does not support blocked meshes.
                    // but it is happy to read the IMJS and add the terminators.
            return IndexedPolyfaceToJson (in, mesh);

        auto indexedMesh = mesh.CloneAsVariableSizeIndexed(mesh);
        IndexedPolyfaceToJson(in, *indexedMesh);
        }


    void GeometryToJson (BeJsValue in, IGeometryCR g)
        {
        ICurvePrimitivePtr cp = g.GetAsICurvePrimitive ();
        if (cp.IsValid ())
            return CurvePrimitiveToJson (in, *cp);
        CurveVectorPtr cv = g.GetAsCurveVector ();
        if (cv.IsValid ())
            return CurveVectorToJson (in, *cv);
        MSBsplineSurfacePtr bsurf = g.GetAsMSBsplineSurface ();
        if (bsurf.IsValid ())
            return BsurfToJson (in, *bsurf);
        ISolidPrimitivePtr sp = g.GetAsISolidPrimitive();
        if (sp.IsValid ())
            return SolidPrimitiveToJson (in, *sp);

        PolyfaceHeaderPtr pf = g.GetAsPolyfaceHeader();
        if (pf.IsValid ())
            {
            return PolyfaceToJson(in, *pf);
            }
        }
    // insert a (single) TaggedNumericData in the json object.
    void ToJson(BeJsValue obj, TaggedNumericData const &data)
        {
        obj["tagA"] = data.m_tagA;
        obj["tagB"] = data.m_tagB;
        BeCGIModelJsonValueWriter builder;

        if (data.m_intData.size() > 0)
            {
            auto dest = obj["intData"];
            for (auto value : data.m_intData)
                dest.appendValue() = value;
            }
        if (data.m_doubleData.size() > 0)
            {
            auto dest = obj["doubleData"];
            for (auto value : data.m_doubleData)
                dest.appendValue() = value;
            }
        }
    // append array of TaggedNumericData to the json object.
    void ToJson(BeJsValue dest, bvector<TaggedNumericData> const &data)
        {
        for (auto const &d : data)
            {
            ToJson (dest.appendValue (), d);
            }
        }

    public: BeCGIModelJsonValueWriter ()
        {
        m_packArrays = true;
        m_packIsolatedPoints = true;
        }

};

bool IModelJson::TryGeometryToIModelJsonValue
(
BeJsValue jsonArray,
bvector<IGeometryPtr> const &data,
bvector<IGeometryPtr> *validGeometry,
bvector<IGeometryPtr> *invalidGeometry
)
    {
    if (validGeometry)
        validGeometry->clear ();

    if (invalidGeometry)
        invalidGeometry->clear ();

    BeCGIModelJsonValueWriter builder;
    for (auto &g : data)
        {
        IGeometryPtr g1 = g;
        if (GeometryValidator::ValidateAndAppend (s_writeValidator,
            g1, //static_cast<IGeometryPtr&> (g),
            validGeometry, invalidGeometry))
            builder.GeometryToJson(jsonArray.appendValue(), *g);
        }
    return !jsonArray.isNull ();
    }

bool IModelJson::TryGeometryToIModelJsonValue (BeJsValue value, IGeometryCR data)
    {
    BeCGIModelJsonValueWriter builder;
    builder.GeometryToJson (value, data);
    return !value.isNull ();
    }

bool IModelJson::TryGeometryToIModelJsonString (Utf8StringR string, IGeometryCR data)
    {
    BeJsDocument value;
    if (TryGeometryToIModelJsonValue(value, data))
        {
        string = value.Stringify();
        return true;
        }
    return false;
    }

bool IModelJson::TryGeometryToIModelJsonString
(
Utf8StringR string,
bvector<IGeometryPtr> const &data,
bvector<IGeometryPtr> *validGeometry,
bvector<IGeometryPtr> *invalidGeometry
)
    {
    rapidjson::Document doc;
    BeJsValue value (doc);
    if (TryGeometryToIModelJsonValue(value, data, validGeometry, invalidGeometry))
        {
        string = value.Stringify ();
        return true;
        }
    return false;
    }

/**
* <ul>
* <li> m_tag is always output as "tag"
* <li> m_intValue and m_doubleValue are output as "iValue" and "dValue" if nonzero
* <li> m_xyz is output as "xyz" if any of its components is nonzero.
* <li> m_vectors is output as 0,1,2, or 3 [x,y,z] values, with trailing zero vectors omitted.
* <li>
* </ul>
*/
void IModelJson::TaggedNumericDataToJson(BeJsValue obj, TaggedNumericData const &data)
    {
    BeCGIModelJsonValueWriter builder;
    builder.ToJson (obj, data);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
