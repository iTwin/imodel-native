/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECJsonUtilities.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <rapidjson/BeRapidJson.h>
#include <Bentley/DateTime.h>
#include <Bentley/ByteStream.h>
#include <Geom/GeomApi.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=================================================================================
//! An ECInstance in the JSON Wire Format is formatted as JSON object made up of property value pairs.
//!
//!     - All property names are <b>camel-cased</b>.
//!     - Reserved property names:
//!         - "id": ECInstanceId of the instance or the navigation id within a Navigation Property
//!         - "className": Fully qualified class name ("{schema name}.{class name}")
//!         - "sourceClassName" : Fully qualified class name ("{schema name}.{class name}") of source instance of a relationship instance
//!         - "targetClassName" : Fully qualified class name ("{schema name}.{class name}") of target instance of a relationship instance
//!         - "relClassName" : Fully qualified name of the relationship class of a Navigation Property ("{schema name}.{relationship class name}")
//!         - "x" : x coordinate of a Point2d/Point3d property value
//!         - "y" : y coordinate of a Point2d/Point3d property value
//!         - "z" : z coordinate of a Point3d property value
//!     - Reserved Id properties (e.g. @ref ECJsonSystemNames::Id "id", 
//!         @ref ECJsonSystemNames::SourceId "sourceId", @ref ECJsonSystemNames::TargetId "targetId")
//!        are formatted as <b>hex string</b>.
//!     - Primitive data types are formatted like this:
//!         - Binary/Blob: Base64 string
//!         - Boolean: true / false
//!         - DateTime: DateTime ISO string (see @ref BentleyApi::DateTime::ToString )
//!         - Double: 3.5
//!         - Integer: 2, 
//!         - Long/Int64:
//!             - FormatOptions::Default: as string because of Int64 JavaScript issues
//!             - FormatOptions::LongsAreIds: as hex string
//!         - Bentley.Common.Geometry.IGeometry: JSON object
//!         - Point2d: <code>{"x" : 1.1, "y" : 2.2}</code>
//!         - Point3d: <code>{"x" : 1.1, "y" : 2.2, "z" : 3.3} </code>
//!         - String: "Test"
//!     - Navigation properties are formatted like this:
//!         <code>{ "id" : "<Hex string>"}</code>
//!       or if the navigation property's RelationshipClass is mandatory: 
//!         <code>{ "id" : "<Hex string>", "relClassName" : "{schema name}.{rel class name}" }</code>
//!     - Struct properties are formatted as JSON object
//!     - Array properties are formatted as JSON arrays
// @bsiclass                                               Krischan.Eberle      09/2017
//+===============+===============+===============+===============+===============+======
struct ECJsonSystemNames final
    {
    private:
        ECJsonSystemNames() = delete;
        ~ECJsonSystemNames() = delete;

    public:
        static constexpr Utf8CP Id() { return "$ECInstanceId"; }
        static constexpr Utf8CP ClassName() { return "$ECClassKey"; }

        static constexpr Utf8CP SourceId() { return "$SourceECInstanceId"; }
        static constexpr Utf8CP SourceClassName() { return "$SourceECClassKey"; }
        static constexpr Utf8CP TargetId() { return "$TargetECInstanceId"; }
        static constexpr Utf8CP TargetClassName() { return "$TargetECClassKey"; }

/*      Proposed name changes - not decided yet
        static constexpr Utf8CP Id() { return "id"; }
        static constexpr Utf8CP ClassName() { return "className"; }

        static constexpr Utf8CP SourceId() { return "sourceId"; }
        static constexpr Utf8CP SourceClassName() { return "sourceClassName"; }
        static constexpr Utf8CP TargetId() { return "targetId"; }
        static constexpr Utf8CP TargetClassName() { return "targetClassName"; }
        */
        struct Navigation final
            {
            public:
                static constexpr Utf8CP Id() { return "id"; }
                static constexpr Utf8CP RelClassName() { return "relClassName"; }

            private:
                Navigation() = delete;
                ~Navigation() = delete;
            };

        struct Point final
            {
            public:

                static constexpr Utf8CP X() { return "x"; }
                static constexpr Utf8CP Y() { return "y"; }
                static constexpr Utf8CP Z() { return "z"; }

            private:
                Point() = delete;
                ~Point() = delete;
            };
    };


