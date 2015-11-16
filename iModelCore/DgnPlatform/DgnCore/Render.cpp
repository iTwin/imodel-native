/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Render.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Scene::_AddGraphic(Graphic& graphic)
    {
    m_scene.push_back(Node(graphic));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Scene::_DropGraphic(Graphic& graphic)
    {
    for (auto it=m_scene.begin(); it != m_scene.end(); ++it)
        {
        if (it->m_graphic.get() == &graphic)
            {
            m_scene.erase(it);
            return;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Scene::_Clear()
    {
    m_scene.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
Render::Graphic* CreateSceneContext::_GetCachedGraphic(double pixelSize) 
    {
    return m_currGeomElement->Graphics().Find(*GetViewport(), pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateSceneContext::_SaveGraphic()
    {
    if (!m_currGraphic.IsValid())
        return;

    m_currGraphic->FinishDrawing(); // save the graphic on the element, even if this fails so we don't attempt to stroke it again.
    m_currGeomElement->Graphics().Save(*m_currGraphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateSceneContext::_OutputElement(GeometricElementCR element)
    {
    T_Super::_OutputElement(element);
    if (m_currGraphic.IsValid())
        m_scene.AddGraphic(*m_currGraphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool CreateSceneContext::CreateScene(DgnViewportR vp)
    {
    m_scene.Clear();

    InitAborted(false);

    vp.GetViewControllerR()._OnFullUpdate(vp, *this);

    if (SUCCESS != _Attach(&vp, DrawPurpose::CreateScene))
        return true;

    VisitAllViewElements(true, nullptr);
    _Detach();

    return WasAborted();
    }
