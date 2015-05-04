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
StatusInt ViewController::_VisitHit (HitPathCR hit, ViewContextR context) const
    {
    GeometricElementCP element = (nullptr != hit.GetHeadElem() ? hit.GetHeadElem()->ToGeometricElement() : nullptr);

    if (nullptr == element)
        return ERROR;

    ViewContext::ContextMark mark(&context);

    // Allow element sub-class involvement for flashing sub-entities...
    if (element->_DrawHit (hit, context))
        return SUCCESS;

    return context.VisitElement(*element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewController::_DrawView (ViewContextR context) 
    {
    for (auto modelId : m_viewedModels)
        context.VisitDgnModel (m_dgndb.Models().GetModelById(modelId));
    }
