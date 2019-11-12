/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/Response/WSFileResponse.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSFileResponse::WSFileResponse() :
m_filePath(),
WSResponse()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSFileResponse::WSFileResponse(BeFileName filePath, HttpStatus status, Utf8String eTag) :
m_filePath(filePath),
WSResponse(status, eTag)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR WSFileResponse::GetFilePath() const
    {
    return m_filePath;
    }
