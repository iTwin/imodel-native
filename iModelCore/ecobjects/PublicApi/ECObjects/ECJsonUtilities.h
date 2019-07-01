/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <json/json.h>
#include <BeRapidJson/BeRapidJson.h>
#include <Bentley/DateTime.h>
#include <Bentley/ByteStream.h>
#include <Geom/GeomApi.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//=================================================================================
//! Formatting options for ECProperty values of the type PrimitiveType::PRIMITIVETYPE_Long "Long" / Int64
//! @remarks Because of JavaScript issues with 64 bit numbers, in many use cases Int64 values cannot be formatted as number
//! without data loss. Using one of the other options from this enumeration allows to workaround that issue.
// @bsienum                                                    09/2017
//+===============+===============+===============+===============+===============+======
enum class ECJsonInt64Format
    {
    //! The Int64 number is formatted as is. I.e. this causes data loss in JavaScript. Only use this option 
    //! if the JSON is not supposed to be used in JavaScript.
    AsNumber,
    AsDecimalString, //!< The Int64 number is formatted as decimal string.
    AsHexadecimalString //!< The Int64 number is formatted as hexadecimal string.
    };

//=================================================================================
//! An ECInstance in the ECJSON Format is formatted as JSON object made up of property value pairs.
//!
//! ### ECJSON Format Description
//!     - <b>Reserved member names</b>:
//!         - "id": ECInstanceId of the instance or the navigation id within a %Navigation property
//!         - "className": Fully qualified class name ("<schema name>.<class name>")
//!         - "sourceId" : ECInstanceId of source instance of a relationship instance
//!         - "sourceClassName" : Fully qualified class name ("<schema name>.<class name>") of source instance of a relationship instance
//!         - "targetId" : ECInstanceId of target instance of a relationship instance
//!         - "targetClassName" : Fully qualified class name ("<schema name>.<class name>") of target instance of a relationship instance
//!     - Reserved member names that only occur nested in %Navigation or Point properties:
//!         - "relClassName" : Fully qualified name of the relationship class of a %Navigation property ("<schema name>.<relationship class name>")
//!         - "x" : x coordinate of a Point2d/Point3d property value
//!         - "y" : y coordinate of a Point2d/Point3d property value
//!         - "z" : z coordinate of a Point3d property value
//!     - Reserved Id properties (e.g. @ref ECJsonSystemNames::Id "id", ECJsonSystemNames::Navigation::Id,
//!         @ref ECJsonSystemNames::SourceId "sourceId", @ref ECJsonSystemNames::TargetId "targetId")
//!        are formatted as <b>hexadecimal string</b> (see BentleyApi::BeInt64Id::ToHexStr).
//!     - Primitive data type formatting:
//!         - Binary/Blob: Base64 string (see BentleyApi::Base64Utilities )
//!         - Boolean: true / false
//!         - DateTime: DateTime ISO string (see BentleyApi::DateTime::ToString )
//!         - Double: 3.5
//!         - Integer: 2, 
//!         - Long/Int64: Because of JavaScript issues with Int64, these format options exist (see BentleyApi::ECN::ECJsonInt64Format):
//!             - ECJsonInt64Format::AsNumber: as number
//!             - ECJsonInt64Format::AsDecimalString: as string because of Int64 JavaScript issues
//!             - ECJsonInt64Format::AsHexadecimalString: as hex string because of Int64 JavaScript issues
//!         - Bentley.Common.Geometry.IGeometry: JSON object
//!         - Point2d: <code>{"x" : 1.1, "y" : 2.2}</code>
//!         - Point3d: <code>{"x" : 1.1, "y" : 2.2, "z" : 3.3} </code>
//!         - String: "Hello, world"
//!     - Navigation properties are formatted like this:
//!         <code>{ "id" : "<Hex string>"}</code>
//!       or if the navigation property's RelationshipClass is mandatory: 
//!         <code>{ "id" : "<Hex string>", "relClassName" : "<schema name>.<rel class name>" }</code>
//!     - Struct properties are formatted as JSON object
//!     - Array properties are formatted as JSON arrays
//!
// @bsiclass                                               09/2017
//+===============+===============+===============+===============+===============+======
struct ECJsonSystemNames final
    {
    public:
        static constexpr Utf8CP Id() { return "id"; }
        static constexpr Utf8CP ClassName() { return "className"; }

        static constexpr Utf8CP SourceId() { return "sourceId"; }
        static constexpr Utf8CP SourceClassName() { return "sourceClassName"; }
        static constexpr Utf8CP TargetId() { return "targetId"; }
        static constexpr Utf8CP TargetClassName() { return "targetClassName"; }

        //! System member names for the representation of BentleyApi::ECN::NavigationECProperty values
        //! in the ECJSON
        struct Navigation final
            {
            public:
                static constexpr Utf8CP Id() { return "id"; }
                static constexpr Utf8CP RelClassName() { return "relClassName"; }

            private:
                Navigation() = delete;
                ~Navigation() = delete;

            public:
                static bool IsSystemMember(Utf8StringCR memberName) { return memberName.Equals(Id()) || memberName.Equals(RelClassName()); }
            };

        //! System member names for the representation of Point property values in the ECJSON
        struct Point final
            {
            public:

                static constexpr Utf8CP X() { return "x"; }
                static constexpr Utf8CP Y() { return "y"; }
                static constexpr Utf8CP Z() { return "z"; }

            private:
                Point() = delete;
                ~Point() = delete;

            public:
                static bool IsSystemMember(Utf8StringCR memberName) { return memberName.Equals(X()) || memberName.Equals(Y()) || memberName.Equals(Z()); }
            };

    private:
        ECJsonSystemNames() = delete;
        ~ECJsonSystemNames() = delete;

    public:
        //!Checks whether @p topLevelMemberName is a system name for top-level members of the ECJSON object.
        //! @remarks System names that can only occur in nested members (ECJsonSystemNames::Navigation or ECJsonSystemNames::Point)
        //! are not considered by the check.
        //! @param[in] topLevelMemberName Name of top-level member of ECJSON object to check
        //! @return true or false
        static bool IsTopLevelSystemMember(Utf8StringCR topLevelMemberName) { return topLevelMemberName.Equals(Id()) || topLevelMemberName.Equals(ClassName()) || topLevelMemberName.Equals(SourceId()) || topLevelMemberName.Equals(TargetId()) || topLevelMemberName.Equals(SourceClassName()) || topLevelMemberName.Equals(TargetClassName()); }
    };


//=================================================================================
// @bsiclass                                                 Ramanujam.Raman      01/2014
//+===============+===============+===============+===============+===============+======
struct ECJsonUtilities
    {
public:

    //don't use BE_JSON_NAME here so that we can maintain a master definition of the reserved words
    //which each JSON API adoption can use

    //! @name Methods for JSON values of the JsonCpp API
    //! @{

    //! @ref ECN::ECJsonSystemNames::Id "ECJsonSystemNames::Id" as JsonCpp StaticString
    static constexpr Json::StaticString json_id() { return Json::StaticString(ECJsonSystemNames::Id()); }
    //! @ref ECN::ECJsonSystemNames::ClassName "ECJsonSystemNames::ClassName" as JsonCpp StaticString
    static constexpr Json::StaticString json_className() { return Json::StaticString(ECJsonSystemNames::ClassName()); }
    //! @ref ECN::ECJsonSystemNames::SourceId "ECJsonSystemNames::SourceId" as JsonCpp StaticString
    static constexpr Json::StaticString json_sourceId() { return Json::StaticString(ECJsonSystemNames::SourceId()); }
    //! @ref ECN::ECJsonSystemNames::SourceClassName "ECJsonSystemNames::SourceClassName" as JsonCpp StaticString
    static constexpr Json::StaticString json_sourceClassName() { return Json::StaticString(ECJsonSystemNames::SourceClassName()); }
    //! @ref ECN::ECJsonSystemNames::TargetId "ECJsonSystemNames::TargetId" as JsonCpp StaticString
    static constexpr Json::StaticString json_targetId() { return Json::StaticString(ECJsonSystemNames::TargetId()); }
    //! @ref ECN::ECJsonSystemNames::TargetClassName "ECJsonSystemNames::TargetClassName" as JsonCpp StaticString
    static constexpr Json::StaticString json_targetClassName() { return Json::StaticString(ECJsonSystemNames::TargetClassName()); }
    //! @ref ECN::ECJsonSystemNames::Navigation::Id "ECJsonSystemNames::Navigation::Id" as JsonCpp StaticString
    static constexpr Json::StaticString json_navId() { return Json::StaticString(ECJsonSystemNames::Navigation::Id()); }
    //! @ref ECN::ECJsonSystemNames::Navigation::RelClassName "ECJsonSystemNames::Navigation::RelClassName" as JsonCpp StaticString
    static constexpr Json::StaticString json_navRelClassName() { return Json::StaticString(ECJsonSystemNames::Navigation::RelClassName()); }
    //! @ref ECN::ECJsonSystemNames::Point::X "ECJsonSystemNames::Point::X" as JsonCpp StaticString
    static constexpr Json::StaticString json_x() { return Json::StaticString(ECJsonSystemNames::Point::X()); }
    //! @ref ECN::ECJsonSystemNames::Point::Y "ECJsonSystemNames::Point::Y" as JsonCpp StaticString
    static constexpr Json::StaticString json_y() { return Json::StaticString(ECJsonSystemNames::Point::Y()); }
    //! @ref ECN::ECJsonSystemNames::Point::Z "ECJsonSystemNames::Point::Z" as JsonCpp StaticString
    static constexpr Json::StaticString json_z() { return Json::StaticString(ECJsonSystemNames::Point::Z()); }
    //! @}

private:
    ECJsonUtilities() = delete;
    ~ECJsonUtilities() = delete;

    static BentleyStatus PointCoordinateFromJson(double&, Json::Value const&, Json::StaticString const& coordinateKey);
    static BentleyStatus PointCoordinateFromJson(double&, RapidJsonValueCR, Utf8CP coordinateKey);

public:
    //! Generates the fully qualified name of an ECClass as used in the ECJSON format: &lt;schema name&gt;.&lt;class name&gt;
    //! @param[in] ecClass ECClass
    //! @return Fully qualified class name for the ECJSON format
    static Utf8String FormatClassName(ECClassCR ecClass) {return Utf8PrintfString("%s.%s", ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str());}

    //! Generates the fully qualified name of an PropertyCategory as used in the ECJSON format: &lt;schema name&gt;.&lt;PropertyCategory name&gt;
    //! @param[in] ecPropertyCategory PropertyCategory
    //! @return Fully qualified property category name for the ECJSON format
    static Utf8String FormatPropertyCategoryName(PropertyCategoryCR ecPropertyCategory) {return Utf8PrintfString("%s.%s", ecPropertyCategory.GetSchema().GetName().c_str(), ecPropertyCategory.GetName().c_str());}

    //! Generates the fully qualified name of an ECEnumeration as used in the ECJSON format: &lt;schema name&gt;.&lt;Enumeration name&gt;
    //! @param[in] ecEnumeration ECEnumeration
    //! @return Fully qualified enumeration name for the ECJSON format
    static Utf8String FormatEnumerationName(ECEnumerationCR ecEnumeration) {return Utf8PrintfString("%s.%s", ecEnumeration.GetSchema().GetName().c_str(), ecEnumeration.GetName().c_str());}

    //! Generates the fully qualified name of a KindOfQuantity as used in the ECJSON format: &lt;schema name&gt;.&lt;KindOfQuantity name&gt;
    //! @param[in] koq KindOfQuantity
    //! @return Fully qualified KindOfQuantity name for the ECJSON format
    static Utf8String FormatKindOfQuantityName(KindOfQuantityCR koq) {return Utf8PrintfString("%s.%s", koq.GetSchema().GetName().c_str(), koq.GetName().c_str());}

    //! Lowers the first char of the specified string.
    //! @remarks Use this method to make a name, e.g. an ECProperty name a JSON member name.
    //! @param[in,out] str String to lower its first character
    static void LowerFirstChar(Utf8StringR str) { str[0] = (Utf8Char) tolower(str[0]); }

    //! @name Methods for JSON values of the JsonCpp API
    //! @{

    //! Writes the fully qualified name of an ECClass into a JSON value: {schema name}.{class name}
    //! @param[out] json JSON value
    //! @param[in] ecClass ECClass
    //! @return SUCCESS or ERROR
    static void ClassNameToJson(Json::Value& json, ECClassCR ecClass) { json = FormatClassName(ecClass); }

    //! Returns a fully qualified name of any SchemaChild
    //! Type must have both GetName() and GetSchema() methods
    //! @param[in] ec The schema child to extract name from
    //! return A string containing the fully qualified name
    template<typename T>
    static Utf8String ECNameToJsonName(T const& ec)
        {
        return ec.GetSchema().GetName() + "." + ec.GetName();
        }

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

    //! Converts a BeInt64Id to a JSON value.
    //! @remarks Because JavaScript has issues with Int64 values, the id is serialized as <b>hex string</b>
    //! (@see BentleyApi::BeInt64Id::ToHexStr)
    //! @param[out] json resulting JSON value
    //! @param[in] id BeInt64Id to convert
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus IdToJson(Json::Value& json, BeInt64Id id);

    //! Converts an id from a JSON value to a BeInt64Id
    //! @remarks The JSON must contain the Id value in one of the formats of BentleyApi::ECN::ECJsonInt64Format.
    //! @param[in] json JSON value containing the id
    //! @return Resulting BeInt64Id. In case of error, an invalid BeInt64Id will be returned.
    template<class TBeInt64Id>
    static TBeInt64Id JsonToId(Json::Value const& json)
        {
        int64_t val = 0;
        if (SUCCESS != JsonToInt64(val, json))
            {
            TBeInt64Id invalidId;
            invalidId.Invalidate();
            return invalidId;
            }

        return TBeInt64Id((uint64_t) val);
        }

    //! Converts an Int64 into a JSON value.
    //! @param[out] json resulting JSON value. 
    //! @param[in] int64Val Int64 value to format
    //! @param[in] int64Format Options for how to format the Int64 value
    ECOBJECTS_EXPORT static void Int64ToJson(Json::Value& json, int64_t int64Val, ECJsonInt64Format int64Format = ECJsonInt64Format::AsDecimalString);

    //! Converts a JSON value to an Int64 number
    //! @remarks The JSON must contain the Int64 value in one of the formats of BentleyApi::ECN::ECJsonInt64Format.
    //! @param[out] int64Val resulting Int64 value
    //! @param[in] json JSON containing the Int64 value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToInt64(int64_t& int64Val, JsonValueCR json);

    //! Converts the specified DateTime to a JSON value as ISO8601 string.
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
    //! @param[in] header this will put before the base64 encoded output
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus BinaryToJson(Json::Value& json, Byte const* binaryArray, size_t binarySize, Utf8CP header = nullptr);

    //! Converts the specified JSON value to a Byte array
    //! The JSON value must hold the Byte array as Base64 encoded string.
    //! @param[out] binaryArray the resulting Byte array
    //! @param[in] json the JSON value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToBinary(bvector<Byte>& binaryArray, Json::Value const& json);
    //! Converts the specified JSON value to a ByteStream
    //! The JSON value must hold a BLOB as Base64 encoded string.
    //! @param[out] byteStream the resulting ByteStream
    //! @param[in] json the Json value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToBinary(ByteStream& byteStream, Json::Value const& json);
    //! Converts the specified DPoint2d to a JSON value
    //! The point is converted to a JSON object with keys "x" and "y" (see BentleyApi::ECN::ECJsonSystemNames::Point).
    //! @param[out] json the resulting JSON value
    //! @param[in] pt Point to convert
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus Point2dToJson(Json::Value& json, DPoint2d pt);
    //! Converts the specified JSON value to a DPoint2d
    //! The JSON value must hold the point as JSON object with keys "x" and "y" (see BentleyApi::ECN::ECJsonSystemNames::Point).
    //! @param[out] pt the resulting point
    //! @param[in] json the JSON value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToPoint2d(DPoint2d& pt, Json::Value const& json);
    //! Converts the specified DPoint3d to a JSON value
    //! The point is converted to a JSON object with keys "x", "y" and "z" (see BentleyApi::ECN::ECJsonSystemNames::Point).
    //! @param[out] json the resulting JSON value
    //! @param[in] pt Point to convert
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus Point3dToJson(Json::Value& json, DPoint3d pt);
    //! Converts the specified JSON value to a DPoint3d
    //! The JSON value must hold the point as JSON object with keys "x", "y" and "z" (see BentleyApi::ECN::ECJsonSystemNames::Point).
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

    //! Converts the specified IGeometry to an iModel JSON value
    //! @param[out] json the resulting IGeometry IModel JSON value
    //! @param[in] geom IGeometry to convert
    //! @return SUCCESS or ERROR
    //! @see BentleyApi::BentleyGeometryJson::TryGeometryToIModelJsonValue
    ECOBJECTS_EXPORT static BentleyStatus IGeometryToIModelJson(Json::Value& json, IGeometryCR geom);

    //! Converts the IGeometry JSON or IModel Json to an IGeometry object
    //! @param[in] json the JSON value
    //! @return The IGeometry object or nullptr in case of errors
    //! @see BentleyApi::BentleyGeometryJson::TryJsonValueToGeometry
    ECOBJECTS_EXPORT static IGeometryPtr JsonToIGeometry(JsonValueCR json);

    //! @}

    //! @name Methods for JSON values of the RapidJson API
    //! @{

    //! Writes the fully qualified name of an ECClass into a JSON value: {schema name}.{class name}
    //! @param[out] json JSON value
    //! @param[in] ecClass ECClass
    //! @param[in] allocator Allocator to use to copy the string into the RapidJson value.
    ECOBJECTS_EXPORT static void ClassToJson(RapidJsonValueR json, ECN::ECClassCR ecClass, rapidjson::MemoryPoolAllocator<>& allocator);
    //! Looks up an ECClass from a JSON string containing the fully qualified class name
    //! @param[in] json JSON containing the class name
    //! @param[in] locater Class locater to look up the class from the name
    //! @return Found ECClass or nullptr if class could not be found
    ECOBJECTS_EXPORT static ECClassCP GetClassFromClassNameJson(RapidJsonValueCR json, IECClassLocaterR locater);
    //! Looks up an ECClassId from a JSON string containing the fully qualified class name
    //! @param[in] json JSON containing the class name
    //! @param[in] locater Class locater to look up the class from the name
    //! @return Retrieved ECClassId. If not found, an invalid ECClassId is returned
    ECOBJECTS_EXPORT static ECClassId GetClassIdFromClassNameJson(RapidJsonValueCR json, IECClassLocaterR locater);

    //! Converts a JSON to an Int64 value.
    //! @remarks The JSON must contain the Int64 value in one of the formats of BentleyApi::ECN::ECJsonInt64Format.
    //! @param[out] val the resulting Int64 value
    //! @param[in] json the source RapidJsonValueCR
    //! @return SUCCESS OR ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToInt64(int64_t& val, RapidJsonValueCR json);

    //! Converts the specified Int64 value to a RapidJson value
    //! @param[out] json the resulting JSON
    //! @param[in] val the Int64 value
    //! @param[in] allocator Allocator to use to copy into the RapidJson value.
    //! @param[in] int64Format Options for how to format the Int64 value
    ECOBJECTS_EXPORT static void Int64ToJson(RapidJsonValueR json, int64_t val, rapidjson::MemoryPoolAllocator<>& allocator, ECJsonInt64Format int64Format = ECJsonInt64Format::AsDecimalString);

    //! Converts a BeInt64Id to a JSON value.
    //! @remarks Because JavaScript has issues with Int64 values, the id is serialized as <b>hex string</b>
    //! (@see BentleyApi::BeInt64Id::ToHexStr)
    //! @param[out] json resulting JSON value
    //! @param[in] id BeInt64Id to convert
    //! @param[in] allocator Allocator to use to copy into the RapidJson value.
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus IdToJson(RapidJsonValueR json, BeInt64Id id, rapidjson::MemoryPoolAllocator<>& allocator);

    //! Converts an id from a JSON value to a BeInt64Id
    //! @remarks The JSON must contain the Id value in one of the formats of BentleyApi::ECN::ECJsonInt64Format.
    //! @param[in] json JSON value containing the id
    //! @return Resulting BeInt64Id. In case of error, an invalid BeInt64Id will be returned.
    template<class TBeInt64Id>
    static TBeInt64Id JsonToId(RapidJsonValueCR json)
        {
        int64_t val = 0;
        if (SUCCESS != JsonToInt64(val, json))
            {
            TBeInt64Id invalidId;
            invalidId.Invalidate();
            return invalidId;
            }

        return TBeInt64Id((uint64_t) val);
        }

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

    //! Converts the specified Byte array to a RapidJson value
    //! The Byte array is converted to a string using a Base64 encoding.
    //! @param[out] json the resulting string JSON
    //! @param[in] binaryArray the Byte array
    //! @param[in] binarySize size of the Byte array
    //! @param[in] allocator Allocator to use to copy the string into the RapidJson value.
    //! @param[in] header this will put before the base64 encoded output
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus BinaryToJson(RapidJsonValueR json, Byte const* binaryArray, size_t binarySize, rapidjson::MemoryPoolAllocator<>& allocator, Utf8CP header = nullptr);

    //! Converts the specified RapidJson value to a Byte array
    //! The RapidJson value must hold the Byte array as Base64 encoded string.
    //! @param[out] binaryArray the resulting Byte array
    //! @param[in] json The RapidJson value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToBinary(bvector<Byte>& binaryArray, RapidJsonValueCR json);

    //! Converts the specified RapidJson value to a ByteStream
    //! The RapidJson value must hold the BLOB as Base64 encoded string.
    //! @param[out] byteStream the resulting ByteStream
    //! @param[in] json The RapidJson value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToBinary(ByteStream& byteStream, RapidJsonValueCR json);

    //! Converts the specified DPoint2d to a JSON value
    //! The point is converted to a JSON object with keys "x" and "y" (see ECN::ECJsonSystemNames::Point).
    //! @param[out] json the resulting Json value
    //! @param[in] pt Point to convert
    //! @param[in] allocator Allocator to use to populate the RapidJson value.
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus Point2dToJson(RapidJsonValueR json, DPoint2d pt, rapidjson::MemoryPoolAllocator<>& allocator);
    //! Converts the specified JSON value to a DPoint2d
    //! The JSON value must hold the point as JSON object with keys "x" and "y" (see ECN::ECJsonSystemNames::Point).
    //! @param[out] pt the resulting point
    //! @param[in] json the RapidJson value.
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToPoint2d(DPoint2d& pt, RapidJsonValueCR json);
    //! Converts the specified DPoint3d to a JSON value
    //! The point is converted to a JSON object with keys "x", "y" and "z" (see ECN::ECJsonSystemNames::Point).
    //! @param[out] json the resulting RapidJson value.
    //! @param[in] pt Point to convert
    //! @param[in] allocator Allocator to use to populate the RapidJson value.
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus Point3dToJson(RapidJsonValueR json, DPoint3d pt, rapidjson::MemoryPoolAllocator<>& allocator);
    //! Converts the specified JSON value to a DPoint3d
    //! The JSON value must hold the point as JSON object with keys "x", "y" and "z" (see ECN::ECJsonSystemNames::Point).
    //! @param[out] pt the resulting point
    //! @param[in] json the JSON value
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT static BentleyStatus JsonToPoint3d(DPoint3d& pt, RapidJsonValueCR json);

    //! Converts the specified IGeometry to a JSON value
    //! @param[out] json the resulting IGeometry JSON value
    //! @param[in] geom IGeometry to convert
    //! @param[in] allocator Allocator to use to populate the RapidJson value.
    //! @return SUCCESS or ERROR
    //! @see BentleyApi::BentleyGeometryJson::TryGeometryToJsonValue
    ECOBJECTS_EXPORT static BentleyStatus IGeometryToJson(RapidJsonValueR json, IGeometryCR geom, rapidjson::MemoryPoolAllocator<>& allocator);

    //! Converts the specified IGeometry to a IModel JSON value
    //! @param[out] json the resulting IGeometry IModel JSON value
    //! @param[in] geom IGeometry to convert
    //! @param[in] allocator Allocator to use to populate the RapidJson value.
    //! @return SUCCESS or ERROR
    //! @see BentleyApi::BentleyGeometryJson::TryGeometryToJsonValue
    ECOBJECTS_EXPORT static BentleyStatus IGeometryToIModelJson(RapidJsonValueR json, IGeometryCR geom, rapidjson::MemoryPoolAllocator<>& allocator);

    //! Converts the IGeometry JSON or IModel Json to an IGeometry object
    //! @param[in] json the JSON value
    //! @return The resulting IGeometry object or nullptr in case of errors
    //! @see BentleyApi::BentleyGeometryJson::TryJsonValueToGeometry
    ECOBJECTS_EXPORT static IGeometryPtr JsonToIGeometry(RapidJsonValueCR json);
    //! @}
    };

//=================================================================================
//! Populates an ECInstance from a JSON in the @ref ECN::ECJsonSystemNames "ECJSON Format".
// @bsiclass                                                 Ramanujam.Raman      01/2014
//+===============+===============+===============+===============+===============+======
struct JsonECInstanceConverter final
    {
    private:
        JsonECInstanceConverter() = delete;
        ~JsonECInstanceConverter() = delete;

        //JsonCpp
        static BentleyStatus JsonToECInstance(ECN::IECInstanceR, Json::Value const&, ECN::ECClassCR currentClass, Utf8StringCR currentAccessString, IECClassLocaterR, bool ignoreUnknownProperties = false, IECSchemaRemapperCP remapper = nullptr, std::function<bool(Utf8CP)> shouldSerializeProperty = nullptr);
        static BentleyStatus JsonToPrimitiveECValue(ECN::ECValueR value, Json::Value const& json, ECN::PrimitiveType type);
        static BentleyStatus JsonToArrayECValue(ECN::IECInstanceR, Json::Value const&, ECN::ArrayECPropertyCR, Utf8StringCR currentAccessString, IECClassLocaterR);

        //rapidjson
        static BentleyStatus JsonToECInstance(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, ECN::ECClassCR currentClass, Utf8StringCR currentAccessString, IECClassLocaterR);
        static BentleyStatus JsonToPrimitiveECValue(ECN::ECValueR ecValue, RapidJsonValueCR jsonValue, ECN::PrimitiveType primitiveType);
        static BentleyStatus JsonToArrayECValue(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, ECN::ArrayECPropertyCR, Utf8StringCR currentAccessString, IECClassLocaterR);

    public:
        ECOBJECTS_EXPORT static BentleyStatus JsonToECInstance(ECN::IECInstanceR instance, Json::Value const& jsonValue, IECClassLocaterR classLocater, bool ignoreUnknownProperties = false, IECSchemaRemapperCP remapper = nullptr);
        ECOBJECTS_EXPORT static BentleyStatus JsonToECInstance(ECN::IECInstanceR instance, Json::Value const& jsonValue, IECClassLocaterR classLocater, std::function<bool(Utf8CP)> shouldSerializeProperty);
        ECOBJECTS_EXPORT static BentleyStatus JsonToECInstance(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, IECClassLocaterR  classLocater);
    };

