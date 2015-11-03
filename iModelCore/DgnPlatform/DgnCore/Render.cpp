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
    m_scene.push_back(&graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Scene::_DropGraphic(Graphic& graphic)
    {
    auto it = std::find(m_scene.begin(), m_scene.end(), &graphic);
    if (it == m_scene.end())
        {
        BeAssert(false);
        return;
        }
    m_scene.erase(it);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Scene::_Clear()
    {
    m_scene.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr CreateSceneContext::_OutputElement(GeometricElementCR element)
    {
    Render::GraphicPtr graphic = element.GetGraphicFor(*this, true);

    if (graphic.IsValid())
        m_scene.AddGraphic(*graphic);

    return graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool CreateSceneContext::CreateScene(DgnViewportR vp)
    {
    InitAborted(false);

    vp.GetViewControllerR()._OnFullUpdate(vp, *this);

    if (SUCCESS != _Attach(&vp, DrawPurpose::Update))
        return true;

    VisitAllViewElements(true, nullptr);
    _Detach();

    return WasAborted();
    }
