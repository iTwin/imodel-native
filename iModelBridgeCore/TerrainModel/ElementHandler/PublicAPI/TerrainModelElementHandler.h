/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/PublicAPI/TerrainModelElementHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#if !defined (BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE)
#define BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE namespace Bentley { namespace TerrainModel { namespace Element {
#define END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE }}}
#define USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT using namespace Bentley::TerrainModel::Element;
#endif

#if defined (CREATE_STATIC_LIBRARIES) || defined (TERRAINMODEL_STATICLIB)
  #define DTMDGNPLATFORM_EXPORT 
#elif defined (__BENTLEY_DTM_DGNPLATFORM_BUILD__)
  #define DTMDGNPLATFORM_EXPORT EXPORT_ATTRIBUTE
#else
  #define DTMDGNPLATFORM_EXPORT IMPORT_ATTRIBUTE
#endif

#if defined (CREATE_STATIC_LIBRARIES) || defined (TERRAINMODEL_STATICLIB)
  #define DTMELEMENT_EXPORT 
#elif defined (__BENTLEY_DTM_ELEMENT_BUILD__)
  #define DTMELEMENT_EXPORT EXPORT_ATTRIBUTE
#else
  #define DTMELEMENT_EXPORT IMPORT_ATTRIBUTE
#endif

#include <TerrainModel\TerrainModel.h>

//__PUBLISH_SECTION_END__
#define REFERENCE_FILE_CLASS
#include <DgnPlatform\DgnPlatformAPI.h>
#include <DgnPlatform\TerrainModel\TMPersistentAppIDs.h>
