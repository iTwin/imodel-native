/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Util/ECCustomAttributeHelper.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <ECObjects/ECObjectsAPI.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECCustomAttributeHelper
    {
    private:
        static void ValidatePropertyName(ECClassCP ecClass, Utf8StringR propertyName);

    public:
        //! Retrieves value from custom attribute property
        //! @returns valid value or null value if does not exist
        WSCACHE_EXPORT static ECValue GetValue(IECCustomAttributeContainerCP caContainer, Utf8CP caName, Utf8CP caPropertyName);

        //! Retrieves integer value from custom attribute property
        //! @returns value or defaultValue if does not exist
        WSCACHE_EXPORT static int32_t GetInteger(IECCustomAttributeContainerCP caContainer, Utf8CP caName, Utf8CP caPropertyName, int32_t defaultValue);

        //! Retrieves bool value from custom attribute property
        //! @returns value or defaultValue if does not exist
        WSCACHE_EXPORT static bool GetBool(IECCustomAttributeContainerCP caContainer, Utf8CP caName, Utf8CP caPropertyName, bool defaultValue);

        //! Retrieves string value from custom attribute property
        //! @returns value or empty string if does not exist
        WSCACHE_EXPORT static Utf8String GetString(IECCustomAttributeContainerCP caContainer, Utf8CP caName, Utf8CP caPropertyName);

        //! Retrieves string value from custom attribute property
        //! @returns values or empty vector if does not exist
        WSCACHE_EXPORT static bvector<Utf8String> GetStringArray(IECCustomAttributeContainerCP caContainer, Utf8CP caName, Utf8CP caPropertyName);

        //! Retrieves property name from custom attribute property
        //! @returns property name or empty string if such property not found
        WSCACHE_EXPORT static Utf8String GetPropertyName(ECClassCP ecClass, Utf8CP caName, Utf8CP caPropertyName);

        //! Retrieves property names from custom attribute array property as vector
        //! @returns property names that exist in ecClass
        WSCACHE_EXPORT static bvector<Utf8String> GetPropertyNameArray(ECClassCP ecClass, Utf8CP caName, Utf8CP caPropertyName);

        //! Retrieves property names from custom attribute array property as set
        //! @returns property names that exist in ecClass
        WSCACHE_EXPORT static bset<Utf8String> GetPropertyNameSet(ECClassCP ecClass, Utf8CP caName, Utf8CP caPropertyName);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