//=================================================================================
// @bsiclass                                                 Ramanujam.Raman      01/2014
//+===============+===============+===============+===============+===============+======
struct ECJsonUtilities
    {
public:
    //don't use BE_JSON_NAME here so that we can maintain a master definition of the reserved words
    //which each JSON API adoption can use

    static constexpr Json::StaticString json_id() { return Json::StaticString(ECJsonSystemNames::Id()); }
    static constexpr Json::StaticString json_className() { return Json::StaticString(ECJsonSystemNames::ClassName()); }
    static constexpr Json::StaticString json_sourceId() { return Json::StaticString(ECJsonSystemNames::SourceId()); }
    static constexpr Json::StaticString json_sourceClassName() { return Json::StaticString(ECJsonSystemNames::SourceClassName()); }
    static constexpr Json::StaticString json_targetId() { return Json::StaticString(ECJsonSystemNames::TargetId()); }
    static constexpr Json::StaticString json_targetClassName() { return Json::StaticString(ECJsonSystemNames::TargetClassName()); }
    static constexpr Json::StaticString json_navId() { return Json::StaticString(ECJsonSystemNames::Navigation::Id()); }
    static constexpr Json::StaticString json_navRelClassName() { return Json::StaticString(ECJsonSystemNames::Navigation::RelClassName()); }
    static constexpr Json::StaticString json_x() { return Json::StaticString(ECJsonSystemNames::Point::X()); }
    static constexpr Json::StaticString json_y() { return Json::StaticString(ECJsonSystemNames::Point::Y()); }
    static constexpr Json::StaticString json_z() { return Json::StaticString(ECJsonSystemNames::Point::Z()); }

private:
    ECJsonUtilities() = delete;
    ~ECJsonUtilities() = delete;

    static BentleyStatus PointCoordinateFromJson(double&, Json::Value const&, Json::StaticString const& coordinateKey);

    static BentleyStatus ECInstanceFromJson(ECN::IECInstanceR, Json::Value const&, ECN::ECClassCR currentClass, Utf8StringCR currentAccessString, IECClassLocaterR, IECSchemaRemapperCP remapper = nullptr);
    static BentleyStatus ECArrayValueFromJson(ECN::IECInstanceR, Json::Value const&, ECN::ECPropertyCR, Utf8StringCR currentAccessString, IECClassLocaterR);

public:
 
    //! Converts a primitive JSON value to an ECValue
    //! @param[out] value to populate
    //! @param[in] json JSON value
    //! @param[in] type primitive value type
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus ECPrimitiveValueFromJson(ECN::ECValueR value, Json::Value const& json, ECN::PrimitiveType type);

    //! @remarks
    //! Ids are formatted as hex strings (see ECJsonUtilities::IdToJson)
    ECOBJECTS_EXPORT static BentleyStatus ECInstanceFromJson(ECN::IECInstanceR, Json::Value const&, IECClassLocaterR, IECSchemaRemapperCP remapper = nullptr);

    //! Serializes a BeInt64Id into a JSON value.
    //! @remarks Because JavaScript has issues with Int64 values, the id is serialized as <b>hex string</b>
    //! (@see BentleyApi::BeInt64Id::ToHexStr)
    //! @param[out] json JSON value into which the id will be serialized
    //! @param[in] id BeInt64Id to serialize
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus IdToJson(Json::Value& json, BeInt64Id id);
    
    //! Deserializes an id from a JSON value into a BeInt64Id
    //! @remarks Because JavaScript has issues with Int64 values, the id in the JSON value must have been
    //! serialized as <b>hex string</b> (@see BentleyApi::BeInt64Id::ToHexStr)
    //! @param[in] json JSON value containing the id as <b>hex string</b>
    //! @return Deserialized BeInt64Id. In case of error, an invalid BeInt64Id will be returned.
    template<class TBeInt64Id>
    static TBeInt64Id JsonToId(Json::Value const& json)
        {
        TBeInt64Id invalidId;
        invalidId.Invalidate();

        if (!json.isString())
            return invalidId;

        BentleyStatus parseStat = SUCCESS;
        uint64_t idVal = BeStringUtilities::ParseHex(json.asCString(), &parseStat);
        if (SUCCESS != parseStat)
            return invalidId;

        return TBeInt64Id(idVal);
        }

    //! Writes the fully qualified name of an ECClass into a JSON value: {schema name}.{class name}
    //! @param[out] json JSON value
    //! @param[in] ecClass ECClass
    //! @return SUCCESS or ERROR
    static void ClassNameToJson(Json::Value& json, ECClassCR ecClass) { json = Utf8PrintfString("%s.%s", ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str()); }

    //! Looks up an ECClass from a JSON string containing the fully qualified class name
    //! @param[in] json JSON containing the class name
    //! @param[in] locater Class locater to look up the class from the name
    //! @return Found ECClass or nullptr if class could not be found
    ECOBJECTS_EXPORT static ECClassCP GetClassFromClassNameJson(JsonValueCR json, IECClassLocaterR locater);
    
    //! Looks up an ECClassId from a JSON string containing the fully qualified class name
    //! @param[in] json JSON containing the class name
    //! @param[in] locater Class locater to look up the class from the name
    //! @return Retrieved ECClassId. If not found, an invalid ECClassId is returned
    ECOBJECTS_EXPORT static ECClassId GetClassIdFromClassNameJson(JsonValueCR json, IECClassLocaterR locater);

    //! Converts the specified DateTime to a JSON value as ISO8601 string
    //! @param[out] json the resulting ISO8601 string JSON value
    //! @param[in] dateTime DateTime to convert
    //! @see BentleyApi::DateTime::ToString
    static void DateTimeToJson(Json::Value& json, DateTimeCR dateTime) { json = dateTime.ToString();  }

    //! Converts the JSON containing an ISO8691 date time string into a DateTime object.
    //! @param[out] dateTime DateTime
    //! @param[in] json ISO8601 string JSON value
    //! @return SUCCESS or ERROR
    //! @see BentleyApi::DateTime::FromString
    ECOBJECTS_EXPORT static BentleyStatus JsonToDateTime(DateTime& dateTime, JsonValueCR json);

    //! Converts the specified Byte array to a JSON value
    //! The Byte array is converted to a string using a Base64 encoding.
    //! @param[out] json the resulting Base64 string JSON value
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

    //! Converts the specified IGeometry to a JSON value
    //! @param[out] json the resulting IGeometry JSON value
    //! @param[in] geom IGeometry to convert
    //! @return SUCCESS or ERROR
    //! @see BentleyApi::BentleyGeometryJson::TryGeometryToJsonValue
    ECOBJECTS_EXPORT static BentleyStatus IGeometryToJson(Json::Value& json, IGeometryCR geom);

    //! Converts the IGeometry JSON to an IGeometry object
    //! @param[in] json the Json value
    //! @return The deserialized IGeometry object or nullptr in case of errors
    //! @see BentleyApi::BentleyGeometryJson::TryJsonValueToGeometry
    ECOBJECTS_EXPORT static IGeometryPtr JsonToIGeometry(JsonValueCR json);
    };

