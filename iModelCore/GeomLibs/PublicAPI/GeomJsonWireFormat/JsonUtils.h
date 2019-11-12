/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

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
    BE_JSON_NAME(radians)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value AngleInDegreesToJson(AngleInDegrees angle) {return angle.Degrees();} // default for angles in json is degrees. 
static AngleInDegrees AngleInDegreesFromJson(JsonValueCR val) {return AngleInDegrees(ToAngle(val));}
static Json::Value FromAngle(Angle angle) {return angle.Degrees();}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Angle ToAngle(JsonValueCR val)
    {
    if (val.isNull()) return Angle::FromDegrees(0.0);
    if (val.isObject())
        {
        if (val.isMember(json_degrees())) 
            return Angle::FromDegrees(val[json_degrees()].asDouble());
        if (val.isMember(json_radians())) 
            return Angle::FromRadians(val[json_radians()].asDouble());
        }
    return Angle::FromDegrees(val.asDouble());
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value YawPitchRollToJson(YawPitchRollAngles angles)
    {
    Json::Value val;
    // omit any members that are zero
    if (angles.GetYaw().Degrees() != 0.0) val[json_yaw()] = AngleInDegreesToJson(angles.GetYaw());
    if (angles.GetPitch().Degrees() != 0.0) val[json_pitch()] = AngleInDegreesToJson(angles.GetPitch());
    if (angles.GetRoll().Degrees() != 0.0) val[json_roll()] = AngleInDegreesToJson(angles.GetRoll());
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
static DPoint3d ToDPoint3d(JsonValueCR inValue) 
    {
    DPoint3d point;
    if (inValue.isArray())
        {
        point.x = inValue[0].asDouble();
        point.y = inValue[1].asDouble();
        point.z = inValue[2].asDouble();
        }
    else
        {
        point.x = inValue["x"].asDouble();
        point.y = inValue["y"].asDouble();
        point.z = inValue["z"].asDouble();
        }
    return point;
    }

static void DPoint3dFromJson(DPoint3dR point, Json::Value const& inValue) {point = ToDPoint3d(inValue);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value DPoint3dToJson(DPoint3dCR point)
    {
    Json::Value val;
    val[0] = point.x;
    val[1] = point.y;
    val[2] = point.z;
    return val;
    }
static void DPoint3dToJson(JsonValueR outValue, DPoint3dCR point) {outValue = DPoint3dToJson(point);}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void Point2dFromJson(Point2dR point, JsonValueCR inValue)
    {
    if (inValue.isArray())
        {
        point.x = inValue[0].asInt();
        point.y = inValue[1].asInt();
        }
    else
        {
        point.x = inValue["x"].asInt();
        point.y = inValue["y"].asInt();
        }
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
    if (inValue.isArray())
        {
        point.x = inValue[0].asDouble();
        point.y = inValue[1].asDouble();
        }
    else
        {
        point.x = inValue["x"].asDouble();
        point.y = inValue["y"].asDouble();
        }
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

static void DVec3dFromJson(DVec3dR vec, JsonValueCR inValue)  {DPoint3dFromJson((DPoint3dR)vec, inValue);}
static Json::Value DVec3dToJson(DVec3dCR vec) {return DPoint3dToJson((DPoint3dCR)vec);}
static DVec3d ToDVec3d(JsonValueCR inValue) 
    {
    DVec3d vec;
    if (inValue.isArray())
        {
        vec.x = inValue[0].asDouble();
        vec.y = inValue[1].asDouble();
        vec.z = inValue[2].asDouble();
        }
    else
        {
        vec.x = inValue["x"].asDouble();
        vec.y = inValue["y"].asDouble();
        vec.z = inValue["z"].asDouble();
        }
    return vec;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DVec3dToJson(JsonValueR outValue, DVec3dCR vec)
    {
    DPoint3dToJson(outValue, (DPoint3dCR)vec);
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static DRange3d ToDRange3d(JsonValueCR inValue)
    {
    DRange3d range = DRange3d::NullRange();
    if (inValue.isArray()) // if it's an array, just extend range by all points.
        {
        for (Json::ArrayIndex i=0; i<inValue.size(); ++i)
            range.Extend(ToDPoint3d(inValue[i]));
        return range;
        }

    range.Extend(ToDPoint3d(inValue[json_low()]));
    range.Extend(ToDPoint3d(inValue[json_high()]));
    return range;
    }
static void DRange3dFromJson(DRange3dR range, JsonValueCR inValue) {range = ToDRange3d(inValue);}

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
static RotMatrix ToRotMatrix(JsonValueCR inValue) 
    {
    RotMatrix rotation;
    for (int x = 0; x < 3; ++x)
        MatrixRowFromJson(rotation.form3d[x], inValue[x]);
    return rotation;
    }

static void RotMatrixFromJson(RotMatrixR rotation, JsonValueCR inValue) {rotation = ToRotMatrix(inValue);}

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
static Json::Value FromTransform(TransformCR trans)
    {
    Json::Value  out;
    for (int x = 0; x < 3; ++x)
        TransformRowToJson(out[x], trans.form3d[x]);
    return out;
    }
static void TransformToJson(JsonValueR outValue, TransformCR trans) { outValue = FromTransform(trans);}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  06/18
//---------------------------------------------------------------------------------------
static DMatrix4d ToDMatrix4d(JsonValueCR inValue)
    {
    DMatrix4d matrix;
    for (int x = 0; x < 4; ++x)
        TransformRowFromJson(matrix.coff[x], inValue[x]);
    return matrix;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  06/18
//---------------------------------------------------------------------------------------
static Json::Value FromDMatrix4d(DMatrix4dCR matrix)
    {
    Json::Value val;
    for (int x = 0; x < 4; ++x)
        TransformRowToJson(val[x], matrix.coff[x]);
    return val;
    }

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

};

END_BENTLEY_GEOMETRY_NAMESPACE
