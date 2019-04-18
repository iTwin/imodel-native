/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>

#include <ScalableMeshSchema/ScalableMeshSchemaCommon.h>

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_SCALABLEMESH_SCHEMA_NAME                      "ScalableMesh"
#define BENTLEY_SCALABLEMESH_SCHEMA_PATH                      L"ECSchemas/Domain/ScalableMesh.ecschema.xml"
#define SCALABLEMESH_SCHEMA(className)                        BENTLEY_SCALABLEMESH_SCHEMA_NAME "." className

//-----------------------------------------------------------------------------------------
// Include from PublicApi
//-----------------------------------------------------------------------------------------
#include <ScalableMeshSchema/ScalableMeshDomain.h>

//__PUBLISH_SECTION_END__
#include <ScalableMeshSchema/ScalableMeshHandler.h>
