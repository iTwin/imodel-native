/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Compatibility/SchemaChangeWSObjectsResponse.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Client/WSRepositoryClient.h>
#include "SchemaChangeWSObjectsReader.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaChangeWSObjectsResponse : WSObjectsResponse
    {
    SchemaChangeWSObjectsResponse(WSObjectsResponse response, Utf8String schemaName) : WSObjectsResponse(response)
        {
        m_reader = std::make_shared<SchemaChangeWSObjectsReader>(m_reader, schemaName);
        }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE