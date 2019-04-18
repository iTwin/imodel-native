/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus JsonPersistenceHelper::BinaryToJson(RapidJsonValueR json, Byte const* binaryArray, size_t binarySize, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    if (binarySize == 0)
        {
        json.SetNull();
        return SUCCESS;
        }

    Utf8String str;
    Base64Utilities::Encode(str, binaryArray, binarySize);

    json.SetString(str.c_str(), (rapidjson::SizeType) str.size(), allocator);
    return SUCCESS;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus JsonPersistenceHelper::JsonToBinary(ByteStream& byteStream, RapidJsonValueCR json)
    {
    if (!json.IsString())
        return ERROR;

    if (json.IsNull())
        {
        byteStream.Clear();
        return SUCCESS;
        }

    Base64Utilities::Decode(byteStream, json.GetString(), (size_t) json.GetStringLength());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus JsonPersistenceHelper::Point2dToJson(RapidJsonValueR json, DPoint2d pt, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    json.SetObject();
    rapidjson::Value coordVal(pt.x);

    json.AddMember(rapidjson::StringRef(PointXMemberName()), coordVal, allocator);
    coordVal.SetDouble(pt.y);
    json.AddMember(rapidjson::StringRef(PointYMemberName()), coordVal, allocator);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus JsonPersistenceHelper::JsonToPoint2d(DPoint2d& pt, RapidJsonValueCR json)
    {
    if (json.IsNull() || !json.IsObject())
        return ERROR;

    double x = 0.0;
    double y = 0.0;

    if (SUCCESS != PointCoordinateFromJson(x, json, PointXMemberName()) ||
        SUCCESS != PointCoordinateFromJson(y, json, PointYMemberName()))
        return ERROR;

    pt = DPoint2d::From(x, y);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus JsonPersistenceHelper::Point3dToJson(RapidJsonValueR json, DPoint3d pt, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    json.SetObject();
    rapidjson::Value coordVal(pt.x);
    json.AddMember(rapidjson::StringRef(PointXMemberName()), coordVal, allocator);
    coordVal.SetDouble(pt.y);
    json.AddMember(rapidjson::StringRef(PointYMemberName()), coordVal, allocator);
    coordVal.SetDouble(pt.z);
    json.AddMember(rapidjson::StringRef(PointZMemberName()), coordVal, allocator);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus JsonPersistenceHelper::JsonToPoint3d(DPoint3d& pt, RapidJsonValueCR json)
    {
    if (json.IsNull() || !json.IsObject())
        return ERROR;

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    if (SUCCESS != PointCoordinateFromJson(x, json, PointXMemberName()) ||
        SUCCESS != PointCoordinateFromJson(y, json, PointYMemberName()) ||
        SUCCESS != PointCoordinateFromJson(z, json, PointZMemberName()))
        return ERROR;

    pt = DPoint3d::From(x, y, z);
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus JsonPersistenceHelper::PointCoordinateFromJson(double& coordinate, RapidJsonValueCR json, Utf8CP coordinateKey)
    {
    auto it = json.FindMember(coordinateKey);
    if (it == json.MemberEnd() || it->value.IsNull() || !it->value.IsNumber())
        return ERROR;

    coordinate = it->value.GetDouble();
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE