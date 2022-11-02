/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "serializationPCH.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include "BeCGWriter.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct BeCGIModelJsonValueReader
    {
private:
    Transform m_transform;
public:
    BeCGIModelJsonValueReader ()
        {
        m_transform = Transform::FromIdentity ();
        }
private:
bool derefAsString(BeJsConst source, CharCP name, Utf8StringR value)
        {
        auto target = source[name];
        if (target.isString())
            {
            value = target.asString();
            return true;
            }
        return false;
        }

private:
// Return true if value is an entirely numeric array.
// Also transfer up to maxNeeded double to values[].  Fill tail of values[] with 0.
// Return true if at least minNeeded were received.
bool derefNumericArray (BeJsConst value, size_t minNeeded, size_t maxNeeded, double values[])
    {
    if (!value.isArray())
        return false;
    uint32_t numOut = 0;
    for (uint32_t i = 0, n = value.size (); i < n; i++)
        {
        if (value[i].isNull())
            return false;
        if (!value[i].isNumeric ())
            return false;
        if (numOut < maxNeeded)
            values[numOut++] = value[i].asDouble ();
        }
    for (uint32_t i = numOut; i < maxNeeded; i++)
        values[i] = 0;
    return numOut >= minNeeded;
    }
// look for a named property to pass to derefNumericArray
bool derefNumericArray(BeJsConst source, char const *name, size_t minNeeded, size_t maxNeeded, double values[])
    {
    auto propertyValue = source[name];
    if (propertyValue.isNull())
        return false;
    return derefNumericArray (propertyValue, minNeeded, maxNeeded, values);
    }

// number ==> degrees
// {degrees: number}
// {radians: number}
bool tryValueToAngle (BeJsConst value, AngleR &angle, AngleCR defaultAngle = Angle::FromRadians (0))
    {
    angle = defaultAngle;
    if (value.isNull ())
        return false;

    if (value.isNumeric ())
        {
        angle = Angle::FromDegrees (value.asDouble ());
        return true;
        }
    double a;
    if (derefNumeric (value, "degrees", a))
        {
        angle = Angle::FromDegrees (a);
        return true;
        }

    if (derefNumeric (value, "radians", a))
        {
        angle = Angle::FromRadians (a);
        return true;
        }

    return false;
    }

// "latitudeStartEnd":[<angle>,<angle>]
bool derefLatitudeStartSweepRadians (BeJsConst value, ValidatedDouble &startRadians, ValidatedDouble &sweepRadians)
    {

    startRadians = ValidatedDouble (-msGeomConst_piOver2, false);
    sweepRadians = ValidatedDouble (msGeomConst_pi, false);
    auto startEndValue = value["latitudeStartEnd"];
    if (value.isNull ())
        return false;
    Angle startAngle, endAngle;
    if (startEndValue.isArray() && startEndValue.size () == 2
        && tryValueToAngle (startEndValue[0], startAngle)
        && tryValueToAngle (startEndValue[1], endAngle))
        {
        startRadians = ValidatedDouble (startAngle.Radians (), true);
        sweepRadians = ValidatedDouble (endAngle.Radians () - startAngle.Radians (), true);
        return true;
        }
    return false;
    }

bool derefNumeric (BeJsConst source, char const *name, double &value, double defaultValue = 0.0)
    {
    value = defaultValue;
    auto jsonValue = source[name];
    if (jsonValue.isNull())
        return false;
    if (jsonValue.isNumeric ())
        {
        value = jsonValue.asDouble ();
        return true;
        }
    return false;
    }

bool derefAngle(BeJsConst source, char const *name, Angle &value, Angle defaultValue = Angle::FromDegrees (0.0))
    {
    value = defaultValue;
    auto jsonValue = source[name];
    if (jsonValue.isNull())
        return false;
    return tryValueToAngle(jsonValue, value, defaultValue);
    return false;
    }

ValidatedDouble derefValidatedDouble (BeJsConst source, char const *name, ValidatedDouble const &defaultValue)
    {
    // unused - auto value = defaultValue;
    auto jsonValue = source[name];
    if (jsonValue.isNumeric ())
        return ValidatedDouble (jsonValue.asDouble (), true);
    return defaultValue;
    }

bool derefBool (BeJsConst source, char const *name, bool &value, bool defaultValue = false)
    {
    value = defaultValue;
    auto jsonValue = source[name];
    if (jsonValue.isBool ())
        {
        value = jsonValue.asBool ();
        return true;
        }
    return false;
    }

// REMARK: The return value is typed as DPoint3d, but since DVec3d has DPoint3d as base class this
// method can be called with DVec3d as the xyz value.
bool tryValueToXYZ (BeJsConst value, DPoint3dR xyz)
    {
    double xyzArray[3];
    bool stat = false;
    if (derefNumericArray (value, 2, 3, xyzArray))  // allow optional z
        {
        xyz.Init (xyzArray[0], xyzArray[1], xyzArray[2]);
        stat = true;
        }
    else
        {
        bool haveX = derefNumeric(value, "x", xyzArray[0], 0.0);
        bool haveY = derefNumeric(value, "y", xyzArray[1], 0.0);
        derefNumeric(value, "z", xyzArray[2], 0.0);
        xyz.Init (xyzArray[0], xyzArray[1], xyzArray[2]);
        stat = haveX && haveY;  // allow optional z
        }

    return stat;
    }

public: bool tryValueToTaggedNumericData(BeJsConst value, TaggedNumericData &data)
    {
    TaggedNumericData nullData(0,0);
    data = nullData;
    double q;
    if (derefNumeric (value, "tagA", q, 0))
        data.m_tagA = (int)q;
    if (derefNumeric(value, "tagB", q, 0))
        data.m_tagB = (int)q;
    tryValueToBVectorInt (value["intData"], data.m_intData);
    tryValueToBVectorDouble(value["doubleData"], data.m_doubleData);

    return true;
    }

private: bool tryValueToBVectorDPoint3d (BeJsConst value, bvector<DPoint3d> &data)
    {
    data.clear ();
    double xyzArray[3];
    if (value.isArray())
        {
        for (uint32_t i = 0, n = value.size(); i < n; i++)
            {
            if (!derefNumericArray (value[i], 2, 3, xyzArray))
                return false;
            data.push_back (DPoint3d::FromArray (xyzArray));
            }
        return true;
        }
    return false;
    }

private: bool tryValueToBVectorFaceData(BeJsConst value, bvector<FacetFaceData> &faceData)
    {
    faceData.clear();
    double doubles[8];
    if (value.isArray())
        {
        for (uint32_t k = 0, n = value.size(); k < n; k++)
            {
            if (!derefNumericArray(value[k], 8, 8, doubles))
                return false;
            FacetFaceData data;
            uint32_t i = 0;
            data.m_paramDistanceRange.low.x = doubles[i++];
            data.m_paramDistanceRange.low.y = doubles[i++];
            data.m_paramDistanceRange.high.x = doubles[i++];
            data.m_paramDistanceRange.high.y = doubles[i++];

            data.m_paramRange.low.x = doubles[i++];
            data.m_paramRange.low.y = doubles[i++];
            data.m_paramRange.high.x = doubles[i++];
            data.m_paramRange.high.y = doubles[i++];
            // Face indices filled with zeros by ctor !!!
            faceData.push_back(data);
            }
        return true;
        }
    return false;
    }
         
 bool tryValueToBVectorDPoint3dAndWeght (BeJsConst value, bvector<DPoint3d> &data, bvector<double> &weights)
    {
    data.clear ();
    double xyzArray[4];
    if (value.isArray())
        {
        for (uint32_t i = 0, n = value.size(); i < n; i++)
            {
            if (!derefNumericArray (value[i], 4, 4, xyzArray))
                return false;
            data.push_back (DPoint3d::FromArray (xyzArray));
            weights.push_back (xyzArray[3]);
            }
        return true;
        }
    return false;
    }

bool tryValueToBVectorDVec3d (BeJsConst value, bvector<DVec3d> &data)
    {
    data.clear ();
    double xyzArray[3];
    if (value.isArray())
        {
        for (uint32_t i = 0, n = value.size(); i < n; i++)
            {
            if (!derefNumericArray (value[i], 2, 3, xyzArray))
                return false;
            data.push_back (DVec3d::FromArray (xyzArray));
            }
        return true;
        }
    return false;
    }


bool tryValueToBVectorDPoint2d (BeJsConst value, bvector<DPoint2d> &data)
    {
    data.clear ();
    double xyzArray[2];
    if (value.isArray())
        {
        for (uint32_t i = 0, n = value.size(); i < n; i++)
            {
            if (!derefNumericArray (value[i], 2, 2, xyzArray))
                return false;
            data.push_back (DPoint2d::FromArray (xyzArray));
            }
        return true;
        }
    return false;
    }



bool tryValueGridToBVectorDPoint3d (BeJsConst value, bvector<DPoint3d> &data, bvector<double> &weight, bvector<uint32_t> &rowCounts)
    {
    data.clear ();
    weight.clear ();
    double xyzArray[4];
    if (value.isArray())
        {
        for (uint32_t i = 0, n = value.size(); i < n; i++)
            {
            if (value[i].isArray ())
                {
                auto row = value[i];
                rowCounts.push_back (value[i].size ());
                for (uint32_t j = 0, nRow = row.size(); j < nRow; j++)
                    {
                    if (row[j].size () == 3)
                        {
                        if (!derefNumericArray (row[j], 2, 3, xyzArray))
                            return false;
                        data.push_back (DPoint3d::FromArray (xyzArray));
                        // weight.push_back (1.0);
                        }
                    else if (row[j].size () == 4)
                        {
                        if (!derefNumericArray (row[j], 4, 4, xyzArray))
                            return false;
                        data.push_back (DPoint3d::FromArray (xyzArray));
                        weight.push_back (xyzArray[3]);
                        }
                    else
                        return false;
                    }
                }
            }
        return true;
        }
    return false;
    }

bool tryValueToBVectorDouble (BeJsConst value, bvector<double> &data)
    {
    data.clear ();
    if (value.isArray())
        {
        for (uint32_t i = 0, n = value.size(); i < n; i++)
            {
            if (!value[i].isNumeric ())
                return false;
            data.push_back (value[i].asDouble ());
            }
        return true;
        }
    return false;
    }

bool tryValueToBVectorInt (BeJsConst value, bvector<int> &data)
    {
    data.clear ();
    if (value.isArray())
        {
        for (uint32_t i = 0, n = value.size(); i < n; i++)
            {
            if (!value[i].isNumeric ())
                return false;
            data.push_back (value[i].asInt ());
            }
        return true;
        }
    return false;
    }

bool tryValueToBVectorUInt32 (BeJsConst value, bvector<uint32_t> &data)
    {
    data.clear ();
    if (value.isArray())
        {
        for (uint32_t i = 0, n = value.size(); i < n; i++)
            {
            if (!value[i].isNumeric ())
                return false;
            data.push_back (value[i].asUInt ());
            }
        return true;
        }
    return false;
    }

bool completeAxesConstruction (DVec3dCR vector0, int index0, DVec3dCR vector1, int index1, int index2, RotMatrixR axes, RotMatrixCR defaultAxes)
    {
    DVec3d columns[3];
    columns[index0] = vector0;
    columns[index1] = vector1;
    columns[index2] = DVec3d::FromCrossProduct (vector0, vector1);
    axes = RotMatrix::FromColumnVectors (columns[0], columns[1], columns[2]);
    if (axes.SquareAndNormalizeColumns (axes, index0, index1))
        return true;
    axes = defaultAxes;
    return false;
    }
bool derefAxes (BeJsConst source, RotMatrixR axes, RotMatrixCR defaultAxes)
    {
    axes = defaultAxes;
    auto xyVectors = source["xyVectors"];
    DVec3d vectorX, vectorY, vectorZ;
    if (xyVectors.isArray () && xyVectors.size () == 2
        && tryValueToXYZ (xyVectors[0], vectorX)
        && tryValueToXYZ (xyVectors[1], vectorY))
        return completeAxesConstruction (vectorX, 0, vectorY, 1, 2, axes, defaultAxes);

    auto zxVectors = source["zxVectors"];
    if (zxVectors.isArray () && zxVectors.size () == 2
        && tryValueToXYZ (zxVectors[0], vectorZ)
        && tryValueToXYZ (zxVectors[1], vectorX))
        return completeAxesConstruction (vectorZ, 2, vectorX, 0, 1, axes, defaultAxes);

    return false;
    }


bool tryValueToLineSegment (BeJsConst value, ICurvePrimitivePtr &result)
    {
    if (value.isNull ())
        return false;
    if (value.isArray () && value.size () > 1)
        {
        DSegment3d segment;
        if (   tryValueToXYZ (value[0], segment.point[0])
            && tryValueToXYZ (value[1], segment.point[1]))
            {
            result = ICurvePrimitive::CreateLine (segment);
            return true;
            }
        }
    return false;
    }

bool tryValueToLineString (BeJsConst value, ICurvePrimitivePtr &result)
    {
    if (value.isNull ())
        return false;
    if (value.isArray () && value.size () > 0)
        {
        auto ls = ICurvePrimitive::CreateLineString (nullptr, 0);
        for (uint32_t i = 0, n = value.size(); i < n; i++)
            {
            DPoint3d xyz;
            if (!tryValueToXYZ (value[i], xyz))
                return false;
            ls->TryAddLineStringPoint (xyz);
            }
        result = ls;
        return true;
        }
    return false;
    }

bool tryValueToPointString (BeJsConst value, ICurvePrimitivePtr &result)
    {
    if (value.isNull ())
        return false;
    if (value.isArray () && value.size () > 0)
        {
        bvector<DPoint3d> points;
        for (uint32_t i = 0, n = value.size(); i < n; i++)
            {
            DPoint3d xyz;
            if (!tryValueToXYZ (value[i], xyz))
                return false;
            points.push_back (xyz);
            }
        result = ICurvePrimitive::CreatePointString (points);
        return true;
        }
    return false;
    }

bool tryValueToBsplineCurve (BeJsConst value, ICurvePrimitivePtr &result)
    {
    if (!value.isNull ())
        {
        bvector<DPoint3d> poles;
        bvector<double> knots;
        bvector<double> weights;
        if (tryValueToBVectorDPoint3dAndWeght (value["points"], poles, weights)
            && value["order"].isNumeric ()
            && tryValueToBVectorDouble (value["knots"], knots)
            )
            {
            auto order = value["order"].asInt ();
            bool closed  = value["closed"].isNull () ? false : value["closed"].asBool ();
            auto bcurve = MSBsplineCurve::CreateFromPolesAndOrder (
                    poles, &weights, &knots,
                    (int)order, closed);
            if (bcurve.IsValid ())
                {
                result = ICurvePrimitive::CreateBsplineCurve (bcurve);
                return true;
                }
            }
        else if (tryValueToBVectorDPoint3d (value["points"], poles)
            && value["order"].isNumeric ()
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

bool tryValueToInterpolationCurve(BeJsConst value, ICurvePrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull())
        {
        bvector<DPoint3d> fitPoints;
        bvector<double> knots;
        if (tryValueToBVectorDPoint3d(value["fitPoints"], fitPoints))
            {
            tryValueToBVectorDouble(value["knots"], knots);
            auto order = AsInt(value["order"], 4).Value();
            bool closed;
            derefBool(value, "closed", closed, false);
            auto isChordLenKnots = AsInt(value["isChordLenKnots"], 0).Value ();
            auto isColinearTangents = AsInt(value["isColinearTangents"], 0);
            auto isChordLenTangents = AsInt(value["isChordLenTangents"], 0);
            auto isNaturalTangents = AsInt(value["isNaturalTangents"], 0);
            DVec3d startTangent = DVec3d::From (0,0,0), endTangent = DVec3d::From(0,0,0);
            if (!value["startTangent"].isNull ())
                tryValueToXYZ (value["startTangent"], startTangent);
            if (!value["endTangent"].isNull())
                tryValueToXYZ (value["endTangent"], endTangent);
            MSInterpolationCurve curve;
            if (SUCCESS == curve.Populate(
                order,
                closed,
                isChordLenKnots,
                isColinearTangents,
                isChordLenTangents,
                isNaturalTangents,
                fitPoints.data (),
                (int)fitPoints.size (),
                knots.size () > 0 ? knots.data () : nullptr,
                (int)knots.size(),
                &startTangent,
                &endTangent
                ))
                {
                result  = ICurvePrimitive::CreateInterpolationCurveSwapFromSource(curve);
                return true;
                }
            }
        }
    return false;
    }

bool tryValueToArc (BeJsConst value, ICurvePrimitivePtr &result)
    {
    if (!value.isNull ())
        {
        DEllipse3d arc;
        DPoint3d sweepPoint = {0,0,0};
        if (   tryValueToXYZ (value["center"], arc.center)
            && tryValueToXYZ (value["vectorX"], arc.vector0)   // treat xyz as vector
            && tryValueToXYZ (value["vectorY"], arc.vector90)  // treat xyz as vector
            && tryValueToXYZ (value["sweepStartEnd"], sweepPoint))
            {
            arc.start = Angle::DegreesToRadians (sweepPoint.x);
            arc.sweep = Angle::DegreesToRadians (sweepPoint.y - sweepPoint.x);
            result = ICurvePrimitive::CreateArc (arc);
            return true;
            }
        }
    return false;
    }

bool tryValueToCylinder (BeJsConst value, ISolidPrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull ())
        {
        DPoint3d centerA, centerB;
        bool capped;
        double radius = 0;
        derefBool (value, "capped", capped, false);
        if (   tryValueToXYZ (value["start"], centerA)
            && tryValueToXYZ (value["end"], centerB)
            && derefNumeric (value, "radius", radius))
            {
            result = ISolidPrimitive::CreateDgnCone (
                    DgnConeDetail (centerA, centerB, radius, radius, capped)
                    );
            return true;
            }
        }
    return false;
    }

bool tryValueToCone (BeJsConst value, ISolidPrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull ())
        {
        DPoint3d centerA, centerB;
        bool capped;
        RotMatrix axes;
        bool axesOK = derefAxes(value, axes, RotMatrix::FromIdentity());
        auto radius = derefValidatedDouble(value, "radius", ValidatedDouble (0, false));
        auto radiusA = derefValidatedDouble (value, "startRadius", radius);
        auto radiusB = derefValidatedDouble(value, "endRadius", radiusA);
        derefBool (value, "capped", capped, false);
        if (   tryValueToXYZ (value["start"], centerA)
            && tryValueToXYZ (value["end"], centerB)
            && radiusA.IsValid ()
            && radiusB.IsValid ())
            {
            if (axesOK)
                result = ISolidPrimitive::CreateDgnCone (DgnConeDetail (centerA, centerB, axes, radiusA, radiusB, capped));
            else
                {
                // allow the points to determine axes.
                result = ISolidPrimitive::CreateDgnCone(DgnConeDetail(centerA, centerB, radiusA, radiusB, capped));
                }
            return true;
            }
        }
    return false;
    }

bool tryValueToSpiral(BeJsConst value, ICurvePrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull())
        {
        RotMatrix axes;
        // optional members ...
        derefAxes (value, axes, RotMatrix::FromIdentity());
        double activeInterval[2];
        // alas, possible name confusion ... take anything ...
        if (derefNumericArray(value, "activeFractionInterval", 2, 2, activeInterval)
            || derefNumericArray(value, "fractionInterval", 2, 2, activeInterval)
            || derefNumericArray(value, "activeInterval", 2, 2, activeInterval)
            )
            {

            }
        else
            {
            activeInterval[0] = 0.0;
            activeInterval[1] = 1.0;
            }
        DPoint3d origin;
        double radius0 = 0.0;
        double radius1 = 0.0;
        Angle angle0 = Angle::FromDegrees (0.0);
        Angle angle1 = Angle::FromDegrees (0.0);
        double length = 0.0;
        bvector<double> extraData;
        bool hasLength = derefNumeric (value, "length", length);
        bool hasAngle1 = derefAngle (value, "endBearing", angle1);
        // assume start bearing 0 if missing. . .
        derefAngle (value, "startBearing", angle0, Angle::FromDegrees (0.0));
        Utf8String typeName;
        bool hasType = derefAsString (value, "type", typeName);
        int typeCode = DSpiral2dBase::TransitionType_Unknown;
        if (hasType)
            typeCode = DSpiral2dBase::StringToTransitionType (typeName.c_str());
        if (tryValueToXYZ(value["origin"], origin)
            && derefNumeric(value, "startRadius", radius0)
            && derefNumeric(value, "endRadius", radius1)
            && (hasLength || hasAngle1)
            && hasType
            )
            {
            auto transform = Transform::From (axes, origin);
            if (hasLength)
                {
                result = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius(
                    typeCode,
                    angle0.Radians (),
                    radius0,
                    length,
                    radius1,
                    transform,
                    activeInterval[0],
                    activeInterval[1],
                    extraData
                );
                }
            else
                {
                result = ICurvePrimitive::CreateSpiralBearingRadiusBearingRadius(
                    typeCode,
                    angle0.Radians(),
                    radius0,
                    angle1.Radians(),
                    radius1,
                    transform,
                    activeInterval[0],
                    activeInterval[1],
                    extraData
                );
                }
            }
        }
    return result.IsValid();
    }
