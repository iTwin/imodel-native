/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/JsonUtils.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeJsonCpp/BeJsonUtilities.h>
#include <DgnPlatform/ClipVector.h>
#include <ECObjects/ECValue.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Utilities to help save and restore values as JSON
// @bsiclass
//=======================================================================================
struct JsonUtils
{
    BE_JSON_NAME(low)
    BE_JSON_NAME(high)
    BE_JSON_NAME(yaw)
    BE_JSON_NAME(pitch)
    BE_JSON_NAME(roll)
    BE_JSON_NAME(degrees)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value AngleInDegreesToJson(AngleInDegrees angle)
    {
    Json::Value val;
    val[json_degrees()] = angle.Degrees();
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static AngleInDegrees AngleInDegreesFromJson(JsonValueCR val)
    {
    return AngleInDegrees::FromDegrees(val[json_degrees()].asDouble());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value YawPitchRollToJson(YawPitchRollAngles angles)
    {
    Json::Value val;
    val[json_yaw()] = AngleInDegreesToJson(angles.GetYaw());
    val[json_pitch()] = AngleInDegreesToJson(angles.GetPitch());
    val[json_roll()] = AngleInDegreesToJson(angles.GetRoll());
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static YawPitchRollAngles YawPitchRollFromJson(JsonValueCR val)
    {
    auto yaw = AngleInDegreesFromJson(val[json_yaw()]);
    auto pitch = AngleInDegreesFromJson(val[json_pitch()]);
    auto roll = AngleInDegreesFromJson(val[json_roll()]);
    return YawPitchRollAngles::FromDegrees(yaw.Degrees(), pitch.Degrees(), roll.Degrees());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DPoint3dFromJson(DPoint3dR point, Json::Value const& inValue)
    {
    point.x = inValue[0].asDouble();
    point.y = inValue[1].asDouble();
    point.z = inValue[2].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DPoint3dToJson(JsonValueR outValue, DPoint3dCR point)
    {
    outValue[0] = point.x;
    outValue[1] = point.y;
    outValue[2] = point.z;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void Point2dFromJson(Point2dR point, JsonValueCR inValue)
    {
    point.x = inValue[0].asInt();
    point.y = inValue[1].asInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void Point2dToJson(JsonValueR outValue, Point2dCR point)
    {
    outValue[0] = point.x;
    outValue[1] = point.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DPoint2dFromJson(DPoint2dR point, JsonValueCR inValue)
    {
    point.x = inValue[0].asDouble();
    point.y = inValue[1].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DPoint2dToJson(JsonValueR outValue, DPoint2dCR point)
    {
    outValue[0] = point.x;
    outValue[1] = point.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DVec2dFromJson(DVec2dR vec, JsonValueCR inValue)
    {
    DPoint2dFromJson((DPoint2dR)vec, inValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DVec2dToJson(JsonValueR outValue, DVec2dCR vec)
    {
    DPoint2dToJson(outValue, (DPoint2dCR)vec);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DVec3dFromJson(DVec3dR vec, JsonValueCR inValue)
    {
    DPoint3dFromJson((DPoint3dR)vec, inValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DVec3dToJson(JsonValueR outValue, DVec3dCR vec)
    {
    DPoint3dToJson(outValue, (DPoint3dCR)vec);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DRange3dFromJson(DRange3dR range, JsonValueCR inValue)
    {
    DPoint3dFromJson(range.low, inValue[json_low()]);
    DPoint3dFromJson(range.high, inValue[json_high()]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DRange3dToJson(JsonValueR outValue, DRange3dCR range)
    {
    DPoint3dToJson(outValue[json_low()], range.low);
    DPoint3dToJson(outValue[json_high()], range.high);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DRange2dFromJson(DRange2dR range, JsonValueCR inValue)
    {
    DPoint2dFromJson(range.low, inValue[json_low()]);
    DPoint2dFromJson(range.high, inValue[json_high()]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DRange2dToJson(JsonValueR outValue, DRange2dCR range)
    {
    DPoint2dToJson(outValue[json_low()], range.low);
    DPoint2dToJson(outValue[json_high()], range.high);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     12/12
//---------------------------------------------------------------------------------------
static void MatrixRowFromJson(double* row, JsonValueCR inValue)
    {
    for (int y = 0; y < 3; ++y)
        row[y] = inValue[y].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     12/12
//---------------------------------------------------------------------------------------
static void MatrixRowToJson(JsonValueR outValue, double const* row)
    {
    for (int y = 0; y < 3; ++y)
        outValue[y] = row[y];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void RotMatrixFromJson(RotMatrixR rotation, JsonValueCR inValue)
    {
    for (int x = 0; x < 3; ++x)
        MatrixRowFromJson(rotation.form3d[x], inValue[x]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void RotMatrixToJson(JsonValueR outValue, RotMatrixCR rotation)
    {
    for (int x = 0; x < 3; ++x)
        MatrixRowToJson(outValue[x], rotation.form3d[x]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
static void TransformRowFromJson(double* row, JsonValueCR inValue)
    {
    for (int y = 0; y < 4; ++y)
        row[y] = inValue[y].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
static void TransformRowToJson(JsonValueR outValue, double const* row)
    {
    for (int y = 0; y < 4; ++y)
        outValue[y] = row[y];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
static void TransformFromJson(TransformR trans, JsonValueCR inValue)
    {
    for (int x = 0; x < 3; ++x)
        TransformRowFromJson(trans.form3d[x], inValue[x]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
static void TransformToJson(JsonValueR outValue, TransformCR trans)
    {
    for (int x = 0; x < 3; ++x)
        TransformRowToJson(outValue[x], trans.form3d[x]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static double GetDouble(JsonValueCR inValue, double defaultValue) {return inValue.isNull() ? defaultValue : inValue.asDouble();}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
static void DPoint3dVectorToJson(JsonValueR outValue, bvector<DPoint3d> const& points)
    {
    for (size_t i=0; i<points.size(); ++i)
        {
        DPoint3dToJson(outValue[(Json::ArrayIndex)i], points[i]);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
static void DPoint3dVectorFromJson(bvector<DPoint3d>& points, JsonValueCR inValue)
    {
    if (!inValue.isArray())
        return;
    for (Json::ArrayIndex i=0; i<inValue.size(); ++i)
        {
        DPoint3d pt;
        DPoint3dFromJson(pt, inValue[(int)i]);
        points.push_back(pt);
        }
    }

/*---------------------------------------------------------------------------------**//**
*! Represent a NavigationProperty in JSON, in the format used by iModelJson.
* @bsimethod                                                    Sam.Wilson      07/17
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static Json::Value NavigationPropertyToJson(ECN::ECValue::NavigationInfo const&);

/*---------------------------------------------------------------------------------**//**
*! Parse a NavigationProperty from JSON, according to the format used by iModelJson.
* @bsimethod                                                    Sam.Wilson      07/17
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static ECN::ECValue NavigationPropertyFromJson(JsonValueCR json, DgnDbR db);

};

END_BENTLEY_DGN_NAMESPACE
