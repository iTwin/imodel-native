/*--------------------------------------------------------------------------------------+
|
|  $Source: serialization/src/IModelJsonReader.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "serializationPCH.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include "BeCGWriter.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//=======================================================================================
//
// </ul>
// @bsiclass                                                    Earlin.Lutz 3/18
//=======================================================================================
struct BeCGIModelJsonValueReader
    {
private:
// Return true if value is an entirely numeric array.
// Also transfer up to numNeeded double to values[].  Fill tail of values[] with 0.
bool accessNumericArray (JsonValueCR value, size_t numNeeded, double values[])
    {
    // Json::FastWriter fastWriter;
    // auto string = fastWriter.write(value);
    if (!value.isArray ())
        return false;
    uint32_t numOut = 0;
    for (uint32_t i = 0, n = value.size (); i < n; i++)
        {
        if (!value[i].isNumeric ())
            return false;
        if (numOut < numNeeded)
            values[numOut++] = value[i].asDouble ();
        }
    for (uint32_t i = numOut; i < numNeeded; i++)
        values[i] = 0;
    return true;
    }
bool tryValueToDPoint3d (JsonValueCR value, DPoint3dR xyz)
    {
    double xyzArray[3];
    if (!accessNumericArray (value, 3, xyzArray))
        return false;
    xyz.Init (xyzArray[0], xyzArray[1], xyzArray[2]);
    return true;
    }
bool tryValueToBVectorDPoint3d (JsonValueCR value, bvector<DPoint3d> &data)
    {
    data.clear ();
    double xyzArray[3];
    if (value.isArray ())
        {
        for (uint32_t i = 0; i < value.size (); i++)
            {
            if (!accessNumericArray (value[i], 3, xyzArray))
                return false;
            data.push_back (DPoint3d::FromArray (xyzArray));
            }
        return true;
        }
    return true;
    }

bool tryValueToBVectorDouble (JsonValueCR value, bvector<double> &data)
    {
    data.clear ();
    if (value.isArray ())
        {
        for (uint32_t i = 0; i < value.size (); i++)
            {
            if (!value[i].isNumeric ())
                return false;
            data.push_back (value[i].asDouble ());
            }
        return true;
        }
    return true;
    }
bool tryValueToLineSegment (JsonValueCR value, ICurvePrimitivePtr &result)
    {
    if (value.isNull ())
        return false;
    if (value.isArray () && value.size () > 1)
        {
        DSegment3d segment;
        if (   tryValueToDPoint3d (value[0], segment.point[0])
            && tryValueToDPoint3d (value[1], segment.point[1]))
            {
            result = ICurvePrimitive::CreateLine (segment);
            return true;
            }
        }
    return false;
    }

bool tryValueToLineString (JsonValueCR value, ICurvePrimitivePtr &result)
    {
    if (value.isNull ())
        return false;
    if (value.isArray () && value.size () > 1)
        {
        auto ls = ICurvePrimitive::CreateLineString (nullptr, 0);
        for (uint32_t i = 0; i < value.size (); i++)
            {
            DPoint3d xyz;
            if (!tryValueToDPoint3d (value[i], xyz))
                return false;
            ls->TryAddLineStringPoint (xyz);
            }
        result = ls;
        return true;
        }
    return false;
    }

bool tryValueToBsplineCurve (JsonValueCR value, ICurvePrimitivePtr &result)
    {
    if (!value.isNull ())
        {
        bvector<DPoint3d> poles;
        bvector<double> knots;
        if (tryValueToBVectorDPoint3d (value["points"], poles)
            && value["order"].isIntegral ()
            && tryValueToBVectorDouble (value["knots"], knots)
            )
            {
            auto order = value["order"].asInt ();
            bool closed  = value["closed"].isNull () ? false : value["closed"].asBool ();
            auto bcurve = MSBsplineCurve::CreateFromPolesAndOrder (
                    poles, nullptr, &knots,
                    (int)order, closed);
            if (bcurve.IsValid ())
                {
                result = ICurvePrimitive::CreateBsplineCurve (bcurve);
                return true;
                }
            }
        }
    return false;
    }

bool tryValueToArc (JsonValueCR value, ICurvePrimitivePtr &result)
    {
    if (!value.isNull ())
        {
        DEllipse3d arc;
        DPoint3d sweepPoint;
        if (   tryValueToDPoint3d (value["center"], arc.center)
            && tryValueToDPoint3d (value["vectorX"], arc.vector0)   // treat xyz as vector
            && tryValueToDPoint3d (value["vectorY"], arc.vector90)  // treat xyz as vector
            && tryValueToDPoint3d (value["sweep"], sweepPoint))
            {
            arc.start = Angle::DegreesToRadians (sweepPoint.x);
            arc.sweep = Angle::DegreesToRadians (sweepPoint.y - sweepPoint.x);
            result = ICurvePrimitive::CreateArc (arc);
            return true;
            }
        }
    return false;
    }

bool tryArrayToCurveVectorMembers
(
JsonValueCR value,
CurveVectorPtr &result,
CurveVector::BoundaryType boundaryType
)
    {
    result = nullptr;
    if (value.isNull ())
        return false;
    if (value.isArray ())
        {
        result = CurveVector::Create (boundaryType);
        for (uint32_t i = 0; i < value.size (); i++)
            {
            auto cp = tryValueToCurvePrimitive (value[i]);
            if (cp.IsValid ())
                {
                result->push_back (cp);
                continue;
                }
            auto cv1 = tryValueToCurveVector (value[i]);
            if (cv1.IsValid ())
                {
                result->Add (cv1);
                continue;
                }
            }
        return true;
        }
    return false;
    }

CurveVectorPtr tryValueToCurveVector (JsonValueCR value)
    {
    CurveVectorPtr cv;
    if (tryArrayToCurveVectorMembers (value["path"], cv, CurveVector::BOUNDARY_TYPE_Open))
        return cv;
    if (tryArrayToCurveVectorMembers (value["loop"], cv, CurveVector::BOUNDARY_TYPE_Outer))
        return cv;
    if (tryArrayToCurveVectorMembers (value["parityRegion"], cv, CurveVector::BOUNDARY_TYPE_ParityRegion))
        return cv;
    if (tryArrayToCurveVectorMembers (value["unionRegion"], cv, CurveVector::BOUNDARY_TYPE_UnionRegion))
        return cv;
    if (tryArrayToCurveVectorMembers (value["bagOfCurves"], cv, CurveVector::BOUNDARY_TYPE_None))
        return cv;
    return nullptr;
    }

ICurvePrimitivePtr tryValueToCurvePrimitive (JsonValueCR value)
    {
    ICurvePrimitivePtr cp;
    if (tryValueToLineSegment (value["lineSegment"], cp))
        return cp;
    if (tryValueToLineString (value["lineString"], cp))
        return cp;
    if (tryValueToBsplineCurve (value["bcurve"], cp))
        return cp;
    if (tryValueToArc (value["arc"], cp))
        return cp;
    return nullptr;
    }
IGeometryPtr tryValueToIGeometry (JsonValueCR value)
    {
    if (value.isObject ())
        {
        ICurvePrimitivePtr cp = tryValueToCurvePrimitive (value);
        if (cp.IsValid ())
            return IGeometry::Create (cp);
        CurveVectorPtr cv = tryValueToCurveVector (value);
        if (cv.IsValid ())
            return IGeometry::Create (cv);
        }
    return nullptr;
    }



public: bool TryParse (JsonValueCR source, bvector<IGeometryPtr> &geometry)
    {
    IGeometryPtr result;
    if (source.isObject ())
        {
        auto result = tryValueToIGeometry (source);
        if (result.IsValid ())
            geometry.push_back (result);
        }
    else if (source.isArray ())
        {
        // just recuse to next level -- deep array structure is flattened in the output.
        int n = source.size ();
        for (int i = 0; i < n; i++)
            {
            auto member = tryValueToIGeometry (source[i]);
            if (member.IsValid ())
                geometry.push_back (member);    
            }
        }
    return geometry.size () > 0;
    } 
};

bool IModelJson::TryIModelJsonValueToGeometry (JsonValueCR value, bvector<IGeometryPtr> &geometry)
    {
    BeCGIModelJsonValueReader reader;
    reader.TryParse (value, geometry);
    return false;
    }

bool IModelJson::TryIModelJsonStringToGeometry (Utf8StringCR string, bvector<IGeometryPtr> &geometry)
    {
    return false;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
