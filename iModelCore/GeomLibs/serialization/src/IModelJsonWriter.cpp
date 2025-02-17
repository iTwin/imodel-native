/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "serializationPCH.h"
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
    void AppendIfNotNull (BeJsValue jsonArray, BeJsConst candidate)
        {
        if (!candidate.isNull())
            jsonArray.appendValue().From(candidate);
        }

    void ToJson (BeJsValue result, DPoint3dCR point, bool compact)
        {
        if (compact)
            {
            result.appendValue() = point.x;
            result.appendValue() = point.y;
            // we could compress even more by skipping zero z, but this would break older readers that expect
            // all three coords (e.g. BeCGIModelJsonValueReader::tryValueGridToBVectorDPoint3d, now fixed)
            result.appendValue() = point.z;
            return;
            }
        result["x"] = point.x;
        result["y"] = point.y;
        result["z"] = point.z;
        }

    void ToJson(BeJsValue result, DPoint4dCR point, bool compact)
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

    void ToJson(BeJsValue result, DPoint2dCR point, bool compact)
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

    void ToJson(BeJsValue result, DPoint3dCR point, double weight, bool compact)
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

    void ToJson(BeJsValue jsonArray, bvector<DPoint3d> const &points)
        {
        for (auto &xyz : points)
            ToJson (jsonArray.appendValue(), xyz, m_packArrays);
        }


    void ToJson(BeJsValue jsonArray, bvector<DPoint4d> const &points)
        {
        for (auto &xyz : points)
            ToJson (jsonArray.appendValue(), xyz, m_packArrays);
        }

    void ToJson(BeJsValue jsonArray, DPoint3dCP points, size_t n)
        {
        for (auto i = 0; i < n; i++)
             ToJson (jsonArray.appendValue(), points[i], m_packArrays);
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
            ToJson(jsonArray.appendValue(), doubles, i);
            }
        }

    void ToJson(BeJsValue jsonArray, DPoint2dCP points, size_t n)
        {
        for (auto i = 0; i < n; i++)
            ToJson (jsonArray.appendValue(), points[i], m_packArrays);
        }
    void ToJson(BeJsValue jsonArray, bvector<int> const &data)
        {
        for (auto &a : data)
            jsonArray.appendValue() = a;
        }

    void ToJson(BeJsValue jsonArray, uint32_t const *data,  size_t n)
        {
        for (size_t i = 0; i < n; i++)
            jsonArray.appendValue() = data[i];
        }

    void ToJson(BeJsValue jsonArray, int32_t const *data,  size_t n)
        {
        for (size_t i = 0; i < n; i++)
            jsonArray.appendValue()= data[i];
        }

    void ToJson(BeJsValue jsonArray, double const *data, size_t n)
        {
        for (size_t i = 0; i < n; i++)
            jsonArray.appendValue() = data[i];
        }

    void ToJson(BeJsValue jsonArray, bvector<double> const &data)
        {
        for (auto &a : data)
            jsonArray.appendValue() = a;
        }

    void CreateSweepStartSweepRadiansToStartEndDegrees (BeJsValue result, double radians0, double sweepRadians)
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
        ToJson (value["uKnots"], knots);
        bsurf.GetVKnots (knots);
        ToJson (value["vKnots"], knots);
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
                    ToJson (row.appendValue(), bsurf.GetPole (i, j), bsurf.GetWeight(i,j), this->m_packArrays);
                else
                    ToJson (row.appendValue(), bsurf.GetPole (i, j), this->m_packArrays);
                }
            }
        CurveVectorPtr boundaries = bsurf.GetUVBoundaryCurves(false, true);     // preserve trim curves (don't stroke)
        if (boundaries.IsValid() && boundaries->size() > 0)
            {
            CurveVectorToJson(value["uvBoundaries"], *boundaries);
            if (!bsurf.IsOuterBoundaryActive())
                value["outerBoundaryActive"] = false;   // default/undefined is true (boundary forms a hole)
            }
        }

    /* RawImjs
    {"torusPipe":
     {"center":[1,50,3],
      "majorRadius":10,
      "minorRadius":1,
      "xyVectors":[[0.6989117773223881,-0.3136225207688945,0.6427777547384671],[0.10329524078492897,0.9335692701842065,0.34318873961555296]],
      "sweepAngle":45,
      "capped":true
     }
    }
    */
    void TorusPipeToJson (BeJsValue in, ISolidPrimitiveCR sp)
        {
        auto value = in["torusPipe"];
        DgnTorusPipeDetail detail;
        Transform localToWorld, worldToLocal;
        if (sp.TryGetDgnTorusPipeDetail (detail)
            && detail.TryGetConstructiveFrame (localToWorld, worldToLocal))
            {
            ToJson(value["center"], detail.m_center, m_packIsolatedPoints);
            value["majorRadius"] = detail.m_majorRadius;
            value["minorRadius"] = detail.m_minorRadius;
            if (!Angle::IsFullCircle (detail.m_sweepAngle))
                value["sweepAngle"] = Angle::RadiansToDegrees (detail.m_sweepAngle);
            if (detail.m_capped)
                value["capped"] = detail.m_capped;
            auto xyVectors = value["xyVectors"];
            ToJson(xyVectors.appendValue(), localToWorld.GetMatrixColumn (0), m_packIsolatedPoints);
            ToJson(xyVectors.appendValue(), localToWorld.GetMatrixColumn (1), m_packIsolatedPoints);
            }
        }

    /* RawImjs
    {"box":
     {"baseOrigin":[1,2,3],
      "origin":[1,2,3],
      "baseX":3,
      "baseY":2,
      "capped":true,
      "topOrigin":[1,2,8],
      "xyVectors":[[0.984807753012208,0.17364817766693033,0],[-0.17364817766693033,0.984807753012208,0]],
      "topX":1.5,
      "topY":1
     }
    }
    */
    void BoxToJson (BeJsValue in, ISolidPrimitiveCR sp)
        {
        auto value = in["box"];
        DgnBoxDetail detail;
        Transform localToWorld, worldToLocal;
        if (sp.TryGetDgnBoxDetail (detail)
            && detail.TryGetConstructiveFrame (localToWorld, worldToLocal))
            {
            ToJson (value["baseOrigin"], detail.m_baseOrigin, m_packIsolatedPoints);
            ToJson (value["origin"], detail.m_baseOrigin, m_packIsolatedPoints);   // both baseOrigin and origin are output, but origin is preferred by reader
            ToJson (value["topOrigin"], detail.m_topOrigin, m_packIsolatedPoints);
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
                ToJson (xyVectors.appendValue(), localToWorld.GetMatrixColumn (0), m_packIsolatedPoints);
                ToJson (xyVectors.appendValue(), localToWorld.GetMatrixColumn (1), m_packIsolatedPoints);
                }
            }
        }

    /* RawImjs
    {"sphere":
     {"center":[1,2,3],
      "zxVectors":[[0,1,0],[0,0,1]],
      "capped":true,
      "radius":4,
      "latitudeStartEnd":[-45,45]
     }
    }
    */
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
            ToJson (value["center"], center, m_packIsolatedPoints);
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
                ToJson (zxVectors.appendValue(),unitZ, m_packIsolatedPoints);
                ToJson (zxVectors.appendValue(),unitX, m_packIsolatedPoints);
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

    /* RawImjs
    {"cylinder":
     {"capped":false,
      "start":[1,2,1],
      "end":[2,3,8],
      "radius":0.5
     }
    },
    {"cone":
     {"capped":true,
      "start":[0,0,0],
      "end":[0,0,5],
      "startRadius":1,
      "endRadius":0.2,
      "xyVectors":[[1,0,0], [0,1,0]]
     }
    }
    */
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
                ToJson (value["start"], centerA, m_packIsolatedPoints);
                ToJson (value["end"], centerB, m_packIsolatedPoints);
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
                ToJson (value["start"], centerA, m_packIsolatedPoints);
                ToJson (value["end"], centerB, m_packIsolatedPoints);
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
                    ToJson (xyVectors.appendValue(), unitX, m_packIsolatedPoints);
                    ToJson (xyVectors.appendValue(), unitY, m_packIsolatedPoints);
                    }
                }
            else
                {
                // ?? elliptic section at base ??
                }
            }
        }

    /* RawImjs
    {"ruledSweep":
     {"contour":[
      {"path":<pathContents>}
      {"path":<pathContents>}
      ],
      "capped":false
     }
    }
    */
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

    /* RawImjs
    {"linearSweep":
     {"contour":{<CurveVector>},
      "capped":false,
      "vector":[0,0,1.234]
     }
    }
    */
    void LinearSweepToJson (BeJsValue in, ISolidPrimitiveCR sp)
        {
        DgnExtrusionDetail detail;
        if (sp.TryGetDgnExtrusionDetail (detail))
            {
            auto value = in["linearSweep"];
            value["capped"] = detail.m_capped;
            CurveVectorToJson (value["contour"], *detail.m_baseCurve);
            ToJson (value["vector"], detail.m_extrusionVector, m_packIsolatedPoints);
            }
        }

    /* RawImjs
    {"rotationalSweep":
     {"axis":[0,1,0],
      "contour":{<CurveVector>},
      "capped":false,
      "center":[0,0,0],
      "sweepAngle":119.99999999999999
     }
    }
    */
    void RotationalSweepToJson (BeJsValue in, ISolidPrimitiveCR sp)
        {
        DgnRotationalSweepDetail detail;
        if (sp.TryGetDgnRotationalSweepDetail (detail))
            {
            auto value = in["rotationalSweep"];
            CurveVectorToJson (value["contour"], *detail.m_baseCurve);
            value["capped"] = detail.m_capped;
            ToJson (value["center"], detail.m_axisOfRotation.origin, m_packIsolatedPoints);
            ToJson (value["axis"], detail.m_axisOfRotation.direction, m_packIsolatedPoints);
            value["sweepAngle"] = Angle::RadiansToDegrees (detail.m_sweepAngle);
            }
        }

    /*--------------------------------------------------------------------------------**//**
    * @bsimethod
    +--------------------------------------------------------------------------------------*/
    void ToJson(BeJsValue value, PolyfaceAuxChannel::DataCR in)
        {
        value["input"] = in.GetInput();

        auto dataValues = value["values"];
        for (auto& dataValue : in.GetValues())
            dataValues.appendValue() = dataValue;
        }

    /*--------------------------------------------------------------------------------**//**
    * @bsimethod
    +--------------------------------------------------------------------------------------*/
    void ToJson(BeJsValue value, PolyfaceAuxChannelCR in)
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
    void ToJson(BeJsValue value, PolyfaceAuxData::ChannelsCR in)
        {
        for (auto& channel : in)
            ToJson(value.appendValue(), *channel);
        }

    /*--------------------------------------------------------------------------------**//**
    * @bsimethod
    +--------------------------------------------------------------------------------------*/
    void ToJson(BeJsValue value, PolyfaceAuxDataCR in)
        {
        ToJson(value["indices"], in.GetIndices().data(), in.GetIndices().size());
        ToJson(value["channels"], in.GetChannels());
        }

    /* RawImjs
    {"indexedMesh":
     {"color":[10,11,12,13,....,83,84,85,86,87,88,89],
      "colorIndex":[1,1,1,0,2,2,2,0,3,3,3,0,......80,80,80,0],
      "point":[[0,0,0], [1,0,0],[2,0,0],  [3,0,0],  [4,0,0],   . . . .   [6,5,0],  [7,5,0],  [8,5,0]],
      "pointIndex":[  1,2,-11,0,11,10,-1,0,2,3,-12,0,   . . .32,33,-42,0,42,41,-32,0],
      "paramIndex":[..],
      "param":[[0,0],[1,0],...],
      "normalIndex":[..],
      "normal":[[1,0,0],[0,1,0],..],
     }
    }
    */
    void IndexedPolyfaceToJson(BeJsValue in, PolyfaceHeaderCR mesh)
        {
        if (mesh.GetMeshStyle () != MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
            return;

        auto allData = in["indexedMesh"];

        // these are the only required fields
        auto indexCount = mesh.GetPointIndexCount ();
        ToJson (allData["point"], mesh.GetPointCP (), mesh.GetPointCount ());
        ToJson (allData["pointIndex"], mesh.GetPointIndexCP (), indexCount);

        if (mesh.GetNumPerFace() > 2)
            allData["numPerFace"] = mesh.GetNumPerFace();
        if (mesh.GetTwoSided())
            allData["twoSided"] = true;
        if (auto expectedClosure = mesh.GetExpectedClosure())
            allData["expectedClosure"] = expectedClosure;

        if (mesh.GetColorCount() > 0)
            ToJson (allData["color"], mesh.GetIntColorCP (), mesh.GetColorCount ());
        if (mesh.GetColorIndexCP() != nullptr)
            ToJson (allData["colorIndex"], mesh.GetColorIndexCP (), indexCount);

        if (mesh.GetParamCount() > 0)
            ToJson (allData["param"], mesh.GetParamCP (), mesh.GetParamCount ());
        if (mesh.GetParamIndexCP() != nullptr)
            ToJson (allData["paramIndex"], mesh.GetParamIndexCP (), indexCount);

        if (mesh.GetNormalCount() > 0)
            ToJson (allData["normal"], mesh.GetNormalCP (), mesh.GetNormalCount ());
        if (mesh.GetNormalIndexCP() != nullptr)
            ToJson (allData["normalIndex"], mesh.GetNormalIndexCP (), indexCount);

        if (mesh.GetFaceDataCP() != nullptr && mesh.GetFaceCount () > 0)
            FaceDataToJson (allData["faceData"], mesh.GetFaceDataCP (), mesh.GetFaceCount ());
        if (mesh.GetFaceIndexCP() != nullptr)
            ToJson (allData["faceIndex"], mesh.GetFaceIndexCP (), indexCount);

        auto taggedData = mesh.GetNumericTagsCP();
        if (taggedData && !taggedData->IsZero())
            ToJson(allData["tags"], *taggedData);

        if(mesh.GetAuxDataCP().IsValid())
            ToJson(allData["auxData"], *mesh.GetAuxDataCP());

        // TODO: currently edgeMateIndex array is ignored for native Polyface
        }

    void CurvePrimitiveToJson(BeJsValue in, ICurvePrimitiveCR cp)
        {
        DSegment3d segment;
        DEllipse3d arc;
        // RawIMJS {"lineSegment":[[0,0,0], [3,3,0]]}
        if (cp.TryGetLine (segment))
            {
            auto value = in["lineSegment"];
            ToJson (value.appendValue(), segment.point[0], m_packIsolatedPoints);
            ToJson (value.appendValue(), segment.point[1], m_packIsolatedPoints);
            return;
            }
        /* RawIMJS
        {"arc":
         {"center":[0,0,0],
          "vectorX":[3,0,0],
          "vectorY":[0,3,0],
          "sweepStartEnd":[-40,270]
         }
        }
        */
        if (cp.TryGetArc (arc))
            {
            auto  value = in["arc"];
            ToJson (value["center"], arc.center, m_packIsolatedPoints);
            ToJson (value["vectorX"], arc.vector0, m_packIsolatedPoints);
            ToJson (value["vectorY"], arc.vector90, m_packIsolatedPoints);
            CreateSweepStartSweepRadiansToStartEndDegrees (value["sweepStartEnd"], arc.start, arc.sweep);
            return;
            }
        bvector<DPoint3d> const *points = cp.GetLineStringCP ();
        // RawIMJS {"lineString":[[0,0,0], [1,0,0],  [1,1,0]]},
        if (points != nullptr)
            return ToJson(in["lineString"], *points);

        points = cp.GetPointStringCP ();
        // RawIMJS {"pointString":[[0,0,0], [1,0,0],  [1,1,0]]},
        if (points != nullptr)
            return ToJson (in["pointString"], *points);

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
                ToJson (value["points"], poles4d);
                }
            else
                {
                bvector<DPoint3d> poles;
                bcurve->GetPoles (poles);
                ToJson (value["points"], poles);
                }
            value["order"] = (int) bcurve->GetOrder();
            ToJson (value["knots"], knots);
            if (bcurve->IsClosed())
                value["closed"] = true;
            return;
            }

        auto spiralPlacement = cp.GetSpiralPlacementCP();
        /* RawIMJS
        {"transitionSpiral":
         {"activeFractionInterval":[0.0,1.0059395557393587],
          "origin":[0,0,0],
          "type":"clothoid",
          "startRadius":0,
          "endRadius":1000,
          "startBearing":0,
          "endBearing":2.8647889756541165
         }
        }
        */
        if (spiralPlacement != nullptr)
            {
            DSpiral2dDirectEvaluation const * nominalLengthSpiral = dynamic_cast <DSpiral2dDirectEvaluation const*> (spiralPlacement->spiral);

            auto value = in["transitionSpiral"];
            Transform frame = spiralPlacement->frame;
            DVec3d unitX, unitY, unitZ;
            DPoint3d origin;
            frame.GetOriginAndVectors(origin, unitX, unitY, unitZ);
            ToJson(value["origin"], origin, m_packIsolatedPoints);

            Utf8String typeName;
            if (DSpiral2dBase::TransitionTypeToString (spiralPlacement->spiral->GetTransitionTypeCode (), typeName))
                value["type"] = typeName;
            else
                value["type"] = "unknown";
            if (!frame.Matrix().IsIdentity())
                {
                auto xyVectors = value["xyVectors"];
                ToJson(xyVectors.appendValue(), unitX, m_packIsolatedPoints);
                ToJson(xyVectors.appendValue(), unitY, m_packIsolatedPoints);
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
            auto value = in["interpolationCurve"];
            ToJson(value["fitPoints"], interpolationCurve->fitPoints, interpolationCurve->params.numPoints);
            if (interpolationCurve->knots != nullptr && interpolationCurve->params.numKnots > 0)
                {
                ToJson(value["knots"], interpolationCurve->knots, interpolationCurve->params.numKnots);
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
                ToJson(value["startTangent"], interpolationCurve->startTangent, m_packIsolatedPoints);
            if (interpolationCurve->endTangent.Magnitude() != 0)
                ToJson(value["endTangent"], interpolationCurve->endTangent, m_packIsolatedPoints);
            }
        }

    void CurveVectorToJson(BeJsValue in, CurveVectorCR cv)
        {
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

        CurveVectorPtr flattened;
        CurveVectorCP pCurves = &cv;
        if (cv.HasNestedUnionRegion())
            {
            // requirement for PowerPlatform and iModel
            flattened = cv.Clone();
            flattened->FlattenNestedUnionRegions();
            if (flattened.IsValid())
                pCurves = flattened.get();
            }

        auto head = in[name];
        for (auto& cp : *pCurves)
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
        if (style == MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
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
    for (auto const& g : data)
        {
        IGeometryPtr g1 = g;    // cast away const
        if (GeometryValidator::ValidateAndAppend (s_writeValidator, g1, validGeometry, invalidGeometry))
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
    BeJsDocument value;
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
