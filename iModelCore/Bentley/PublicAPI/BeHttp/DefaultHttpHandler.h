/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeHttp/DefaultHttpHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/IHttpHandler.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct DefaultHttpHandler
    {
    public:
        BEHTTP_EXPORT static IHttpHandlerPtr GetInstance ();
    };

END_BENTLEY_HTTP_NAMESPACE
