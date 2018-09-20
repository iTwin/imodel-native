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
    Transform m_transform;
public:
    BeCGIModelJsonValueReader ()
        {
        m_transform = Transform::FromIdentity ();
        }
private:
// Return true if value is an entirely numeric array.
// Also transfer up to maxNeeded double to values[].  Fill tail of values[] with 0.
// Return true if at least minNeeded were received.
bool derefNumericArray (JsonValueCR value, size_t minNeeded, size_t maxNeeded, double values[])
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
        if (numOut < maxNeeded)
            values[numOut++] = value[i].asDouble ();
        }
    for (uint32_t i = numOut; i < maxNeeded; i++)
        values[i] = 0;
    return numOut >= minNeeded;
    }

// number ==> degrees
// {degrees: number}
// {radians: number}
bool tryValueToAngle (JsonValueCR value, AngleR &angle, AngleCR defaultAngle = Angle::FromRadians (0))
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
bool derefLatitudeStartSweepRadians (JsonValueCR value, ValidatedDouble &startRadians, ValidatedDouble &sweepRadians)
    {

    startRadians = ValidatedDouble (-msGeomConst_piOver2, false);
    sweepRadians = ValidatedDouble (msGeomConst_pi, false);
    auto startEndValue = value["latitudeStartEnd"];
    if (value.isNull ())
        return false;
    Angle startAngle, endAngle;
    if (startEndValue.isArray () && startEndValue.size () == 2
        && tryValueToAngle (startEndValue[0], startAngle)
        && tryValueToAngle (startEndValue[1], endAngle))
        {
        startRadians = ValidatedDouble (startAngle.Radians (), true);
        sweepRadians = ValidatedDouble (endAngle.Radians () - startAngle.Radians (), true);
        return true;
        }
    return false;
    }

bool derefNumeric (JsonValueCR source, char const *name, double &value, double defaultValue = 0.0)
    {
    value = defaultValue;
    auto jsonValue = source[name];
    if (jsonValue.isNumeric ())
        {
        value = jsonValue.asDouble ();
        return true;
        }
    return false;
    }

ValidatedDouble derefValidatedDouble (JsonValueCR source, char const *name, ValidatedDouble const &defaultValue)
    {
    // unused - auto value = defaultValue;
    auto jsonValue = source[name];
    if (jsonValue.isNumeric ())
        return ValidatedDouble (jsonValue.asDouble (), true);
    return defaultValue;
    }


bool derefBool (JsonValueCR source, char const *name, bool &value, bool defaultValue = false)
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
bool tryValueToXYZ (JsonValueCR value, DPoint3dR xyz)
    {
    double xyzArray[3];
    bool stat = false;
    if (derefNumericArray (value, 2, 3, xyzArray))
        {
        xyz.Init (xyzArray[0], xyzArray[1], xyzArray[2]);
        stat = true;
        }
    else
        {
        // IMPORTANT -- single bar to ensure all three are called . . 
        stat = derefNumeric (value, "x", xyzArray[0], 0.0)
             | derefNumeric (value, "y", xyzArray[1], 0.0)
             | derefNumeric (value, "z", xyzArray[2], 0.0);
        xyz.Init (xyzArray[0], xyzArray[1], xyzArray[2]);
        stat = true;
        }
    return stat;
    }
bool tryValueToBVectorDPoint3d (JsonValueCR value, bvector<DPoint3d> &data)
    {
    data.clear ();
    double xyzArray[3];
    if (value.isArray ())
        {
        for (uint32_t i = 0; i < value.size (); i++)
            {
            if (!derefNumericArray (value[i], 2, 3, xyzArray))
                return false;
            data.push_back (DPoint3d::FromArray (xyzArray));
            }
        return true;
        }
    return true;
    }

bool tryValueToBVectorDPoint3dAndWeght (JsonValueCR value, bvector<DPoint3d> &data, bvector<double> &weights)
    {
    data.clear ();
    double xyzArray[4];
    if (value.isArray ())
        {
        for (uint32_t i = 0; i < value.size (); i++)
            {
            if (!derefNumericArray (value[i], 4, 4, xyzArray))
                return false;
            data.push_back (DPoint3d::FromArray (xyzArray));
            weights.push_back (xyzArray[3]);
            }
        return true;
        }
    return true;
    }

