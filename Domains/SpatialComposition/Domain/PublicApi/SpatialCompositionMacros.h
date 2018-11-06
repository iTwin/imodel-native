/*--------------------------------------------------------------------------------------+
|
|     $Source: Domain/PublicApi/SpatialCompositionMacros.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#define SPATIALCOMPOSITION_NAMESPACE_NAME  SpatialComposition
#define BEGIN_SPATIALCOMPOSITION_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace SPATIALCOMPOSITION_NAMESPACE_NAME {
#define END_SPATIALCOMPOSITION_NAMESPACE } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_SPATIALCOMPOSITION using namespace BENTLEY_NAMESPACE_NAME::SPATIALCOMPOSITION_NAMESPACE_NAME;

#define SPATIALCOMPOSITION_SCHEMA_NAME                          "SpatialComposition"
#define SPATIALCOMPOSITION_CLASS_CompositeBoundary              "CompositeBoundary"
#define SPATIALCOMPOSITION_CLASS_CompositeVolume                "CompositeVolume"
#define SPATIALCOMPOSITION_CLASS_CompositeElement               "CompositeElement"

#define SPATIALCOMPOSITION_REL_CompositeComposesSubComposites   "CompositeComposesSubComposites"
#define SPATIALCOMPOSITION_REL_CompositeOverlapsSpatialElements "CompositeOverlapsSpatialElements"

#define SPATIALCOMPOSITION_SCHEMA(className)                    SPATIALCOMPOSITION_SCHEMA_NAME "." className

#if defined (__SPATIALCOMPOSITIONELEMENTS_BUILD__)
#define SPATIALCOMPOSITION_EXPORT EXPORT_ATTRIBUTE
#else
#define SPATIALCOMPOSITION_EXPORT IMPORT_ATTRIBUTE
#endif
