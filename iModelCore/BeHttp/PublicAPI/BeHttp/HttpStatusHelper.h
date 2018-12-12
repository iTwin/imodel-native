/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeHttp/HttpStatusHelper.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/Http.h>
#include <BeHttp/HttpStatus.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

enum class HttpStatusType
    {
    None,
    Informational,
    Success,
    Redirection,
    ClientError,
    ServerError
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Beneditas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpStatusHelper
    {
private:
    HttpStatusHelper ();
    
public:
    BEHTTP_EXPORT static HttpStatusType GetType(HttpStatus status);
    };

END_BENTLEY_HTTP_NAMESPACE