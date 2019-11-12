/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECObjects/ECObjectsAPI.h>

struct DynamicSchemaComparer
{
public:
    static bool RequireSchemaUpdate(BENTLEY_NAMESPACE_NAME::ECN::ECSchemaR dynamicSchema, 
                                    BENTLEY_NAMESPACE_NAME::ECN::ECSchemaCR existingSchema,
                                    BENTLEY_NAMESPACE_NAME::ECN::ECClassCR graphicalElement3dClass);
}; // DynamicSchemaComparer