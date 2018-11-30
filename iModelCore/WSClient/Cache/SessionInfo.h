/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/SessionInfo.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Persistence/IDataSourceCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct SessionInfo
    {
    std::unique_ptr<bool> repositorySupportsFileAccessUrl;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
