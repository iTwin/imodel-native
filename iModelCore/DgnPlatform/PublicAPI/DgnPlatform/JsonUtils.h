/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/JsonUtils.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeJsonCpp/BeJsonUtilities.h>
#include <DgnPlatform/ClipVector.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Utilities to help save and restore values as JSON
// @bsiclass
//=======================================================================================
struct JsonUtils
{
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
    DPoint3dFromJson(range.low, inValue["low"]);
    DPoint3dFromJson(range.high, inValue["high"]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DRange3dToJson(JsonValueR outValue, DRange3dCR range)
    {
    DPoint3dToJson(outValue["low"], range.low);
    DPoint3dToJson(outValue["high"], range.high);
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
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
static void ClipPlaneToJson(JsonValueR outValue, ClipPlaneCR clipPlane)
    {
    DVec3dToJson(outValue["normal"], clipPlane.m_normal);
    outValue["distance"] = clipPlane.m_distance;
    if (clipPlane.GetIsInterior())
        outValue["interior"] = true;
    if (clipPlane.GetIsInvisible())
        outValue["invisible"] = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
static ClipPlane ClipPlaneFromJson(JsonValueCR inValue)
    {
    ClipPlane clipPlane;
    DVec3dFromJson(clipPlane.m_normal, inValue["normal"]);
    clipPlane.m_distance = GetDouble(inValue["distance"], 0.0);
    bool invisible = inValue.isMember("invisible");
    bool interior = inValue.isMember("interior");
    clipPlane.SetFlags(invisible, interior);
    return clipPlane;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
static void ClipPlanesToJson(JsonValueR outValue, T_ClipPlanes const& planes)
    {
    for (size_t i=0; i<planes.size(); ++i)
        {
        ClipPlaneToJson(outValue[(Json::ArrayIndex)i], planes[i]);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
static void ClipPlanesFromJson(T_ClipPlanes& planes, JsonValueCR inValue)
    {
    if (!inValue.isArray())
        return;
    for (Json::ArrayIndex i=0; i<inValue.size(); ++i)
        {
        planes.push_back(ClipPlaneFromJson(inValue[(int)i]));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.wilson      3/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void ClipVectorToJson(Json::Value& val, ClipVector const& clipVector)
    {
    Json::ArrayIndex i = 0;
    for (auto clipPrimitive : clipVector)
        {
        Json::Value& clipPrimitiveJson = val[i++];

        //! A ClipPlaneSet is an array of ConvexClipPlaneSet representing the union of all of these sets.
        ClipPlaneSetCP clipPlaneSet = clipPrimitive->GetClipPlanes();
        if (clipPlaneSet != NULL)
            {
            //! A ConvexClipPlaneSet is an array of planes oriented so the intersection of their inside halfspaces is a convex volume.
            Json::ArrayIndex j = 0;
            for (auto convexSet : *clipPlaneSet)
                {
                JsonUtils::ClipPlanesToJson(clipPrimitiveJson[j++], convexSet);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.wilson      3/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void ClipVectorFromJson(ClipVector& clipVector, Json::Value const& val)
    {
    for (Json::ArrayIndex i = 0; i < val.size(); ++i)
        {
        Json::Value const& clipPrimitiveJson = val[i];
        
        ClipPlaneSet clipPlaneSet;
        for (Json::ArrayIndex j = 0; j < clipPrimitiveJson.size(); ++j)
            {
            ConvexClipPlaneSet convexSet;
            JsonUtils::ClipPlanesFromJson(convexSet, clipPrimitiveJson[j]);
            clipPlaneSet.push_back(convexSet);
            }

        clipVector.push_back(ClipPrimitive::CreateFromClipPlanes(clipPlaneSet));
        }
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     06/16
//---------------------------------------------------------------------------------------
template<typename T>
static T IdFromJson(JsonValueCR inValue)
    {
    uint64_t idValue;
    BeStringUtilities::ParseUInt64(idValue, inValue.asCString());
    return T(idValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     06/16
//---------------------------------------------------------------------------------------
template<typename T>
static void IdToJson(JsonValueR outValue, T id)
    {
    Utf8Char buf[32];
    BeStringUtilities::FormatUInt64(buf, id.GetValueUnchecked());
    outValue = buf;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     06/16
//---------------------------------------------------------------------------------------
template<typename T>
static void IdVectorFromJson(bvector<T>& ids, JsonValueCR jsonArray)
    {
    for (Json::ArrayIndex i=0; i<jsonArray.size(); ++i)
        {
        ids.push_back(IdFromJson<T>(jsonArray.get(i, "0")));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     06/16
//---------------------------------------------------------------------------------------
template<typename T>
static void IdVectorToJson(JsonValueR outValue, bvector<T> const& ids)
    {
    outValue = Json::arrayValue;
    for (auto id : ids)
        {
        Json::Value member;
        IdToJson(member, id);
        outValue.append(member);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     06/16
//---------------------------------------------------------------------------------------
template<typename T>
static void IdSetFromJson(BeSQLite::IdSet<T>& ids, JsonValueCR jsonArray)
    {
    for (Json::ArrayIndex i = 0; i<jsonArray.size(); ++i)
        {
        ids.insert(IdFromJson<T>(jsonArray.get(i, "0")));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     06/16
//---------------------------------------------------------------------------------------
template<typename T>
static void IdSetToJson(JsonValueR outValue, BeSQLite::IdSet<T> const& ids)
    {
    outValue = Json::arrayValue;
    for (auto id : ids)
        {
        Json::Value member;
        IdToJson(member, id);
        outValue.append(member);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static Utf8String IdVectorToJsonString(bvector<T> const& ids)
    {
    Json::Value jsonArray;
    IdVectorToJson(jsonArray, ids);
    return Json::FastWriter::ToString(jsonArray);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static bvector<T> IdVectorFromJsonString(Utf8StringCR jsonString)
    {
    Json::Value jsonArray;
    Json::Reader::Parse(jsonString, jsonArray);
    bvector<T> ids;
    IdVectorFromJson(ids, jsonArray);
    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static Utf8String IdSetToJsonString(BeSQLite::IdSet<T> const& ids)
    {
    Json::Value jsonArray;
    IdSetToJson(jsonArray, ids);
    return Json::FastWriter::ToString(jsonArray);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static BeSQLite::IdSet<T> IdSetFromJsonString(Utf8StringCR jsonString)
    {
    Json::Value jsonArray;
    Json::Reader::Parse(jsonString, jsonArray);
    BeSQLite::IdSet<T> ids;
    IdSetFromJson(ids, jsonArray);
    return ids;
    }

};

END_BENTLEY_DGN_NAMESPACE
