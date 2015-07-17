/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMRasterDrapingDisplayHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <stdafx.h>

#include "MrDTMDataRef.h"  
   
USING_NAMESPACE_RASTER
    
BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

//Testing new mesh region loading mechanism
#ifdef DEBUG
    static bool s_dumpMeshLoadingInfo = false;
#endif DEBUG

/*=================================================================================**//**
* @bsiclass                                                  Mathieu.St-Pierre   08/2011
+===============+===============+===============+===============+===============+======*/
bool DTMElementRasterDrapingDisplayHandler::_CanDraw(DTMDataRef* dtmDataRef, ViewContextCR context) const 
    {
    bool canDraw = false;

    if ((dtmDataRef->IsMrDTM() == true) && 
        (false == IsWireframeRendering(context)) && 
        (DTMElementHandlerManager::GetMrDTMActivationRefCount() > 0) &&
        (static_cast<MrDTMDataRef*>(dtmDataRef)->GetRasterTextureSource() != 0))
        {                    
        canDraw = true;
        }        

    return canDraw;                       
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre
//=======================================================================================
bool DTMElementRasterDrapingDisplayHandler::_Draw (ElementHandleCR el, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context)
    {             
    if ((IsWireframeRendering(context) == false) && (drawingInfo.GetDTMDataRef()->CanDrapeRasterTexture() == true))
        {
        return __super::_Draw(el, xAttr, drawingInfo, context);
        }
    return false;
    }


SUBDISPLAYHANDLER_DEFINE_MEMBERS (DTMElementRasterDrapingDisplayHandler);

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
