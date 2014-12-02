/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt visitCursorElement (DisplayPathCR path, void* arg, ViewContextR context)
    {
    // NOTE: To "properly" support DrawPath for legacy complex w/o public children we'd have to start from
    //       the path head and loop to cursor element stopping when we encounter an element
    //       that doesn't have public children. Not going to worry about this until someone needs it...
    ElementRefP elemRef = path.GetCursorElem ();

    if (NULL == elemRef)
        {
        BeAssert (false);
        return ERROR;
        }

    ElementHandle   eh (elemRef);
    DisplayHandlerP dHandler = eh.GetDisplayHandler ();

    if (NULL != dHandler && SUCCESS == dHandler->DrawPath (path, context))
        return SUCCESS;

    return context.VisitElemRef (elemRef, arg, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt visitPath (DisplayPathCR path, void* arg, ViewContextR context)
    {
#if defined (NEEDS_WORK_DGNITEM)
    for (int index = 0; index < path.GetCursorIndex (); index++)
        {
        ElementRefP pathElm = path.GetPathElem (index);

        if (NULL == pathElm || pathElm->IsUndisplayed())
            return ViewContext::VISIT_PATH_Filtered;

        ElementHandle pathEh (pathElm);
        
        IDisplayHandlerPathEntryExtension* extension = IDisplayHandlerPathEntryExtension::Cast (pathEh.GetHandler ());

        if (NULL != extension)
            extension->_PushDisplayEffects (pathEh, context);
        }
#endif

    return visitCursorElement (path, arg, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewController::_VisitPath (DisplayPathCP path, void* arg, ViewContextR context) const
    {
    if (NULL == path)
        return ERROR;

    // NEEDSWORK: _VisitPath with a DisplayPathCP is now un-necessary as you can just use VisitElemRef.
    //            We do still need to be able to visit a HitPathCP for segment/sub-selection flash/hilite...
    ViewContext::ContextMark mark (&context);

    return visitPath (*path, arg, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_DrawView (ViewContextR context) 
    {
    for (auto modelId : m_viewedModels)
        context.VisitDgnModel (m_project.Models().GetModelById(modelId));

    // Next, draw available reality data. It may be transparent, so we draw it after the element data. Note that reality data handlers will schedule themselves for progressive display as needed.
    bvector<RefCountedPtr<IRealityDataHandler>> realityHandlers;
    T_HOST.GetRealityDataAdmin()._GetRealityDataHandlers (realityHandlers, *context.GetViewport());

    for (auto realityHandler : realityHandlers)
        realityHandler->_DrawView (context);
    }