bool tryValueToBVectorDVec3d (JsonValueCR value, bvector<DVec3d> &data)
    {
    data.clear ();
    double xyzArray[3];
    if (value.isArray ())
        {
        for (uint32_t i = 0; i < value.size (); i++)
            {
            if (!derefNumericArray (value[i], 2, 3, xyzArray))
                return false;
            data.push_back (DVec3d::FromArray (xyzArray));
            }
        return true;
        }
    return true;
    }


bool tryValueToBVectorDPoint2d (JsonValueCR value, bvector<DPoint2d> &data)
    {
    data.clear ();
    double xyzArray[2];
    if (value.isArray ())
        {
        for (uint32_t i = 0; i < value.size (); i++)
            {
            if (!derefNumericArray (value[i], 2, 2, xyzArray))
                return false;
            data.push_back (DPoint2d::FromArray (xyzArray));
            }
        return true;
        }
    return true;
    }



bool tryValueGridToBVectorDPoint3d (JsonValueCR value, bvector<DPoint3d> &data, bvector<double> &weight, bvector<uint32_t> &rowCounts)
    {
    data.clear ();
    weight.clear ();
    double xyzArray[4];
    if (value.isArray ())
        {
        for (uint32_t i = 0; i < value.size (); i++)
            {
            if (value[i].isArray ())
                {
                JsonValueCR row = value[i];
                rowCounts.push_back (value[i].size ());
                for (uint32_t j = 0; j < row.size (); j++)
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

bool tryValueToBVectorInt (JsonValueCR value, bvector<int> &data)
    {
    data.clear ();
    if (value.isArray ())
        {
        for (uint32_t i = 0; i < value.size (); i++)
            {
            if (!value[i].isIntegral ())
                return false;
            data.push_back (value[i].asInt ());
            }
        return true;
        }
    return true;
    }

bool tryValueToBVectorUInt32 (JsonValueCR value, bvector<uint32_t> &data)
    {
    data.clear ();
    if (value.isArray ())
        {
        for (uint32_t i = 0; i < value.size (); i++)
            {
            if (!value[i].isIntegral ())
                return false;
            data.push_back (value[i].asInt ());
            }
        return true;
        }
    return true;
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
bool derefAxes (JsonValueCR source, RotMatrixR axes, RotMatrixCR defaultAxes)
    {
    axes = defaultAxes;
    auto &xyVectors = source["xyVectors"];
    DVec3d vectorX, vectorY, vectorZ;
    if (!xyVectors.isNull () && xyVectors.isArray () && xyVectors.size () == 2
        && tryValueToXYZ (xyVectors[0], vectorX)
        && tryValueToXYZ (xyVectors[1], vectorY))
        return completeAxesConstruction (vectorX, 0, vectorY, 1, 2, axes, defaultAxes);

    auto &zxVectors = source["zxVectors"];
    if (!zxVectors.isNull () && zxVectors.isArray () && zxVectors.size () == 2
        && tryValueToXYZ (zxVectors[0], vectorZ)
        && tryValueToXYZ (zxVectors[1], vectorX))
        return completeAxesConstruction (vectorZ, 2, vectorX, 0, 1, axes, defaultAxes);

    return false;
    }


bool tryValueToLineSegment (JsonValueCR value, ICurvePrimitivePtr &result)
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
            if (!tryValueToXYZ (value[i], xyz))
                return false;
            ls->TryAddLineStringPoint (xyz);
            }
        result = ls;
        return true;
        }
    return false;
    }

bool tryValueToPointString (JsonValueCR value, ICurvePrimitivePtr &result)
    {
    if (value.isNull ())
        return false;
    if (value.isArray () && value.size () > 1)
        {
        bvector<DPoint3d> points;
        for (uint32_t i = 0; i < value.size (); i++)
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

bool tryValueToBsplineCurve (JsonValueCR value, ICurvePrimitivePtr &result)
    {
    if (!value.isNull ())
        {
        bvector<DPoint3d> poles;
        bvector<double> knots;
        bvector<double> weights;
        if (tryValueToBVectorDPoint3dAndWeght (value["points"], poles, weights)
            && value["order"].isIntegral ()
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

bool tryValueToCylinder (JsonValueCR value, ISolidPrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull ())
        {
        DPoint3d centerA, centerB;
        bool capped;
        double radius;
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

bool tryValueToCone (JsonValueCR value, ISolidPrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull ())
        {
        DPoint3d centerA, centerB;
        bool capped;
        double radiusA, radiusB;
        derefBool (value, "capped", capped, false);
        if (   tryValueToXYZ (value["start"], centerA)
            && tryValueToXYZ (value["end"], centerB)
            && derefNumeric (value, "startRadius", radiusA))
            {
            derefNumeric (value, "endRadius", radiusB, radiusA);
            result = ISolidPrimitive::CreateDgnCone (DgnConeDetail (centerA, centerB, radiusA, radiusB, capped));
            return true;
            }
        }
    return false;
    }

bool tryValueToBox (JsonValueCR value, ISolidPrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull ())
        {
        DPoint3d baseOrigin, topOrigin;
        bool capped;
        double baseX, baseY, topX, topY, height;
        DVec3d vectorX, vectorY, vectorZ;
        RotMatrix axes;
        // strictly optional . ..
        derefBool (value, "capped", capped, false);
        // required ...
        if (   tryValueToXYZ (value["baseOrigin"], baseOrigin)
            && derefNumeric (value, "baseX", baseX))
            {
            // optional with default from required values . ..
            derefNumeric (value, "baseY", baseY, baseX);
            derefNumeric (value, "topX", topX, baseX);
            derefNumeric (value, "topY", topY, baseY);
            derefAxes (value, axes, RotMatrix::FromIdentity ());
            axes.GetColumns (vectorX, vectorY, vectorZ);
            if (derefNumeric (value, "height", height))
                {
                topOrigin = baseOrigin + height * vectorZ;
                }
            else if (tryValueToXYZ (value["topOrigin"], topOrigin))
                {
                }
            else
                return false;

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

bool tryValueToTorusPipe (JsonValueCR value, ISolidPrimitivePtr &result)
    {
    result = nullptr;
    if (!value.isNull ())
        {
        DPoint3d origin;
        bool capped;
        double majorRadius, minorRadius;
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

bool tryValueToSphere (JsonValueCR value, ISolidPrimitivePtr &result)
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

bool tryValueToLinearSweep (JsonValueCR value, ISolidPrimitivePtr &result)
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

bool tryValueToRotationalSweep (JsonValueCR value, ISolidPrimitivePtr &result)
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

bool tryValueToRuledSweep (JsonValueCR value, ISolidPrimitivePtr &result)
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
* @bsimethod                                                    Ray.Bentley      04/2018
+--------------------------------------------------------------------------------------*/
PolyfaceAuxChannel::DataPtr  tryValueToPolyfaceAuxDataChannelData(JsonValueCR value)
    {
    if (!value.isObject() ||
        !value["values"].isArray())
        return nullptr;

    bvector<double> values;
    JsonValueCR     valuesValue = value["values"];

    for (uint32_t i=0; i<valuesValue.size(); i++)
        values.push_back(valuesValue[i].asDouble());

    return values.empty() ? nullptr : new PolyfaceAuxChannel::Data(value["input"].asDouble(), std::move(values));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      04/2018
+--------------------------------------------------------------------------------------*/
PolyfaceAuxChannelPtr  tryValueToPolyfaceAuxDataChannel(JsonValueCR value)
    {
    if (!value.isObject() ||
        !value["dataType"].isIntegral() ||
        !value["name"].isString() ||
        !value["data"].isArray())
        return nullptr;
        
    bvector<PolyfaceAuxChannel::DataPtr>   dataVector;
    JsonValueCR                         dataValue = value["data"];


    for (uint32_t i=0; i < dataValue.size(); i++)
        {
        PolyfaceAuxChannel::DataPtr    data;

        if ((data = tryValueToPolyfaceAuxDataChannelData(dataValue[i])).IsValid())
            dataVector.push_back(data);
        }
    
    return dataVector.empty() ? nullptr : new PolyfaceAuxChannel((PolyfaceAuxChannel::DataType) value["dataType"].asInt(), value["name"].asString().c_str(), value["inputName"].asString().c_str(), std::move(dataVector));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      04/2018
+--------------------------------------------------------------------------------------*/
bool  tryValueToPolyfaceAuxDataChannels(PolyfaceAuxData::ChannelsR channels, JsonValueCR value)
    {
    if (!value.isArray())
        return false;

    for (uint32_t i=0; i < value.size(); i++)
        {
        PolyfaceAuxChannelPtr     channel;

        if ((channel = tryValueToPolyfaceAuxDataChannel(value[i])).IsValid())
            channels.push_back(channel);
        }
    return !channels.empty();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      04/2018
+--------------------------------------------------------------------------------------*/
PolyfaceAuxDataPtr  tryValueToPolyfaceAuxData(JsonValueCR value)
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

PolyfaceHeaderPtr tryValueToPolyfaceHeader (JsonValueCR parentValue)
    {
    if (parentValue.isNull ())
        return nullptr;
    JsonValueCR value = parentValue["indexedMesh"];
    if (value.isNull ())
        return nullptr;
    PolyfaceHeaderPtr pf = PolyfaceHeader::CreateVariableSizeIndexed ();    // this makes numPerFace 0
    JsonValueCR jNumPerFace = value["numPerFace"];
    if (jNumPerFace.isIntegral ())
        pf->SetNumPerFace (jNumPerFace.asInt ());
    JsonValueCR jTwoSided = value["twoSided"];
    if (jTwoSided.isBool ())
        pf->SetTwoSided (jTwoSided.asBool ());

    if (tryValueToBVectorDPoint3d (value["point"], pf->Point ()))
        pf->Point().SetActive (true);
    if (tryValueToBVectorInt(value["pointIndex"], pf->PointIndex ()))
        pf->PointIndex().SetActive (true);

    if (tryValueToBVectorUInt32 (value["color"], pf->IntColor ()))
        pf->IntColor().SetActive (true);
    if (tryValueToBVectorInt(value["colorIndex"], pf->ColorIndex ()))
        pf->ColorIndex().SetActive (true);

    if (tryValueToBVectorDVec3d (value["normal"], pf->Normal ()))
        pf->Normal().SetActive (true);
    if (tryValueToBVectorInt(value["normalIndex"], pf->NormalIndex()))
        pf->NormalIndex().SetActive (true);

    if (tryValueToBVectorDPoint2d (value["param"], pf->Param ()))
        pf->Param().SetActive (true);
    if (tryValueToBVectorInt(value["paramIndex"], pf->ParamIndex()))
        pf->ParamIndex().SetActive (true);

    PolyfaceAuxDataPtr      auxData;
    if ((auxData = tryValueToPolyfaceAuxData(value["auxData"])).IsValid())
        pf->SetAuxData(auxData);

    return pf;
    }

MSBsplineSurfacePtr tryValueToMSBsplineSurface (JsonValueCR parentValue)
    {
    if (parentValue.isNull ())
        return nullptr;
    JsonValueCR value = parentValue["bsurf"];
    if (value.isNull())
        return nullptr;
    bvector<DPoint3d> poles;
    bvector<double> uKnots;
    bvector<double> vKnots;
    bvector<uint32_t> rowCounts;
    bvector<double> weights;
    if (tryValueGridToBVectorDPoint3d (value["points"], poles, weights, rowCounts)
        && value["orderU"].isIntegral ()
        && value["orderV"].isIntegral ()
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
        if (bsurf.IsValid ())
            return bsurf;
        }
    return nullptr;
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

bool tryArrayToArrayOfCurveVectors
(
JsonValueCR value,
bvector<CurveVectorPtr> &curveVectors
)
    {
    curveVectors.clear ();
    if (value.isNull ())
        return false;
    if (value.isArray ())
        {
        for (uint32_t i = 0; i < value.size (); i++)
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

CurveVectorPtr tryValueToCurveVector (JsonValueCR value)
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
    if (tryValueToPointString (value["pointString"], cp))
        return cp;
    return nullptr;
    }
ISolidPrimitivePtr tryValueToSolidPrimitive (JsonValueCR value)
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


bool tryValueToAction (JsonValueCR value)
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


public: bool TryParse (JsonValueCR source, bvector<IGeometryPtr> &geometry)
    {
    IGeometryPtr result;
    if (source.isObject ())
        {
        auto result = tryValueToIGeometry (source);
        if (result.IsValid ())
            geometry.push_back (ApplyState (result));
        }
    else if (source.isArray ())
        {
        // just recuse to next level -- deep array structure is flattened in the output.
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

bool IModelJson::TryIModelJsonValueToGeometry (JsonValueCR value, bvector<IGeometryPtr> &geometry)
    {
    BeCGIModelJsonValueReader reader;
    return reader.TryParse (value, geometry);
    }

bool IModelJson::TryIModelJsonStringToGeometry (Utf8StringCR string, bvector<IGeometryPtr> &geometry)
    {
    Json::Value value;
    Json::Reader::Parse (string, value, false);
    if (value.isNull ())
        return false;
    return TryIModelJsonValueToGeometry (value, geometry);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
