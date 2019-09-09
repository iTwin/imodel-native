/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/DynamicSchemaComparer.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ECObjects/ECObjectsAPI.h>

struct DynamicSchemaComparer
{
public:
    static bool RequireSchemaUpdate(BENTLEY_NAMESPACE_NAME::ECN::ECSchemaR dynamicSchema, 
                                    BENTLEY_NAMESPACE_NAME::ECN::ECSchemaCR existingSchema,
                                    BENTLEY_NAMESPACE_NAME::ECN::ECClassCR graphicalElement3dClass);
}; // DynamicSchemaComparer