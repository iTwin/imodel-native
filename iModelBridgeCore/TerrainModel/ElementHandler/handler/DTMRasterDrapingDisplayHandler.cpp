/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMRasterDrapingDisplayHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 09/11
//=======================================================================================
void DTMElementRasterDrapingDisplayHandler::_EditProperties (EditElementHandleR element, ElementHandle::XAttributeIter xAttr, DTMSubElementId const &sid, PropertyContextR context)
    {
    T_Super::_EditProperties (element, xAttr, sid, context);

    DTMElementRasterDrapingHandler::DisplayParams displayParams (element, sid);
    PropsCallbackFlags propsFlag = displayParams.GetVisible() ?  PROPSCALLBACK_FLAGS_NoFlagsSet : PROPSCALLBACK_FLAGS_UndisplayedID;
    bool changed = false;

    if (0 != (ELEMENT_PROPERTY_Material & context.GetElementPropertiesMask ()))
        {
        MaterialId materialId (displayParams.GetMaterialElementID());
        EachMaterialArg arg (materialId, propsFlag, context);
        if (context.DoMaterialCallback (&materialId, arg))
            {
            changed = true;
            displayParams.SetMaterialElementID (materialId.GetElementId ());
            }
        }

    if (changed)
        displayParams.SetElement (element, sid);
    }


SUBDISPLAYHANDLER_DEFINE_MEMBERS (DTMElementRasterDrapingDisplayHandler);

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
