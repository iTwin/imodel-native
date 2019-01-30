/*--------------------------------------------------------------------------------------+
|
|     $Source: Domain/PublicApi/BuildingSpatialMacros.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#define BUILDINGSPATIAL_NAMESPACE_NAME  BuildingSpatial
#define BEGIN_BUILDINGSPATIAL_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace BUILDINGSPATIAL_NAMESPACE_NAME {
#define END_BUILDINGSPATIAL_NAMESPACE } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BUILDINGSPATIAL using namespace BENTLEY_NAMESPACE_NAME::BUILDINGSPATIAL_NAMESPACE_NAME;

#define BUILDINGSPATIAL_SCHEMA_NAME                          "BuildingSpatial"
#define BUILDINGSPATIAL_CLASS_Building                       "Building"
#define BUILDINGSPATIAL_CLASS_Space                          "Space"
#define BUILDINGSPATIAL_CLASS_Story                          "Story"

#define BUILDINGSPATIAL_SCHEMA(className)                    BUILDINGSPATIAL_SCHEMA_NAME "." className

#if defined (__BUILDINGSPATIALELEMENTS_BUILD__)
#define BUILDINGSPATIAL_EXPORT EXPORT_ATTRIBUTE
#else
#define BUILDINGSPATIAL_EXPORT IMPORT_ATTRIBUTE
#endif

#define BUILDINGSPATIAL_CODESPEC_CODE(categoryName)          BUILDINGSPATIAL_SCHEMA_NAME "::" categoryName