/*=================================================================================**//**
* @bsiclass                                     Shaun.Sewall                    01/2014
+===============+===============+===============+===============+===============+======*/
struct ECRapidJsonUtilities final
    {
private:
    ECRapidJsonUtilities() = delete;
    ~ECRapidJsonUtilities() = delete;

    static BentleyStatus PointCoordinateFromJson(double&, RapidJsonValueCR, Utf8CP coordinateKey);

    static BentleyStatus ECInstanceFromJson(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, ECN::ECClassCR currentClass, Utf8StringCR currentAccessString, IECClassLocaterR);
    static BentleyStatus ECArrayValueFromJson(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, ECN::ArrayECPropertyCR, Utf8StringCR currentAccessString, IECClassLocaterR);
    static BentleyStatus ECPrimitiveValueFromJson(ECN::ECValueR ecValue, RapidJsonValueCR jsonValue, ECN::PrimitiveType primitiveType);

    static void LogJsonParseError(RapidJsonValueCR, ECN::ECClassCR, Utf8StringCR propAccessString);


public:
    ECOBJECTS_EXPORT static BentleyStatus IdToJson(RapidJsonValueR, BeInt64Id, rapidjson::MemoryPoolAllocator<>&);
    template<class TBeInt64Id>
    static TBeInt64Id JsonToId(RapidJsonValueCR json)
        {
        TBeInt64Id invalidId;
        invalidId.Invalidate();

        if (!json.IsString())
            return invalidId;

        BentleyStatus parseStat = SUCCESS;
        uint64_t idVal = BeStringUtilities::ParseHex(json.GetString(), &parseStat);
        if (SUCCESS != parseStat)
            return invalidId;

        return TBeInt64Id(idVal);
        }

    ECOBJECTS_EXPORT static void ClassToJson(RapidJsonValueR json, ECN::ECClassCR ecClass, rapidjson::MemoryPoolAllocator<>& allocator);
    ECOBJECTS_EXPORT static ECClassCP JsonToClass(RapidJsonValueCR, IECClassLocaterR);
    ECOBJECTS_EXPORT static ECClassId JsonToClassId(RapidJsonValueCR, IECClassLocaterR);

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

    //! Converts the specified DateTime to a JSON value as ISO8601 string
    //! @param[out] json the resulting ISO8601 string JSON value
    //! @param[in] dateTime DateTime to convert
    //! @param[in] allocator Allocator to use to copy the string into the RapidJson value.
    //! @see BentleyApi::DateTime::ToString
    static void DateTimeToJson(RapidJsonValueR json, DateTimeCR dateTime, rapidjson::MemoryPoolAllocator<>& allocator) { Utf8String isoStr = dateTime.ToString(); json.SetString(isoStr.c_str(), (rapidjson::SizeType) isoStr.size(), allocator); }

    //! Converts the JSON containing an ISO8691 date time string into a DateTime object.
    //! @param[out] dateTime DateTime
    //! @param[in] json ISO8601 string JSON value
    //! @return SUCCESS or ERROR
    //! @see BentleyApi::DateTime::FromString
    ECOBJECTS_EXPORT static BentleyStatus JsonToDateTime(DateTime& dateTime, RapidJsonValueCR json);

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

    //! Converts the specified IGeometry to a JSON value
    //! @param[out] json the resulting IGeometry JSON value
    //! @param[in] geom IGeometry to convert
    //! @param[in] allocator Allocator to use to populate the RapidJson value.
    //! @return SUCCESS or ERROR
    //! @see BentleyApi::BentleyGeometryJson::TryGeometryToJsonValue
    ECOBJECTS_EXPORT static BentleyStatus IGeometryToJson(RapidJsonValueR json, IGeometryCR geom, rapidjson::MemoryPoolAllocator<>& allocator);

    //! Converts the IGeometry JSON to an IGeometry object
    //! @param[in] json the JSON value
    //! @return The deserialized IGeometry object or nullptr in case of errors
    //! @see BentleyApi::BentleyGeometryJson::TryJsonValueToGeometry
    ECOBJECTS_EXPORT static IGeometryPtr JsonToIGeometry(RapidJsonValueCR json);

    ECOBJECTS_EXPORT static BentleyStatus ECInstanceFromJson(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, IECClassLocaterR);
    };

