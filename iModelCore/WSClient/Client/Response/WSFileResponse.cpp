/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/Response/WSFileResponse.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/Response/WSFileResponse.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSFileResponse::WSFileResponse() :
m_filePath(),
m_isModified(false),
m_eTag()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSFileResponse::WSFileResponse(BeFileName filePath, HttpStatus status, Utf8String eTag) :
m_filePath(filePath),
m_isModified(HttpStatus::OK == status),
m_eTag(eTag)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSFileResponse::IsModified() const
    {
    return m_isModified;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR WSFileResponse::GetFilePath() const
    {
    return m_filePath;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSFileResponse::GetETag() const
    {
    return m_eTag;
    }
