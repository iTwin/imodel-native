/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/Response/WSCreateObjectResponse.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/Response/WSCreateObjectResponse.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSCreateObjectResponse::WSCreateObjectResponse() :
m_createdObject()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jahan.Zeb    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSCreateObjectResponse::WSCreateObjectResponse(JsonValueCR createdObject) :
m_createdObject(createdObject)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jahan.Zeb    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueCR WSCreateObjectResponse::GetObject() const
    {
    return m_createdObject;
    }
