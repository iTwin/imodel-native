/*--------------------------------------------------------------------------------------+
|
|     $Source: Domain/PublicApi/BuildingSpatialMacros.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>

#define BUILDINGSPATIAL_NAMESPACE_NAME  BuildingSpatial
#define BEGIN_BUILDINGSPATIAL_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace BUILDINGSPATIAL_NAMESPACE_NAME {
#define END_BUILDINGSPATIAL_NAMESPACE } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BUILDINGSPATIAL using namespace BENTLEY_NAMESPACE_NAME::BUILDINGSPATIAL_NAMESPACE_NAME;

#define BUILDINGSPATIAL_SCHEMA_NAME                          "BuildingSpatial"
#define BUILDINGSPATIAL_SCHEMA_PATH                          L"ECSchemas/Domain/BuildingSpatial.ecschema.xml"

#define BUILDINGSPATIAL_CLASS_Building                       "Building"
#define BUILDINGSPATIAL_CLASS_ElevationStory                 "ElevationStory"
#define BUILDINGSPATIAL_CLASS_Space                          "Space"
#define BUILDINGSPATIAL_CLASS_Story                          "Story"

#define BUILDINGSPATIAL_SCHEMA(className)                    BUILDINGSPATIAL_SCHEMA_NAME "." className
#define BUILDINGSPATIAL_CODESPEC_CODE(categoryName)          BUILDINGSPATIAL_SCHEMA_NAME "::" categoryName

#define BUILDINGSPATIAL_AUTHORITY_Building                   BUILDINGSPATIAL_CODESPEC_CODE(BUILDINGSPATIAL_CLASS_Building)
#define BUILDINGSPATIAL_AUTHORITY_Space                      BUILDINGSPATIAL_CODESPEC_CODE(BUILDINGSPATIAL_CLASS_Space)
#define BUILDINGSPATIAL_AUTHORITY_ElevationStory             BUILDINGSPATIAL_CODESPEC_CODE(BUILDINGSPATIAL_CLASS_ElevationStory)

#if defined (__BUILDINGSPATIALELEMENTS_BUILD__)
#define BUILDINGSPATIAL_EXPORT EXPORT_ATTRIBUTE
#else
#define BUILDINGSPATIAL_EXPORT IMPORT_ATTRIBUTE
#endif

#if defined (__BUILDINGSPATIALHANDLERS_BUILD__)
#define BUILDINGSPATIALHANDLERS_EXPORT EXPORT_ATTRIBUTE
#else
#define BUILDINGSPATIALHANDLERS_EXPORT IMPORT_ATTRIBUTE
#endif

#define BUILDINGSPATIAL_CODESPEC_CODE(categoryName)          BUILDINGSPATIAL_SCHEMA_NAME "::" categoryName

#define BUILDINGSPATIAL_CATEGORY_CODE_Building                  "Building"
#define BUILDINGSPATIAL_CATEGORY_CODE_Space                     "Space"
#define BUILDINGSPATIAL_CATEGORY_CODE_ElevationStory            "ElevationStory"
#define BUILDINGSPATIAL_SUBCATEGORY_CODE_SpatialElementLabels   "Labels"

//-----------------------------------------------------------------------------------------
// Define both RefCounterPtr/CPtr and (P, CP, R, CR) types
//-----------------------------------------------------------------------------------------
#define BUILDINGSPATIAL_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
    BEGIN_BUILDINGSPATIAL_NAMESPACE \
        DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
        DEFINE_REF_COUNTED_PTR(_name_) \
    END_BUILDINGSPATIAL_NAMESPACE
