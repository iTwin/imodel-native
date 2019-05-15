/*--------------------------------------------------------------------------------------+
|
|     $Source: Domain/PublicApi/SpatialCompositionMacros.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>

#define SPATIALCOMPOSITION_NAMESPACE_NAME  SpatialComposition
#define BEGIN_SPATIALCOMPOSITION_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace SPATIALCOMPOSITION_NAMESPACE_NAME {
#define END_SPATIALCOMPOSITION_NAMESPACE } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_SPATIALCOMPOSITION using namespace BENTLEY_NAMESPACE_NAME::SPATIALCOMPOSITION_NAMESPACE_NAME;

#define SPATIALCOMPOSITION_SCHEMA_NAME                          "SpatialComposition"
#define SPATIALCOMPOSITION_CLASS_CompositeElement               "CompositeElement"

#define SPATIALCOMPOSITION_CATEGORY_CODE_CompositeElement        "CompositeElement"

#define SPATIALCOMPOSITION_REL_CompositeComposesSubComposites   "CompositeComposesSubComposites"
#define SPATIALCOMPOSITION_REL_CompositeOverlapsSpatialElements "CompositeOverlapsSpatialElements"

#define SPATIALCOMPOSITION_SCHEMA(className)                    SPATIALCOMPOSITION_SCHEMA_NAME "." className

#if defined (__SPATIALCOMPOSITIONELEMENTS_BUILD__)
#define SPATIALCOMPOSITION_EXPORT EXPORT_ATTRIBUTE
#else
#define SPATIALCOMPOSITION_EXPORT IMPORT_ATTRIBUTE
#endif

#define SPATIALCOMPOSITION_CODESPEC_CODE(categoryName)          SPATIALCOMPOSITION_SCHEMA_NAME "::" categoryName
#define SPATIALCOMPOSITION_AUTHORITY_CompositeElement       SPATIALCOMPOSITION_CODESPEC_CODE(SPATIALCOMPOSITION_CLASS_CompositeElement)

//-----------------------------------------------------------------------------------------
// Define both RefCounterPtr/CPtr and (P, CP, R, CR) types
//-----------------------------------------------------------------------------------------
#define SPATIALCOMPOSITION_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
    BEGIN_SPATIALCOMPOSITION_NAMESPACE \
        DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
        DEFINE_REF_COUNTED_PTR(_name_) \
    END_SPATIALCOMPOSITION_NAMESPACE