/*=================================================================================**//**
* JsonEcInstanceWriter - creates Json object from ECInstance
* If writeFormattedQuanties is true then primitive values with koq specification will be save as a JSON object with rawValue, formattedValue, and fusSpec.
* @bsistruct                                                     Bill.Steinbock  02/2016
+===============+===============+===============+===============+===============+======*/
struct JsonEcInstanceWriter final
{
public:
    enum class MemberNameCasing
        {
        KeepOriginal, //!< The JSON member name will be the same as the ECProperty name
        LowerFirstChar //!< The first character of the ECProperty name will be lowercased to create the JSON member name
        };

private:
    static void          AppendAccessString(Utf8String& compoundAccessString, Utf8CP baseAccessString, Utf8StringCR propertyName);
    static StatusInt     WritePropertyValuesOfClassOrStructArrayMember(Json::Value& valueToPopulate, ECN::ECClassCR ecClass, ECN::IECInstanceCR ecInstance, Utf8CP baseAccessString, ECClassLocatorByClassIdCP classLocator, bool writeFormattedQuantities = false, bool serializeNullValues = false, MemberNameCasing casing = MemberNameCasing::KeepOriginal, std::function<bool(Utf8CP)> shouldWriteProperty = nullptr);
    static StatusInt     WritePrimitiveValue(Json::Value& valueToPopulate, Utf8CP propertyName, ECN::ECValueCR ecValue, ECN::PrimitiveType propertyType, KindOfQuantityCP koq = nullptr, MemberNameCasing casing = MemberNameCasing::KeepOriginal);
    static StatusInt     WriteArrayPropertyValue(Json::Value& valueToPopulate, ECN::ArrayECPropertyR arrayProperty, ECN::IECInstanceCR ecInstance, Utf8CP baseAccessString, ECClassLocatorByClassIdCP classLocator, bool writeFormattedQuantities = false, bool serializeNullValues = false, MemberNameCasing casing = MemberNameCasing::KeepOriginal);
    static StatusInt     WriteNavigationPropertyValue(Json::Value& valueToPopulate, ECN::NavigationECPropertyR navigationProperty, ECN::IECInstanceCR ecInstance, Utf8CP baseAccessString, ECClassLocatorByClassIdCP classLocator, bool writeFormattedQuantities = false, bool serializeNullValues = false, MemberNameCasing casing = MemberNameCasing::KeepOriginal);
    static StatusInt     WritePrimitivePropertyValue(Json::Value& valueToPopulate, ECN::PrimitiveECPropertyR primitiveProperty, ECN::IECInstanceCR ecInstance, Utf8CP baseAccessString, bool writeFormattedQuantities = false, bool serializeNullValues = false, MemberNameCasing casing = MemberNameCasing::KeepOriginal);
    static StatusInt     WriteEmbeddedStructPropertyValue(Json::Value& valueToPopulate, ECN::StructECPropertyR structProperty, ECN::IECInstanceCR ecInstance, Utf8CP baseAccessString, bool writeFormattedQuantities = false, bool serializeNullValues = false, MemberNameCasing casing = MemberNameCasing::KeepOriginal);

public:
    //! Convert an ECPropertyName to a JSON member name according to the specified MemberNameCasing option
    ECOBJECTS_EXPORT static Utf8String FormatMemberName(Utf8StringCR propertyName, MemberNameCasing casing);

