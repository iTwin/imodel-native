/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECJsonUtilities.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "ECSchema.h"
#include "ECInstance.h"
#include <Geom/GeomApi.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <rapidjson/BeRapidJson.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

#define JSON_POINT_X_KEY "x"
#define JSON_POINT_Y_KEY "y"
#define JSON_POINT_Z_KEY "z"

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      01/2014
+===============+===============+===============+===============+===============+======*/
struct ECJsonUtilities
    {
private:
    ECJsonUtilities();
    ~ECJsonUtilities();

    static BentleyStatus PointCoordinateFromJson(double&, Json::Value const&, Utf8CP coordinateKey);

    static BentleyStatus ECInstanceFromJson(ECN::IECInstanceR, Json::Value const&, ECN::ECClassCR currentClass, Utf8StringCR currentAccessString);
    static BentleyStatus ECArrayValueFromJson(ECN::IECInstanceR, Json::Value const&, ECN::ECPropertyCR, Utf8StringCR currentAccessString);
    static BentleyStatus ECPrimitiveValueFromJson(ECN::ECValueR, Json::Value const&, ECN::PrimitiveType);

public:
    ECOBJECTS_EXPORT static BentleyStatus ECInstanceFromJson(ECN::IECInstanceR, Json::Value const&);

    //! Converts the specified Byte array to a Json value
    //! The Byte array is converted to a string using a Base64 encoding.
    //! @param[out] json the resulting string Json value
    //! @param[in] binaryArray the Byte array
    //! @param[in] binarySize size of the Byte array
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus BinaryToJson(Json::Value& json, Byte const* binaryArray, size_t binarySize);
    //! Converts the specified Json value to a Byte array
    //! The Json value must hold the Byte array as Base64 encoded string.
    //! @param[out] binaryArray the resulting Byte array
    //! @param[in] json the Json value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToBinary(bvector<Byte>&, Json::Value const&);
    //! Converts the specified DPoint2d to a Json value
    //! The point is converted to a Json object with keys "x" and "y".
    //! @param[out] json the resulting Json value
    //! @param[in] pt Point to convert
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus Point2DToJson(Json::Value& json, DPoint2d pt);
    //! Converts the specified Json value to a DPoint2d
    //! The Json value must hold the point as Json object with keys "x" and "y"
    //! @param[out] pt the resulting point
    //! @param[in] json the Json value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToPoint2D(DPoint2d& pt, Json::Value const& json);
    //! Converts the specified DPoint3d to a Json value
    //! The point is converted to a Json object with keys "x", "y" and "z".
    //! @param[out] json the resulting Json value
    //! @param[in] pt Point to convert
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus Point3DToJson(Json::Value& json, DPoint3d pt);
    //! Converts the specified Json value to a DPoint3d
    //! The Json value must hold the point as Json object with keys "x", "y" and "z"
    //! @param[out] pt the resulting point
    //! @param[in] json the Json value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToPoint3D(DPoint3d& pt, Json::Value const& json);
    };

/*=================================================================================**//**
* @bsiclass                                     Shaun.Sewall                    01/2014
+===============+===============+===============+===============+===============+======*/
struct ECRapidJsonUtilities
    {
private:
    ECRapidJsonUtilities();
    ~ECRapidJsonUtilities();

    static BentleyStatus PointCoordinateFromJson(double&, RapidJsonValueCR, Utf8CP coordinateKey);

    static BentleyStatus ECInstanceFromJson(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, ECN::ECClassCR currentClass, Utf8StringCR currentAccessString);
    static BentleyStatus ECArrayValueFromJson(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, ECN::ECPropertyCR, Utf8StringCR currentAccessString);
    static BentleyStatus ECPrimitiveValueFromJson(ECN::ECValueR ecValue, RapidJsonValueCR jsonValue, ECN::PrimitiveType primitiveType);

public:
    //! Return an Int64 value from a RapidJsonValueCR that may be a number or a string.
    //! @param[in] json the source RapidJsonValueCR
    //! @param[in] defaultOnError the Int64 value to return if it is not possible to convert the source value to Int64.
    //! @return the resulting Int64 value
    ECOBJECTS_EXPORT static int64_t Int64FromJson(RapidJsonValueCR json, int64_t defaultOnError = 0);
    
    //! Converts the specified Int64 value to a RapidJsonValue
    //! Because of Javascript having issues holding Int64, this method creates a RapidJsonValue that
    //! holds the Int64 as string
    //! @param[out] json the resulting string RapidJsonValue
    //! @param[in] val the Int64 value
    //! @param[in] allocator Allocator to use to copy the string into the RapidJson value.
    ECOBJECTS_EXPORT static void Int64ToStringJsonValue(RapidJsonValueR json, int64_t val, rapidjson::MemoryPoolAllocator<>& allocator);

    //! Converts the specified Byte array to a RapidJsonValue
    //! The Byte array is converted to a string using a Base64 encoding.
    //! @param[out] json the resulting string RapidJsonValue
    //! @param[in] binaryArray the Byte array
    //! @param[in] binarySize size of the Byte array
    //! @param[in] allocator Allocator to use to copy the string into the RapidJson value.
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus BinaryToJson(RapidJsonValueR json, Byte const* binaryArray, size_t binarySize, rapidjson::MemoryPoolAllocator<>& allocator);

    //! Converts the specified RapidJsonValue to a Byte array
    //! The RapidJsonValue must hold the Byte array as Base64 encoded string.
    //! @param[out] binaryArray the resulting Byte array
    //! @param[in] json The RapidJsonValue
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToBinary(bvector<Byte>& byteArray, RapidJsonValueCR json);

    //! Converts the specified DPoint2d to a Json value
    //! The point is converted to a Json object with keys "x", "y" and "z".
    //! @param[out] json the resulting Json value
    //! @param[in] pt Point to convert
    //! @param[in] allocator Allocator to use to populate the RapidJson value.
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus Point2DToJson(RapidJsonValueR json, DPoint2d pt, rapidjson::MemoryPoolAllocator<>& allocator);
    //! Converts the specified Json value to a DPoint2d
    //! The Json value must hold the point as Json object with keys "x" and "y"
    //! @param[out] pt the resulting point
    //! @param[in] json the Json value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToPoint2D(DPoint2d& pt, RapidJsonValueCR json);
    //! Converts the specified DPoint3d to a Json value
    //! The point is converted to a Json object with keys "x", "y" and "z".
    //! @param[out] json the resulting Json value
    //! @param[in] pt Point to convert
    //! @param[in] allocator Allocator to use to populate the RapidJson value.
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus Point3DToJson(RapidJsonValueR json, DPoint3d pt, rapidjson::MemoryPoolAllocator<>& allocator);
    //! Converts the specified Json value to a DPoint3d
    //! The Json value must hold the point as Json object with keys "x", "y" and "z"
    //! @param[out] pt the resulting point
    //! @param[in] json the Json value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToPoint3D(DPoint3d& pt, RapidJsonValueCR json);

    ECOBJECTS_EXPORT static BentleyStatus ECInstanceFromJson(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
