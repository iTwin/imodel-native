/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxSchemaInternal.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/DgnDomain.h>

#include <ThreeMxSchema/ThreeMxSchemaCommon.h>
#include <ThreeMxSchema/ThreeMxSchemaAPI.h>
#include <ThreeMxSchema/ThreeMxHandler.h>

#include "Reader/ThreeMxReader.h"
#include "MrMesh/MRMesh.h"

#if defined (__THREEMXSCHEMA_BUILD__)
#   define THREEMXSCHEMA_EXPORT      EXPORT_ATTRIBUTE
#else
#   define THREEMXSCHEMA_EXPORT      IMPORT_ATTRIBUTE
#endif


USING_NAMESPACE_BENTLEY_THREEMX_SCHEMA

