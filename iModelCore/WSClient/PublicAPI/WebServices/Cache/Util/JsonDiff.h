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
    private:
        bool m_copyValues;
        bool m_ignoreDeletedProperties;

    private:
        void AddMember(RapidJsonValueR jsonOut, RapidJsonValueCR name, RapidJsonValueCR value, rapidjson::Value::AllocatorType& allocator);

        void CopyValues(RapidJsonValueCR from, RapidJsonValueR to, rapidjson::Value::AllocatorType& allocator);

        bool ValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2, bool deep);

        bool StringValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2);
        bool ArrayValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2);
        bool ObjectValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2, bool deep);

    public:
        WSCACHE_EXPORT JsonDiff(bool copyValues = true, bool ignoreDeletedProperties = true);
        WSCACHE_EXPORT virtual ~JsonDiff();
        
        WSCACHE_EXPORT BentleyStatus GetChanges
            (
            RapidJsonValueCR oldJson,
            RapidJsonValueCR newJson,
            RapidJsonDocumentR jsonOut
            );
        WSCACHE_EXPORT BentleyStatus GetChanges
            (
            RapidJsonValueCR oldJson,
            RapidJsonValueCR newJson,
            RapidJsonValueR jsonOut,
            rapidjson::Value::AllocatorType& allocator
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
