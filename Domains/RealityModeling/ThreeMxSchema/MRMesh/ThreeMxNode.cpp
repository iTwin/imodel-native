/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/ThreeMxNode.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ThreeMxSchemaInternal.h"

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Node::~Node()
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    CacheManager::GetManager().RemoveRequest(*this);
#endif
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::Clear()
    {
    m_dir.clear();
    m_children.clear();
    m_meshes.clear();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::PushNode(NodeInfo const& nodeInfo)
    {
    if (nodeInfo.GetChildren().empty())
        return;

    m_children.push_back(new Node(nodeInfo, this));
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::AddGeometry(int32_t nodeId, Graphic::TriMeshArgs& args, int32_t textureIndex, SceneR scene)
    {
    ByteStream* jpeg = GetTextureData(textureIndex);
    if (nullptr != jpeg)
        {
        RgbImageInfo imageInfo;
        Render::Image rgba;
        if (SUCCESS != imageInfo.ReadImageFromJpgBuffer(rgba, jpeg->GetData(), jpeg->GetSize()))
            return;

        Render::TexturePtr texture;
        if (scene.m_renderSystem)
            {
            texture = scene.m_renderSystem->_CreateTexture(rgba, imageInfo.m_hasAlpha);
            jpeg->Clear(); // we no longer need the jpeg data
            }
        args.m_texture = texture.get();
        }

    m_meshes.push_back(new Geometry(args, scene));
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName Node::GetFileName() const
    {
    return m_info.GetChildren().empty() ? BeFileName() : Scene::ConstructNodeName(m_info.GetChildren().front(), nullptr == m_parent ? nullptr : &m_parent->m_dir);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Node::Load(SceneR scene)
    {
    if (!m_dir.empty())
        return SUCCESS;         // Already loaded.

    if (1 != m_info.GetChildren().size())
        {
        BeAssert(false);
        return ERROR;
        }

    if (SUCCESS != scene.SynchronousRead(*this, GetFileName()))
        {
        scene.DisplayNodeFailureWarning(GetFileName());
        m_dir.clear();
        return ERROR;
        }

    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Node::LoadUntilDisplayable(SceneR scene)
    {
    static uint32_t s_sleepMillis = 20;

    while (!LoadedUntilDisplayable())
        {
        scene.ProcessRequests();
        RequestLoadUntilDisplayable(scene);
        BeThreadUtilities::BeSleep(s_sleepMillis);

#ifdef NEEDS_WORK_LOAD_NOTIFY
        if (!loadingMessageDisplayed &&
            (mdlSystem_getTicks() - startTicks) > s_notifyTicks)
            {
            WString     message;
            RmgrResource::LoadWString(message, g_rfHandle, STRINGLISTID_Misc, MISC_Loading);

            loadingMessageDisplayed = true;
            NotificationManager().OutputMessage(NotifyMessageDetails(OutputMessagePriority::Info, message.c_str()));
            }
#endif
        }

#ifdef NEEDS_WORK_LOAD_NOTIFY
     if (loadingMessageDisplayed)
        {
        WString     message;

        RmgrResource::LoadWString(message, g_rfHandle, STRINGLISTID_Misc, MISC_LoadingComplete);

        NotificationManager().OutputMessage(NotifyMessageDetails(OutputMessagePriority::Info, message.c_str()));
        }
#endif
    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::LoadedUntilDisplayable() const
    {
    if (!IsLoaded())
        return false;

    if (IsDisplayable())
        return true;

    for (auto const& child : m_children)
        {
        if (!child->LoadedUntilDisplayable())
            return false;
        }

    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::RequestLoadUntilDisplayable(SceneR scene)
    {
    if (IsLoaded() && !IsDisplayable())
        {
        m_primary = true;           // Mark this as a primary node (will never get flushed).

        if (!m_childrenRequested)
            {
            m_childrenRequested = true;
            scene.QueueChildLoad(m_children, nullptr);
            }
        else
            {
            for (auto const& child : m_children)
                child->RequestLoadUntilDisplayable(scene);
            }
        }
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::AreChildrenLoaded() const
    {
    for (auto const& child : m_children)
        {
        if (!child->IsLoaded())
            return false;
        }

    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::Validate(NodeCP parent) const
    {
    for (auto const& child : m_children)
        {
        if (!child->Validate(this))
            return false;
        }

    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::IsCached() const
    {
    for (auto const& mesh : m_meshes)
        {
        if (!mesh->IsCached())
            return false;
        }

    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::AreVisibleChildrenLoaded(MeshNodes& unloadedVisibleChildren, ViewContextR viewContext, SceneR scene) const
    {
    if (scene.GetLoadSynchronous())
        {
        // If the LOD filter is off we assume that this is an application that is interested in full detail (and isn't going to wait for nodes to load.
        // We synchronously load unloaded but visible nodes and release non-visible nodes.
        for (auto& child : m_children)
            {
            if (child->IsDisplayable())
                {
                if (viewContext.IsPointVisible(scene.GetNodeCenter(child->m_info), ViewContext::WantBoresite::No, scene.GetNodeRadius(child->m_info)))
                    child->Load(scene);
                else if (!child->m_primary)
                    child->Clear();
                }
            }
        return true;
        }

    for (auto const& child : m_children)
        {
        if (!child->IsLoaded() && child->IsDisplayable() && viewContext.IsPointVisible(scene.GetNodeCenter(child->m_info), ViewContext::WantBoresite::No, scene.GetNodeRadius(child->m_info)))
            unloadedVisibleChildren.push_back(child);
        }

    return unloadedVisibleChildren.empty();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::DrawMeshes(RenderContextR context)
    {
    if (!IsLoaded())
        {
        BeAssert(false);
        return;
        }

    for (auto const& mesh : m_meshes)
        mesh->Draw(context);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::DrawBoundingSphere(RenderContextR context, SceneCR scene) const
    {
    Render::GraphicPtr graphic = context.CreateGraphic(Render::Graphic::CreateParams());

    graphic->AddSolidPrimitive(*ISolidPrimitive::CreateDgnSphere(DgnSphereDetail(scene.GetNodeCenter(m_info), scene.GetNodeRadius(m_info))));
    graphic->Close();
    context.OutputGraphic(*graphic, nullptr);
    }


#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::DrawMeshes(Render::GraphicR graphic, TransformCR transform)
    {
    if (SUCCESS != Load())
        return;

    for (auto const& mesh : m_meshes)
        {
        PolyfaceHeaderPtr  polyface = PolyfaceHeader::CreateVariableSizeIndexed();
        polyface->CopyFrom(*mesh->GetPolyface());
        polyface->Transform(transform);
        graphic.AddPolyface(*polyface, false);
        }
    }
#endif

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::TestVisibility(bool& isUnderMaximumSize, ViewContextR viewContext, SceneR scene)
    {
    if (!IsDisplayable())
        return true; // this seems wierd, but "is displayable" really means its a root node with no graphics. That means we need to draw its children.

    DPoint3d center = scene.GetNodeCenter(m_info);
    double radius = scene.GetNodeRadius(m_info);
    if (!viewContext.IsPointVisible(center, ViewContext::WantBoresite::No, radius))
        return false;

    if (scene.UseFixedResolution())
        {
        isUnderMaximumSize = (radius / scene.GetFixedResolution()) < m_info.GetMaxDiameter();
        }
    else
        {
        double pixelSize  =  radius / viewContext.GetPixelSizeAtPoint(&center);
        isUnderMaximumSize = pixelSize < Scene::CalculateResolutionRatio() * m_info.GetMaxDiameter();
        }

    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::Draw(RenderContextR context, SceneR scene)
    {
    bool childrenScheduled = false;
    bool isUnderMaximumSize = false, childrenNeedLoading = false;

    if (!TestVisibility(isUnderMaximumSize, context, scene) || !IsLoaded())
        return childrenScheduled;

    m_lastUsed = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    MeshNodes unloadedVisibleChildren;

    if ((!isUnderMaximumSize || !IsDisplayable()) &&
        !m_children.empty() &&
        !(childrenNeedLoading = !AreVisibleChildrenLoaded(unloadedVisibleChildren, context, scene)))
        {
        for (auto const& child : m_children)
            childrenScheduled |= child->Draw(context, scene);

        return childrenScheduled;
        }

    DrawMeshes(context);

    if (scene.GetLoadSynchronous() &&         // If we just are exporting fixed resolution clear here to mimimize memory usage.
        scene.UseFixedResolution() &&
        !m_primary)
        Clear();

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if ((DrawPurpose::Update  == viewContext.GetDrawPurpose() || DrawPurpose::UpdateHealing == viewContext.GetDrawPurpose()) &&
        !isUnderMaximumSize &&
        childrenNeedLoading &&
        nullptr != viewContext.GetViewport())
        {
        childrenScheduled = true;
        CacheManager::GetManager().QueueChildLoad(unloadedVisibleChildren, viewContext.GetViewport(), meshContext.GetTransform());
        }
#endif

    return childrenScheduled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Node::GetMeshCount() const
    {
    size_t count = m_meshes.size();

    for (auto const& child : m_children)
        count += child->GetMeshCount();

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Node::GetMeshMemorySize() const
    {
    size_t memorySize = 0;

    for (auto const& mesh : m_meshes)
        memorySize += mesh->GetMemorySize();

    for (auto const& child : m_children)
        memorySize += child->GetMeshMemorySize();

    return memorySize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Node::GetTextureMemorySize() const
    {
    size_t memorySize = 0;

    for (auto const& texture : m_jpegTextures)
        memorySize += texture.GetSize();

    for (auto const& child : m_children)
        memorySize += child->GetTextureMemorySize();

    return memorySize;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Node::GetNodeCount() const
    {
    size_t  count = IsLoaded() ? 1 : 0;

    for (auto const& child : m_children)
        count += child->GetNodeCount();

    return count;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Node::GetMaxDepth() const
    {
    size_t maxChildDepth = 0;

    for (auto const& child : m_children)
        {
        size_t childDepth = child->GetMaxDepth();
        if (childDepth > maxChildDepth)
            maxChildDepth = childDepth;
        }

    return 1 + maxChildDepth;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::RemoveChild(NodeP failedChild)
    {
    for (auto child = m_children.begin(); child != m_children.end(); ++child)
        {
        if (child->get() == failedChild)
            {
            m_children.erase(child);
            return;
            }
        }
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::ClearGraphics()
    {
    for (auto const& mesh : m_meshes)
        mesh->ClearGraphic();

    for (auto const& child : m_children)
        child->ClearGraphics();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                        Grigas.Petraitis            04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::Clone(Node const& other)
    {
    BeAssert(&other != this);
    m_parent    = other.m_parent;
    m_info      = other.m_info;
    m_dir       = other.m_dir;
    m_meshes    = other.m_meshes;
    m_children.clear();

    for (auto const& otherChild : other.m_children)
        {
        NodePtr child = new Node(NodeInfo(), this);
        child->Clone(*otherChild);
        child->m_parent = this;
        m_children.push_back(child);
        }
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void  Node::GetDepthMap(bvector<size_t>& map, bvector <bset<BeFileName>>& fileNames, size_t depth)
    {
    if (!IsLoaded())
        return;

    if (map.size() <= depth)
        map.push_back(1);
    else
        map[depth]++;

    if (fileNames.size() <= depth)
        {
        bset<BeFileName>    thisNames;

        thisNames.insert(GetFileName());
        fileNames.push_back(thisNames);
        }
    else
        {
        if (fileNames[depth].find(GetFileName()) != fileNames[depth].end())
            {
            BeAssert(false && ">>>>>>>>>>>>>>>>>>>>>Duplicate<<<<<<<<<<<<<<<<<<<<<<<<");
            }
        fileNames[depth].insert(GetFileName());
        }

    for (auto& child : m_children)
        child->GetDepthMap(map, fileNames, depth+1);
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Node::GetSphereRange(SceneCR scene) const
    {
    DRange3d range;
    range.InitFrom(scene.GetNodeCenter(m_info));
    range.Extend(scene.GetNodeRadius(m_info));
    return range;
    }
#endif

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Node::GetRange(TransformCR trans) const
    {
    DRange3d range = DRange3d::NullRange();

    for (auto const& child : m_children)     // Prefer the more accurate child range...
        range.UnionOf(range, child->GetRange(trans));

    if (range.IsNull())
        {
        for (auto const& mesh : m_meshes)
            range.UnionOf(range, mesh->GetRange(trans));
        }

    return range;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::FlushStale(uint64_t staleTime)
    {
    if (m_lastUsed < staleTime)
        {
        Clear();
        }
    else
        {
        for (auto const& child : m_children)
            child->FlushStale(staleTime);
        }
    }