bool tryValueToBox (BeJsConst value, ISolidPrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull ())
        {
        DPoint3d baseOrigin, topOrigin;
        bool capped;
        double baseX = 0, baseY, topX, topY, height;
        DVec3d vectorX, vectorY, vectorZ;
        RotMatrix axes;
        // required ...
        if ((tryValueToXYZ(value["origin"], baseOrigin) || tryValueToXYZ (value["baseOrigin"], baseOrigin))
            && derefNumeric (value, "baseX", baseX))
            {
            // optional with default from required values
            derefBool (value, "capped", capped, false);
            derefNumeric (value, "baseY", baseY, baseX);
            derefNumeric (value, "topX", topX, baseX);
            derefNumeric (value, "topY", topY, baseY);
            derefAxes (value, axes, RotMatrix::FromIdentity ());
            axes.GetColumns (vectorX, vectorY, vectorZ);

            if (!tryValueToXYZ (value["topOrigin"], topOrigin))
                {
                derefNumeric (value, "height", height, baseX);
                topOrigin = baseOrigin + height * vectorZ;
                }

            result = ISolidPrimitive::CreateDgnBox (DgnBoxDetail (
                    baseOrigin,
                    topOrigin,
                    vectorX,
                    vectorY,
                    baseX,
                    baseY,
                    topX,
                    topY,
                    capped));
            return true;
            }
        }
    return false;
    }

