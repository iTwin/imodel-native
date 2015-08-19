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
    //! Convert a Json value to an ECValue
    //! @note Only the following Json "primitive" types are supported: bool, integer, double, string
    DGNPLATFORM_EXPORT static DgnDbStatus ECPrimitiveValueFromJson(ECN::ECValueR ecv, Json::Value const& json);

    //! Convert a primtive ECValue to a Json value
    //! @note Only the following EC primitive types are supported: Boolean, Double, Integer, Long, String, Point2d, Point3d
    DGNPLATFORM_EXPORT static DgnDbStatus ECPrimitiveValueToJson(Json::Value& json, ECN::ECValueCR ecv);

    //! Utility function to convert an ECValue to a JSON value
    //! @note Only the following EC primitive types are supported: Boolean, Double, Integer, Long, String, Point2d, Point3d
    //! @param json     The JSON object to be set
    //! @param ec       an ECValue to convert
    //! @return non-zero if the property could not be converted.
    DGNPLATFORM_EXPORT static BentleyStatus ToJsonFromEC(Json::Value& json, ECN::ECValue const& ec);

    //! Utility function to convert a JSON value to an ECValue
    //! @note Only the following Json "primitive" types are supported: bool, integer, double, string
    //! @param ecvalue  The ECValue to set
    //! @param json     The JSON value
    //! @param typeRequired The ECType required, if known
    //! @return non-zero if the property could not be converted.
    DGNPLATFORM_EXPORT static BentleyStatus ToECFromJson(ECN::ECValue& ec, Json::Value const& json, ECN::PrimitiveType typeRequired);

    //! Utility function to convert ECProperty values to JSON properties. 
    //! @note Only the following EC primitive types are supported: Boolean, Double, Integer, Long, String, Point2d, Point3d
    //! @param json     The JSON object to be populated
    //! @param ec       an ECObject that contains values
    //! @param props    comma-separated list of property names
    //! @return non-zero if even one ECProperty value could not be converted.
    DGNPLATFORM_EXPORT static BentleyStatus ToJsonPropertiesFromECProperties(Json::Value& json, ECN::IECInstanceCR ec, Utf8StringCR props);

    //! @private
    static Utf8CP ECPrimtiveTypeToString(ECN::PrimitiveType pt);
    //! @private
    static ECN::PrimitiveType ECPrimtiveTypeFromString(Utf8CP);

};



END_BENTLEY_DGNPLATFORM_NAMESPACE
