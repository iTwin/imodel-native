/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Util/JsonDiff.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <WebServices/Cache/WebServicesCache.h>
#include <MobileDgn/Utils/Utils.h>
#include <rapidjson/BeRapidJson.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                          
+---------------+---------------+---------------+---------------+---------------+------*/
struct JsonDiff
    {
    public:
        WSCACHE_EXPORT JsonDiff();
        WSCACHE_EXPORT virtual ~JsonDiff();

        WSCACHE_EXPORT BentleyStatus GetChanges(RapidJsonValueCR oldJson, RapidJsonValueCR newJson, RapidJsonValueR changesJsonOut);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