bool tryValueToTorusPipe (BeJsConst value, ISolidPrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull ())
        {
        DPoint3d origin;
        bool capped;
        double majorRadius = 0, minorRadius= 0;
        DVec3d vectorX, vectorY, vectorZ;
        RotMatrix axes;

        // strictly optional . ..
        derefBool (value, "capped", capped, false);
        derefAxes (value, axes, RotMatrix::FromIdentity ());
        Angle sweepAngle = Angle::FromFullCircle ();
        tryValueToAngle (value["sweepAngle"], sweepAngle, sweepAngle);
        // required ...
        if (   tryValueToXYZ (value["center"], origin)
            && derefNumeric (value, "majorRadius", majorRadius)
            && derefNumeric (value, "minorRadius", minorRadius))
            {
            axes.GetColumns (vectorX, vectorY, vectorZ);
            result = ISolidPrimitive::CreateDgnTorusPipe (
                DgnTorusPipeDetail (
                    origin, vectorX, vectorY,
                    majorRadius, minorRadius, sweepAngle.Radians (),
                    capped));
            return true;
            }
        }
    return false;
    }

bool tryValueToSphere (BeJsConst value, ISolidPrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull ())
        {
        DPoint3d center;
        bool capped;
        RotMatrix axes;

        // strictly optional . ..
        derefBool (value, "capped", capped, false);
        derefAxes (value, axes, RotMatrix::FromIdentity ());
        auto radius = derefValidatedDouble (value, "radius", ValidatedDouble (0.0, false));
        auto radiusX = derefValidatedDouble (value, "radius", radius);
        auto radiusY = derefValidatedDouble (value, "radius", radiusX);
        auto radiusZ = derefValidatedDouble (value, "radius", radiusY);
        // required ...
        if (   tryValueToXYZ (value["center"], center)
            && radiusX.IsValid ()
            && radiusY.IsValid ()
            && radiusZ.IsValid ())
            {
            // hm .. insider knowledge here
            // 1) sphere by radius and axes multiplies the axes by the radius -- radius=1 preserves "our" radii.
            axes.ScaleColumns (radiusX.Value (), radiusY.Value (), radiusZ.Value ());
            DgnSphereDetail dgnSphere (center, axes, 1.0);
            ValidatedDouble latitudeStartRadians, latitudeSweepRadians;
            if (derefLatitudeStartSweepRadians (value, latitudeStartRadians, latitudeSweepRadians))
                {
                dgnSphere.m_capped = capped;
                dgnSphere.m_startLatitude = latitudeStartRadians.Value ();
                dgnSphere.m_latitudeSweep = latitudeSweepRadians.Value ();
                }
            result = ISolidPrimitive::CreateDgnSphere (dgnSphere);
            return true;
            }
        }
    return false;
    }

