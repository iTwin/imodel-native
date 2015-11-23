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
        void AddMember(RapidJsonValueR parent, RapidJsonValueR name, RapidJsonValueR value, rapidjson::Value::AllocatorType& allocator);
        void AddMember(RapidJsonValueR parent, RapidJsonValueCR name, RapidJsonValueCR value, rapidjson::Value::AllocatorType& allocator);

        void CopyValues(RapidJsonValueCR source, RapidJsonValueR target, rapidjson::Value::AllocatorType& allocator);

        static bool StringValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2);
        static bool ArrayValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2);
        static bool ObjectValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2);

        static RapidJsonValueCR ValidateObject(RapidJsonValueCR value);

    public:
        WSCACHE_EXPORT JsonDiff(bool copyValues = true, bool ignoreDeletedProperties = true);
        WSCACHE_EXPORT virtual ~JsonDiff();
        
        //! Get changes between JSON objects
        //! @param oldJson 
        //! @param newJson
        //! @param jsonOut output for changes
        //! @return true if any changes found
        WSCACHE_EXPORT bool GetChanges
            (
            RapidJsonValueCR oldJson,
            RapidJsonValueCR newJson,
            RapidJsonDocumentR jsonOut
            );

        //! Get changes between JSON objects
        //! @param oldJson
        //! @param newJson
        //! @param jsonOut output for changes
        //! @param allocator allocator for jsonOut values
        //! @return true if any changes found
        WSCACHE_EXPORT bool GetChanges
            (
            RapidJsonValueCR oldJson,
            RapidJsonValueCR newJson,
            RapidJsonValueR jsonOut,
            rapidjson::Value::AllocatorType& allocator
            );

        //! Check if values are equal
        WSCACHE_EXPORT static bool ValuesEqual(RapidJsonValueCR value1, RapidJsonValueCR value2);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
