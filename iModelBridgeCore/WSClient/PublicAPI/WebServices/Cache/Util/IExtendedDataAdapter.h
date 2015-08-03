/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Util/IExtendedDataAdapter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Util/ExtendedData.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE IExtendedDataAdapter
    {
    public:
        virtual ~IExtendedDataAdapter ()
            {
            };
        virtual ExtendedData GetData (ECInstanceKeyCR instanceKey) = 0;
        virtual BentleyStatus UpdateData (ExtendedData& data) = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