bool tryValueToLinearSweep (BeJsConst value, ISolidPrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull ())
        {
        bool capped;
        DVec3d vector;
        derefBool (value, "capped", capped, false);
        CurveVectorPtr cv = tryValueToCurveVector (value["contour"]);
        if (tryValueToXYZ (value["vector"], vector)
            && cv.IsValid ())
            {
            result = ISolidPrimitive::CreateDgnExtrusion (DgnExtrusionDetail(cv, vector, capped));
            return true;
            }

        }
    return false;
    }

bool tryValueToRotationalSweep (BeJsConst value, ISolidPrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull ())
        {
        bool capped;
        DVec3d axis;
        DPoint3d center;
        Angle sweepAngle;
        derefBool (value, "capped", capped, false);
        CurveVectorPtr cv = tryValueToCurveVector (value["contour"]);
        if (  tryValueToXYZ (value["axis"], axis)
           && tryValueToXYZ (value["center"], center)
           && tryValueToAngle (value["sweepAngle"], sweepAngle, Angle::FromFullCircle ())
           && cv.IsValid ())
            {
            result = ISolidPrimitive::CreateDgnRotationalSweep (DgnRotationalSweepDetail(cv, center, axis, sweepAngle.Radians (), capped));
            return true;
            }
        }
    return false;
    }

