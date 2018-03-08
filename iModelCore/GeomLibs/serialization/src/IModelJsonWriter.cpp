/*--------------------------------------------------------------------------------------+
|
|  $Source: serialization/src/IModelJsonWriter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

    Json::Value toJson (DPoint3dCR point)
        {
        auto result = Json::Value();
        result.append (Json::Value(point.x));
        result.append (Json::Value(point.y));
        result.append (Json::Value(point.z));
        return result;
        }

    Json::Value toJson(bvector<DPoint3d> const &points)
        {
        Json::Value jsonArray;
        for (auto &xyz : points)
            jsonArray.append (toJson (xyz));
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

    Json::Value createCurvePrimitive (ICurvePrimitiveCR cp)
        {
        DSegment3d segment;
        DEllipse3d arc;
        if (cp.TryGetLine (segment))
            {
            Json::Value value;
            value.append (toJson (segment.point[0]));
            value.append (toJson (segment.point[1]));
            auto result = Json::Value ();
            return singleton ("lineSegment", value);
            }

        if (cp.TryGetArc (arc))
            {
            Json::Value value;
            value["center"] = toJson (arc.center);
            value["vectorX"] = toJson (arc.vector0);
            value["vectorY"] = toJson (arc.vector90);
            value["sweep"] = createSweepStartSweepRadiansToStartEndDegrees (arc.start, arc.sweep);
            return singleton ("arc", value);
            }
        bvector<DPoint3d> const *points = cp.GetLineStringCP ();
        if (points != nullptr)
            return singleton ("lineString", toJson (*points));

        auto child = cp.GetChildCurveVectorCP ();
        if (child != nullptr)
            return createCurveVector (*child);

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
            return singleton ("bcurve", value);
            }
        return Json::Value ();
        }

    Json::Value createCurveVector (CurveVectorCR cv)
        {
        auto type = cv.GetBoundaryType ();
        Json::Value children;
        for (auto &cp : cv)
            appendIfNotNull (children, createCurvePrimitive (*cp));
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


    Json::Value createGeometry (IGeometryCR g)
        {
        ICurvePrimitivePtr cp = g.GetAsICurvePrimitive ();
        if (cp.IsValid ())
            return createCurvePrimitive (*cp);
        CurveVectorPtr cv = g.GetAsCurveVector ();
        if (cv.IsValid ())
            return createCurveVector (*cv);

        return Json::Value ();
        }

    public: BeCGIModelJsonValueWriter ()
        {
        }

};

bool IModelJson::TryGeometryToIModelJsonValue(Json::Value &jsonArray, bvector<IGeometryPtr> const &data)
    {
    BeCGIModelJsonValueWriter builder;
    jsonArray = Json::Value ();
    for (auto &g : data)
        builder.appendIfNotNull (jsonArray, builder.createGeometry (*g));
    return !jsonArray.isNull ();
    }

bool IModelJson::TryGeometryToIModelJsonValue (Json::Value &value, IGeometryCR data)
    {
    BeCGIModelJsonValueWriter builder;
    value = builder.createGeometry (data);
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
