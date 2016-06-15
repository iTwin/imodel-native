/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/HttpStatusHelper.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <BeHttp/HttpStatusHelper.h>

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpStatusType HttpStatusHelper::GetType (HttpStatus status)
    {
    int httpStatusCode = static_cast<int>(status);
    
    if (status == HttpStatus::None)
        return HttpStatusType::None;
        
    if (httpStatusCode < 200)
        return HttpStatusType::Informational;
        
    if (httpStatusCode < 300)
        return HttpStatusType::Success;
        
    if (httpStatusCode < 400)
        return HttpStatusType::Redirection;
    
    if (httpStatusCode < 500)
        return HttpStatusType::ClientError;
        
    if (httpStatusCode < 600)
        return HttpStatusType::ServerError;
        
    return HttpStatusType::None;
    }