bool tryValueToRuledSweep (BeJsConst value, ISolidPrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull ())
        {
        bool capped;
        derefBool (value, "capped", capped, false);
        bvector<CurveVectorPtr> contours;
        if (tryArrayToArrayOfCurveVectors (value["contour"], contours))
            {
            result = ISolidPrimitive::CreateDgnRuledSweep (DgnRuledSweepDetail(contours, capped));
            return true;
            }
        }
    return false;
    }
// AUXDATA -- DO NOT PORT THIS TO CONNECT
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PolyfaceAuxChannel::DataPtr  tryValueToPolyfaceAuxDataChannelData(BeJsConst value)
    {
    if (!value.isObject() ||
        !value["values"].isArray())
        return nullptr;

    bvector<double> values;
    auto valuesValue = value["values"];

    for (uint32_t i = 0, n = valuesValue.size(); i < n; i++)
        values.push_back(valuesValue[i].asDouble());

    return values.empty() ? nullptr : new PolyfaceAuxChannel::Data(value["input"].asDouble(), std::move(values));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PolyfaceAuxChannelPtr  tryValueToPolyfaceAuxDataChannel(BeJsConst value)
    {
    if (!value.isObject() ||
        !value["dataType"].isNumeric() ||
        !value["name"].isString() ||
        !value["data"].isArray())
        return nullptr;

    bvector<PolyfaceAuxChannel::DataPtr>   dataVector;
    auto dataValue = value["data"];

    for (uint32_t i = 0, n = dataValue.size(); i < n; i++)
        {
        PolyfaceAuxChannel::DataPtr    data;

        if ((data = tryValueToPolyfaceAuxDataChannelData(dataValue[i])).IsValid())
            dataVector.push_back(data);
        }

    return dataVector.empty() ? nullptr : new PolyfaceAuxChannel((PolyfaceAuxChannel::DataType) value["dataType"].asInt(), value["name"].asString().c_str(), value["inputName"].asString().c_str(), std::move(dataVector));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool  tryValueToPolyfaceAuxDataChannels(PolyfaceAuxData::ChannelsR channels, BeJsConst value)
    {
    if (!value.isArray())
        return false;

    for (uint32_t i = 0, n = value.size(); i < n; i++)
        {
        PolyfaceAuxChannelPtr     channel;

        if ((channel = tryValueToPolyfaceAuxDataChannel(value[i])).IsValid())
            channels.push_back(channel);
        }
    return !channels.empty();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PolyfaceAuxDataPtr  tryValueToPolyfaceAuxData(BeJsConst value)
    {
    bvector<int32_t>            indices;
    PolyfaceAuxData::Channels   channels;

    if (value.isNull() ||
        !tryValueToBVectorInt(value["indices"], indices) ||
        !tryValueToPolyfaceAuxDataChannels(channels, value["channels"]))
        return nullptr;

    return new PolyfaceAuxData(
std::move(indices), std::move(channels));
    }
typedef ValidatedValue<int> ValidatedInt;

// EDL 12/23/2020 using value.asInt() is unpredictable.
// test for BlockedPolyface
//   1) Fails when run under bb
//   2) OK in debugger
//   3) OK for geomlibstest.exe from console
// isolating for downcast seems ok this way.
ValidatedInt AsInt(BeJsConst value, int defaultValue = 0)
    {
    if (value.isNumeric ())
        {
        double a = value.asDouble ();
        double ia = trunc(a);
        if (ia == a)
            return ValidatedInt ((int)ia, true);
        return ValidatedInt(defaultValue, false);
        }
    return ValidatedInt(defaultValue, false);

    }
PolyfaceHeaderPtr tryValueToPolyfaceHeader (BeJsConst parentValue)
    {
    if (parentValue.isNull ())
        return nullptr;
    auto value = parentValue["indexedMesh"];
    if (value.isNull ())
        return nullptr;
    PolyfaceHeaderPtr pf = PolyfaceHeader::CreateVariableSizeIndexed ();    // this makes numPerFace 0
    auto iNumPerFace = AsInt (value["numPerFace"], 0);
    if (iNumPerFace.IsValid ())
        pf->SetNumPerFace (iNumPerFace.Value ());

    auto jTwoSided = value["twoSided"];
    if (jTwoSided.isBool ())
        pf->SetTwoSided (jTwoSided.asBool ());
    auto jExpectedClosure = value["expectedClosure"];
    if (jExpectedClosure.isNumeric())
        pf->SetExpectedClosure(jExpectedClosure.asInt());

    auto iExpectedClosure = AsInt (value["expectedClosure"]);
    if (iExpectedClosure.IsValid())
        pf->SetExpectedClosure(iExpectedClosure.Value());

    if (tryValueToBVectorDPoint3d (value["point"], pf->Point ()))
        pf->Point().SetActive (true);
    if (tryValueToBVectorInt(value["pointIndex"], pf->PointIndex ()))
        pf->PointIndex().SetActive (true);
        
    if (tryValueToBVectorUInt32 (value["color"], pf->IntColor ()))
        pf->IntColor().SetActive (true);
    if (tryValueToBVectorInt(value["colorIndex"], pf->ColorIndex ()))
        pf->ColorIndex().SetTags(pf->ColorIndex().NumPerStruct(), pf->ColorIndex().StructsPerRow(),
                                 MESH_ELM_TAG_FACE_LOOP_TO_INT_COLOR_INDICES,   // force correct tag
                                 pf->ColorIndex().IndexFamily(), pf->ColorIndex().IndexedBy(), true);

    if (tryValueToBVectorDVec3d (value["normal"], pf->Normal ()))
        pf->Normal().SetActive (true);
    if (tryValueToBVectorInt(value["normalIndex"], pf->NormalIndex()))
        pf->NormalIndex().SetActive (true);

    if (tryValueToBVectorDPoint2d (value["param"], pf->Param ()))
        pf->Param().SetActive (true);
    if (tryValueToBVectorInt(value["paramIndex"], pf->ParamIndex()))
        pf->ParamIndex().SetActive (true);

    if (tryValueToBVectorFaceData(value["faceData"], pf->FaceData()))
        pf->FaceData().SetActive(true);
    if (tryValueToBVectorInt(value["faceIndex"], pf->FaceIndex()))
        pf->FaceIndex().SetActive(true);

    PolyfaceAuxDataPtr      auxData;
    if ((auxData = tryValueToPolyfaceAuxData(value["auxData"])).IsValid())
        pf->SetAuxData(auxData);

    TaggedNumericData numericData;
    if (tryValueToTaggedNumericData(value["tags"], numericData))
        {
        pf->SetNumericTags (numericData);
        }
    return pf;
    }

MSBsplineSurfacePtr tryValueToMSBsplineSurface (BeJsConst parentValue)
    {
    if (parentValue.isNull ())
        return nullptr;
    auto value = parentValue["bsurf"];
    if (value.isNull())
        return nullptr;
    bvector<DPoint3d> poles;
    bvector<double> uKnots;
    bvector<double> vKnots;
    bvector<uint32_t> rowCounts;
    bvector<double> weights;
    if (tryValueGridToBVectorDPoint3d (value["points"], poles, weights, rowCounts)
        && value["orderU"].isNumeric ()
        && value["orderV"].isNumeric ()
        && tryValueToBVectorDouble (value["uKnots"], uKnots)
        && tryValueToBVectorDouble (value["vKnots"], vKnots)
        && rowCounts.size () >= 1
        && rowCounts[0] * rowCounts.size () == poles.size ()
        )
        {
        uint32_t numV = (uint32_t)rowCounts.size ();
        uint32_t numU = rowCounts[0];
        bool closedU, closedV;
        derefBool (value, "closedU", closedU, false);
        derefBool (value, "closedV", closedV, false);
        int orderU = value["orderU"].asInt ();
        int orderV = value["orderV"].asInt ();
        auto bsurf = MSBsplineSurface::CreateFromPolesAndOrder (
                poles, weights.size () == poles.size () ? &weights : nullptr,
                &uKnots,(int)orderU, numU, closedU,
                &vKnots,(int)orderV, numV, closedV,
                true);

        if (bsurf.IsValid())
            {
            CurveVectorPtr uvBoundaries = tryValueToCurveVector(value["uvBoundaries"]);
            if (uvBoundaries.IsValid() && uvBoundaries->size() > 0)
                bsurf->SetTrim(*uvBoundaries);
            if (value["outerBoundaryActive"].isBool())
                {
                // The new surface implicitly has outerBoundaryActive true. Don't reset it unless explicitly false
                bool active = value["outerBoundaryActive"].asBool();
                if (!active)
                    bsurf->SetOuterBoundaryActive(false);
                }
            return bsurf;
            }
        }
    return nullptr;
    }

bool tryArrayToCurveVectorMembers
(
BeJsConst value,
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
        for (uint32_t i = 0, n = value.size(); i < n; i++)
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
        if (boundaryType == CurveVector::BOUNDARY_TYPE_ParityRegion && result->size () > 0)
            {
            // PP demands that only one loop be called outer ... check it . .
            int numOuter = 0;
            int numInner = 0;
            int numOther = 0;
            for (auto & cp : *result)
                {
                auto loop = cp->GetChildCurveVectorP ();
                if (loop.IsValid ())    // it really has to be valid
                    {
                    if (loop->GetBoundaryType () == CurveVector::BOUNDARY_TYPE_Outer)
                        numOuter++;
                    else if (loop->GetBoundaryType () == CurveVector::BOUNDARY_TYPE_Inner)
                        numInner++;
                    else
                        numOther++;
                    }
                }
            if (numOther != 0)
                {
                // This is a mess.  Just leave it.
                }
            else if (numOuter == 1)
                {
                // This is right.
                }
            else // mark the first one outer, others inner.
                {
                numOuter = 0;
                for (size_t i = 0; i < result->size (); i++)
                    {
                    auto loop = result->at(i)->GetChildCurveVectorP ();
                    if (loop.IsValid ())
                        {
                        if (numOuter == 0)
                            {
                            loop->SetBoundaryType (CurveVector::BOUNDARY_TYPE_Outer);
                            numOuter++;
                            }
                        else
                            loop->SetBoundaryType (CurveVector::BOUNDARY_TYPE_Inner);
                        }
                    }
                }
            }
        return true;
        }
    return false;
    }

bool tryArrayToArrayOfCurveVectors
(
BeJsConst value,
bvector<CurveVectorPtr> &curveVectors
)
    {
    curveVectors.clear ();
    if (value.isNull ())
        return false;
    if (value.isArray ())
        {
        for (uint32_t i = 0, n = value.size(); i < n; i++)
            {
            auto cv1 = tryValueToCurveVector (value[i]);
            if (cv1.IsValid ())
                {
                curveVectors.push_back (cv1);
                }
            }
        return true;
        }
    return false;
    }

CurveVectorPtr tryValueToCurveVector (BeJsConst value)
    {
    if (value.isNull ())
        return nullptr;
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

ICurvePrimitivePtr tryValueToCurvePrimitive (BeJsConst value)
    {
    ICurvePrimitivePtr cp;
    if (tryValueToLineSegment (value["lineSegment"], cp))
        return cp;
    if (tryValueToLineString (value["lineString"], cp))
        return cp;
    if (tryValueToBsplineCurve (value["bcurve"], cp))
        return cp;
    if (tryValueToInterpolationCurve(value["interpolationCurve"], cp))
        return cp;
    if (tryValueToArc (value["arc"], cp))
        return cp;
    if (tryValueToSpiral(value["transitionSpiral"], cp))
        return cp;
    if (tryValueToPointString (value["pointString"], cp))
        return cp;
    return nullptr;
    }
ISolidPrimitivePtr tryValueToSolidPrimitive (BeJsConst value)
    {
    ISolidPrimitivePtr cp;
    if (tryValueToCylinder (value["cylinder"], cp))
        return cp;
    if (tryValueToCone(value["cone"], cp))
        return cp;
    if (tryValueToBox(value["box"], cp))
        return cp;
    if (tryValueToTorusPipe(value["torusPipe"], cp))
        return cp;
    if (tryValueToSphere(value["sphere"], cp))
        return cp;
    if (tryValueToLinearSweep(value["linearSweep"], cp))
        return cp;
    if (tryValueToRotationalSweep(value["rotationalSweep"], cp))
        return cp;
    if (tryValueToRuledSweep(value["ruledSweep"], cp))
        return cp;
    return nullptr;
    }


bool tryValueToAction (BeJsConst value)
    {
    DVec3d vector;
    if (tryValueToXYZ (value["moveOrigin"], vector))
        {
        m_transform = Transform::From (vector) * m_transform;
        return true;
        }
    else if (tryValueToXYZ (value["setOrigin"], vector))
        {
        m_transform = Transform::From(vector);
        return true;
        }
    return false;
    }
IGeometryPtr ApplyState (IGeometryPtr &geometry)
    {
    if (!m_transform.IsIdentity () && geometry.IsValid ())
        geometry->TryTransformInPlace (m_transform);
    return geometry;
    }
IGeometryPtr tryValueToIGeometry (BeJsConst value)
    {
    if (value.isObject ())
        {
        ICurvePrimitivePtr cp = tryValueToCurvePrimitive (value);
        if (cp.IsValid ())
            return IGeometry::Create (cp);
        CurveVectorPtr cv = tryValueToCurveVector (value);
        if (cv.IsValid ())
            return IGeometry::Create (cv);
        ISolidPrimitivePtr sp = tryValueToSolidPrimitive (value);
        if (sp.IsValid ())
            return IGeometry::Create (sp);
        MSBsplineSurfacePtr bsurf = tryValueToMSBsplineSurface(value);
        if (bsurf.IsValid ())
            return IGeometry::Create (bsurf);
        PolyfaceHeaderPtr pf = tryValueToPolyfaceHeader(value);
        if (pf.IsValid ())
            return IGeometry::Create (pf);
        if (tryValueToAction (value))
            return nullptr;     // not an error, just nothing to return.
        }
    return nullptr;
    }


public: bool TryParse (BeJsConst source, bvector<IGeometryPtr> &geometry)
    {
    if (source.isObject ())
        {
        auto result = tryValueToIGeometry (source);
        if (result.IsValid ())
            geometry.push_back (ApplyState (result));
        }
    else if (source.isArray ())
        {
        // just recurse to next level -- deep array structure is flattened in the output.
        int n = source.size ();
        for (int i = 0; i < n; i++)
            {
            auto member = tryValueToIGeometry (source[i]);
            if (member.IsValid ())
                geometry.push_back (ApplyState (member));
            }
        }
    return geometry.size () > 0;
    }
};
static GeometryValidatorPtr s_readValidator = GeometryValidator::Create ();

bool IModelJson::TryIModelJsonValueToGeometry
(
BeJsConst value,
bvector<IGeometryPtr> &geometry,
bvector<IGeometryPtr> *invalidGeometry
)
    {
    BeCGIModelJsonValueReader reader;
    if (invalidGeometry)
        invalidGeometry->clear();
    bool stat = reader.TryParse (value, geometry);
    if (!stat)
        return false;
    GeometryValidator::RemoveInvalidGeometry(s_readValidator, geometry, invalidGeometry);
    return true;
    }

bool IModelJson::TryIModelJsonStringToGeometry (Utf8StringCR string, bvector<IGeometryPtr> &geometry)
    {
    BeJsDocument value(string.c_str());
    if (value.isNull())
        return false;
    return TryIModelJsonValueToGeometry (value, geometry);
    }

bool IModelJson::TryIModelJsonValueToTaggedNumericData(BeJsConst value, TaggedNumericData &data)
    {
    BeCGIModelJsonValueReader reader;
    if (value.isNull())
        return false;
    bool stat = reader.tryValueToTaggedNumericData (value, data);
    return stat;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