    //! Write the supplied primitive property value as JSON
    //! @param[out] valueToPopulate the JSON object to populate
    //! @param[in] structProperty the property to write
    //! @param[in] ecInstance the IEInstance containing the structProperty
    //! @param[in] baseAccessString The prefix to use to determine the full access string to the property, typically this would be the containing ECStruct's name.
    //! @return SUCCESS or error status.
    ECOBJECTS_EXPORT static StatusInt WriteEmbeddedStructValue(Json::Value& valueToPopulate, ECN::StructECPropertyR structProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString);

    //! Write the supplied primitive property value as JSON and include presentation data such as KOQ info in the Json
    //! @param[out] valueToPopulate the JSON object to populate
    //! @param[in] structProperty the property to write
    //! @param[in] ecInstance the IEInstance containing the structProperty value
    //! @param[in] baseAccessString The prefix to use to determine the full access string to the property, typically this would be the containing ECStruct's name.
    //! @return SUCCESS or error status.
    ECOBJECTS_EXPORT static StatusInt WriteEmbeddedStructValueForPresentation(Json::Value& valueToPopulate, ECN::StructECPropertyR structProperty, ECN::IECInstanceCR ecInstance, Utf8String* baseAccessString);

    //! Write the supplied primitive property value as JSON
    //! @param[out] valueToPopulate the JSON object to populate
    //! @param[in] primitiveProperty the property to write
    //! @param[in] ecInstance the IEInstance containing the property value
    //! @param[in] baseAccessString The prefix to use to determine the full access string to the property, typically this would be the containing ECStruct name.
    //! @return SUCCESS or error status.
    ECOBJECTS_EXPORT static StatusInt WritePrimitiveValue(Json::Value& valueToPopulate, PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString);

