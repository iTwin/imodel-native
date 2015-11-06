/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Util/ExtendedData.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <ECDb/ECDb.h>
#include <DgnClientFx/Utils/Http/Credentials.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExtendedData
    {
    public:
        friend struct ExtendedDataAdapter;

    private:
        ECInstanceKey m_instanceKey;
        ECInstanceKey m_extendedDataKey;
        std::shared_ptr<Json::Value> m_extendedData;

    public:
        WSCACHE_EXPORT ExtendedData();
        WSCACHE_EXPORT ExtendedData
            (
            ECInstanceKey instanceKey,
            ECInstanceKey extendedDataKey,
            std::shared_ptr<Json::Value> extendedData
            );

        //! Get all extended data for reading
        WSCACHE_EXPORT JsonValueCR GetData() const;

        //! Check if value with specific key exists
        WSCACHE_EXPORT bool HasValue(Utf8StringCR key) const;

        //! Get value with specific key
        WSCACHE_EXPORT JsonValueCR GetValue(Utf8StringCR key) const;

        //! Set value with specific key
        WSCACHE_EXPORT void SetValue(Utf8StringCR key, JsonValueCR value);

        //! Remove value with specific key
        WSCACHE_EXPORT void RemoveValue(Utf8StringCR key);
    };

typedef const ExtendedData& ExtendedDataCR;
typedef ExtendedData& ExtendedDataR;

END_BENTLEY_WEBSERVICES_NAMESPACE
