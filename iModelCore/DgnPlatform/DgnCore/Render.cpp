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
    for (auto it=m_scene.begin(); it != m_scene.end(); ++it)
        {
        if (it->get() == &graphic)
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
    return m_currentGeomSource->Graphics().Find(*GetViewport(), pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt CreateSceneContext::_InitContextForView()
    {
    m_viewport->InitViewSettings(true);

    if (SUCCESS != T_Super::_InitContextForView())
        return ERROR;

//    InitializeRendering(true); // init rendering for whole view, including lights

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateSceneContext::_SaveGraphic()
    {
    if (!m_currGraphic.IsValid())
        return;

    m_currGraphic->FinishDrawing(); // save the graphic on the element, even if this fails so we don't attempt to stroke it again.
    m_currentGeomSource->Graphics().Save(*m_currGraphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateSceneContext::_OutputElement(GeometrySourceCR element)
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

    if (SUCCESS != _Attach(&vp, DrawPurpose::CreateScene))
        return true;

    VisitAllViewElements(true, nullptr);
    _Detach();

    return WasAborted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RenderManager::AddTask(Task& task)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    m_tasks.push_back(&task);

    m_cv.notify_all();
    }
