/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewController::_VisitPath (DisplayPathCP path, void* arg, ViewContextR context) const
    {
    // NEEDSWORK: _VisitPath with a DisplayPathCP is now un-necessary as you can just use VisitElement.
    //            We do still need to be able to visit a "HitPath" for segment/sub-selection flash/hilite...
    GeometricElementCP geomElement = (nullptr != path ? path->GetHeadElem ()->_ToGeometricElement() : nullptr);

    if (nullptr == geomElement)
        return ERROR;

    ViewContext::ContextMark mark(&context);

#if defined (V10_WIP_ELEMENTHANDLER)
    // NEEDSWORK: Still want handler involvement for flashing single segments of linestrings, etc. for a "HitPath".
    if (SUCCESS == ElementHandler::_DrawPath (path, context))
        return SUCCESS;
#endif

    return context.VisitElement(*geomElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_DrawView (ViewContextR context) 
    {
    for (auto modelId : m_viewedModels)
        context.VisitDgnModel (m_dgndb.Models().GetModelById(modelId));
    }
