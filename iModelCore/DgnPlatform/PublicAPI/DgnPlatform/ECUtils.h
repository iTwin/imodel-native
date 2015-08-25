/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ECUtils.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECObjects/ECValue.h>
#include <json/json.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Mostly EC <-> Json conversion utilities
//! *** NEEDS WORK: Can we leverage what we already have in the EC   Json adapter?
//=======================================================================================
struct ECUtils
{
    //! De-serialize an ECValue from JSON
    //! @param[out] ecvalue the resulting ECValue
    //! @param json     a JSON object to convert
    //! @note Only the following Json "primitive" types are supported: bool, integer, double, string
    DGNPLATFORM_EXPORT static DgnDbStatus LoadECValueFromJson(ECN::ECValueR ecvalue, Json::Value const& json);

    //! Serialize a primtive ECValue to JSON
    //! @note Only the following EC primitive types are supported: Boolean, Double, Integer, Long, String, Point2d, Point3d, DateTime, IGeometry, and Binary
    //! @param[out] json The resulting JSON object 
    //! @param ecvalue  an ECValue to convert
    DGNPLATFORM_EXPORT static DgnDbStatus StoreECValueAsJson(Json::Value& json, ECN::ECValueCR ecvalue);

    //! Utility function to convert an ECValue to a JSON value
    //! @note Only the following EC primitive types are supported: Boolean, Double, Integer, Long, String, Point2d, Point3d
    //! @note DateTime is converted to a string. IGeometry and Binary are not supported at all.
    //! @param[out] json     The JSON object to be set
    //! @param ecvalue  an ECValue to convert
    //! @return non-zero if the property could not be converted.
    DGNPLATFORM_EXPORT static BentleyStatus ConvertECValueToJson(Json::Value& json, ECN::ECValue const& ecvalue);

    //! Utility function to convert a JSON value to an ECValue.
    //! @note Only the following Json "primitive" types are supported: bool, integer, double, string
    //! @param[out] ecvalue  The ECValue to set
    //! @param json     The JSON value to convert
    //! @param typeRequired The ECType required, if known
    //! @return non-zero if the property could not be converted.
    DGNPLATFORM_EXPORT static BentleyStatus ConvertJsonToECValue(ECN::ECValue& ecvalue, Json::Value const& json, ECN::PrimitiveType typeRequired);

    //! Utility function to convert ECProperty values to JSON properties. 
    //! @note Only the following EC primitive types are supported: Boolean, Double, Integer, Long, String, Point2d, Point3d
    //! @param[out] json     The JSON object to be populated
    //! @param ecInstance an EC instance that contains values
    //! @param props    comma-separated list of property names
    //! @return non-zero if even one ECProperty value could not be converted.
    DGNPLATFORM_EXPORT static BentleyStatus ToJsonPropertiesFromECProperties(Json::Value& json, ECN::IECInstanceCR ecInstance, Utf8StringCR props);

    //! @private
    static Utf8CP ECPrimtiveTypeToString(ECN::PrimitiveType pt);
    //! @private
    static ECN::PrimitiveType ECPrimtiveTypeFromString(Utf8CP);

};



END_BENTLEY_DGNPLATFORM_NAMESPACE
