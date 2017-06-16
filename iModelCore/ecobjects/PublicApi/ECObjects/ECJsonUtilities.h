/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECJsonUtilities.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/ByteStream.h>
#include <Geom/GeomApi.h>
#include <rapidjson/BeRapidJson.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

#define JSON_POINT_X_KEY "x"
#define JSON_POINT_Y_KEY "y"
#define JSON_POINT_Z_KEY "z"

#define JSON_NAVIGATION_ID_KEY "id"
#define JSON_NAVIGATION_RELECCLASSID_KEY "relECClassId"

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      01/2014
+===============+===============+===============+===============+===============+======*/
struct ECJsonUtilities
    {
private:
    ECJsonUtilities() = delete;
    ~ECJsonUtilities() = delete;

    static BentleyStatus PointCoordinateFromJson(double&, Json::Value const&, Utf8CP coordinateKey);

    static BentleyStatus ECInstanceFromJson(ECN::IECInstanceR, Json::Value const&, ECN::ECClassCR currentClass, Utf8StringCR currentAccessString, IECSchemaRemapperCP remapper = nullptr);
    static BentleyStatus ECArrayValueFromJson(ECN::IECInstanceR, Json::Value const&, ECN::ECPropertyCR, Utf8StringCR currentAccessString);

public:
    //! Populates an ECValue given a Json value and a PrimitiveType
    //! @param[out] value to populate
    //! @param[in] json Json value
    //! @param[in] type primitive value type
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus ECPrimitiveValueFromJson(ECN::ECValueR value, Json::Value const& json, ECN::PrimitiveType type);

    ECOBJECTS_EXPORT static BentleyStatus ECInstanceFromJson(ECN::IECInstanceR, Json::Value const&, IECSchemaRemapperCP remapper = nullptr);

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
    ECOBJECTS_EXPORT static BentleyStatus JsonToBinary(bvector<Byte>& binaryArray, Json::Value const& json);
    //! Converts the specified Json value to a ByteStream
    //! The Json value must hold a BLOB as Base64 encoded string.
    //! @param[out] byteStream the resulting ByteStream
    //! @param[in] json the Json value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToBinary(ByteStream& byteStream, Json::Value const& json);
    //! Converts the specified DPoint2d to a Json value
    //! The point is converted to a Json object with keys "x" and "y".
    //! @param[out] json the resulting Json value
    //! @param[in] pt Point to convert
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus Point2dToJson(Json::Value& json, DPoint2d pt);
    //! Converts the specified Json value to a DPoint2d
    //! The Json value must hold the point as Json object with keys "x" and "y"
    //! @param[out] pt the resulting point
    //! @param[in] json the Json value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToPoint2d(DPoint2d& pt, Json::Value const& json);
    //! Converts the specified DPoint3d to a Json value
    //! The point is converted to a Json object with keys "x", "y" and "z".
    //! @param[out] json the resulting Json value
    //! @param[in] pt Point to convert
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus Point3dToJson(Json::Value& json, DPoint3d pt);
    //! Converts the specified Json value to a DPoint3d
    //! The Json value must hold the point as Json object with keys "x", "y" and "z"
    //! @param[out] pt the resulting point
    //! @param[in] json the Json value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToPoint3d(DPoint3d& pt, Json::Value const& json);
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

    static void LogJsonParseError(RapidJsonValueCR, ECN::ECClassCR, Utf8StringCR propAccessString);

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
    ECOBJECTS_EXPORT static BentleyStatus JsonToBinary(bvector<Byte>& binaryArray, RapidJsonValueCR json);

    //! Converts the specified RapidJsonValue to a ByteStream
    //! The RapidJsonValue must hold the BLOB as Base64 encoded string.
    //! @param[out] byteStream the resulting ByteStream
    //! @param[in] json The RapidJsonValue
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToBinary(ByteStream& byteStream, RapidJsonValueCR json);

    //! Converts the specified DPoint2d to a Json value
    //! The point is converted to a Json object with keys "x", "y" and "z".
    //! @param[out] json the resulting Json value
    //! @param[in] pt Point to convert
    //! @param[in] allocator Allocator to use to populate the RapidJson value.
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus Point2dToJson(RapidJsonValueR json, DPoint2d pt, rapidjson::MemoryPoolAllocator<>& allocator);
    //! Converts the specified Json value to a DPoint2d
    //! The Json value must hold the point as Json object with keys "x" and "y"
    //! @param[out] pt the resulting point
    //! @param[in] json the Json value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToPoint2d(DPoint2d& pt, RapidJsonValueCR json);
    //! Converts the specified DPoint3d to a Json value
    //! The point is converted to a Json object with keys "x", "y" and "z".
    //! @param[out] json the resulting Json value
    //! @param[in] pt Point to convert
    //! @param[in] allocator Allocator to use to populate the RapidJson value.
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus Point3dToJson(RapidJsonValueR json, DPoint3d pt, rapidjson::MemoryPoolAllocator<>& allocator);
    //! Converts the specified Json value to a DPoint3d
    //! The Json value must hold the point as Json object with keys "x", "y" and "z"
    //! @param[out] pt the resulting point
    //! @param[in] json the Json value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToPoint3d(DPoint3d& pt, RapidJsonValueCR json);

    ECOBJECTS_EXPORT static BentleyStatus ECInstanceFromJson(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue);
    };

/*=================================================================================**//**
* JsonEcInstanceWriter - creates Json object from ECInstance
* @bsiclass                                                     Bill.Steinbock  02/2016
+===============+===============+===============+===============+===============+======*/
struct JsonEcInstanceWriter
    {
    private:
        static void          AppendAccessString(Utf8String& compoundAccessString, Utf8String& baseAccessString, const Utf8String& propertyName);
        static StatusInt     WritePropertyValuesOfClassOrStructArrayMember(Json::Value& valueToPopulate, ECN::ECClassCR ecClass, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString);
        static StatusInt     WritePrimitiveValue(Json::Value& valueToPopulate, Utf8CP propertyName, ECN::ECValueCR ecValue, ECN::PrimitiveType propertyType);
        static StatusInt     WriteArrayPropertyValue(Json::Value& valueToPopulate, ECN::ArrayECPropertyR arrayProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString);
        static StatusInt     WriteNavigationPropertyValue(Json::Value& valueToPopulate, ECN::NavigationECPropertyR navigationProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString);
    public:
        ECOBJECTS_EXPORT static StatusInt     WriteEmbeddedStructPropertyValue(Json::Value& valueToPopulate, ECN::StructECPropertyR structProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString);
        ECOBJECTS_EXPORT static StatusInt     WritePrimitivePropertyValue(Json::Value& valueToPopulate, ECN::PrimitiveECPropertyR primitiveProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString);
        ECOBJECTS_EXPORT static StatusInt     WriteInstanceToJson(Json::Value& valueToPopulate, ECN::IECInstanceCR ecInstance, Utf8CP instanceName, bool writeInstanceId);
    };


END_BENTLEY_ECOBJECT_NAMESPACE
