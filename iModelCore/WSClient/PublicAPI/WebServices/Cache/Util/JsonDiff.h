/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Util/JsonDiff.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <WebServices/Cache/WebServicesCache.h>
#include <rapidjson/BeRapidJson.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                          
+---------------+---------------+---------------+---------------+---------------+------*/
struct JsonDiff
    {
    public:
        enum Flags
            {
            Default         = 0,        // No flags
            DoNotCopyValues = 1 << 0,   // Dont copying values and just reference original JSON values
            FindDeletions   = 1 << 1    // Find deletions in change and present them as nulls
            };

    private:
        bool m_avoidCopies;
        bool m_findDeletions;

    private:
        void AddMember(RapidJsonValueR parent, RapidJsonValueR name, RapidJsonValueR value, rapidjson::Value::AllocatorType& allocator);
        void AddMember(RapidJsonValueR parent, RapidJsonValueCR name, RapidJsonValueCR value, rapidjson::Value::AllocatorType& allocator);

        void CopyValues(RapidJsonValueCR source, RapidJsonValueR target, rapidjson::Value::AllocatorType& allocator);

        static RapidJsonValueCR ValidateObject(RapidJsonValueCR value);

    public:
        WSCACHE_EXPORT JsonDiff(int flags = Flags::Default);
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
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
