/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityDataService.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <RealityPlatform/RealityDataService.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

bool RealityDataByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGUrl::_PrepareHttpRequestStringAndPayload();
    }

RealityDataProjectRelationshipByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGUrl::_PrepareHttpRequestStringAndPayload();
    }

RealityDataFolderByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGUrl::_PrepareHttpRequestStringAndPayload();
    }

RealityDataDocumentByIdRequest::_PrepareHttpRequestStringAndPayload() const
    {
    WSGUrl::_PrepareHttpRequestStringAndPayload();
    }
