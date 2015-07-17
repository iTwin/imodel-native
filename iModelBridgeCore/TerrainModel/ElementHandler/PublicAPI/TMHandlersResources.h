/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/PublicAPI/TMHandlersResources.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once
#include <TerrainModel/ElementHandler/TerrainModelElementHandler.h>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
struct TerrainModelElementResources
    {
    DTMELEMENT_EXPORT static WString          GetString (UInt stringId);
    };
END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
/*__PUBLISH_SECTION_END__*/
