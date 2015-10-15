/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMElementSubDisplayHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h" 

#include "MrDTMDataRef.h"
#include <DgnPlatform\TerrainModel\TMElementHandler.h>
#include <DgnPlatform\TerrainModel\TMElementSubHandler.h>


BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

bmap<UInt16, DTMElementSubDisplayHandler*> DTMElementSubDisplayHandler::s_subHandlers;

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 01/11
//=======================================================================================
bool DTMElementSubDisplayHandler::CanDoPickFlash(RefCountedPtr<DTMDataRef>& dtmDataRef, 
                                          DrawPurpose                drawPurpose)
    {
    bool canDoPickFlash = true;
    if ((dtmDataRef != 0) && (dtmDataRef->IsMrDTM() == true))
        {
        if (DrawPurpose::Pick == drawPurpose)
            {                
            canDoPickFlash = ((MrDTMDataRef*)dtmDataRef.get())->CanLocate() || !Bentley::DgnPlatform::AccuSnap::GetInstance ().IsLocateEnabled();
            }
        else
            {
            canDoPickFlash = ((MrDTMDataRef*)dtmDataRef.get())->CanLocate();
            }       
        }           
    return canDoPickFlash;
    }

void DTMElementSubDisplayHandler::Register()
    {
    s_subHandlers[GetSubHandlerId()] = this;
    }

void DTMElementSubDisplayHandler::DrawSubElement (ElementHandleCR el, const ElementHandle::XAttributeIter& xAttr, ViewContextR context, const DTMFenceParams& fence)
    {
    DTMElementDisplayHandler::DrawSubElement (el, xAttr, context, fence);
    }

bool DTMElementSubDisplayHandler::_CanDraw(DTMDataRef* dtmDataRef, ViewContextCR context) const
    {
    return true;
    }

SnapStatus DTMElementSubDisplayHandler::_OnSnap (ElementHandleCR elem, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& drawingInfoProvider, SnapContextP context, int snapPathIndex)
    {
    if (!context->IsSnappableElement (snapPathIndex))
        return SnapStatus::NotSnappable;

    return context->DoDefaultDisplayableSnap (snapPathIndex);
    }

void DTMElementSubDisplayHandler::_GetDescription (ElementHandleCR elem, const ElementHandle::XAttributeIter& xAttr, WString& string, uint32_t desiredLength)
    {
    DTMElementSubHandler* subHandler = DTMElementSubHandler::FindHandler (xAttr);
    if (subHandler)
        subHandler->_GetDescription (elem, xAttr, string, desiredLength);  // start with element's description
    }

void DTMElementSubDisplayHandler::_GetPathDescription (ElementHandleCR elem, const ElementHandle::XAttributeIter& xAttr, LazyDTMDrawingInfoProvider& ldip, WStringR string, HitPathCR path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr)
    {
    _GetDescription (elem, xAttr, string, 100);
    }

bool DTMElementSubDisplayHandler::CanDraw(DTMDataRef* dtmDataRef, ViewContextCR context)
    {
    return _CanDraw (dtmDataRef, context);
    }

DTMElementSubDisplayHandler* DTMElementSubDisplayHandler::FindHandler (const DTMSubElementId& id)
    {
    bmap<UInt16, DTMElementSubDisplayHandler*>::const_iterator it = s_subHandlers.find(id.GetHandlerId());

    if (it != s_subHandlers.end())
        {
        return it->second;
        }
    return nullptr;
    }

DTMElementSubDisplayHandler* DTMElementSubDisplayHandler::FindHandler (const ElementHandle::XAttributeIter& xAttr)
    {
    UInt16 handlerId = *(UInt16*)(xAttr.PeekData());
    bmap<UInt16, DTMElementSubDisplayHandler*>::const_iterator it = s_subHandlers.find(handlerId);

    if (it != s_subHandlers.end())
        {
        return it->second;
        }
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 09/11
//=======================================================================================
void DTMElementSubDisplayHandler::_EditProperties (EditElementHandleR element, ElementHandle::XAttributeIter iter, DTMSubElementId const &sid, PropertyContextR context)
    {
    bool changed = false;
    DTMElementSubHandler::SymbologyParams params(iter);
    PropsCallbackFlags propsFlag = params.GetVisible() ?  PROPSCALLBACK_FLAGS_NoFlagsSet : PROPSCALLBACK_FLAGS_UndisplayedID;

    if (0 != (ELEMENT_PROPERTY_Level & context.GetElementPropertiesMask ()))
        {
        LevelId levelId = params.GetLevelId();
        EachLevelArg arg (levelId, propsFlag, context);
        changed |= context.DoLevelCallback (&levelId, arg);
        params.SetLevelId (levelId);
        }

    if (0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
        {
        EachColorArg arg (params.GetSymbology().color, propsFlag, context);
        changed |= context.DoColorCallback (&params.GetSymbology().color, arg);
        }

    if (0 != (ELEMENT_PROPERTY_Weight & context.GetElementPropertiesMask ()))
        {
        EachWeightArg arg (params.GetSymbology().weight, propsFlag, context);
        changed |= context.DoWeightCallback (&params.GetSymbology().weight, arg);
        }

    if (0 != (ELEMENT_PROPERTY_Linestyle & context.GetElementPropertiesMask ()))
        {
        EachLineStyleArg arg (params.GetSymbology().style, NULL, PROPSCALLBACK_FLAGS_NoFlagsSet, context);
        changed |= context.DoLineStyleCallback (&params.GetSymbology().style, arg);
        }

    if (0 != (ELEMENT_PROPERTY_Transparency & context.GetElementPropertiesMask ()))
        {
        double transparency = params.GetTransparency();
        EachTransparencyArg arg (transparency, propsFlag, context);
        changed |= context.DoTransparencyCallback (&transparency, arg);
        params.SetTransparency (transparency);
        }


    if (changed)
        params.UpdateElementSymbologyParams (element, sid);
    }


END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
