/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECJsonUtility.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECDbTypes.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <rapidjson/BeRapidJson.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      01/2014
+===============+===============+===============+===============+===============+======*/
struct ECJsonCppUtility
    {
private:
    ECJsonCppUtility();
    ~ECJsonCppUtility();

#if !defined (DOCUMENTATION_GENERATOR)
    static StatusInt ECInstanceFromJsonValue(ECN::IECInstanceR instance, const Json::Value& jsonValue, ECN::ECClassCR currentClass, Utf8StringCR currentAccessString);
    static StatusInt ECArrayValueFromJsonValue(ECN::IECInstanceR instance, const Json::Value& jsonValue, ECN::ArrayECPropertyCR arrayProperty, Utf8StringCR currentAccessString);
    static StatusInt ECPrimitiveValueFromJsonValue(ECN::ECValueR ecValue, const Json::Value& jsonValue, ECN::PrimitiveType primitiveType);
#endif

public:
    ECDB_EXPORT static StatusInt ECInstanceFromJsonValue(ECN::IECInstanceR instance, const Json::Value& jsonValue);
    };

/*=================================================================================**//**
* @bsiclass                                     Shaun.Sewall                    01/2014
+===============+===============+===============+===============+===============+======*/
struct ECRapidJsonUtility
    {
private:
    ECRapidJsonUtility();
    ~ECRapidJsonUtility();

#if !defined (DOCUMENTATION_GENERATOR)
    static StatusInt ECInstanceFromJsonValue(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, ECN::ECClassCR currentClass, Utf8StringCR currentAccessString);
    static StatusInt ECArrayValueFromJsonValue(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, ECN::ArrayECPropertyCR arrayProperty, Utf8StringCR currentAccessString);
    static StatusInt ECPrimitiveValueFromJsonValue(ECN::ECValueR ecValue, RapidJsonValueCR jsonValue, ECN::PrimitiveType primitiveType);
#endif

public:
    //! Return an Int64 value from a RapidJsonValueCR that may be a number or a string.
    //! @param[in] value the source RapidJsonValueCR
    //! @param[in] defaultOnError the Int64 value to return if it is not possible to convert the source value to Int64.
    ECDB_EXPORT static int64_t Int64FromValue(RapidJsonValueCR value, int64_t defaultOnError = 0);

    ECDB_EXPORT static StatusInt ECInstanceFromJsonValue(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
