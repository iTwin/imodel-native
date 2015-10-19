/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Client/MockWSSchemaProvider.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Client/WSRepositoryClient.h>
#include "MockWSClient.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockWSSchemaProvider : public IWSSchemaProvider
    {
    MOCK_METHOD1 (GetSchema, BeFileName (WSInfoCR info));
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
