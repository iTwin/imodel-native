/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