    //! Write the supplied primitive property value as JSON and include presentation data such as KOQ info in the Json
    //! @param[out] valueToPopulate the JSON object to populate
    //! @param[in] primitiveProperty the property to write
    //! @param[in] ecInstance the IEInstance containing the property value
    //! @param[in] baseAccessString The prefix to use to determine the full access string to the property, typically this would be the containing ECStruct name.
    //! @return SUCCESS or error status.
    ECOBJECTS_EXPORT static StatusInt WritePrimitiveValueForPresentation(Json::Value& valueToPopulate, PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString);

    //! Write the supplied instance as JSON
    //! @param[out] valueToPopulate the JSON object to populate
    //! @param[in] ecInstance the IEInstance containing the property values
    //! @param[in] instanceName the name of the JSON object that will contain the IEInstance values. This allows the valueToPopulate to contain multiple instances.
    //! @param[in] writeInstanceId if true the instance Id is saved in the JSON data.
    //! @param[in] serializeNullValues If true all values, even null values, of the IECInstance will be serialized.
    //! @param[in] classLocator to look-up class by ECClassId.
    //! @return SUCCESS or error status.
    ECOBJECTS_EXPORT static StatusInt WriteInstanceToJson(Json::Value& valueToPopulate, ECN::IECInstanceCR ecInstance, Utf8CP instanceName, bool writeInstanceId, bool serializeNullValues = false, ECClassLocatorByClassIdCP classLocator = nullptr);