/*=================================================================================**//**
* JsonEcInstanceWriter - creates Json object from ECInstance
* If writeFormattedQuanties is true then primitive values with koq specification will be save as a JSON object with rawValue, formattedValue, and fusSpec.
* @bsiclass                                                     Bill.Steinbock  02/2016
+===============+===============+===============+===============+===============+======*/
struct JsonEcInstanceWriter
    {
    private:
        static void          AppendAccessString(Utf8String& compoundAccessString, Utf8String& baseAccessString, const Utf8String& propertyName);
        static StatusInt     WritePropertyValuesOfClassOrStructArrayMember(Json::Value& valueToPopulate, ECN::ECClassCR ecClass, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString, bool writeFormattedQuanties = false, bool serializeNullValues = false);
        static StatusInt     WritePrimitiveValue(Json::Value& valueToPopulate, Utf8CP propertyName, ECN::ECValueCR ecValue, ECN::PrimitiveType propertyType, KindOfQuantityCP koq = nullptr);
        static StatusInt     WriteArrayPropertyValue(Json::Value& valueToPopulate, ECN::ArrayECPropertyR arrayProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString, bool writeFormattedQuanties = false, bool serializeNullValues = false);
        static StatusInt     WriteNavigationPropertyValue(Json::Value& valueToPopulate, ECN::NavigationECPropertyR navigationProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString, bool writeFormattedQuanties = false, bool serializeNullValues = false);
        static StatusInt     WritePrimitivePropertyValue(Json::Value& valueToPopulate, ECN::PrimitiveECPropertyR primitiveProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString, bool writeFormattedQuanties = false, bool serializeNullValues = false);
        static StatusInt     WriteEmbeddedStructPropertyValue(Json::Value& valueToPopulate, ECN::StructECPropertyR structProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString, bool writeFormattedQuanties = false, bool serializeNullValues = false);

    public:
        //! Write the supplied primitive property value as JSON
        //! @param[out] valueToPopulate the JSON object to populate
        //! @param[in] structProperty the property to write
        //! @param[in] ecInstance the IEInstance containing the structProperty
        //! @param[in] baseAccessString The prefix to use to determine the full access string to the property, typically this would be the containing ECStruct's name.
        //! @return SUCCESS or error status.
        ECOBJECTS_EXPORT static StatusInt     WriteEmbeddedStructValue(Json::Value& valueToPopulate, ECN::StructECPropertyR structProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString);

        //! Write the supplied primitive property value as JSON and include presentation data such as KOQ info in the Json
        //! @param[out] valueToPopulate the JSON object to populate
        //! @param[in] structProperty the property to write
        //! @param[in] ecInstance the IEInstance containing the structProperty value
        //! @param[in] baseAccessString The prefix to use to determine the full access string to the property, typically this would be the containing ECStruct's name.
        //! @return SUCCESS or error status.
        ECOBJECTS_EXPORT static StatusInt     WriteEmbeddedStructValueForPresentation(Json::Value& valueToPopulate, ECN::StructECPropertyR structProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString);

        //! Write the supplied primitive property value as JSON
        //! @param[out] valueToPopulate the JSON object to populate
        //! @param[in] primitiveProperty the property to write
        //! @param[in] ecInstance the IEInstance containing the property value
        //! @param[in] baseAccessString The prefix to use to determine the full access string to the property, typically this would be the containing ECStruct name.
        //! @return SUCCESS or error status.
        ECOBJECTS_EXPORT static StatusInt   WritePrimitiveValue(Json::Value& valueToPopulate, PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString);

        //! Write the supplied primitive property value as JSON and include presentation data such as KOQ info in the Json
        //! @param[out] valueToPopulate the JSON object to populate
        //! @param[in] primitiveProperty the property to write
        //! @param[in] ecInstance the IEInstance containing the property value
        //! @param[in] baseAccessString The prefix to use to determine the full access string to the property, typically this would be the containing ECStruct name.
        //! @return SUCCESS or error status.
        ECOBJECTS_EXPORT static StatusInt   WritePrimitiveValueForPresentation(Json::Value& valueToPopulate, PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString);

        //! Write the supplied instance as JSON
        //! @param[out] valueToPopulate the JSON object to populate
        //! @param[in] ecInstance the IEInstance containing the property values
        //! @param[in] instanceName the name of the JSON object that will contain the IEInstance values. This allows the valueToPopulate to contain multiple instances.
        //! @param[in] writeInstanceId if true the instance Id is saved in the JSON data.
        //! @param[in] serializeNullValues If true all values, even null values, of the IECInstance will be serialized.
        //! @return SUCCESS or error status.
        ECOBJECTS_EXPORT static StatusInt     WriteInstanceToJson(Json::Value& valueToPopulate, ECN::IECInstanceCR ecInstance, Utf8CP instanceName, bool writeInstanceId, bool serializeNullValues = false);

        //! Write the supplied instance as JSON and include presentation data such as KOQ info in the Json
        //! @param[out] valueToPopulate the JSON object to populate
        //! @param[in] ecInstance the IEInstance containing the property values
        //! @param[in] instanceName the name of the JSON object that will contain the IEInstance values. This allows the valueToPopulate to contain multiple instances.
        //! @param[in] writeInstanceId if true the instance Id is saved in the JSON data.
        //! @return SUCCESS or error status.
        ECOBJECTS_EXPORT static StatusInt     WriteInstanceToPresentationJson(Json::Value& valueToPopulate, IECInstanceCR ecInstance, Utf8CP instanceName, bool writeInstanceId);
    };


END_BENTLEY_ECOBJECT_NAMESPACE
