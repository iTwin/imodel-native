/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Transactions/ITransactionHandler.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ITransactionHandler
    {
    public:
        virtual ~ITransactionHandler()
            {};

        virtual BentleyStatus CommitTransaction() = 0;
        virtual BentleyStatus RollbackTransaction() = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
