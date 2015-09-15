/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Util/ECExpressionHelper.h $
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
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECExpressionHelper
    {
    public:
        //! Retrieves class properties used in ECExpression used with "this.". Does not return struct accessors, only base struct property.
        WSCACHE_EXPORT static bset<ECPropertyCP> GetRequiredProperties(Utf8StringCR ecExpression, ECClassCR ecClass);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