    //! Write the supplied instance as JSON excluding system properties and optionally filtering by property name
    //! @return SUCCESS or error status.
    ECOBJECTS_EXPORT static StatusInt WritePartialInstanceToJson(Json::Value& valueToPopulate, ECN::IECInstanceCR ecInstance, MemberNameCasing casing, std::function<bool(Utf8CP)> shouldWriteProperty);

    //! Write the supplied instance in the ECSchemaJSON format
    //! @param[out] valueToPopulate the JSON object to populate
    //! @param[in] ecInstance the IEInstance containing the property values
    //! @param[in] classLocator to look-up class by ECClassId.
    ECOBJECTS_EXPORT static StatusInt WriteInstanceToSchemaJson(Json::Value& valueToPopulate, ECN::IECInstanceCR ecInstance, ECClassLocatorByClassIdCP classLocator = nullptr);

    //! Write the supplied instance as JSON and include presentation data such as KOQ info in the Json
    //! @param[out] valueToPopulate the JSON object to populate
    //! @param[in] ecInstance the IEInstance containing the property values
    //! @param[in] instanceName the name of the JSON object that will contain the IEInstance values. This allows the valueToPopulate to contain multiple instances.
    //! @param[in] writeInstanceId if true the instance Id is saved in the JSON data.
    //! @param[in] classLocator to look-up class by ECClassId.
    //! @return SUCCESS or error status.
    ECOBJECTS_EXPORT static StatusInt WriteInstanceToPresentationJson(Json::Value& valueToPopulate, IECInstanceCR ecInstance, Utf8CP instanceName, bool writeInstanceId, ECClassLocatorByClassIdCP classLocator = nullptr);
};

END_BENTLEY_ECOBJECT_NAMESPACE
